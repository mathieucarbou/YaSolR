// ******************************
// * Client d'un Smart Gateways *
// ******************************
void LectureSmartG() {
  String S = "";
  String SmartG_Data = "";
  String Gr[4];
  String data_[20];

  Pva_valide=false;
  // Use WiFiClient class to create TCP connections
  WiFiClient clientESP_RMS;
  byte arr[4];
  arr[0] = RMSextIP & 0xFF;          // 0x78
  arr[1] = (RMSextIP >> 8) & 0xFF;   // 0x56
  arr[2] = (RMSextIP >> 16) & 0xFF;  // 0x34
  arr[3] = (RMSextIP >> 24) & 0xFF;  // 0x12

  String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
  if (!clientESP_RMS.connect(host.c_str(), 82)) {  // PORT 82 pour Smlart Gateways
    StockMessage("connection to SmartGateways failed : " + host);
    delay(200);
    return;
  }
  String url = "/smartmeter/api/read";
  clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (clientESP_RMS.available() == 0) {
    if (millis() - timeout > 5000) {
      StockMessage(">>> client SmartGateways Timeout ! : " + host);
      clientESP_RMS.stop();
      return;
    }
  }
  timeout = millis();
  // Lecture des données brutes distantes
  while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
    SmartG_Data += clientESP_RMS.readStringUntil('\r');
  }
  int p = SmartG_Data.indexOf("{");
  SmartG_Data = SmartG_Data.substring(p+1);
  p = SmartG_Data.indexOf("}");
  SmartG_Data = SmartG_Data.substring(0,p);
  PuissanceS_M_inst=PfloatMax(ValJsonSG("PowerDelivered_total", SmartG_Data));
  PuissanceI_M_inst=PfloatMax(ValJsonSG("PowerReturned_total", SmartG_Data));
  long EnergyDeliveredTariff1=int(1000*ValJsonSG("EnergyDeliveredTariff1", SmartG_Data));
  long EnergyDeliveredTariff2=int(1000*ValJsonSG("EnergyDeliveredTariff2", SmartG_Data));
  Energie_M_Soutiree=EnergyDeliveredTariff1+EnergyDeliveredTariff2;
  long EnergyReturnedTariff1=int(1000*ValJsonSG("EnergyReturnedTariff1", SmartG_Data));
  long EnergyReturnedTariff2=int(1000*ValJsonSG("EnergyReturnedTariff2", SmartG_Data));
  Energie_M_Injectee=EnergyReturnedTariff1+EnergyReturnedTariff2;
  SG_dataBrute=SmartG_Data;
  filtre_puissance();
  PuissanceRecue=true;  //Reset du Watchdog à chaque trame du SmartGateways reçue
  EnergieActiveValide = true;
  
  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}

float ValJsonSG(String nom, String Json) {
  int p = Json.indexOf(nom);
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 2);
  p = Json.indexOf(",");
  float val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toFloat();
  }
  return val;
}
