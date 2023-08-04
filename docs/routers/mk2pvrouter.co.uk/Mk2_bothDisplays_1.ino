/* Mk2_bothDisplays_1.ino
 *
 * This sketch is for diverting surplus PV power to a dump load using a triac. 
 * It is based on the Mk2i PV Router code that I have posted in on the 
 * OpenEnergyMonitor forum.  The original version, and other related material, 
 * can be found on my Summary Page at www.openenergymonitor.org/emon/node/1757
 *
 * In this latest version, the pin-allocations have been changed to suit my 
 * PCB-based hardware for the Mk2 PV Router.  The integral voltage sensor is 
 * fed from one of the secondary coils of the transformer.  Current is measured 
 * via Current Transformers at the CT1 and CT2 ports.  
 * 
 * CT1 is for 'grid' current, to be measured at the grid supply point.
 * CT2 is for the load current, so that diverted energy can be recorded
 *
 * A persistence-based 4-digit display is supported. This can be driven in two
 * different ways, one with an extra pair of logic chips, and one without.  The 
 * appropriate version of the sketch must be selected by including or commenting 
 * out the "#define PIN_SAVING_HARDWARE" statement near the top of the code.
 *
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 *      February 2014
 */

#include <Arduino.h> 

#include <TimerOne.h>
#define ADC_TIMER_PERIOD 125 // uS (determines the sampling rate / amount of idle time)

// Physical constants, please do not change!
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define JOULES_PER_WATT_HOUR 3600 //  (0.001 kWh = 3600 Joules)

// Change these values to suit the local mains frequency and supply meter
#define CYCLES_PER_SECOND 50 
#define SWEETZONE_IN_JOULES 3600 

// to prevent the diverted energy total from 'creeping'
#define ANTI_CREEP_LIMIT 5 // in Joules per mains cycle (has no effect when set to 0)
long antiCreepLimit_inIEUperMainsCycle;

//  The two versions of the hardware require different logic.  The following line should 
//  be included if the additional logic chips are present, or excluded if they are 
//  absent (in which case some wire links need to be fitted)
//
//#define PIN_SAVING_HARDWARE 

// definition of enumerated types
enum polarities {NEGATIVE, POSITIVE};
enum triacStates {TRIAC_ON, TRIAC_OFF}; // the external trigger device is active low
enum outputModes {ANTI_FLICKER, NORMAL};

// ----------------  Extra Features selection ----------------------
//
// - WORKLOAD_CHECK, for determining how much spare processing time there is. 
//  
// #define WORKLOAD_CHECK  // <-- Include this line is this feature is required 


// The power-diversion logic can operate in either of two modes:
//
// - NORMAL, where the triac switches rapidly on/off to maintain a constant energy level.
// - ANTI_FLICKER, whereby the repetition rate is reduced to avoid rapid fluctuations
//    of the local mains voltage.
//
// The output mode is determined in realtime via a selector switch
enum outputModes outputMode;                                                   

// allocation of digital pins for prototype PCB-based rig (with simple display adapter)
// ******************************************************
// D0 & D1 are reserved for the Serial i/f
// D2 is a driver line for the 4-digit display (segment D, via series resistor)
const byte outputModeSelectorPin = 3; // <-- with the internal pullup 
const byte outputForTrigger = 4;   
// D5 is a driver line for the 4-digit display (segment B, via series resistor)
// D6 is a driver line for the 4-digit display (digit 3, via wire link)
// D7 is a driver line for the 4-digit display (digit 2, via wire link)
// D8 is a driver line for the 4-digit display (segment F, via series resistor)
// D9 is a driver line for the 4-digit display (segment A, via series resistor)
// D10 is a driver line for the 4-digit display (segment DP, via series resistor)
// D11 is a driver line for the 4-digit display (segment C, via series resistor)
// D12 is a driver line for the 4-digit display (segment G, via series resistor)
// D13 is a driver line for the 4-digit display (digit 4, via wire link)

// allocation of analogue pins
// ***************************
// A0 (D14) is a driver line for the 4-digit display (digit 1, via wire link)
// A1 (D15) is a driver line for the 4-digit display (segment E, via series resistor)
// A2 (D16) is unused (it's routed to pin 1 of IC4 which is not fitted)
const byte voltageSensor = 3;          // A3 is for the voltage sensor
const byte currentSensor_diverted = 4; // A4 is for CT2 which measures diverted current
const byte currentSensor_grid = 5;     // A5 is for CT1 which measures grid current


const byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;    // nominal mid-point value of ADC @ x1 scale

// General global variables that are used in multiple blocks so cannot be static.
// For integer maths, many variables need to be 'long'
//
boolean beyondStartUpPhase = false;     // start-up delay, allows things to settle
long cycleCount = 0;                    // counts mains cycles from start-up 
long triggerThreshold_long;        // for determining when the trigger may be safely armed
long energyInBucket_long;          // in Integer Energy Units
long capacityOfEnergyBucket_long;  // depends on powerCal, frequency & the 'sweetzone' size.
long lowerEnergyThreshold_long;    // for turning triac off
long upperEnergyThreshold_long;    // for turning triac on
// int phaseCal_grid_int;             // to avoid the need for floating-point maths
// int phaseCal_diverted_int;         // to avoid the need for floating-point maths
long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF
long divertedEnergyRecent_IEU = 0; // Hi-res accumulator of limited range
unsigned int divertedEnergyTotal_Wh = 0; // WattHour register of 63K range
long IEU_per_Wh; // depends on powerCal, frequency & the 'sweetzone' size.

unsigned long displayShutdown_inMainsCycles; 
unsigned long absenceOfDivertedEnergyCount = 0;
long mainsCyclesPerHour;

// this setting is only used if anti-flicker mode is enabled
float offsetOfEnergyThresholdsInAFmode = 0.1; // <-- must not exceeed 0.5

// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
int sampleI_grid;
int sampleI_diverted;
int sampleV;


// Calibration values
//-------------------
// Two calibration values are used: powerCal and phaseCal. 
// With most hardware, the default values are likely to work fine without 
// need for change.  A full explanation of each of these values now follows:
//   
// powerCal is a floating point variable which is used for converting the 
// product of voltage and current samples into Watts.
//
// The correct value of powerCal is dependent on the hardware that is 
// in use.  For best resolution, the hardware should be configured so that the 
// voltage and current waveforms each span most of the ADC's usable range.  For 
// many systems, the maximum power that will need to be measured is around 3kW. 
//
// My sketch "MinAndMaxValues.ino" provides a good starting point for 
// system setup.  First arrange for the CT to be clipped around either core of a  
// cable which supplies a suitable load; then run the tool.  The resulting values 
// should sit nicely within the range 0-1023.  To allow some room for safety, 
// a margin of around 100 levels should be left at either end.  This gives a 
// output range of around 800 ADC levels, which is 80% of its usable range.
//
// My sketch "RawSamplesTool.ino" provides a one-shot visual display of the
// voltage and current waveforms.  This provides an easy way for the user to be 
// confident that their system has been set up correctly for the power levels 
// that are to be measured.
//
// The ADC has an input range of 0-5V and an output range of 0-1023 levels.
// The purpose of each input sensor is to convert the measured parameter into a 
// low-voltage signal which fits nicely within the ADC's input range. 
//
// In the case of 240V mains voltage, the numerical value of the input signal 
// in Volts is likely to be fairly similar to the output signal in ADC levels.  
// 240V AC has a peak-to-peak amplitude of 679V, which is not far from the ideal 
// output range.  Stated more formally, the conversion rate of the overall system 
// for measuring VOLTAGE is likely to be around 1 ADC-step per Volt (RMS).
//
// In the case of AC current, however, the situation is very different.  At
// mains voltage, a power of 3kW corresponds to an RMS current of 12.5A which 
// has a peak-to-peak range of 35A.  This is smaller than the output signal by 
// around a factor of twenty.  The conversion rate of the overall system for 
// measuring CURRENT is therefore likely to be around 20 ADC-steps per Amp.
//
// When calculating "real power", which is what this code does, the individual 
// conversion rates for voltage and current are not of importance.  It is 
// only the conversion rate for POWER which is important.  This is the 
// product of the individual conversion rates for voltage and current.  It 
// therefore has the units of ADC-steps squared per Watt.  Most systems will
// have a power conversion rate of around 20 (ADC-steps squared per Watt).
// 
// powerCal is the RECIPR0CAL of the power conversion rate.  A good value 
// to start with is therefore 1/20 = 0.05 (Watts per ADC-step squared)
//
const float powerCal_grid = 0.0435;  // for CT1
const float powerCal_diverted = 0.0435;  // for CT2
 
                        
// phaseCal is used to alter the phase of the voltage waveform relative to the
// current waveform.  The algorithm interpolates between the most recent pair
// of voltage samples according to the value of phaseCal. 
//
//    With phaseCal = 1, the most recent sample is used.  
//    With phaseCal = 0, the previous sample is used
//    With phaseCal = 0.5, the mid-point (average) value in used
//
// Values ouside the 0 to 1 range involve extrapolation, rather than interpolation
// and are not recommended.  By altering the order in which V and I samples are 
// taken, and for how many loops they are stored, it should always be possible to
// arrange for the optimal value of phaseCal to lie within the range 0 to 1.  When 
// measuring a resistive load, the voltage and current waveforms should be perfectly 
// aligned.  In this situation, the Power Factor will be 1.
//
// My sketch "PhasecalChecker.ino" provides an easy way to determine the correct 
// value of phaseCal for any hardware configuration.  An index of my various Mk2-related
// exhibits is available at http://openenergymonitor.org/emon/node/1757
//
//const float  phaseCal_grid = 1.0;  <--- not used in this version
//const float  phaseCal_diverted = 1.0;  <--- not used in this version

// Various settings for the 4-digit display, which needs to be refreshed every few mS
const byte noOfDigitLocations = 4;
const byte noOfPossibleCharacters = 22;
#define MAX_DISPLAY_TIME_COUNT 10// no of processing loops between display updates
#define UPDATE_PERIOD_FOR_DISPLAYED_DATA 50 // mains cycles
#define DISPLAY_SHUTDOWN_IN_HOURS 10 // auto-reset after this period of inactivity
// #define DISPLAY_SHUTDOWN_IN_HOURS 0.01 // for testing that the display clears after 36 seconds

//  The two versions of the hardware require different logic.
#ifdef PIN_SAVING_HARDWARE

#define DRIVER_CHIP_DISABLED HIGH
#define DRIVER_CHIP_ENABLED LOW

// the primary segments are controlled by a pair of logic chips
const byte noOfDigitSelectionLines = 4; // <- for the 74HC4543 7-segment display driver
const byte noOfDigitLocationLines = 2; // <- for the 74HC138 2->4 line demultiplexer

byte enableDisableLine = 5; // <- affects the primary 7 segments only (not the DP)
byte decimalPointLine = 14; // <- this line has to be individually controlled. 

byte digitLocationLine[noOfDigitLocationLines] = {16,15};
byte digitSelectionLine[noOfDigitSelectionLines] = {7,9,8,6};

