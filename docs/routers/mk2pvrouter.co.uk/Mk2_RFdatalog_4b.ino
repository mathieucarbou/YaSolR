/* Mk2_RFdatalog_4b.ino
 *
 * This sketch is for diverting surplus PV power to a dump load using a triac. 
 * Routine datalogging is also supported using the on-board RFM12B module.
 *
 * This sketch is intended for use with my PCB-based hardware for the Mk2 PV Router.  
 * The integral voltage sensor is fed from one of the secondary coils of the 
 * transformer.  Current is measured via Current Transformers at the 
 * CT1 and CT2 ports.  
 * 
 * CT1 is for 'grid' current, to be measured at the grid supply point.
 * CT2 is for the load current, so that diverted energy can be recorded
 *
 * A persistence-based 4-digit display is supported. When the RFM12B module is 
 * in use, the display can only be used in conjunction with an extra pair of 
 * logic chips.  These are ICs 3 and 4, which reduce the number of processor pins 
 * that are needed to drive the display.
 *
 * This sketch is based on the Mk2i PV Router code that I have posted in on the 
 * OpenEnergyMonitor forum.  The original version, and other related material, 
 * can be found on my Summary Page at www.openenergymonitor.org/emon/node/1757
 *
 * September 2014: renamed as Mk2_RFdatalog_3, with these changes:
 * - cycleCount removed (was not actually used in this sketch, but could have overflowed);
 * - tidier initialisation of display logic in setup();
 *
 * December 2014, upgraded to Mk2_RFdatalog_4:  
 *    This sketch has been restructured in order to make better use of the ISR.  All of 
 * the time-critical code is now contained within the ISR and its helper functions.  
 * Values for datalogging are transferred to the main code using a flag-based handshake 
 * mechanism.  The diversion of surplus power can no longer be affected by slower 
 * activities which may be running in the main code such as Serial statements and RF.  
 *    Temperature sensing is supported by re-allocating the "mode" port for this 
 * purpose. A pullup resistor (4K7 or similar) is required for the Dallas sensor.  The 
 * output mode, i.e. NORMAL or ANTI_FLICKER, is now set at compile time.
 * Also:
 * - The ADC is now in free-running mode, at ~104 us per conversion.
 * - a persistence check has been added for zero-crossing detection (polarityConfirmed)
 * - a lowestNoOfSampleSetsPerMainsCycle check has been added, to detect any disturbances
 * - Vrms has been added to the datalog payload (as Vrms x 100)
 * - temperature has been added to the datalog payload (as degrees C x 100)
 * - the phaseCal mechanism has been reinstated
 *
 * January 2016: renamed as Mk2_RFdatalog_4a, with a minor change in the ISR to reinstate
 *   the phaseCal calculation.  Previously, this feature was having no effect because 
 *   two assignment lines were in the wrong order.  When measuring "real power", which
 *   is what this application does, the phaseCal refinement has very little effect even
 *   when correctly implemented, as it now is.
 *   Support for the RF69 RF module has also been added.
 *
 * January 2016: updated to Mk2_RFdatalog_4b:
 *   The variables to store copies of ADC results for use by the main code are now declared 
 *   as "volatile" to remove any possibility of incorrect operation due to optimisation 
 *   by the compiler.
 *
 * Robin Emley
 *      www.Mk2PVrouter.co.uk
 */

#define RF69_COMPAT 0  // <-- include this line for the RFM12B
// #define RF69_COMPAT 1  // <-- include this line for the RF69


#include <Arduino.h> 
#include <OneWire.h>
#include <JeeLib.h>     // JeeLib is available at from: http://github.com/jcw/jeelib

// Physical constants, please do not change!
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define JOULES_PER_WATT_HOUR 3600 //  (0.001 kWh = 3600 Joules)

// -----------------------------------------------------
// Change these values to suit the local mains frequency and supply meter
#define CYCLES_PER_SECOND 50 
#define SWEETZONE_IN_JOULES 3600 
#define REQUIRED_EXPORT_IN_WATTS 0 // when set to a negative value, this acts as a PV generator 

// --------------------------
// Dallas DS18B20 commands
#define SKIP_ROM 0xcc 
#define CONVERT_TEMPERATURE 0x44
#define READ_SCRATCHPAD 0xbe
#define BAD_TEMPERATURE 30000 // this value (300C) is sent if no sensor is present

