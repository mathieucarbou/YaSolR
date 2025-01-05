// **********************************************************************************************
// *                        MQTT AUTO-DISCOVERY POUR HOME ASSISTANT ou DOMOTICZ                            *
// **********************************************************************************************
char DEVICE[300];
char ESP_ID[15];
char mdl[30];
char StateTopic[50];
char PrefixMQTT[25];
char PrefixMQTTEtat[25];
char AvailableTopic[60];


// Types de composants reconnus par HA et obligatoires pour l'Auto-Discovery.
const char *SSR = "sensor";
const char *SLCT = "select";
const char *NB = "number";
const char *BINS = "binary_sensor";
const char *SWTC = "switch";
const char *TXT = "text";
void GestionMQTT() {

  bool Temper = false;
  if (ModeWifi < 2) {
    for (int C = 0; C < 4; C++) {
      if (Source_Temp[C] == "tempMqtt") Temper = true;
    }
    if (MQTTRepet > 0 || Temper || Source == "Pmqtt" || subMQTT == 1) {
      if (testMQTTconnected()) {
        clientMQTT.loop();
        envoiVersMQTT();
      }
    }
  }
}

bool testMQTTconnected() {
  bool connecte = true;
  if (!clientMQTT.connected()) {  // si le mqtt n'est pas connecté (utile aussi lors de la 1ere connexion)
    Serial.println("Connection au serveur MQTT ...");
    String host = IP2String(MQTTIP);
    String S = "";
    if (MQTTPrefix != "") S = MQTTPrefix + "/";
    sprintf(PrefixMQTT, "%s", S.c_str());
    S = "";
    if (MQTTPrefixEtat != "") S = MQTTPrefixEtat + "/";
    sprintf(PrefixMQTTEtat, "%s", S.c_str());
    String TopicA = "Available";
    sprintf(AvailableTopic, "%s%s/%s", PrefixMQTTEtat, MQTTdeviceName.c_str(), TopicA.c_str());
    clientMQTT.setServer(host.c_str(), MQTTPort);
    clientMQTT.setCallback(callback);  //Déclaration de la fonction de souscription
    // if (clientMQTT.connect(MQTTdeviceName.c_str(), MQTTUser.c_str(), MQTTPwd.c_str())) {  // si l'utilisateur est connecté au mqtt
    if (clientMQTT.connect(MQTTdeviceName.c_str(), MQTTUser.c_str(), MQTTPwd.c_str(), AvailableTopic, 2, true, "offline")) {  // si l'utilisateur est connecté au mqtt
      StockMessage(MQTTdeviceName + " connecté au broker MQTT");
      clientMQTT.publish(AvailableTopic, "online", true);
      for (int C = 0; C < 4; C++) {
        if (Source_Temp[C] == "tempMqtt") {
          char TopicV[50];
          sprintf(TopicV, "%s", TopicT[C].c_str());
          clientMQTT.subscribe(TopicV);
        }
      }
      if (Source == "Pmqtt") {
        char Topicp[50];
        sprintf(Topicp, "%s", TopicP.c_str());
        clientMQTT.subscribe(Topicp);
      }
      if (subMQTT == 1) {
        for (int i = 0; i < NbActions; i++) {
          if (LesActions[i].Actif > 0) {
            char TopicAct[50];
            sprintf(TopicAct, "%s", LesActions[i].Titre.c_str());
            clientMQTT.subscribe(TopicAct);
          }
        }
      }
      sprintf(StateTopic, "%s%s_state", PrefixMQTTEtat, MQTTdeviceName.c_str());
      byte mac[6];  // the MAC address of your Wifi shield
      WiFi.macAddress(mac);
      sprintf(ESP_ID, "%02x%02x%02x", mac[2], mac[1], mac[0]);  // ID de l'entité pour HA
      sprintf(mdl, "%s%s", "ESP32 - ", ESP_ID);                 // ID de l'entité pour HA
      String mf = "F1ATB - https://f1atb.fr";
      String cu = "http://" + WiFi.localIP().toString();
      String hw = String(ESP.getChipModel()) + " rev." + String(ESP.getChipRevision());
      String sw = Version;
      sprintf(DEVICE, "{\"ids\":\"%s\",\"name\":\"%s\",\"mdl\":\"%s\",\"mf\":\"%s\",\"hw\":\"%s\",\"sw\":\"%s\",\"cu\":\"%s\"}", ESP_ID, nomRouteur.c_str(), mdl, mf.c_str(), hw.c_str(), sw.c_str(), cu.c_str());
      PeriodeMQTTMillis = 500;
    } else {  // si utilisateur pas connecté au mqtt
      StockMessage("Echec connexion MQTT : " + host);
      connecte = false;
      delay(1);
      PeriodeMQTTMillis = 30000;  //Penalisé 30s
      previousMQTTMillis = millis();
    }
  }
  return connecte;
}
void envoiVersMQTT() {
  unsigned long tps = millis();                                                     // utilisé pour l'envoie de l'état On/Off des actions.
  if (int((tps - previousMQTTenvoiMillis) / 1000) > MQTTRepet && MQTTRepet != 0) {  // Si Service MQTT activé avec période sup à 0
    previousMQTTenvoiMillis = tps;
    if (!Discovered) {  //(uniquement au démarrage discovery = 0 et toute les 5mn si HA redemarre)
      sendMQTTDiscoveryMsg_global();
    }
    SendDataToHomeAssistant();  // envoie du Payload au State topic
    clientMQTT.loop();
  }
}
//Callback  après souscription à un topic et  réaliser une action
void callback(char *topic, byte *payload, unsigned int length) {
  char Message[length + 1];
  for (int i = 0; i < length; i++) {
    Message[i] = payload[i];
  }
  Message[length] = '\0';
  String message = String(Message) + ",";
  for (int canal = 0; canal < 4; canal++) {
    if (String(topic) == TopicT[canal] && Source_Temp[canal] == "tempMqtt") {
      temperature[canal] = ValJson("temperature", message);
      TemperatureValide[canal] = 5;
    }
  }

  if (String(topic) == TopicP && Source == "Pmqtt") {  //Mesure de puissance
    PwMQTT = ValJson("Pw", message);
    PvaMQTT = ValJson("Pva", message);
    PfMQTT = ValJson("Pf", message);
    P_MQTT_Brute = String(Message);
    if (message.indexOf("Pw") > 0) LastPwMQTTMillis = millis();
  }
  if (subMQTT == 1) {
    for (int i = 0; i < NbActions; i++) {
      if (LesActions[i].Actif > 0 && LesActions[i].Titre == String(topic)) {
        LesActions[i].tOnOff = ValJson("tOnOff", message);
        LesActions[i].Prioritaire();
      }
    }
  }
}
//*************************************************************************
//*          CONFIG OF DISCOVERY MESSAGE FOR HOME ASSISTANT  / DOMOTICZ             *
//*************************************************************************


