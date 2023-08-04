/* remote_Mk2_receiver_1b.ino
 *
 * This sketch is to control a remote load for a Mk2 PV Router at the receiver end
 * of an RF link.  If RF transmission is lost, the triac is turned off.  A repeater
 * signal is available at the 'mode' connector.  This is intended to drive an LED
 * with an appropriate series resistor, e.g. 120R. 
 *
 * The ability to measure and display the amount of energy which has been diverted 
 * via the remote load is included.  For this to happen, one of the live cores
 * needs to pass through a CT which connects to the 'CT2' connector.  
 *
 * The 'CT1' connector has been re-used in this sketch to provide a 2-colour 
 * indication of the state of the RF link.  A schematic for this circuit may be 
 * found immediately below this header.
 *
 * A persistence-based 4-digit display is supported. When the RFM12B module is 
 * in use, the display can only be used in conjunction with an extra pair of 
 * logic chips.  These are ICs 3 and 4, which reduce the number of processor pins 
 * that are needed to drive the display.
 *
 * This sketch is similar in function to RF_for_Mk2_rx.ino, as posted on the 
 * OpenEnergyMonitor forum.  That version, and other related material, can be
 * found on my Summary Page at www.openenergymonitor.org/emon/node/1757
 *
 * January 2016: renamed as remote_Mk2_receiver_1a, with a minor change in the ISR to 
 * remove a timing uncertainty.  Support for the RF69 RF module has also been included.
 *
 * January 2016: updated to remote_Mk2_receiver_1b:
 *   The variables to store the ADC results are now declared as "volatile" to remove 
 *   any possibility of incorrect operation due to optimisation by the compiler.
 *
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 */
 
 /*******************************************************
  suggested circuit for the bi-colour RF-status indicator
  
      ------------------> +3.3V
        |
       ---
       \ /  Red LED (to show when the RF link is faulty)
       ---   
        |
        /
        \  120R
        /
        |
        |--------> CT1 (the lower pin of the two,
        |             not Vref which is the upper pin)
        |
        /
        \  120R
        /
        |
       ---
       \ /  Green LED (to show when the RF link is OK)
       ---   
        |
      -----------------> GND 
 
*******************************************************
*/
  
#define RF69_COMPAT 0  // <-- include this line for the RFM12B
// #define RF69_COMPAT 1  // <-- include this line for the RF69

#include <Arduino.h> 
#include <JeeLib.h>     // JeeLib is available at from: http://github.com/jcw/jeelib
#include <TimerOne.h>

#define ADC_TIMER_PERIOD 200 // uS (determines the sampling rate / amount of idle time)

// Physical constants, please do not change!
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define JOULES_PER_WATT_HOUR 3600 //  (0.001 kWh = 3600 Joules)

// Change this values to suit the local mains frequency
#define CYCLES_PER_SECOND 50 

// to prevent the diverted energy total from 'creeping'
#define ANTI_CREEP_LIMIT 5 // in Joules per mains cycle (has no effect when set to 0)
long antiCreepLimit_inIEUperMainsCycle;

// definition of enumerated types
enum polarities {NEGATIVE, POSITIVE};
//enum triacStates {TRIAC_ON, TRIAC_OFF}; // the external trigger device is active low
enum outputModes {ANTI_FLICKER, NORMAL};

enum loadStates {LOAD_ON, LOAD_OFF}; // to match Tx protocol, the load is active low ...
enum loadStates loadState;

enum transmissionStates {RF_FAULT, RF_IS_OK}; // two LEDs are driven from one o/p pin
enum transmissionStates transmissionState;

/* frequency options are RF12_433MHZ, RF12_868MHZ or RF12_915MHZ
 */
#define freq RF12_433MHZ // Use the freq to match the module you have.

const int TXnodeID = 10;
const int myNode = 15;
const int networkGroup = 210; 
const int UNO = 1; // Set to 0 if you're not using the UNO bootloader 

//  define the data structure for RF comms
typedef struct 
{ byte dumpState;
  int msgNumber;
} Rx_struct;    
Rx_struct receivedData; // an instance of this type

unsigned long timeAtLastMessage = 0;
int lastMsgNumber = 0;
unsigned long timeAtLastTransmissionLostDisplay;

// allocation of digital pins when pin-saving hardware is in use
// *************************************************************
// D0 & D1 are reserved for the Serial i/f
// D2 is for the RFM12B
const byte loadIndicator_LED = 3; // <-- active high 
const byte outputForTrigger = 4; // <- active low
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
const byte transmissionStatusPin = 19;  // A5 is to control a pair of red & green LEDs


const byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;    // nominal mid-point value of ADC @ x1 scale