// ----------------
// general literals
#define DATALOG_PERIOD_IN_MAINS_CYCLES  250 
// #define POST_DATALOG_EVENT_DELAY_MILLIS 40
#define ANTI_CREEP_LIMIT 0 // <- to prevent the diverted energy total from 'creeping'
                           // in Joules per mains cycle (has no effect when set to 0)

// to prevent the diverted energy total from 'creeping'
#define ANTI_CREEP_LIMIT 5 // in Joules per mains cycle (has no effect when set to 0)
long antiCreepLimit_inIEUperMainsCycle;

// -------------------------------
// definitions of enumerated types
enum polarities {NEGATIVE, POSITIVE};
enum triacStates {TRIAC_ON, TRIAC_OFF}; // the external trigger device is active low
enum outputModes {ANTI_FLICKER, NORMAL}; // retained for compatibility with previous versions.

// ----  Output mode selection -----
enum outputModes outputMode = ANTI_FLICKER; // <- needs to be set here unless an                                                  
//enum outputModes outputMode = NORMAL;     //    external switch is in use                                              

/* --------------------------------------
 * RF configuration (for the RFM12B module)
 * frequency options are RF12_433MHZ, RF12_868MHZ or RF12_915MHZ
 */
#define freq RF12_433MHZ

const int nodeID = 10;  //  RFM12B node ID
const int networkGroup = 210;  // wireless network group - needs to be same for all nodes 
const int UNO = 1;  // for when the processor contains the UNO bootloader.

typedef struct { 
  int powerAtSupplyPoint_Watts; // import = +ve, to match OEM convention
  int divertedEnergyTotal_Wh; // always positive
  int Vrms_times100;
  int temperature_times100;
} Tx_struct; 
Tx_struct tx_data; 

// allocation of digital pins when pin-saving hardware is in use
// *************************************************************
// D0 & D1 are reserved for the Serial i/f
// D2 is for the RFM12B
const byte tempSensorPin = 3; // <-- the "mode" port 
const byte outputForTrigger = 4;   
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

const byte delayBeforeSerialStarts = 5;  // in seconds, to allow Serial window to be opened
const byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;    // nominal mid-point value of ADC @ x1 scale

/* -------------------------------------------------------------------------------------
 * Global variables that are used in multiple blocks so cannot be static.
 * For integer maths, many variables need to be 'long'
 */
boolean beyondStartUpPhase = false;     // start-up delay, allows things to settle
long energyInBucket_long; // in Integer Energy Units (for controlling the dump-load) 
long capacityOfEnergyBucket_long;  // depends on powerCal, frequency & the 'sweetzone' size.
long lowerEnergyThreshold_long;    // for turning load off
long upperEnergyThreshold_long;    // for turning load on
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
float IEUtoJoulesConversion_CT1;

float offsetOfEnergyThresholdsInAFmode = 0.1; // <-- not wise to exceeed 0.4

long sumP_forEnergyBucket;        // for per-cycle summation of 'real power' 
long sumP_diverted;              // for per-cycle summation of diverted power
long sumP_atSupplyPoint;         // for summation of 'real power' values during datalog period 
long sum_Vsquared;                // for summation of V^2 values during datalog period            
int sampleSetsDuringThisCycle;    // for counting the sample sets during each mains cycle
long sampleSetsDuringThisDatalogPeriod; // for counting the sample sets during each datalogging period

long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
long lastSampleVminusDC_long;     //    for the phaseCal algorithm
byte cycleCountForDatalogging = 0;  
long sampleVminusDC_long;
long requiredExportPerMainsCycle_inIEU;

// for interaction between the main code and the ISR
volatile boolean datalogEventPending = false;
volatile boolean newMainsCycle = false;
volatile long copyOf_sumP_atSupplyPoint;          
volatile long copyOf_sum_Vsquared;
volatile long copyOf_divertedEnergyTotal_Wh;          
volatile int copyOf_lowestNoOfSampleSetsPerMainsCycle;
volatile long copyOf_sampleSetsDuringThisDatalogPeriod;

// For temperature sensing
OneWire oneWire(tempSensorPin);
int tempTimes100;