void sendMQTTDiscoveryMsg_global() {
  String ActType;
  String ActifType;
  String ActionDur;
  String ActionOnOff;
  // augmente la taille du buffer wifi Mqtt (voir PubSubClient.h)
  clientMQTT.setBufferSize(1700);  // voir -->#define MQTT_MAX_PACKET_SIZE 256 is the default value in PubSubClient.h
  if (Source == "UxIx2" || Source == "ShellyEm" || Source == "ShellyPro") {
    DeviceToDiscover("PuissanceS_T", "Puissance T Soutirée", "W", "power", "0");
    DeviceToDiscover("PuissanceI_T", "Puissance T Injectée", "W", "power", "0");
    DeviceToDiscover("Tension_T", "Tension T", "V", "voltage", "2");
    DeviceToDiscover("Intensite_T", "Intensité T", "A", "current", "2");
    DeviceToDiscover("PowerFactor_T", "Facteur de Puissance T", "", "power_factor", "2");
    DeviceToDiscover("Energie_T_Soutiree", "Energie Totale T Soutirée", "Wh", "energy", "0");
    DeviceToDiscover("Energie_T_Injectee", "Energie Totale T Injectée", "Wh", "energy", "0");
    DeviceToDiscover("EnergieJour_T_Soutiree", "Energie Jour T Soutirée", "Wh", "energy", "0");
    DeviceToDiscover("EnergieJour_T_Injectee", "Energie Jour T Injectée", "Wh", "energy", "0");
    DeviceToDiscover("Frequence", "Fréquence", "Hz", "frequency", "2");
  }
  for (int canal = 0; canal < 4; canal++) {
    if (Source_Temp[canal] != "tempNo") DeviceToDiscover("Temperature_" + String(canal), nomTemperature[canal], "°C", "temperature", "1");
  }


  if (Source == "Linky" || TempoRTEon == 1) {
    DeviceTextToDiscover("LTARF", "Option Tarifaire");
    DeviceToDiscoverWithoutUnit("Code_Tarifaire", "Code Tarifaire", "0");
  }
  if (Source == "Linky") {
    DeviceTextToDiscover("NGTF", "Calendrier Tarifaire");
    DeviceToDiscover("EASF01", "EASF01", "Wh", "energy", "0");
    DeviceToDiscover("EASF02", "EASF02", "Wh", "energy", "0");
    DeviceToDiscover("EASF03", "EASF03", "Wh", "energy", "0");
    DeviceToDiscover("EASF04", "EASF04", "Wh", "energy", "0");
    DeviceToDiscover("EASF05", "EASF05", "Wh", "energy", "0");
    DeviceToDiscover("EASF06", "EASF06", "Wh", "energy", "0");
    DeviceToDiscover("EASF07", "EASF07", "Wh", "energy", "0");
    DeviceToDiscover("EASF08", "EASF08", "Wh", "energy", "0");
    DeviceToDiscover("EASF09", "EASF09", "Wh", "energy", "0");
    DeviceToDiscover("EASF10", "EASF10", "Wh", "energy", "0");
  }
  if (Source == "Enphase") {
    DeviceToDiscover("PactProd", "Puissance produite", "W", "power", "0");
    DeviceToDiscover("PactConso_M", "Puissance conso.", "W", "power", "0");
  }

  DeviceToDiscover("PuissanceS_M", "Puissance M Soutirée", "W", "power", "0");
  DeviceToDiscover("PuissanceI_M", "Puissance M Injectée", "W", "power", "0");
  DeviceToDiscover("Tension_M", "Tension M", "V", "voltage", "2");
  DeviceToDiscover("Intensite_M", "Intensité M", "A", "current", "2");
  DeviceToDiscover("PowerFactor_M", "Facteur de Puissance M", "", "power_factor", "2");
  DeviceToDiscover("Energie_M_Soutiree", "Energie Totale M Soutirée", "Wh", "energy", "0");
  DeviceToDiscover("Energie_M_Injectee", "Energie Totale M Injectée", "Wh", "energy", "0");
  DeviceToDiscover("EnergieJour_M_Soutiree", "Energie Jour M Soutirée", "Wh", "energy", "0");
  DeviceToDiscover("EnergieJour_M_Injectee", "Energie Jour M Injectée", "Wh", "energy", "0");

  for (int i = 0; i < NbActions; i++) {
    ActType = "Ouverture_Relais_" + String(i);
    ActifType = "Actif_Relais_" + String(i);
    ActionDur = "Duree_Relais_" + String(i);
    ActionOnOff = "Force_OnOff_Relais_" + String(i);
    if (i == 0) {
      ActType = "Ouverture_Triac";
      ActifType = "Actif_Triac";
      ActionDur = "Duree_Triac";
      ActionOnOff = "Force_Triac_OnOff";
    }
    if (pTriac > 0 || i > 0) {
      DeviceToDiscoverWithoutClass(ActType, LesActions[i].Titre + " Ouverture", "%", "0");
      DeviceBin2Discover(ActifType, LesActions[i].Titre + " Actif");
      DeviceToDiscover(ActionDur, LesActions[i].Titre + " Durée Equiv.", "h", "duration", "2");
      DeviceToDiscoverWithoutClass(ActionOnOff, LesActions[i].Titre + " Force OnOff", "min", "0");
    }
  }
  Serial.println("Paramètres Auto-Discovery publiés !");
  Discovered = true;


}  // END OF sendMQTTDiscoveryMsg_global