// The final column of digitValueMap[] is for the decimal point status.  In this version, 
// the decimal point has to be treated differently than the other seven segments, so 
// a convenient means of accessing this column is provided.
//
byte digitValueMap[noOfPossibleCharacters][noOfDigitSelectionLines +1] = {
                 LOW , LOW , LOW , LOW , LOW , // '0' <- element 0
                 LOW , LOW , LOW , HIGH, LOW , // '1' <- element 1
                 LOW , LOW , HIGH, LOW , LOW , // '2' <- element 2
                 LOW , LOW , HIGH, HIGH, LOW , // '3' <- element 3
                 LOW , HIGH, LOW , LOW , LOW , // '4' <- element 4
                 LOW , HIGH, LOW , HIGH, LOW , // '5' <- element 5
                 LOW , HIGH, HIGH, LOW , LOW , // '6' <- element 6
                 LOW , HIGH, HIGH, HIGH, LOW , // '7' <- element 7
                 HIGH, LOW , LOW , LOW , LOW , // '8' <- element 8
                 HIGH, LOW , LOW , HIGH, LOW , // '9' <- element 9
                 LOW , LOW , LOW , LOW , HIGH, // '0.' <- element 10
                 LOW , LOW , LOW , HIGH, HIGH, // '1.' <- element 11
                 LOW , LOW , HIGH, LOW , HIGH, // '2.' <- element 12
                 LOW , LOW , HIGH, HIGH, HIGH, // '3.' <- element 13
                 LOW , HIGH, LOW , LOW , HIGH, // '4.' <- element 14
                 LOW , HIGH, LOW , HIGH, HIGH, // '5.' <- element 15
                 LOW , HIGH, HIGH, LOW , HIGH, // '6.' <- element 16
                 LOW , HIGH, HIGH, HIGH, HIGH, // '7.' <- element 17
                 HIGH, LOW , LOW , LOW , HIGH, // '8.' <- element 18
                 HIGH, LOW , LOW , HIGH, HIGH, // '9.' <- element 19
                 HIGH, HIGH, HIGH, HIGH, LOW , // ' '  <- element 20          
                 HIGH, HIGH, HIGH, HIGH, HIGH  // '.'  <- element 21
};

// a tidy means of identifying the DP status data when accessing the above table
const byte DPstatus_columnID = noOfDigitSelectionLines; 

byte digitLocationMap[noOfDigitLocations][noOfDigitLocationLines] = {
                 LOW , LOW ,    // Digit 1
                 LOW , HIGH,    // Digit 2
                 HIGH, LOW ,    // Digit 3
                 HIGH, HIGH,    // Digit 4
};

#else // PIN_SAVING_HARDWARE

#define ON HIGH
#define OFF LOW

const byte noOfSegmentsPerDigit = 8; // includes one for the decimal point
enum digitEnableStates {DIGIT_ENABLED, DIGIT_DISABLED};

byte digitSelectorPin[noOfDigitLocations] = {16,10,13,11};
byte segmentDrivePin[noOfSegmentsPerDigit] = {2,5,12,6,7,9,8,14};

// The final column of segMap[] is for the decimal point status.  In this version, 
// the decimal point is treated just like all the other segments, so there is
// no need to access this column specifically.
//
byte segMap[noOfPossibleCharacters][noOfSegmentsPerDigit] = {
                 ON , ON , ON , ON , ON , ON , OFF, OFF, // '0' <- element 0
                 OFF, ON , ON , OFF, OFF, OFF, OFF, OFF, // '1' <- element 1
                 ON , ON , OFF, ON , ON , OFF, ON , OFF, // '2' <- element 2
                 ON , ON , ON , ON , OFF, OFF, ON , OFF, // '3' <- element 3
                 OFF, ON , ON , OFF, OFF, ON , ON , OFF, // '4' <- element 4
                 ON , OFF, ON , ON , OFF, ON , ON , OFF, // '5' <- element 5
                 ON , OFF, ON , ON , ON , ON , ON , OFF, // '6' <- element 6
                 ON , ON , ON , OFF, OFF, OFF, OFF, OFF, // '7' <- element 7
                 ON , ON , ON , ON , ON , ON , ON , OFF, // '8' <- element 8
                 ON , ON , ON , ON , OFF, ON , ON , OFF, // '9' <- element 9
                 ON , ON , ON , ON , ON , ON , OFF, ON , // '0.' <- element 10
                 OFF, ON , ON , OFF, OFF, OFF, OFF, ON , // '1.' <- element 11
                 ON , ON , OFF, ON , ON , OFF, ON , ON , // '2.' <- element 12
                 ON , ON , ON , ON , OFF, OFF, ON , ON , // '3.' <- element 13
                 OFF, ON , ON , OFF, OFF, ON , ON , ON , // '4.' <- element 14
                 ON , OFF, ON , ON , OFF, ON , ON , ON , // '5.' <- element 15
                 ON , OFF, ON , ON , ON , ON , ON , ON , // '6.' <- element 16
                 ON , ON , ON , OFF, OFF, OFF, OFF, ON , // '7.' <- element 17
                 ON , ON , ON , ON , ON , ON , ON , ON , // '8.' <- element 18
                 ON , ON , ON , ON , OFF, ON , ON , ON , // '9.' <- element 19
                 OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, // ' ' <- element 20
                 OFF, OFF, OFF, OFF, OFF, OFF, OFF, ON  // '.' <- element 11
};
#endif // PIN_SAVING_HARDWARE

byte charsForDisplay[noOfDigitLocations] = {20,20,20,20}; // all blank 

boolean EDD_isActive = false; // energy divertion detection


