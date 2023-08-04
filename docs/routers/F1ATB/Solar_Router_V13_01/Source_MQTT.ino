
// ******************************************************
// * Informations de puissance reçue via un Broker MQTT *
// ******************************************************
void UpdatePmqtt() {
  float Pw = PfloatMax(PwMQTT);
  float Pf = 1;
  if (P_MQTT_Brute.indexOf("Pf") > 0) {
    Pf = abs(PfMQTT);
  }
  if (P_MQTT_Brute.indexOf("Pva") > 0) {
    if (PvaMQTT != 0) {
      Pf = abs(Pw / PfloatMax(PvaMQTT));
    }
  }
  if (P_MQTT_Brute.indexOf("Pva") > 0 || P_MQTT_Brute.indexOf("Pf") > 0) {
    Pva_valide = true;
  } else {
    Pva_valide = false;
  }
  if (Pf > 1) Pf = 1;
  if (Pw >= 0) {
    PuissanceS_M_inst = Pw;
    PuissanceI_M_inst = 0;
    if (Pf > 0.01) {
      PVAS_M_inst = PfloatMax(Pw / Pf);
    } else {
      PVAS_M_inst = 0;
    }
    PVAI_M_inst = 0;
    EASfloat += Pw / 6000.0;               // Watt Hour,Every 600ms. Soutirée
    Energie_M_Soutiree = int(EASfloat);  // Watt Hour,Every 40ms. Soutirée
  } else {
    PuissanceS_M_inst = 0;
    PuissanceI_M_inst = -Pw;
    if (Pf > 0.01) {
      PVAI_M_inst = PfloatMax(-Pw / Pf);
    } else {
      PVAI_M_inst = 0;
    }
    PVAS_M_inst = 0;
    EAIfloat += -Pw / 6000.0;
    Energie_M_Injectee = int(EAIfloat);
  }

  filtre_puissance();

  if (P_MQTT_Brute.indexOf("Pw") > 0) EnergieActiveValide = true;
  if (millis() - LastPwMQTTMillis < 30000) PuissanceRecue = true;  //Reset du Watchdog si trame  MQTT reçue avec au minimum Pw récente

  if (cptLEDyellow > 30) {
    cptLEDyellow = 4;
  }
}