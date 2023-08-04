/* Mk2_3phase_RFdatalog_2.ino
 *
 * Issue 1 was released in January 2015.
 *
 * This sketch provides continuous monitoring of real power on three phases. 
 * Surplus power is diverted to multiple loads in sequential order.
 * Datalogging of real power and Vrms is provided for each phase.
 * The presence or absence of the RFM12B needs to be set at compile time
 *
 * Jan 2016, renamed as Mk2_3phase_RFdatalog_2 with these changes:
 * - Improved control of multiple loads has been imported from the 
 *     equivalent 1-phase sketch, Mk2_multiLoad_wired_6.ino
 * - the ISR has been upgraded to fix a possible timing anomaly
 * - variables to store ADC samples are now declared as "volatile"
 * - for RF69 RF module is now supported
 * - a performance check has been added with the result being sent to the Serial port 
 * - control signals for loads are now active-high to suit the latest 3-phase PCB
 *
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 */

#include <Arduino.h> // may not be needed, but it's probably a good idea to include this

// #define RF_PRESENT // <- this line should be commented out if the RFM12B module is not present

#ifdef RF_PRESENT
//#define RF69_COMPAT 0 // for the RFM12B
#define RF69_COMPAT 1 // for the RF69
#include <JeeLib.h>     
#endif

// In this sketch, the ADC is free-running with a cycle time of ~104uS.

//  WORKLOAD_CHECK is available for determining how much spare processing time there 
//  is.  To activate this mode, the #define line below should be included: 
//#define WORKLOAD_CHECK  

#define CYCLES_PER_SECOND 50
//#define JOULES_PER_WATT_HOUR 3600 // may be needed for datalogging
#define WORKING_ZONE_IN_JOULES 3600
#define REQUIRED_EXPORT_IN_WATTS 0 // when set to a negative value, this acts as a PV generator 
#define NO_OF_PHASES 3
#define DATALOG_PERIOD_IN_SECONDS 5

const byte noOfDumploads = 3; 

enum polarities {NEGATIVE, POSITIVE};
enum outputModes {ANTI_FLICKER, NORMAL};
enum loadPriorityModes {LOAD_1_HAS_PRIORITY, LOAD_0_HAS_PRIORITY};

// enum loadStates {LOAD_ON, LOAD_OFF}; // for use if loads are active low (original PCB)
enum loadStates {LOAD_OFF, LOAD_ON}; // for use if loads are active high (Rev 2 PCB)
enum loadStates logicalLoadState[noOfDumploads]; 
enum loadStates physicalLoadState[noOfDumploads]; 

// For this multi-load version, the same mechanism has been retained but the 
// output mode is hard-coded as below:
enum outputModes outputMode = ANTI_FLICKER;    

// In this multi-load version, the external switch is re-used to determine the load priority
enum loadPriorityModes loadPriorityMode = LOAD_0_HAS_PRIORITY;                                                   

#ifdef RF_PRESENT
#define freq RF12_433MHZ // Use the freq to match the module you have.

const int nodeID = 10;                                          
const int networkGroup = 210;                        
const int UNO = 1;
#endif
                            
typedef struct { 
  int power_L1;
  int power_L2;
  int power_L3; 
  int Vrms_L1;
  int Vrms_L2;
  int Vrms_L3;} Tx_struct;    // revised data for RF comms
Tx_struct tx_data;


// ----------- Pinout assignments  -----------
//
// digital pins:
const byte loadPrioritySelectorPin = 3; // // for 3-phase PCB  
// D4 is not in use
const byte physicalLoad_0_pin = 5; // for 3-phase PCB, Load #1 (Rev 2 PCB)
const byte physicalLoad_1_pin = 6; // for 3-phase PCB, Load #2 (Rev 2 PCB)
const byte physicalLoad_2_pin = 7; // for 3-phase PCB, Load #3 (Rev 2 PCB) 
// D8 is not in use
// D9 is not in use

// analogue input pins 
const byte sensorV[NO_OF_PHASES] = {0,2,4}; // for 3-phase PCB
const byte sensorI[NO_OF_PHASES] = {1,3,5}; // for 3-phase PCB 


