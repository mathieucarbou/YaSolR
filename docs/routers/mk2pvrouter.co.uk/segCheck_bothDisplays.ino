/*
 * This sketch exercises every digit of the 4-digit display that forms part of my
 * PCB-based Mk2 PV Router hardware.  Before fitting a display module into an
 * enclosure, this sketch provides an easy way of ensuring that it is fully working.
 *
 * The hardware that drives the display can be assembled in two ways, one with an 
 * extra pair of logic chips, and one without.  The appropriate version of this 
 * sketch must be selected by including or commenting out the 
 * "#define PIN_SAVING_HARDWARE" statement which is near the top of the code.
 * 
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 *      March 2014
 */
 
#include <Arduino.h> // may not be needed, but it's probably a good idea to include this

const byte noOfDigitLocations = 4;
const byte noOfPossibleCharacters = 22;

#define MAX_DISPLAY_TIME_COUNT 3// no of processing loops between display updates

//  The two versions of the hardware require different logic.
#define PIN_SAVING_HARDWARE

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
                 OFF, OFF, OFF, OFF, OFF, OFF, OFF, ON  // '.' <- element 21
};
#endif // PIN_SAVING_HARDWARE


byte charsForDisplay[noOfDigitLocations] = {18, 18, 18, 18}; // all segmnents on 


void setup()
{
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

  // display the start-up state with all segments on
  for (int i = 0; i < 3000; i++) {
    delay(1); // to simulate one iteration of loop() in the Mk2 sketch
    refreshDisplay(); }  
  
  // clear all segments
  for (int i = 0; i < noOfDigitLocations; i++)
  {
    charsForDisplay[i] = 20; // blank
  }
  
  // display the "all segments off" state for a short while
  for (int i = 0; i < 1000; i++)
  {
    delay(1); // to simulate one iteration of loop() in the Mk2 sketch
    refreshDisplay();
  }  
  
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:    segCheck_bothDisplays.ino");
  
  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on
}


void loop()             
{ 
  static unsigned long timeOfLastDisplayChange = 0;
  static byte digit = 0; // runs from 0 to 3
  static byte value = 0; // runs from 0 to 21 (states 10 to 19 are bypassed)
  
  unsigned long timeNow = millis();
  if (timeNow - timeOfLastDisplayChange > 500)
  {
    // the display needs to be updated
    timeOfLastDisplayChange = timeNow;
      
    if (value == 10)
    {
      value += 10;  // to omit the repetition of all digits with the '.' active
    }
    else
    {  
      if (value >= noOfPossibleCharacters)
      {
        value = 0;
    
        charsForDisplay[digit] = 20; // set the digit that's just been exercised to blank.
        digit++; 
        if (digit >= noOfDigitLocations)
        {
          digit = 0;
        }
      }
    } 
    
    Serial.print (digit);
    Serial.print (",  ");
    Serial.println (value);
    charsForDisplay[digit] = value;
    
    value++;
    
  }
  
  delay(1); // to simulate one iteration of loop() in the Mk2 sketch
  refreshDisplay();
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



