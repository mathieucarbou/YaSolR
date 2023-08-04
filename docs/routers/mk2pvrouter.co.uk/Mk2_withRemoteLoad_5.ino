/* Mk2_RFremoteLoad_5.ino
 *
 * This sketch is for diverting surplus PV power to a local dump load using a triac 
 * or a SolidStateRelay.  A remote load is also supported, this being controlled 
 * using the on-board RFM12B module. An external switch allows either load to be 
 * selected as having the higher priority.  If a local load is not provided, all 
 * surplus power is available for use by the remote load. 
 *
 * This sketch is intended for use with my PCB-based hardware for the Mk2 PV Router.  
 * The selector switch, as mentioned above, connects to the "mode" port.  For this
 * version of the Mk2 code, the system uses an alternative version of the anti-flicker 
 * algorithm which is well suited for multiple loads.  As the 'normal' mode is no longer 
 * required, the 'mode' port can be re-assigned for priority selection.   
 *
 * The integral voltage sensor is fed from one of the secondary coils of the transformer. 
 * Current is measured via Current Transformers at the CT1 and CT2 ports.  
 *
 * CT1 is for 'grid' current, to be measured at the grid supply point.
 * CT2 is for the 'diverted' current, so that energy which is diverted via the
 * local dump load can be recorded and displayed locally.
 *
 * A persistence-based 4-digit display is supported. When the RFM12B module is 
 * in use, the display can only be used in conjunction with an extra pair of 
 * logic chips.  These are ICs 3 and 4, which reduce the number of processor pins 
 * that are needed to drive the display.
 *
 * This sketch has many similarities with Revision 5c of the Mk2i PV Router code that I 
 * have posted on the OpenEnergyMonitor forum.  That version, and other related material, 
 * can be found on my Summary Page at www.openenergymonitor.org/emon/node/1757
 *
 * September 2014: renamed as Mk2_withRemoteLoad_3, with these changes:
 * - reimplementation of cycleCount, as it could have overflowed with unpredictable results;
 * - the functions increaseLoadIfPossible() and decreaseLoadIfPossible() have been tidied;
 * - energyThreshold_long has been renamed as midPointOfEnergyBucket_long.
 *
 * December 2014: renamed as Mk2_withRemoteLoad_4, with these changes:
 * - persistence check added for zero-crossing detection (polarityConfirmed);
 * - lowestNoOfSampleSetsPerMainsCycle added, to check for any disturbances;
 *
 * January 2016: renamed as Mk2_withRemoteLoad_4a, with these changes:
 * - a minor change in the routine timerIsr() has removed a timing uncertainty.
 * - support for the RF69 RF module has been added.
 *
 * January 2016: updated to Mk2_withRemoteLoad_4b:
 *   The variables to store the ADC results are now declared as "volatile" to remove 
 *   any possibility of incorrect operation due to optimisation by the compiler.
 *
 * February 2016: updated to Mk2_bothDisplays_5, with these changes:
 * - improvements to the start-up logic.  The start of normal operation is now 
 *    synchronised with the start of a new mains cycle.
 * - reduce the amount of feedback in the Low Pass Filter for removing the DC content
 *     from the Vsample stream. This resolves an anomaly which has been present since 
 *     the start of this project.  Although the amount of feedback has previously been 
 *     excessive, this anomaly has had minimal effect on the system's overall behaviour.
 * - removal of the unhelpful "triggerNeedsToBeArmed" mechanism
 * - tidying of the "confirmPolarity" logic to make its behaviour more clear
 * - SWEETZONE_IN_JOULES changed to WORKING_RANGE_IN_JOULES 
 *
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 */

// #define RF69_COMPAT 0  // <-- include this line for the RFM12B
#define RF69_COMPAT 1  // <-- include this line for the RF69

#include <Arduino.h> 
#include <JeeLib.h>     // JeeLib is available at from: http://github.com/jcw/jeelib
#include <TimerOne.h>

#define ADC_TIMER_PERIOD 125 // uS (determines the sampling rate / amount of idle time)

// Physical constants, please do not change!
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define JOULES_PER_WATT_HOUR 3600 //  (0.001 kWh = 3600 Joules)

