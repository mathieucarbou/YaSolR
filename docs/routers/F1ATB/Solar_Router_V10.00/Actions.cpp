// ********************
// Gestion des Actions
// ********************
#include <Arduino.h>
#include "Actions.h"
#include "EEPROM.h"
#include <WiFiClient.h>



//Class Action
Action::Action() {
  Gpio = -1;
}
Action::Action(int aIdx) {
  Gpio = -1;  // si le n° de pin n'est pas valid, on ne fait rien
  Idx = aIdx;
  T_LastAction = int(millis() / 1000);
  On = false;
  Actif = 0;
  Reactivite = 50;
  OutOn = 1;
  OutOff = 0;
  Tempo = 0;
  Repet = 0;
  tOnOff = 0;
}



void Action::Arreter() {
  int Tseconde = int(millis() / 1000);
  if ((Tseconde - T_LastAction) >= Tempo || Idx == 0 || Actif != 1) {
    if (Gpio > 0 || Idx == 0) {
      digitalWrite(Gpio, OutOff);
      T_LastAction = Tseconde;
    } else {
      if (On || ((Tseconde - T_LastAction) > Repet && Repet != 0)) {
        CallExterne(Host, OrdreOff, Port);
        T_LastAction = Tseconde;
      }
    }
    On = false;
  }
}
void Action::RelaisOn() {
  int Tseconde = int(millis() / 1000);
  if ((Tseconde - T_LastAction) >= Tempo) {
    if (Gpio > 0) {
      digitalWrite(Gpio, OutOn);
      T_LastAction = Tseconde;
      On = true;
    } else {
      if (Actif == 1) {
        if (!On || ((Tseconde - T_LastAction) > Repet && Repet != 0)) {
          CallExterne(Host, OrdreOn, Port);
          T_LastAction = Tseconde;
        }
        On = true;
      }
    }
  }
}
void Action::Prioritaire() {
  int tempo_ = Tempo;
  if (tOnOff != 0) {
    Tempo = 0;
    if (tOnOff > 0) {
      RelaisOn();
    } else {
      Arreter();
    }
    Tempo = tempo_;
  }
}

