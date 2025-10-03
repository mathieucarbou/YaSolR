// ****************************************************
// * Client d'un Shelly Em sur voie 0 ou 1 ou triphasé*
// ****************************************************

String ReadStringMulti(WiFiClient &client, const String &terminator, uint32_t timeout = 1000) {
  String result = "";
  unsigned long start = millis();

  while (millis() - start < timeout) {
    while (client.available()) {
      char c = client.read();
      result += c;

      // Vérifie si la fin correspond au terminateur
      if (result.endsWith(terminator)) {
        // Supprime le terminateur si tu ne veux pas le garder
        result.remove(result.length() - terminator.length());
        return result;
      }
      start = millis();  // reset du timeout à chaque caractère reçu
    }
  }
  return result;  // timeout atteint
}


void LectureShellyEm() {
  String S = "";
  String Shelly_Data = "";
  float Pw = 0;
  float voltage = 0;
  float pf = 0;
  float Pva;
  int p = 0;
  // Use WiFiClient class to create TCP connections
  WiFiClient clientESP_RMS;
  String host = IP2String(RMSextIP);
  if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {
    delay(500);
    if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {
      delay(100);
      clientESP_RMS.stop();
      StockMessage("connection to Shelly Em failed : " + host);
      return;
    }
  }
  int voie = EnphaseSerial.toInt();
  int Voie = voie % 2;

  if (ShEm_comptage_appels == 1) {
    Voie = (Voie + 1) % 2;
  }
  
  String url = "/emeter/" + String(Voie);
  if (voie == 3)  url = "/status";  //Triphasé Shelly 3Em
  if (voie >= 30) url = "/rpc/Shelly.GetStatus";
  
  ShEm_comptage_appels = (ShEm_comptage_appels + 1) % 5;  // 1 appel sur 6 vers la deuxième voie qui ne sert pas au routeur
  clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (clientESP_RMS.available() == 0) {
    if (millis() - timeout > 5000) {
      StockMessage("client Shelly Em Timeout ! : " + host);
      clientESP_RMS.stop();
      delay(100);
      return;
    }
  }
  if (voie ==3) {   //Triphasé
    Shelly_Data = ReadStringMulti(clientESP_RMS, "fs_mounted");
  } else {
    timeout = millis();
    // Lecture des données brutes distantes jusqu'à Fin (Defini d'après ChatGPT et à verifier pour tous les Shelly)
    while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
      Shelly_Data += clientESP_RMS.readStringUntil('}');
    }
    Shelly_Data +="}";
  }
  clientESP_RMS.stop();
  p = Shelly_Data.indexOf("{");
  Shelly_Data = Shelly_Data.substring(p) ;
  if (voie == 3) {  //Triphasé
    ShEm_dataBrute = "<strong>Triphasé</strong><br>" + Shelly_Data;
    p = Shelly_Data.indexOf("emeters");
    Shelly_Data = Shelly_Data.substring(p + 10);
    Pw = PfloatMax(ValJson("power", Shelly_Data));  //Phase 1
    pf = ValJson("pf", Shelly_Data);
    pf = abs(pf);
    float total_Pw = Pw;
    float total_Pva = 0;
    if (pf > 0) {
      total_Pva = abs(Pw) / pf;
    }
    float total_E_soutire = ValJson("total", Shelly_Data);
    float total_E_injecte = ValJson("total_returned", Shelly_Data);
    p = Shelly_Data.indexOf("}");
    Shelly_Data = Shelly_Data.substring(p + 1);
    Pw = PfloatMax(ValJson("power", Shelly_Data));  //Phase 2
    pf = ValJson("pf", Shelly_Data);
    pf = abs(pf);
    total_Pw += Pw;
    if (pf > 0) {
      total_Pva += abs(Pw) / pf;
    }
    total_E_soutire += ValJson("total", Shelly_Data);
    total_E_injecte += ValJson("total_returned", Shelly_Data);
    p = Shelly_Data.indexOf("}");
    Shelly_Data = Shelly_Data.substring(p + 1);
    Pw = PfloatMax(ValJson("power", Shelly_Data));  //Phase 3
    pf = ValJson("pf", Shelly_Data);
    pf = abs(pf);
    total_Pw += Pw;
    if (pf > 0) {
      total_Pva += abs(Pw) / pf;
    }
    total_E_soutire += ValJson("total", Shelly_Data);
    total_E_injecte += ValJson("total_returned", Shelly_Data);
    Energie_M_Soutiree = int(total_E_soutire);
    Energie_M_Injectee = int(total_E_injecte);
    if (total_Pw == 0) {
      total_Pva = 0;
    }
    if (total_Pw > 0) {
      PuissanceS_M_inst = total_Pw;
      PuissanceI_M_inst = 0;
      PVAS_M_inst = total_Pva;
      PVAI_M_inst = 0;
    } else {
      PuissanceS_M_inst = 0;
      PuissanceI_M_inst = -total_Pw;
      PVAI_M_inst = total_Pva;
      PVAS_M_inst = 0;
    }
  } else if (voie < 3) {  //Monophasé Shelly Em de base
    ShEm_dataBrute = "<strong>Voie : " + String(Voie) + "</strong><br>" + Shelly_Data;
    Shelly_Data = Shelly_Data + ",";
    if (Shelly_Data.indexOf("true") > 0) {  // Donnée valide
      Pw = PfloatMax(ValJson("power", Shelly_Data));
      voltage = ValJson("voltage", Shelly_Data);
      pf = ValJson("pf", Shelly_Data);
      pf = abs(pf);
      if (pf > 1) pf = 1;
      if (Voie == voie) {  //voie du routeur
        if (Pw >= 0) {
          PuissanceS_M_inst = Pw;
          PuissanceI_M_inst = 0;
          if (pf > 0.01) {
            PVAS_M_inst = PfloatMax(Pw / pf);
          } else {
            PVAS_M_inst = 0;
          }
          PVAI_M_inst = 0;
        } else {
          PuissanceS_M_inst = 0;
          PuissanceI_M_inst = -Pw;
          if (pf > 0.01) {
            PVAI_M_inst = PfloatMax(-Pw / pf);
          } else {
            PVAI_M_inst = 0;
          }
          PVAS_M_inst = 0;
        }
        Energie_M_Soutiree = int(ValJson("total", Shelly_Data));
        Energie_M_Injectee = int(ValJson("total_returned", Shelly_Data));
        PowerFactor_M = pf;
        Tension_M = voltage;
        Pva_valide = true;
      } else {  // voie secondaire
        if (LissageLong) {
          PwMoy2 = 0.2 * Pw + 0.8 * PwMoy2;  //Lissage car moins de mesure sur voie secondaire
          pfMoy2 = 0.2 * pf + 0.8 * pfMoy2;
          Pw = PwMoy2;
          pf = pfMoy2;
        }
        if (Pw >= 0) {
          PuissanceS_T_inst = Pw;
          PuissanceI_T_inst = 0;
          if (pf > 0.01) {
            PVAS_T_inst = PfloatMax(Pw / pf);
          } else {
            PVAS_T_inst = 0;
          }
          PVAI_T_inst = 0;
        } else {
          PuissanceS_T_inst = 0;
          PuissanceI_T_inst = -Pw;
          if (pf > 0.01) {
            PVAI_T_inst = PfloatMax(-Pw / pf);
          } else {
            PVAI_T_inst = 0;
          }
          PVAS_T_inst = 0;
        }
        Energie_T_Soutiree = int(ValJson("total", Shelly_Data));
        Energie_T_Injectee = int(ValJson("total_returned", Shelly_Data));
        PowerFactor_T = pf;
        Tension_T = voltage;
      }
    }

  } else {  //Shelly Em Gen3  . On recupere les 2 voies à chaque message
    Voie = voie % 2;
    ShEm_dataBrute = "<strong>Shelly Em Gen3 Voie : " + String(Voie) + "</strong><br>" + Shelly_Data;
    if (Shelly_Data.indexOf("ssid") > 0) {  // Donnée valide
      Shelly_Data = SubJson("em1:" + String(Voie), "}", Shelly_Data);
      Pw = PfloatMax(ValJson("act_power", Shelly_Data));
      pf = ValJson("pf", Shelly_Data);
      voltage = ValJson("voltage", Shelly_Data);
      if (Pw >= 0) {
        PuissanceS_M_inst = Pw;
        PuissanceI_M_inst = 0;
        if (pf > 0.01) {
          PVAS_M_inst = PfloatMax(Pw / pf);
        } else {
          PVAS_M_inst = 0;
        }
        PVAI_M_inst = 0;
      } else {
        PuissanceS_M_inst = 0;
        PuissanceI_M_inst = -Pw;
        if (pf > 0.01) {
          PVAI_M_inst = PfloatMax(-Pw / pf);
        } else {
          PVAI_M_inst = 0;
        }
        PVAS_M_inst = 0;
      }
      Shelly_Data = SubJson("em1data:" + String(Voie), "}", ShEm_dataBrute);
      Energie_M_Soutiree = int(ValJson("total_act_energy", Shelly_Data));
      Energie_M_Injectee = int(ValJson("total_act_ret_energy", Shelly_Data));
      PowerFactor_M = pf;
      Tension_M = voltage;
      Pva_valide = true;
      Voie = (Voie + 1) % 2;
      Shelly_Data = SubJson("em1:" + String(Voie), "}", ShEm_dataBrute);
      Pw = PfloatMax(ValJson("act_power", Shelly_Data));
      pf = ValJson("pf", Shelly_Data);
      if (Pw >= 0) {
        PuissanceS_T_inst = Pw;
        PuissanceI_T_inst = 0;
        if (pf > 0.01) {
          PVAS_T_inst = PfloatMax(Pw / pf);
        } else {
          PVAS_T_inst = 0;
        }
        PVAI_T_inst = 0;
      } else {
        PuissanceS_T_inst = 0;
        PuissanceI_T_inst = -Pw;
        if (pf > 0.01) {
          PVAI_T_inst = PfloatMax(-Pw / pf);
        } else {
          PVAI_T_inst = 0;
        }
        PVAS_T_inst = 0;
      }
      Shelly_Data = SubJson("em1data:" + String(Voie), "}", ShEm_dataBrute);
      Energie_T_Soutiree = int(ValJson("total_act_energy", Shelly_Data));
      Energie_T_Injectee = int(ValJson("total_act_ret_energy", Shelly_Data));
      PowerFactor_T = pf;
      Tension_T = voltage;
    }
  }
  filtre_puissance();
  PuissanceRecue = true;  //Reset du Watchdog à chaque trame du Shelly reçue
  if (ShEm_comptage_appels > 1) EnergieActiveValide = true;
  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}
