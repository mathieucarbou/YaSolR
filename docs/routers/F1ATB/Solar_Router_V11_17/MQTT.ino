// **********************************************************************************************
// *                        MQTT AUTO-DISCOVERY POUR HOME ASSISTANT ou DOMOTICZ                            *
// **********************************************************************************************
char DEVICE[300];
char ESP_ID[15];
char mdl[30];
char StateTopic[50];


// Types de composants reconnus par HA et obligatoires pour l'Auto-Discovery.
const char *SSR = "sensor";
const char *SLCT = "select";
const char *NB = "number";
const char *BINS = "binary_sensor";
const char *SWTC = "switch";
const char *TXT = "text";
void GestionMQTT() {
  if (MQTTRepet > 0 || Source_Temp == "tempMqtt" || Source == "Pmqtt" || subMQTT == 1) {
    if (testMQTTconnected()) {
      clientMQTT.loop();
      envoiVersMQTT();
    }
  }
}

bool testMQTTconnected() {
  bool connecte = true;
  if (!clientMQTT.connected()) {  // si le mqtt n'est pas connecté (utile aussi lors de la 1ere connexion)
    Serial.println("Connection au serveur MQTT ...");
    byte arr[4];
    arr[0] = MQTTIP & 0xFF;          // 0x78
    arr[1] = (MQTTIP >> 8) & 0xFF;   // 0x56
    arr[2] = (MQTTIP >> 16) & 0xFF;  // 0x34
    arr[3] = (MQTTIP >> 24) & 0xFF;  // 0x12
    String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
    clientMQTT.setServer(host.c_str(), MQTTPort);
    clientMQTT.setCallback(callback);                                                     //Déclaration de la fonction de souscription
    if (clientMQTT.connect(MQTTdeviceName.c_str(), MQTTUser.c_str(), MQTTPwd.c_str())) {  // si l'utilisateur est connecté au mqtt
      StockMessage(MQTTdeviceName + " connecté au broker MQTT");
      if (Source_Temp == "tempMqtt") {
        char TopicV[50];
        sprintf(TopicV, "%s", TopicT.c_str());
        clientMQTT.subscribe(TopicV);
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
      sprintf(StateTopic, "%s/%s_state", MQTTPrefix.c_str(), MQTTdeviceName.c_str());
      byte mac[6];  // the MAC address of your Wifi shield
      WiFi.macAddress(mac);
      sprintf(ESP_ID, "%02x%02x%02x", mac[2], mac[1], mac[0]);  // ID de l'entité pour HA
      sprintf(mdl, "%s%s", "ESP32 - ", ESP_ID);                 // ID de l'entité pour HA
      String mf = "F1ATB - https://f1atb.fr";
      String cu = "http://" + WiFi.localIP().toString();
      String hw = String(ESP.getChipModel()) + " rev." + String(ESP.getChipRevision());
      String sw = Version;
      sprintf(DEVICE, "{\"ids\":\"%s\",\"name\":\"%s\",\"mdl\":\"%s\",\"mf\":\"%s\",\"hw\":\"%s\",\"sw\":\"%s\",\"cu\":\"%s\"}", ESP_ID, nomRouteur.c_str(), mdl, mf.c_str(), hw.c_str(), sw.c_str(), cu.c_str());

    } else {  // si utilisateur pas connecté au mqtt
      StockMessage("Echec connexion MQTT : " + host);
      connecte = false;
      delay(100);
      previousMQTTMillis=millis();
    }
  }
  return connecte;
}
void envoiVersMQTT() {
  unsigned long tps = millis();
  int etat = 0;                                                                     // utilisé pour l'envoie de l'état On/Off des actions.
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
  if (String(topic) == TopicT && Source_Temp == "tempMqtt") {  //Temperature attendue
    temperature = ValJson("temperature", message);
    TemperatureValide = 5;
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
        LesActions[i].tOnOff=ValJson("tOnOff", message);
        LesActions[i].Prioritaire();
      }
    }
  }
  Serial.print(topic);
  Serial.println(Message);
}
//*************************************************************************
//*          CONFIG OF DISCOVERY MESSAGE FOR HOME ASSISTANT  / DOMOTICZ             *
//*************************************************************************