void setup()
{  
  pinMode(outputForTrigger, OUTPUT);  
  digitalWrite (outputForTrigger, TRIAC_OFF); // the external trigger is active low
  
  pinMode(outputModeSelectorPin, INPUT);
  digitalWrite(outputModeSelectorPin, HIGH); // enable the internal pullup resistor
  delay (100); // allow time to settle
  int pinState = digitalRead(outputModeSelectorPin);  // initial selection and
  outputMode = (enum outputModes)pinState;            //   assignment of output mode
 
  delay(5000); // allow time to open Serial monitor     
 
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      Mk2_bothDisplays_1.ino");
  Serial.println();
       
#ifdef PIN_SAVING_HARDWARE
  pinMode(enableDisableLine, OUTPUT); // for the 74HC4543 7-seg display driver
  digitalWrite( enableDisableLine, DRIVER_CHIP_DISABLED);  
  pinMode(decimalPointLine, OUTPUT); // the 'decimal point' line 
  
  // control lines for the 74HC4543 7-seg display driver and the DP line
  for (int i = 0; i < noOfDigitLocationLines; i++) {
    pinMode(digitLocationLine[i], OUTPUT); } 
  
  // control lines for the 74HC138 2->4 demux 
  for (int i = 0; i < noOfDigitSelectionLines; i++) {
    pinMode(digitSelectionLine[i], OUTPUT); }
#else
  for (int i = 0; i < noOfSegmentsPerDigit; i++) {
    pinMode(segmentDrivePin[i], OUTPUT); }
  
  for (int i = 0; i < noOfDigitLocations; i++) {
    pinMode(digitSelectorPin[i], OUTPUT); }
    
   for (int i = 0; i < noOfDigitLocations; i++) {
    digitalWrite(digitSelectorPin[i], DIGIT_DISABLED); }
  
  for (int i = 0; i < noOfSegmentsPerDigit; i++) {
    digitalWrite(segmentDrivePin[i], OFF); }
#endif
      
  
       
  // When using integer maths, calibration values that have supplied in floating point 
  // form need to be rescaled.  
  //
//  phaseCal_grid_int = phaseCal_grid * 256; // for integer maths
//  phaseCal_diverted_int = phaseCal_diverted * 256; // for integer maths
  
  // When using integer maths, the SIZE of the ENERGY BUCKET is altered to match the
  // scaling of the energy detection mechanism that is in use.  This avoids the need 
  // to re-scale every energy contribution, thus saving processing time.  This process 
  // is described in more detail in the function, allGeneralProcessing(), just before 
  // the energy bucket is updated at the start of each new cycle of the mains.
  //
  // An electricity meter has a small range over which energy can ebb and flow without 
  // penalty.  This has been termed its "sweet-zone".  For optimal performance, the energy
  // bucket of a PV Router should match this value.  The sweet-zone value is therefore 
  // included in the calculation below.
  //
  // For the flow of energy at the 'grid' connection point (CT1) 
  capacityOfEnergyBucket_long = 
     (long)SWEETZONE_IN_JOULES * CYCLES_PER_SECOND * (1/powerCal_grid);
  energyInBucket_long = capacityOfEnergyBucket_long * 0.45; // for rapid start up
  
  // For recording the accumulated amount of diverted energy data (using CT2), a similar 
  // calibration mechanism is required.  Rather than a bucket with a fixed capacity, the 
  // accumulator for diverted energy just needs to be scaled correctly.  As soon as its 
  // value exceeds 1 Wh, an associated WattHour register is incremented, and the 
  // accumulator's value is decremented accordingly. The calculation below is to determine
  // the scaling for this accumulator.
  
  IEU_per_Wh = 
     (long)JOULES_PER_WATT_HOUR * CYCLES_PER_SECOND * (1/powerCal_diverted); 
 
  antiCreepLimit_inIEUperMainsCycle = (float)ANTI_CREEP_LIMIT * (1/powerCal_grid);

  mainsCyclesPerHour = (long)CYCLES_PER_SECOND * 
                             SECONDS_PER_MINUTE * MINUTES_PER_HOUR;
                             
  displayShutdown_inMainsCycles = DISPLAY_SHUTDOWN_IN_HOURS * mainsCyclesPerHour;                           
      
  // Define operating limits for the LP filter which identifies DC offset in the voltage 
  // sample stream.  By limiting the output range, the filter always should start up 
  // correctly.
  DCoffset_V_long = 512L * 256; // nominal mid-point value of ADC @ x256 scale  
  DCoffset_V_min = (long)(512L - 100) * 256; // mid-point of ADC minus a working margin
  DCoffset_V_max = (long)(512L + 100) * 256; // mid-point of ADC plus a working margin

  Serial.print ("ADC mode:       ");
  Serial.print (ADC_TIMER_PERIOD);
  Serial.println ( " uS fixed timer");

  // Set up the ADC to be triggered by a hardware timer of fixed duration  
  ADCSRA  = (1<<ADPS0)+(1<<ADPS1)+(1<<ADPS2);  // Set the ADC's clock to system clock / 128
  ADCSRA |= (1 << ADEN);                 // Enable ADC

  Timer1.initialize(ADC_TIMER_PERIOD);   // set Timer1 interval
  Timer1.attachInterrupt( timerIsr );    // declare timerIsr() as interrupt service routine

  Serial.print ( "Output mode:    ");
  if (outputMode == NORMAL) {
    Serial.println ( "normal"); }
  else 
  {  
    Serial.println ( "anti-flicker");
    Serial.print ( "  offsetOfEnergyThresholds  = ");
    Serial.println ( offsetOfEnergyThresholdsInAFmode);    
  }
    
  Serial.print ( "Extra Features: ");  
#ifdef WORKLOAD_CHECK  
  Serial.println ( "WORKLOAD_CHECK ");
#else
    Serial.println ("none"); 
#endif
  Serial.println ();
        
  Serial.print ( "powerCal_grid =      "); Serial.println (powerCal_grid,4);
  Serial.print ( "powerCal_diverted = "); Serial.println (powerCal_diverted,4);
  
  Serial.print ("Anti-creep limit (Joules / mains cycle) = ");
  Serial.println (ANTI_CREEP_LIMIT);
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on
  configureParamsForSelectedOutputMode(); 

  Serial.println ("----");    

#ifdef WORKLOAD_CHECK
   Serial.println ("WELCOME TO WORKLOAD_CHECK ");
  
//   <<- start of commented out section, to save on RAM space!
/*   
   Serial.println ("  This mode of operation allows the spare processing capacity of the system");
   Serial.println ("to be analysed.  Additional delay is gradually increased until all spare time");
   Serial.println ("has been used up.  This value (in uS) is noted and the process is repeated.  ");
   Serial.println ("The delay setting is increased by 1uS at a time, and each value of delay is ");
   Serial.println ("checked several times before the delay is increased. "); 
 */ 
//  <<- end of commented out section, to save on RAM space!

   Serial.println ("  The displayed value is the amount of spare time, per set of V & I samples, ");
   Serial.println ("that is available for doing additional processing.");
   Serial.println ();
 #endif
}