// --------------  general global variables -----------------
//
// Some of these variables are used in multiple blocks so cannot be static.
// For integer maths, some variables need to be 'long'
//
boolean beyondStartUpPeriod = false;    // start-up delay, allows things to settle
byte initialDelay = 3;  // in seconds, to allow time to open the Serial monitor
byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle

long DCoffset_V_long[NO_OF_PHASES];              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF (min limit)
long DCoffset_V_max;               // <--- for LPF (max limit)
int DCoffset_I_nom;               // nominal mid-point value of ADC @ x1 scale

// for 3-phase use, with units of Joules * CYCLES_PER_SECOND
float capacityOfEnergyBucket_main; 
float energyInBucket_main; 
float midPointOfEnergyBucket_main;
float lowerThreshold_default;  
float lowerEnergyThreshold;
float upperThreshold_default;
float upperEnergyThreshold;
float offsetOfEnergyThresholdsInAFmode = 0.1; // <-- must not exceeed 0.4

// for improved control of multiple loads
boolean recentTransition = false;
byte postTransitionCount;
#define POST_TRANSITION_MAX_COUNT 3 // <-- allows each transition to take effect
//#define POST_TRANSITION_MAX_COUNT 50 // <-- for testing only
byte activeLoad = 0;

// for datalogging
int datalogCountInMainsCycles;
const int maxDatalogCountInMainsCycles = DATALOG_PERIOD_IN_SECONDS * CYCLES_PER_SECOND;
float energyStateOfPhase[NO_OF_PHASES]; // only used for datalogging

// for interaction between the main processor and the ISR 
volatile boolean dataReady = false;
volatile int sampleV[NO_OF_PHASES];
volatile int sampleI[NO_OF_PHASES];

// For a mechanism to check the continuity of the sampling sequence
#define CONTINUITY_CHECK_MAXCOUNT 250 // mains cycles
int mainsCycles_forContinuityChecker;
int lowestNoOfSampleSetsPerMainsCycle;

// Calibration values
//-------------------
// Three calibration values are used in this sketch: powerCal, phaseCal and voltageCal. 
// With most hardware, the default values are likely to work fine without 
// need for change.  A compact explanation of each of these values now follows:

// When calculating real power, which is what this code does, the individual 
// conversion rates for voltage and current are not of importance.  It is 
// only the conversion rate for POWER which is important.  This is the 
// product of the individual conversion rates for voltage and current.  It 
// therefore has the units of ADC-steps squared per Watt.  Most systems will
// have a power conversion rate of around 20 (ADC-steps squared per Watt).
// 
// powerCal is the RECIPR0CAL of the power conversion rate.  A good value 
// to start with is therefore 1/20 = 0.05 (Watts per ADC-step squared)
//
const float powerCal[NO_OF_PHASES] = {0.043, 0.043, 0.043};
                        
// phaseCal is used to alter the phase of the voltage waveform relative to the
// current waveform.  The algorithm interpolates between the most recent pair
// of voltage samples according to the value of phaseCal. 
//
//    With phaseCal = 1, the most recent sample is used.  
//    With phaseCal = 0, the previous sample is used
//    With phaseCal = 0.5, the mid-point (average) value in used
//
// NB. Any tool which determines the optimal value of phaseCal must have a similar 
// scheme for taking sample values as does this sketch.
//
const float  phaseCal[NO_OF_PHASES] = {0.5, 0.5, 0.5}; // <- nominal values only
int phaseCal_int[NO_OF_PHASES];           // to avoid the need for floating-point maths

// For datalogging purposes, voltageCal has been added too.  Because the range of ADC values is 
// similar to the actual range of volts, the optimal value for this cal factor is likely to be
// close to unity. 
const float voltageCal[NO_OF_PHASES] = {1.03, 1.03, 1.03}; // compared with Fluke 77 meter




