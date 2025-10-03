// ****************************
// * Source de Mesures U et I *
// *          UXI             *
// ****************************


void Setup_UxI() {
  for (int i = 0; i < 100; i++) {  //Reset table measurements
    voltM[i] = 0;
    ampM[i] = 0;
  }
}
void LectureUxI() {
  MeasurePower();
  ComputePower();
}
void MeasurePower() {  //Lecture Tension et courants pendant 20ms
  int iStore;
  value0 = analogRead(AnalogIn0);  //Mean value. Should be at 3.3v/2 /35
  static unsigned long OverflowOffset = 0;
  static unsigned long PrevMicros = 0;
  unsigned long NowMicros;
  unsigned long MeasureMillis = millis();
  while (millis() - MeasureMillis < 21) {  //Read values in continuous during 20ms. One loop is around 150 micro seconds
    NowMicros = micros();
    if (NowMicros < PrevMicros) {
      OverflowOffset = (OverflowOffset+ 7296)%20000;
    }
    iStore = ((NowMicros%20000 + OverflowOffset) % 20000) / 200;  //We have more results that we need during 20ms to fill the tables of 100 samples
    volt[iStore] = analogRead(AnalogIn1) - value0;//32
    amp[iStore] = analogRead(AnalogIn2) - value0;//33
    PrevMicros = NowMicros;
  }
}

void ComputePower() {
  float PWcal = 0;  //Computation Power in Watt
  float V;
  float I;
  float Uef2 = 0;
  float Ief2 = 0;
  int idx0 = 0;
  int j = 0;
  
  for (int i = 0; i < 100; i++) {
    voltM[i] = (19.0 * voltM[i] + float(volt[i])) / 20.0;  //Mean value. First Order Filter. Short Integration
    V = kV * voltM[i];
    Uef2 += sq(V);
    ampM[i] = (19.0 * ampM[i] + float(amp[i])) / 20.0;  //Mean value. First Order Filter
    I = kI * ampM[i];
    Ief2 += sq(I);
    PWcal += V * I;
  }
  Uef2 = Uef2 / 100.0;       //square of voltage
  Tension_M = sqrt(Uef2);    //RMS voltage
  Ief2 = Ief2 / 100.0;       //square of current
  Intensite_M = sqrt(Ief2);  // RMS current
  PWcal = PfloatMax(PWcal / 100.0);
  float PVA = PfloatMax(floor(Tension_M * Intensite_M));
  float PowerFactor = 0;
  if (PVA > 0) {
    PowerFactor = floor(100.0 * PWcal / PVA) / 100.0;
  }
  PowerFactor_M = PowerFactor;
  if (PWcal >= 0) {
    EASfloat += PWcal / 90000.0;         // Watt Hour,Every 40ms. Soutirée
    Energie_M_Soutiree = int(EASfloat);  
    PuissanceS_M_inst = PWcal;
    PuissanceI_M_inst = 0;
    PVAS_M_inst = PVA;
    PVAI_M_inst = 0;
  } else {
    EAIfloat += -PWcal / 90000.0;
    Energie_M_Injectee = int(EAIfloat);
    PuissanceS_M_inst = 0;
    PuissanceI_M_inst = -PWcal;
    PVAS_M_inst = 0;
    PVAI_M_inst = PVA;
  }
  Pva_valide = true;
  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
  filtre_puissance();
  EnergieActiveValide = true;
  PuissanceRecue = true;
}