void Action::Definir(String ligne) {
  String RS = String((char)30);  //Record Separator
  Actif = byte(ligne.substring(0, ligne.indexOf(RS)).toInt());
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  Titre = ligne.substring(0, ligne.indexOf(RS));
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  Host = ligne.substring(0, ligne.indexOf(RS));
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  Port = ligne.substring(0, ligne.indexOf(RS)).toInt();
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  OrdreOn = ligne.substring(0, ligne.indexOf(RS));
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  OrdreOff = ligne.substring(0, ligne.indexOf(RS));
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  Repet = ligne.substring(0, ligne.indexOf(RS)).toInt();
  Repet = min(Repet, 32000);
  Repet = max(0, Repet);
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  Tempo = ligne.substring(0, ligne.indexOf(RS)).toInt();
  Tempo = min(Tempo, 32000);
  Tempo = max(0, Tempo);
  if (Repet > 0) {
    Repet = max(Tempo + 4, Repet);  //Pour eviter conflit
  }
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  Reactivite = byte(ligne.substring(0, ligne.indexOf(RS)).toInt());
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  NbPeriode = byte(ligne.substring(0, ligne.indexOf(RS)).toInt());
  ligne = ligne.substring(ligne.indexOf(RS) + 1);
  int Hdeb_ = 0;
  for (byte i = 0; i < NbPeriode; i++) {
    Type[i] = byte(ligne.substring(0, ligne.indexOf(RS)).toInt());  //NO,OFF,ON,PW,Triac
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Hfin[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
    Hdeb[i] = Hdeb_;
    Hdeb_ = Hfin[i];
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Vmin[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Vmax[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Tinf[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Tsup[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
    Tarif[i] = ligne.substring(0, ligne.indexOf(RS)).toInt();
    ligne = ligne.substring(ligne.indexOf(RS) + 1);
  }
}
String Action::Lire() {
  String GS = String((char)29);  //Group Separator
  String RS = String((char)30);  //Record Separator
  String S;
  S += String(Actif) + RS;
  S += Titre + RS;
  S += Host + RS;
  S += String(Port) + RS;
  S += OrdreOn + RS;
  S += OrdreOff + RS;
  S += String(Repet) + RS;
  S += String(Tempo) + RS;
  S += String(Reactivite) + RS;
  S += String(NbPeriode) + RS;
  for (byte i = 0; i < NbPeriode; i++) {
    S += String(Type[i]) + RS;
    S += String(Hfin[i]) + RS;
    S += String(Vmin[i]) + RS;
    S += String(Vmax[i]) + RS;
    S += String(Tinf[i]) + RS;
    S += String(Tsup[i]) + RS;
    S += String(Tarif[i]) + RS;
  }
  return S + GS;
}




byte Action::TypeEnCours(int Heure, float Temperature, int Ltarfbin) {  //Retourne type d'action  active à cette heure et test temperature OK
  byte S = 1;
  bool TemperatureOk;
  bool TarifOk;
  for (int i = 0; i < NbPeriode; i++) {
    TemperatureOk = true;
    if (Temperature > -100) {
      if (Tinf[i] <= 1000 && Temperature * 10 > Tinf[i]) { TemperatureOk = false; }
      if (Tsup[i] <= 1000 && Temperature * 10 < Tsup[i]) { TemperatureOk = false; }
    }
    TarifOk = true;
    if (Ltarfbin > 0 && (Ltarfbin & Tarif[i]) == 0) TarifOk = false;
    if (Heure >= Hdeb[i] && Heure <= Hfin[i] && TemperatureOk && TarifOk) S = Type[i];
  }
  if (tOnOff > 0) S = 2;  // Force On
  if (tOnOff < 0) S = 1;  // Force Off
  return S;               //0=NO,1=OFF,2=ON,3=PW,4=Triac
}
int Action::Valmin(int Heure) {  //Retourne la valeur Vmin (ex seuil Triac) à cette heure
  int S = 0;
  for (int i = 0; i < NbPeriode; i++) {
    if (Heure >= Hdeb[i] && Heure <= Hfin[i]) {
      S = Vmin[i];
    }
  }
  return S;
}
int Action::Valmax(int Heure) {  //Retourne la valeur Vmax (ex ouverture du Triac) à cette heure
  int S = 0;
  for (int i = 0; i < NbPeriode; i++) {
    if (Heure >= Hdeb[i] && Heure <= Hfin[i]) {
      S = Vmax[i];
    }
  }
  return S;
}

void Action::InitGpio() {  //Initialise les sorties GPIO pour des relais
  int p;
  String S;
  String IS = String((char)31);  //Input Separator

  if (Idx > 0) {
    T_LastAction = 0;
    Gpio = -1;
    p = OrdreOn.indexOf(IS);
    if (p >= 0) {
      Gpio = OrdreOn.substring(0, p).toInt();
      OutOn = OrdreOn.substring(p+1).toInt();
      OutOff=(1+OutOn)%2;
      if (Gpio > 0) {
        pinMode(Gpio, OUTPUT);
        digitalWrite(Gpio, OutOff);
      }
    }
  }
}
void Action::CallExterne(String host, String url, int port) {
  if (url != "") {
    // Use WiFiClient class to create TCP connections
    WiFiClient clientExt;
    char hostbuf[host.length() + 1];
    host.toCharArray(hostbuf, host.length() + 1);

    if (!clientExt.connect(hostbuf, port)) {
      StockMessage("connection to :" + host + " failed");
      return;
    }
    clientExt.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (clientExt.available() == 0) {
      if (millis() - timeout > 5000) {
        StockMessage(">>> clientESP_Ext Timeout ! : " + host);
        clientExt.stop();
        return;
      }
    }

    // Read all the lines of the reply from server
    while (clientExt.available()) {
      String line = clientExt.readStringUntil('\r');
    }
  }
}