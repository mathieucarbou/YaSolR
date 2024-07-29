
void Setup_Enphase() {

  //Obtention Session ID
  //********************
  const char* server1Enphase = "enlighten.enphaseenergy.com";
  String Host = String(server1Enphase);
  String adrEnphase = "https://" + Host + "/login/login.json";
  String requestBody = "user[email]=" + EnphaseUser + "&user[password]=" + urlEncode( EnphasePwd);

  if (EnphaseUser != "" && EnphasePwd != "") {
    Serial.println("Essai connexion  Enlighten server 1 pour obtention session_id!");
    clientSecu.setInsecure();  //skip verification
    if (!clientSecu.connect(server1Enphase, 443))
      StockMessage("Connection failed to Enlighten server :" + Host);
    else {
      Serial.println("Connected to Enlighten server:" + Host);
      clientSecu.println("POST " + adrEnphase + "?" + requestBody + " HTTP/1.0");
      clientSecu.println("Host: " + Host);
      clientSecu.println("Connection: close");
      clientSecu.println();
      String line = "";
      while (clientSecu.connected()) {
        line = clientSecu.readStringUntil('\n');
        if (line == "\r") {
          Serial.println("headers 1 Enlighten received");
          JsonToken = "";
        }

        JsonToken += line;
      }
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (clientSecu.available()) {
        char c = clientSecu.read();
        Serial.write(c);
      }
      clientSecu.stop();
    }
    Session_id = StringJson("session_id", JsonToken);
    Serial.println("session_id :" + Session_id);
  } else {
    Serial.println("Connexion  vers Envoy-S en firmware version 5");
  }
  //Obtention Token
  //********************
  if (Session_id != "" && EnphaseSerial != "" && EnphaseUser != "") {
    const char* server2Enphase = "entrez.enphaseenergy.com";
    Host = String(server2Enphase);
    adrEnphase = "https://" + Host + "/tokens";
    requestBody = "{\"session_id\":\"" + Session_id + "\", \"serial_num\":" + EnphaseSerial + ", \"username\":\"" + EnphaseUser + "\"}";
    Serial.println("Essai connexion  Enlighten server 2 pour obtention token!");
    clientSecu.setInsecure();  //skip verification
    if (!clientSecu.connect(server2Enphase, 443))
      StockMessage("Connection failed to :" + Host);
    else {
      Serial.println("Connected to :" + Host);
      clientSecu.println("POST " + adrEnphase + " HTTP/1.0");
      clientSecu.println("Host: " + Host);
      clientSecu.println("Content-Type: application/json");
      clientSecu.println("content-length:" + String(requestBody.length()));
      clientSecu.println("Connection: close");
      clientSecu.println();
      clientSecu.println(requestBody);
      clientSecu.println();
      Serial.println("Attente user est connecté");
      String line = "";
      JsonToken = "";
      while (clientSecu.connected()) {
        line = clientSecu.readStringUntil('\n');
        if (line == "\r") {
          Serial.println("headers 2 enlighten received");
          JsonToken = "";
        }

        JsonToken += line;
      }
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (clientSecu.available()) {
        char c = clientSecu.read();
        Serial.write(c);
      }
      clientSecu.stop();
      JsonToken.trim();
      Serial.println("Token :" + JsonToken);
      if (JsonToken.length() > 50) {
        TokenEnphase = JsonToken;
        previousTimeRMSMin = 1000;
        previousTimeRMSMax = 1;
        previousTimeRMSMoy = 1;
        previousTimeRMS = millis();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 1000;
      }
    }
  }
}

