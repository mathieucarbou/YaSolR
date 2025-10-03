// *********************************************************************************************
// * Client d'autres ESP32 en charge de mesurer les températures ou donner l'état des routages *
// *********************************************************************************************
void Liste_des_Noms() {  // Les noms des routeurs partenaires
  for (int i = 0; i < LesRouteursMax; i++) {
    if (RMS_IP[i] > 0) {
      Liste_NomsEtats(i);
    }
  }
}

void Liste_NomsEtats(int Idx_RMS) {
  if (RMS_NbCx[Idx_RMS] < 100) {
    RMS_NbCx[Idx_RMS]++;
  } else {
    RMS_NbCx[Idx_RMS] = 2;
    RMS_Note[Idx_RMS] = 1;
  }
  if (Idx_RMS == 0) {  //Canal interne
    String S = nomRouteur + US;
    for (int canal = 0; canal < 4; canal++) {
      if (Source_Temp[canal] != "tempNo") {
        String St = String(temperature[canal]);
        if (St.indexOf(".") > 0) St = St.substring(0, St.indexOf(".") + 2);
        S += String(canal) + ES + nomTemperature[canal] + ES + St + ES + Source_Temp[canal] + FS;
      }
    }
    S += US;
    for (int i = 0; i < NbActions; i++) {
      if (LesActions[i].Actif != 0) {
        int8_t Ouvre = 100 - Retard[i];
        int16_t Hequiv = int(100 * LesActions[i].H_Ouvre);
        S += String(i) + ES + LesActions[i].Titre + ES + String(Ouvre) + ES + String(Hequiv) + ES + LesActions[i].tOnOff + FS;
      }
    }
    RMS_NomEtat[0] = S;
    RMS_Note[0] = 100;
    RMS_NbCx[0] = 100;
  } else {
    String RMSExtDataB = "";
    // Use WiFiClient class to create TCP connections
    WiFiClient clientESP_RMS;
    String host = IP2String(RMS_IP[Idx_RMS]);
    if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {
      delay(500);
      if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {
        delay(100); //Necessaire
        clientESP_RMS.stop();       
        StockMessage("Connection (Name) Failed: " + host);
        if (RMS_Note[Idx_RMS] > 0) RMS_Note[Idx_RMS]--;
        return;
      }
    }
    String url = "/ajax_Noms";
    clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (clientESP_RMS.available() == 0) {
      if (millis() - timeout > 5000) {

        StockMessage("Timeout ESP_RMS (Etat)!" + host);
        clientESP_RMS.stop();
        if (RMS_Note[Idx_RMS] > 0) RMS_Note[Idx_RMS]--;
        return;
      }
    }
    timeout = millis();
    // Lecture des données brutes distantes
    while (clientESP_RMS.available() && (millis() - timeout < 3000)) {
      RMSExtDataB += clientESP_RMS.readStringUntil('\r');
    }
    if (RMSExtDataB.length() > 600) {
      RMSExtDataB = "";
    }
    clientESP_RMS.stop();
    int p = RMSExtDataB.indexOf(GS);
    RMSExtDataB = RMSExtDataB.substring(p + 1);
    if (RMSExtDataB.indexOf(US) > 0) {  //Trame semble OK
      if (RMS_Note[Idx_RMS] < 100) RMS_Note[Idx_RMS]++;
      RMS_NbCx[Idx_RMS] = max(RMS_NbCx[Idx_RMS], RMS_Note[Idx_RMS]);
      RMS_NomEtat[Idx_RMS] = RMSExtDataB;
    }
  }
}
void InfoActionExterne() {  //Relevé périodique etat des actions internes et externes
  if (NbActions > 0) {
    RMS_Datas_idx = (RMS_Datas_idx + 1) % NbActions;
    byte SelectAction = LesActions[RMS_Datas_idx].SelActEnCours(HeureCouranteDeci);
    if (LesActions[RMS_Datas_idx].ExtSelAct != SelectAction) {  //Changement action surveillé
      LesActions[RMS_Datas_idx].ExtValide = 0;
      LesActions[RMS_Datas_idx].ExtSelAct = SelectAction;
    }
    if (SelectAction != 255) {                                                              //Action sous condition d'une autre
      if (SelectAction < 10) {                                                              //Action sur même routeur
        LesActions[RMS_Datas_idx].ExtValide = 3;                                            //Condition OK Action externe
        LesActions[RMS_Datas_idx].ExtHequiv = int(100 * LesActions[SelectAction].H_Ouvre);  //Duree heure decimale *100 action externe
        LesActions[RMS_Datas_idx].ExtOuvert = 100 - Retard[SelectAction];
      } else {  //Action d'un autre ESP32
        int Idx_RMS = int(SelectAction / 10);
        int NumAction = int(SelectAction) - int(10 * Idx_RMS);
        if (RMS_Note[Idx_RMS] > 0) {
          String Etat, Valeur;
          SplitS(RMS_NomEtat[Idx_RMS], Valeur, US, Etat);  //Nom
          SplitS(Etat, Valeur, US, Etat);                  //Températures
          String ActionsR, ActionR;
          SplitS(Etat, ActionsR, US, Etat);  //Actions
          if (LesActions[RMS_Datas_idx].ExtValide > 0) LesActions[RMS_Datas_idx].ExtValide--;
          while (ActionsR.length() > 1) {
            SplitS(ActionsR, ActionR, FS, ActionsR);
            SplitS(ActionR, Valeur, ES, ActionR);  //Index
            if (Valeur.toInt() == NumAction) {
              SplitS(ActionR, Valeur, ES, ActionR);  //Titre
              SplitS(ActionR, Valeur, ES, ActionR);  //Ouverture
              LesActions[RMS_Datas_idx].ExtOuvert = Valeur.toInt();
              SplitS(ActionR, Valeur, ES, ActionR);  //Hequiv
              LesActions[RMS_Datas_idx].ExtHequiv = Valeur.toInt();
              LesActions[RMS_Datas_idx].ExtValide = 5;
            }
          }
        } else {
          if (LesActions[RMS_Datas_idx].ExtValide > 0) LesActions[RMS_Datas_idx].ExtValide--;
        }
      }
    }
  }
}
