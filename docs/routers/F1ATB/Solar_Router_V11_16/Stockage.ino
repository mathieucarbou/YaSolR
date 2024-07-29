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
  DateCeJour = EEPROM.readString(adr_DateCeJour);
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
  if (DATEvalid) {
    //Time Update / de l'heure
    time_t timestamp = time(NULL);
    char buffer[MAX_SIZE_T];
    struct tm *pTime = localtime(&timestamp);
    strftime(buffer, MAX_SIZE_T, "%d/%m/%Y %H:%M:%S", pTime);
    DATE = String(buffer);
    strftime(buffer, MAX_SIZE_T, "%d%m%Y", pTime);
    String JourCourant = String(buffer);
    strftime(buffer, MAX_SIZE_T, "%Y-%m-%d", pTime);
    DateEDF = String(buffer);
    strftime(buffer, MAX_SIZE_T, "%H", pTime);
    int hour = String(buffer).toInt();
    strftime(buffer, MAX_SIZE_T, "%M", pTime);
    int minute = String(buffer).toInt();
    strftime(buffer, MAX_SIZE_T, "%s", pTime);
    unsigned long Tactu = String(buffer).toInt();
    if (T0_seconde == 0) T0_seconde = Tactu;
    T_On_seconde = Tactu - T0_seconde;
    HeureCouranteDeci = hour * 100 + minute * 10 / 6;
    if (DateCeJour != JourCourant) {                  //Changement de jour
      if (EnergieActiveValide && DateCeJour != "") {  //Données recues
        idxPromDuJour = (idxPromDuJour + 1 + NbJour) % NbJour;
        //On enregistre les conso en début de journée pour l'historique de l'année
        long energie = Energie_M_Soutiree - Energie_M_Injectee;  //Bilan energie du jour
        EEPROM.writeLong(idxPromDuJour * 4, energie);
        EEPROM.writeULong(adr_E_T_soutire0, long(Energie_T_Soutiree));
        EEPROM.writeULong(adr_E_T_injecte0, long(Energie_T_Injectee));
        EEPROM.writeULong(adr_E_M_soutire0, long(Energie_M_Soutiree));
        EEPROM.writeULong(adr_E_M_injecte0, long(Energie_M_Injectee));
        EEPROM.writeString(adr_DateCeJour, JourCourant);
        EEPROM.writeUShort(adr_lastStockConso, idxPromDuJour);
        EEPROM.commit();
        LectureConsoMatinJour();
      }
      DateCeJour = JourCourant;
    }
    if (HeureCouranteDeci >= 599 && HeureCouranteDeci <= 600) {
      for (int i = 0; i < LesActionsLength; i++) {
        H_Ouvre[i] = 0;  //RAZ temps equivalent ouverture à 6h du matin
      }
    }
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
  IP_Fixe = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  Gateway = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  masque = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  dns = EEPROM.readULong(address);
  address += sizeof(unsigned long);
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
  MQTTdeviceName = EEPROM.readString(address);
  address += MQTTdeviceName.length() + 1;
  TopicP = EEPROM.readString(address);
  address += TopicP.length() + 1;
  TopicT = EEPROM.readString(address);
  address += TopicT.length() + 1;
  subMQTT = EEPROM.readByte(address);
  address += sizeof(byte);
  nomRouteur = EEPROM.readString(address);
  address += nomRouteur.length() + 1;
  nomSondeFixe = EEPROM.readString(address);
  address += nomSondeFixe.length() + 1;
  nomSondeMobile = EEPROM.readString(address);
  address += nomSondeMobile.length() + 1;
  nomTemperature = EEPROM.readString(address);
  address += nomTemperature.length() + 1;
  Source_Temp = EEPROM.readString(address);
  address += Source_Temp.length() + 1;
  IPtemp = EEPROM.readULong(address);
  address += sizeof(unsigned long);
  CalibU = EEPROM.readUShort(address);
  address += sizeof(unsigned short);
  CalibI = EEPROM.readUShort(address);
  address += sizeof(unsigned short);
  TempoEDFon = EEPROM.readByte(address);
  address += sizeof(byte);
  WifiSleep = EEPROM.readByte(address);
  address += sizeof(byte);
  pSerial = EEPROM.readByte(address);
  address += sizeof(byte);
  pTriac = EEPROM.readByte(address);
  address += sizeof(byte);

  address += 100;  //Réserve de 100 bytes

  //Zone des actions
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
    address += 40;  //Réserve de 40 bytes
    LesActions[iAct].NbPeriode = EEPROM.readByte(address);
    address += sizeof(byte);
    Hdeb = 0;
    for (byte i = 0; i < LesActions[iAct].NbPeriode; i++) {
      LesActions[iAct].Type[i] = EEPROM.readByte(address);
      address += sizeof(byte);
      LesActions[iAct].Hfin[i] = EEPROM.readUShort(address);
      LesActions[iAct].Hdeb[i] = Hdeb;
      Hdeb = LesActions[iAct].Hfin[i];
      address += sizeof(unsigned short);
      LesActions[iAct].Vmin[i] = EEPROM.readShort(address);
      address += sizeof(unsigned short);
      LesActions[iAct].Vmax[i] = EEPROM.readShort(address);
      address += sizeof(unsigned short);
      LesActions[iAct].Tinf[i] = EEPROM.readShort(address);
      address += sizeof(unsigned short);
      LesActions[iAct].Tsup[i] = EEPROM.readShort(address);
      address += sizeof(unsigned short);
      LesActions[iAct].Tarif[i] = EEPROM.readByte(address);
      address += sizeof(byte);
      address += 10;  //Réserve de 10 bytes
    }
  }
  Calibration(address);
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
  EEPROM.writeULong(address, IP_Fixe);
  address += sizeof(unsigned long);
  EEPROM.writeULong(address, Gateway);
  address += sizeof(unsigned long);
  EEPROM.writeULong(address, masque);
  address += sizeof(unsigned long);
  EEPROM.writeULong(address, dns);
  address += sizeof(unsigned long);
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
  EEPROM.writeString(address, MQTTdeviceName);
  address += MQTTdeviceName.length() + 1;
  EEPROM.writeString(address, TopicP);
  address += TopicP.length() + 1;
  EEPROM.writeString(address, TopicT);
  address += TopicT.length() + 1;
  EEPROM.writeByte(address, subMQTT);
  address += sizeof(byte);
  EEPROM.writeString(address, nomRouteur);
  address += nomRouteur.length() + 1;
  EEPROM.writeString(address, nomSondeFixe);
  address += nomSondeFixe.length() + 1;
  EEPROM.writeString(address, nomSondeMobile);
  address += nomSondeMobile.length() + 1;
  EEPROM.writeString(address, nomTemperature);
  address += nomTemperature.length() + 1;
  EEPROM.writeString(address, Source_Temp);
  address += Source_Temp.length() + 1;
  EEPROM.writeULong(address, IPtemp);
  address += sizeof(unsigned long);
  EEPROM.writeUShort(address, CalibU);
  address += sizeof(unsigned short);
  EEPROM.writeUShort(address, CalibI);
  address += sizeof(unsigned short);
  EEPROM.writeByte(address, TempoEDFon);
  address += sizeof(byte);
  EEPROM.writeByte(address, WifiSleep);
  address += sizeof(byte);
  EEPROM.writeByte(address, pSerial);
  address += sizeof(byte);
  EEPROM.writeByte(address, pTriac);
  address += sizeof(byte);

  address += 100;  //Réserve de 100 bytes

  //Enregistrement des Actions
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
    address += 40;  //Réserve de 40 bytes
    EEPROM.writeByte(address, LesActions[iAct].NbPeriode);
    address += sizeof(byte);
    for (byte i = 0; i < LesActions[iAct].NbPeriode; i++) {
      EEPROM.writeByte(address, LesActions[iAct].Type[i]);
      address += sizeof(byte);
      EEPROM.writeUShort(address, LesActions[iAct].Hfin[i]);
      address += sizeof(unsigned short);
      EEPROM.writeShort(address, LesActions[iAct].Vmin[i]);
      address += sizeof(unsigned short);
      EEPROM.writeShort(address, LesActions[iAct].Vmax[i]);
      address += sizeof(unsigned short);
      EEPROM.writeShort(address, LesActions[iAct].Tinf[i]);
      address += sizeof(unsigned short);
      EEPROM.writeShort(address, LesActions[iAct].Tsup[i]);
      address += sizeof(unsigned short);
      EEPROM.writeByte(address, LesActions[iAct].Tarif[i]);
      address += sizeof(byte);
      address += 10;  //Réserve de 10 bytes
    }
  }
  Calibration(address);
  EEPROM.commit();
  return address;
}
void Calibration(int address) {
  kV = KV * CalibU / 1000;  //Calibration coefficient to be applied
  kI = KI * CalibI / 1000;
  P_cent_EEPROM = int(100 * address / EEPROM_SIZE);
  Serial.println("Mémoire EEPROM utilisée : " + String(P_cent_EEPROM) + "%");
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
  Serial.println(m);
  MessageH[idxMessage] = m;
  idxMessage = (idxMessage + 1) % 10;
}

