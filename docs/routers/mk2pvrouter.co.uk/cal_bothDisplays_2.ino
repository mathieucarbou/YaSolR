/* cal_bothDisplays_2.ino
 *
 * This sketch provides an easy way of calibrating the current sensors that are 
 * connected to the the CT1 and CT2 ports.  Channel selection is provided by a 
 * switch at the "mode" connector on the main board.  The measured value is shown 
 * on the 4-digit display, and also at the Serial Monitor.
 * 
 * CT1 is for 'grid' current, to be measured at the grid supply point.
 * CT2 is for the load current, so that diverted energy can be recorded
 *
 * The selected CT should be clipped around a lead through which a known
 * amount of power is flowing.  This can be compared against the measured value 
 * which is proportional to the powerCal setting.  Once the optimal powerCal values 
 * have been obtained for each channel, these values can be transferred into the 
 * main Mk2 PV Router sketch.  
 * 
 * Depending on which way around the CT is connected, the measured value may be
 * positive or negative.  If it is negative, the display will either flash or 
 * display a negative symbol.  Its behaviour will depend on the way that the display
 * has been configured.
 *
 * The 4-digit display can be driven in two different ways, one with an extra pair
 * of logic chips, and one without.  The appropriate version of the sketch must be 
 * selected by including or commenting out the "#define PIN_SAVING_HARDWARE" 
 * statement near the top of the code.
 *
 * With the pin-saving logic, the display is not able to show a '-' symbol.  But 
 * in the alternative mode, it is. 
 *
 * December 2017, upgraded to cal_bothDisplays_2:  
 * In the original version, the mains cycle counter cycled through the values 0 to 
 * CYCLES_PER_SECOND inclusive so data was only processed every (CYCLES_PER_SECOND + 1) 
 * mains cycles. Because the accumulated energy data was divided by CYCLES_PER_SECOND, 
 * there was an error of approx 2% in the displayed power value.  In version 2, 
 * the logic has been corrected to avoid this error.
 * 
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 *      December 2017
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
//  The two versions of the hardware require different logic.  The following line should 
//  be included if the additional logic chips are present, or excluded if they are 
//  absent (in which case some wire links need to be fitted)
//
//#define PIN_SAVING_HARDWARE 

// definition of enumerated types
enum polarities {NEGATIVE, POSITIVE};
enum triacStates {TRIAC_ON, TRIAC_OFF}; // the external trigger device is active low
enum powerChannels {DIVERTED, GRID};

// The selected channel is determined in realtime via an external switch
enum powerChannels powerChannel;                                                   

// allocation of digital pins (excluding the display which is dealt with separately)
// ******************************************************
// D0 & D1 are reserved for the Serial i/f
const byte powerChannelSelectorPin = 3; // <-- with the internal pullup 

// allocation of analogue pins (excluding the display which is dealt with separately)
// ***************************
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
long energyThisSecond_grid;
long energyThisSecond_diverted;
long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF
long IEU_per_Joule_grid; 
long IEU_per_Joule_diverted; 

// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
int sampleI_grid;
int sampleI_diverted;
int sampleV;


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
// With my PCB-based hardware, the ADC has an input range of 0 to 3.3V and an 
// output range of 0 to 1023.  The purpose of each input sensor is to 
// convert the measured parameter into a low-voltage signal which fits nicely 
// within the ADC's input range. 
//
// In the case of 240V mains voltage, the numerical value of the input signal 
// in Volts is likely to be fairly similar to the output signal in ADC levels.  
// 240V AC has a peak-to-peak amplitude of 679V, which is not far from the ideal 
// output range.  Stated more formally, the conversion rate of the overall system 
// for measuring VOLTAGE is likely to be around 1 ADC-step per Volt (RMS).
//
// In the case of AC current, however, the situation is very different.  At
// mains voltage, a power of 3kW corresponds to an RMS current of 12.5A which 
// has a peak-to-peak range of 35A.  This is smaller than the output signal of
// the ADC by around a factor of twenty.  The conversion rate of the overall system 
// for measuring CURRENT is therefore likely to be around 20 ADC-steps per Amp.
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
const float powerCal_grid = 0.05;  // for CT1
const float powerCal_diverted = 0.05;  // for CT2
 

// Various settings for the 4-digit display, which needs to be refreshed every few mS
const byte noOfDigitLocations = 4;
const byte noOfPossibleCharacters = 23;
#define MAX_DISPLAY_TIME_COUNT 10// no of processing loops between display updates

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
                 HIGH, HIGH, HIGH, HIGH, LOW , // ' ' <- element 20          
                 HIGH, HIGH, HIGH, HIGH, HIGH, // '.' <- element 21
                 HIGH, HIGH, HIGH, HIGH, LOW  // ' ' <- element 22 (for compatibility)       
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
                 OFF, OFF, OFF, OFF, OFF, OFF, OFF, ON , // '.' <- element 21
                 OFF, OFF, OFF, OFF, OFF, OFF, ON , OFF // '-' <- element 22
};
#endif // PIN_SAVING_HARDWARE

byte charsForDisplay[noOfDigitLocations] = {20,20,20,20}; // all blank 


void setup()
{  
  delay(5000); // allow time to open Serial monitor     
 
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      cal_bothDisplays_2.ino");
  Serial.println();
       
  pinMode(powerChannelSelectorPin, INPUT);
  digitalWrite(powerChannelSelectorPin, HIGH); // enable the internal pullup resistor
  delay (100); // allow time to settle
  int pinState = digitalRead(powerChannelSelectorPin);  // initial selection and
  powerChannel = (enum powerChannels)pinState;            //   assignment of output mode
 
  Serial.print ("powerChannel is set to ");
  if (powerChannel == GRID) {
    Serial.println ( "grid (CT1)"); }
  else {  
    Serial.println ( "diverted (CT2)"); }
 
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
  
  // For converting Integer Energy Units into Joules
  IEU_per_Joule_grid = (long)CYCLES_PER_SECOND * (1/powerCal_grid); 
  IEU_per_Joule_diverted = (long)CYCLES_PER_SECOND * (1/powerCal_diverted); 
 
      
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
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on

  Serial.println ("----");    
}

// An Interrupt Service Routine is now defined in which the ADC is instructed to 
// perform the three analogue measurements in sequence: Voltage, I_diverted and I_grid.  
// A "data ready" flag is set after each voltage conversion has been completed.  This
// flag is cleared within loop().
//   This Interrupt Service Routine is executed whenever the ADC timer expires.  The next 
// ADC conversion is initiated from within this ISR.  
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
// ADC samples.  It then returns to loop() to wait for the next set to become 
// available.
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
  static int samplesDuringThisCycle;             // for normalising the power in each mains cycle
  static long sumP_grid;                              // for per-cycle summation of 'real power' 
  static long sumP_diverted;                              // for per-cycle summation of 'real power' 
  static enum polarities polarityOfLastSampleV;  // for zero-crossing detection
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static long lastSampleVminusDC_long;     //    for the phaseCal algorithm
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
        //
        // The per-mainsCycle variables can now be reset for ongoing use  
        samplesDuringThisCycle = 0;
        sumP_grid = 0;
        sumP_diverted = 0;


        // Next, the energy content of this power rating needs to be determined.  Energy is 
        // power multiplied by time, so the next step is normally to multiply the measured
        // value of power by the time over which it was measured.
        //   Instanstaneous power is calculated once every mains cycle. When integer maths is 
        // being used, a repetitive power-to-energy conversion seems an unnecessary workload.  
        // As all sampling intervals are of equal duration, it is more efficient simply to 
        // add all of the power samples together, and note that their sum is actually 
        // CYCLES_PER_SECOND greater than it would otherwise be.
        //   Although the numerical value itself does not change, I thought that a new name 
        // may be helpful so as to minimise confusion.  
        //   The 'energy' variable below is CYCLES_PER_SECOND * (1/powerCal) times larger than 
        // the actual energy in Joules.
        //
        long realEnergy_grid = realPower_grid; 
        long realEnergy_diverted = realPower_diverted; 
        
   
        // Add these energy contributions to the relevant accumulator. 
        energyThisSecond_grid += realEnergy_grid;   
        energyThisSecond_diverted += realEnergy_diverted;   
        
        // Once per second, the contents of the selected accumulator is 
        // converted to Joules and displayed as Watts.  Both accumulators
        // are then reset.
        // 
        perSecondCounter++;
        if(perSecondCounter >= CYCLES_PER_SECOND) 
        { 
          perSecondCounter = 0;   
          int powerInWatts;
          float powerInWatts_float;
          
          if(powerChannel == GRID) {
            powerInWatts = energyThisSecond_grid / IEU_per_Joule_grid; 
            powerInWatts_float = (float)energyThisSecond_grid / IEU_per_Joule_grid; }
          else {
            powerInWatts = energyThisSecond_diverted / IEU_per_Joule_diverted; 
            powerInWatts_float = (float)energyThisSecond_diverted / IEU_per_Joule_diverted; }
          
          configureValueForDisplay(powerInWatts);
          Serial.println (powerInWatts_float);
          
          energyThisSecond_grid = 0;
          energyThisSecond_diverted = 0;
        }

      } // end of processing that is specific to the first Vsample in each +ve half cycle 
      
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
        
      checkPowerChannelSelection(); // updates outputMode if switch is changed
           
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
  
  samplesDuringThisCycle++;
  
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
  polarityOfLastSampleV = polarityNow;  // for identification of half cycle boundaries

  refreshDisplay();
}
//  ----- end of main Mk2i code -----


// this function changes the value of powerChannel if the state of the external switch is altered 
void checkPowerChannelSelection()  
{
  static byte count = 0;
  int pinState = digitalRead(powerChannelSelectorPin);
  if (pinState != powerChannel)
  {
    count++;
  }  
  if (count >= 20)
  {
    count = 0;
    powerChannel = (enum powerChannels)pinState;  // change the global variable
    Serial.print ("powerChannel selection changed to ");
    if (powerChannel == GRID) {
      Serial.println ( "grid (CT1)"); }
    else {  
      Serial.println ( "diverted (CT2)"); }
  }
}


// called every second, to update the characters to be displayed
void configureValueForDisplay(int power)
{
  boolean energyValueExceeds10kWh;
  boolean powerIsNegative;
  int val;
  static boolean perSecondToggle = false; 
    
  if (power < 0) {
    powerIsNegative = true; 
    val = -1 * power; }
  else {
    powerIsNegative = false; 
    val = power; }
        
  if (powerIsNegative && !perSecondToggle)
  {
    // do something different
    charsForDisplay[0] = 22;        
    charsForDisplay[1] = 20;        
    charsForDisplay[2] = 20;        
    charsForDisplay[3] = 20;        
  }
  else
  {
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
  
  perSecondToggle = !perSecondToggle;
  
  Serial.println(power); // in case a display is not available
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