// An Interrupt Service Routine is now defined in which the ADC is instructed to 
// measure V and I alternately.  A "data ready"flag is set after each voltage conversion 
// has been completed.  
//   For each pair of samples, this means that current is measured before voltage.  The 
// current sample is taken first because the phase of the waveform for current is generally 
// slightly advanced relative to the waveform for voltage.  The data ready flag is cleared 
// within loop().
//   This Interrupt Service Routine is for use when the ADC is fixed timer mode.  It is 
// executed whenever the ADC timer expires.  In this mode, the next ADC conversion is 
// initiated from within this ISR.  
//
void timerIsr(void)
{                                         
  static unsigned char sample_index = 0;

  switch(sample_index)
  {
    case 0:
      sampleV = ADC;                    // store the ADC value (this one is for Voltage)
      ADMUX = 0x40 + currentSensor_diverted;  // set up the next conversion, which is for Diverted Current
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index++;                   // increment the control flag
      dataReady = true;                 // all three ADC values can now be processed
      break;
    case 1:
      sampleI_diverted = ADC;               // store the ADC value (this one is for Diverted Current)
      ADMUX = 0x40 + currentSensor_grid;  // set up the next conversion, which is for Grid Current
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index++;                   // increment the control flag
      break;
    case 2:
      sampleI_grid = ADC;               // store the ADC value (this one is for Grid Current)
      ADMUX = 0x40 + voltageSensor;  // set up the next conversion, which is for Voltage
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index = 0;                 // reset the control flag
      break;
    default:
      sample_index = 0;                 // to prevent lockup (should never get here)      
  }
}


// When using interrupt-based logic, the main processor waits in loop() until the 
// dataReady flag has been set by the ADC.  Once this flag has been set, the main
// processor clears the flag and proceeds with all the processing for one set of 
// V & I samples.  It then returns to loop() to wait for the next set to become 
// available.
//   If the next set of samples become available before the processing of the 
// previous set has been completed, data could be lost.  This situation can be 
// avoided by prior use of the WORKLOAD_CHECK mode.  Using this facility, the amount
// of spare processing capacity per loop can be determined.  
//   If there is insufficient processing capacity to do all that is required, the 
// base workload can be reduced by increasing the duration of ADC_TIMER_PERIOD.
//
void loop()             
{ 
#ifdef WORKLOAD_CHECK
  static int del = 0; // delay, as passed to delayMicroseconds()
  static int res = 0; // result, to be displayed at the next opportunity
  static byte count = 0; // to allow multiple runs per setting
  static byte displayFlag = 0; // to determine when printing may occur
#endif
  
  if (dataReady)   // flag is set after every pair of ADC conversions
  {
    dataReady = false; // reset the flag
    allGeneralProcessing(); // executed once for each pair of V&I samples
    
#ifdef WORKLOAD_CHECK 
    delayMicroseconds(del); // <--- to assess how much spare time there is
    if (dataReady)       // if data is ready again, delay was too long
    { 
      res = del;             // note the exact value
      del = 1;               // and start again with 1us delay   
      count = 0;
      displayFlag = 0;   
    }
    else
    {
      count++;          // to give several runs with the same value
      if (count > 50)
      {
        count = 0;
        del++;          //  increase delay by 1uS
      } 
    }
#endif  

  }  // <-- this closing brace needs to be outside the WORKLOAD_CHECK blocks! 
  
#ifdef WORKLOAD_CHECK 
  switch (displayFlag) 
  {
    case 0: // the result is available now, but don't display until the next loop
      displayFlag++;
      break;
    case 1: // with minimum delay, it's OK to print now
      Serial.print(res);
      displayFlag++;
      break;
    case 2: // with minimum delay, it's OK to print now
      Serial.println("uS");
      displayFlag++;
      break;
    default:; // for most of the time, displayFlag is 3           
  }
#endif
  
} // end of loop()


