//****************************************
// Source de puissance non définie       *
//  Simulation de données                *
// ***************************************
void LectureNotDef() {

  float Pw = float(int(millis() / 30) % 2000 - 500);  //Simulation

  if (Pw >= 0) {
    PuissanceS_M_inst = Pw;
    PuissanceI_M_inst = 0;
    PVAS_M_inst = Pw + 250;
    PVAI_M_inst = 0;
    EASfloat += Pw / 6000.0;             // Watt Hour,Every 600ms. Soutirée
    Energie_M_Soutiree = int(EASfloat);  //
  } else {
    PuissanceS_M_inst = 0;
    PuissanceI_M_inst = -Pw;
    PVAI_M_inst = -Pw + 250;
    PVAS_M_inst = 0;
    EAIfloat += -Pw / 6000.0;
    Energie_M_Injectee = int(EAIfloat);
  }
  Pva_valide = true;
  filtre_puissance();
  PuissanceRecue = true;
  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}