// For an enhanced polarity detection mechanism, which includes a persistence check
#define POLARITY_CHECK_MAXCOUNT 2
enum polarities polarityNow;   
enum polarities polarityConfirmed;  // for zero-crossing detection
enum polarities polarityConfirmedOfLastSampleV;  // for zero-crossing detection

// For a mechanism to check the integrity of this code structure
int lowestNoOfSampleSetsPerMainsCycle;
unsigned long timeAtLastDelay;


// Calibration values (not important for the Router's basic operation)
//-------------------
// For accurate calculation of real power/energy, two calibration values are 
// used: powerCal and phaseCal.  With most hardware, the default values are 
// likely to work fine without need for change.  A full explanation of each 
// of these values now follows:
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
// My sketch "RawSamplesTool_2chan.ino" provides a one-shot visual display of the
// voltage and current waveforms.  This provides an easy way for the user to be 
// confident that their system has been set up correctly for the power levels 
// that are to be measured.  Any pre-built system that I supply will have been
// checked with this tool to ensure that the input sensors are working correctly.
//
// The ADC has an input range of 0-5V and an output range of 0-1023 levels.
// The purpose of each input sensor is to convert the measured parameter into a 
// low-voltage signal which fits nicely within the ADC's input range. 
//
// In the case of 230V mains voltage, the numerical value of the input signal 
// in Volts is likely to be fairly similar to the output signal in ADC levels.  
// 230V AC has a peak-to-peak amplitude of 651V, which is not far from the ideal 
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
// for 3.3V operation, the optimum value is generally around 0.044
// for 5V operation, the optimum value is generally around 0.072
//
const float powerCal_grid = 0.072;  
const float powerCal_diverted = 0.073;  
 
                        
// phaseCal is used to alter the phase of the voltage waveform relative to the
// current waveform.  This mechanism can be used to offset any difference in 
// phase delay between the voltage and current sensors.  The algorithm interpolates 
// between the most recent pair of voltage samples according to the phaseCal value. 
//
//    With phaseCal = 1, the most recent sample is used.  
//    With phaseCal = 0, the previous sample is used
//    With phaseCal = 0.5, the mid-point (average) value is used
//
// Values ouside the 0 to 1 range involve extrapolation, rather than interpolation
// and are not recommended.  By altering the order in which V and I samples are 
// taken, and for how many loops they are stored, it should always be possible to
// arrange for the optimal value of phaseCal to lie within the range 0 to 1. 
//
// The calculation for real power is very insensitive to the value of phaseCal.   
// When a "real power" calculation is used to determine how much surplus energy 
// is available for diversion, a nominal value such as 1.0 is generally thought 
// to be sufficient for this purpose.
//
const float  phaseCal_grid = 1.0; 
const float  phaseCal_diverted = 1.0;  


// For datalogging purposes, voltageCal has been included too.  When running at 
// 230 V AC, the range of ADC values will be similar to the actual range of volts, 
// so the optimal value for this cal factor will be close to unity.  
//
const float voltageCal = 1.0; 


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

// The final column of this array is for the decimal point status.  
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

volatile boolean EDD_isActive = false; // energy diversion detection
//volatile boolean EDD_isActive = true; // energy diversion detection