// This routine is called to process each set of V & I samples. The main processor and 
// the ADC work autonomously, their operation being only linked via the dataReady flag.  
// As soon as a new set of data is made available by the ADC, the main processor can 
// start to work on it immediately.  
//
void allGeneralProcessing()
{
  static boolean triggerNeedsToBeArmed = false;  // once per mains cycle (+ve half)
  static int samplesDuringThisCycle;             // for normalising the power in each mains cycle
  static long sumP_grid;                              // for per-cycle summation of 'real power' 
  static long sumP_diverted;                              // for per-cycle summation of 'real power' 
  static enum polarities polarityOfLastSampleV;  // for zero-crossing detection
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static long lastSampleVminusDC_long;     //    for the phaseCal algorithm
  static byte timerForDisplayUpdate = 0;
  static enum triacStates nextStateOfTriac = TRIAC_OFF;  

  // remove DC offset from the raw voltage sample by subtracting the accurate value 
  // as determined by a LP filter.
  long sampleVminusDC_long = ((long)sampleV<<8) - DCoffset_V_long; 

  // determine the polarity of the latest voltage sample
  enum polarities polarityNow;   
  if(sampleVminusDC_long > 0) { 
    polarityNow = POSITIVE; }
  else { 
    polarityNow = NEGATIVE; }

  if (polarityNow == POSITIVE) 
  { 
    if (beyondStartUpPhase)
    {     
      if (polarityOfLastSampleV != POSITIVE)
      {
        // This is the start of a new +ve half cycle (just after the zero-crossing point)
        cycleCount++;  
        
        triggerNeedsToBeArmed = true; // the trigger is armed once during each +ve half-cycle 
    
        // Calculate the real power and energy during the last whole mains cycle.
        //
        // sumP contains the sum of many individual calculations of instantaneous power.  In  
        // order to obtain the average power during the relevant period, sumP must first be 
        // divided by the number of samples that have contributed to its value.
        //
        // The next stage would normally be to apply a calibration factor so that real power 
        // can be expressed in Watts.  That's fine for floating point maths, but it's not such
        // a good idea when integer maths is being used.  To keep the numbers large, and also 
        // to save time, calibration of power is omitted at this stage.  Real Power (stored as 
        // a 'long') is therefore (1/powerCal) times larger than the actual power in Watts.
        //
        long realPower_grid = sumP_grid / samplesDuringThisCycle; // proportional to Watts
        long realPower_diverted = sumP_diverted / samplesDuringThisCycle; // proportional to Watts
   
        // Next, the energy content of this power rating needs to be determined.  Energy is 
        // power multiplied by time, so the next step is normally to multiply the measured
        // value of power by the time over which it was measured.
        //   Instanstaneous power is calculated once every mains cycle. When integer maths is 
        // being used, a repetitive power-to-energy conversion seems an unnecessary workload.  
        // As all sampling periods are of similar duration, it is more efficient simply to 
        // add all of the power samples together, and note that their sum is actually 
        // CYCLES_PER_SECOND greater than it would otherwise be.
        //   Although the numerical value itself does not change, I thought that a new name 
        // may be helpful so as to minimise confusion.  
        //   The 'energy' variable below is CYCLES_PER_SECOND * (1/powerCal) times larger than 
        // the actual energy in Joules.
        //
        long realEnergy_grid = realPower_grid; 
        long realEnergy_diverted = realPower_diverted; 
        
   
        // Energy contributions from the grid connection point (CT1) are summed in an 
        // accumulator which is known as the energy bucket.  The purpose of the energy bucket 
        // is to mimic the operation of the supply meter.  The range over which energy can 
        // pass to and fro without loss or charge to the user is known as its 'sweet-zone'.
        // The capacity of the energy bucket is set to this same value within setup().
        //    
        // The latest contribution can now be added to this energy bucket
        energyInBucket_long += realEnergy_grid;   
         
        // Apply max and min limits to bucket's level.  This is to ensure correct operation
        // when conditions change, i.e. when import changes to export, and vici versa.
        //
        if (energyInBucket_long > capacityOfEnergyBucket_long) { 
          energyInBucket_long = capacityOfEnergyBucket_long; } 
        else         
        if (energyInBucket_long < 0) {
          energyInBucket_long = 0; }  
  
        if (EDD_isActive) // Energy Diversion Display
        {
          // For diverted energy, the latest contribution needs to be added to an 
          // accumulator which operates with maximum precision.
          
          if (realEnergy_diverted < antiCreepLimit_inIEUperMainsCycle)
          {
            realEnergy_diverted = 0;
          }  

          divertedEnergyRecent_IEU += realEnergy_diverted;
      
          // Whole kWhours are then recorded separately
          if (divertedEnergyRecent_IEU > IEU_per_Wh)
          {
            divertedEnergyRecent_IEU -= IEU_per_Wh;
            divertedEnergyTotal_Wh++;
          }  
        }
        
        if(timerForDisplayUpdate > UPDATE_PERIOD_FOR_DISPLAYED_DATA) 
        { // the 4-digit display needs to be refreshed every few mS. For convenience,
          // this action is performed every N times around this processing loop.
          timerForDisplayUpdate = 0;
          
/*
// Need to comment this section out if WORKLOAD_CHECK is enabled
          Serial.print("Diverted: " );
          Serial.print(divertedEnergyTotal_Wh);
          Serial.print(" Wh plus ");
          Serial.print((powerCal_diverted / CYCLES_PER_SECOND) * divertedEnergyRecent_IEU);

          Serial.print(" J , EDD is" );
*/

          // After a pre-defined period of inactivity, the 4-digit display needs to 
          // close down in readiness for the next's day's data. 
          //
          if (absenceOfDivertedEnergyCount > displayShutdown_inMainsCycles)
          {
            // clear the accumulators for diverted energy
            divertedEnergyTotal_Wh = 0;
            divertedEnergyRecent_IEU = 0;
            EDD_isActive = false; // energy diversion detector is now inactive
          }

/*          
// Need to comment this section out if WORKLOAD_CHECK is enabled
          if (EDD_isActive) {
            Serial.println(" on" ); }
          else {
            Serial.println(" off" ); }        
*/       
          configureValueForDisplay();
        }
        else
        {
          timerForDisplayUpdate++;
        }

        // clear the per-cycle accumulators for use in this new mains cycle.  
        samplesDuringThisCycle = 0;
        sumP_grid = 0;
        sumP_diverted = 0;

      } // end of processing that is specific to the first Vsample in each +ve half cycle 
  
      // still processing samples where the voltage is POSITIVE ...
      if (triggerNeedsToBeArmed == true)
      {
        // check to see whether the trigger device can now be reliably armed
        if (samplesDuringThisCycle == 3) // much easier than checking the voltage level
        {
          if (energyInBucket_long < lowerEnergyThreshold_long) {
            // when below the lower threshold, always set the triac to "off" 
            nextStateOfTriac = TRIAC_OFF; }
          else
          if (energyInBucket_long > upperEnergyThreshold_long) {
            // when above the upper threshold, always set the triac to "off"
            nextStateOfTriac = TRIAC_ON; }
          else {
            // otherwise, leave the triac's state unchanged (hysteresis)
               }          
                  
          // set the Arduino's output pin accordingly, and clear the flag
          digitalWrite(outputForTrigger, nextStateOfTriac);   
          triggerNeedsToBeArmed = false;  
      
          // update the Energy Diversion Detector
          if (nextStateOfTriac == TRIAC_ON) {
            absenceOfDivertedEnergyCount = 0; 
            EDD_isActive = true; }            
          else {
            absenceOfDivertedEnergyCount++; }   
        }
      }   
    }
    else
    {  
      // wait until the DC-blocking filters have had time to settle
      if(millis() > startUpPeriod * 1000) 
      {
        beyondStartUpPhase = true;
        sumP_grid = 0;
        sumP_diverted = 0;
        samplesDuringThisCycle = 0;
        Serial.println ("Go!");
      }
    }
    
  } // end of processing that is specific to samples where the voltage is positive
  
  else // the polatity of this sample is negative
  {     
    if (polarityOfLastSampleV != NEGATIVE)
    {
      // This is the start of a new -ve half cycle (just after the zero-crossing point)      
      // which is a convenient point to update the Low Pass Filter for DC-offset removal
      //
      long previousOffset = DCoffset_V_long;
      DCoffset_V_long = previousOffset + (cumVdeltasThisCycle_long>>6); // faster than * 0.01
      cumVdeltasThisCycle_long = 0;
      
      // To ensure that the LPF will always start up correctly when 240V AC is available, its
      // output value needs to be prevented from drifting beyond the likely range of the 
      // voltage signal.  This avoids the need to use a HPF as was done for initial Mk2 builds.
      //
      if (DCoffset_V_long < DCoffset_V_min) {
        DCoffset_V_long = DCoffset_V_min; }
      else  
      if (DCoffset_V_long > DCoffset_V_max) {
        DCoffset_V_long = DCoffset_V_max; }
        
      checkOutputModeSelection(); // updates outputMode if switch is changed
           
    } // end of processing that is specific to the first Vsample in each -ve half cycle
  } // end of processing that is specific to samples where the voltage is positive
  
  // processing for EVERY pair of samples
  //
  // First, deal with the power at the grid connection point (as measured via CT1)
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_grid = ((long)(sampleI_grid-DCoffset_I))<<8;
   
  // phase-shift the voltage waveform so that it aligns with the grid current waveform
//  long  phaseShiftedSampleVminusDC_grid = lastSampleVminusDC_long
//         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_grid_int)>>8);  
  long  phaseShiftedSampleVminusDC_grid = sampleVminusDC_long; // <- simple version for when
                                                               // phaseCal is not in use
                                                               
  // calculate the "real power" in this sample pair and add to the accumulated sum
  long filtV_div4 = phaseShiftedSampleVminusDC_grid>>2;  // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4 = sampleIminusDC_grid>>2; // reduce to 16-bits (now x64, or 2^6)
  long instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
  instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_grid +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  // Now deal with the diverted power (as measured via CT2)
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_diverted = ((long)(sampleI_diverted-DCoffset_I))<<8;

  // phase-shift the voltage waveform so that it aligns with the diverted current waveform
//  long phaseShiftedSampleVminusDC_diverted = lastSampleVminusDC_long
//         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_diverted_int)>>8);  
  long phaseShiftedSampleVminusDC_diverted = sampleVminusDC_long; // <- simple version for when
                                                                  // phaseCal is not in use
                                                               
  // calculate the "real power" in this sample pair and add to the accumulated sum
  filtV_div4 = phaseShiftedSampleVminusDC_diverted>>2;  // reduce to 16-bits (now x64, or 2^6)
  filtI_div4 = sampleIminusDC_diverted>>2; // reduce to 16-bits (now x64, or 2^6)
  instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
  instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_diverted +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  samplesDuringThisCycle++;
  
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
  polarityOfLastSampleV = polarityNow;  // for identification of half cycle boundaries

  refreshDisplay();
}
//  ----- end of main Mk2i code -----