void sendMQTTDiscoveryMsg_global() {
  String ActType;
  String ActifType;
  String ActionDur;
  // augmente la taille du buffer wifi Mqtt (voir PubSubClient.h)
  clientMQTT.setBufferSize(700);  // voir -->#define MQTT_MAX_PACKET_SIZE 256 is the default value in PubSubClient.h
  if (Source == "UxIx2" || Source == "ShellyEm" || Source == "ShellyPro") {
    DeviceToDiscover("PuissanceS_T", "W", "power", "0");
    DeviceToDiscover("PuissanceI_T", "W", "power", "0");
    DeviceToDiscover("Tension_T", "V", "voltage", "2");
    DeviceToDiscover("Intensite_T", "A", "current", "2");
    DeviceToDiscover("PowerFactor_T", "", "power_factor", "2");
    DeviceToDiscover("Energie_T_Soutiree", "Wh", "energy", "0");
    DeviceToDiscover("Energie_T_Injectee", "Wh", "energy", "0");
    DeviceToDiscover("EnergieJour_T_Soutiree", "Wh", "energy", "0");
    DeviceToDiscover("EnergieJour_T_Injectee", "Wh", "energy", "0");
    DeviceToDiscover("Frequence", "Hz", "frequency", "2");
  }
  if (Source_Temp != "tempNo") DeviceToDiscover("Temperature", "°C", "temperature", "1");


  if (Source == "Linky") {
    DeviceTextToDiscover("LTARF", "Option Tarifaire");
    DeviceToDiscover("Code_Tarifaire", "", "", "0");
  }
  if (Source == "Enphase") {
    DeviceToDiscover("PactProd", "W", "power", "0");
    DeviceToDiscover("PactConso_M", "W", "power", "0");
  }

  DeviceToDiscover("PuissanceS_M", "W", "power", "0");
  DeviceToDiscover("PuissanceI_M", "W", "power", "0");
  DeviceToDiscover("Tension_M", "V", "voltage", "2");
  DeviceToDiscover("Intensite_M", "A", "current", "2");
  DeviceToDiscover("PowerFactor_M", "", "power_factor", "2");
  DeviceToDiscover("Energie_M_Soutiree", "Wh", "energy", "0");
  DeviceToDiscover("Energie_M_Injectee", "Wh", "energy", "0");
  DeviceToDiscover("EnergieJour_M_Soutiree", "Wh", "energy", "0");
  DeviceToDiscover("EnergieJour_M_Injectee", "Wh", "energy", "0");

  for (int i = 0; i < NbActions; i++) {
    ActType = "Ouverture_Relais_" + String(i);
    ActifType = "Actif_Relais_" + String(i);
    ActionDur = "Duree_relais_"+ String(i);
    if (i == 0) {
      ActType = "Ouverture_Triac";
      ActifType = "Actif_Triac";
      ActionDur = "Duree_Triac";
    }
    DeviceToDiscover(ActType, "%", "power_factor", "0");  //Type power factor pour etre accepté par HA
    DeviceBin2Discover(ActifType);
    DeviceToDiscover(ActionDur, "h", "duration", "2");  
  }


  Serial.println("Paramètres Auto-Discovery publiés !");

  //clientMQTT.setBufferSize(512);  // go to initial value wifi/mqtt buffer
  Discovered = true;


}  // END OF sendMQTTDiscoveryMsg_global

void DeviceToDiscover(String VarName, String Unit, String Class, String Round) {
  char value[700];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[60];
  char state_class[60];
  String TitleName = String(MQTTdeviceName) + " " + String(VarName);
  sprintf(DiscoveryTopic, "%s/%s/%s_%s/%s", MQTTPrefix.c_str(), SSR, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s|default(0)|round(%s)}}", VarName.c_str(), Round.c_str());
  sprintf(state_class, "%s", "");
  if (Unit == "Wh" || Unit == "kWh") {
    sprintf(state_class, "\"state_class\":\"total_increasing\"%s,", state_class);
  }
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"device_class\": \"%s\",\"unit_of_meas\": \"%s\",%s\"val_tpl\": \"%s\",\"device\": %s}", TitleName.c_str(), UniqueID, StateTopic, Class.c_str(), Unit.c_str(), state_class, ValTpl, DEVICE);
  clientMQTT.publish(DiscoveryTopic, value);
}
void DeviceBin2Discover(String VarName) {
  char value[700];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[60];
  int init = 0;  // default value
  String ic = "mdi:electric-switch";
  String TitleName = String(MQTTdeviceName) + " " + String(VarName);
  sprintf(DiscoveryTopic, "%s/%s/%s_%s/%s", MQTTPrefix.c_str(), BINS, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s}}", VarName.c_str());
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"init\": %d,\"ic\": \"%s\",\"payload_off\":\"0\",\"payload_on\":\"1\",\"val_tpl\": \"%s\",\"device\": %s}", TitleName.c_str(), UniqueID, StateTopic, init, ic.c_str(), ValTpl, DEVICE);
  clientMQTT.publish(DiscoveryTopic, value);
}
void DeviceBinToDiscover(String VarName, String TitleName) {
  char value[700];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[60];
  String init = "OFF";  // default value
  String ic = "mdi:electric-switch";
  sprintf(DiscoveryTopic, "%s/%s/%s_%s/%s", MQTTPrefix.c_str(), BINS, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s}}", VarName.c_str());
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"init\": \"%s\",\"ic\": \"%s\",\"val_tpl\": \"%s\",\"device\": %s}", TitleName.c_str(), UniqueID, StateTopic, init.c_str(), ic.c_str(), ValTpl, DEVICE);
  clientMQTT.publish(DiscoveryTopic, value);
}