void setup()
{  
  pinMode(outputForTrigger, OUTPUT);  
  digitalWrite (outputForTrigger, TRIAC_OFF); // the external trigger is active low

  delay(delayBeforeSerialStarts * 1000); // allow time to open Serial monitor      
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:   Mk2_RFdatalog_4b.ino");
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
  // is described in more detail just before the energy bucket is updated at the start 
  // of each new mains cycle.
  //
  // An electricity meter has a small range over which energy can ebb and flow without 
  // penalty.  This has been termed its "sweet-zone".  For optimal performance, the energy
  // bucket of a PV Router should match this value.  The sweet-zone's value is therefore 
  // included in the calculation below.
  //
  // For the flow of energy at the 'grid' connection point (CT1): 
  capacityOfEnergyBucket_long = 
     (long)SWEETZONE_IN_JOULES * CYCLES_PER_SECOND * (1/powerCal_grid);
  energyInBucket_long = capacityOfEnergyBucket_long * 0.45; // for rapid start up 
  
  IEUtoJoulesConversion_CT1 = powerCal_grid / CYCLES_PER_SECOND; // may be useful
  
  // For recording the accumulated amount of diverted energy data (using CT2), a similar 
  // calibration mechanism is required.  Rather than a bucket with a fixed capacity, the 
  // accumulator for diverted energy just needs to be scaled in a known way.  As soon as its 
  // value exceeds 1 Wh, an associated WattHour register is incremented, and the 
  // accumulator's value is decremented accordingly. The calculation below is to determine
  // the scaling for this accumulator.     
  IEU_per_Wh = 
     (long)JOULES_PER_WATT_HOUR * CYCLES_PER_SECOND * (1/powerCal_diverted); 
 
  // to avoid the diverted energy accumulator 'creeping' when the load is not active
  antiCreepLimit_inIEUperMainsCycle = (float)ANTI_CREEP_LIMIT * (1/powerCal_diverted);

  long mainsCyclesPerHour = (long)CYCLES_PER_SECOND * 
                             SECONDS_PER_MINUTE * MINUTES_PER_HOUR;                           
  displayShutdown_inMainsCycles = DISPLAY_SHUTDOWN_IN_HOURS * mainsCyclesPerHour;                           
      
  requiredExportPerMainsCycle_inIEU = (long)REQUIRED_EXPORT_IN_WATTS * (1/powerCal_grid); 

  // Define operating limits for the LP filter which identifies DC offset in the voltage 
  // sample stream.  By limiting the output range, the filter always should start up 
  // correctly.
  DCoffset_V_long = 512L * 256; // nominal mid-point value of ADC @ x256 scale  
  DCoffset_V_min = (long)(512L - 100) * 256; // mid-point of ADC minus a working margin
  DCoffset_V_max = (long)(512L + 100) * 256; // mid-point of ADC plus a working margin

  Serial.println ("ADC mode:       free-running");
  
  // Set up the ADC to be free-running 
  ADCSRA  = (1<<ADPS0)+(1<<ADPS1)+(1<<ADPS2);  // Set the ADC's clock to system clock / 128
  ADCSRA |= (1 << ADEN);                 // Enable the ADC 
  
  ADCSRA |= (1<<ADATE);  // set the Auto Trigger Enable bit in the ADCSRA register.  Because 
                         // bits ADTS0-2 have not been set (i.e. they are all zero), the 
                         // ADC's trigger source is set to "free running mode".
                         
  ADCSRA |=(1<<ADIE);    // set the ADC interrupt enable bit. When this bit is written 
                         // to one and the I-bit in SREG is set, the 
                         // ADC Conversion Complete Interrupt is activated. 

  ADCSRA |= (1<<ADSC);   // start ADC manually first time 
  sei();                 // Enable Global Interrupts  

  Serial.print ( "Output mode:    ");
  if (outputMode == NORMAL) {
    Serial.println ( "normal"); }
  else 
  {  
    Serial.println ( "anti-flicker");
    Serial.print ( "  offsetOfEnergyThresholds  = ");
    Serial.println ( offsetOfEnergyThresholdsInAFmode);    
  }
    
  Serial.print ( "powerCal_CT1, for grid consumption =      "); Serial.println (powerCal_grid,4);
  Serial.print ( "powerCal_CT2, for diverted power  =      "); Serial.println (powerCal_diverted,4);
  Serial.print ( "voltageCal, for Vrms  =      "); Serial.println (voltageCal,4);
  
  Serial.print ("Anti-creep limit (Joules / mains cycle) = ");
  Serial.println (ANTI_CREEP_LIMIT);
  Serial.print ("Export rate (Watts) = ");
  Serial.println (REQUIRED_EXPORT_IN_WATTS);
  
  Serial.print ("zero-crossing persistence (sample sets) = ");
  Serial.println (POLARITY_CHECK_MAXCOUNT);

  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on
  configureParamsForSelectedOutputMode(); 
  Serial.println ("----");    
  
  convertTemperature(); // start initial temperature conversion  
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RF
//  rf12_sleep(RF12_SLEEP); <- the RFM12B now stays awake throughout

  convertTemperature(); // start initial temperature conversion    
}

/* None of the workload in loop() is time-critical.  All the processing of 
 * ADC data is done within the ISR.
 */
