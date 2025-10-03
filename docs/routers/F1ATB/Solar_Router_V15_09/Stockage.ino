// ***************************
// Stockage des données en ROM
// ***************************
//Plan stockage
#define EEPROM_SIZE 4090
#define NbJour 370             //Nb jour historique stocké
#define adr_HistoAn 0          //taille 2* 370*4=1480
#define adr_E_T_soutire0 1480  // 1 long. Taille 4 Triac
#define adr_E_T_injecte0 1484
#define adr_E_M_soutire0 1488    // 1 long. Taille 4 Maison
#define adr_E_M_injecte0 1492    // 1 long. Taille 4
#define adr_DateCeJour 1496      // String 8+1
#define adr_lastStockConso 1505  // Short taille 2
#define adr_ParaActions 1507     //Clé + ensemble parametres peu souvent modifiés


void INIT_EEPROM(void) {
  if (!EEPROM.begin(EEPROM_SIZE)) {
    StockMessage("Failed to initialise EEPROM");
    delay(10000);
    ESP.restart();
  }
}

void RAZ_Histo_Conso() {
  //Mise a zero Zone stockage
  int Adr_SoutInjec = adr_HistoAn;
  for (int i = 0; i < NbJour; i++) {
    EEPROM.writeLong(Adr_SoutInjec, 0);
    Adr_SoutInjec = Adr_SoutInjec + 4;
  }
  EEPROM.writeULong(adr_E_T_soutire0, 0);
  EEPROM.writeULong(adr_E_T_injecte0, 0);
  EEPROM.writeULong(adr_E_M_soutire0, 0);
  EEPROM.writeULong(adr_E_M_injecte0, 0);
  EEPROM.writeString(adr_DateCeJour, "");
  EEPROM.writeUShort(adr_lastStockConso, 0);
  EEPROM.commit();
}

void LectureConsoMatinJour(void) {

  Energie_jour_Soutiree = 0;  // en Wh
  Energie_jour_Injectee = 0;  // en Wh

  EAS_T_J0 = EEPROM.readULong(adr_E_T_soutire0);  //Triac
  EAI_T_J0 = EEPROM.readULong(adr_E_T_injecte0);
  EAS_M_J0 = EEPROM.readULong(adr_E_M_soutire0);  //Maison
  EAI_M_J0 = EEPROM.readULong(adr_E_M_injecte0);
  //DateCeJour = EEPROM.readString(adr_DateCeJour); Plus utilisé depuis V13
  idxPromDuJour = EEPROM.readUShort(adr_lastStockConso);
  if (Energie_T_Soutiree < EAS_T_J0) {
    Energie_T_Soutiree = EAS_T_J0;
  }
  if (Energie_T_Injectee < EAI_T_J0) {
    Energie_T_Injectee = EAI_T_J0;
  }
  if (Energie_M_Soutiree < EAS_M_J0) {
    Energie_M_Soutiree = EAS_M_J0;
  }
  if (Energie_M_Injectee < EAI_M_J0) {
    Energie_M_Injectee = EAI_M_J0;
  }
}


