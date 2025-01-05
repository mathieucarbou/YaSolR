// *********************************************************************************************
// * Client d'autres ESP32 en charge de mesurer les températures ou donner l'état des routages *
// *********************************************************************************************
void Liste_des_Noms() {  // Les noms des routeurs partenaires
  for (int i = 0; i < LesRouteursMax; i++) {
    if (RMS_IP[i] > 0) {
      Liste_Noms(i);
    }
  }
}

void Liste_Noms(int Idx_RMS) {
  if (Idx_RMS == 0) {  //Canal interne
    String S = nomRouteur + US;
    for (int canal = 0; canal < 4; canal++) {
      if (Source_Temp[canal] != "tempNo") {
        S += String(canal) + ES + nomTemperature[canal] + FS;
      }
    }
    S += US;
    for (int i = 0; i < NbActions; i++) {
      if (LesActions[i].Actif != 0) {
        S += String(i) + ES + LesActions[i].Titre + FS;
      }
    }
    RMS_Nom[0] = S;
    RMS_Actif[0] = true;
  } else {
    String RMSExtDataB = "";

    // Use WiFiClient class to create TCP connections
    WiFiClient clientESP_RMS;
    String host = IP2String(RMS_IP[Idx_RMS]);
    if (!clientESP_RMS.connect(host.c_str(), 80)) {

      StockMessage("connection to ESP_RMS (Noms) : " + host + " failed");
      RMS_Actif[Idx_RMS] = false;
      delay(2);
      return;
    }
    String url = "/ajax_Noms";
    clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (clientESP_RMS.available() == 0) {
      if (millis() - timeout > 5000) {

        StockMessage("client ESP_RMS (Noms) Timeout !" + host);
        clientESP_RMS.stop();
        RMS_Actif[Idx_RMS] = false;
        return;
      }
    }
    timeout = millis();
    // Lecture des données brutes distantes
    while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
      RMSExtDataB += clientESP_RMS.readStringUntil('\r');
    }
    if (RMSExtDataB.length() > 600) {
      RMSExtDataB = "";
    }
    int p = RMSExtDataB.indexOf(GS);
    RMSExtDataB = RMSExtDataB.substring(p + 1);
    if (RMSExtDataB.indexOf(US) > 0) {  //Trame semble OK
      RMS_Nom[Idx_RMS] = RMSExtDataB;
      RMS_Actif[Idx_RMS] = true;
    }
  }
}
void InfoActionExterne() { //Relevé périodique etat des actions internes et externes
  if (NbActions > 0) {
    RMS_Datas_idx = (RMS_Datas_idx + 1) % NbActions;
    byte SelectAction=LesActions[RMS_Datas_idx].SelActEnCours(HeureCouranteDeci);
    if (LesActions[RMS_Datas_idx].ExtSelAct !=SelectAction){ //Changement action surveillé
        LesActions[RMS_Datas_idx].ExtValide = 0;
        LesActions[RMS_Datas_idx].ExtSelAct =SelectAction;
    }
    if (SelectAction != 255) {                                                              //Action sous condition d'une autre
      if (SelectAction < 10) {                                                              //Action sur même routeur
        LesActions[RMS_Datas_idx].ExtValide = 3;                                            //Condition OK Action externe
        LesActions[RMS_Datas_idx].ExtHequiv = int(100 * LesActions[SelectAction].H_Ouvre);  //Duree heure decimale *100 action externe
        LesActions[RMS_Datas_idx].ExtOuvert = 100 - Retard[SelectAction];
      } else {  //Action d'un autre ESP32
        int Idx_RMS = int(SelectAction / 10);
        int NumAction = int(SelectAction) - int(10 * Idx_RMS);
        if (RMS_Actif[Idx_RMS]) {
          WiFiClient clientESP_RMS;
          String host = IP2String(RMS_IP[Idx_RMS]);
          if (!clientESP_RMS.connect(host.c_str(), 80)) {
            StockMessage("connection to ESP_RMS (Data) : " + host + " failed");
            if (LesActions[RMS_Datas_idx].ExtValide > 0) LesActions[RMS_Datas_idx].ExtValide--;
            delay(2);
            return;
          }
          String url = "/ajax_etatActionX?NumAction=" + String(NumAction);
          clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
          unsigned long timeout = millis();
          while (clientESP_RMS.available() == 0) {
            if (millis() - timeout > 5000) {
              StockMessage("client ESP_RMS (Noms) Timeout !" + host);
              clientESP_RMS.stop();
              if (LesActions[RMS_Datas_idx].ExtValide > 0) LesActions[RMS_Datas_idx].ExtValide--;
              return;
            }
          }
          timeout = millis();
          // Lecture des données brutes distantes
          String RMSExtDataB = "";
          while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
            RMSExtDataB += clientESP_RMS.readStringUntil('\r');
          }
          if (RMSExtDataB.length() > 200) {
            RMSExtDataB = "";
          }
          int p = RMSExtDataB.indexOf(GS);
          int Actif = RMSExtDataB.substring(p - 1, p).toInt();
          if (Actif > 0) {
            RMSExtDataB = RMSExtDataB.substring(p + 1);
            p = RMSExtDataB.indexOf(GS);
            LesActions[RMS_Datas_idx].ExtOuvert = RMSExtDataB.substring(0, p).toInt();
            RMSExtDataB = RMSExtDataB.substring(p + 1);
            LesActions[RMS_Datas_idx].ExtHequiv = RMSExtDataB.toInt();
            LesActions[RMS_Datas_idx].ExtValide = 3;  //Les données sont valides pour être utilisé dans les conditions
          } else {
            LesActions[RMS_Datas_idx].ExtValide = 0;
          }
        } else {
          if (LesActions[RMS_Datas_idx].ExtValide > 0) LesActions[RMS_Datas_idx].ExtValide--;
        }
      }
    }
  }
}