void loop()             
{ 
//  unsigned long timeNow = millis();
  static byte perSecondTimer = 0;
//  
  // The ISR provides a 50 Hz 'tick' which the main code is free to use.
  if (newMainsCycle)
  {
    newMainsCycle = false;
    perSecondTimer++;
    
    if(perSecondTimer >= CYCLES_PER_SECOND) 
    {       
      perSecondTimer = 0; 
      
      // After a pre-defined period of inactivity, the 4-digit display needs to 
      // close down in readiness for the next's day's data. 
      //
      if (absenceOfDivertedEnergyCount > displayShutdown_inMainsCycles)
      {
        // Clear the accumulators for diverted energy.  These are the "genuine" 
        // accumulators that are used by ISR rather than the copies that are 
        // regularly made available for use by the main code.
        //
        divertedEnergyTotal_Wh = 0;
        divertedEnergyRecent_IEU = 0;
        EDD_isActive = false; // energy diversion detector is now inactive
      }
      
      configureValueForDisplay(); // this timing is not critical so does not need to be in the ISR
    }  
  }
  
  if (datalogEventPending)   
  {
    datalogEventPending= false; 
    tx_data.powerAtSupplyPoint_Watts = copyOf_sumP_atSupplyPoint * powerCal_grid / copyOf_sampleSetsDuringThisDatalogPeriod;
    tx_data.powerAtSupplyPoint_Watts *= -1; // to match the OEM convention (import is =ve; export is -ve)
    tx_data.divertedEnergyTotal_Wh = copyOf_divertedEnergyTotal_Wh;     
    tx_data.Vrms_times100 = (int)(100 * voltageCal * sqrt(copyOf_sum_Vsquared / copyOf_sampleSetsDuringThisDatalogPeriod));
    tx_data.temperature_times100 = readTemperature();
    send_rf_data();    
    
    Serial.print("datalog event: grid power ");  Serial.print(tx_data.powerAtSupplyPoint_Watts);
    Serial.print(", diverted energy (Wh) ");  Serial.print(tx_data.divertedEnergyTotal_Wh);
    Serial.print(", Vrms ");  Serial.print((float)tx_data.Vrms_times100 / 100);
    Serial.print(", temperature ");  Serial.print((float)tx_data.temperature_times100 / 100);
    Serial.print(",  (minSampleSets/MC ");  Serial.print(copyOf_lowestNoOfSampleSetsPerMainsCycle);
    Serial.print(",  #ofSampleSets "); Serial.print(copyOf_sampleSetsDuringThisDatalogPeriod);
    Serial.println(')');
//    delay(POST_DATALOG_EVENT_DELAY_MILLIS);
    convertTemperature(); // for use next time around    
  }  

/*
  // occasional delays should not affect the operation of this revised code structure. 
  if (timeNow - timeAtLastDelay > 1000)
  {
    delay(100);
    Serial.println("100ms delay");
    timeAtLastDelay = timeNow;
  }  
*/
}


