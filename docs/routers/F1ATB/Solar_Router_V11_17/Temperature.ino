// ***************
// * Temperature *
// ***************
void LectureTemperature() {
  float temperature_brute = -127;
  if (Source_Temp == "tempNo") {
    temperature = temperature_brute;
  }
  if (Source_Temp == "tempInt") {
    if (!ds18b20_Init) {
      ds18b20_Init = true;
      ds18b20.begin();
    }
    ds18b20.requestTemperatures();
    temperature_brute = ds18b20.getTempCByIndex(0);
    if (temperature_brute < -20 || temperature_brute > 130) {  //Invalide. Pas de capteur ou parfois mauvaise réponse
      if (TemperatureValide > 0) {
        TemperatureValide = TemperatureValide - 1;  // Perte éventuels de quelques mesures
      } else {
        StockMessage("Mesure Température invalide ou pas de capteur DS18B20");  //Trop de pertes
        temperature = temperature_brute;
      }
    } else {
      TemperatureValide = 5;
      temperature = temperature_brute;
    }
  }
  if (Source_Temp == "tempExt") {
    String RMSExtTemp = "";

    // Use WiFiClient class to create TCP connections
    WiFiClient clientESP_RMS;
    byte arr[4];
    arr[0] = IPtemp & 0xFF;
    arr[1] = (IPtemp >> 8) & 0xFF;
    arr[2] = (IPtemp >> 16) & 0xFF;
    arr[3] = (IPtemp >> 24) & 0xFF;

    String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
    if (!clientESP_RMS.connect(host.c_str(), 80)) {
      StockMessage("connection to ESP_RMS Temperature failed : " + host);
      delay(200);
      if (TemperatureValide > 0) {
        TemperatureValide = TemperatureValide - 1;  // Perte éventuels de quelques mesures
      } else {                                      //Trop de pertes
        temperature = temperature_brute;
      }
      return;
    }
    String url = "/ajax_Temperature";
    clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (clientESP_RMS.available() == 0) {
      if (millis() - timeout > 5000) {
        StockMessage("client ESP_RMS Temperature Timeout !" + host);
        clientESP_RMS.stop();
        if (TemperatureValide > 0) {
          TemperatureValide = TemperatureValide - 1;  // Perte éventuels de quelques mesures
        } else {                                      //Trop de pertes
          temperature = temperature_brute;
        }
        return;
      }
    }
    timeout = millis();
    // Lecture des données brutes distantes
    while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
      RMSExtTemp += clientESP_RMS.readStringUntil('\r');
    }

    if (RMSExtTemp.length() > 150) {
      RMSExtTemp = "";
    }
    if (RMSExtTemp.indexOf(GS) >= 0 && RMSExtTemp.indexOf(RS) > 0) {  //Trame complète reçue
      RMSExtTemp = RMSExtTemp.substring(RMSExtTemp.indexOf(GS) + 1);
      RMSExtTemp = RMSExtTemp.substring(0, RMSExtTemp.indexOf(RS));
      temperature_brute = RMSExtTemp.toFloat();
      RMSExtTemp = "";
      TemperatureValide =5;
      temperature = temperature_brute;
    }
    
  }
  if (Source_Temp == "tempMqtt") {
     if (TemperatureValide > 0) {
          TemperatureValide = TemperatureValide - 1;  // Watchdog pour verfier mesures arrivent voir MQTT.ino
        } else {
          temperature = temperature_brute;
        }
  }
}