// this function changes the value of outputMode if the state of the external switch is altered 
void checkOutputModeSelection()  
{
  static byte count = 0;
  int pinState = digitalRead(outputModeSelectorPin);
  if (pinState != outputMode)
  {
    count++;
  }  
  if (count >= 20)
  {
    count = 0;
    outputMode = (enum outputModes)pinState;  // change the global variable
    Serial.print ("outputMode selection changed to ");
    if (outputMode == NORMAL) {
      Serial.println ( "normal"); }
    else {  
      Serial.println ( "anti-flicker"); }
    
    configureParamsForSelectedOutputMode();
  }
}


void configureParamsForSelectedOutputMode()
{
  if (outputMode == ANTI_FLICKER)
  {
    // settings for anti-flicker mode
    lowerEnergyThreshold_long = 
       capacityOfEnergyBucket_long * (0.5 - offsetOfEnergyThresholdsInAFmode); 
    upperEnergyThreshold_long = 
       capacityOfEnergyBucket_long * (0.5 + offsetOfEnergyThresholdsInAFmode);   
  }
  else
  { 
    // settings for normal mode
    lowerEnergyThreshold_long = capacityOfEnergyBucket_long * 0.5; 
    upperEnergyThreshold_long = capacityOfEnergyBucket_long * 0.5;   
  }
  
  // display relevant settings for selected output mode
  Serial.print("  capacityOfEnergyBucket_long = ");
  Serial.println(capacityOfEnergyBucket_long);
  Serial.print("  lowerEnergyThreshold_long   = ");
  Serial.println(lowerEnergyThreshold_long);
  Serial.print("  upperEnergyThreshold_long   = ");
  Serial.println(upperEnergyThreshold_long);
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on
}