ISR(ADC_vect)  
/*
 * This Interrupt Service Routine looks after the acquisition and processing of
 * raw samples from the ADC sub-processor.  By means of various helper functions, all of 
 * the time-critical activities are processed within the ISR.  The main code is notified
 * by means of a flag when fresh copies of loggable data are available.
 */
{                                         
  static unsigned char sample_index = 0;
  int rawSample;
  long sampleIminusDC;
  long  phaseShiftedSampleVminusDC;
  long filtV_div4;
  long filtI_div4;
  long instP;
  long inst_Vsquared;
   
  switch(sample_index)
  {
    case 0:
      rawSample = ADC;                    // store the ADC value (this one is for Voltage)
      ADMUX = 0x40 + currentSensor_diverted;  // the conversion for I_grid is already under way
      sample_index++;                   // increment the control flag
      //
      lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
      sampleVminusDC_long = ((long)rawSample<<8) - DCoffset_V_long; 
      if(sampleVminusDC_long > 0) { 
        polarityNow = POSITIVE; }
      else { 
        polarityNow = NEGATIVE; }
      confirmPolarity();
      //  
      checkProgress(); // deals with aspects that only occur at particular stages of each mains cycle
      //
      // for the Vrms calculation (for datalogging only)
      filtV_div4 = sampleVminusDC_long>>2;  // reduce to 16-bits (now x64, or 2^6)
      inst_Vsquared = filtV_div4 * filtV_div4; // 32-bits (now x4096, or 2^12)
      inst_Vsquared = inst_Vsquared>>12;     // scaling is now x1 (V_ADC x I_ADC)
      sum_Vsquared += inst_Vsquared; // cumulative V^2 (V_ADC x I_ADC)
      sampleSetsDuringThisDatalogPeriod++; 
      //  
      // store items for use during next loop
      cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
//      lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
      polarityConfirmedOfLastSampleV = polarityConfirmed;  // for identification of half cycle boundaries
      sampleSetsDuringThisCycle++;  // for real power calculations
      refreshDisplay();
    break;
    case 1:
      rawSample = ADC;               // store the ADC value (this one is for Grid Current)
      ADMUX = 0x40 + voltageSensor;  // the conversion for I_diverted is already under way
      sample_index++;                   // increment the control flag
      //
      // remove most of the DC offset from the current sample (the precise value does not matter)
      sampleIminusDC = ((long)(rawSample-DCoffset_I))<<8;
      //
      // phase-shift the voltage waveform so that it aligns with the grid current waveform
      phaseShiftedSampleVminusDC = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_grid_int)>>8);  
      //                                                          
      // calculate the "real power" in this sample pair and add to the accumulated sum
      filtV_div4 = phaseShiftedSampleVminusDC>>2;  // reduce to 16-bits (now x64, or 2^6)
      filtI_div4 = sampleIminusDC>>2; // reduce to 16-bits (now x64, or 2^6)
      instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
      instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       

      sumP_forEnergyBucket+=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
      sumP_atSupplyPoint +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
      break;
    case 2:
      rawSample = ADC;               // store the ADC value (this one is for Diverted Current)
      ADMUX = 0x40 + currentSensor_grid;  // the conversion for Voltage is already under way
      sample_index = 0;                   // reset the control flag
      //       
      // remove most of the DC offset from the current sample (the precise value does not matter)
      sampleIminusDC = ((long)(rawSample-DCoffset_I))<<8;
      //
      // phase-shift the voltage waveform so that it aligns with the diverted current waveform
      phaseShiftedSampleVminusDC = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_diverted_int)>>8);  
      //
      // calculate the "real power" in this sample pair and add to the accumulated sum
      filtV_div4 = phaseShiftedSampleVminusDC>>2;  // reduce to 16-bits (now x64, or 2^6)
      filtI_div4 = sampleIminusDC>>2; // reduce to 16-bits (now x64, or 2^6)
      instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
      instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
      sumP_diverted +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
      break;
     default:
      sample_index = 0;                 // to prevent lockup (should never get here)      
  }
}

/* -----------------------------------------------------------
 * Start of various helper functions which are used by the ISR 
 */
 
