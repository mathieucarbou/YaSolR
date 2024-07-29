// ***************************************************************
// * Client d'un autre ESP32 en charge de mesurer les puissances *
// ***************************************************************
void CallESP32_Externe() {
  String S = "";
  String RMSExtDataB = "";
  String Gr[4];
  String data_[22];
 

  // Use WiFiClient class to create TCP connections
  WiFiClient clientESP_RMS;
  byte arr[4];
  arr[0] = RMSextIP & 0xFF;          // 0x78
  arr[1] = (RMSextIP >> 8) & 0xFF;   // 0x56
  arr[2] = (RMSextIP >> 16) & 0xFF;  // 0x34
  arr[3] = (RMSextIP >> 24) & 0xFF;  // 0x12

  String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
  if (!clientESP_RMS.connect(host.c_str(), 80)) {
    
    StockMessage("connection to ESP_RMS : " + host +" failed");
    delay(200);
    return;
  }
  String url = "/ajax_data";
  clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (clientESP_RMS.available() == 0) {
    if (millis() - timeout > 5000) {
      
      StockMessage("client ESP_RMS Timeout !" + host);
      clientESP_RMS.stop();
      return;
    }
  }
  timeout = millis();
  // Lecture des données brutes distantes
  while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
    RMSExtDataB += clientESP_RMS.readStringUntil('\r');
  }
  if (RMSExtDataB.length() > 400) {
    RMSExtDataB = "";
  }

  if (RMSExtDataB.indexOf("Deb") >= 0 && RMSExtDataB.indexOf("Fin") > 0) {  //Trame complète reçue
    RMSExtDataB = RMSExtDataB.substring(RMSExtDataB.indexOf("Deb") + 4);
    RMSExtDataB = RMSExtDataB.substring(0, RMSExtDataB.indexOf("Fin") + 3);
    String Sval = "";
    int idx = 0;
    while (RMSExtDataB.indexOf(GS) > 0) {
      Sval = RMSExtDataB.substring(0, RMSExtDataB.indexOf(GS));
      RMSExtDataB = RMSExtDataB.substring(RMSExtDataB.indexOf(GS) + 1);
      Gr[idx] = Sval;
      idx++;
    }
    Gr[idx] = RMSExtDataB;
    idx = 0;
    for (int i = 0; i < 3; i++) {
      while (Gr[i].indexOf(RS) >= 0) {
        Sval = Gr[i].substring(0, Gr[i].indexOf(RS));
        Gr[i] = Gr[i].substring(Gr[i].indexOf(RS) + 1);
        data_[idx] = Sval;
        idx++;
      }
      data_[idx] = Gr[i];
      idx++;
    }
    for (int i = 0; i <= idx; i++) {
      switch (i) {

        case 1:
          Source_data = data_[i];
          break;
        case 2:
          if (TempoEDFon == 0) LTARF = data_[i];
          break;
        case 3:
          if (TempoEDFon == 0) STGE = data_[i];
          break;
        case 4:
          //Temperature non utilisé
          break;
        case 5:
          Pva_valide=data_[i].toInt();
          break;
        case 6:
          PuissanceS_M = PintMax(data_[i].toInt());
          break;
        case 7:
          PuissanceI_M =  PintMax(data_[i].toInt());
          break;
        case 8:
          PVAS_M =  PintMax(data_[i].toInt());
          break;
        case 9:
          PVAI_M =  PintMax(data_[i].toInt());
          break;
        case 10:
          EnergieJour_M_Soutiree = data_[i].toInt();
          break;
        case 11:
          EnergieJour_M_Injectee = data_[i].toInt();
          break;
        case 12:
          Energie_M_Soutiree = data_[i].toInt();
          break;
        case 13:
          Energie_M_Injectee = data_[i].toInt();
          PuissanceRecue=true; //Reset du Watchdog à chaque trame du RMS reçue
          cptLEDyellow = 4;
          EnergieActiveValide=true;
          break;
        case 14:  //CAS UxIx2 avec une deuxieme sonde
          PuissanceS_T = data_[i].toInt();
          break;
        case 15:
          PuissanceI_T = data_[i].toInt();
          break;
        case 16:
          PVAS_T = data_[i].toInt();
          break;
        case 17:
          PVAI_T = data_[i].toInt();
          break;
        case 18:
          EnergieJour_T_Soutiree = data_[i].toInt();
          break;
        case 19:
          EnergieJour_T_Injectee = data_[i].toInt();
          break;
        case 20:
          Energie_T_Soutiree = data_[i].toInt();
          break;
        case 21:
          Energie_T_Injectee = data_[i].toInt();
          break;
      }
      
    }
    RMSExtDataB = "";
  }
}
