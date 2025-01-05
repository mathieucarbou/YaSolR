// ***************
// *  WEB SERVER *
// ***************
bool opened = false;
String ConfImport;
void Init_Server() {
  //Init Web Server on port 80
  server.on("/", handleRoot);
  server.on("/MainJS", handleMainJS);
  server.on("/Para", handlePara);
  server.on("/ParaJS", handleParaJS);
  server.on("/ParaRouteurJS", handleParaRouteurJS);
  server.on("/ParaAjax", handleParaAjax);
  server.on("/ParaRouteurAjax", handleParaRouteurAjax);
  server.on("/ParaUpdate", handleParaUpdate);
  server.on("/CleUpdate", handleCleUpdate);
  server.on("/Actions", handleActions);
  server.on("/ActionsJS", handleActionsJS);
  server.on("/ActionsJS2", handleActionsJS2);
  server.on("/ActionsJS3", handleActionsJS3);
  server.on("/ActionsUpdate", handleActionsUpdate);
  server.on("/ActionsAjax", handleActionsAjax);
  server.on("/Brute", handleBrute);
  server.on("/BruteJS", handleBruteJS);
  server.on("/BruteJS2", handleBruteJS2);
  server.on("/ajax_histo48h", handleAjaxHisto48h);
  server.on("/ajax_histo1an", handleAjaxHisto1an);
  server.on("/ajax_dataRMS", handleAjaxRMS);
  server.on("/ajax_dataESP32", handleAjaxESP32);
  server.on("/ajax_data", handleAjaxData);
  server.on("/ajax_data10mn", handleAjaxData10mn);
  server.on("/ajax_etatActions", handleAjax_etatActions);
  server.on("/ajax_etatActionX", handleAjax_etatActionX);
  server.on("/ajax_Temperature", handleAjaxTemperature);
  server.on("/ajax_Noms", handleAjaxNoms);
  server.on("/ajaxRAZhisto", handleajaxRAZhisto);
  server.on("/SetGPIO", handleSetGpio);
  server.on("/Export", handleExport);
  server.on("/export_file", handleExport_file);
  server.on("/restart", handleRestart);
  server.on("/Wifi", handleWifi);
  server.on("/AP_ScanWifi", handleAP_ScanWifi);
  server.on("/AP_SetWifi", handleAP_SetWifi);
  server.on("/Heure", handleHeure);
  server.on("/HourUpdate", handleHourUpdate);
  server.on("/Couleurs", handleCouleurs);
  server.on("/CommunCouleurJS", handleCommunCouleurJS);
  server.on("/CouleursAjax", handleCouleursAjax);
  server.on("/CouleurUpdate", handleCouleurUpdate);
  server.on("/commun.css", handleCommunCSS);
  server.onNotFound(handleNotFound);

  //SERVER OTA
  server.on("/OTA", HTTP_GET, []() {
    lectureCookie(OtaHtml);
  });
  /*handling uploading firmware file */
  server.on(
    "/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {  //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {  //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });

  /*handling uploading file */
  server.on(
    "/import", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", "OK");
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (opened == false) {
        opened = true;

        Serial.println("Debut Upload");
        ConfImport = "";
      }
      if (upload.status == UPLOAD_FILE_WRITE) {
        for (int i = 0; i < upload.currentSize; i++) {

          ConfImport += String(char(upload.buf[i]));
        }
      } else if (upload.status == UPLOAD_FILE_END) {

        Serial.println("UPLOAD_FILE_END");
        Serial.println(ConfImport);
        ImportParametres(ConfImport);
        opened = false;
      }
    });

  //here the list of headers to be recorded
  const char* headerkeys[] = { "User-Agent", "Cookie" };
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);


  server.begin();
}


void handleRoot() {  //Pages principales
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", MainHtml);
}
void handleWifi() {
  server.sendHeader("Connection", "close");
  lectureCookie(ConnectAP_Html);
}
void handleMainJS() {  //Code Javascript
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", MainJS);  // Javascript code
}

void handleBrute() {  //Page données brutes
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", PageBrute);
}
void handleBruteJS() {  //Code Javascript
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", PageBruteJS);  // Javascript code
}
void handleBruteJS2() {  //Code Javascript
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", PageBruteJS2);  // Javascript code
}