// General global variables that are used in multiple blocks so cannot be static.
// For integer maths, many variables need to be 'long'
//
boolean beyondStartUpPhase = false;     // start-up delay, allows things to settle
long cycleCount = 0;                    // counts mains cycles from start-up 
long energyInBucket_long;          // in Integer Energy Units
long capacityOfEnergyBucket_long;  // depends on powerCal, frequency & the 'sweetzone' size.
long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF
long divertedEnergyRecent_IEU = 0; // Hi-res accumulator of limited range
unsigned int divertedEnergyTotal_Wh = 0; // WattHour register of 63K range
long IEU_per_Wh; // depends on powerCal, frequency & the 'sweetzone' size.

unsigned long displayShutdown_inMainsCycles; 
unsigned long absenceOfDivertedEnergyCount = 0;
long mainsCyclesPerHour;

// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
volatile int sampleI_diverted;
volatile int sampleV;


// Calibration
//------------
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
// The ADC has an input range of 3.3V and an output range of 1023 levels.
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
// has a peak-to-peak range of 35A.  This is numerically smaller than the 
// output signal by around a factor of twenty.  The conversion rate of the
// overall system for measuring CURRENT is therefore likely to be around 
// 20 ADC-steps per Amp.
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

// a tidy means of identifying the DP status data when accessing the above array
const byte DPstatus_columnID = noOfDigitSelectionLines; 

byte digitLocationMap[noOfDigitLocations][noOfDigitLocationLines] = {
                 LOW , LOW ,    // Digit 1
                 LOW , HIGH,    // Digit 2
                 HIGH, LOW ,    // Digit 3
                 HIGH, HIGH,    // Digit 4
};


byte charsForDisplay[noOfDigitLocations] = {20,20,20,20}; // all blank 

boolean EDD_isActive = false; // Energy Diversion Detection


void setup()
{  
  pinMode(outputForTrigger, OUTPUT);  
  digitalWrite (outputForTrigger, LOAD_OFF); // the external trigger is active low

  pinMode(loadIndicator_LED, OUTPUT);  
//  digitalWrite (outputForTrigger, LOAD_OFF); // the external trigger is active low
  
  pinMode(transmissionStatusPin, OUTPUT);  
//  digitalWrite (outputForTrigger, TRIAC_OFF); // the external trigger is active low

  delay(5000); // allow time to open Serial monitor     
 
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      remote_Mk2_receiver_1b.ino");
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
  
  
  // When using integer maths, the energy measurement scale is altered to match the
  // energy detection mechanism that is in use.  This avoids the need to re-scale 
  // every energy contribution, thus saving processing time.  This process is 
  // described in more detail in the function, allGeneralProcessing(), at the start 
  // of each new mains cycle.
  //
  // Diverted energy data, as measured using CT2, is stored in an 'integer maths' 
  // accumulator.  Whenever its value exceeds 1 Wh, an associated WattHour register 
  // is incremented, and the accumulator's value is decremented accordingly. The 
  // calculation below is to determine the correct scaling for this accumulator.
  
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
       
//  Serial.print ( "powerCal_grid =      "); Serial.println (powerCal_grid,4);
  Serial.print ( "powerCal_diverted = "); Serial.println (powerCal_diverted,4);
  
  Serial.print ("Anti-creep limit (Joules / mains cycle) = ");
  Serial.println (ANTI_CREEP_LIMIT);
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on

  Serial.println ("----");    

  delay(1000);
//  rf12_set_cs(10); //emonTx, emonGLCD, NanodeRF, JeeNode

  rf12_initialize(myNode, freq, networkGroup);                      
}