void setup()
{  
  delay (initialDelay * 1000); // allows time to open the Serial Monitor
  
  Serial.begin(9600);   // initialize Serial interface
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("----------------------------------");
  Serial.println("Sketch ID:  Mk2_3phase_RFdatalog_2.ino");
      
  pinMode(physicalLoad_0_pin, OUTPUT); // driver pin for Load #1
  pinMode(physicalLoad_1_pin, OUTPUT); // driver pin for Load #2
  pinMode(physicalLoad_2_pin, OUTPUT); // driver pin for Load #3

  for(int i = 0; i< noOfDumploads; i++)
  {
    logicalLoadState[i] = LOAD_OFF;
  } 
  updatePhysicalLoadStates(); // allows the logical-to-physical mapping to be changed
  
  digitalWrite(physicalLoad_0_pin, physicalLoadState[0]); // update the local load's state.      
  digitalWrite(physicalLoad_1_pin, physicalLoadState[1]); // update the additional load state (inverse logic).      
  digitalWrite(physicalLoad_2_pin, physicalLoadState[2]); // update the additional load state (inverse logic).      

  pinMode(loadPrioritySelectorPin, INPUT); 
  digitalWrite(loadPrioritySelectorPin, HIGH); // enable the internal pullup resistor
  delay (100); // allow time to settle
  int pinState = digitalRead(loadPrioritySelectorPin);  // initial selection and
  loadPriorityMode = (enum loadPriorityModes)pinState;  //   assignment of priority mode
        
  for (byte phase = 0; phase < NO_OF_PHASES; phase++)
  {
    // When using integer maths, calibration values that have been supplied in  
    // floating point form need to be rescaled.  
    phaseCal_int[phase] = phaseCal[phase] * 256;  // for integer maths
    DCoffset_V_long[phase] = 512L * 256;  // nominal mid-point value of ADC @ x256 scale  
  }
     
  // Define operating limits for the LP filters which identify DC offset in the voltage 
  // sample streams.  By limiting the output range, these filters always should start up 
  // correctly.
  DCoffset_V_min = (long)(512L - 100) * 256; // mid-point of ADC minus a working margin
  DCoffset_V_max = (long)(512L + 100) * 256; // mid-point of ADC plus a working margin
  DCoffset_I_nom = 512;        // nominal mid-point value of ADC @ x1 scale

  // for the main energy bucket
  capacityOfEnergyBucket_main = (float)WORKING_ZONE_IN_JOULES * CYCLES_PER_SECOND;
  midPointOfEnergyBucket_main = capacityOfEnergyBucket_main * 0.5; // for resetting flexible thresholds
  energyInBucket_main = 0; 
    
  Serial.println ("ADC mode:       free-running");  
  Serial.print ("requiredExport in Watts = ");
  Serial.println (REQUIRED_EXPORT_IN_WATTS);
  
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

     
  char flag = 0;
  Serial.print ( "Extra Features: ");  
#ifdef WORKLOAD_CHECK  
  Serial.print ( "WORKLOAD_CHECK ");
  flag++;
#endif
  if (flag == 0) {
    Serial.print ("none"); }
  Serial.println ();
   
  for (byte phase = 0; phase < NO_OF_PHASES; phase++)
  {  
    Serial.print ( "powerCal for L"); Serial.print(phase +1); 
      Serial.print (" =    "); Serial.println (powerCal[phase],4);
    Serial.print ( "phaseCal for L"); Serial.print(phase +1); 
      Serial.print (" =     "); Serial.println (phaseCal[phase]);
    Serial.print ( "voltageCal for L"); Serial.print(phase +1); 
      Serial.print (" =    "); Serial.println (voltageCal[phase],3);
  }
  
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
 
   configureParamsForSelectedOutputMode(); 

   Serial.print ("loadPriority: ");
   if (loadPriorityMode == LOAD_0_HAS_PRIORITY) {
     Serial.println ( "load 0"); }
   else {  
     Serial.println ( "load 1"); }

   Serial.println();
   Serial.print ("free RAM = ");
   Serial.println (freeRam());
   
   Serial.print ("RF capability ");
   
#ifdef RF_PRESENT
   Serial.print ("IS present, freq = ");
   if (freq == RF12_433MHZ) { Serial.println ("433 MHz"); }
   if (freq == RF12_868MHZ) { Serial.println ("868 MHz"); }
  rf12_initialize(nodeID, freq, networkGroup);          // initialize RF
#else
   Serial.println ("is NOT present");
#endif
  
}

