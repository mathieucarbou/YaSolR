/* Mk2_fasterControl_twoLoads_5.ino
 *
 *  (initially released as Mk2_bothDisplays_1 in March 2014)
 * This sketch is for diverting suplus PV power to a dump load using a triac or  
 * Solid State Relay. It is based on the Mk2i PV Router code that I have posted in on  
 * the OpenEnergyMonitor forum.  The original version, and other related material, 
 * can be found on my Summary Page at www.openenergymonitor.org/emon/node/1757
 *
 * In this latest version, the pin-allocations have been changed to suit my 
 * PCB-based hardware for the Mk2 PV Router.  The integral voltage sensor is 
 * fed from one of the secondary coils of the transformer.  Current is measured 
 * via Current Transformers at the CT1 and CT1 ports.  
 * 
 * CT1 is for 'grid' current, to be measured at the grid supply point.
 * CT2 is for the load current, so that diverted energy can be recorded
 *
 * A persistence-based 4-digit display is supported. This can be driven in two
 * different ways, one with an extra pair of logic chips, and one without.  The 
 * appropriate version of the sketch must be selected by including or commenting 
 * out the "#define PIN_SAVING_HARDWARE" statement near the top of the code.
 *
 * September 2014: renamed as Mk2_bothDisplays_2, with these changes:
 * - cycleCount removed (was not actually used in this sketch, but could have overflowed);
 * - removal of unhelpful comments in the IO pin section;
 * - tidier initialisation of display logic in setup();
 * - addition of REQUIRED_EXPORT_IN_WATTS logic (useful as a built-in PV simulation facility);
 *
 * December 2014: renamed as Mk2_bothDisplays_3, with these changes:
 * - persistence check added for zero-crossing detection (polarityConfirmed)
 * - lowestNoOfSampleSetsPerMainsCycle added, to check for any disturbances
 *
 * December 2014: renamed as Mk2_bothDisplays_3a, with some typographical errors fixed.
 *
 * January 2016: renamed as Mk2_bothDisplays_3b, with a minor change in the ISR to 
 *   remove a timing uncertainty.
 *
 * January 2016: updated to Mk2_bothDisplays_3c:
 *   The variables to store the ADC results are now declared as "volatile" to remove 
 *   any possibility of incorrect operation due to optimisation by the compiler.
 *
 * February 2016: updated to Mk2_bothDisplays_4, with these changes:
 * - improvements to the start-up logic.  The start of normal operation is now 
 *    synchronised with the start of a new mains cycle.
 * - reduce the amount of feedback in the Low Pass Filter for removing the DC content
 *     from the Vsample stream. This resolves an anomaly which has been present since 
 *     the start of this project.  Although the amount of feedback has previously been 
 *     excessive, this anomaly has had minimal effect on the system's overall behaviour.
 * - removal of the unhelpful "triggerNeedsToBeArmed" mechanism
 * - tidying of the "confirmPolarity" logic to make its behaviour more clear
 * - SWEETZONE_IN_JOULES changed to WORKING_RANGE_IN_JOULES 
 * - change "triac" to "load" wherever appropriate
 *
 * November 2019: updated to Mk2_fasterControl_1 with these changes:
 * - Half way through each mains cycle, a prediction is made of the likely energy level at the
 *   end of the cycle.  That predicted value allows the triac to be switched at the +ve going 
 *   zero-crossing point rather than waiting for a further 10 ms.  These changes allow for 
 *   faster switching of the load.
 * - The range of the energy bucket has been reduced to one tenth of its former value. This
 *   allows the unit's operation to commence more rapidly whenever surplus power is available.
 * - controlMode is no longer selectable, the unit's operation being effectively hard-coded 
 *   as "Normal" rather than Anti-flicker. 
 * - Port D3 now supports an indicator which shows when the level in the energy bucket
 *   reaches either end of its range.  While the unit is actively diverting surplus power,
 *   it is vital that the level in the reduced capacity energy bucket remains within its 
 *   permitted range, hence the addition of this indicator.
 *   
 * February 2020: updated to Mk2_fasterControl_1a with these changes:
 * - removal of some redundant code in the logic for determining the next load state.
 * 
 * March 2021: updated to Mk2_fasterControl_2 with these changes:
 * - extra filtering added to offset the HPF effect of CT1.  This allows the energy state in
 *   10 ms time to be predicted with more confidence.  Specifically, it is no longer necessary 
 *   to include a 30% boost factor after each change of load state.
 * 
 * June 2021: updated to Mk2_fasterControl_3 with these changes:
 * - to reflect the performance of recently manufactured YHDC SCT_013_000 CTs, 
 *   the value of the parameter lpf_gain has been reduced from 12 to 8.
 *   
 * July 2023: updated to Mk2_fasterControl_twoLoads_5 with these changes:
 * - the ability to control two loads has been transferred from the latest version of my
 *   standard multiLoad sketch, Mk2_multiLoad_wired_7a.  The faster control algorithm  
 *   has been retained.  
 *      The previous 2-load "faster control" sketch (version 4) has been archived as its 
 *   behaviour was found to be problematic.
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
#define WORKING_RANGE_IN_JOULES 360 // 0.1 Wh, reduced for faster start-up
#define REQUIRED_EXPORT_IN_WATTS 0 // when set to a negative value, this acts as a PV generator 

// to prevent the diverted energy total from 'creeping'
#define ANTI_CREEP_LIMIT 5 // in Joules per mains cycle (has no effect when set to 0)
long antiCreepLimit_inIEUperMainsCycle;

const byte noOfDumploads = 2; // The logic expects a minimum of 2 dumploads, 
                              // for local & remote loads, but neither has to
                              // be physically present. 

//  The two versions of the hardware require different logic.  The following line should 
//  be included if the additional logic chips are present, or excluded if they are 
//  absent (in which case some wire links need to be fitted)
//
#define PIN_SAVING_HARDWARE 

// definition of enumerated types
enum polarities {NEGATIVE, POSITIVE};
enum loadStates {LOAD_ON, LOAD_OFF}; // all loads are active low
enum loadStates logicalLoadState[noOfDumploads]; 
enum loadStates physicalLoadState[noOfDumploads]; 

// For this go-faster version, the unit's operation will effectively always be "Normal";  
// there is no "Anti-flicker" option. The controlMode variable has been removed.

// allocation of digital pins which are not dependent on the display type that is in use
// *************************************************************************************
const byte physicalLoad_1_pin = 3;  // <-- the "mode" port is active-high 
const byte physicalLoad_0_pin = 4;  // <-- the "trigger" port is active-low 

// allocation of analogue pins which are not dependent on the display type that is in use
// **************************************************************************************
const byte voltageSensor = 3;          // A3 is for the voltage sensor
const byte currentSensor_diverted = 4; // A4 is for CT2 which measures diverted current
const byte currentSensor_grid = 5;     // A5 is for CT1 which measures grid current

const byte delayBeforeSerialStarts = 1;  // in seconds, to allow Serial window to be opened
const byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;    // nominal mid-point value of ADC @ x1 scale

// General global variables that are used in multiple blocks so cannot be static.
// For integer maths, many variables need to be 'long'
//
boolean beyondStartUpPhase = false;     // start-up delay, allows things to settle
long energyInBucket_long;          // in Integer Energy Units
long capacityOfEnergyBucket_long;  // depends on powerCal, frequency & the 'sweetzone' size.
long midPointOfEnergyBucket_long;         // used for 'normal' and single-threshold 'AF' logic

long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF
long divertedEnergyRecent_IEU = 0; // Hi-res accumulator of limited range
unsigned int divertedEnergyTotal_Wh = 0; // WattHour register of 63K range
long IEU_per_Wh; // depends on powerCal, frequency & the 'sweetzone' size.

unsigned long displayShutdown_inMainsCycles; 
unsigned long absenceOfDivertedEnergyCount = 0;
long mainsCyclesPerHour;

long lowerThreshold_default;
long lowerEnergyThreshold;
long upperThreshold_default;
long upperEnergyThreshold;

boolean recentTransition = false;
byte postTransitionCount;
#define POST_TRANSITION_MAX_COUNT 3 // <-- allows each transition to take effect
byte activeLoad = 0;

// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
volatile int sampleI_grid;
volatile int sampleI_diverted;
volatile int sampleV;

// For an enhanced polarity detection mechanism, which includes a persistence check
#define PERSISTENCE_FOR_POLARITY_CHANGE 1 // sample sets
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
// A full explanation of each of these values now follows:
//   
// powerCal is a floating point variable which is used for converting the 
// product of voltage and current samples into Watts.
//
// The correct value of powerCal is dependent on the hardware that is 
// in use.  For best resolution, the hardware should be configured so that the 
// voltage and current waveforms each span most of the ADC's usable range.  For 
// many systems, the maximum power that will need to be measured is around 3kW. 
//
// My sketch "RawSamplesTool_2chan.ino" provides a one-shot visual display of the
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

// for this go-faster sketch, the phaseCal logic has been removed.  If required, it can be
// found in most of the standard Mk2_bothDisplay_n versions  

// Various settings for the 4-digit display, which needs to be refreshed every few mS
const byte noOfDigitLocations = 4;
const byte noOfPossibleCharacters = 22;
#define MAX_DISPLAY_TIME_COUNT 10// no of processing loops between display updates
#define UPDATE_PERIOD_FOR_DISPLAYED_DATA 50 // mains cycles
#define DISPLAY_SHUTDOWN_IN_HOURS 8 // auto-reset after this period of inactivity
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
long requiredExportPerMainsCycle_inIEU;


void setup()
{  
  pinMode(physicalLoad_0_pin, OUTPUT); // driver pin for the local dump-load
  pinMode(physicalLoad_1_pin, OUTPUT); // driver pin for an additional load
  
  for(int i = 0; i< noOfDumploads; i++)
  {
    logicalLoadState[i] = LOAD_OFF;
    physicalLoadState[i] = LOAD_OFF;
  } 
  
  digitalWrite(physicalLoad_0_pin, physicalLoadState[0]); // the local load is active low.      
  digitalWrite(physicalLoad_1_pin, !physicalLoadState[1]); // additional loads are active high.      

  delay(delayBeforeSerialStarts * 1000); // allow time to open Serial monitor      
 
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      Mk2_fasterControl_twoLoads_5.ino");
  Serial.println();
       
#ifdef PIN_SAVING_HARDWARE
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
  midPointOfEnergyBucket_long = capacityOfEnergyBucket_long / 2;
  lowerThreshold_default = capacityOfEnergyBucket_long * 0.5; 
  upperThreshold_default = capacityOfEnergyBucket_long * 0.5;   
  energyInBucket_long = 0;
     
  // For recording the accumulated amount of diverted energy data (using CT2), a similar 
  // calibration mechanism is required.  Rather than a bucket with a fixed capacity, the 
  // accumulator for diverted energy just needs to be scaled correctly.  As soon as its 
  // value exceeds 1 Wh, an associated WattHour register is incremented, and the 
  // accumulator's value is decremented accordingly. The calculation below is to determine
  // the scaling for this accumulator.
  
  IEU_per_Wh = 
     (long)JOULES_PER_WATT_HOUR * CYCLES_PER_SECOND * (1/powerCal_diverted); 
 
  // to avoid the diverted energy accumulator 'creeping' when the load is not active
  antiCreepLimit_inIEUperMainsCycle = (float)ANTI_CREEP_LIMIT * (1/powerCal_grid);

  mainsCyclesPerHour = (long)CYCLES_PER_SECOND * 
                             SECONDS_PER_MINUTE * MINUTES_PER_HOUR;
                             
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

    
  Serial.print ( "powerCal_grid =      "); Serial.println (powerCal_grid,4);
  Serial.print ( "powerCal_diverted = "); Serial.println (powerCal_diverted,4);
  
  Serial.print ("Anti-creep limit (Joules / mains cycle) = ");
  Serial.println (ANTI_CREEP_LIMIT);
  Serial.print ("Export rate (Watts) = ");
  Serial.println (REQUIRED_EXPORT_IN_WATTS);
  
  Serial.print ("zero-crossing persistence (sample sets) = ");
  Serial.println (PERSISTENCE_FOR_POLARITY_CHANGE);
  Serial.print ("continuity sampling display rate (mains cycles) = ");
  Serial.println (CONTINUITY_CHECK_MAXCOUNT);  
  
  Serial.print("  capacityOfEnergyBucket_long = ");
  Serial.println(capacityOfEnergyBucket_long);
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on

  Serial.println ("----");    
}

// An Interrupt Service Routine is now defined in which the ADC is instructed to 
// measure each analogue input in sequence.  A "data ready" flag is set after each 
// voltage conversion has been completed.  
//   For each set of samples, the two samples for current  are taken before the one 
// for voltage.  This is appropriate because each waveform current is generally slightly 
// advanced relative to the waveform for voltage.  The data ready flag is cleared 
// within loop().
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
//   If there is insufficient processing capacity to do all that is required, the 
//  workload rate can be reduced by increasing the duration of ADC_TIMER_PERIOD.
//
void loop()             
{   
  if (dataReady)   // flag is set after every set of ADC conversions
  {
    dataReady = false; // reset the flag
    allGeneralProcessing(); // executed once for each set of V&I samples
  }  
} // end of loop()


// This routine is called to process each set of V & I samples. The main processor and 
// the ADC work autonomously, their operation being only linked via the dataReady flag.  
// As soon as a new set of data is made available by the ADC, the main processor can 
// start to work on it immediately.  
//
void allGeneralProcessing()
{
  static long sumP_grid;                              // for per-cycle summation of 'real power' 
  static long sumP_diverted;                              // for per-cycle summation of 'real power' 
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static byte timerForDisplayUpdate = 0;
  static enum loadStates nextStateOfLoad = LOAD_OFF;  
  static int sampleSetsDuringNegativeHalfOfMainsCycle; // for arming the triac/trigger
  static long energyInBucket_prediction;
  //
  // extra items for an LPF to improve the processing of data samples from CT1
  static long lpf_long;  // new LPF, for ofsetting the behaviour of CT1 as a HPF
  //
  // The next two constants determine the profile of the LPF.
  // They are matched to the physical behaviour of the YHDC SCT-013-000 CT 
  // and the CT1 samples being 375 us apart
  //
  const float lpf_gain = 8;  // <- setting this to 0 disables this extra processing 
  //const float lpf_gain = 0;  // <- setting this to 0 disables this extra processing 
  const float alpha = 0.002; // 

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
    if (polarityConfirmedOfLastSampleV != POSITIVE)
    {
      // This is the start of a new +ve half cycle (just after the zero-crossing point)
      if (beyondStartUpPhase)
      {     
        // a simple routine for checking the performance of this new ISR structure     
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
   
        realPower_grid -= requiredExportPerMainsCycle_inIEU; // <- useful for PV simulation
 
        // Next, the energy content of this power rating needs to be determined.  Energy is 
        // power multiplied by time, so the next step would normally be to multiply the measured
        // value of power by the time over which it was measured.
        //   Instanstaneous power is calculated once every mains cycle. When integer maths is 
        // being used, a repetitive power-to-energy conversion seems an unnecessary workload.  
        // As all sampling periods are of similar duration, it is more efficient to just 
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
          energyInBucket_long = capacityOfEnergyBucket_long;}
        else         
        if (energyInBucket_long < 0) {
          energyInBucket_long = 0;}   

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

          configureValueForDisplay();
//          Serial.println(energyInBucket_prediction);
        }
        else
        {
          timerForDisplayUpdate++;
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
        sampleSetsDuringNegativeHalfOfMainsCycle = 0;
 
      }
      else
      {  
        // wait until the DC-blocking filters have had time to settle
        if(millis() > (delayBeforeSerialStarts + startUpPeriod) * 1000) 
        {
          beyondStartUpPhase = true;
          sumP_grid = 0;
          sumP_diverted = 0;
          sampleSetsDuringThisMainsCycle = 0; // not yet dealt with for this cycle
          sampleCount_forContinuityChecker = 1; // opportunity has been missed for this cycle
          lowestNoOfSampleSetsPerMainsCycle = 999;
          Serial.println ("Go!");
        }
      }
    } // end of processing that is specific to the first Vsample in each +ve half cycle 
  
    // still processing samples where the voltage is POSITIVE ...    
    // (in this go-faster code, the action from here has moved to the negative half of the cycle)
    
  } // end of processing that is specific to samples where the voltage is positive
  
  else // the polarity of this sample is negative
  {     
    if (polarityConfirmedOfLastSampleV != NEGATIVE)
    {
      // This is the start of a new -ve half cycle (just after the zero-crossing point)      
      // which is a convenient point to update the Low Pass Filter for DC-offset removal
      //  The portion which is fed back into the integrator is approximately one percent
      // of the average offset of all the Vsamples in the previous mains cycle.
      //
      long previousOffset = DCoffset_V_long;
      DCoffset_V_long = previousOffset + (cumVdeltasThisCycle_long>>12); 
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

      // The average power that has been measured during the first half of this mains cycle can now be used
      // to predict the energy state at the end of this mains cycle.  That prediction will be used to alter 
      // the state of the load as necessary. The arming signal for the triac can't be set yet - that must 
      // wait until the voltage has advanced further beyond the -ve going zero-crossing point.
      //
      long averagePower = sumP_grid / sampleSetsDuringThisMainsCycle;// for 1st half of this mains cycle only
      //
      // To avoid repetitive and unnecessary calculations, the increase in energy during each mains cycle is
      // deemed to be numerically equal to the average power.  The predicted value for the energy state at the 
      // end of this mains cycle will therefore be the known energy state at its start plus the average power 
      // as measured. Although the average power has been determined over only half a mains cycle, the correct
      // number of contributing sample sets has been used so the result can be expected to be a true measurement 
      // of average power, not half of it. 
      // 
      energyInBucket_prediction = energyInBucket_long + averagePower; // at end of this mains cycle
        
           
    } // end of processing that is specific to the first Vsample in each -ve half cycle
    
    // check to see whether the trigger device can now be reliably armed
    if (sampleSetsDuringNegativeHalfOfMainsCycle == 3) 
    {
   if (beyondStartUpPhase)
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
  
        if (energyInBucket_prediction > midPointOfEnergyBucket_long)
        {  
          // the energy state is in the upper half of the working range
          lowerEnergyThreshold = lowerThreshold_default; // reset the "opposite" threshold 
          if (energyInBucket_prediction > upperEnergyThreshold) 
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
                upperEnergyThreshold = energyInBucket_prediction; 
                  
                // the energy thresholds must remain within range
                if (upperEnergyThreshold > capacityOfEnergyBucket_long)
                {
                  upperEnergyThreshold = capacityOfEnergyBucket_long;
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
          if (energyInBucket_prediction < lowerEnergyThreshold)
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
                  lowerEnergyThreshold = energyInBucket_prediction; 
                  
                  // the energy thresholds must remain within range
                  if (lowerEnergyThreshold < 0)
                  {
                    lowerEnergyThreshold = 0;
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
        
    sampleSetsDuringNegativeHalfOfMainsCycle++;
  } // end of processing that is specific to samples where the voltage is negative
  
  // processing for EVERY set of samples
  //
  // First, deal with the power at the grid connection point (as measured via CT1)
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_grid = ((long)(sampleI_grid-DCoffset_I))<<8;
  //
  // extra filtering to offset the HPF effect of CT1
  long last_lpf_long = lpf_long;
  lpf_long = last_lpf_long + alpha *(sampleIminusDC_grid - last_lpf_long);
  sampleIminusDC_grid += (lpf_gain * lpf_long);

  // 
  // calculate the "real power" in this sample pair and add to the accumulated sum
  long filtV_div4 = sampleVminusDC_long>>2;  // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4 = sampleIminusDC_grid>>2; // reduce to 16-bits (now x64, or 2^6)
  long instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
  instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_grid +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  // Now deal with the diverted power (as measured via CT2)
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_diverted = ((long)(sampleI_diverted-DCoffset_I))<<8;

  // calculate the "real power" in this sample pair and add to the accumulated sum
  filtI_div4 = sampleIminusDC_diverted>>2; // reduce to 16-bits (now x64, or 2^6)
  instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
  instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_diverted +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  sampleSetsDuringThisMainsCycle++;
  
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  polarityConfirmedOfLastSampleV = polarityConfirmed;  // for identification of half cycle boundaries

  refreshDisplay();
}
//  ----- end of main Mk2i code -----

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
    
  if (count > PERSISTENCE_FOR_POLARITY_CHANGE)
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
   
/*
  if (loadPriorityMode == LOAD_1_HAS_PRIORITY)
  {
    // swap physical loads 0 & 1 if remote load has priority 
    physicalLoadState[0] = logicalLoadState[1];
    physicalLoadState[1] = logicalLoadState[0];
  } 
*/  
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





