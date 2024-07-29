// *************************************************
// * Client lecture JSY-MK-333 * Triphasé *
// * Développement initial de Pierre F (Mars 2024) *
// * update PhDV61 Juin 2024 *
// *************************************************


void Setup_JSY333() {
  MySerial.setRxBufferSize(SER_BUF_SIZE);
  MySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);  //PORT DE CONNEXION AVEC LE CAPTEUR JSY-MK-333
}

void Lecture_JSY333() {
  float Tension_M1, Tension_M2, Tension_M3;
  float Intensite_M1, Intensite_M2, Intensite_M3;
  float PVA_M_inst1, PVA_M_inst2, PVA_M_inst3;
  float PW_inst1, PW_inst2, PW_inst3;

  byte Lecture333[200];
  bool injection;
  bool sens1, sens2, sens3;
  long delta_temps = 0;

  int i;
  byte msg_send[] = { 0x01, 0x03, 0x01, 0x00, 0x00, 0x44, 0x44, 0x05 };
  for (i = 0; i < 8; i++) {
    MySerial.write(msg_send[i]);
  }

  int a = 0;
  while (MySerial.available()) {
    Lecture333[a] = MySerial.read();
    a++;
  }

  if (a == 141) {                                               //message complet reçu
    delta_temps = (unsigned long)(millis() - Temps_precedent);  // temps écoulé depuis le dernier appel
    Temps_precedent = millis();                                 // on conserve la valeur du temps actuel pour le calcul précédent

    Tension_M1 = ((float)(Lecture333[3] * 256 + Lecture333[4])) / 100;
    Tension_M2 = ((float)(Lecture333[5] * 256 + Lecture333[6])) / 100;
    Tension_M3 = ((float)(Lecture333[7] * 256 + Lecture333[8])) / 100;
    Intensite_M1 = ((float)(Lecture333[9] * 256 + Lecture333[10])) / 100;
    Intensite_M2 = ((float)(Lecture333[11] * 256 + Lecture333[12])) / 100;
    Intensite_M3 = ((float)(Lecture333[13] * 256 + Lecture333[14])) / 100;

    sens1 = (Lecture333[104]) & 0x01;
    sens2 = (Lecture333[104] >> 1) & 0x01;
    sens3 = (Lecture333[104] >> 2) & 0x01;

    if (sens1) { Intensite_M1 *= -1; }
    if (sens2) { Intensite_M2 *= -1; }
    if (sens3) { Intensite_M3 *= -1; }

    injection = (Lecture333[104] >> 3) & 0x01;  //si sens est true, injection

    // Lecture des Puissances actives de chacune des phases
    PW_inst1 = (float)(Lecture333[15] * 256.0) + (float)Lecture333[16];
    PW_inst2 = (float)(Lecture333[17] * 256.0) + (float)Lecture333[18];
    PW_inst3 = (float)(Lecture333[19] * 256.0) + (float)Lecture333[20];

    //Lecture des puissances apparentes de chacune des phases, qu'on signe comme le Linky
    PVA_M_inst1 = (float)(Lecture333[35] * 256) + (float)Lecture333[36];
    if (sens1) { PVA_M_inst1 = -PVA_M_inst1; }
    PVA_M_inst2 = (float)(Lecture333[37] * 256) + (float)Lecture333[38];
    if (sens2) { PVA_M_inst2 = -PVA_M_inst2; }
    PVA_M_inst3 = (float)(Lecture333[39] * 256) + (float)Lecture333[40];
    if (sens3) { PVA_M_inst3 = -PVA_M_inst3; }

    if (injection) {
      PuissanceS_M_inst = 0;
      PuissanceI_M_inst = ((float)((float)(Lecture333[21] * 16777216) + (float)(Lecture333[22] * 65536) + (float)(Lecture333[23] * 256) + (float)Lecture333[24]));
      PVAS_M_inst = 0;
      PVAI_M_inst = abs(PVA_M_inst1 + PVA_M_inst2 + PVA_M_inst3);  // car la somme des puissances apparentes "signées" est négative puisqu'en "injection" au global

      // PhDV61 : on considère que cette puissance active "globale" a duré "delta_temps", et on l'intègre donc pour obtenir une énergie en Wh
      Energie_jour_Injectee += ((float)delta_temps / 1000) * (PuissanceI_M_inst / 3600.0);

    } else {  // soutirage
      PuissanceI_M_inst = 0;
      PuissanceS_M_inst = ((float)((float)(Lecture333[21] * 16777216) + (float)(Lecture333[22] * 65536) + (float)(Lecture333[23] * 256) + (float)Lecture333[24]));
      PVAI_M_inst = 0;
      PVAS_M_inst = PVA_M_inst1 + PVA_M_inst2 + PVA_M_inst3;

      // PhDV61 : on considère que cette puissance active "globale" a duré "delta_temps", et on l'intègre donc pour obtenir pour obtenir une énergie en Wh
      Energie_jour_Soutiree += ((float)delta_temps / 1000) * (PuissanceS_M_inst / 3600.0);
    }

    // PowerFactor_M = ((float)(Lecture333[53] * 256 + Lecture333[54])) / 1000; ce facteur de puissance ne veut rien dire en tri-phasé en cas d'injection sur au moins une phase

    Energie_M_Soutiree = ((float)((float)(Lecture333[119] * 16777216) + (float)(Lecture333[120] * 65536) + (float)(Lecture333[121] * 256) + (float)Lecture333[122])) * 10;
    Energie_M_Injectee = ((float)((float)(Lecture333[135] * 16777216) + (float)(Lecture333[136] * 65536) + (float)(Lecture333[137] * 256) + (float)Lecture333[138])) * 10;

    MK333_dataBrute = "<strong>Triphasé</strong><br>Phase1 : " + String(int(Tension_M1)) + "V " + String(Intensite_M1) + "A</br>";
    MK333_dataBrute += "<br>Phase2 : " + String(int(Tension_M2)) + "V " + String(Intensite_M2) + "A</br>";
    MK333_dataBrute += "<br>Phase3 : " + String(int(Tension_M3)) + "V " + String(Intensite_M3) + "A</br>";
    MK333_dataBrute += "<br>Puissance active soutirée : " + String(PuissanceS_M_inst) + "W</br>";
    MK333_dataBrute += "<br>Puissance active injectée : " + String(PuissanceI_M_inst) + "W</br>";
    MK333_dataBrute += "<br>Puissance apparente soutirée : " + String(PVAS_M_inst) + "VA</br>";
    MK333_dataBrute += "<br>Puissance apparente injectée : " + String(PVAI_M_inst) + "VA</br>";

    if (PVA_M_inst1 != 0)
      MK333_dataBrute += "<br>Facteur de puissance phase 1 : " + String(abs(PW_inst1 / PVA_M_inst1)) + "</br>";
    if (PVA_M_inst2 != 0)
      MK333_dataBrute += "<br>Facteur de puissance phase 2 : " + String(abs(PW_inst2 / PVA_M_inst2)) + "</br>";
    if (PVA_M_inst3 != 0)
      MK333_dataBrute += "<br>Facteur de puissance phase 3 : " + String(abs(PW_inst3 / PVA_M_inst3)) + "</br>";

    MK333_dataBrute += "<br>Energie jour nette soutirée (Linky): " + String(Energie_jour_Soutiree) + "Wh</br>";
    MK333_dataBrute += "<br>Energie jour nette injectée (Linky): " + String(Energie_jour_Injectee) + "Wh</br>";

    MK333_dataBrute += "<br>Energie totale soutirée : " + String(Energie_M_Soutiree) + "Wh</br>";
    MK333_dataBrute += "<br>Energie totale injectée : " + String(Energie_M_Injectee) + "Wh</br>";

    Pva_valide = true;
    filtre_puissance();
    PuissanceRecue=true;  //Reset du Watchdog à chaque trame du JSY reçue
    EnergieActiveValide = true;
    if (cptLEDyellow > 30) {
      cptLEDyellow = 4;
    }
  } else {
    StockMessage("Pas tout reçu, pas traité... nombre de données : " + String(a));
  }
}