// An Interrupt Service Routine is now defined which instructs the ADC to perform a conversion 
// for each of the voltage and current sensors in turn.  A "data ready" flag is set after 
// each set of converstions has been completed.  
//   This Interrupt Service Routine is for use when the ADC is in the free-running mode.
// It is executed whenever an ADC conversion has finished, approx every 104 us.  In 
// free-running mode, the ADC has already started its next conversion by the time that
// the ISR is executed.  The ISR therefore needs to "look ahead". 
//   At the end of conversion Type N, conversion Type N+1 will start automatically.  The ISR 
// which runs at this point therefore needs to capture the results of conversion Type N, 
// and set up the conditions for conversion Type N+2, and so on.  
// 
ISR(ADC_vect)  
{                                         
  static unsigned char sample_index = 0;
  static int sample_V0_raw;
  static int sample_V1_raw;
  static int sample_I0_raw;
  static int sample_I1_raw;
  static int sample_I2_raw;
  
  switch(sample_index)
  {
    case 0:
      sample_I0_raw = ADC; 
      ADMUX = 0x40 + sensorI[1]; // set up the next-but-one conversion
      sample_index++; // advance the control flag             
      break;
    case 1:
      sample_V0_raw = ADC; 
      ADMUX = 0x40 + sensorV[1]; // for the next-but-one conversion
      sample_index++; // advance the control flag                
      break;
    case 2:
      sample_I1_raw = ADC; 
      ADMUX = 0x40 + sensorI[2]; // for the next-but-one conversion
      sample_index++; // advance the control flag                
      break;
    case 3:
      sample_V1_raw = ADC; 
      ADMUX = 0x40 + sensorV[2]; // for the next-but-one conversion
      sample_index++; // advance the control flag                 
      break;
    case 4:
      sample_I2_raw = ADC; 
      ADMUX = 0x40 + sensorI[0]; // for the next-but-one conversion
      sample_index++; // advance the control flag                 
      break;
    case 5:
      sampleV[2] = ADC; 
      ADMUX = 0x40 + sensorV[0]; // for the next-but-one conversion
      sample_index = 0; // reset the control flag                
      sampleV[0] = sample_V0_raw;
      sampleV[1] = sample_V1_raw;
      sampleI[0] = sample_I0_raw;
      sampleI[1] = sample_I1_raw;
      sampleI[2] = sample_I2_raw;
      dataReady = true; 
      break;
    default:
      sample_index = 0;                 // to prevent lockup (should never get here)      
  }  
}


// The main processor waits in loop() until the DataReady flag has been set by the ADC.  
// Once this flag has been set, the main processor clears the flag and proceeds with 
// the processing for a complete set of 3 pairs of V & I samples.  It then returns to 
// loop() to wait for the next set to become available.
//   If the next set of samples become available before the processing of the previous set 
// has been completed, data could be lost.  This situation can be avoided by prior use of 
// the WORKLOAD_CHECK mode.  Using this facility, the amount of spare processing capacity 
// per 6-sample set can be determined.  
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
    processRawSamples(); // executed once for each pair of V&I samples
    
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


