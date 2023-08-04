/* Mk2_multiLoad_wired_6a.ino                    <--- suitable for use (with fast switching of loads)
 *   is based on Mk2_multiLoad_wired_6.ino         <--- suitable for use (with fast switching of loads)
 *     is based on Mk2_multiLoad_wired_5a.ino         <--- not to be used
 *       which is based on Mk2_multiLoad_wired_5.ino    <---  not to be used
 *         which is based on Mk2_multiLoad_CAT5_4.ino     <--- suitable for use (with slower switching of loads)
 *
 * This sketch is for diverting surplus PV power using multiple hard-wired loads.  
 * An external switch allows either load 0 or Load 1 to have the highest 
 * priority.  Any number of loads can be supported by the logic, a dedicated 
 * IO pin being required for each one. 
 *
 * This sketch is intended for use with my PCB-based hardware for the Mk2 PV Router.  
 * The selector switch, as mentioned above, connects to the "mode" port which has been 
 * re-assigned for priority selection. "Normal" mode can be achieved by setting the 
 * anti-flicker offset prameter to zero at compile-time.
 *
 * The integral voltage sensor is fed from one of the secondary coils of the transformer. 
 * Current is measured via Current Transformers at the CT1 and CT2 ports.  
 * CT1 is for 'grid' current, to be measured at the grid supply point.
 * CT2 is for the 'diverted' current, so that energy which is diverted via the primary
 *  dump-load can be recorded and displayed locally.
 *
 * A persistence-based 4-digit display is supported. To free up the necessary IO pins
 * for driving multiple loads, the pin-saving hardware needs to be in place.  
 * These extra logic chips (ICs 3 and 4) reduce the number of IO pins 
 * that are needed to drive the display.  The freed-up pins are available at the 
 * J1-5 connector.  The uppermost position has been assigned to drive Load 1,
 * the next one down is for Load 2, and the lowest one is for Load 5.  The control signal
 * for Load 0 is available at the "trigger" connector.  
 *
 * With the green (rev 2.1) version of my PCB, each of the additional outputs has an 
 * associated ground pin.  It is therefore more sensible for those outputs to be active-high
 * rather than active-low.  With a 5V regulator rather than the normal 3.3V one, these 
 * outputs are able to drive an SSR directly. energymonitor.org/emon/node/1757
 *
 * September 2014: renamed as Mk2_multiLoad_CAT5_3, with these changes:
 * - reimplementation of cycleCount, as it could have overflowed with unpredictable results;
 * - the functions increaseLoadIfPossible() and decreaseLoadIfPossible() have been tidied;
 * - energyThreshold_long has been renamed as midPointOfEnergyBucket_long.
 *
 * December 2014: renamed as Mk2_multiLoad_CAT5_4, with these changes:
 * - persistence check added for zero-crossing detection (polarityConfirmed);
 * - lowestNoOfSampleSetsPerMainsCycle added, to check for any disturbances;
 * - the logic for each of the 5 additional loads has been inverted by use of the '!' character.
 *     these outputs are now active-high rather than active-low
 *
 * November 2015: renamed as Mk2_multiLoad_wired_5, with these changes:
 * - the original twin-threshold algorithm for energy state management has been reinstated;
 * - improved mechanism for controlling multiple loads (faster and more accurate);
 * - the phaseCal mechanism has been reinstated;
 * - SWEETZONE_IN_JOULES has been replaced by WORKING_RANGE_IN_JOULES.
 *
 * January 2015: renamed as Mk2_multiLoad_wired_5a, with this change:
 * - minor bug-fix in allGeneralProcessing() which affects how the energy thresholds are adjusted immediately
 *     after a change of load-state has tqaken place. 
 *
 * January 2015: renamed as Mk2_multiLoad_wired_6, with this change:
 * - reinstatement of min & max limits for the energy bucket's level.  This section was lost during the 
 *   conversion from version 4 to version 5.  The absence of this section prevents diversion from starting 
 *   in the correct manner.  Versions 5 and 5a should therefore not be used.  Version 6 is believed to be
 *   a correct implementation of the improved mechanism for controlling multiple loads.  
 *
 * January 2016: renamed as Mk2_multiLoad_wired_6a, with a minor change in the ISR to 
 *   remove a timing uncertainty.
 *
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
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
#define WORKING_RANGE_IN_JOULES 3600 
#define REQUIRED_EXPORT_IN_WATTS 0 // when set to a negative value, this acts as a PV generator 

// to prevent the diverted energy total from 'creeping'
#define ANTI_CREEP_LIMIT 5 // in Joules per mains cycle (has no effect when set to 0)
long antiCreepLimit_inIEUperMainsCycle;

const byte noOfDumploads = 6; // The logic expects a minimum of 2 dumploads, 
                              // for local & remote loads, but neither has to
                              // be physically present. 

// definitions of enumerated types and instances of same
enum polarities {NEGATIVE, POSITIVE};
enum outputModes {ANTI_FLICKER, NORMAL};
enum loadPriorityModes {LOAD_1_HAS_PRIORITY, LOAD_0_HAS_PRIORITY};

enum loadStates {LOAD_ON, LOAD_OFF}; // all loads are active low
enum loadStates logicalLoadState[noOfDumploads]; 
enum loadStates physicalLoadState[noOfDumploads]; 

// ----------------  Extra Features selection ----------------------
//
// - WORKLOAD_CHECK, for determining how much spare processing time there is. 
//  
// #define WORKLOAD_CHECK  // <-- Include this line is this feature is required 


// For most single-load Mk2 systems, the power-diversion logic can operate in either of two modes:
//
// - NORMAL, where the triac switches rapidly on/off to maintain a constant energy level.
// - ANTI_FLICKER, whereby the repetition rate is reduced to avoid rapid fluctuations
//    of the local mains voltage.
//
// For this multi-load version, the same mechanism has been retained but the 
// output mode is hard-coded as below:
enum outputModes outputMode = ANTI_FLICKER;    

// In this multi-load version, the external switch is re-used to determine the load priority
enum loadPriorityModes loadPriorityMode = LOAD_0_HAS_PRIORITY;                                                   

// allocation of digital pins when pin-saving hardware is in use
// *************************************************************
// D0 & D1 are reserved for the Serial i/f
const byte physicalLoad_2_pin = 2;  // <-- to control an additional load  
const byte loadPrioritySelectorPin = 3; // <-- this is the "mode" port  
const byte physicalLoad_0_pin = 4;  // <-- this is the "trigger" port  
// D5 is the enable line for the 7-segment display driver, IC3 
// D6 is a data input line for the 7-segment display driver, IC3
// D7 is a data input line for the 7-segment display driver, IC3
// D8 is a data input line for the 7-segment display driver, IC3
// D9 is a data input line for the 7-segment display driver, IC3
const byte physicalLoad_3_pin = 10;  // <-- to control an additional load  
const byte physicalLoad_5_pin = 11;  // <-- to control an additional load  
const byte physicalLoad_1_pin = 12;  // <-- to control an additional load  
const byte physicalLoad_4_pin = 13;  // <-- to control an additional load  

// allocation of analogue pins
// ***************************
// A0 (D14) is the decimal point driver line for the 4-digit display 
// A1 (D15) is a digit selection line for the 4-digit display, via IC4
// A2 (D16) is a digit selection line for the 4-digit display, via IC4
const byte voltageSensor = 3;          // A3 is for the voltage sensor
const byte currentSensor_diverted = 4; // A4 is for CT2 which measures diverted current
const byte currentSensor_grid = 5;     // A5 is for CT1 which measures grid current


const byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;    // nominal mid-point value of ADC @ x1 scale

// General global variables that are used in multiple blocks so cannot be static.
// For integer maths, many variables need to be 'long'
//
long energyInBucket_long = 0;          // in Integer Energy Units
long capacityOfEnergyBucket_long;  // depends on powerCal, frequency & the 'sweetzone' size.
long midPointOfEnergyBucket_long;         // used for 'normal' and single-threshold 'AF' logic
int phaseCal_grid_int;             // to avoid the need for floating-point maths
int phaseCal_diverted_int;         // to avoid the need for floating-point maths
long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF
long divertedEnergyRecent_IEU = 0; // Hi-res accumulator of limited range
unsigned int divertedEnergyTotal_Wh = 0; // WattHour register of 63K range
long IEU_per_Wh; // depends on powerCal, frequency & the 'sweetzone' size.

unsigned long displayShutdown_inMainsCycles; 
unsigned long absenceOfDivertedEnergyCount = 0;

long lowerThreshold_default;
long lowerEnergyThreshold;
long upperThreshold_default;
long upperEnergyThreshold;

boolean recentTransition;
byte postTransitionCount;
#define POST_TRANSITION_MAX_COUNT 3 // <-- allows each transition to take effect
byte activeLoad = 0;

float offsetOfEnergyThresholdsInAFmode = 0.1; // <-- must not exceeed 0.4

// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
int sampleI_grid;
int sampleI_diverted;
int sampleV;

// For an enhanced polarity detection mechanism, which includes a persistence check
#define POLARITY_CHECK_MAXCOUNT 2 // sample sets
enum polarities polarityOfMostRecentVsample;   
enum polarities polarityConfirmed;  
enum polarities polarityConfirmedOfLastSampleV;  

// For a mechanism to check the continuity of the sampling sequence
#define CONTINUITY_CHECK_MAXCOUNT 250 // mains cycles
int sampleCount_forContinuityChecker;
int sampleSetsDuringThisMainsCycle;    
int lowestNoOfSampleSetsPerMainsCycle;


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
const float powerCal_diverted = 0.044;  // for CT2
 
                        
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
const float  phaseCal_grid = 1.0; // default value
const float  phaseCal_diverted = 1.0; // default value

// Various settings for the 4-digit display, which needs to be refreshed every few mS
const byte noOfDigitLocations = 4;
const byte noOfPossibleCharacters = 22;
#define MAX_DISPLAY_TIME_COUNT 10// no of processing loops between display updates
#define DISPLAY_SHUTDOWN_IN_HOURS 8 // auto-reset after this period of inactivity
// #define DISPLAY_SHUTDOWN_IN_HOURS 0.01 // for testing that the display clears after 36 seconds


#define DRIVER_CHIP_DISABLED HIGH
#define DRIVER_CHIP_ENABLED LOW

// the primary segments are controlled by a pair of logic chips
const byte noOfDigitSelectionLines = 4; // <- for the 74HC4543 7-segment display driver
const byte noOfDigitLocationLines = 2; // <- for the 74HC138 2->4 line demultiplexer

byte enableDisableLine = 5; // <- affects the primary 7 segments only (not the DP)
byte decimalPointLine = 14; // <- this line has to be individually controlled. 

byte digitLocationLine[noOfDigitLocationLines] = {16,15};
byte digitSelectionLine[noOfDigitSelectionLines] = {7,9,8,6};

// The final column of digitValueMap[] is for the decimal point status. 
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


byte charsForDisplay[noOfDigitLocations] = {20,20,20,20}; // all blank 

boolean EDD_isActive = false; // Energy Diversion Detection (for the local load only)
long requiredExportPerMainsCycle_inIEU;
float IEUtoJoulesConversion_CT1;

void setup()
{  
  pinMode(physicalLoad_0_pin, OUTPUT); // driver pin for the local dump-load
  pinMode(physicalLoad_1_pin, OUTPUT); // driver pin for an additional load
  pinMode(physicalLoad_2_pin, OUTPUT); // driver pin for an additional load
  pinMode(physicalLoad_3_pin, OUTPUT); // driver pin for an additional load
  pinMode(physicalLoad_4_pin, OUTPUT); // driver pin for an additional load
  pinMode(physicalLoad_5_pin, OUTPUT); // driver pin for an additional load

  for(int i = 0; i< noOfDumploads; i++)
  {
    logicalLoadState[i] = LOAD_OFF;
    physicalLoadState[i] = LOAD_OFF;
  } 
  
  digitalWrite(physicalLoad_0_pin, physicalLoadState[0]); // the local load is active low.      
  digitalWrite(physicalLoad_1_pin, !physicalLoadState[1]); // additional loads are active high.      
  digitalWrite(physicalLoad_2_pin, !physicalLoadState[2]); // additional loads are active high      
  digitalWrite(physicalLoad_3_pin, !physicalLoadState[3]); // additional loads are active high      
  digitalWrite(physicalLoad_4_pin, !physicalLoadState[4]); // additional loads are active high      
  digitalWrite(physicalLoad_5_pin, !physicalLoadState[5]); // additional loads are active high      

  pinMode(loadPrioritySelectorPin, INPUT); // this pin is tracked to the "mode" connector
  digitalWrite(loadPrioritySelectorPin, HIGH); // enable the internal pullup resistor
  delay (100); // allow time to settle
  int pinState = digitalRead(loadPrioritySelectorPin);  // initial selection and
  loadPriorityMode = (enum loadPriorityModes)pinState;  //   assignment of priority 
 
  delay(5000); // allow time to open Serial monitor     
 
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      Mk2_multiLoad_wired_6a.ino");
  Serial.println();
       
  // configure the IO drivers for the 4-digit display   
  //
  // the Decimal Point line is driven directly from the processor
  pinMode(decimalPointLine, OUTPUT); // the 'decimal point' line 
  
  // set up the control lines for the 74HC4543 7-seg display driver
  for (int i = 0; i < noOfDigitSelectionLines; i++) {
    pinMode(digitSelectionLine[i], OUTPUT); }

  // an enable line is required for the 74HC4543 7-seg display driver
  pinMode(enableDisableLine, OUTPUT); // for the 74HC4543 7-seg display driver
  digitalWrite( enableDisableLine, DRIVER_CHIP_DISABLED);  
  
  // set up the control lines for the 74HC138 2->4 demux
  for (int i = 0; i < noOfDigitLocationLines; i++) {
    pinMode(digitLocationLine[i], OUTPUT); } 
  
       
  // When using integer maths, calibration values that are supplied in floating point 
  // form need to be rescaled.  
  //
  phaseCal_grid_int = phaseCal_grid * 256; // for integer maths 
  phaseCal_diverted_int = phaseCal_diverted * 256; // for integer maths 
  
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
     (long)WORKING_RANGE_IN_JOULES * CYCLES_PER_SECOND * (1/powerCal_grid);
//  energyInBucket_long = capacityOfEnergyBucket_long * 0.45; // for rapid start up
  midPointOfEnergyBucket_long = capacityOfEnergyBucket_long / 2;
  
  // For recording the accumulated amount of diverted energy data (using CT2), a similar 
  // calibration mechanism is required.  Rather than a bucket with a fixed capacity, the 
  // accumulator for diverted energy just needs to be scaled correctly.  As soon as its 
  // value exceeds 1 Wh, an associated WattHour register is incremented, and the 
  // accumulator's value is decremented accordingly.  The calculation below is to determine
  // the correct scaling for this accumulator.
  
  IEU_per_Wh = 
     (long)JOULES_PER_WATT_HOUR * CYCLES_PER_SECOND * (1/powerCal_diverted); 

  IEUtoJoulesConversion_CT1 = powerCal_grid / CYCLES_PER_SECOND;
 
  // to avoid the diverted energy accumulator 'creeping' when the load is not active
  antiCreepLimit_inIEUperMainsCycle = (float)ANTI_CREEP_LIMIT * (1/powerCal_grid);

  long mainsCyclesPerHour = (long)CYCLES_PER_SECOND * SECONDS_PER_MINUTE * MINUTES_PER_HOUR;
                             
  displayShutdown_inMainsCycles = DISPLAY_SHUTDOWN_IN_HOURS * mainsCyclesPerHour;                           
  
  requiredExportPerMainsCycle_inIEU = (long)REQUIRED_EXPORT_IN_WATTS * (1/powerCal_grid); 
      
  
      
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

  Serial.print ( "Priority load:  ");
  if (loadPriorityMode == LOAD_0_HAS_PRIORITY) {
    Serial.println ( "load 0"); }
  else 
  {  
    Serial.println ( "load 1");
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
  Serial.print ("Export rate (Watts) = ");
  Serial.println (REQUIRED_EXPORT_IN_WATTS);
  
  Serial.print ("zero-crossing persistence (sample sets) = ");
  Serial.println (POLARITY_CHECK_MAXCOUNT);
  Serial.print ("continuity sampling display rate (mains cycles) = ");
  Serial.println (CONTINUITY_CHECK_MAXCOUNT);  
  

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
// perform V and I measurements in sequence.  A "data ready"flag is set after each 
// set of conversions has been completed.  This flag is cleared  within loop().
//   This Interrupt Service Routine is for use when the ADC is fixed timer mode.  It is 
// executed whenever the ADC timer expires.  In this mode, the next ADC conversion is 
// initiated from within this ISR.  
//
void timerIsr(void)
{                                         
  static unsigned char sample_index = 0;
  static int  sampleI_grid_raw;
  static int sampleI_diverted_raw;
  
  switch(sample_index)
  {
    case 0:
      sampleV = ADC;                    // store the ADC value (this one is for Voltage)
      ADMUX = 0x40 + currentSensor_diverted;  // set up the next conversion, which is for Diverted Current
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index++;                   // increment the control flag
      sampleI_diverted = sampleI_diverted_raw;
      sampleI_grid = sampleI_grid_raw;
      dataReady = true;                 // all three ADC values can now be processed
      break;
    case 1:
      sampleI_diverted_raw = ADC;               // store the ADC value (this one is for Diverted Current)
      ADMUX = 0x40 + currentSensor_grid;  // set up the next conversion, which is for Grid Current
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index++;                   // increment the control flag
      break;
    case 2:
      sampleI_grid_raw = ADC;               // store the ADC value (this one is for Grid Current)
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
//   If the next set of samples becomes available before the processing of the 
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
  static boolean beyondStartUpPhase = false;     // start-up delay, allows things to settle
  static long sumP_grid;                              // for per-cycle summation of 'real power' 
  static long sumP_diverted;                              // for per-cycle summation of 'real power' 
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static long lastSampleVminusDC_long;     //    for the phaseCal algorithm
  static byte perSecondCounter = 0;
   
  // remove DC offset from the raw voltage sample by subtracting the accurate value 
  // as determined by a LP filter.
  long sampleVminusDC_long = ((long)sampleV<<8) - DCoffset_V_long; 

  // determine the polarity of the latest voltage sample
  if(sampleVminusDC_long > 0) { 
    polarityOfMostRecentVsample = POSITIVE; }
  else { 
    polarityOfMostRecentVsample = NEGATIVE; }
    confirmPolarity();

  if (polarityConfirmed == POSITIVE) 
  { 
    if (beyondStartUpPhase)
    {     
      if (polarityConfirmedOfLastSampleV != POSITIVE)
      {
        // This is the start of a new +ve half cycle (just after the zero-crossing point)
        // cycleCount++;  <- this mechanism is unsafe, it will eventually overflow
        
        // a simple routine for checking the continuity of the sampling scheme     
        if (sampleSetsDuringThisMainsCycle < lowestNoOfSampleSetsPerMainsCycle) {
          lowestNoOfSampleSetsPerMainsCycle = sampleSetsDuringThisMainsCycle; }
        
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
        long realPower_grid = sumP_grid / sampleSetsDuringThisMainsCycle; // proportional to Watts
        long realPower_diverted = sumP_diverted / sampleSetsDuringThisMainsCycle; // proportional to Watts
   
        realPower_grid -= requiredExportPerMainsCycle_inIEU; // <- for non-standard use
        
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
         
        // Applying max and min limits to bucket's level has been  
        // deferred until after energy-related decisions have been taken.       

        if (EDD_isActive) // Energy Diversion Display
        {
          // When locally diverted energy is being monitored, the latest contribution 
          // needs to be added to an accumulator which operates with maximum precision.
          //
          if (realEnergy_diverted < antiCreepLimit_inIEUperMainsCycle) {
            realEnergy_diverted = 0; } // to avoid 'creep'

          divertedEnergyRecent_IEU += realEnergy_diverted;
      
          // Whole kWhours are then recorded separately
          if (divertedEnergyRecent_IEU > IEU_per_Wh)
          {
            divertedEnergyRecent_IEU -= IEU_per_Wh;
            divertedEnergyTotal_Wh++;
          }  
        }
        
        perSecondCounter++;
        if(perSecondCounter >= CYCLES_PER_SECOND) 
        { 
          perSecondCounter = 0;          

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

          configureValueForDisplay(); // occurs every second
/*          
          Serial.print (energyInBucket_long * IEUtoJoulesConversion_CT1);
          Serial.print (",  ");
          for (byte loadID = 0; loadID < noOfDumploads; loadID++)
          {
            Serial.print (!physicalLoadState[loadID]); // 1 = "on", 0 = "off"
            Serial.print ("  ");
          }
          Serial.println();
*/          
//          Serial.println(activeLoad);
        }
        
        // continuity checker
        sampleCount_forContinuityChecker++;
        if (sampleCount_forContinuityChecker >= CONTINUITY_CHECK_MAXCOUNT)
        {
          sampleCount_forContinuityChecker = 0;
          Serial.println(lowestNoOfSampleSetsPerMainsCycle);
          lowestNoOfSampleSetsPerMainsCycle = 999;
        }  

        // clear the per-cycle accumulators for use in this new mains cycle.  
        sampleSetsDuringThisMainsCycle = 0;
        sumP_grid = 0;
        sumP_diverted = 0;

      } // end of processing that is specific to the first Vsample in each +ve half cycle 
  
      // still processing samples where the voltage is POSITIVE ...
      // check to see whether the trigger device can now be reliably armed
      if(sampleSetsDuringThisMainsCycle == 3) // should always exceed 20V (the min for trigger)
      {
        /* Determining whether any of the loads need to be changed is is a 3-stage process:
         * - change the LOGICAL load states as necessary to maintain the energy level
         * - update the PHYSICAL load states according to the logical -> physical mapping 
         * - update the driver lines for each of the loads.
         */
          
        // Restrictions apply for the period immediately after a load has been switched.
        // Here the recentTransition flag is checked and updated as necessary.
        if (recentTransition)
        {
          postTransitionCount++;
          if (postTransitionCount >= POST_TRANSITION_MAX_COUNT)
          {
            recentTransition = false;
          }
        }
  
        if (energyInBucket_long > midPointOfEnergyBucket_long)
        {  
          // the energy state is in the upper half of the working range
          lowerEnergyThreshold = lowerThreshold_default; // reset the "opposite" threshold 
          if (energyInBucket_long > upperEnergyThreshold) 
          {
            // Because the energy level is high, some action may be required
            boolean OK_toAddLoad = true;
            byte tempLoad = nextLogicalLoadToBeAdded();
            if (tempLoad < noOfDumploads)
            {
              // a load which is now OFF has been identified for potentially being switched ON
              if (recentTransition)
              {
                // During the post-transition period, any increase in the energy level is noted. 
                if (energyInBucket_long > upperEnergyThreshold)
                {
                  upperEnergyThreshold = energyInBucket_long; 
                  
                  // the energy thresholds must remain within range
                  if (upperEnergyThreshold > capacityOfEnergyBucket_long)
                  {
                    upperEnergyThreshold = capacityOfEnergyBucket_long;
                  }
                }
              
                // Only the active load may be switched during this period.  All other loads must
               // wait until the recent transition has had sufficient opportunity to take effect. 
                if (tempLoad != activeLoad)
                {
                  OK_toAddLoad = false;
                }
              }
      
              if (OK_toAddLoad)
              {
                logicalLoadState[tempLoad] = LOAD_ON;
                activeLoad = tempLoad;
                postTransitionCount = 0;
                recentTransition = true;
              }
            }
          }
        }
        else
        { // the energy state is in the lower half of the working range
          upperEnergyThreshold = upperThreshold_default; // reset the "opposite" threshold 
          if (energyInBucket_long < lowerEnergyThreshold)
          {
            // Because the energy level is low, some action may be required
            boolean OK_toRemoveLoad = true;
            byte tempLoad = nextLogicalLoadToBeRemoved();
            if (tempLoad < noOfDumploads)
            {
              // a load which is now ON has been identified for potentially being switched OFF
              if (recentTransition)
              {
                // During the post-transition period, any decrease in the energy level is noted. 
                if (energyInBucket_long < lowerEnergyThreshold)
                {
                  lowerEnergyThreshold = energyInBucket_long; 
                  
                  // the energy thresholds must remain within range
                  if (lowerEnergyThreshold < 0)
                  {
                    lowerEnergyThreshold = 0;
                  }
                }

                // Only the active load may be switched during this period.  All other loads must
                // wait until the recent transition has had sufficient opportunity to take effect. 
                if (tempLoad != activeLoad)
                {  
                  OK_toRemoveLoad = false; 
                }
              }
      
              if (OK_toRemoveLoad)
              {
                logicalLoadState[tempLoad] = LOAD_OFF;
                activeLoad = tempLoad;
                postTransitionCount = 0;
                recentTransition = true;
              }
            }
          }
        }  
          
        updatePhysicalLoadStates(); // allows the logical-to-physical mapping to be changed
               
        // update each of the physical loads
        digitalWrite(physicalLoad_0_pin, physicalLoadState[0]); // active low for trigger     
        digitalWrite(physicalLoad_1_pin, !physicalLoadState[1]); // active high for additional load     
        digitalWrite(physicalLoad_2_pin, !physicalLoadState[2]); // active high for additional load    
        digitalWrite(physicalLoad_3_pin, !physicalLoadState[3]); // active high for additional load    
        digitalWrite(physicalLoad_4_pin, !physicalLoadState[4]); // active high for additional load    
        digitalWrite(physicalLoad_5_pin, !physicalLoadState[5]); // active high for additional load    
        
        // update the Energy Diversion Detector
        if (physicalLoadState[0] == LOAD_ON) {
          absenceOfDivertedEnergyCount = 0; 
          EDD_isActive = true; }            
        else {
          absenceOfDivertedEnergyCount++; }   
          
        // Now that the energy-related decisions have been taken, min and max limits can now
        // be applied  to the level of the energy bucket.  This is to ensure correct operation
        // when conditions change, i.e. when import changes to export, and vici versa.
        //
        if (energyInBucket_long > capacityOfEnergyBucket_long) { 
          energyInBucket_long = capacityOfEnergyBucket_long; } 
        else         
        if (energyInBucket_long < 0) {
          energyInBucket_long = 0; }  
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
        sampleSetsDuringThisMainsCycle = 0;
        sampleCount_forContinuityChecker = 0;
        lowestNoOfSampleSetsPerMainsCycle = 999;
        Serial.println ("Go!");
      }
    }
    
  } // end of processing that is specific to samples where the voltage is positive
  
  else // the polatity of this sample is negative
  {     
    if (polarityConfirmedOfLastSampleV != NEGATIVE)
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
        
//      checkOutputModeSelection(); // updates outputMode if switch is changed
        checkLoadPrioritySelection(); // updates load priorities if switch is changed
           
    } // end of processing that is specific to the first Vsample in each -ve half cycle
  } // end of processing that is specific to samples where the voltage is positive
  
  // processing for EVERY pair of samples
  //
  // First, deal with the power at the grid connection point (as measured via CT1)
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_grid = ((long)(sampleI_grid-DCoffset_I))<<8;
   
  // phase-shift the voltage waveform so that it aligns with the grid current waveform
  long  phaseShiftedSampleVminusDC_grid = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_grid_int)>>8);  