// PORT SERIE
void LireSerial() {
  int inbyte;
  while (Serial.available() > 0) {
    inbyte = Serial.read();
    String sw;
    String valeur = "";
    int p;

    if (inbyte == 13) {
      SerialIn.trim();
      Serial.println(SerialIn);
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
        EcritureEnROM();
      }
      if (sw.indexOf("password") >= 0) {
        password = valeur;
        EcritureEnROM();
      }


      SerialIn = "";
    } else {
      SerialIn += String(char(inbyte));
    }
  }
}

//Fichier parametres à dowloader
String Fichier_parametres(String ip, String para, String action) {
  byte NbPeriode;
  String S = "{\"Routeur\":\"F1ATB\"";
  String V = Version;
  int VersionStocke = int(100 * V.toFloat());
  S += AddInt("VersionStocke", VersionStocke);
  if (ip == "true") {
    S += AddStr("ssid", ssid) + AddStr("password", password) + AddByte("dhcpOn", dhcpOn) + AddUlong("IP_Fixe", IP_Fixe);
    S += AddUlong("Gateway", Gateway) + AddUlong("masque", masque) + AddUlong("dns", dns);
  }
  if (para == "true") {
    S += AddStr("Source", Source) + AddUlong("RMSextIP", RMSextIP) + AddStr("EnphaseUser", EnphaseUser) + AddStr("EnphasePwd", EnphasePwd) + AddStr("EnphaseSerial", EnphaseSerial);
    S += AddUshort("MQTTRepet", MQTTRepet) + AddUlong("MQTTIP", MQTTIP) + AddUshort("MQTTPort", MQTTPort);
    S += AddStr("MQTTUser", MQTTUser) + AddStr("MQTTPwd", MQTTPwd) + AddStr("MQTTPrefix", MQTTPrefix);
    S += AddStr("MQTTdeviceName", MQTTdeviceName) + AddStr("TopicP", TopicP) + AddStr("TopicT", TopicT);
    S += AddByte("subMQTT", subMQTT) + AddStr("nomRouteur", nomRouteur) + AddStr("nomSondeFixe", nomSondeFixe);
    S += AddStr("nomSondeMobile", nomSondeMobile) + AddStr("nomTemperature", nomTemperature) + AddStr("Source_Temp", Source_Temp);
    S += AddUlong("IPtemp", IPtemp) + AddUshort("CalibU", CalibU) + AddUshort("CalibI", CalibI);
    S += AddByte("TempoEDFon", TempoEDFon) + AddByte("WifiSleep", WifiSleep) + AddByte("pSerial", pSerial) + AddByte("pTriac", pTriac);
  }
  if (action == "true") {
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
        S += AddInt("Tinf", LesActions[iAct].Tinf[i]) + AddInt("Tsup", LesActions[iAct].Tsup[i]) + AddByte("Tarif", LesActions[iAct].Tarif[i]) + "}";
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
    IP_Fixe = ULongJson("IP_Fixe", Conf);
    Gateway = ULongJson("Gateway", Conf);
    masque = ULongJson("masque", Conf);
    dns = ULongJson("dns", Conf);
  }
  if (Conf.indexOf("\"Source\":") > 0) {  //Autres parametres
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
    MQTTdeviceName = StringJson("MQTTdeviceName", Conf);
    TopicP = StringJson("TopicP", Conf);
    TopicT = StringJson("TopicT", Conf);
    subMQTT = ByteJson("subMQTT", Conf);
    nomRouteur = StringJson("nomRouteur", Conf);
    nomSondeFixe = StringJson("nomSondeFixe", Conf);
    nomSondeMobile = StringJson("nomSondeMobile", Conf);
    nomTemperature = StringJson("nomTemperature", Conf);
    Source_Temp = StringJson("Source_Temp", Conf);
    IPtemp = ULongJson("IPtemp", Conf);
    CalibU = UShortJson("CalibU", Conf);
    CalibI = UShortJson("CalibI", Conf);
    TempoEDFon = ByteJson("TempoEDFon", Conf);
    WifiSleep = ByteJson("WifiSleep", Conf);
    pSerial = ByteJson("pSerial", Conf);
    pTriac = ByteJson("pTriac", Conf);
  }
  if (Conf.indexOf("\"NbActions\":") > 0) {  //ACTIONS
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
        LesActions[iAct].Hfin[i] = IntJson("Hfin", Conf);
        LesActions[iAct].Hdeb[i] = Hdeb;
        Hdeb = LesActions[iAct].Hfin[i];
        LesActions[iAct].Vmin[i] = IntJson("Vmin", Conf);
        LesActions[iAct].Vmax[i] = IntJson("Vmax", Conf);
        LesActions[iAct].Tinf[i] = IntJson("Tinf", Conf);
        LesActions[iAct].Tsup[i] = IntJson("Tsup", Conf);
        LesActions[iAct].Tarif[i] = ByteJson("Tarif", Conf);
      }
    }
  }

  EcritureEnROM();
}