void DeviceTextToDiscover(String VarName, String TitleName) {
  char value[600];
  char DiscoveryTopic[120];
  char UniqueID[50];
  char ValTpl[50];
  sprintf(DiscoveryTopic, "%s/%s/%s_%s/%s", MQTTPrefix.c_str(), SSR, MQTTdeviceName.c_str(), VarName.c_str(), "config");
  sprintf(UniqueID, "%s_%s", MQTTdeviceName.c_str(), VarName.c_str());
  sprintf(ValTpl, "{{ value_json.%s }}", VarName.c_str());
  sprintf(value, "{\"name\": \"%s\",\"uniq_id\": \"%s\",\"stat_t\": \"%s\",\"device_class\": \"%s\",\"val_tpl\": \"%s\",\"device\": %s}", TitleName.c_str(), UniqueID, StateTopic, "enum", ValTpl, DEVICE);
  clientMQTT.publish(DiscoveryTopic, value);
}
//****************************************
//* ENVOIE DES DATAS VERS HOME ASSISTANT *
//****************************************

void SendDataToHomeAssistant() {
  String ActType;
  String ActifType;
  String ActionDur;
  char value[1000];
  sprintf(value, "{\"PuissanceS_M\": %d, \"PuissanceI_M\": %d, \"Tension_M\": %.1f, \"Intensite_M\": %.1f, \"PowerFactor_M\": %.2f, \"Energie_M_Soutiree\":%d,\"Energie_M_Injectee\":%d, \"EnergieJour_M_Soutiree\":%d, \"EnergieJour_M_Injectee\":%d", PuissanceS_M, PuissanceI_M, Tension_M, Intensite_M, PowerFactor_M, Energie_M_Soutiree, Energie_M_Injectee, EnergieJour_M_Soutiree, EnergieJour_M_Injectee);

  if (Source == "UxIx2" || Source == "ShellyEm" || Source == "ShellyPro") {
    sprintf(value, "%s,\"PuissanceS_T\": %d, \"PuissanceI_T\": %d, \"Tension_T\": %.1f, \"Intensite_T\": %.1f, \"PowerFactor_T\": %.2f, \"Energie_T_Soutiree\":%d,\"Energie_T_Injectee\":%d, \"EnergieJour_T_Soutiree\":%d, \"EnergieJour_T_Injectee\":%d, \"Frequence\":%.2f", value, PuissanceS_T, PuissanceI_T, Tension_T, Intensite_T, PowerFactor_T, Energie_T_Soutiree, Energie_T_Injectee, EnergieJour_T_Soutiree, EnergieJour_T_Injectee, Frequence);
  }
  if (temperature > -100 && Source_Temp != "tempNo") {
    sprintf(value, "%s,\"Temperature\": %.1f", value, temperature);
  }
  if (Source == "Linky") {
    int code = 0;
    if (LTARF.indexOf("HEURE  CREUSE") >= 0) code = 1;  //Code Linky
    if (LTARF.indexOf("HEURE  PLEINE") >= 0) code = 2;
    if (LTARF.indexOf("HC BLEU") >= 0) code = 11;
    if (LTARF.indexOf("HP BLEU") >= 0) code = 12;
    if (LTARF.indexOf("HC BLANC") >= 0) code = 13;
    if (LTARF.indexOf("HP BLANC") >= 0) code = 14;
    if (LTARF.indexOf("HC ROUGE") >= 0) code = 15;
    if (LTARF.indexOf("HP ROUGE") >= 0) code = 16;
    if (LTARF.indexOf("TEMPO_BLEU") >= 0) code = 17;  // Code EDF
    if (LTARF.indexOf("TEMPO_BLANC") >= 0) code = 18;
    if (LTARF.indexOf("TEMPO_ROUGE") >= 0) code = 19;
    sprintf(value, "%s,\"LTARF\":\"%s\", \"Code_Tarifaire\":%d", value, LTARF, code);
  }

  if (Source == "Enphase") {
    sprintf(value, "%s,\"PactProd\":%d, \"PactConso_M\":%d", value, PactProd, PactConso_M);
  }

  for (int i = 0; i < NbActions; i++) {
    ActType = "Ouverture_Relais_" + String(i);
    ActifType = "Actif_Relais_" + String(i);
    ActionDur = "Duree_relais_"+ String(i);
    if (i == 0) {
      ActType = "Ouverture_Triac";
      ActifType = "Actif_Triac";
      ActionDur = "Duree_Triac";
    }
    int Ouv = 100 - Retard[i];
    sprintf(value, "%s,\"%s\":%d", value, ActType.c_str(), Ouv);
    if (Ouv != 0) {
      sprintf(value, "%s,\"%s\":%d", value, ActifType.c_str(), 1);
    }
    else{
      sprintf(value, "%s,\"%s\":%d", value, ActifType.c_str(), 0);
    }
    sprintf(value, "%s,\"%s\":%f", value, ActionDur.c_str(), H_Ouvre[i]);
  }
  sprintf(value, "%s}", value);
  bool published = clientMQTT.publish(StateTopic, value);
}