void JourHeureChange() {
  char buffer[MAX_SIZE_T];
  unsigned short Tnow = CptIT;
  if (Horloge > 4 && ITmode > 0) {  // Horloge sur IT 20ms
    StepIT = 2;
  } else {
    StepIT = 1;
  }

  unsigned short deltaT = Tnow - Int_Last_10Millis;
  int16_t old_Heure = Int_Heure;
  while (deltaT >= 100) {
    Int_Last_10Millis = Int_Last_10Millis + 100;
    Int_Seconde++;
    T_On_seconde++;
    deltaT = Tnow - Int_Last_10Millis;
    while (Int_Seconde >= 60) {
      Int_Minute++;
      Int_Seconde = Int_Seconde - 60;
      while (Int_Minute >= 60) {
        Int_Heure = (Int_Heure + 1) % 24;
        Int_Minute = Int_Minute - 60;
      }
    }
  }

  if (Horloge == 0) {  //Heure Internet
    //Time Update / de l'heure
    time_t timestamp = time(NULL);
    struct tm *pTime = localtime(&timestamp);
    strftime(buffer, MAX_SIZE_T, "%d/%m/%Y %H:%M:%S", pTime);
    DATE = String(buffer);
    strftime(buffer, MAX_SIZE_T, "%H", pTime);
    int16_t hour = atoi(buffer);
    Int_Heure = hour;
    strftime(buffer, MAX_SIZE_T, "%M", pTime);
    int16_t minute = atoi(buffer);
    Int_Minute = minute;
  } else if (Horloge == 1) {  //Heure Linky
    Int_Heure = Int_HeureLinky;
    Int_Minute = Int_MinuteLinky;
    Int_Seconde = Int_SecondeLinky;
    sprintf(buffer, "%d:%02d:%02d", Int_Heure, Int_Minute, Int_Seconde);
    DATE = JourLinky + " " + String(buffer);
  } else {  //Horloge interne ou par IT 10 ou 20ms
    sprintf(buffer, "%d:%02d:%02d", Int_Heure, Int_Minute, Int_Seconde);
    DATE = String(buffer);
  }
  HeureCouranteDeci = Int_Heure * 100 + Int_Minute * 10 / 6;
  if (HeureCouranteDeci >= 599 && HeureCouranteDeci <= 600) {
    for (int i = 0; i < LesActionsLength; i++) {
      LesActions[i].H_Ouvre = 0;  //RAZ temps equivalent ouverture à 6h du matin
    }
  }
  if (old_Heure == 23 && Int_Heure == 0) {
    erreurTriac = false;
    if (EnergieActiveValide) {  //Données recues
      idxPromDuJour = (idxPromDuJour + 1 + NbJour) % NbJour;
      //On enregistre les conso en début de journée pour l'historique de l'année
      long energie = Energie_M_Soutiree - Energie_M_Injectee;  //Bilan energie du jour
      EEPROM.writeLong(idxPromDuJour * 4, energie);
      EEPROM.writeULong(adr_E_T_soutire0, long(Energie_T_Soutiree));
      EEPROM.writeULong(adr_E_T_injecte0, long(Energie_T_Injectee));
      EEPROM.writeULong(adr_E_M_soutire0, long(Energie_M_Soutiree));
      EEPROM.writeULong(adr_E_M_injecte0, long(Energie_M_Injectee));
      EEPROM.writeString(adr_DateCeJour, "");
      EEPROM.writeUShort(adr_lastStockConso, idxPromDuJour);
      EEPROM.commit();
      LectureConsoMatinJour();
    }
    //Puissance Max du jour à zero
    PuisMaxS_T = 0;
    PuisMaxS_M = 0;
    PuisMaxI_T = 0;
    PuisMaxI_M = 0;
  }
}
String HistoriqueEnergie1An(void) {
  String S = "";
  int Adr_SoutInjec = 0;
  long EnergieJour = 0;
  long DeltaEnergieJour = 0;
  int iS = 0;
  long lastDay = 0;

  for (int i = 0; i < NbJour; i++) {
    iS = (idxPromDuJour + i + 1) % NbJour;
    Adr_SoutInjec = adr_HistoAn + iS * 4;
    EnergieJour = EEPROM.readLong(Adr_SoutInjec);
    if (lastDay == 0) { lastDay = EnergieJour; }
    DeltaEnergieJour = EnergieJour - lastDay;
    lastDay = EnergieJour;
    S += String(DeltaEnergieJour) + ",";
  }
  return S;
}
unsigned long LectureCle() {
  return EEPROM.readULong(adr_ParaActions);
}
void LectureEnROM() {
  int Hdeb;
  int address = adr_ParaActions;
  int VersionStocke;
  Cle_ROM = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  VersionStocke = EEPROM.readUShort(address);
  address += sizeof(unsigned short);
  ssid = EEPROM.readString(address);
  address += ssid.length() + 1;
  password = EEPROM.readString(address);
  address += password.length() + 1;
  dhcpOn = EEPROM.readByte(address);
  address += sizeof(byte);
  RMS_IP[0] = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  Gateway = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  masque = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  dns = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  int VersionMajeur = int(VersionStocke / 100);
  String V = Version;
  int Vr = V.toInt();
  TelnetPrint("Version stockée (partie entière) :");
  TelnetPrintln(String(VersionMajeur));
  TelnetPrint("Version du logiciel( partie entière) :");
  TelnetPrintln(String(Vr));
  if (Vr == VersionMajeur) {  //La partie entière  ne change pas. On lit la suite
    CleAccesRef = EEPROM.readString(address);
    address += CleAccesRef.length() + 1;
    Couleurs = EEPROM.readString(address);
    address += Couleurs.length() + 1;
    ModePara = EEPROM.readByte(address);
    address += sizeof(byte);
    ModeReseau = EEPROM.readByte(address);
    address += sizeof(byte);
    Horloge = EEPROM.readByte(address);
    address += sizeof(byte);
    ESP32_Type = EEPROM.readByte(address);
    address += sizeof(byte);
    LEDgroupe = EEPROM.readByte(address);
    address += sizeof(byte);
    rotation = EEPROM.readByte(address);
    address += sizeof(byte);
    DurEcran = EEPROM.readULong(address);
    address += sizeof(unsigned long);
    for (int i = 0; i < 8; i++) {
      Calibre[i] = EEPROM.readShort(address);
      address += sizeof(unsigned short);
    }
    pUxI = EEPROM.readByte(address);
    address += sizeof(byte);
    pTemp = EEPROM.readByte(address);
    address += sizeof(byte);
    Source = EEPROM.readString(address);
    address += Source.length() + 1;
    RMSextIP = EEPROM.readULong(address);
    address += sizeof(unsigned long);
    EnphaseUser = EEPROM.readString(address);
    address += EnphaseUser.length() + 1;
    EnphasePwd = EEPROM.readString(address);
    address += EnphasePwd.length() + 1;
    EnphaseSerial = EEPROM.readString(address);
    address += EnphaseSerial.length() + 1;
    MQTTRepet = EEPROM.readUShort(address);
    address += sizeof(unsigned short);
    MQTTIP = EEPROM.readULong(address);
    address += sizeof(unsigned long);
    MQTTPort = EEPROM.readUShort(address);
    address += sizeof(unsigned short);
    MQTTUser = EEPROM.readString(address);
    address += MQTTUser.length() + 1;
    MQTTPwd = EEPROM.readString(address);
    address += MQTTPwd.length() + 1;
    MQTTPrefix = EEPROM.readString(address);
    address += MQTTPrefix.length() + 1;
    MQTTPrefixEtat = EEPROM.readString(address);
    address += MQTTPrefixEtat.length() + 1;
    MQTTdeviceName = EEPROM.readString(address);
    address += MQTTdeviceName.length() + 1;
    TopicP = EEPROM.readString(address);
    address += TopicP.length() + 1;
    subMQTT = EEPROM.readByte(address);
    address += sizeof(byte);
    nomRouteur = EEPROM.readString(address);
    address += nomRouteur.length() + 1;
    nomSondeFixe = EEPROM.readString(address);
    address += nomSondeFixe.length() + 1;
    nomSondeMobile = EEPROM.readString(address);
    address += nomSondeMobile.length() + 1;
    for (int i = 1; i < LesRouteursMax; i++) {
      RMS_IP[i] = EEPROM.readULong(address);
      address += sizeof(unsigned long);
    }
    for (int c = 0; c < 4; c++) {
      nomTemperature[c] = EEPROM.readString(address);
      address += nomTemperature[c].length() + 1;
      Source_Temp[c] = EEPROM.readString(address);
      address += Source_Temp[c].length() + 1;
      refTempIP[c] = EEPROM.readByte(address);
      address += sizeof(byte);
      TopicT[c] = EEPROM.readString(address);
      address += TopicT[c].length() + 1;
      canalTempExterne[c] = EEPROM.readByte(address);
      address += sizeof(byte);
      offsetTemp[c] = EEPROM.readShort(address);
      address += sizeof(unsigned short);
    }
    CalibU = EEPROM.readUShort(address);
    address += sizeof(unsigned short);
    CalibI = EEPROM.readUShort(address);
    address += sizeof(unsigned short);
    TempoRTEon = EEPROM.readByte(address);
    address += sizeof(byte);
    WifiSleep = EEPROM.readByte(address);
    address += sizeof(byte);
    ComSurv = EEPROM.readShort(address);
    address += sizeof(short);
    pSerial = EEPROM.readByte(address);
    address += sizeof(byte);
    pTriac = EEPROM.readByte(address);
    address += sizeof(byte);

    //Zone des actions
    ReacCACSI = EEPROM.readByte(address);
    address += sizeof(byte);
    Fpwm = EEPROM.readUShort(address);
    address += sizeof(unsigned short);
    NbActions = EEPROM.readUShort(address);
    address += sizeof(unsigned short);
    for (int iAct = 0; iAct < NbActions; iAct++) {
      LesActions[iAct].Actif = EEPROM.readByte(address);
      address += sizeof(byte);
      LesActions[iAct].Titre = EEPROM.readString(address);
      address += LesActions[iAct].Titre.length() + 1;
      LesActions[iAct].Host = EEPROM.readString(address);
      address += LesActions[iAct].Host.length() + 1;
      LesActions[iAct].Port = EEPROM.readUShort(address);
      address += sizeof(unsigned short);
      LesActions[iAct].OrdreOn = EEPROM.readString(address);
      address += LesActions[iAct].OrdreOn.length() + 1;
      LesActions[iAct].OrdreOn.replace(String((char)31), "|");  //Depuis la V10.01 (Input separator)
      LesActions[iAct].OrdreOff = EEPROM.readString(address);
      address += LesActions[iAct].OrdreOff.length() + 1;
      LesActions[iAct].Repet = EEPROM.readUShort(address);
      address += sizeof(unsigned short);
      LesActions[iAct].Tempo = EEPROM.readUShort(address);
      address += sizeof(unsigned short);
      LesActions[iAct].Reactivite = EEPROM.readByte(address);
      address += sizeof(byte);
      LesActions[iAct].NbPeriode = EEPROM.readByte(address);
      address += sizeof(byte);
      Hdeb = 0;
      for (byte i = 0; i < LesActions[iAct].NbPeriode; i++) {
        LesActions[iAct].Type[i] = EEPROM.readByte(address);
        address += sizeof(byte);
        LesActions[iAct].Hfin[i] = EEPROM.readUShort(address);
        LesActions[iAct].Hdeb[i] = Hdeb;
        Hdeb = LesActions[iAct].Hfin[i];
        address += sizeof(short);
        LesActions[iAct].Vmin[i] = EEPROM.readShort(address);
        address += sizeof(short);
        LesActions[iAct].Vmax[i] = EEPROM.readShort(address);
        address += sizeof(short);
        LesActions[iAct].Tinf[i] = EEPROM.readShort(address);
        address += sizeof(short);
        LesActions[iAct].Tsup[i] = EEPROM.readShort(address);
        address += sizeof(short);
        LesActions[iAct].Hmin[i] = EEPROM.readShort(address);
        address += sizeof(short);
        LesActions[iAct].Hmax[i] = EEPROM.readShort(address);
        address += sizeof(short);
        LesActions[iAct].CanalTemp[i] = EEPROM.readShort(address);
        address += sizeof(short);
        LesActions[iAct].SelAct[i] = EEPROM.readByte(address);
        address += sizeof(byte);
        LesActions[iAct].Ooff[i] = EEPROM.readByte(address);
        address += sizeof(byte);
        LesActions[iAct].O_on[i] = EEPROM.readByte(address);
        address += sizeof(byte);
        LesActions[iAct].Tarif[i] = EEPROM.readByte(address);
        address += sizeof(byte);
      }
    }
    Calibration(address);
  }
}
int EcritureEnROM() {
  int address = adr_ParaActions;
  int VersionStocke = 0;
  String V = Version;
  VersionStocke = int(100 * V.toFloat());
  EEPROM.writeULong(address, Cle_ROM);
  address += sizeof(unsigned long);
  EEPROM.writeUShort(address, VersionStocke);
  address += sizeof(unsigned short);
  EEPROM.writeString(address, ssid);
  address += ssid.length() + 1;
  EEPROM.writeString(address, password);
  address += password.length() + 1;
  EEPROM.writeByte(address, dhcpOn);
  address += sizeof(byte);
  EEPROM.writeULong(address, RMS_IP[0]);
  address += sizeof(unsigned long);
  EEPROM.writeULong(address, Gateway);
  address += sizeof(unsigned long);
  EEPROM.writeULong(address, masque);
  address += sizeof(unsigned long);
  EEPROM.writeULong(address, dns);
  address += sizeof(unsigned long);
  EEPROM.writeString(address, CleAccesRef);
  address += CleAccesRef.length() + 1;
  EEPROM.writeString(address, Couleurs);
  address += Couleurs.length() + 1;
  EEPROM.writeByte(address, ModePara);
  address += sizeof(byte);
  EEPROM.writeByte(address, ModeReseau);
  address += sizeof(byte);
  EEPROM.writeByte(address, Horloge);
  address += sizeof(byte);
  EEPROM.writeByte(address, ESP32_Type);
  address += sizeof(byte);
  EEPROM.writeByte(address, LEDgroupe);
  address += sizeof(byte);
  EEPROM.writeByte(address, rotation);
  address += sizeof(byte);
  EEPROM.writeULong(address, DurEcran);
  address += sizeof(unsigned long);
  for (int i = 0; i < 8; i++) {
    EEPROM.writeShort(address, Calibre[i]);
    address += sizeof(unsigned short);
  }
  EEPROM.writeByte(address, pUxI);
  address += sizeof(byte);
  EEPROM.writeByte(address, pTemp);
  address += sizeof(byte);
  EEPROM.writeString(address, Source);
  address += Source.length() + 1;
  EEPROM.writeULong(address, RMSextIP);
  address += sizeof(unsigned long);
  EEPROM.writeString(address, EnphaseUser);
  address += EnphaseUser.length() + 1;
  EEPROM.writeString(address, EnphasePwd);
  address += EnphasePwd.length() + 1;
  EEPROM.writeString(address, EnphaseSerial);
  address += EnphaseSerial.length() + 1;
  if (ModePara == 0) {
    MQTTRepet = 0;
    subMQTT = 0;
  }
  EEPROM.writeUShort(address, MQTTRepet);
  address += sizeof(unsigned short);
  EEPROM.writeULong(address, MQTTIP);
  address += sizeof(unsigned long);
  EEPROM.writeUShort(address, MQTTPort);
  address += sizeof(unsigned short);
  EEPROM.writeString(address, MQTTUser);
  address += MQTTUser.length() + 1;
  EEPROM.writeString(address, MQTTPwd);
  address += MQTTPwd.length() + 1;
  EEPROM.writeString(address, MQTTPrefix);
  address += MQTTPrefix.length() + 1;
  EEPROM.writeString(address, MQTTPrefixEtat);
  address += MQTTPrefixEtat.length() + 1;
  EEPROM.writeString(address, MQTTdeviceName);
  address += MQTTdeviceName.length() + 1;
  EEPROM.writeString(address, TopicP);
  address += TopicP.length() + 1;
  EEPROM.writeByte(address, subMQTT);
  address += sizeof(byte);
  EEPROM.writeString(address, nomRouteur);
  address += nomRouteur.length() + 1;
  EEPROM.writeString(address, nomSondeFixe);
  address += nomSondeFixe.length() + 1;
  EEPROM.writeString(address, nomSondeMobile);
  address += nomSondeMobile.length() + 1;
  for (int i = 1; i < LesRouteursMax; i++) {
    EEPROM.writeULong(address, RMS_IP[i]);
    address += sizeof(unsigned long);
  }
  for (int c = 0; c < 4; c++) {
    EEPROM.writeString(address, nomTemperature[c]);
    address += nomTemperature[c].length() + 1;
    EEPROM.writeString(address, Source_Temp[c]);
    address += Source_Temp[c].length() + 1;
    EEPROM.writeByte(address, refTempIP[c]);
    address += sizeof(byte);
    EEPROM.writeString(address, TopicT[c]);
    address += TopicT[c].length() + 1;
    EEPROM.writeByte(address, canalTempExterne[c]);
    address += sizeof(byte);
    EEPROM.writeShort(address, offsetTemp[c]);
    address += sizeof(unsigned short);
  }
  EEPROM.writeUShort(address, CalibU);
  address += sizeof(unsigned short);
  EEPROM.writeUShort(address, CalibI);
  address += sizeof(unsigned short);
  EEPROM.writeByte(address, TempoRTEon);
  address += sizeof(byte);
  EEPROM.writeByte(address, WifiSleep);
  address += sizeof(byte);
  EEPROM.writeShort(address, ComSurv);
  address += sizeof(unsigned short);
  EEPROM.writeByte(address, pSerial);
  address += sizeof(byte);
  EEPROM.writeByte(address, pTriac);
  address += sizeof(byte);
  //Enregistrement des Actions
  EEPROM.writeByte(address, ReacCACSI);
  address += sizeof(byte);
  EEPROM.writeUShort(address, Fpwm);
  address += sizeof(short);
  EEPROM.writeUShort(address, NbActions);
  address += sizeof(unsigned short);
  for (int iAct = 0; iAct < NbActions; iAct++) {
    EEPROM.writeByte(address, LesActions[iAct].Actif);
    address += sizeof(byte);
    EEPROM.writeString(address, LesActions[iAct].Titre);
    address += LesActions[iAct].Titre.length() + 1;
    EEPROM.writeString(address, LesActions[iAct].Host);
    address += LesActions[iAct].Host.length() + 1;
    EEPROM.writeUShort(address, LesActions[iAct].Port);
    address += sizeof(unsigned short);
    EEPROM.writeString(address, LesActions[iAct].OrdreOn);
    address += LesActions[iAct].OrdreOn.length() + 1;
    EEPROM.writeString(address, LesActions[iAct].OrdreOff);
    address += LesActions[iAct].OrdreOff.length() + 1;
    EEPROM.writeUShort(address, LesActions[iAct].Repet);
    address += sizeof(unsigned short);
    EEPROM.writeUShort(address, LesActions[iAct].Tempo);
    address += sizeof(unsigned short);
    EEPROM.writeByte(address, LesActions[iAct].Reactivite);
    address += sizeof(byte);
    EEPROM.writeByte(address, LesActions[iAct].NbPeriode);
    address += sizeof(byte);
    for (byte i = 0; i < LesActions[iAct].NbPeriode; i++) {
      if (ModePara == 0) {  //standard
        LesActions[iAct].CanalTemp[i] = -1;
        LesActions[iAct].SelAct[i] = 255;
      }
      EEPROM.writeByte(address, LesActions[iAct].Type[i]);
      address += sizeof(byte);
      EEPROM.writeUShort(address, LesActions[iAct].Hfin[i]);
      address += sizeof(short);
      EEPROM.writeShort(address, LesActions[iAct].Vmin[i]);
      address += sizeof(short);
      EEPROM.writeShort(address, LesActions[iAct].Vmax[i]);
      address += sizeof(short);
      EEPROM.writeShort(address, LesActions[iAct].Tinf[i]);
      address += sizeof(short);
      EEPROM.writeShort(address, LesActions[iAct].Tsup[i]);
      address += sizeof(short);
      EEPROM.writeShort(address, LesActions[iAct].Hmin[i]);
      address += sizeof(short);
      EEPROM.writeShort(address, LesActions[iAct].Hmax[i]);
      address += sizeof(short);
      EEPROM.writeShort(address, LesActions[iAct].CanalTemp[i]);
      address += sizeof(short);
      EEPROM.writeByte(address, LesActions[iAct].SelAct[i]);
      address += sizeof(byte);
      EEPROM.writeByte(address, LesActions[iAct].Ooff[i]);
      address += sizeof(byte);
      EEPROM.writeByte(address, LesActions[iAct].O_on[i]);
      address += sizeof(byte);
      EEPROM.writeByte(address, LesActions[iAct].Tarif[i]);
      address += sizeof(byte);
    }
  }
  Calibration(address);
  EEPROM.commit();
  return address;
}
void Calibration(int address) {
  kV = KV * float(CalibU) / 1000.0;  //Calibration coefficient to be applied
  kI = KI * float(CalibI) / 1000.0;
  P_cent_EEPROM = int(100 * address / EEPROM_SIZE);
  TelnetPrintln("Mémoire EEPROM utilisée : " + String(P_cent_EEPROM) + "%");
}