void DeviceToDiscover(String VarName, String TitleName, String Unit, String Class, String Round) {
  char value[700];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[60];
  char state_class[60];
  sprintf(DiscoveryTopic, "%s%s/%s_%s/%s", PrefixMQTT, SSR, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s|default(0)|round(%s)}}", VarName.c_str(), Round.c_str());
  sprintf(state_class, "%s", "");
  if (Unit == "Wh" || Unit == "kWh") {
    sprintf(state_class, "\"state_class\":\"total_increasing\"%s,", state_class);
  }
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"device_class\": \"%s\",\"unit_of_meas\": \"%s\",%s\"val_tpl\": \"%s\",\"device\": %s, \"availability_topic\": \"%s\"}", TitleName.c_str(), UniqueID, StateTopic, Class.c_str(), Unit.c_str(), state_class, ValTpl, DEVICE, AvailableTopic);
  clientMQTT.publish(DiscoveryTopic, value);
}
void DeviceToDiscoverWithoutUnit(String VarName, String TitleName, String Round) {
  char value[700];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[60];
  sprintf(DiscoveryTopic, "%s%s/%s_%s/%s", PrefixMQTT, SSR, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s|default(0)|round(%s)}}", VarName.c_str(), Round.c_str());
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"val_tpl\": \"%s\",\"device\": %s, \"availability_topic\": \"%s\"}", TitleName.c_str(), UniqueID, StateTopic, ValTpl, DEVICE, AvailableTopic);
  clientMQTT.publish(DiscoveryTopic, value);
}
void DeviceToDiscoverWithoutClass(String VarName, String TitleName, String Unit, String Round) {
  char value[700];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[60];
  sprintf(DiscoveryTopic, "%s%s/%s_%s/%s", PrefixMQTT, SSR, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s|default(0)|round(%s)}}", VarName.c_str(), Round.c_str());
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"unit_of_meas\": \"%s\",\"val_tpl\": \"%s\",\"device\": %s, \"availability_topic\": \"%s\"}", TitleName.c_str(), UniqueID, StateTopic, Unit.c_str(), ValTpl, DEVICE, AvailableTopic);
  clientMQTT.publish(DiscoveryTopic, value);
}

