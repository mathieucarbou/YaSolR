//*********************************************************************
// Variante Shelly Pro Em proposé par Raphael591 (Juillet 2024)       *
//  + Correction Octobre 2024  et Janvier 2025                        *
// ********************************************************************
// dash (février 2025) - Adaptation à l'utilisation du Pro 3EM en     *
// monophasé tout en ayant conservé le profile du matériel en triphasé*
// ********************************************************************
// * Paramétrage de la voie pour le shelly pro 3 em :                 *
// *   - 3 sur le triphasé.                                           *
// *   - 0 ou 1 ou 2 sur le monophasé fonction de la pince à utiliser.*
// ********************************************************************

void LectureShellyProEm() {
  String S = "";
  String Shelly_Data = "";
  bool Shelly_Triphase_As_Monophase = false;
  String Shelly_Phase = "";
  float Pw = 0;
  float voltage = 0;
  float pf = 0;

  // ADD PERSO : AJOUT VARIABLE JSON pour facilité la lecture des infos EM PRO
  String tmp;  // ADD PERSO

  // Use WiFiClient class to create TCP connections
  WiFiClient clientESP_RMS;
  String host = IP2String(RMSextIP);

  int voie = EnphaseSerial.toInt();
  int Voie = voie % 3;  //Pro3Em trois voies
  int p;
  if (ShEm_comptage_appels == 1) {
    Voie = (Voie + 1) % 3;  //Pro3Em trois voies
  }
  unsigned long timeout = millis();
  String url = "/rpc/Shelly.GetDeviceInfo";
  // Connaître modèle du shelly *******************************************
  if (Shelly_Name == "") {
    if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {
      StockMessage("connection to Shelly Pro Em failed : " + host);
      clientESP_RMS.stop();
      delay(200);
      return;
    }

    clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

    while (clientESP_RMS.available() == 0) {
      if (millis() - timeout > 5000) {
        StockMessage("client Shelly Em Timeout ! : " + host);
        clientESP_RMS.stop();
        delay(200);
        return;
      }
    }
    timeout = millis();
    // Lecture des données brutes distantes
    while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
      Shelly_Data += clientESP_RMS.readStringUntil('\r');
    }
    Shelly_Name = StringJson("id", Shelly_Data);
    p = Shelly_Name.indexOf("-");
    Shelly_Name = Shelly_Name.substring(0, p);
    Shelly_Profile = StringJson("profile", Shelly_Data);
    Shelly_Data = "";
  }
  // Modèle shelly FIN ******

  // Protocole monophasé ou triphasé DÉBUT
  if (Shelly_Profile == "triphase" && voie != 3) {
    Shelly_Triphase_As_Monophase = true;
  }
  // Protocole monophasé ou triphasé FIN
  if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {
    delay(500);
    if (!clientESP_RMS.connect(host.c_str(), 80, 3000)) {
      delay(100); //Necessaire
      clientESP_RMS.stop();
      StockMessage("connection to Shelly Em failed : " + host);
      
      delay(200);
      return;
    }
  }

  url = "/rpc/Shelly.GetStatus";                          // pour Pro Em
  ShEm_comptage_appels = (ShEm_comptage_appels + 1) % 5;  // 1 appel sur 6 vers la deuxième voie qui ne sert pas au routeur
  clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  timeout = millis();
  while (clientESP_RMS.available() == 0) {
    if (millis() - timeout > 5000) {
      StockMessage("client Shelly Em Timeout 2 ! : " + host);
      clientESP_RMS.stop();
      delay(100);
      return;
    }
  }
  timeout = millis();
  // Lecture des données brutes distantes
  while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
    Shelly_Data += clientESP_RMS.readStringUntil('}');
  }
  clientESP_RMS.stop();
  p = Shelly_Data.indexOf("{");
  Shelly_Data = Shelly_Data.substring(p) +"}}";
  if (Shelly_Name == "shellypro3em" && voie == 3) {
    // 3 em Triphasé
    ShEm_dataBrute = "<strong>" + Shelly_Name + "</strong><br>" + Shelly_Data;

    float pf1, pf2, pf3;
    float volt1, volt2, volt3;

    tmp = PrefiltreJson("em:0", ":", Shelly_Data);

    Pw = ValJson("total_act_power", tmp);

    volt1 = ValJson("a_voltage", tmp);
    pf1 = ValJson("a_pf", tmp);

    volt2 = ValJson("b_voltage", tmp);
    pf2 = ValJson("b_pf", tmp);

    volt3 = ValJson("c_voltage", tmp);
    pf3 = ValJson("c_pf", tmp);

    voltage = (volt1 + volt2 + volt3) / 3;
    pf = abs((pf1 + pf2 + pf3) / 3);
    if (pf > 1) pf = 1;
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
    tmp = PrefiltreJson("emdata:0", ":", Shelly_Data);      // ADD PERSO
    Energie_M_Soutiree = myLongJson("total_act", tmp);      // ADD PERSO
    Energie_M_Injectee = myLongJson("total_act_ret", tmp);  // ADD PERSO
    PowerFactor_M = pf;
    Tension_M = voltage;
    Pva_valide = true;
  } else if (Shelly_Name == "shellypro3em" && Shelly_Triphase_As_Monophase) {
    // on utilise le code ASCII de a (97) pour obtenir le prefix de la phase. Voie 0=a 1=b 2=c
    Shelly_Phase = char(voie + 97);

    // 3 em Monophasé : Voie != 3 et profile Triphase actif
    ShEm_dataBrute = "<strong>" + Shelly_Name + " " + "Phase " + Shelly_Phase + "</strong><br>" + Shelly_Data;

    tmp = PrefiltreJson("em:0", ":", Shelly_Data);

    Pw = ValJson(Shelly_Phase + "_act_power", tmp);
    voltage = ValJson(Shelly_Phase + "_voltage", tmp);
    pf = abs(ValJson(Shelly_Phase + "_pf", tmp));

    if (pf > 1) pf = 1;
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
    tmp = PrefiltreJson("emdata:0", ":", Shelly_Data);                             // ADD PERSO
    Energie_M_Soutiree = myLongJson(Shelly_Phase + "_total_act_energy", tmp);      // ADD PERSO
    Energie_M_Injectee = myLongJson(Shelly_Phase + "_total_act_ret_energy", tmp);  // ADD PERSO
    PowerFactor_M = pf;
    Tension_M = voltage;
    Pva_valide = true;
  } else if (Shelly_Name == "shellypro3em") {
    // 3 em Monophasé : Voie != 3
    ShEm_dataBrute = "<strong>" + Shelly_Name + "</strong><br>" + Shelly_Data;

    tmp = PrefiltreJson("em1:" + String(Voie), ":", Shelly_Data);  // Voie mono
    Pw = ValJson("act_power", tmp);
    voltage = ValJson("voltage", tmp);
    pf = ValJson("pf", tmp);
    pf = abs(pf);
    if (pf > 1) pf = 1;
    if (Voie == voie) {  // voie du routeur
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
      tmp = PrefiltreJson("em1data:" + String(Voie), ":", Shelly_Data);  // ADD PERSO
      Energie_M_Soutiree = myLongJson("total_act_energy", tmp);          // ADD PERSO
      Energie_M_Injectee = myLongJson("total_act_ret_energy", tmp);      // ADD PERSO
      PowerFactor_M = pf;
      Tension_M = voltage;
      Pva_valide = true;
    } else {  // voie secondaire
      if (LissageLong) {
        PwMoy2 = 0.2 * Pw + 0.8 * PwMoy2;  // Lissage car moins de mesure sur voie secondaire
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
      tmp = PrefiltreJson("em1data:" + String(Voie), ":", Shelly_Data);  // ADD PERSO
      Energie_T_Soutiree = myLongJson("total_act_energy", tmp);          // ADD PERSO
      Energie_T_Injectee = myLongJson("total_act_ret_energy", tmp);      // ADD PERSO
      PowerFactor_T = pf;
      Tension_T = voltage;
    }
  } else if (Shelly_Name == "shellyproem50") {  // Monophasé pro EM

    //redifinition proEm50 2 voie (0 ou 1)
    Voie = voie % 2;
    if (ShEm_comptage_appels == 1) {
      Voie = (Voie + 1) % 2;
    }

    ShEm_dataBrute = "<strong>" + Shelly_Name + "</strong><br>" + Shelly_Data;
    Shelly_Data = Shelly_Data + ",";
    if (Shelly_Data.indexOf("act_power") > 0) {                      // Donnée valide
      tmp = PrefiltreJson("em1:" + String(Voie), ":", Shelly_Data);  // ADD PERSO
      Pw = ValJson("act_power", tmp);                                // ADD PERSO
      voltage = ValJson("voltage", tmp);                             // ADD PERSO
      pf = ValJson("pf", tmp);                                       // ADD PERSO
      pf = abs(pf);
      if (pf > 1) pf = 1;
      if (Voie == voie) {  // voie du routeur
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
        tmp = PrefiltreJson("em1data:" + String(Voie), ":", Shelly_Data);  // ADD PERSO
        Energie_M_Soutiree = myLongJson("total_act_energy", tmp);          // ADD PERSO
        Energie_M_Injectee = myLongJson("total_act_ret_energy", tmp);      // ADD PERSO
        PowerFactor_M = pf;
        Tension_M = voltage;
        Pva_valide = true;
      } else {  // voie secondaire
        if (LissageLong) {
          PwMoy2 = 0.2 * Pw + 0.8 * PwMoy2;  // Lissage car moins de mesure sur voie secondaire
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
        tmp = PrefiltreJson("em1data:" + String(Voie), ":", Shelly_Data);  // ADD PERSO
        Energie_T_Soutiree = myLongJson("total_act_energy", tmp);          // ADD PERSO
        Energie_T_Injectee = myLongJson("total_act_ret_energy", tmp);      // ADD PERSO
        PowerFactor_T = pf;
        Tension_T = voltage;
      }
    }
  }
  filtre_puissance();
  PuissanceRecue = true;  // Reset du Watchdog à chaque trame du Shelly reçue
  if (ShEm_comptage_appels > 1)
    EnergieActiveValide = true;
  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}