// Change these values to suit the local mains frequency and supply meter
#define CYCLES_PER_SECOND 50 
#define WORKING_RANGE_IN_JOULES 3600 
#define REFRESH_PERIOD_IN_CYCLES 50  // max allowed interval between RF messages
#define REQUIRED_EXPORT_IN_WATTS 0 // when set to a negative value, this acts as a PV generator 

// to prevent the diverted energy total from 'creeping'
#define ANTI_CREEP_LIMIT 5 // in Joules per mains cycle (has no effect when set to 0)
long antiCreepLimit_inIEUperMainsCycle;

const byte noOfDumploads = 2; // The logic expects a minimum of 2 dumploads, 
                              // for local & remote loads, but neither has to
                              // be physically present. 

// definitions of enumerated types and instances of same
enum polarities {NEGATIVE, POSITIVE};
enum outputModes {ANTI_FLICKER, NORMAL};
enum loadPriorityModes {REMOTE_HAS_PRIORITY, LOCAL_HAS_PRIORITY};

enum loadStates {LOAD_ON, LOAD_OFF}; // all loads are active low
enum loadStates logicalLoadState[noOfDumploads]; 
enum loadStates physicalLoadState[noOfDumploads]; 

enum energyStates {LOWER_HALF, UPPER_HALF}; // for single threshold AF algorithm
enum energyStates energyStateNow;

// ----------------  Extra Features selection ----------------------
//
// - WORKLOAD_CHECK, for determining how much spare processing time there is. 
//  
// #define WORKLOAD_CHECK  // <-- Include this line is this feature is required 


// For most single-load Mk2 systems, the power-diversion logic can operate in either of two modes:
//
// - NORMAL, where the load switches rapidly on/off to maintain a constant energy level.
// - ANTI_FLICKER, whereby the repetition rate is reduced to avoid rapid fluctuations
//    of the local mains voltage.
//
// For this multi-load version, the same mechanism has been retained but the 
// output mode is hard-coded as below:
enum outputModes outputMode = ANTI_FLICKER;    

// In this multi-load version, the external switch is re-used to determine the load priority
enum loadPriorityModes loadPriorityMode = LOCAL_HAS_PRIORITY;                                                   

/* frequency options are RF12_433MHZ, RF12_868MHZ or RF12_915MHZ
 */
#define freq RF12_433MHZ // Use the freq to match the module you have.

const int nodeID = 10;  //  RFM12B node ID
const int networkGroup = 210;  //  RFM12B wireless network group - needs to be same as emonBase and emonGLCD 
const int UNO = 1;  // Set to 0 if you're not using the UNO bootloader (i.e using Duemilanove) 
                    // - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader

const int noOfCyclesBeforeRefresh = 50; // nominally every second, but not critical
//long cycleCountAtLastRFtransmission = 0;
int messageNumber = 0;

//  data structure for RF comms
typedef struct { byte dumpState; int msgNumber; } Tx_struct;    //  data for RF comms
Tx_struct tx_data;


// allocation of digital pins when pin-saving hardware is in use
// *************************************************************
// D0 & D1 are reserved for the Serial i/f
// D2 is for the RFM12B
const byte loadPrioritySelectorPin = 3; // <-- this is the "mode" port  
const byte physicalLoad_0_pin = 4;  // <-- this is the "trigger" port  
// D5 is the enable line for the 7-segment display driver, IC3 
// D6 is a data input line for the 7-segment display driver, IC3
// D7 is a data input line for the 7-segment display driver, IC3
// D8 is a data input line for the 7-segment display driver, IC3
// D9 is a data input line for the 7-segment display driver, IC3
// D10 is for the RFM12B
// D11 is for the RFM12B
// D12 is for the RFM12B
// D13 is for the RFM12B

// allocation of analogue pins
// ***************************
// A0 (D14) is the decimal point driver line for the 4-digit display 
// A1 (D15) is a digit selection line for the 4-digit display, via IC4
// A2 (D16) is a digit selection line for the 4-digit display, via IC4
const byte voltageSensor = 3;          // A3 is for the voltage sensor
const byte currentSensor_diverted = 4; // A4 is for CT2 which measures diverted current
const byte currentSensor_grid = 5;     // A5 is for CT1 which measures grid current