void DeviceBin2Discover(String VarName, String TitleName) {
  char value[700];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[60];
  int init = 0;  // default value
  String ic = "mdi:electric-switch";
  sprintf(DiscoveryTopic, "%s%s/%s_%s/%s", PrefixMQTT, BINS, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s}}", VarName.c_str());
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"init\": %d,\"ic\": \"%s\",\"payload_off\":\"0\",\"payload_on\":\"1\",\"val_tpl\": \"%s\",\"device\": %s, \"availability_topic\": \"%s\"}", TitleName.c_str(), UniqueID, StateTopic, init, ic.c_str(), ValTpl, DEVICE, AvailableTopic);
  clientMQTT.publish(DiscoveryTopic, value);
}


void DeviceTextToDiscover(String VarName, String TitleName) {
  char value[600];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[50];
  sprintf(DiscoveryTopic, "%s%s/%s_%s/%s", PrefixMQTT, SSR, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s }}", VarName.c_str());
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"device_class\": \"%s\",\"val_tpl\": \"%s\",\"device\": %s, \"availability_topic\": \"%s\"}", TitleName.c_str(), UniqueID, StateTopic, "enum", ValTpl, DEVICE, AvailableTopic);
  clientMQTT.publish(DiscoveryTopic, value);
}
//****************************************
//* ENVOIE DES DATAS VERS HOME ASSISTANT *
//****************************************