void init_puissance() {
  PuissanceS_T = 0;
  PuissanceS_M = 0;
  PuissanceI_T = 0;
  PuissanceI_M = 0;  //Puissance Watt affichée en entiers Maison et Triac
  PVAS_T = 0;
  PVAS_M = 0;
  PVAI_T = 0;
  PVAI_M = 0;  //Puissance VA affichée en entiers Maison et Triac
  PuissanceS_T_inst = 0.0;
  PuissanceS_M_inst = 0.0;
  PuissanceI_T_inst = 0.0;
  PuissanceI_M_inst = 0.0;
  PVAS_T_inst = 0.0;
  PVAS_M_inst = 0.0;
  PVAI_T_inst = 0.0;
  PVAI_M_inst = 0.0;
  Puissance_T_moy = 0.0;
  Puissance_M_moy = 0.0;
  PVA_T_moy = 0.0;
  PVA_M_moy = 0.0;
}
void filtre_puissance() {  //Filtre RC

  float A = 0.3;  //Coef pour un lissage en multi-sinus et train de sinus sur les mesures de puissance courte
  float B = 0.7;
  if (!LissageLong) {
    A = 1;
    B = 0;
  }

  Puissance_T_moy = A * (PuissanceS_T_inst - PuissanceI_T_inst) + B * Puissance_T_moy;
  if (Puissance_T_moy < 0) {
    PuissanceI_T = -int(Puissance_T_moy);  //Puissance Watt affichée en entier  Triac
    PuissanceS_T = 0;
  } else {
    PuissanceS_T = int(Puissance_T_moy);
    PuissanceI_T = 0;
  }



  Puissance_M_moy = A * (PuissanceS_M_inst - PuissanceI_M_inst) + B * Puissance_M_moy;
  if (Puissance_M_moy < 0) {
    PuissanceI_M = -int(Puissance_M_moy);  //Puissance Watt affichée en entier Maison
    PuissanceS_M = 0;
  } else {
    PuissanceS_M = int(Puissance_M_moy);
    PuissanceI_M = 0;
  }


  PVA_T_moy = A * (PVAS_T_inst - PVAI_T_inst) + B * PVA_T_moy;  //Puissance VA affichée en entiers
  if (PVA_T_moy < 0) {
    PVAI_T = -int(PVA_T_moy);
    PVAS_T = 0;
  } else {
    PVAS_T = int(PVA_T_moy);
    PVAI_T = 0;
  }

  PVA_M_moy = A * (PVAS_M_inst - PVAI_M_inst) + B * PVA_M_moy;
  if (PVA_M_moy < 0) {
    PVAI_M = -int(PVA_M_moy);
    PVAS_M = 0;
  } else {
    PVAS_M = int(PVA_M_moy);
    PVAI_M = 0;
  }
}