// This routine is called to process each set of V & I samples (3 pairs).  The main processor and 
// the ADC work autonomously, their operation being synchnonised only via the dataReady flag.  
//
void processRawSamples()
{
  static long sumP[NO_OF_PHASES];                         
  static enum polarities polarityOfLastSampleV[NO_OF_PHASES];  // for zero-crossing detection
  static long lastSampleV_minusDC_long[NO_OF_PHASES];     //    for the phaseCal algorithm
  static long cumVdeltasThisCycle_long[NO_OF_PHASES];    // for the LPF which determines DC offset (voltage)
  static int samplesDuringThisMainsCycle[NO_OF_PHASES];            
  static long sum_Vsquared[NO_OF_PHASES];                         
  static long samplesDuringThisDatalogPeriod;
  enum polarities polarityNow;  
  
  // The raw V and I samples are processed in "phase pairs"
  for (byte phase = 0; phase < NO_OF_PHASES; phase++)
  {
    // remove DC offset from each raw voltage sample by subtracting the accurate value 
    // as determined by its associated LP filter.
    long sampleV_minusDC_long = ((long)sampleV[phase]<<8) - DCoffset_V_long[phase]; 

    // determine polarity, to aid the logical flow  
    if(sampleV_minusDC_long > 0) { 
      polarityNow = POSITIVE; }
    else { 
      polarityNow = NEGATIVE; }

    if (polarityNow == POSITIVE) 
    {                           
      if (beyondStartUpPeriod)
      {  
        if (polarityOfLastSampleV[phase] != POSITIVE)
        {
          // This is the start of a new +ve half cycle, for this phase, just after the 
          // zero-crossing point.  Before the contribution from this phase can be added 
          // to the running total, the cal factor for this phase must be applied. 
          //
          float realPower = (sumP[phase] / samplesDuringThisMainsCycle[phase]) * powerCal[phase];
          
          processLatestContribution(phase, realPower); // runs at 6.6 ms intervals        
         
          // A performance check to monitor and display the minimum number of sets of
          // ADC samples per mains cycle, the expected number being 20ms / (104us * 6) = 32.05
          //
          if (phase == 0)
          {
            if (samplesDuringThisMainsCycle[phase] < lowestNoOfSampleSetsPerMainsCycle)
            {
              lowestNoOfSampleSetsPerMainsCycle = samplesDuringThisMainsCycle[phase];
            }
            mainsCycles_forContinuityChecker++;
            if (mainsCycles_forContinuityChecker >= CONTINUITY_CHECK_MAXCOUNT)
            {
              mainsCycles_forContinuityChecker = 0;
              Serial.println(lowestNoOfSampleSetsPerMainsCycle);
              lowestNoOfSampleSetsPerMainsCycle = 999;
            }              
          }
         
          sumP[phase] = 0;
          samplesDuringThisMainsCycle[phase] = 0;
          
        } // end of processing that is specific to the first Vsample in each +ve half cycle   
       
        // still processing samples where the voltage is POSITIVE ...
        if ((phase == 0) && samplesDuringThisMainsCycle[0] == 2) // lower value for larger sample set 
        {
          // This code is executed once per 20mS, shortly after the start of each new
          // mains cycle on phase 0.
          //
          datalogCountInMainsCycles++;

          // Changling the state of the loads  is is a 3-part process:
          // - change the LOGICAL load states as necessary to maintain the energy level
          // - update the PHYSICAL load states according to the logical -> physical mapping 
          // - update the driver lines for each of the loads.
          //          
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
  
          if (energyInBucket_main > midPointOfEnergyBucket_main)
          {  
            // the energy state is in the upper half of the working range
            lowerEnergyThreshold = lowerThreshold_default; // reset the "opposite" threshold 
            if (energyInBucket_main > upperEnergyThreshold) 
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
                  if (energyInBucket_main > upperEnergyThreshold)
                  {
                    upperEnergyThreshold = energyInBucket_main; 
                  
                    // the energy thresholds must remain within range
                    if (upperEnergyThreshold > capacityOfEnergyBucket_main)
                    {
                      upperEnergyThreshold = capacityOfEnergyBucket_main;
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
                  Serial.print('+');
                  Serial.println(activeLoad);
                }
              }
            }
          }
          else
          { // the energy state is in the lower half of the working range
            upperEnergyThreshold = upperThreshold_default; // reset the "opposite" threshold 
            if (energyInBucket_main < lowerEnergyThreshold)
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
                  if (energyInBucket_main < lowerEnergyThreshold)
                  {
                    lowerEnergyThreshold = energyInBucket_main; 
                  
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
                  Serial.print('-');
                  Serial.println(activeLoad);
                }
              }
            }
          }  

          updatePhysicalLoadStates(); // allows the logical-to-physical mapping to be changed
               
          // update the control ports for each of the physical loads
          digitalWrite(physicalLoad_0_pin, physicalLoadState[0]); // active low for trigger    
          digitalWrite(physicalLoad_1_pin, physicalLoadState[1]); // active low for trigger     
          digitalWrite(physicalLoad_2_pin, physicalLoadState[2]); // active low for trigger    
          
          // Now that the energy-related decisions have been taken, min and max limits can now
          // be applied  to the level of the energy bucket.  This is to ensure correct operation
          // when conditions change, i.e. when import changes to export, and vici versa.
          //
          if (energyInBucket_main > capacityOfEnergyBucket_main) { 
            energyInBucket_main = capacityOfEnergyBucket_main; } 
          else         
          if (energyInBucket_main < 0) {
            energyInBucket_main = 0; }            
          
          if (datalogCountInMainsCycles >= maxDatalogCountInMainsCycles)
          {
            datalogCountInMainsCycles = 0;
            
            tx_data.power_L1 = energyStateOfPhase[0] / maxDatalogCountInMainsCycles;
            tx_data.power_L2 = energyStateOfPhase[1] / maxDatalogCountInMainsCycles;
            tx_data.power_L3 = energyStateOfPhase[2] / maxDatalogCountInMainsCycles;
            tx_data.Vrms_L1 = (int)(voltageCal[0] * sqrt(sum_Vsquared[0] / samplesDuringThisDatalogPeriod));
            tx_data.Vrms_L2 = (int)(voltageCal[1] * sqrt(sum_Vsquared[1] / samplesDuringThisDatalogPeriod));
            tx_data.Vrms_L3 = (int)(voltageCal[2] * sqrt(sum_Vsquared[2] / samplesDuringThisDatalogPeriod));
#ifdef RF_PRESENT
            send_rf_data();         
#endif            
/*     <-- Warning - Unlike its 1-phase equivalent, this 3-phase code can be affected by Serial statements!
            Serial.print(energyInBucket_main / CYCLES_PER_SECOND);   
            Serial.print(", ");
//            
            Serial.print(tx_data.power_L1);
            Serial.print(", ");
            Serial.print(tx_data.power_L2);
            Serial.print(", ");
            Serial.print(tx_data.power_L3);
            Serial.print(", ");
            Serial.print(tx_data.Vrms_L1);
            Serial.print(", ");
            Serial.print(tx_data.Vrms_L2);
            Serial.print(", ");
            Serial.println(tx_data.Vrms_L3);
*/           
            energyStateOfPhase[0] = 0;
            energyStateOfPhase[1] = 0;
            energyStateOfPhase[2] = 0;   
            sum_Vsquared[0] = 0;
            sum_Vsquared[1] = 0;
            sum_Vsquared[2] = 0;
            samplesDuringThisDatalogPeriod = 0;  

     
/*     <-- Warning - Unlike its 1-phase equivalent, this 3-phase code can be affected by Serial statements!
            for (int i = 0; i < noOfDumploads; i++)
            {            
              Serial.print(logicalLoadState[i]);
            } 
            Serial.println();
*/            
          }
        }
      }
      else
      {  
        // wait until the DC-blocking filters have had time to settle
        if(millis() > (initialDelay + startUpPeriod) * 1000) 
        {
          beyondStartUpPeriod = true;
          mainsCycles_forContinuityChecker = 0;
          lowestNoOfSampleSetsPerMainsCycle = 999;
          Serial.println ("Go!");
        }
      }
    } // end of processing that is specific to samples where the voltage is positive
  
    else // the polarity of this sample is negative
    {     
      if (polarityOfLastSampleV[phase] != NEGATIVE)
      {
        // This is the start of a new -ve half cycle (just after the zero-crossing point)
        // This is a convenient point to update the Low Pass Filter for the phase that is
        // being processed.  This needs to be done right from the start.
        //
        DCoffset_V_long[phase] += (cumVdeltasThisCycle_long[phase]>>6); // faster than * 0.01
        cumVdeltasThisCycle_long[phase] = 0;
      
        // To ensure that this LP filter will always start up correctly when 240V AC is 
        // available, its output value needs to be prevented from drifting beyond the likely range 
        // of the voltage signal.  
        //
        if (DCoffset_V_long[phase] < DCoffset_V_min) {  
          DCoffset_V_long[phase] = DCoffset_V_min; }
        else  
        if (DCoffset_V_long[phase] > DCoffset_V_max) {
          DCoffset_V_long[phase] = DCoffset_V_max; }
    
        if (phase == 0)
        {
          checkLoadPrioritySelection(); // updates load priorities if the switch is changed
        }
        
      } // end of processing that is specific to the first Vsample in each -ve half cycle
    } // end of processing that is specific to samples where the voltage is negitive
  
    // Processing for EVERY pair of samples. Most of this code is not used during the 
    // start-up period, but it does no harm to leave it in place.  Accumulated values 
    // are cleared when the beyondStartUpPhase flag is set to true.
    //
    // remove most of the DC offset from the current sample (the precise value does not matter)
    long sampleIminusDC_long = ((long)(sampleI[phase] - DCoffset_I_nom))<<8;
  
    // phase-shift the voltage waveform so that it aligns with the current when a 
    // resistive load is used
    long  phaseShiftedSampleV_minusDC_long = lastSampleV_minusDC_long[phase]
         + (((sampleV_minusDC_long - lastSampleV_minusDC_long[phase])*phaseCal_int[phase])>>8); 

    // calculate the "real power" in this sample pair and add to the accumulated sum
    long filtV_div4 = phaseShiftedSampleV_minusDC_long>>2;  // reduce to 16-bits (now x64, or 2^6)
    long filtI_div4 = sampleIminusDC_long>>2; // reduce to 16-bits (now x64, or 2^6)
    long instP = filtV_div4 * filtI_div4;  // 32-bits (now x4096, or 2^12)
    instP = instP>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
    sumP[phase] +=instP; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
 
    // for the Vrms calculation (for datalogging only)
    long inst_Vsquared = filtV_div4 * filtV_div4; // 32-bits (now x4096, or 2^12)
    inst_Vsquared = inst_Vsquared>>12;     // scaling is now x1 (V_ADC x I_ADC)
    sum_Vsquared[phase] += inst_Vsquared; // cumulative V^2 (V_ADC x I_ADC)
    if (phase == 0) {
      samplesDuringThisDatalogPeriod ++; } // no need to keep separate counts for each phase
   
    // general housekeeping
    cumVdeltasThisCycle_long[phase] += sampleV_minusDC_long; // for use with LP filter
    samplesDuringThisMainsCycle[phase] ++;
    
    // store items for use during next loop
    lastSampleV_minusDC_long[phase] = sampleV_minusDC_long;  // required for phaseCal algorithm
    polarityOfLastSampleV[phase] = polarityNow;  // for identification of half cycle boundaries
  }
}
// end of processRawSamples()