//  long  phaseShiftedSampleVminusDC_grid = sampleVminusDC_long; // <- simple version for when
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
  long phaseShiftedSampleVminusDC_diverted = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_diverted_int)>>8);  
//  long phaseShiftedSampleVminusDC_diverted = sampleVminusDC_long; // <- simple version for when
                                                                  // phaseCal is not in use
                                                               
  // calculate the "real power" in this sample pair and add to the accumulated sum
  filtV_div4 = phaseShiftedSampleVminusDC_diverted>>2;  // reduce to 16-bits (now x64, or 2^6)
  filtI_div4 = sampleIminusDC_diverted>>2; // reduce to 16-bits (now x64, or 2^6)
  instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
  instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_diverted +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  sampleSetsDuringThisMainsCycle++;
 
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
  polarityConfirmedOfLastSampleV = polarityConfirmed;  // for identification of half cycle boundaries

  refreshDisplay();
}


void confirmPolarity()
{
  /* This routine prevents a zero-crossing point from being declared until 
   * a certain number of consecutive samples in the 'other' half of the 
   * waveform have been encountered.  
   */ 
  static byte count = 0;
  if (polarityOfMostRecentVsample != polarityConfirmedOfLastSampleV) { 
    count++; }  
  else {
    count = 0; }
    
  if (count >= POLARITY_CHECK_MAXCOUNT)
  {
    count = 0;
    polarityConfirmed = polarityOfMostRecentVsample;
  }
}