void StockMessage(String m) {
  m = DATE + " : " + m;
  MessageH[idxMessage] = m;
  idxMessage = (idxMessage + 1) % 10;
  PrintScroll(m);
}



//Fichier parametres à dowloader
String Fichier_parametres(String ip, String para, String action) {
  byte NbPeriode;
  String S = "{\"Routeur\":\"F1ATB\"";
  String V = Version;
  int VersionStocke = int(100 * V.toFloat());
  S += AddInt("VersionStocke", VersionStocke);
  if (ip == "true") {
    S += AddStr("ssid", ssid) + AddStr("password", password) + AddByte("dhcpOn", dhcpOn) + AddUlong("IP_Fixe", RMS_IP[0]);
    S += AddUlong("Gateway", Gateway) + AddUlong("masque", masque) + AddUlong("dns", dns);
  }
  if (para == "true") {
    S += AddStr("CleAccesRef", CleAccesRef) + AddStr("Couleurs", Couleurs);
    S += AddByte("ModePara", ModePara) + AddByte("ModeReseau", ModeReseau) + AddByte("Horloge", Horloge);
    S += AddByte("ESP32_Type", ESP32_Type) + AddByte("LEDgroupe", LEDgroupe) + AddByte("rotation", rotation) + AddUlong("DurEcran", DurEcran);
    for (int i = 0; i < 8; i++) {
      S += AddUshort("Calibre" + String(i), Calibre[i]);
    }
    S += AddByte("pUxI", pUxI) + AddByte("pTemp", pTemp);
    S += AddStr("Source", Source) + AddUlong("RMSextIP", RMSextIP) + AddStr("EnphaseUser", EnphaseUser) + AddStr("EnphasePwd", EnphasePwd) + AddStr("EnphaseSerial", EnphaseSerial);
    S += AddUshort("MQTTRepet", MQTTRepet) + AddUlong("MQTTIP", MQTTIP) + AddUshort("MQTTPort", MQTTPort);
    S += AddStr("MQTTUser", MQTTUser) + AddStr("MQTTPwd", MQTTPwd) + AddStr("MQTTPrefix", MQTTPrefix) + AddStr("MQTTPrefixEtat", MQTTPrefixEtat);
    S += AddStr("MQTTdeviceName", MQTTdeviceName) + AddStr("TopicP", TopicP);
    S += AddByte("subMQTT", subMQTT) + AddStr("nomRouteur", nomRouteur) + AddStr("nomSondeFixe", nomSondeFixe) + AddStr("nomSondeMobile", nomSondeMobile);
    for (int i = 1; i < LesRouteursMax; i++) {
      S += AddUlong("RMS_IP" + String(i), RMS_IP[i]);
    }
    for (int c = 0; c < 4; c++) {
      S += AddStr("nomTemperature" + String(c), nomTemperature[c]) + AddStr("Source_Temp" + String(c), Source_Temp[c]) + AddByte("refTempIP" + String(c), refTempIP[c]);
      S += AddStr("TopicT" + String(c), TopicT[c]) + AddByte("canalTempExterne" + String(c), canalTempExterne[c]) + AddInt("offsetTemp" + String(c), offsetTemp[c]);
    }
    S += AddUshort("CalibU", CalibU) + AddUshort("CalibI", CalibI);
    S += AddByte("TempoRTEon", TempoRTEon) + AddByte("WifiSleep", WifiSleep) + AddInt("ComSurv", ComSurv) + AddByte("pSerial", pSerial) + AddByte("pTriac", pTriac);
  }
  if (action == "true") {
    S += AddByte("ReacCACSI", ReacCACSI) + AddUshort("Fpwm", Fpwm);
    S += AddUshort("NbActions", NbActions) + ",\"Actions\":[";
    for (int iAct = 0; iAct < NbActions; iAct++) {
      S += "{\"Action\":" + String(iAct) + AddByte("Actif", LesActions[iAct].Actif) + AddStr("Titre", LesActions[iAct].Titre);
      S += AddStr("Host", LesActions[iAct].Host) + AddUshort("Port", LesActions[iAct].Port) + AddStr("OrdreOn", LesActions[iAct].OrdreOn) + AddStr("OrdreOff", LesActions[iAct].OrdreOff);
      S += AddUshort("Repet", LesActions[iAct].Repet) + AddUshort("Tempo", LesActions[iAct].Tempo) + AddByte("Reactivite", LesActions[iAct].Reactivite);
      NbPeriode = LesActions[iAct].NbPeriode;
      S += AddByte("NbPeriode", NbPeriode) + ",\"Périodes\":[";
      for (byte i = 0; i < NbPeriode; i++) {
        S += "{\"Periode\":" + String(i) + AddByte("Type", LesActions[iAct].Type[i]);
        S += AddInt("Hfin", LesActions[iAct].Hfin[i]) + AddInt("Vmin", LesActions[iAct].Vmin[i]) + AddInt("Vmax", LesActions[iAct].Vmax[i]);
        S += AddInt("Tinf", LesActions[iAct].Tinf[i]) + AddInt("Tsup", LesActions[iAct].Tsup[i]);
        S += AddInt("Hmin", LesActions[iAct].Hmin[i]) + AddInt("Hmax", LesActions[iAct].Hmax[i]);
        S += AddUshort("CanalTemp", LesActions[iAct].CanalTemp[i]);
        S += AddByte("SelAct", LesActions[iAct].SelAct[i]);
        S += AddByte("Ooff", LesActions[iAct].Ooff[i]) + AddByte("O_on", LesActions[iAct].O_on[i]);
        S += AddByte("Tarif", LesActions[iAct].Tarif[i]) + "}";
        if (i != NbPeriode - 1) S += ",";
      }

      S += "]}";
      if (iAct != NbActions - 1) S += ",";
    }
    S += "]";
  }
  S += "}";
  return S;
}