const byte delayBeforeSerialStarts = 3;  // in seconds, to allow Serial window to be opened
const byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;    // nominal mid-point value of ADC @ x1 scale

// General global variables that are used in multiple blocks so cannot be static.
// For integer maths, many variables need to be 'long'
//
long energyInBucket_long = 0;      // in Integer Energy Units
long capacityOfEnergyBucket_long;  // depends on powerCal, frequency & the 'sweetzone' size.
long midPointOfEnergyBucket_long;         // used for 'normal' and single-threshold 'AF' logic
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

int postMidPointCrossingDelay_cycles; // assigned in setup(), different for each output mode
const int postMidPointCrossingDelayForAF_cycles = 10; // in 20 ms counts 
const int interLoadSeparationDelay_cycles = 25; // in 20 ms cycle counts (for both output modes)
byte activeLoadID; // only one load may operate freely at a time.

long energyAtLastOffTransition_long; 
long energyAtLastOnTransition_long; 
int mainsCyclesSinceLastMidPointCrossing = 0;
int mainsCyclesSinceLastChangeOfLoadState = 0;
int mainsCyclesSinceLastRF_tx = 0;


// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
volatile int sampleI_grid;
volatile int sampleI_diverted;
volatile int sampleV;

// for the remote load that is controlled by RF
unsigned long cycleCountAtLastTransmission = 0;
boolean sendRFcommandNextTime = false;

// For an enhanced polarity detection mechanism, which includes a persistence check
#define PERSISTENCE_FOR_POLARITY_CHANGE 3 // sample sets
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
  for(int i = 0; i< noOfDumploads; i++)
  {
    logicalLoadState[i] = LOAD_OFF;
    physicalLoadState[i] = LOAD_OFF;
  } 
  
  digitalWrite(physicalLoad_0_pin, physicalLoadState[0]); // force the local load to be in the "off" state.      

  pinMode(loadPrioritySelectorPin, INPUT); // this pin is tracked to the "mode" connector
  digitalWrite(loadPrioritySelectorPin, HIGH); // enable the internal pullup resistor
  delay (100); // allow time to settle
  int pinState = digitalRead(loadPrioritySelectorPin);  // initial selection and
  loadPriorityMode = (enum loadPriorityModes)pinState;  //   assignment of priority 
 
  delay(delayBeforeSerialStarts * 1000); // allow time to open Serial monitor      
 
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      Mk2_withRemoteLoad_5.ino");
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
//  phaseCal_grid_int = phaseCal_grid * 256; // for integer maths <-- not supported
//  phaseCal_diverted_int = phaseCal_diverted * 256; // for integer maths  <-- not supported
  
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
  energyAtLastOffTransition_long = midPointOfEnergyBucket_long; 
  energyAtLastOnTransition_long = midPointOfEnergyBucket_long; 

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
  if (loadPriorityMode == LOCAL_HAS_PRIORITY) {
    Serial.println ( "local"); }
  else 
  {  
    Serial.println ( "remote");
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
  Serial.println (PERSISTENCE_FOR_POLARITY_CHANGE);
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
 
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RF
//  rf12_sleep(RF12_SLEEP); <- the RFM12B now stays awake throughout

}

// An Interrupt Service Routine is now defined in which the ADC is instructed to 
// measure each analogue input in sequence.  A "data ready"flag is set after each 
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
  static boolean triggerNeedsToBeArmed = false;  // once per mains cycle (+ve half)
//  static int samplesDuringThisMainsCycle;             // for normalising the power in each mains cycle
  static long sumP_grid;                              // for per-cycle summation of 'real power' 
  static long sumP_diverted;                              // for per-cycle summation of 'real power' 