void checkProgress()
/* 
 * This routine is called by the ISR when each voltage sample becomes available. 
 * At the start of each new mains cycle, another helper function is called.  
 * All other processing is done within this function.
 */
{  
  static enum triacStates nextStateOfTriac = TRIAC_OFF;  

  if (polarityConfirmed == POSITIVE) 
  { 
    if (beyondStartUpPhase)
    {     
      if (polarityConfirmedOfLastSampleV != POSITIVE)
      {
        // The start of a new mains cycle, just after the +ve going zero-crossing point.    
        
        // a simple routine for checking the performance of this new ISR structure     
        if (sampleSetsDuringThisCycle < lowestNoOfSampleSetsPerMainsCycle) {
          lowestNoOfSampleSetsPerMainsCycle = sampleSetsDuringThisCycle; }

        processLatestContribution(); // for activities at the start of each new mains cycle
        
      } // end of processing that is specific to the first +ve Vsample in each mains cycle 
  
      // still processing samples where the voltage is POSITIVE ...
      // check to see whether the trigger device can now be reliably armed
      // 
      if (sampleSetsDuringThisCycle == 5) // part way through the +ve half cycle
      {
        if (energyInBucket_long < lowerEnergyThreshold_long) {
          // when below the lower threshold, always set the triac to "off" 
          nextStateOfTriac = TRIAC_OFF; }
        else
        if (energyInBucket_long > upperEnergyThreshold_long) {
          // when above the upper threshold, always set the triac to "off"
          nextStateOfTriac = TRIAC_ON; }
        else { 
        } // leave the triac's state unchanged (hysteresis)
                                        
        // set the Arduino's output pin accordingly
        digitalWrite(outputForTrigger, nextStateOfTriac);   
      
        // update the Energy Diversion Detector
        if (nextStateOfTriac == TRIAC_ON) {
          absenceOfDivertedEnergyCount = 0; 
          EDD_isActive = true; }            
        else {
          absenceOfDivertedEnergyCount++; }   
      }    
    }
    else
    {  
      // wait until the DC-blocking filters have had time to settle
      if(millis() > (delayBeforeSerialStarts + startUpPeriod) * 1000) 
      {
        beyondStartUpPhase = true;
        sumP_forEnergyBucket = 0;
        sumP_atSupplyPoint;
        sumP_diverted = 0;
        sampleSetsDuringThisCycle = 0;
        sampleSetsDuringThisDatalogPeriod = 0;
      }
    }   
  } // end of processing that is specific to samples where the voltage is positive
  
  else // the polatity of this sample is negative
  {     
    if (polarityConfirmedOfLastSampleV != NEGATIVE)
    {
      // Half way through the mains cycle, just after the -ve going zero-crossing point.      
      // This is a convenient place to update the Low Pass Filter for DC-offset removal.
      //
      long previousOffset = DCoffset_V_long;
      DCoffset_V_long = previousOffset + (cumVdeltasThisCycle_long>>6); // faster than * 0.01
      cumVdeltasThisCycle_long = 0;
      
      // To ensure that the LPF will always start up correctly when 230V AC is available, its
      // output value needs to be prevented from drifting beyond the likely range of the 
      // voltage signal.  This avoids the need for a HPF as was done for initial Mk2 builds.
      //
      if (DCoffset_V_long < DCoffset_V_min) {
        DCoffset_V_long = DCoffset_V_min; }
      else  
      if (DCoffset_V_long > DCoffset_V_max) {
        DCoffset_V_long = DCoffset_V_max; }
        
//      checkOutputModeSelection(); // updates outputMode if the external switch is in use
           
    } // end of processing that is specific to the first Vsample in each -ve half cycle
  } // end of processing that is specific to samples where the voltage is negative
} //  end of checkProgress()

void confirmPolarity()
{
  /* This routine prevents a zero-crossing point from being declared until 
   * a certain number of consecutive samples in the 'other' half of the 
   * waveform have been encountered.  It forms part of the ISR.
   */ 
  static byte count = 0;
  if (polarityNow != polarityConfirmedOfLastSampleV) { 
    count++; }  
  else {
    count = 0; }
    
  if (count >= POLARITY_CHECK_MAXCOUNT)
  {
    count = 0;
    polarityConfirmed = polarityNow;
  }
}


