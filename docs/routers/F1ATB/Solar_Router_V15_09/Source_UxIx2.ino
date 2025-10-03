// *******************************
// * Source de Mesures UI Double *
// *      Capteur JSY-MK-194     *
// *******************************

void Setup_UxIx2() {
  MySerial.setRxBufferSize(SER_BUF_SIZE);
  MySerial.begin(4800, SERIAL_8N1, RXD2, TXD2);  //PORT DE CONNEXION AVEC LE CAPTEUR JSY-MK-194
}
void LectureUxIx2() {  //Ecriture et Lecture port série du JSY-MK-194  .

  int i, j;

  if (RAZ_JSY) {  //RAZ Message proposé par F6AAM
    byte msg_send[] = { 0x01, 0x10, 0x00, 0x0C, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0xF3, 0xFA };
    // Envoi commande raz sur le JSY-MK_194T
    for (i = 0; i < 13; i++) {
      MySerial.write(msg_send[i]);
    }
    RAZ_JSY = false;
    delay(100);
  }

  byte msg_send[] = { 0x01, 0x03, 0x00, 0x48, 0x00, 0x0E, 0x44, 0x18 };
  // Demande Info sur le Serial port 2 (Modbus RTU)
  for (i = 0; i < 8; i++) {
    MySerial.write(msg_send[i]);
  }

  //Réponse en général à l'appel précédent (seulement 4800bauds)
  int a = 0;
  while (MySerial.available()) {
    ByteArray[a] = MySerial.read();
    a++;
  }


  if (a == 61) {  //Message complet reçu
    j = 3;
    for (i = 0; i < 14; i++) {  // conversion séries de 4 octets en long
      LesDatas[i] = 0;
      LesDatas[i] += ByteArray[j] << 24;
      j += 1;
      LesDatas[i] += ByteArray[j] << 16;
      j += 1;
      LesDatas[i] += ByteArray[j] << 8;
      j += 1;
      LesDatas[i] += ByteArray[j];
      j += 1;
    }
    Sens_1 = ByteArray[27];  // Sens 1
    Sens_2 = ByteArray[28];

    //Données du Triac
    Tension_T = LesDatas[0] * .0001;
    Intensite_T = LesDatas[1] * .0001;
    float Puiss_1 = PfloatMax(LesDatas[2] * .0001);
    Energie_T_Soutiree = int(LesDatas[3] * .1);
    PowerFactor_T = LesDatas[4] * .001;
    Energie_T_Injectee = int(LesDatas[5] * .1);
    Frequence = LesDatas[7] * .01;
    float PVA1 = 0;
    if (PowerFactor_T > 0) {
      PVA1 = Puiss_1 / PowerFactor_T;
    }
    if (Sens_1 > 0) {  //Injection sur TRiac. Ne devrait pas arriver
      PuissanceI_T_inst = Puiss_1;
      PuissanceS_T_inst = 0;
      PVAI_T_inst = PVA1;
      PVAS_T_inst = 0;
    } else {
      PuissanceS_T_inst = Puiss_1;
      PuissanceI_T_inst = 0;
      PVAI_T_inst = 0;
      PVAS_T_inst = PVA1;
    }
    // Données générale de la Maison
    Tension_M = LesDatas[8] * .0001;
    Intensite_M = LesDatas[9] * .0001;
    float Puiss_2 = PfloatMax(LesDatas[10] * .0001);
    Energie_M_Soutiree = int(LesDatas[11] * .1);
    PowerFactor_M = LesDatas[12] * .001;
    Energie_M_Injectee = int(LesDatas[13] * .1);
    float PVA2 = 0;
    if (PowerFactor_M > 0) {
      PVA2 = Puiss_2 / PowerFactor_M;
    }
    if (Sens_2 > 0) {  //Injection en entrée de Maison
      PuissanceI_M_inst = Puiss_2;
      PuissanceS_M_inst = 0;
      PVAI_M_inst = PVA2;
      PVAS_M_inst = 0;
    } else {
      PuissanceS_M_inst = Puiss_2;
      PuissanceI_M_inst = 0;
      PVAI_M_inst = 0;
      PVAS_M_inst = PVA2;
    }
    filtre_puissance();
    EnergieActiveValide = true;
    Pva_valide = true;
    PuissanceRecue = true;  //Reset du Watchdog à chaque trame du module JSY-MK-194 reçue
    if (cptLEDyellow > 30) {
      cptLEDyellow = 4;
    }
  }
}