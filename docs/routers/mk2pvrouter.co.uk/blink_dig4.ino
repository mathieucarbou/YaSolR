
/* A modified version of the standard example sketch, blink.ino, in which 
 * digital pin 4 is driven up and down instead of pin 13.  
 * 
 *      Robin Emley
 *      www.Mk2PVrouter.co.uk
 *      February 2014
 */


/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 4 is used by my Mk2 PV Router PCB, rev 1.1,  to control the output device
int controlPin = 4; 

// The signal from pin 4 is generally used in an active low manner
#define ON LOW
#define OFF  HIGH

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(controlPin, OUTPUT);     
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(controlPin, ON);  // drives the output pin low 
  delay(1000);               // wait for a second
  digitalWrite(controlPin, OFF);  // drives the output pin high  
  delay(2000);               // wait for a second
}