// called infrequently, to update the characters to be displayed
void configureValueForDisplay()
{
  static byte locationOfDot = 0;
  
//  Serial.println(divertedEnergyTotal_Wh);
  
  if (EDD_isActive)
  {
    unsigned int val = divertedEnergyTotal_Wh;
    boolean energyValueExceeds10kWh;

    if (val < 10000) {
      // no need to re-scale (display to 3 DPs)
      energyValueExceeds10kWh = false; }
    else {
      // re-scale is needed (display to 2 DPs)
      energyValueExceeds10kWh = true;
      val = val/10; }
      
    byte thisDigit = val / 1000;
    charsForDisplay[0] = thisDigit;     
    val -= 1000 * thisDigit;
        
    thisDigit = val / 100;
    charsForDisplay[1] = thisDigit;        
    val -= 100 * thisDigit;
        
    thisDigit = val / 10;
    charsForDisplay[2] = thisDigit;        
    val -= 10 * thisDigit;
        
    charsForDisplay[3] = val; 
  
    // assign the decimal point location
    if (energyValueExceeds10kWh) {
      charsForDisplay[1] += 10; } // dec point after 2nd digit
    else {
      charsForDisplay[0] += 10; } // dec point after 1st digit
  }
  else
  {
    // "walking dots" display
    charsForDisplay[locationOfDot] = 20; // blank
    
    locationOfDot++;
    if (locationOfDot >= noOfDigitLocations) {
     locationOfDot = 0; }
     
    charsForDisplay[locationOfDot] = 21; // dot
  }
/*  
  Serial.print(charsForDisplay[0]);
  Serial.print("  "); 
  Serial.print(charsForDisplay[1]);
  Serial.print("  "); 
  Serial.print(charsForDisplay[2]);
  Serial.print("  "); 
  Serial.print(charsForDisplay[3]);
  Serial.println(); 
*/  
//  valueToBeDisplayed++;
}

void refreshDisplay()
{
  // This routine keeps track of which digit is being displayed and checks when its 
  // display time has expired.  It then makes the necessary adjustments for displaying
  // the next digit.
  //   The two versions of the hardware require different logic.
 
#ifdef PIN_SAVING_HARDWARE
  // With this version of the hardware, care must be taken that all transitory states
  // are masked out.  Note that the enableDisableLine only masks the seven primary
  // segments, not the Decimal Point line which must therefore be treated separately.
  // The sequence is:
  // 
  // 1. set the decimal point line to 'off'
  // 2. disable the 7-segment driver chip 
  // 3. determine the next location which is to be active
  // 4. set up the location lines for the new active location
  // 5. determine the relevant character for the new active location
  // 6. configure the driver chip for the new character to be displayed
  // 7. set up decimal point line for the new active location
  // 8. enable the 7-segment driver chip 
  
  static byte displayTime_count = 0;
  static byte digitLocationThatIsActive = 0;
  
  displayTime_count++;
  
  if (displayTime_count > MAX_DISPLAY_TIME_COUNT)
  {
    byte lineState;
    
    displayTime_count = 0;   

    // 1. disable the Decimal Point driver line;
      digitalWrite( decimalPointLine, LOW); 
        
    // 2. disable the driver chip while changes are taking place
    digitalWrite( enableDisableLine, DRIVER_CHIP_DISABLED);  

    // 3. determine the next digit location to be active
    digitLocationThatIsActive++;
    if (digitLocationThatIsActive >= noOfDigitLocations) {
      digitLocationThatIsActive = 0; }
           
    // 4. set up the digit location drivers for the new active location
    for (byte line = 0; line < noOfDigitLocationLines; line++) {
      lineState = digitLocationMap[digitLocationThatIsActive][line];
      digitalWrite( digitLocationLine[line], lineState); }
        
    // 5. determine the character to be displayed at this new location
    // (which includes the decimal point information)
    byte digitVal = charsForDisplay[digitLocationThatIsActive];
    
    // 6. configure the 7-segment driver for the character to be displayed 
    for (byte line = 0; line < noOfDigitSelectionLines; line++) { 
      lineState = digitValueMap[digitVal][line];
      digitalWrite( digitSelectionLine[line], lineState); }
      
    // 7. set up the Decimal Point driver line;
      digitalWrite( decimalPointLine, digitValueMap[digitVal][DPstatus_columnID]); 
      
    // 8. enable the 7-segment driver chip   
    digitalWrite( enableDisableLine, DRIVER_CHIP_ENABLED);  
  }

#else // PIN_SAVING_HARDWARE

  // This version is more straightforward because the digit-enable lines can be
  // used to mask out all of the transitory states, including the Decimal Point.
  // The sequence is:
  // 
  // 1. de-activate the digit-enable line that was previously active
  // 2. determine the next location which is to be active
  // 3. determine the relevant character for the new active location
  // 4. set up the segment drivers for the character to be displayed (includes the DP)
  // 5. activate the digit-enable line for the new active location 
 
  static byte displayTime_count = 0;
  static byte digitLocationThatIsActive = 0;
  
  displayTime_count++;
  
  if (displayTime_count > MAX_DISPLAY_TIME_COUNT)
  { 
    displayTime_count = 0;   
    
    // 1. de-activate the location which is currently being displayed
    digitalWrite(digitSelectorPin[digitLocationThatIsActive], DIGIT_DISABLED);
   
    // 2. determine the next digit location which is to be displayed
    digitLocationThatIsActive++;
    if (digitLocationThatIsActive >= noOfDigitLocations) {
      digitLocationThatIsActive = 0; }
           
    // 3. determine the relevant character for the new active location
    byte digitVal = charsForDisplay[digitLocationThatIsActive];
    
    // 4. set up the segment drivers for the character to be displayed (includes the DP)
    for (byte segment = 0; segment < noOfSegmentsPerDigit; segment++) { 
      byte segmentState = segMap[digitVal][segment];
      digitalWrite( segmentDrivePin[segment], segmentState); }

  // 5. activate the digit-enable line for the new active location 
    digitalWrite(digitSelectorPin[digitLocationThatIsActive], DIGIT_ENABLED);   
  } 
#endif // PIN_SAVING_HARDWARE

} // end of refreshDisplay()

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}




