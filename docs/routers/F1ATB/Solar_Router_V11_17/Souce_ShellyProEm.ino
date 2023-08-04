// *************************************************************
// Varaiante Shelle Pro Em proposé par Raphael591 (Juillet 2024)
// ****************************************************
// * Client d'un Shelly Em sur voie 0 ou 1 ou triphasé*
// ****************************************************
void LectureShellyProEm() {
  String S = "";
  String Shelly_Data = "";
  float Pw = 0;
  float voltage = 0;
  float pf = 0;
  
  //ADD PERSO : AJOUT VARIABLE JSON pour facilité la lecture des infos EM PRO
	String tmp; //ADD PERSO

  // Use WiFiClient class to create TCP connections
  WiFiClient clientESP_RMS;
  byte arr[4];
  arr[0] = RMSextIP & 0xFF;          // 0x78
  arr[1] = (RMSextIP >> 8) & 0xFF;   // 0x56
  arr[2] = (RMSextIP >> 16) & 0xFF;  // 0x34
  arr[3] = (RMSextIP >> 24) & 0xFF;  // 0x12

  String host = String(arr[3]) + "." + String(arr[2]) + "." + String(arr[1]) + "." + String(arr[0]);
  if (!clientESP_RMS.connect(host.c_str(), 80)) {
    StockMessage("connection to Shelly Em failed : " + host);
    delay(200);
    return;
  }
  int voie = EnphaseSerial.toInt();
  int Voie = voie % 2;

  if (ShEm_comptage_appels == 1) {
    Voie = (Voie + 1) % 2;
  }

  String url = "/rpc/Shelly.GetStatus"; //pour Pro Em
  ShEm_comptage_appels = (ShEm_comptage_appels + 1) % 5;  // 1 appel sur 6 vers la deuxième voie qui ne sert pas au routeur
  clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (clientESP_RMS.available() == 0) {
    if (millis() - timeout > 5000) {
      StockMessage("client Shelly Em Timeout ! : " + host);
      clientESP_RMS.stop();
      return;
    }
  }
  timeout = millis();
  // Lecture des données brutes distantes
  while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
    Shelly_Data += clientESP_RMS.readStringUntil('\r');
  }
  int p = Shelly_Data.indexOf("{");
  Shelly_Data = Shelly_Data.substring(p);
  if (voie == 3) {  //Triphasé
    ShEm_dataBrute = "<strong>Triphasé</strong><br>" + Shelly_Data;

    tmp=PrefiltreJson("em1:0",":",Shelly_Data); //Phase 1
    Pw = ValJson("act_power",tmp); 
	  voltage = ValJson("voltage",tmp); 
    pf = ValJson("pf",tmp); 
    pf = abs(pf);
    float total_Pw = Pw;
    float total_Pva = 0;
    if (pf > 0) {
      total_Pva = abs(Pw) / pf;
    }
    tmp=PrefiltreJson("em1data:0",":",Shelly_Data); 
    float total_E_soutire = myLongJson("total_act_energy",tmp); 
    float total_E_injecte = myLongJson("total_act_ret_energy",tmp); 

     tmp=PrefiltreJson("em1:1",":",Shelly_Data); //Phase 2
    Pw = ValJson("act_power",tmp); 
	  voltage = ValJson("voltage",tmp); 
    pf = ValJson("pf",tmp); 
    pf = abs(pf);
    total_Pw += Pw;
    if (pf > 0) {
      total_Pva += abs(Pw) / pf;
    }
    tmp=PrefiltreJson("em1data:1",":",Shelly_Data); 
    total_E_soutire += myLongJson("total_act_energy",tmp); 
    total_E_injecte += myLongJson("total_act_ret_energy",tmp); 

     tmp=PrefiltreJson("em1:2",":",Shelly_Data); //Phase 3
    Pw = ValJson("act_power",tmp); 
	  voltage = ValJson("voltage",tmp); 
    pf = ValJson("pf",tmp); 
    pf = abs(pf);
    total_Pw += Pw;
    if (pf > 0) {
      total_Pva += abs(Pw) / pf;
    }
    tmp=PrefiltreJson("em1data:2",":",Shelly_Data); 
    total_E_soutire += myLongJson("total_act_energy",tmp); 
    total_E_injecte += myLongJson("total_act_ret_energy",tmp);
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
  } else {  //Monophasé
    ShEm_dataBrute = "<strong>Voie : " + String(voie) + "</strong><br>" + Shelly_Data;
    Shelly_Data = Shelly_Data + ",";
    if (Shelly_Data.indexOf("true") > 0) {  // Donnée valide
	  tmp=PrefiltreJson("em1:"+ String(Voie),":",Shelly_Data); //ADD PERSO
      Pw = ValJson("act_power",tmp); //ADD PERSO
	  voltage = ValJson("voltage",tmp); //ADD PERSO
	  pf = ValJson("pf",tmp); //ADD PERSO
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
        tmp=PrefiltreJson("em1data:"+ String(Voie),":",Shelly_Data); //ADD PERSO
        Energie_M_Soutiree = myLongJson("total_act_energy",tmp); //ADD PERSO
        Energie_M_Injectee = myLongJson("total_act_ret_energy",tmp); //ADD PERSO
        PowerFactor_M = pf;
        Tension_M = voltage;
        Pva_valide=true;
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
        tmp=PrefiltreJson("em1data:"+ String(Voie),":",Shelly_Data); //ADD PERSO
        Energie_T_Soutiree = myLongJson("total_act_energy",tmp); //ADD PERSO
        Energie_T_Injectee = myLongJson("total_act_ret_energy",tmp); //ADD PERSO
        PowerFactor_T = pf;
        Tension_T = voltage;
      }
    }
  }
  filtre_puissance();
  PuissanceRecue=true; //Reset du Watchdog à chaque trame du Shelly reçue
  if (ShEm_comptage_appels > 1) EnergieActiveValide = true;
  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}