void handleAjaxRMS() {  // Envoi des dernières données  brutes reçues du RMS
  String S = "";
  String RMSExtDataB = "";
  int LastIdx = server.arg(0).toInt();
  if (Source == "Ext") {
    // Use WiFiClient class to create TCP connections
    WiFiClient clientESP_RMS;
    String host = IP2String(RMSextIP);
    if (!clientESP_RMS.connect(host.c_str(), 80)) {
      StockMessage("connection to ESP_RMS external failed (call from  handleAjaxRMS)");
      return;
    }
    String url = "/ajax_dataRMS?idx=" + String(LastIdx);
    clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (clientESP_RMS.available() == 0) {
      if (millis() - timeout > 5000) {
        StockMessage(">>> clientESP_RMS Timeout !");
        clientESP_RMS.stop();
        return;
      }
    }
    // Lecture des données brutes distantes
    while (clientESP_RMS.available()) {
      RMSExtDataB += clientESP_RMS.readStringUntil('\r');
    }
    S = RMSExtDataB.substring(RMSExtDataB.indexOf("\n\n") + 2);
  } else {
    S = DATE + RS + Source_data;
    if (Source_data == "UxI") {
      S += RS + String(Tension_M) + RS + String(Intensite_M) + RS + String(PowerFactor_M) + GS;
      int i0 = 0;
      int i1 = 0;
      for (int i = 0; i < 100; i++) {
        i1 = (i + 1) % 100;
        if (voltM[i] <= 0 && voltM[i1] > 0) {
          i0 = i1;  //Point de départ tableau . Phase positive
          i = 100;
        }
      }
      for (int i = 0; i < 100; i++) {
        i1 = (i + i0) % 100;
        S += String(int(10 * voltM[i1])) + RS;  //Voltages*10. Increase dynamic
      }
      S += "0" + GS;
      for (int i = 0; i < 100; i++) {
        i1 = (i + i0) % 100;
        S += String(int(10 * ampM[i1])) + RS;  //Currents*10
      }
      S += "0";
    }
    if (Source_data == "UxIx2") {

      S += GS + String(Tension_M) + RS + String(Intensite_M) + RS + String(PuissanceS_M - PuissanceI_M) + RS + String(PowerFactor_M) + RS + String(Energie_M_Soutiree) + RS + String(Energie_M_Injectee);
      S += RS + String(Tension_T) + RS + String(Intensite_T) + RS + String(PuissanceS_T - PuissanceI_T) + RS + String(PowerFactor_T) + RS + String(Energie_T_Soutiree) + RS + String(Energie_T_Injectee);
      S += RS + String(Frequence);
    }
    if (Source_data == "Linky") {
      S += GS;
      while (LastIdx != IdxDataRawLinky) {
        S += String(DataRawLinky[LastIdx]);
        LastIdx = (1 + LastIdx) % 10000;
      }
      S += GS + String(IdxDataRawLinky);
    }
    if (Source_data == "Enphase") {
      S += GS + String(Tension_M) + RS + String(Intensite_M) + RS + String(PuissanceS_M - PuissanceI_M) + RS + String(PowerFactor_M) + RS + String(Energie_M_Soutiree) + RS + String(Energie_M_Injectee);
      S += RS + String(PactProd) + RS + String(PactConso_M);
      String SessionId = "Not Received from Enphase";
      if (Session_id != "") {
        SessionId = "Ok Received from Enphase";
      }
      String Token_Enphase = "Not Received from Enphase";
      if (TokenEnphase.length() > 50) {
        Token_Enphase = "Ok Received from Enphase";
      }
      if (EnphaseUser == "") {
        SessionId = "Not Requested";
        Token_Enphase = "Not Requested";
      }
      S += RS + SessionId;

      S += RS + Token_Enphase;
    }
    if (Source_data == "SmartG") {
      S += GS + SG_dataBrute;
    }
    if (Source_data == "ShellyEm" || Source_data == "ShellyPro") {
      S += GS + ShEm_dataBrute;
    }
    if (Source_data == "UxIx3") {
      S += GS + MK333_dataBrute;
    }
    if (Source_data == "Pmqtt") {
      S += GS + P_MQTT_Brute;
    }
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleAjaxHisto48h() {  // Envoi Historique de 50h (600points) toutes les 5mn
  String S = "";
  String T = "";
  String U = "";
  String Ouverture = "";
  int iS = IdxStockPW;
  for (int i = 0; i < 600; i++) {
    S += String(tabPw_Maison_5mn[iS]) + ",";
    T += String(tabPw_Triac_5mn[iS]) + ",";
    iS = (1 + iS) % 600;
  }

  for (int canal = 0; canal < 4; canal++) {
    iS = IdxStockPW;
    for (int i = 0; i < 600; i++) {
      U += String(float(tabTemperature_5mn[canal][iS]) * 0.1) + ",";
      iS = (1 + iS) % 600;
    }
    U += String(temperature[canal]) + "|";
  }
  for (int i = 0; i < NbActions; i++) {
    if ((LesActions[i].Actif > 0) && (ITmode > 0 || i > 0)) {
      iS = IdxStockPW;
      if (LesActions[i].Actif > 0) {
        Ouverture += GS;
        for (int j = 0; j < 600; j++) {
          Ouverture += String(tab_histo_ouverture[i][iS]) + RS;
          iS = (1 + iS) % 600;
        }
        Ouverture += LesActions[i].Titre;
      }
    }
  }

  server.sendHeader("Connection", "close");
  server.send(200, "text/html", Source_data + GS + S + GS + T + GS + U + Ouverture);
}
void handleAjaxESP32() {  // Envoi des dernières infos sur l'ESP32
  IT10ms = 0;
  IT10ms_in = 0;
  String S = "";
  float H = float(T_On_seconde) / 3600.0;
  String coeur0 = String(int(previousTimeRMSMin)) + ", " + String(int(previousTimeRMSMoy)) + ", " + String(int(previousTimeRMSMax));
  String coeur1 = String(int(previousLoopMin)) + ", " + String(int(previousLoopMoy)) + ", " + String(int(previousLoopMax));
  S += String(H) + RS + WiFi.RSSI() + RS + WiFi.BSSIDstr() + RS + WiFi.macAddress() + RS + ssid + RS + WiFi.localIP().toString() + RS + WiFi.gatewayIP().toString() + RS + WiFi.subnetMask().toString();
  S += RS + coeur0 + RS + coeur1 + RS + String(P_cent_EEPROM) + RS;
  S += String(esp_get_free_internal_heap_size()) + RS + String(esp_get_minimum_free_heap_size()) + RS;
  delay(15);  //Comptage interruptions
  if (IT10ms_in > 0) {
    S += String(IT10ms_in) + "/" + String(IT10ms);
  } else {
    S += "Pas de Triac";
  }
  if (ITmode > 0) {
    S += RS + "Secteur";
  } else {
    S += RS + "Horloge ESP";
  }
  S += RS + String(Nbr_DS18B20);
  S += RS + AllTemp;
  int j = idxMessage;
  for (int i = 0; i < 10; i++) {
    S += RS + MessageH[j];
    j = (j + 1) % 10;
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleAjaxHisto1an() {  // Envoi Historique Energie quotiiienne sur 1 an 370 points
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", HistoriqueEnergie1An());
}
void handleAjaxData() {  //Données page d'accueil
  String DateLast = "Attente d'une mise à l'heure par internet";
  if (Horloge==1) DateLast = "Attente d'une mise à l'heure par le Linky";
  if (ModeWifi == 0 && WiFi.getMode() != WIFI_STA) DateLast = "Sélectionnez un réseau <a href='/Wifi'>Wifi</a>";
  if (Horloge>1) DateLast = "Attente d'une mise à l'heure  <a href='/Heure' >manuellement</a> ";
  if (HeureValide) {
    DateLast = DATE;
  }
  String S = LesTemperatures();
  S = "Deb" + RS + DateLast + RS + Source_data + RS + LTARF + RS + STGE + RS + S + RS + String(Pva_valide);
  S += GS + String(PuissanceS_M) + RS + String(PuissanceI_M) + RS + String(PVAS_M) + RS + String(PVAI_M);
  S += RS + String(EnergieJour_M_Soutiree) + RS + String(EnergieJour_M_Injectee) + RS + String(Energie_M_Soutiree) + RS + String(Energie_M_Injectee);
  if (Source_data == "UxIx2" || ((Source_data == "ShellyEm" || Source_data == "ShellyPro") && EnphaseSerial.toInt() < 3)) {  //UxIx2 ou Shelly monophasé avec 2 sondes
    S += GS + String(PuissanceS_T) + RS + String(PuissanceI_T) + RS + String(PVAS_T) + RS + String(PVAI_T);
    S += RS + String(EnergieJour_T_Soutiree) + RS + String(EnergieJour_T_Injectee) + RS + String(Energie_T_Soutiree) + RS + String(Energie_T_Injectee);
  }
  S += GS + "Fin";
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleAjax_etatActions() {
  int Force = server.arg("Force").toInt();
  int NumAction = server.arg("NumAction").toInt();
  if (Force != 0) {
    if (Force > 0) {
      if (LesActions[NumAction].tOnOff < 0) {
        LesActions[NumAction].tOnOff = 0;
      } else {
        LesActions[NumAction].tOnOff += 30;
      }
    } else {
      if (LesActions[NumAction].tOnOff > 0) {
        LesActions[NumAction].tOnOff = 0;
      } else {
        LesActions[NumAction].tOnOff -= 30;
      }
    }
    LesActions[NumAction].Prioritaire();
  }
  int NbActifs = 0;
  String S = "";
  String On_;
  for (int i = 0; i < NbActions; i++) {
    if ((LesActions[i].Actif > 0) && (ITmode > 0 || i > 0)) {  //Pas de Triac en synchro horloge interne
      S += String(i) + RS + LesActions[i].Titre + RS;
      if (LesActions[i].Actif == 1 && i > 0) {
        if (LesActions[i].On) {
          S += "On" + RS;
        } else {
          S += "Off" + RS;
        }
      } else {
        S += String(100 - Retard[i]) + RS;
      }
      S += String(LesActions[i].tOnOff) + RS;
      S += String(int(LesActions[i].H_Ouvre * 100.0)) + RS;
      S += GS;
      NbActifs++;
    }
  }
  String LesTemp = LesTemperatures();
  S = LesTemp + GS + String(Source_data) + GS + String(RMSextIP) + GS + NbActifs + GS + S;
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleAjax_etatActionX() {
  int NumAction = server.arg("NumAction").toInt();
  byte Actif = 0;
  int Ouvre = 0;
  int Hequiv = 0;
  if (NumAction < NbActions) {
    Actif = LesActions[NumAction].Actif;
    Ouvre = 100 - Retard[NumAction];
    Hequiv = int(100 * LesActions[NumAction].H_Ouvre);
  }
  String S = String(Actif) + GS + String(Ouvre) + GS + String(Hequiv);
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleAjaxTemperature() {
  String LesTemp = LesTemperatures();
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", GS + LesTemp + RS);
}
void handleRestart() {  // Eventuellement Reseter l'ESP32 à distance
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "OK Reset. Attendez.");
  delay(1000);
  ESP.restart();
}
void handleAjaxData10mn() {  // Envoi Historique de 10mn (300points)Energie Active Soutiré - Injecté
  String S = "";
  String T = "";
  int iS = IdxStock2s;
  for (int i = 0; i < 300; i++) {
    S += String(tabPw_Maison_2s[iS]) + ",";
    S += String(tabPva_Maison_2s[iS]) + ",";
    T += String(tabPw_Triac_2s[iS]) + ",";
    T += String(tabPva_Triac_2s[iS]) + ",";
    iS = (1 + iS) % 300;
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", Source_data + GS + S + GS + T);
}
void handleAjaxNoms() {
  Liste_Noms(0);  // Les noms de ce routeur
  String S = GS + RMS_Nom[0];
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}

void handleActions() {
  server.sendHeader("Connection", "close");
  lectureCookie(ActionsHtml);
}
void handleActionsJS() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", ActionsJS);
}
void handleActionsJS2() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", ActionsJS2);
}
void handleActionsJS3() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", ActionsJS3);
}
void handleActionsUpdate() {
  int adresse_max = 0;
  String s = server.arg("actions");
  String ligne = "";
  InitGpioActions();  //RAZ anciennes actions
  NbActions = 0;
  while (s.indexOf(GS) > 3 && NbActions < LesActionsLength) {
    ligne = s.substring(0, s.indexOf(GS));
    s = s.substring(s.indexOf(GS) + 1);
    LesActions[NbActions].Definir(ligne);
    NbActions = NbActions + 1;
  }
  adresse_max = EcritureEnROM();
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "OK" + String(adresse_max));
  InitGpioActions();
  Liste_des_Noms();
}
void handleActionsAjax() {
  String S = LesTemperatures() + RS;
  for (int c = 0; c < 4; c++) {
    S += nomTemperature[c] + US;
  }
  S = S + RS + String(LTARFbin) + RS + String(pTriac) + GS;
  for (int i = 0; i < NbActions; i++) {
    S += LesActions[i].Lire();
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}

void handlePara() {
  server.sendHeader("Connection", "close");
  lectureCookie(ParaHtml);
  previousTempMillis = millis() - 120000;
}
void handleParaUpdate() {
  lectureCookie("");
  CleAccesRef = CleAcces;  //Nouvelle cle d'accès ou mise à jour
  String Vp[64];
  String lesparas = server.arg("lesparas") + RS;
  int idx = 0;
  Serial.println(lesparas);
  while (lesparas.length() > 0) {
    Vp[idx] = lesparas.substring(0, lesparas.indexOf(RS));
    lesparas = lesparas.substring(lesparas.indexOf(RS) + 1);
    idx++;
    Vp[idx].trim();
  }
  dhcpOn = byte(Vp[0].toInt());
  RMS_IP[0] = strtoul(Vp[1].c_str(), NULL, 10);
  Gateway = strtoul(Vp[2].c_str(), NULL, 10);
  masque = strtoul(Vp[3].c_str(), NULL, 10);
  dns = strtoul(Vp[4].c_str(), NULL, 10);
  ModePara = byte(Vp[5].toInt());
  ModeWifi = byte(Vp[6].toInt());
  Horloge = byte(Vp[7].toInt());
  Source = Vp[8];
  RMSextIP = strtoul(Vp[9].c_str(), NULL, 10);
  EnphaseUser = Vp[10];
  EnphasePwd = Vp[11];
  EnphaseSerial = Vp[12];
  TopicP = Vp[13];
  MQTTRepet = Vp[14].toInt();
  MQTTIP = strtoul(Vp[15].c_str(), NULL, 10);
  MQTTPort = Vp[16].toInt();  //2 bytes
  MQTTUser = Vp[17];
  MQTTPwd = Vp[18];
  MQTTPrefix = Vp[19];
  MQTTPrefixEtat = Vp[20];
  MQTTdeviceName = Vp[21];
  subMQTT = byte(Vp[22].toInt());
  nomRouteur = Vp[23];
  nomSondeFixe = Vp[24];
  nomSondeMobile = Vp[25];
  CalibU = Vp[26].toInt();  //2 bytes
  CalibI = Vp[27].toInt();  //2 bytes
  TempoRTEon = byte(Vp[28].toInt());
  WifiSleep = byte(Vp[29].toInt());
  pSerial = byte(Vp[30].toInt());
  pTriac = byte(Vp[31].toInt());
  int canal = 0;
  for (int c = 0; c < 4; c++) {
    nomTemperature[c] = Vp[32 + 6 * c];
    Source_Temp[c] = Vp[33 + 6 * c];
    TopicT[c] = Vp[34 + 6 * c];
    refTempIP[c] = byte(Vp[35 + 6 * c].toInt());
    canalTempExterne[c] = byte(Vp[36 + 6 * c].toInt());
    offsetTemp[c] = Vp[37 + 6 * c].toInt();
  }
  int j = 1;
  for (int i = 1; i < LesRouteursMax; i++) {
    RMS_IP[i] = 0;
    unsigned long IP = strtoul(Vp[56 + i].c_str(), NULL, 10);
    if (IP > 0 && ModeWifi < 2) {
      RMS_IP[j] = IP;
      j++;
    }
  }

  previousTempMillis = millis() - 60000;
  int adresse_max = EcritureEnROM();
  if (Source != "Ext") {
    Source_data = Source;
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "OK" + String(adresse_max));
  LastHeureRTE = -1;
  //Recherche des Noms (routeurs, températures,actions) des RMS partenaires
  Liste_des_Noms();
}
void handleCleUpdate() {
  lectureCookie("");
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "OKcle");
}
void handleParaJS() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", ParaJS);
}
void handleParaRouteurJS() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", ParaRouteurJS);
}
void handleParaAjax() {
  String S = String(dhcpOn) + RS + String(RMS_IP[0]) + RS + String(Gateway) + RS + String(masque) + RS + String(dns) + RS;
  S += String(ModePara) + RS + String(ModeWifi) + RS + String(Horloge) + RS + Source + RS + String(RMSextIP) + RS;
  S += EnphaseUser + RS + EnphasePwd + RS + EnphaseSerial + RS + TopicP;
  S += RS + String(MQTTRepet) + RS + String(MQTTIP) + RS + String(MQTTPort) + RS + MQTTUser + RS + MQTTPwd;
  S += RS + MQTTPrefix + RS + MQTTPrefixEtat + RS + MQTTdeviceName + RS + String(subMQTT) + RS + nomRouteur + RS + nomSondeFixe + RS + nomSondeMobile;
  S += RS + String(CalibU) + RS + String(CalibI);
  S += RS + String(TempoRTEon) + RS + String(WifiSleep) + RS + String(pSerial) + RS + String(pTriac);
  for (int c = 0; c < 4; c++) {
    S += GS + nomTemperature[c] + RS + Source_Temp[c] + RS + TopicT[c] + RS + String(refTempIP[c]) + RS + String(canalTempExterne[c]) + RS + String(offsetTemp[c]);
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleajaxRAZhisto() {
  RAZ_Histo_Conso();
  for (int i = 0; i < 600; i++) {
    tabPw_Maison_5mn[i] = 0;  //Puissance Active:Soutiré-Injecté toutes les 5mn
    tabPw_Triac_5mn[i] = 0;
    for (int j = 0; j < 4; j++) {
      tabTemperature_5mn[j][i] = 0;
    }
    for (int j = 0; j < LesActionsLength; j++) {
      tab_histo_ouverture[j][i] = 0;
    }
  }
  for (int i = 0; i < 300; i++) {
    tabPw_Maison_2s[i] = 0;   //Puissance Active: toutes les 2s
    tabPw_Triac_2s[i] = 0;    //Puissance Triac: toutes les 2s
    tabPva_Maison_2s[i] = 0;  //Puissance Active: toutes les 2s
    tabPva_Triac_2s[i] = 0;
  }
  RAZ_JSY = true; 
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", "OK");
}
void handleParaRouteurAjax() {
  String S = Source + GS + Source_data + GS + WiFi.localIP().toString() + GS + nomRouteur + GS + Version + GS + nomSondeFixe + GS + nomSondeMobile + GS + String(RMSextIP) + GS + String(ModeWifi) + GS + String(ModePara) + GS+ String(Horloge) + GS;
  for (int i = 0; i < LesRouteursMax; i++) {  //index 0 pour ESP32 local
    if (RMS_IP[i] > 0) {
      S += RMS_IP[i] + US + RMS_Nom[i] + RS;
    }
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleSetGpio() {
  int gpio = server.arg("gpio").toInt();
  int out = server.arg("out").toInt();
  String S = "Refut : gpio =" + String(gpio) + " out =" + String(out);
  if (gpio >= 0 && gpio <= 33 && out >= 0 && out <= 1) {
    pinMode(gpio, OUTPUT);
    digitalWrite(gpio, out);
    S = "OK : gpio =" + String(gpio) + " out =" + String(out);
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
}
void handleExport() {
  server.sendHeader("Connection", "close");
  lectureCookie(ExportHtml);
}
void handleExport_file() {
  String S = Fichier_parametres(server.arg("ip"), server.arg("para"), server.arg("action"));
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", S);
}
void handleAP_ScanWifi() {
  server.send(200, "text/html", Liste_AP);
}
void Liste_WIFI() {  //Doit être fait avant toute connection WIFI depuis biblio ESP32 3.0.1
  WIFIbug = 0;

  int n = 0;
  WiFi.disconnect();
  delay(100);
  Serial.println("Scan start");
  // WiFi.scanNetworks will return the number of networks found.
  n = WiFi.scanNetworks();
  Serial.println("Scan done");
  Liste_AP = "";
  if (n == 0) {
    Serial.println("Pas de réseau Wifi trouvé");
  } else {
    Serial.print(n);
    Serial.println(" réseaux trouvés");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4d", WiFi.RSSI(i));
      Serial.println();
      Liste_AP += WiFi.SSID(i).c_str() + RS + WiFi.RSSI(i) + GS;
    }
  }
  WiFi.scanDelete();
}
void handleAP_SetWifi() {
  delay(1);
  Serial.println("Set Wifi");
  String NewSsid = server.arg("ssid");
  NewSsid.trim();
  String NewPassword = server.arg("passe");
  NewPassword.trim();
  Serial.println(NewSsid);
  Serial.println(NewPassword);
  ssid = NewSsid;
  password = NewPassword;
  ModeWifi = 0;
  StockMessage("Wifi Begin : " + ssid);
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long newstartMillis = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - newstartMillis < 20000)) {  // Attente connexion au Wifi
    Serial.write('!');
    Gestion_LEDs();
    Serial.print(WiFi.status());
    delay(300);
  }
  Serial.println();
  String S = "";
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    String IP = WiFi.localIP().toString();
    S = "Ok" + RS;
    S += "ESP 32 connecté avec succès au wifi : " + ssid + " avec l'adresse IP : " + IP;
    S += "<br><br> Connectez vous au wifi : " + ssid;
    S += "<br><br> Cliquez sur l'adresse : <a href='http://" + IP + "' >http://" + IP + "</a>";
    dhcpOn = 1;
    ModeWifi = 0;  //A priori
    EcritureEnROM();
  } else {
    S = "No" + RS + "ESP32 non connecté à :" + ssid + "<br>";
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", S);
  delay(1000);
  ESP.restart();
}

void handleHeure() {
  lectureCookie(HeureHtml);
}
void handleHourUpdate() {
  String New_H = server.arg("New_H");
  int p = New_H.indexOf(":");
  if (p > 0) {
    Int_Heure = (New_H.substring(0, p).toInt()) % 24;
    Int_Minute = (New_H.substring(p + 1).toInt()) % 60;
    HeureValide = true;
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "OKheure");
}

void handleCouleurs() {
  lectureCookie(CouleursHtml);
}
void handleCommunCouleurJS() {  //Code Javascript
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", CommunCouleurJS);  // Javascript code
}
void handleCouleursAjax() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/javascript", Couleurs);  // tableau des couleurs
}
void handleCouleurUpdate() {
  Couleurs = server.arg("couleurs");
  EcritureEnROM();
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", "OK couleurs");
}
void handleCommunCSS() {
  server.sendHeader("Connection", "close");
  String S="* {box-sizing: border-box;}\n";
  S+="body {font-size:150%;text-align:center;width:100%;max-width:1000px;margin:auto;padding:10px;background:linear-gradient(";
  if (Couleurs=="") {
    S +="#000033,#77b5fe,#000033";
  } else {
    S +="#"+Couleurs.substring(12, 18)+",#"+Couleurs.substring(6, 12)+",#"+Couleurs.substring(12, 18);
  } 
  S +=");background-attachment:fixed;color:";
  if (Couleurs=="") {
    S +="#ffffff";
  } else {
    S +="#"+Couleurs.substring(0,6);
  } 
  S+=";}\n";
  server.send(200, "text/css", S + CommunCSS);
}


void handleNotFound() {  //Page Web pas trouvé
  String message = "Fichier non trouvé\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.sendHeader("Connection", "close");
  server.send(404, "text/plain", message);
}
void lectureCookie(String S) {
  CleAcces = "";
  if (server.hasHeader("Cookie")) {
    String cookie = server.header("Cookie");
    cookie.trim();
    int p = cookie.indexOf("CleAcces=");
    if (p >= 0) {
      CleAcces = cookie.substring(p + 9);
      p = CleAcces.indexOf(";");
      if (p >= 0) CleAcces = CleAcces.substring(0, p - 1);
    }
    CleAcces.trim();
  }
  Serial.print("Clé accès reçue :" + CleAcces);
  Serial.println("  Attendue :" + CleAccesRef);
  if (S != "") {
    server.sendHeader("Connection", "close");
    if (CleAccesRef == CleAcces) {
      server.send(200, "text/html", S);
    } else {
      server.send(200, "text/html", ParaCleHtml);  //Demande clé d'acces / mot de passe
    }
  }
}