//  static enum polarities polarityOfLastSampleV;  // for zero-crossing detection
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static long lastSampleVminusDC_long;     //    for the phaseCal algorithm
  static byte perSecondCounter = 0;
   
  // remove DC offset from the raw voltage sample by subtracting the accurate value 
  // as determined by a LP filter.
  long sampleVminusDC_long = ((long)sampleV<<8) - DCoffset_V_long; 

  // determine the polarity of the latest voltage sample
  enum polarities polarityNow;   
  if(sampleVminusDC_long > 0) { 
    polarityOfMostRecentVsample = POSITIVE; }
  else { 
    polarityOfMostRecentVsample = NEGATIVE; }
  confirmPolarity();

  if (polarityConfirmed == POSITIVE) 
  { 
    if (polarityConfirmedOfLastSampleV != POSITIVE)
    {
      if (beyondStartUpPhase)
      {     
        // This is the start of a new +ve half cycle (just after the zero-crossing point)
        // cycleCount++;  <- this mechanism is unsafe, it will eventually overflow
        mainsCyclesSinceLastMidPointCrossing++;
        mainsCyclesSinceLastChangeOfLoadState++;
        mainsCyclesSinceLastRF_tx++;
        
        // a simple routine for checking the continuity of the sampling scheme     
        if (sampleSetsDuringThisMainsCycle < lowestNoOfSampleSetsPerMainsCycle) {
          lowestNoOfSampleSetsPerMainsCycle = sampleSetsDuringThisMainsCycle; }
        
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
          
//          Serial.println (energyInBucket_long * IEUtoJoulesConversion_CT1);
        }


        // when using the single-threshold power diversion algorithm, a counter needs
        // to be reset whenever the energy level in the accumulator crosses the mid-point 
        //
        enum energyStates energyStateOnLastLoop = energyStateNow;

        if (energyInBucket_long > midPointOfEnergyBucket_long) {
          energyStateNow = UPPER_HALF; }
        else {
          energyStateNow = LOWER_HALF; }
        
        if (energyStateNow != energyStateOnLastLoop) {
          mainsCyclesSinceLastMidPointCrossing = 0; }

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
    // check to see whether the trigger device can now be reliably armed
    if(sampleSetsDuringThisMainsCycle == 3) // much easier than checking the voltage level
    {
      if (beyondStartUpPhase)
      {           
        boolean OKtoSendRFcommandNow = false; 
        boolean changeOfLoadState = false;
        
        // a pending RF command takes priority over the normal logic
        if (sendRFcommandNextTime)
        {
          OKtoSendRFcommandNow = true;
          sendRFcommandNextTime = false;
        } 
        else
        {   
          /* Now it's time to determine whether any of the the loads need to be changed.  
           * This is a 2-stage process:
           *   First, change the LOGICAL loads as necessary, then update the PHYSICAL
           * loads according to the mapping that exists between them.  The mapping is 
           * 1:1 by default but can be altered by a hardware switch which allows the 
           * priority of the remote load to be altered.
           *   This code uses a single-threshold algorithm which relies on regular switching 
           * of the load.  
           */
          if (mainsCyclesSinceLastMidPointCrossing > postMidPointCrossingDelay_cycles)
          {           
            if (energyInBucket_long > midPointOfEnergyBucket_long) 
            {
              increaseLoadIfPossible();  // to reduce the level in the bucket
            }  
            else
            {
              decreaseLoadIfPossible();  // to increase the level in the bucket       
            }
          }
          
          
          /* Update the state of all physical loads and determine whether the  
           * state of the remote load has changed. The remote load is hard-coded 
           * as Physical Load 1 (Physical Load 0 is a local load)
           */
          boolean remoteLoadHasChangedState = false;
          byte prevStateOfRemoteLoad = (byte)physicalLoadState[1];
          updatePhysicalLoadStates();
          if ((byte)physicalLoadState[1] != prevStateOfRemoteLoad)
          {
            remoteLoadHasChangedState = true;
          }
                
          /* Now determine whether an RF command should be sent.  This can be for 
           * either of two reasons:
           *
           * - the on/off state of the remote load needs to be changed
           * - a refresh command is due ('cos no change of state has occurred recently)
           *
           * If the on/off state needs to be changed, but a refresh command was sent on the
           * previous cycle, the 'change of state' command is deferred until the next 
           * cycle.  A refresh command can always be sent straight away because it can 
           * be guaranteed that no command has been sent immediately beforehand.
           */           
          if (remoteLoadHasChangedState)
          {
            // ensure that RF commands are not sent on consecutive cycles
            if (mainsCyclesSinceLastRF_tx > 1)
            {
              // the "change of state" can be acted on immediately
              OKtoSendRFcommandNow = true; // local flag
            }
            else
            {
              // the "change of state" must be deferred until next cycle
              sendRFcommandNextTime = true; // global flag
            }
          }
          else
          {
            // no "change of state", so check whether a refresh command is due
            if (mainsCyclesSinceLastRF_tx >= noOfCyclesBeforeRefresh)
            {
              // a refresh command is due (which can always be sent immediately)
              OKtoSendRFcommandNow = true;
            }
          }
        }
     
        // update the local load, which is physical load 0
        digitalWrite(physicalLoad_0_pin, physicalLoadState[0]);       
        
        // update the remote load, which is physical load 1 
        if (OKtoSendRFcommandNow)
        {
          mainsCyclesSinceLastRF_tx = 0;
          tx_data.msgNumber = messageNumber++;
          tx_data.dumpState = physicalLoadState[1];
          send_rf_data();
/*         
          Serial.print(tx_data.dumpState);    
          Serial.print(", ");    
          Serial.print(tx_data.msgNumber);             
          Serial.print(";   ");          
          Serial.println(activeLoadID); // useful for keeping track of different priority loads
*/         
        }       
                   
        // update the Energy Diversion Detector
        if (physicalLoadState[0] == LOAD_ON) {
          absenceOfDivertedEnergyCount = 0; 
          EDD_isActive = true; }            
        else {
          absenceOfDivertedEnergyCount++; }   
 
        // clear the flag which ensures that loads are only updated once per mains cycle
        triggerNeedsToBeArmed = false;           
      }
    }
    
  } // end of processing that is specific to samples where the voltage is positive
  
  else // the polatity of this sample is negative
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
        
//      checkOutputModeSelection(); // updates outputMode if switch is changed
        checkLoadPrioritySelection(); // updates load priorities if switch is changed
           
    } // end of processing that is specific to the first Vsample in each -ve half cycle
  } // end of processing that is specific to samples where the voltage is negative
  
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
  
  sampleSetsDuringThisMainsCycle++;
 
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
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
void increaseLoadIfPossible()
{
  /* if permitted by A/F rules, turn on the highest priority logical load that is not already on.
   */
  boolean changed = false;
  
  // Only one load may operate freely at a time.  Other loads are prevented from 
  // switching until a sufficient period has elapsed since the last transition. 
  // This scheme allows a lower priority load to contribute if a higher priority
  // load is not having the desired effect, but not immediately.
  // 
  if (energyInBucket_long >= energyAtLastOnTransition_long)
  {     
//    Serial.print('+'); // useful for testing this logic
    boolean timeout = (mainsCyclesSinceLastChangeOfLoadState > interLoadSeparationDelay_cycles); 
    for (int i = 0; i < noOfDumploads && !changed; i++)
    {
      if (logicalLoadState[i] == LOAD_OFF)
      {
        if ((i == activeLoadID) || timeout)
        {
          logicalLoadState[i] = LOAD_ON;
          mainsCyclesSinceLastChangeOfLoadState = 0;
          energyAtLastOnTransition_long = energyInBucket_long;
          energyAtLastOffTransition_long = midPointOfEnergyBucket_long; // reset the 'opposite' mechanism.
          activeLoadID = i;
          changed = true; 
        }
      }
    }
  }
  else
  {
    // energy level has not risen so there's no need to apply any more load
  }
//  return (changed);
}
 