byte nextLogicalLoadToBeAdded()
{ 
  byte retVal = noOfDumploads; 
  boolean success = false;
  for (byte index = 0; index < noOfDumploads && !success; index++)
  {
    if (logicalLoadState[index] == LOAD_OFF) 
    {
      success = true; 
      retVal = index;
    }
  }
  return(retVal);
}


byte nextLogicalLoadToBeRemoved()
{
  byte retVal = noOfDumploads; 
  boolean success = false;
  
  // this index counter can't be a 'byte' because the loop would run forever!
  //
  for (int index = (noOfDumploads -1); index >= 0 && !success; index--)
  {
    if (logicalLoadState[index] == LOAD_ON)
    {
      success = true; 
      retVal = index;
    }
  }
  return(retVal);
}


void updatePhysicalLoadStates()
/*
 * This function provides the link between the logical and physical loads.  The 
 * array, logicalLoadState[], contains the on/off state of all logical loads, with 
 * element 0 being for the one with the highest priority.  The array, 
 * physicalLoadState[], contains the on/off state of all physical loads. 
 * 
 * The association between the physical and logical loads is 1:1.  By default, numerical
 * equivalence is maintained, so logical(N) maps to physical(N).  If physical load 1 is set 
 * to have priority, rather than physical load 0, the logical-to-physical association for 
 * loads 0 and 1 are swapped.
 *
 * Any other mapping relaionships could be configured here.
 */
{
  for (int i = 0; i < noOfDumploads; i++)
  {
    physicalLoadState[i] = logicalLoadState[i]; 
  }
   
  if (loadPriorityMode == LOAD_1_HAS_PRIORITY)
  {
    // swap physical loads 0 & 1 if remote load has priority 
    physicalLoadState[0] = logicalLoadState[1];
    physicalLoadState[1] = logicalLoadState[0];
  } 
}