void SendDataToHomeAssistant() {
  String ActType;
  String ActifType;
  String ActionDur;
  String ActionOnOff;
  char value[1200];
  sprintf(value, "{\"PuissanceS_M\": %d, \"PuissanceI_M\": %d, \"Tension_M\": %.1f, \"Intensite_M\": %.1f, \"PowerFactor_M\": %.2f, \"Energie_M_Soutiree\":%d,\"Energie_M_Injectee\":%d, \"EnergieJour_M_Soutiree\":%d, \"EnergieJour_M_Injectee\":%d", PuissanceS_M, PuissanceI_M, Tension_M, Intensite_M, PowerFactor_M, Energie_M_Soutiree, Energie_M_Injectee, EnergieJour_M_Soutiree, EnergieJour_M_Injectee);

  if (Source == "UxIx2" || Source == "ShellyEm" || Source == "ShellyPro") {
    sprintf(value, "%s,\"PuissanceS_T\": %d, \"PuissanceI_T\": %d, \"Tension_T\": %.1f, \"Intensite_T\": %.1f, \"PowerFactor_T\": %.2f, \"Energie_T_Soutiree\":%d,\"Energie_T_Injectee\":%d, \"EnergieJour_T_Soutiree\":%d, \"EnergieJour_T_Injectee\":%d, \"Frequence\":%.2f", value, PuissanceS_T, PuissanceI_T, Tension_T, Intensite_T, PowerFactor_T, Energie_T_Soutiree, Energie_T_Injectee, EnergieJour_T_Soutiree, EnergieJour_T_Injectee, Frequence);
  }
  for (int canal = 0; canal < 4; canal++) {
    if (temperature[canal] > -100 && Source_Temp[canal] != "tempNo") {
      sprintf(value, "%s,\"Temperature_%s\": %.1f", value, String(canal), temperature[canal]);
    }
  }

  if (Source == "Linky" || TempoRTEon == 1) {
    int code = 0;
    if (LTARF.indexOf("HEURE  CREUSE") >= 0) code = 1;  //Code Linky
    if (LTARF.indexOf("HEURE  PLEINE") >= 0) code = 2;
    if (LTARF.indexOf("HC BLEU") >= 0) code = 11;
    if (LTARF.indexOf("HP BLEU") >= 0) code = 12;
    if (LTARF.indexOf("HC BLANC") >= 0) code = 13;
    if (LTARF.indexOf("HP BLANC") >= 0) code = 14;
    if (LTARF.indexOf("HC ROUGE") >= 0) code = 15;
    if (LTARF.indexOf("HP ROUGE") >= 0) code = 16;
    if (LTARF.indexOf("TEMPO_BLEU") >= 0) code = 17;  // Code RTE
    if (LTARF.indexOf("TEMPO_BLANC") >= 0) code = 18;
    if (LTARF.indexOf("TEMPO_ROUGE") >= 0) code = 19;
    sprintf(value, "%s,\"LTARF\":\"%s\", \"Code_Tarifaire\":%d", value, LTARF.c_str(), code);
  }
  if (Source == "Linky") {
    sprintf(value, "%s,\"NGTF\":\"%s\"", value, NGTF.c_str());
    sprintf(value, "%s,\"EASF01\":%d, \"EASF02\":%d, \"EASF03\":%d, \"EASF04\":%d, \"EASF05\":%d, \"EASF06\":%d,\"EASF07\":%d, \"EASF08\":%d, \"EASF09\":%d, \"EASF10\":%d", value, EASF01, EASF02, EASF03, EASF04, EASF05, EASF06, EASF07, EASF08, EASF09, EASF10);
  }
  if (Source == "Enphase") {
    sprintf(value, "%s,\"PactProd\":%d, \"PactConso_M\":%d", value, PactProd, PactConso_M);
  }

  for (int i = 0; i < NbActions; i++) {
    if (pTriac > 0 || i > 0) {  //On envoi pas Triac si pas présent
      ActType = "Ouverture_Relais_" + String(i);
      ActifType = "Actif_Relais_" + String(i);
      ActionDur = "Duree_Relais_" + String(i);
      ActionOnOff = "Force_OnOff_Relais_" + String(i);
      if (i == 0) {
        ActType = "Ouverture_Triac";
        ActifType = "Actif_Triac";
        ActionDur = "Duree_Triac";
        ActionOnOff = "Force_Triac_OnOff";
      }
      int Ouv = 100 - Retard[i];
      sprintf(value, "%s,\"%s\":%d", value, ActType.c_str(), Ouv);
      if (Ouv != 0) {
        sprintf(value, "%s,\"%s\":%d", value, ActifType.c_str(), 1);
      } else {
        sprintf(value, "%s,\"%s\":%d", value, ActifType.c_str(), 0);
      }
      sprintf(value, "%s,\"%s\":%f", value, ActionDur.c_str(), LesActions[i].H_Ouvre);
      sprintf(value, "%s,\"%s\":%d", value, ActionOnOff.c_str(), LesActions[i].tOnOff);
    }
  }
  sprintf(value, "%s}", value);
  bool published = clientMQTT.publish(StateTopic, value);
}