// An Interrupt Service Routine is now defined in which the ADC is instructed to 
// measure V and I alternately.  A "data ready" flag is set after each voltage conversion 
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
  static int sampleI_diverted_raw;

  switch(sample_index)
  {
    case 0:
      sampleV = ADC;                    // store the ADC value (this one is for Voltage)
      ADMUX = 0x40 + currentSensor_diverted;  // set up the next conversion, which is for Diverted Current
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index++;                   // increment the control flag
      sampleI_diverted = sampleI_diverted_raw;
      dataReady = true;                 // both ADC values can now be processed
      break;
    case 1:
      sampleI_diverted_raw = ADC;               // store the ADC value (this one is for Diverted Current)
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
//
// If there is insufficient processing capacity to do all that is required, the 
// base workload can be reduced by increasing the duration of ADC_TIMER_PERIOD.
//
void loop()             
{ 
  if (dataReady)   // flag is set after every pair of ADC conversions
  {
    dataReady = false; // reset the flag
    allGeneralProcessing(); // executed once for each pair of V&I samples
  }  
} // end of loop()


// This routine is called to process each set of V & I samples. The main processor and 
// the ADC work autonomously, their operation being only linked via the dataReady flag.  
// As soon as a new set of data is made available by the ADC, the main processor can 
// start to work on it immediately.  
//
void allGeneralProcessing()
{
  static int samplesDuringThisCycle;             // for normalising the power in each mains cycle
  static long sumP_diverted;                              // for per-cycle summation of 'real power' 
  static enum polarities polarityOfLastSampleV;  // for zero-crossing detection
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static byte perSecondCounter = 0;
  
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
        
        // update the Energy Diversion Detector which is determined by the 
        // state of the remote load, as instruction via the RF link
        //
        if (loadState == LOAD_ON) {
          absenceOfDivertedEnergyCount = 0; 
          EDD_isActive = true; }            
        else {
          absenceOfDivertedEnergyCount++; }   
    
        if (EDD_isActive) // Energy Diversion Display (EDD)
        {
          // In this sketch, energy contributions need only be processed if EDD is active.
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
          long realEnergy_diverted = realPower_diverted; 
        
          // to avoid 'creep', small energy contributions are ignored
          if (realEnergy_diverted < antiCreepLimit_inIEUperMainsCycle) {
              realEnergy_diverted = 0; } 

          // The latest energy contribution needs to be added to an accumulator which operates 
          // with maximum precision. 
          divertedEnergyRecent_IEU += realEnergy_diverted;
      
          // Whole kWhours are then recorded separately
          if (divertedEnergyRecent_IEU > IEU_per_Wh)
          {
            divertedEnergyRecent_IEU -= IEU_per_Wh;
            divertedEnergyTotal_Wh++;
         }  
        }
        
              
        // the data to be displayed is configured every second
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
          
/*
          Serial.print("Diverted: " );
          Serial.print(divertedEnergyTotal_Wh);
          Serial.print(" Wh plus ");
          Serial.print((powerCal_diverted / CYCLES_PER_SECOND) * divertedEnergyRecent_IEU);
          Serial.print("J, EDD is" );
          if (EDD_isActive) {
            Serial.println(" on" ); }
          else {
            Serial.println(" off" ); }        
*/       
          configureValueForDisplay(); // occurs every second
        }
        
        // clear the per-cycle accumulators for use in this new mains cycle.  
        samplesDuringThisCycle = 0;
        sumP_diverted = 0;

      } // end of processing that is specific to the first Vsample in each +ve half cycle 
    }
    else
    {  
      // wait until the DC-blocking filters have had time to settle
      if(millis() > startUpPeriod * 1000) 
      {
        beyondStartUpPhase = true;
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
        
    } // end of processing that is specific to the first Vsample in each -ve half cycle
  } // end of processing that is specific to samples where the voltage is negative
  
  // processing for EVERY pair of samples
  //

  // Now deal with the diverted power (as measured via CT2)
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_diverted = ((long)(sampleI_diverted-DCoffset_I))<<8;
                                                               
  // calculate the "real power" in this sample pair and add to the accumulated sum
  long filtV_div4 = sampleVminusDC_long>>2;  // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4 = sampleIminusDC_diverted>>2; // reduce to 16-bits (now x64, or 2^6)
  long instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
  instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_diverted +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  samplesDuringThisCycle++;
  
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  polarityOfLastSampleV = polarityNow;  // for identification of half cycle boundaries


  // Every time that this function is run, a check is performed to find out
  // whether any new RF instructions have been received. This occurs every 400 uS.
  //
  unsigned long timeNow = millis(); // to detect when the RF-link has failed
  
  if (rf12_recvDone())
  {      
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)
    {
      int node_id = (rf12_hdr & 0x1F);
      byte n = rf12_len;
    
      if (node_id == TXnodeID)
      {
        receivedData = *(Rx_struct*) rf12_data;
        loadState = (enum loadStates)receivedData.dumpState;
        
        // process load-state data        
	digitalWrite(outputForTrigger, loadState); // active low, same as Tx protocol
	digitalWrite(loadIndicator_LED, !loadState); // active high

        // process message number data        
        byte msgNumber = receivedData.msgNumber;
        if ((msgNumber != lastMsgNumber + 1) ||
            ((msgNumber == 0) && (lastMsgNumber != 255)))
        {
          Serial.println("Message numbering error!"); 
        }
//
        Serial.print(msgNumber); 
        Serial.print(", ");
        Serial.println(loadState); 
//        
        timeAtLastMessage = timeNow;
        lastMsgNumber = msgNumber;
        
      }
    }	
    else
    {
      Serial.println("Corrupt message!");
    }
  }
   
  if ((timeNow - timeAtLastMessage) > 3500)
  {
    // transmission has been lost
    transmissionState = RF_FAULT;
    loadState = LOAD_OFF;   
    digitalWrite(outputForTrigger, loadState); 
    digitalWrite(loadIndicator_LED, !loadState); 
     
    if(timeNow > timeAtLastTransmissionLostDisplay + 1000)
    {
      Serial.println("transmission lost!");
      timeAtLastTransmissionLostDisplay = timeNow;
    }
  }
  else
  {
    transmissionState = RF_IS_OK;
  }

  digitalWrite(transmissionStatusPin, transmissionState);    
  refreshDisplay();
  
} // end of allGeneralProcessing()


// called every second, to update the characters to be displayed
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
      // a re-scaling is necessary (display to 2 DPs)
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
  //
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