// this function changes the value of the load priorities if the state of the external switch is altered 
void checkLoadPrioritySelection()  
{
  static byte loadPrioritySwitchCount = 0;
  int pinState = digitalRead(loadPrioritySelectorPin);
  if (pinState != loadPriorityMode)
  {
    loadPrioritySwitchCount++;
  }  
  if (loadPrioritySwitchCount >= 20)
  {
    loadPrioritySwitchCount = 0;
    loadPriorityMode = (enum loadPriorityModes)pinState;  // change the global variable
    Serial.print ("loadPriority selection changed to ");
    if (loadPriorityMode == LOAD_0_HAS_PRIORITY) {
      Serial.println ( "load 0"); }
    else {  
      Serial.println ( "load 1"); }
  }
}


// Although this sketch always operates in ANTI_FLICKER mode, it was convenient
// to leave this mechanism in place.
//
void configureParamsForSelectedOutputMode()
{
  if (outputMode == ANTI_FLICKER)
  {
    // settings for anti-flicker mode
    lowerThreshold_default = 
       capacityOfEnergyBucket_long * (0.5 - offsetOfEnergyThresholdsInAFmode); 
    upperThreshold_default = 
       capacityOfEnergyBucket_long * (0.5 + offsetOfEnergyThresholdsInAFmode);   
  }
  else
  { 
    // settings for normal mode
    lowerThreshold_default = capacityOfEnergyBucket_long * 0.5; 
    upperThreshold_default = capacityOfEnergyBucket_long * 0.5;   
  }
  
  // display relevant settings for selected output mode
  Serial.print("  capacityOfEnergyBucket_long = ");
  Serial.println(capacityOfEnergyBucket_long);
  Serial.print("  lowerEnergyThreshold   = ");
  Serial.println(lowerThreshold_default);
  Serial.print("  upperEnergyThreshold   = ");
  Serial.println(upperThreshold_default);
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on
}


// called infrequently, to update the characters to be displayed
void configureValueForDisplay()
{
  static byte locationOfDot = 0;
  
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
} // end of refreshDisplay()


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}





