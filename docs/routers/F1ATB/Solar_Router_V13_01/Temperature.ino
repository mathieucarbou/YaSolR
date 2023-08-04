// ***************
// * Temperature *
// ***************

void InitTemperature() {
  ds18b20.begin();
  Nbr_DS18B20 = ds18b20.getDeviceCount();
  for (int i = 0; i < 4; i++) {  // 4 canaux max de température
    Source_Temp[i] = "tempNo";
    temperature[i] = -127;
    offsetTemp[i] = 0;
    TemperatureValide[i] = 0;
    canalTempExterne[i] = 0;
    nomTemperature[i] = "Température canal " + String(i);
    TopicT[i] = "TemperatureMQTT_" + String(i);
    refTempIP[i] = 1;
  }
}

void LectureTemperature() {
  float temperature_brute = -127;
  float temperDs18B20[4];
  bool tempInterneOK = true;
  AllTemp = "|";
  if (Nbr_DS18B20 > 0) {
    ds18b20.requestTemperatures();
    int i = 0;
    for (int canal = 0; canal < 4; canal++) {
      temperDs18B20[canal] = -127;
      if (Source_Temp[canal] == "tempInt") {
        temperDs18B20[canal] = ds18b20.getTempCByIndex(i) + float(offsetTemp[canal]) / 100.0;
        AllTemp += String(temperDs18B20[canal]) + "°C|";
        i++;
      }
    }
    while (i < Nbr_DS18B20) {
      AllTemp += String(ds18b20.getTempCByIndex(i)) + "°C|";  //On rajoute pour l'affichage brute les ds18b20 non utilisés
      i++;
    }
  }

  for (int canal = 0; canal < 4; canal++) {
    temperature_brute = -127;

    if (Source_Temp[canal] == "tempInt") {
      temperature_brute = temperDs18B20[canal];
      if (temperature_brute < -50 || temperature_brute > 150) {  //Invalide. Pas de capteur ou parfois mauvaise réponse
        if (TemperatureValide[canal] > 0) {
          TemperatureValide[canal] = TemperatureValide[canal] - 1;  // Perte éventuels de quelques mesures
        } else {
          StockMessage("Mesure Température invalide ou pas de capteur DS18B20, canal : " + String(canal));  //Trop de pertes
          temperature[canal] = temperature_brute;
          tempInterneOK = false;
        }
      } else {
        TemperatureValide[canal] = 5;
        temperature[canal] = temperature_brute;
      }
    } else if (Source_Temp[canal] == "tempExt" && refTempIP[canal] > 0 && ModeWifi<2) {
      String RMSExtTemp = "";

      // Use WiFiClient class to create TCP connections
      WiFiClient clientESP_RMS;
      String host = IP2String(RMS_IP[refTempIP[canal]]);
      if (!clientESP_RMS.connect(host.c_str(), 80)) {
        StockMessage("connection to ESP_RMS Temperature failed : " + host);
        delay(200);
        if (TemperatureValide[canal] > 0) {
          TemperatureValide[canal] = TemperatureValide[canal] - 1;  // Perte éventuels de quelques mesures
        } else {                                                    //Trop de pertes
          temperature[canal] = temperature_brute;
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
          if (TemperatureValide[canal] > 0) {
            TemperatureValide[canal] = TemperatureValide[canal] - 1;  // Perte éventuels de quelques mesures
          } else {                                                    //Trop de pertes
            temperature[canal] = temperature_brute;
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
        for (byte c = 0; c < 4; c++) {
          String Tempera = RMSExtTemp.substring(0, RMSExtTemp.indexOf("|"));
          if (canalTempExterne[canal] == c) {
            temperature_brute = RMSExtTemp.toFloat();
            TemperatureValide[canal] = 5;
            temperature[canal] = temperature_brute;
          }
          RMSExtTemp = RMSExtTemp.substring(RMSExtTemp.indexOf("|") + 1);
        }
        RMSExtTemp = "";
      }
    } else if (Source_Temp[canal] == "tempMqtt"  && ModeWifi<2) {
      if (TemperatureValide[canal] > 0) {
        TemperatureValide[canal] = TemperatureValide[canal] - 1;  // Watchdog pour verfier mesures arrivent voir MQTT.ino
      } else {
        temperature[canal] = temperature_brute;
      }
    } else {  // "tempNo"
      temperature[canal] = temperature_brute;
    }
  }
  for (int canal = 0; canal < 4; canal++) {
    if (Source_Temp[canal] == "tempInt" && !tempInterneOK) {
      TemperatureValide[canal] = 0;  //On arrete les mesures de tous les internes car l'ordre des DS18B20 peut être rompu
    }
  }
}
String LesTemperatures() {
  String LesTemp = "";
  for (int c = 0; c < 4; c++) {
    LesTemp += String(temperature[c]) + "|";
  }
  return LesTemp;
}