String AddInt(String nom, int valeur) {
  return ",\"" + nom + "\":" + String(valeur);
}
String AddByte(String nom, byte valeur) {
  return ",\"" + nom + "\":" + String(valeur);
}
String AddUlong(String nom, unsigned long valeur) {
  return ",\"" + nom + "\":" + String(valeur);
}
String AddUshort(String nom, unsigned short valeur) {
  return ",\"" + nom + "\":" + String(valeur);
}

String AddStr(String nom, String valeur) {
  return ",\"" + nom + "\":\"" + valeur + "\"";
}
//Importation des paramètres
//***************************
void ImportParametres(String Conf) {
  int Hdeb;
  int VersionStocke = IntJson("VersionStocke", Conf);
  if (Conf.indexOf("\"ssid\":") > 0) {  //On a les données IP
    ssid = StringJson("ssid", Conf);
    password = StringJson("password", Conf);
    dhcpOn = ByteJson("dhcpOn", Conf);
    RMS_IP[0] = ULongJson("IP_Fixe", Conf);
    Gateway = ULongJson("Gateway", Conf);
    masque = ULongJson("masque", Conf);
    dns = ULongJson("dns", Conf);
  }
  if (Conf.indexOf("\"Source\":") > 0) {  //Autres parametres
    CleAccesRef = StringJson("CleAccesRef", Conf);
    Couleurs = StringJson("Couleurs", Conf);
    ModePara = ByteJson("ModePara", Conf);
    ModeReseau = ByteJson("ModeReseau", Conf);
    Horloge = ByteJson("Horloge", Conf);
    ESP32_Type = ByteJson("ESP32_Type", Conf);
    LEDgroupe = ByteJson("LEDgroupe", Conf);
    rotation = ByteJson("rotation", Conf);
    DurEcran = ULongJson("DurEcran", Conf);
    for (int i = 0; i < 8; i++) {
      Calibre[i] = UShortJson("Calibre" + String(i), Conf);
    }
    pUxI = ByteJson("pUxI", Conf);
    pTemp = ByteJson("pTemp", Conf);
    Source = StringJson("Source", Conf);
    RMSextIP = ULongJson("RMSextIP", Conf);
    EnphaseUser = StringJson("EnphaseUser", Conf);
    EnphasePwd = StringJson("EnphasePwd", Conf);
    EnphaseSerial = StringJson("EnphaseSerial", Conf);
    MQTTRepet = UShortJson("MQTTRepet", Conf);
    MQTTIP = ULongJson("MQTTIP", Conf);
    MQTTPort = UShortJson("MQTTPort", Conf);
    MQTTUser = StringJson("MQTTUser", Conf);
    MQTTPwd = StringJson("MQTTPwd", Conf);
    MQTTPrefix = StringJson("MQTTPrefix", Conf);
    MQTTPrefixEtat = StringJson("MQTTPrefixEtat", Conf);
    MQTTdeviceName = StringJson("MQTTdeviceName", Conf);
    TopicP = StringJson("TopicP", Conf);
    subMQTT = ByteJson("subMQTT", Conf);
    nomRouteur = StringJson("nomRouteur", Conf);
    nomSondeFixe = StringJson("nomSondeFixe", Conf);
    nomSondeMobile = StringJson("nomSondeMobile", Conf);
    //V11 uniquement. Une seule température
    if (Conf.indexOf("\"nomTemperature\":") > 0) {  //On a la temperarure
      nomTemperature[0] = StringJson("nomTemperature", Conf);
      Source_Temp[0] = StringJson("Source_Temp", Conf);
      RMS_IP[1] = ULongJson("IPtemp", Conf);
      refTempIP[0] = 1;
      TopicT[0] = StringJson("TopicT", Conf);
    }
    //V12
    for (int i = 1; i < LesRouteursMax; i++) {
      RMS_IP[i] = ULongJson("RMS_IP" + String(i), Conf);
    }
    if (Conf.indexOf("\"nomTemperature0\":") > 0) {  //On a les temperatures
      for (int c = 0; c < 4; c++) {
        nomTemperature[c] = StringJson("nomTemperature" + String(c), Conf);
        Source_Temp[c] = StringJson("Source_Temp" + String(c), Conf);
        refTempIP[c] = ByteJson("refTempIP" + String(c), Conf);
        TopicT[c] = StringJson("TopicT" + String(c), Conf);
        canalTempExterne[c] = ByteJson("canalTempExterne" + String(c), Conf);
        offsetTemp[c] = ShortJson("offsetTemp" + String(c), Conf);
      }
    }
    CalibU = UShortJson("CalibU", Conf);
    CalibI = UShortJson("CalibI", Conf);
    TempoRTEon = ByteJson("TempoRTEon", Conf);
    WifiSleep = ByteJson("WifiSleep", Conf);
    ComSurv = ShortJson("ComSurv", Conf);
    if (ComSurv < 6) ComSurv = 6;
    pSerial = ByteJson("pSerial", Conf);
    pTriac = ByteJson("pTriac", Conf);
  }
  if (Conf.indexOf("\"NbActions\":") > 0) {  //ACTIONS
    ReacCACSI = ByteJson("ReacCACSI", Conf);
    if (ReacCACSI < 1) ReacCACSI = 1;
    Fpwm = UShortJson("Fpwm", Conf);
    if (Fpwm < 5) Fpwm = 500;
    NbActions = UShortJson("NbActions", Conf);
    for (int iAct = 0; iAct < NbActions; iAct++) {
      int p = Conf.indexOf("{\"Action\":");
      Conf = Conf.substring(p + 10);  //On enlève les precédents
      LesActions[iAct].Actif = ByteJson("Actif", Conf);
      LesActions[iAct].Titre = StringJson("Titre", Conf);
      LesActions[iAct].Host = StringJson("Host", Conf);
      LesActions[iAct].Port = UShortJson("Port", Conf);
      LesActions[iAct].OrdreOn = StringJson("OrdreOn", Conf);
      LesActions[iAct].OrdreOff = StringJson("OrdreOff", Conf);
      LesActions[iAct].Repet = UShortJson("Repet", Conf);
      LesActions[iAct].Tempo = UShortJson("Tempo", Conf);
      LesActions[iAct].Reactivite = ByteJson("Reactivite", Conf);
      LesActions[iAct].NbPeriode = ByteJson("NbPeriode", Conf);
      Hdeb = 0;
      for (byte i = 0; i < LesActions[iAct].NbPeriode; i++) {
        int p = Conf.indexOf("{\"Periode\":");
        Conf = Conf.substring(p + 11);  //On enlève les precédents
        LesActions[iAct].Type[i] = ByteJson("Type", Conf);
        LesActions[iAct].Hfin[i] = UShortJson("Hfin", Conf);
        LesActions[iAct].Hdeb[i] = Hdeb;
        Hdeb = LesActions[iAct].Hfin[i];
        LesActions[iAct].Vmin[i] = ShortJson("Vmin", Conf);
        LesActions[iAct].Vmax[i] = ShortJson("Vmax", Conf);
        LesActions[iAct].Tinf[i] = ShortJson("Tinf", Conf);
        LesActions[iAct].Tsup[i] = ShortJson("Tsup", Conf);
        LesActions[iAct].Hmin[i] = ShortJson("Hmin", Conf);
        LesActions[iAct].Hmax[i] = ShortJson("Hmax", Conf);
        LesActions[iAct].CanalTemp[i] = UShortJson("CanalTemp", Conf);  //V13
        LesActions[iAct].SelAct[i] = ByteJson("SelAct", Conf);          //V13
        LesActions[iAct].Ooff[i] = ByteJson("Ooff", Conf);
        LesActions[iAct].O_on[i] = ByteJson("O_on", Conf);
        LesActions[iAct].Tarif[i] = ByteJson("Tarif", Conf);
      }
    }
  }

  EcritureEnROM();
}
void SplitS(String Str, String &Before, String Separ, String &After) {
  int p = Str.indexOf(Separ);
  Before = Str.substring(0, p);
  After = Str.substring(p + 1);
}
// Conversion des adresses IP suivant le coeur