void LectureEnphase() {  //Lecture des consommations
  int Num_portIQ = 443;
  String JsonEnPhase = "";
  byte arr[4];
  arr[0] = RMSextIP & 0xFF;          // 0x78
  arr[1] = (RMSextIP >> 8) & 0xFF;   // 0x56
  arr[2] = (RMSextIP >> 16) & 0xFF;  // 0x34
  arr[3] = (RMSextIP >> 24) & 0xFF;  // 0x12
  String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);

  if (TokenEnphase.length() > 50 && EnphaseUser != "") {  //Connexion por firmware V7
    if (millis() > 2592000000) {                          //Tout les 30 jours on recherche un nouveau Token
      Setup_Enphase();
    }

    clientSecu.setInsecure();  //skip verification
    if (!clientSecu.connect(host.c_str(), Num_portIQ)) {
      StockMessage("Connection failed to Envoy-S server! : " + host);
    } else {
      //Serial.println("Connected to Envoy-S server!");
      clientSecu.println("GET https://" + host + "/ivp/meters/reports/consumption HTTP/1.0");
      clientSecu.println("Host: " + host);
      clientSecu.println("Accept: application/json");
      clientSecu.println("Authorization: Bearer " + TokenEnphase);
      clientSecu.println("Connection: close");
      clientSecu.println();

      String line = "";
      while (clientSecu.connected()) {
        line = clientSecu.readStringUntil('\n');
        if (line == "\r") {
          //Serial.println("headers received");
          JsonEnPhase = "";
        }
        JsonEnPhase += line;
      }
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (clientSecu.available()) {
        char c = clientSecu.read();
        Serial.write(c);
      }

      clientSecu.stop();
    }
  } else { // Conexion Envoy V5
    // Use WiFiClient class to create TCP connections http
    WiFiClient clientFirmV5;
    if (!clientFirmV5.connect(host.c_str(), 80)) {
      StockMessage("connection to client clientFirmV5 failed (call to Envoy-S)");
      delay(200);
      return;
    }
    String url = "/ivp/meters/reports/consumption";
    clientFirmV5.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");;
    unsigned long timeout = millis();
    while (clientFirmV5.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> client clientFirmV5 Timeout !");
        clientFirmV5.stop();
        return;
      }
    }
    timeout = millis();
    String line;
    // Lecture des données brutes distantes
    while (clientFirmV5.available() && (millis() - timeout < 5000)) {
      line = clientFirmV5.readStringUntil('\n');
      if (line == "\r") {
          //Serial.println("headers received");
          JsonEnPhase = "";
        }
        JsonEnPhase += line;
    }
  }
 
  // On utilise pas la librairie ArduinoJson.h, pour décoder message Json, qui crache sur de grosses données
  String TotConso = PrefiltreJson("total-consumption", "cumulative", JsonEnPhase);
  PactConso_M = int(ValJson("actPower", TotConso));
  String NetConso = PrefiltreJson("net-consumption", "cumulative", JsonEnPhase);
  float PactReseau = ValJson("actPower", NetConso);
  PactReseau = PfloatMax(PactReseau);
  if (PactReseau < 0) {
    PuissanceS_M_inst = 0;
    PuissanceI_M_inst = int(-PactReseau);
  } else {
    PuissanceI_M_inst = 0;
    PuissanceS_M_inst = int(PactReseau);
  }
  float PvaReseau = ValJson("apprntPwr", NetConso);
  PvaReseau = PfloatMax(PvaReseau);
  if (PvaReseau < 0) {
    PVAS_M_inst = 0;
    PVAI_M_inst = int(-PvaReseau);
  } else {
    PVAI_M_inst = 0;
    PVAS_M_inst = int(PvaReseau);
  }
  Pva_valide=true;
  filtre_puissance();
  float PowerFactor = 0;
  if ((PVA_M_moy ) != 0) {
    PowerFactor = floor(100 * abs(Puissance_M_moy ) / PVA_M_moy ) / 100;
    PowerFactor = min(PowerFactor, float(1));
  }
  PowerFactor_M = PowerFactor;
  long whDlvdCum = LongJson("whDlvdCum", NetConso);
  long DeltaWh = 0;
  if (whDlvdCum != 0) {  // bonne donnée
    if (LastwhDlvdCum == 0) {
      LastwhDlvdCum = whDlvdCum;
    }
    DeltaWh = whDlvdCum - LastwhDlvdCum;
    LastwhDlvdCum = whDlvdCum;
    if (DeltaWh < 0) {
      Energie_M_Injectee = Energie_M_Injectee - DeltaWh;
    } else {
      Energie_M_Soutiree = Energie_M_Soutiree + DeltaWh;
    }
  }
  Tension_M = ValJson("rmsVoltage", NetConso);
  Intensite_M = ValJson("rmsCurrent", NetConso);
  PactProd = PactConso_M - int(PactReseau);
  EnergieActiveValide = true;
  if (PactReseau != 0 || PvaReseau != 0) {
    PuissanceRecue=true;  //Reset du Watchdog à chaque trame  reçue de la passerelle Envoy-S metered
  }
  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}

String PrefiltreJson(String F1, String F2, String Json) {
  int p = Json.indexOf(F1);
  Json = Json.substring(p);
  p = Json.indexOf(F2);
  Json = Json.substring(p);
  return Json;
}

float ValJson(String nom, String Json) {
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  p = Json.indexOf("}");
  p = min(p, q);
  float val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toFloat();
  }
  return val;
}
long LongJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(".");
  p = Json.indexOf("}");
  p = min(p, q);
  long val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}

long myLongJson(String nom, String Json) {  // Alternative a LongJson au dessus pour extraire chez EDF nb jour Tempo  https://particulier.edf.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");//<==== Recherche d'une virgule et non d'un point
  if (q == -1) q = 999;  //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
  p = Json.indexOf("}");
  p = min(p, q);
  long val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
unsigned long ULongJson(String nom, String Json) {  // Alternative a LongJson au dessus pour extraire chez EDF nb jour Tempo  https://particulier.edf.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");//<==== Recherche d'une virgule et non d'un point
  if (q == -1) q = 999;  //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
  p = Json.indexOf("}");
  p = min(p, q);
  unsigned long val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    Json ="0000"  + Json;
    int L=Json.length();
    unsigned long y = (Json.substring(0, L-5)).toInt(); //Problème des valeurs signées dans un unsigned
    unsigned long z = (Json.substring(L-5)).toInt();
    val = (y * 100000) + z;
  }
  return val;
}
int IntJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  int val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
byte  ByteJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  byte val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
unsigned short UShortJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  unsigned short val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
unsigned short ShortJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  short val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}


String StringJson(String nom, String Json) {
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  p = Json.indexOf("\"");
  Json = Json.substring(p + 1);
  p = Json.indexOf("\"");
  Json = Json.substring(0, p);
  return Json;
}