void decreaseLoadIfPossible()
{
  /* if permitted by A/F rules, turn off the lowest priority logical load that is not already off.
   */
  boolean changed = false;
  
  // Only one load may operate freely at a time.  Other loads are prevented from 
  // switching until a sufficient period has elapsed since the last transition. 
  // This scheme allows a lower priority load to contribute if a higher priority
  // load is not having the desired effect, but not immediately.
  // 
  if (energyInBucket_long <= energyAtLastOffTransition_long)
  {     
//    Serial.print('-');  // useful for testing this logic
    boolean timeout = (mainsCyclesSinceLastChangeOfLoadState > interLoadSeparationDelay_cycles); 
//    for (int i = 0; i < noOfDumploads && !done; i++)
    for (int i = (noOfDumploads -1); i >= 0 && !changed; i--)
    {
      if (logicalLoadState[i] == LOAD_ON)
      {
        if ((i == activeLoadID) || timeout)
        {
          logicalLoadState[i] = LOAD_OFF;
          mainsCyclesSinceLastChangeOfLoadState = 0;
          energyAtLastOffTransition_long = energyInBucket_long;
          energyAtLastOnTransition_long = midPointOfEnergyBucket_long; // reset the 'opposite' mechanism.
          activeLoadID = i;
          changed = true; 
        }
      }
    }
  }
  else
  {
    // energy level has not fallen so there's no need to remove any more load
  }
//  return (changed);
}