String IP2String(unsigned long IP) {
  byte arr[4];
  arr[0] = IP & 0xFF;            // 0x78
  arr[1] = (IP >> 8) & 0xFF;     // 0x56
  arr[2] = (IP >> 16) & 0xFF;    // 0x34
  arr[3] = (IP >> 24) & 0xFF;    // 0x12
  for (int i = 0; i < 4; i++) {  //Pour eviter corruption données entre coeur 0 et coeur1 (arr en variable local)
    arrIP[i] = arr[i];           //Pour le WIFI au setup
  }
  return String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
}
unsigned long String2IP(String S) {
  unsigned long IP = 0;
  for (int i = 0; i < 3; i++) {
    int p = S.indexOf(".");
    String s = S.substring(0, p);
    S = S.substring(p + 1);
    IP += s.toInt();
    IP = IP * 256;
  }
  IP += S.toInt();
  return IP;
}

//Gestion couleurs
String ULtoHex(unsigned long x) {
  char buffer[15];
  ltoa(x, buffer, 16);
  String S = "000000" + String(buffer);
  int p = S.length();
  S = "#" + S.substring(p - 6);  //Format pour HTML color
  return S;
}
unsigned long ConvCouleur(String V) {  //Notation CSS en UL
  return strtoul(V.c_str(), NULL, 16);
}

