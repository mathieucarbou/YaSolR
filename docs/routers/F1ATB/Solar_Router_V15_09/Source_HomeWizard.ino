// ******************************
// * Client d'un HomeWizard     *
// * ESSAI A VALIDER            *
// ******************************
void LectureHomeW() {
  String S = "";
  String HomeW_Data = "";
  String Gr[4];
  String data_[20];

  Pva_valide = false;
  // Use WiFiClient class to create TCP connections
  WiFiClient clientESP_RMS;
  String host = IP2String(RMSextIP);
  if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {  // PORT 80 pour Home Wizard
    delay(500);
    if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {      
      clientESP_RMS.stop();
      delay(100);
      StockMessage("connection to HomeWizard failed : " + host);
      return;
    }
  }
  String url = "/api/v1/data";
  clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (clientESP_RMS.available() == 0) {
    if (millis() - timeout > 5000) {
      StockMessage(">>> client HomeWizard Timeout ! : " + host);
      clientESP_RMS.stop();
      delay(100);
      return;
    }
  }
  timeout = millis();
  // Lecture des données brutes distantes
  while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
    HomeW_Data += clientESP_RMS.readStringUntil('\r');
  }
  clientESP_RMS.stop();
  int p = HomeW_Data.indexOf("{");
  HomeW_Data = HomeW_Data.substring(p + 1);
  p = HomeW_Data.indexOf("}");
  HomeW_Data = HomeW_Data.substring(0, p);
  if (PfloatMax(ValJsonSG("active_power_w", HomeW_Data)) > 0) {
    PuissanceS_M_inst = PfloatMax(ValJsonSG("active_power_w", HomeW_Data));
    PuissanceI_M_inst = 0;
  } else {
    PuissanceS_M_inst = 0;
    PuissanceI_M_inst = -PfloatMax(ValJsonSG("active_power_w", HomeW_Data));
  }
  long EnergyDeliveredTariff1 = int(1000 * ValJsonSG("total_power_import_t1_kwh", HomeW_Data));
  long EnergyDeliveredTariff2 = int(1000 * ValJsonSG("total_power_import_t2_kwh", HomeW_Data));
  Energie_M_Soutiree = EnergyDeliveredTariff1 + EnergyDeliveredTariff2;
  long EnergyReturnedTariff1 = int(1000 * ValJsonSG("total_power_export_t1_kwh", HomeW_Data));
  long EnergyReturnedTariff2 = int(1000 * ValJsonSG("total_power_export_t2_kwh", HomeW_Data));
  Energie_M_Injectee = EnergyReturnedTariff1 + EnergyReturnedTariff2;
  HW_dataBrute = HomeW_Data;
  filtre_puissance();
  PuissanceRecue = true;  //Reset du Watchdog à chaque trame du HomeWizard reçue
  EnergieActiveValide = true;

  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}
