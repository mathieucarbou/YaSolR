// ***************
// * Temperature *
// ***************

void InitTemperature() {
  Nbr_DS18B20 = 0;
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
    } else if (Source_Temp[canal] == "tempExt" && refTempIP[canal] > 0 && ModeReseau < 2) {
      if (TemperatureValide[canal] > 0)   TemperatureValide[canal] = TemperatureValide[canal] - 1;  // Watchdog pour verfier mesures arrivent 
      if (RMS_Note[refTempIP[canal]]>0) {
        String Etat,TempsR, TempR, Valeur;
        SplitS(RMS_NomEtat[refTempIP[canal]], Valeur, US, Etat);  //Nom
        SplitS(Etat, TempsR, US, Etat);                           //Les Températures        
        while (TempsR.length() > 1) {
          SplitS(TempsR, TempR, FS, TempsR);
          SplitS(TempR, Valeur, ES, TempR);  //Canal Externe
          if (Valeur.toInt() == canalTempExterne[canal]) {
            SplitS(TempR, Valeur, ES, TempR);  //Titre
            SplitS(TempR, Valeur, ES, TempR);  //Température
            temperature_brute = Valeur.toFloat();
            TemperatureValide[canal] = 5;
          }
        }
        
      }
      temperature[canal] = temperature_brute;
    } else if (Source_Temp[canal] == "tempMqtt" && ModeReseau < 2) {
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