void processLatestContribution()
/* 
 * This routine runs once per mains cycle.  It forms part of the ISR.
 */
{
  newMainsCycle = true; // <--  a 50 Hz 'tick' for use by the main code  

  // For the mechanism which controls the diversion of surplus power, the AVERAGE power 
  // at the 'grid' point during the previous mains cycle must be quantified. The first 
  // stage in this process is for the sum of all instantaneous power values to be divided 
  // by the number of sample sets that have contributed to its value.  A similar operation 
  // is required for the diverted power data. 
  //
  // The next stage would normally be to apply a calibration factor so that real power 
  // can be expressed in Watts.  That's fine for floating point maths, but it's not such
  // a good idea when integer maths is being used.  To keep the numbers large, and also 
  // to save time, calibration of power is omitted at this stage.  Real Power (stored as 
  // a 'long') is therefore (1/powerCal) times larger than the actual power in Watts.
  //
  long realPower_for_energyBucket  = sumP_forEnergyBucket / sampleSetsDuringThisCycle; 
  long realPower_diverted = sumP_diverted / sampleSetsDuringThisCycle; 
  //
  // The per-mainsCycle variables can now be reset for ongoing use 
  sampleSetsDuringThisCycle = 0;
  sumP_forEnergyBucket = 0;
  sumP_diverted = 0;


  // Next, the energy content of this power rating needs to be determined.  Energy is 
  // power multiplied by time, so the next step is normally to multiply the measured
  // value of power by the time over which it was measured.
  //   Average power is calculated once every mains cycle. When integer maths is 
  // being used, a repetitive power-to-energy conversion seems an unnecessary workload.  
  // As all sampling periods are of similar duration, it is more efficient simply to 
  // add all of the power samples together, and note that their sum is actually 
  // CYCLES_PER_SECOND greater than it would otherwise be.
  //   Although the numerical value itself does not change, I thought that a new name 
  // may be helpful so as to minimise confusion.  
  //   The 'energy' variables below are CYCLES_PER_SECOND * (1/powerCal) times larger than 
  // their actual values in Joules.
  //
  long realEnergy_for_energyBucket = realPower_for_energyBucket; 
  long realEnergy_diverted = realPower_diverted; 
          
  // The latest energy contribution from the grid connection point can now be added
  // to the energy bucket which determines the state of the dump-load.  
  //
  energyInBucket_long += realEnergy_for_energyBucket;   
  energyInBucket_long -= requiredExportPerMainsCycle_inIEU; // <- useful for PV simulation
         
  // Apply max and min limits to the bucket's level.  This is to ensure correct operation
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
    // accumulator which operates with maximum precision.  To avoid the displayed
    // value from creeping, any small contributions which are likely to be 
    // caused by noise are ignored.
    //   
    if (realEnergy_diverted > antiCreepLimit_inIEUperMainsCycle) {
      divertedEnergyRecent_IEU += realEnergy_diverted; }
      
    // Whole Watt-Hours are then recorded separately
    if (divertedEnergyRecent_IEU > IEU_per_Wh)
    {
      divertedEnergyRecent_IEU -= IEU_per_Wh;
      divertedEnergyTotal_Wh++;
    }  
  }

  /* At the end of each datalogging period, copies are made of the relevant variables
   * for use by the main code.  These variable are then reset for use during the next 
   * datalogging period.
   */       
  cycleCountForDatalogging ++;       
  if (cycleCountForDatalogging  >= DATALOG_PERIOD_IN_MAINS_CYCLES ) 
  { 
    cycleCountForDatalogging = 0;
    
    copyOf_sumP_atSupplyPoint = sumP_atSupplyPoint;
    copyOf_divertedEnergyTotal_Wh = divertedEnergyTotal_Wh;
    copyOf_sum_Vsquared = sum_Vsquared; 
    copyOf_sampleSetsDuringThisDatalogPeriod = sampleSetsDuringThisDatalogPeriod; // (for diags only)  
    copyOf_lowestNoOfSampleSetsPerMainsCycle = lowestNoOfSampleSetsPerMainsCycle; // (for diags only)
    
    sumP_atSupplyPoint = 0;
    sum_Vsquared = 0;
    lowestNoOfSampleSetsPerMainsCycle = 999;
    sampleSetsDuringThisDatalogPeriod = 0;
    datalogEventPending = true;
  }
} 
/* End of helper functions which are used by the ISR
 * -------------------------------------------------
 */

// this function changes the value of outputMode if the external switch is in use for this purpose 
void checkOutputModeSelection()  
{
  static byte count = 0;
  int pinState; 
  // pinState = digitalRead(outputModeSelectorPin); <- pin re-allocated for Dallas sensor
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
/* 
 * retained for compatibility with previous versions
 */
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

void convertTemperature()
{
  oneWire.reset();
  oneWire.write(SKIP_ROM);
  oneWire.write(CONVERT_TEMPERATURE);
}

int readTemperature()
{
  byte buf[9];
  int result;
  
  oneWire.reset();
  oneWire.write(SKIP_ROM);
  oneWire.write(READ_SCRATCHPAD);
  for(int i=0; i<9; i++) buf[i]=oneWire.read();
  if(oneWire.crc8(buf,8)==buf[8])
  {
    result=(buf[1]<<8)|buf[0];
    // result is temperature x16, multiply by 6.25 to convert to temperature x100
    result=(result*6)+(result>>2);
  }
  else result=BAD_TEMPERATURE;
  return result;
}


void send_rf_data()
//
// To avoid disturbance to the sampling process, the RFM12B needs to remain in its
// active state rather than being periodically put to sleep.
{
  // check whether it's ready to send, and an exit route if it gets stuck 
  int i = 0; 
  while (!rf12_canSend() && i<10)
  { 
    rf12_recvDone(); 
    i++;
  }
  rf12_sendNow(0, &tx_data, sizeof tx_data);
}


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}