//Telnet et Serial
//****************

// --- Fonction de sortie partagée ---
void TelnetPrint(const String &message) {
  Serial.print(message);  // Sortie sur port série
  if (telnetClient && telnetClient.connected()) {
    telnetClient.print(message);  // Sortie sur Telnet
  }
}
void TelnetPrintln(const String &message) {
  Serial.println(message);
  if (telnetClient && telnetClient.connected()) {
    telnetClient.println(message);
  }
}
// PORT SERIE ou TELNET
void LireSerial() {
  int inbyte;
  //Port Serie
  while (Serial.available() > 0) {
    inbyte = Serial.read();


    if ((inbyte == 10) || (inbyte == 13)) {
      DecodeSerial();
    } else {
      SerialIn += String(char(inbyte));
    }
  }
  //Telnet
  if (telnetClient && telnetClient.connected() && telnetClient.available()) {
    char c = telnetClient.read();  // Lire caractère
    if (c == '\n' || c == '\r') {  // Fin de ligne → commande complète
      DecodeSerial();
    } else {
      SerialIn += c;  // Ajouter caractère au buffer
    }
  }
}
void DecodeSerial() {
  String sw;
  String valeur = "";
  int p;
  SerialIn.trim();
  TelnetPrintln(SerialIn);
  p = SerialIn.indexOf(":");
  if (p > 0) {
    sw = SerialIn.substring(0, p);
    valeur = SerialIn.substring(p + 1);
    sw.trim();
    valeur.trim();
  } else {
    sw = SerialIn;
  }

  if (sw.indexOf("restart") >= 0) {
    ESP.restart();
  }
  if (sw.indexOf("ssid") >= 0) {
    ssid = valeur;
    ModeReseau = 0;          // A priori
    dhcpOn = 1;              //Au cas ou l'on change de mapping des adresses LAN
    if (ESP32_Type >= 10) {  //Carte Ethernet
      ESP32_Type = 0;
    }
    EcritureEnROM();
  }
  if (sw.indexOf("password") >= 0) {
    password = valeur;
    EcritureEnROM();
  }
  if (sw.indexOf("ETH01") >= 0) {
    ESP32_Type = 10;
    EcritureEnROM();
  }
  if (sw.indexOf("dispPw") >= 0) {
    dispPw = !dispPw;
  }
  if (sw.indexOf("dispAct") >= 0) {
    dispAct = !dispAct;
  }
  // dispAct
  SerialIn = "";
}
void MessageCommandes(){
  telnetClient.println("");
  telnetClient.println("*** Commandes par le port série USB ou Telnet port 23 ***");
  telnetClient.println("ssid:xxx     | pour définir le nom xxx du Wifi à utiliser");
  telnetClient.println("password:yyy | pour définir le mot de passe yyy du Wifi");
  telnetClient.println("restart      | pour redémarrer l'ESP32");
  telnetClient.println("dispPw       | pour afficher les mesures de puissance Pw");
  telnetClient.println("dispAct      | pour afficher les ouvertures des Actions");
}