void updatePhysicalLoadStates()
/*
 * This function provides the link between the logical and physical loads.  The 
 * array, logicalLoadState[], contains the on/off state of all logical loads, with 
 * element 0 being for the one with the highest priority.  The array, 
 * physicalLoadState[], contains the on/off state of all physical loads. 
 * 
 * By default, the association between the physical and logical loads is 1:1.  If
 * the remote load is set to have priority, the logical-to-physical association for
 * loads 0 and 1 are swapped.
 *
 * - Physical load 0 is local, and is controlled by use of an output pin.  
 * - Physical load 1 is remote, and is controlled via the associated RFM12B module.
 */
{
  for (int i = 0; i < noOfDumploads; i++)
  {
    physicalLoadState[i] = logicalLoadState[i]; 
  }
   
  if (loadPriorityMode == REMOTE_HAS_PRIORITY)
  {
    // swap physical loads 0 & 1 if remote load has priority 
    physicalLoadState[0] = logicalLoadState[1];
    physicalLoadState[1] = logicalLoadState[0];
  } 
}


// this function changes the value of the load priorities if the state of the external switch is altered 
void checkLoadPrioritySelection()  
{
  static byte loadPrioritySwitchCcount = 0;
  int pinState = digitalRead(loadPrioritySelectorPin);
  if (pinState != loadPriorityMode)
  {
    loadPrioritySwitchCcount++;
  }  
  if (loadPrioritySwitchCcount >= 20)
  {
    loadPrioritySwitchCcount = 0;
    loadPriorityMode = (enum loadPriorityModes)pinState;  // change the global variable
    Serial.print ("loadPriority selection changed to ");
    if (loadPriorityMode == LOCAL_HAS_PRIORITY) {
      Serial.println ( "local"); }
    else {  
      Serial.println ( "remote"); }
  }
}


// Although this sketch always operates in ANTI_FLICKER mode, it was convenient
// to leave this mechanism in place.
//
void configureParamsForSelectedOutputMode()
{
       
  if (outputMode == ANTI_FLICKER)
  {
    postMidPointCrossingDelay_cycles = postMidPointCrossingDelayForAF_cycles; 
  }
  else
  { 
    postMidPointCrossingDelay_cycles = 0;
  }
  
  // display relevant settings for selected output mode
  Serial.print("  capacityOfEnergyBucket_long = ");
  Serial.println(capacityOfEnergyBucket_long);
  Serial.print("  midPointOfEnergyBucket_long   = ");
  Serial.println(midPointOfEnergyBucket_long);
  Serial.print("  postMidPointCrossingDelay_cycles = ");
  Serial.println(postMidPointCrossingDelay_cycles); 
  Serial.print("  interLoadSeparationDelay_cycles = ");
  Serial.println(interLoadSeparationDelay_cycles); 
  
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

void send_rf_data()
{
 // rf12_sleep(RF12_WAKEUP);
  // if ready to send + exit route if it gets stuck 
  int i = 0; 
  while (!rf12_canSend() && i<10)
  { 
    rf12_recvDone(); 
    i++;
  }
  rf12_sendNow(0, &tx_data, sizeof tx_data);

 // rf12_sendStart(0, &tx_data, sizeof tx_data);
 // rf12_sendWait(2);
 // rf12_sleep(RF12_SLEEP);
}


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}




