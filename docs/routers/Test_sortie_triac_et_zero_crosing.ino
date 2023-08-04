/*

*/

int pushButton =33;
  int buttonState = 0;
// the setup function runs once when you press reset or power the board
void setup() {
  // pinmode(27, OUTPUT ) = Sortie triac 1 / pinmode(13, OUTPUT ) = Sortie triac 2
  Serial.begin(115200);
  pinMode(pushButton, INPUT);
  pinMode(27, OUTPUT);
  pinMode(13, OUTPUT);
}

void test_triac(){
  digitalWrite(27, HIGH); 
  Serial.println ("La sortie S1  TRIAC ON");
  delay(1000);                       // wait for a second
  digitalWrite(27, LOW);    // turn the LED off by making the voltage LOW
  Serial.println ("La sortie S1  TRIAC OFF");
  delay(500);                       // wait for a second
  digitalWrite(13, HIGH);
  Serial.println ("La sortie  S2 TRIAC ON");
  delay(1000);                       // wait for a second
  digitalWrite(13, LOW); 
  Serial.println ("La sortie  S2 TRIAC OFF");
  delay(500);                       // wait for a second
}


void test_zeroC(){
 unsigned long start_times;
  Serial.println("test Zero crossing :");
 
   start_times = millis();

   while (digitalRead(pushButton) ) if(millis()>start_times+100) return;
  while (!digitalRead(pushButton) ) if(millis()>start_times+100) return;
  start_times = micros();
    while (digitalRead(pushButton) );
  start_times = micros()- start_times;

  buttonState += 1;
  Serial.print ("Compteur Zero crossing :");
  Serial.println(buttonState);
  Serial.print ("impulsion Zero crossing : ");
  Serial.print(start_times);
 Serial.println (" us");
 delay(2000);
}


// the loop function runs over and over again forever
void loop() {
  test_triac();
  test_zeroC();
  delay(1000);       
}