void processLatestContribution(byte phase, float power)
{
  float latestEnergyContribution = power; // for efficiency, the energy scale is Joules * CYCLES_PER_SECOND

  // add the latest energy contribution to the relevant per-phase accumulator
  // (only used for datalogging of power)
  energyStateOfPhase[phase] += latestEnergyContribution;  
  
  // add the latest energy contribution to the main energy accumulator
  energyInBucket_main += latestEnergyContribution;
  
  // apply any adjustment that is required. 
  if (phase == 0)
  {
    energyInBucket_main -= REQUIRED_EXPORT_IN_WATTS; // energy scale is Joules x 50
  }
  
  // Applying max and min limits to the main accumulator's level
  // is deferred until after the energy related decisions have been taken
  //  
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
  
  // NB. the index cannot be a 'byte' because the loop would not terminate correctly!
  for (char index = (noOfDumploads -1); index >= 0 && !success; index--)
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
       capacityOfEnergyBucket_main * (0.5 - offsetOfEnergyThresholdsInAFmode); 
    upperThreshold_default = 
       capacityOfEnergyBucket_main * (0.5 + offsetOfEnergyThresholdsInAFmode);   
  }
  else
  { 
    // settings for normal mode
    lowerThreshold_default = capacityOfEnergyBucket_main * 0.5; 
    upperThreshold_default = capacityOfEnergyBucket_main * 0.5;   
  }
  
  // display relevant settings for selected output mode
  Serial.print("  capacityOfEnergyBucket_main = ");
  Serial.println(capacityOfEnergyBucket_main);
  Serial.print("  lowerEnergyThreshold   = ");
  Serial.println(lowerThreshold_default);
  Serial.print("  upperEnergyThreshold   = ");
  Serial.println(upperThreshold_default);
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on
}
 
 
#ifdef RF_PRESENT
void send_rf_data()
{
  int i = 0; 
  while (!rf12_canSend() && i<10)
  { 
    rf12_recvDone(); 
    i++;
  }
  rf12_sendStart(0, &tx_data, sizeof tx_data);
}
#endif


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}







