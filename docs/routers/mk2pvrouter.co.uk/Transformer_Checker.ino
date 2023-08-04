/* Transformer_Checker is based on Mk2_RF_datalog_3.ino
 *  
 * Every 1-phase Mk2 PV Router control board has a mains transformer with two secondary outputs.  One output provides 
 * a low-voltage replica of the AC mains voltage; the other is rectified to provide a low-voltage DC supply for the 
 * processor.  Although the power consumption of the Atmel 328P processor is fairly constant, it will be increase 
 * whenever the output stage is activated.  The increased draw from the DC supply will cause the amplitude of the AC signal 
 * from the other output to slightly decrease.
 * 
 * This sketch can be used to quantify the above effect. A standard output stage should be connected to the primary 
 * output port but no AC load should be connected otherwise a consequent reduction in the local mains voltage 
 * could adversely affect this test.
 * 
 * Via the Serial Monitor, this sketch will display the percentage reduction in the measured Vrms value whenever 
 * the output stage is activated.  By adding an extra LED which operates in anti-phase with the primary output, the  
 * reduction in Vrms can be effectively eliminated.  Both LEDs can be driven by the same output port but with their other 
 * terminals connected to opposite power rails via series resistors of appropriate values. 
 * 
 * Any reduction in the measured Vrms value when the output stage is activated represents a non-linearity which will
 * result in less than ideal performance.  By means of this sketch, an extra LED and series resistor can be used to 
 * minimise any such effect.
 * 
 * July 2021: first release.
 * 
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 */

#include <Arduino.h> 
#include <TimerOne.h>

#define ADC_TIMER_PERIOD 125 // uS (determines the sampling rate / amount of idle time)

// Change these values to suit the local mains frequency and supply meter
#define CYCLES_PER_SECOND 50 
#define DATALOG_PERIOD 5 // seconds

// definition of enumerated types
enum polarities {NEGATIVE, POSITIVE};
enum outputStates {OUTPUT_STAGE_OFF, OUTPUT_STAGE_ON}; 
enum outputStates nextOutputState = OUTPUT_STAGE_ON;
enum outputStates outputStateNow;

int outputStateCounter = 0;
float Vrms_whileOutputStageIsOn;
float Vrms_whileOutputStageIsOff;
#define MAX_CONSECUTIVE_CYCLES 10

// allocation of digital pins 
// **************************
const byte outputForTrigger = 4;   

// allocation of analogue pins
// ***************************
const byte voltageSensor = 3;          // A3 is for the voltage sensor
const byte currentSensor_diverted = 4; // A4 is for CT2 (which is not used by this sketch)
const byte currentSensor_grid = 5;     // A5 is for CT1 (which is not used by this sketch)

const byte startUpPeriod = 1;  // in seconds, to allow LP filter to settle

boolean beyondStartUpPhase = false;     // start-up delay, allows things to settle
long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF

long sum_Vsquared_whileOutputStageIsOn; 
long sum_Vsquared_whileOutputStageIsOff; 

long sampleSets_whileOutputStageIsOn;
long sampleSets_whileOutputStageIsOff;

// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
int sampleI_grid;
int sampleI_diverted;
int sampleV;


// Calibration values
//-------------------
// When operating at 230 V AC, the range of ADC values will be similar to the actual range of volts, 
// so the optimal value for this cal factor will be close to unity.  For this sketch, the value of voltageCal 
// makes no difference because the key output is a ratio between the results of two calculations which both
// use the same voltageCal value.
//
const float voltageCal = 1.0; 


void setup()
{  
  pinMode(outputForTrigger, OUTPUT);  
  digitalWrite (outputForTrigger, OUTPUT_STAGE_ON); 
  
  delay(1000); // allow time to open Serial monitor     
 
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      Transformer_Checker.ino");
  Serial.println();
       
                             
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
          
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on
  Serial.println ("----");    
}

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
  static enum polarities polarityOfLastSampleV;  // for zero-crossing detection
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static byte perSecondCounter = 0;
  static int sampleSetsDuringNegativeHalfOfMainsCycle; // for arming the triac/trigger
  
  // extra items for datalogging
  static int datalog_counter = 0;  // counts seconds
 
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
        outputStateNow = nextOutputState; // to correspond with the action of the opto-isolator
        
        perSecondCounter++;
        if(perSecondCounter >= CYCLES_PER_SECOND) 
        { 
          perSecondCounter = 0;          

          // routine data is calculated every N seconds
          datalog_counter++;
          if (datalog_counter >= DATALOG_PERIOD)
          {
            datalog_counter = 0;
            
           Vrms_whileOutputStageIsOn = 
       voltageCal * sqrt(sum_Vsquared_whileOutputStageIsOn / sampleSets_whileOutputStageIsOn);
           Vrms_whileOutputStageIsOff = 
       voltageCal * sqrt(sum_Vsquared_whileOutputStageIsOff / sampleSets_whileOutputStageIsOff);
            
            sum_Vsquared_whileOutputStageIsOn = 0;
            sampleSets_whileOutputStageIsOn = 0;
            sum_Vsquared_whileOutputStageIsOff = 0;
            sampleSets_whileOutputStageIsOff = 0;
            
            Serial.print(Vrms_whileOutputStageIsOff); 
            Serial.print(", "); 
            Serial.print(Vrms_whileOutputStageIsOn); 
            Serial.print(", "); 
            float Vrms_reduction = (Vrms_whileOutputStageIsOn / Vrms_whileOutputStageIsOff) * 100;
            Serial.print(Vrms_reduction, 2); 
            Serial.println('%'); 
          }
        }
      } // end of processing that is specific to the first Vsample in each +ve half cycle 
    }
    else
    {  
      // wait until the DC-blocking filters have had time to settle
      if(millis() > startUpPeriod * 1000) 
      {
        beyondStartUpPhase = true;
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

      sampleSetsDuringNegativeHalfOfMainsCycle = 0;

      outputStateCounter++;
      if(outputStateCounter >= MAX_CONSECUTIVE_CYCLES) 
      {
        outputStateCounter = 0;
        nextOutputState = (enum outputStates)!outputStateNow;
      }        
    } // end of processing that is specific to the first Vsample in each -ve half cycle
    
    sampleSetsDuringNegativeHalfOfMainsCycle++;    

    // check to see whether the trigger device can now be reliably armed
    if (sampleSetsDuringNegativeHalfOfMainsCycle == 3) 
    {
        digitalWrite(outputForTrigger, !nextOutputState); // the trigger control circuit is active low       
    }
    
  } // end of processing that is specific to samples where the voltage is negative
  
  // processing for EVERY pair of samples
  //
  // for the Vrms calculations
  long filtV_div4 = sampleVminusDC_long>>2;  // reduce to 16-bits (now x64, or 2^6)
  long inst_Vsquared = filtV_div4 * filtV_div4; // 32-bits (now x4096, or 2^12)
  inst_Vsquared = inst_Vsquared>>12;     // scaling is now x1 (V_ADC x I_ADC)

  if (outputStateNow == OUTPUT_STAGE_ON)
  {
    sum_Vsquared_whileOutputStageIsOn += inst_Vsquared; 
    sampleSets_whileOutputStageIsOn++;         
  }
  else
  {
    sum_Vsquared_whileOutputStageIsOff += inst_Vsquared; 
    sampleSets_whileOutputStageIsOff++;         
  }
  
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  polarityOfLastSampleV = polarityNow;  // for identification of half cycle boundaries
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}





