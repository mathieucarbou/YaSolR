#define Version "13.01"
#define HOSTNAME "RMS-ESP32-"
#define CLE_Rom_Init 912567899  //Valeur pour tester si ROM vierge ou pas. Un changement de valeur remet à zéro toutes les données. / Value to test whether blank ROM or not.

/*
  PV Router / Routeur Solaire 
  ****************************************
  
  RMS=Routeur Multi Sources

  Choix de 10 sources différentes pour lire la consommation électrique en entrée de maison
  - lecture de la tension avec un transformateur et du courant avec une sonde ampèremétrique (UxI)
  - lecture des données du Linky (Linky)
  - module (JSY-MK-194T) intégrant une mesure de tension secteur et 2 sondes ampèmétriques (UxIx2)
  - module (JSY-MK-333) pour une installation triphasé
  - Lecture passerelle Enphase - Envoy-S metered (firmware V5 et V7,V8)
  - Lecture avec Shelly Em
  - Lecture avec Shelly Pro Em
  - Lecture compteur SmartG 
  - Lecture via MQTT
  - Lecture depuis un autre ESP depuis une des sources citées plus haut
  
  En option une à 4 mesures de température en interne (DS18B20), en externe ou via MQTT est possible.

  Historique des versions
  - V9.00_RMS 
    Stockage des températures avec une décimale
    Simplification changement de nom de réseau WIFI
    Choix mode Wifi avec ou sans veille
    Sélection source de température
    Source de puissance reçue via MQTT
    Souscription MQTT à une température externe
    Souscription MQTT pour forcer On ou Off les actionneurs.
  - V9.01_RMS fonctionne avec la bibliothèque ESP32 Version 2.0.17
    Validation Pva_valide pour les Linky en CACSI
  - V9.02_RMS fonctionne avec la bibliothèque ES¨P32 V 3.01 . 
    Suite au passage de la bibliothèque ESP32 en Version 3.01 importants changement pour le routeur sur le WIFI, les Timers, Le Watchdog et la partition mémoire FLASH. 
    Attention à ne pas utiliser la bibliothèque ESP32 en Version 3.00, elle est bugée et génère 20% de plus de code.
    Filtrage des températures pour tolérer une perte éventuelle de mesure
  - V9.03_RMS 
    Suite au changement de bibliothèque ESP32 en V3.0.1, le scan réseau pour un changement de nom de WIFI ne fonctionnait plus. Scan fait maintenant au boot.
  - V10.00 
    OTA par le Web directement en complément de l'Arduino IDE
    Modification des calculs de puissance en UxIx3 pour avoir une représentation similaire au Linky (Merci PhDV61)
    Modification de la surveillance Watchdog
  - V11.00
    Possibilité de définir le SSID et le mot de passe du Wifi par le port série
    Import / Export des paramètres et actions
    Relance découverte MQTT toutes les 5mn
    Re-écriture de la surveillance par watchdog suite au changement de bibliothèque 3.0.x carte ESP32
    Estimation temps equivalent d'ouverture max du Triac et relais cumulée depuis 6h du matin. Prise en compte de la puissance en sin² du mode découpe
    Correction d'un bug de syntaxe non détecté par le compilateur depuis la version V9 affectant les communications d'un ESP esclave vers le maître
    Affichage de l'occupation RAM
  - V11.10
    Nouvelle source de mesure Shelly Pro Em
  - V11.11
    Correction bug mesure de température distante
  - V11.12
    Correction bug mesure données Shelly Em suite évolution bibliothèque ESP32 V3.0.2
  - V11.13
    Correction bug sur export des paramètres
  - V11.14
    Correction bug choix sortie 0V ou 3.3V actif
  - V11.15
    Correction bug sur export des paramètres
  - V11.16
    Modification pour pouvoir faire des imports de paramètres avec Firefox
  - V11.17
    Compilation avec la nouvelle version 3.03 de la carte ESP32
  - V11.18
    Recherche de la couleur Tempo non plus chez EDF mais RTE (sauf pour senseur Linky)
    Améliorations UxIx3
  - V11.19
    Nouvelle adresse de recherche Tempo chez  RTE (sauf pour senseur Linky) plus simple
    Compilation avec la bibliothèque V3.0.4 pour l'ESP32
  - V11.20
    Compilation avec la bibliothèque V3.1.0-RC1 pour l'ESP32
  - V12.00
    Jusqu'à 4 capteurs de température DS18B20 ou extérieurs
    Offset sur les températures si besoin de corriger les mesures
    Rajout d'informations en sortie MQTT
    Les Actions peuvent être conditionnées à l'état d'autres Actions sur le même ESP32 ou un distant
    RAZ des historiques sur demande
  - V12.01
    Correction bug sur les dixièmes de degrés des températures
  - V12.03
    Corrections sur les multiplications et divisions de float par une constante
  - V12.04
    Mise à jour Shelly Em Pro
    Clarification mise en page Actions
  - V12.05
    Correction bugs Duree_Relais dans Mqtt.ino et débordement micros() dans Source_UxI.ino 
  - V12.06
    Compilation avec une partition mémoire  NoFS  suite à comportement anormal du watchdog. 
  - V13.00
    Compilation à faire avec une partition mémoire  NoFS. 
    Conditionnement Actions par d'autres actions différentes pour chaque tranche horaire.
    Introduction Mot de passe/Clé d'accès pour modifier les paramètres ou actions
    MQTT: un prefixe pour la déclaration et un autre pour la publication de l'état
    Si Action inactive arrêt envoi commande Off sur relais distant.
    Création d'une hysteresis sur les température si Tinf<Tsup
    Choix de la connexion, WIFI avec Internet, WIFI sans internet ou pas de WIFI (mode AP)
    Retrait du watchdog. Il ne fonctionne plus, sauf si on retire des lignes de code sur des sujets qui n'ont rien à voir. Problème occupation/débordement mémoire ? Pas clair.
    Choix des couleurs sur les pages Web
    Choix de l'horloge :internet,Linky,Interneou Secteur
    Choix paramétrage en mode standard ou expert.
  - V13.01
    Mystère du watchdog qui fait planter les ESP esclaves après quelques minutes, bien que plus présent. Il faut lui dire de ne pas s'activer avec un esp_task_wdt_deinit(); en début de programme
    RAZ du JSY-MK-194 quand on demande un RAZ dans la page paramètre
    Enrichissement des messages MQTT pour l'option Linky avec les énergies par index.
            
  
  Les détails sont disponibles sur / Details are available here:
  https://f1atb.fr  Section Domotique / Home Automation

  F1ATB Janvier 2025

  GNU Affero General Public License (AGPL) / AGPL-3.0-or-later

*/

//Librairies
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <ArduinoOTA.h>    //Modification On The Air
#include <PubSubClient.h>  //Librairie pour la gestion Mqtt
#include "EEPROM.h"        //Librairie pour le stockage en EEPROM historique quotidien
#include "esp_sntp.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "UrlEncode.h"
#include <HardwareSerial.h>
#include <Update.h>
#include <esp_task_wdt.h>  //Pour deinitialiser le watchdog. Nécessaire pour les gros program en ROM. Mystère non élucidé

//Program routines
#include "pageHtmlBrute.h"
#include "pageHtmlMain.h"
#include "pageHtmlConnect.h"
#include "pageHtmlPara.h"
#include "pageHtmlActions.h"
#include "pageHtmlOTA.h"
#include "pageHtmlExport.h"
#include "pageHtmlHeure.h"
#include "pageHtmlCouleurs.h"
#include "Actions.h"



//PINS - GPIO

#define AnalogIn0 35  //Pour Routeur Uxi
#define AnalogIn1 32
#define AnalogIn2 33  //Note: si GPIO 33 non disponible sur la carte ESP32, utilisez la 34. If GPIO 33 not available on the board replace by GPIO 34
#define RXD2_1 16     //Pour Routeur Linky ou UxIx2 (sur carte ESP32 simple): Couple RXD2=26 et TXD2=27 . Pour carte ESP32 4 relais : Couple RXD2=17 et TXD2=27
#define TXD2_1 17
#define RXD2_2 26  //Pour Routeur Linky ou UxIx2 (sur carte ESP32 simple): Couple RXD2=26 et TXD2=27 . Pour carte ESP32 4 relais : Couple RXD2=17 et TXD2=27
#define TXD2_2 27
#define SER_BUF_SIZE 4096
#define LedYellow 18
#define LedGreen 19
#define pulseTriac_1 4
#define zeroCross_1 5
#define pulseTriac_2 22
#define zeroCross_2 23
#define pinTemp 13  //Capteur température
#define TEMPERATURE_PRECISION 12


//Nombre Actions Max
#define LesActionsLength 10  //Ne pas toucher -Javascript connais pas
//Nombre Routeurs réseau Max
#define LesRouteursMax 8  //Ne pas toucher -Javascript connais pas
//VARIABLES
const char *ap_default_ssid;        // Mode Access point  IP: 192.168.4.1
const char *ap_default_psk = NULL;  // Pas de mot de passe en AP,

//Paramètres pour le stockage en ROM apres les données du RMS
unsigned long Cle_ROM;

String ssid = "";
String password = "";
String CleAcces = "";
String CleAccesRef = "";
String Source = "UxI";
String Source_data = "UxI";
String SerialIn = "";
String hostname = "";
byte dhcpOn = 1;
byte ModePara = 0;  //0 = Minimal, 1= Expert
byte ModeWifi = 0;  //0 = Internet, 1= LAN only, 2 =AP pas de réseau
byte Horloge = 0;   //0=Internet, 1=Linky, 2=Interne, 3=IT 10ms/triac, 4=IT 20ms
unsigned long Gateway = 0;
unsigned long masque = 4294967040;
unsigned long dns = 0;
unsigned long RMSextIP = 0;
unsigned int MQTTRepet = 0;
unsigned long MQTTIP = 0;
unsigned int MQTTPort = 1883;
String MQTTUser = "User";
String MQTTPwd = "password";
String MQTTPrefix = "homeassistant";  // prefix obligatoire pour l'auto-discovery entre HA et Core-Mosquitto (par défaut c'est homeassistant)
String MQTTPrefixEtat = "homeassistant";
String MQTTdeviceName = "routeur_rms";
String TopicP = "PuissanceMaison";
byte subMQTT = 0;
String nomRouteur = "Routeur - RMS";
String nomSondeFixe = "Données seconde sonde";
String nomSondeMobile = "Données Maison";
String Couleurs = "";  // Couleurs pages web
byte WifiSleep = 1;
byte pSerial = 2;              //Choix Pin port serie
byte pTriac = 2;               //Choix Pin Triac
String ES = String((char)27);  //ESC Separator
String FS = String((char)28);  //File Separator
String GS = String((char)29);  //Group Separator
String RS = String((char)30);  //Record Separator
String US = String((char)31);  //Unit Separator
String MessageH[10];
int idxMessage = 0;
int P_cent_EEPROM;
int cptLEDyellow = 0;
int cptLEDgreen = 0;

unsigned int CalibU = 1000;  //Calibration Routeur UxI
unsigned int CalibI = 1000;
int value0;
int volt[100];
int amp[100];
float KV = 0.2083;  //Calibration coefficient for the voltage. Value for CalibU=1000 at startup
float KI = 0.0642;  //Calibration coefficient for the current. Value for CalibI=1000 at startup
float kV = 0.2083;  //Calibration coefficient for the voltage. Corrected value
float kI = 0.0642;  //Calibration coefficient for the current. Corrected value
float voltM[100];   //Voltage Mean value
float ampM[100];

bool EnergieActiveValide = false;
long EAS_T_J0 = 0;
long EAI_T_J0 = 0;
long EAS_M_J0 = 0;  //Debut du jour energie active
long EAI_M_J0 = 0;


int adr_debut_para = 0;  //Adresses Para après le Wifi


//Paramètres électriques
float Tension_T, Intensite_T, PowerFactor_T, Frequence;
float Tension_M, Intensite_M, PowerFactor_M;
long Energie_T_Soutiree = 0;
long Energie_T_Injectee = 0;
long Energie_M_Soutiree = 0;
long Energie_M_Injectee = 0;
long EnergieJour_T_Injectee = 0;
long EnergieJour_M_Injectee = 0;
long EnergieJour_T_Soutiree = 0;
long EnergieJour_M_Soutiree = 0;
int PuissanceS_T, PuissanceS_M, PuissanceI_T, PuissanceI_M;
int PVAS_T, PVAS_M, PVAI_T, PVAI_M;
float PuissanceS_T_inst, PuissanceS_M_inst, PuissanceI_T_inst, PuissanceI_M_inst;
float PVAS_T_inst, PVAS_M_inst, PVAI_T_inst, PVAI_M_inst;
float Puissance_T_moy, Puissance_M_moy;
float PVA_T_moy, PVA_M_moy;
float EASfloat = 0;
float EAIfloat = 0;
int PactConso_M, PactProd;
int16_t tabPw_Maison_5mn[600];  //Puissance Active:Soutiré-Injecté toutes les 5mn
int16_t tabPw_Triac_5mn[600];
int16_t tabTemperature_5mn[4][600];
int16_t tabPw_Maison_2s[300];   //Puissance Active: toutes les 2s
int16_t tabPw_Triac_2s[300];    //Puissance Triac: toutes les 2s
int16_t tabPva_Maison_2s[300];  //Puissance Active: toutes les 2s
int16_t tabPva_Triac_2s[300];
int8_t tabPulseSinusOn[101];
int8_t tabPulseSinusTotal[101];
int8_t tab_histo_ouverture[LesActionsLength][600];
int16_t IdxStock2s = 0;
int16_t IdxStockPW = 0;
float PmaxReseau = 36000;  //Puissance Max pour eviter des débordements
bool LissageLong = false;
bool Pva_valide = false;
int RXD2, TXD2;  //Port serie
int pulseTriac, zeroCross;
bool erreurTriac = false;

//Parameters for JSY-MK-194T module
byte ByteArray[130];
long LesDatas[14];
int Sens_1, Sens_2;
bool RAZ_JSY = false;


//Parameters for JSY-MK-333 module triphasé
String MK333_dataBrute = "";
//  ajout PhDV61 compteur d'énergie quotidienne soutirée et injectée comme calculées par le Linky
float Energie_jour_Soutiree = 0;
float Energie_jour_Injectee = 0;
long Temps_precedent = 0;  // mesure précise du temps entre deux appels au JSY-MK-333

//Parameters for Linky
bool LFon = false;
bool EASTvalid = false;
bool EAITvalid = false;
volatile int IdxDataRawLinky = 0;
volatile int IdxBufDecodLinky = 0;
volatile char DataRawLinky[10000];  //Buffer entrée données Linky
float moyPWS = 0;
float moyPWI = 0;
float moyPVAS = 0;
float moyPVAI = 0;
float COSphiS = 1;
float COSphiI = 1;
long TlastEASTvalide = 0;
long TlastEAITvalide = 0;
String LTARF = "";  //Option tarifaire RTE
String STGE = "";   //Status Tempo uniquement RTE
String NGTF = "";   //Calendrier tarifaire
String JourLinky = "";
long EASF01 = 0;
long EASF02 = 0;
long EASF03 = 0;
long EASF04 = 0;
long EASF05 = 0;
long EASF06 = 0;
long EASF07 = 0;
long EASF08 = 0;
long EASF09 = 0;
long EASF10 = 0;

//Paramètres for Enphase-Envoy-Smetered
String TokenEnphase = "";
String EnphaseUser = "";
String EnphasePwd = "";
String EnphaseSerial = "0";  //Sert égalemnet au Shelly comme numéro de voie
String JsonToken = "";
String Session_id = "";
long LastwhDlvdCum = 0;  //Dernière valeur cumul Wh Soutire-injecté.
float EMI_Wh = 0;        //Energie entrée Maison Injecté Wh
float EMS_Wh = 0;        //Energie entrée Maison Soutirée Wh

//Paramètres for SmartGateways
String SG_dataBrute = "";

//Paramètres for Shelly Em
String ShEm_dataBrute = "";
int ShEm_comptage_appels = 0;
float PwMoy2 = 0;  //Moyenne voie secondsaire
float pfMoy2 = 1;  //pf Voie secondaire

//Paramètres pour puissance via MQTT
String P_MQTT_Brute = "";
float PwMQTT = 0;
float PvaMQTT = 0;
float PfMQTT = 1;

//Paramètres pour RTE
byte TempoRTEon = 0;
int LastHeureRTE = -1;
int LTARFbin = 0;  //Code binaire  des tarifs



//Actions
Action LesActions[LesActionsLength];  //Liste des actions
volatile int NbActions = 0;



//Internal Timers
unsigned long startMillis;
unsigned long previousWifiMillis;
unsigned long previousHistoryMillis;
unsigned long previousWsMillis;
unsigned long previousWiMillis;
unsigned long LastRMS_Millis;
unsigned long previousTimer2sMillis;
unsigned long previousOverProdMillis;
unsigned long previousLEDsMillis;
unsigned long previousActionMillis;
unsigned long previousActionExterneMillis;
unsigned long previousTempMillis;
unsigned long previousLoop;
unsigned long previousETX;
unsigned long PeriodeProgMillis = 1000;
unsigned long T_On_seconde = 0;
float previousLoopMin = 1000;
float previousLoopMax = 0;
float previousLoopMoy = 0;
unsigned long previousTimeRMS;
float previousTimeRMSMin = 1000;
float previousTimeRMSMax = 0;
float previousTimeRMSMoy = 0;
unsigned long previousMQTTenvoiMillis;
unsigned long previousMQTTMillis;
unsigned long LastPwMQTTMillis = 0;
unsigned long PeriodeMQTTMillis = 500;

//Actions et Triac(action 0)
float RetardF[LesActionsLength];  //Floating value of retard
//Variables in RAM for interruptions
volatile unsigned long lastIT = 0;
volatile int16_t IT10ms = 0;        //Interruption avant deglitch
volatile int16_t IT10ms_in = 0;     //Interruption apres deglitch
volatile int16_t ITmode = 0;        //IT exerne Triac ou interne
volatile unsigned short CptIT = 0;  //Compeur IT Triac ou 20ms;
volatile unsigned short StepIT = 1;
hw_timer_t *timer = NULL;
hw_timer_t *timer10ms = NULL;


volatile int Retard[LesActionsLength];
volatile int Actif[LesActionsLength];
volatile int PulseOn[LesActionsLength];
volatile int PulseTotal[LesActionsLength];
volatile int PulseComptage[LesActionsLength];
volatile int Gpio[LesActionsLength];
volatile int OutOn[LesActionsLength];
volatile int OutOff[LesActionsLength];

WebServer server(80);  // Simple Web Server on port 80

//Port Serie 2 - Remplace Serial2 qui bug
HardwareSerial MySerial(2);

// Heure et Date
#define MAX_SIZE_T 80
const char *ntpServer1 = "fr.pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
String DATE = "";
String DateCeJour = "";  //Plus utilisé depuis V13
bool HeureValide = false;
int16_t HeureCouranteDeci = 0;
int16_t idxPromDuJour = 0;
int16_t Int_Heure = 0;  //Heure interne
int16_t Int_Minute = 0;
int16_t Int_Seconde = 0;
unsigned short Int_Last_10Millis = 0;


//Température Capteur DS18B20
OneWire oneWire(pinTemp);
DallasTemperature ds18b20(&oneWire);
float temperature[4];  // 4 canaux max de températurre
int offsetTemp[4];     //erreur *100
int TemperatureValide[4];
byte canalTempExterne[4];
byte refTempIP[4];
int Nbr_DS18B20 = 0;
String Source_Temp[4];
String nomTemperature[4];
String TopicT[4];
String AllTemp = "";


//MQTT
WiFiClient MqttClient;
PubSubClient clientMQTT(MqttClient);
bool Discovered = false;

//WIFI
int WIFIbug = 0;
WiFiClientSecure clientSecu;
WiFiClientSecure clientSecuRTE;
String Liste_AP = "";

// Routeurs du réseau
unsigned long RMS_IP[LesRouteursMax];  //RMS_IP[0] = adresse IP de cet ESP32
String RMS_Nom[LesRouteursMax];
bool RMS_Actif[LesRouteursMax];
int RMS_Noms_idx = 0;
int RMS_Datas_idx = 0;

//Adressage IP coeur0 et coeur1
byte arrIP[4];

//Multicoeur - Processeur 0 - Collecte données RMS local ou distant
TaskHandle_t Task1;
esp_err_t ESP32_ERROR;
bool PuissanceRecue = false;
int PuissanceValide = 5;

//Interruptions, Current Zero Crossing from Triac device and Internal Timer
//*************************************************************************
void IRAM_ATTR onTimer10ms() {  //Interruption interne toutes 10ms
  ITmode = ITmode - 1;
  if (ITmode < -5) ITmode = -5;
  if (ITmode < 0) GestionIT_10ms();  //IT non synchrone avec le secteur . Horloge interne
}


// Interruption du Triac Signal Zc, toutes les 10ms si Triac, toutes les 20ms si systeme redressement secteur
void IRAM_ATTR currentNull() {
  IT10ms = IT10ms + 1;

  if ((millis() - lastIT) > 2) {  // to avoid glitch detection during 2ms
    ITmode = ITmode + 3;
    if (ITmode > 5) ITmode = 5;
    IT10ms_in = IT10ms_in + 1;
    lastIT = millis();
    if (ITmode > 0) GestionIT_10ms();  //IT synchrone avec le secteur signal Zc toutes les 10ms
  }
}


void GestionIT_10ms() {
  CptIT = CptIT + StepIT;
  for (int i = 0; i < NbActions; i++) {
    switch (Actif[i]) {  //valeur en RAM
      case 0:            //Inactif

        break;
      case 1:  //Decoupe Sinus uniquement pour Triac
        if (i == 0) {
          PulseComptage[0] = 0;
          digitalWrite(pulseTriac, LOW);  //Stop Découpe Triac
        }
        break;
      default:              // Multi Sinus ou Train de sinus
        if (Gpio[i] > 0) {  //Gpio valide
          if (PulseComptage[i] < PulseOn[i]) {
            digitalWrite(Gpio[i], OutOn[i]);
          } else {
            digitalWrite(Gpio[i], OutOff[i]);  //Stop
          }
          PulseComptage[i] = PulseComptage[i] + 1;
          if (PulseComptage[i] >= PulseTotal[i]) {
            PulseComptage[i] = 0;
          }
        }
        break;
    }
  }
}

// Interruption Timer interne toutes les 100 micro secondes
void IRAM_ATTR onTimer() {  //Interruption every 100 micro second
  if (Actif[0] == 1) {      // Découpe Sinus
    PulseComptage[0] = PulseComptage[0] + 1;
    if (PulseComptage[0] > Retard[0] && Retard[0] < 98 && ITmode > 0) {  //100 steps in 10 ms
      digitalWrite(pulseTriac, HIGH);                                    //Activate Triac
    } else {
      digitalWrite(pulseTriac, LOW);  //Stop Triac
    }
  }
}

// SETUP
//*******
void setup() {
  startMillis = millis();
  previousLEDsMillis = startMillis;

  //Pin initialisation
  pinMode(LedYellow, OUTPUT);
  pinMode(LedGreen, OUTPUT);
  digitalWrite(LedYellow, LOW);
  digitalWrite(LedGreen, LOW);

  //Ports Série ESP
  Serial.begin(115200);
  Serial.println("Booting");

  esp_task_wdt_deinit();  // ARRET nécessaire du Watchdog bien que non utilisé ! Mystère

  for (int i = 0; i < LesActionsLength; i++) {
    LesActions[i] = Action(i);  //Creation objets
    PulseOn[i] = 0;             //1/2 sinus
    PulseTotal[i] = 100;
    PulseComptage[i] = 0;
    Retard[i] = 100;
    RetardF[i] = 100;
    OutOn[i] = 1;
    OutOff[i] = 0;
    Gpio[i] = -1;
  }


  //Tableau Longueur Pulse et Longueur Trame pour Multi-Sinus de 0 à 100%
  float erreur;
  float vrai;
  float target;
  for (int I = 0; I < 101; I++) {
    tabPulseSinusTotal[I] = -1;
    tabPulseSinusOn[I] = -1;
    target = float(I) / 100.0;
    for (int T = 20; T < 101; T++) {
      for (int N = 0; N <= T; N++) {
        if (T % 2 == 1 || N % 2 == 0) {  // Valeurs impaires du total ou pulses pairs pour éviter courant continu
          vrai = float(N) / float(T);
          erreur = abs(vrai - target);
          if (erreur < 0.004) {
            tabPulseSinusTotal[I] = T;
            tabPulseSinusOn[I] = N;
            N = 101;
            T = 101;
          }
        }
      }
    }
  }
  for (int i = 0; i < LesRouteursMax; i++) {
    RMS_IP[i] = 0;  //IP du reseau
  }
  init_puissance();
  InitTemperature();
  //Liste Wifi à faire avant connexion à un AP. Necessaire depuis biblio ESP32 3.0.1
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Liste_WIFI();

  Serial.print("Version : ");
  Serial.println(Version);
  // Configure WIFI
  // **************
  hostname = String(HOSTNAME);
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  hostname += String(chipId);  //Add chip ID to hostname
  Serial.println(hostname);
  WiFi.hostname(hostname);
  ap_default_ssid = (const char *)hostname.c_str();
  // Check WiFi connection
  // ... check mode
  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    delay(10);
  }
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);

  INIT_EEPROM();


  //Lecture Clé pour identifier si la ROM a déjà été initialisée
  Cle_ROM = CLE_Rom_Init;
  unsigned long Rcle = LectureCle();
  Serial.println("cle : " + String(Rcle));
  if (Rcle == Cle_ROM) {  // Programme déjà executé
    LectureEnROM();
    LectureConsoMatinJour();
    InitGpioActions();
  } else {
    RAZ_Histo_Conso();
  }
  //Triac init
  if (pTriac > 0) {
    pulseTriac = pulseTriac_2;
    zeroCross = zeroCross_2;
    if (pTriac == 1) {
      pulseTriac = pulseTriac_1;
      zeroCross = zeroCross_1;
    }
    pinMode(zeroCross, INPUT_PULLUP);
    pinMode(pulseTriac, OUTPUT);
    digitalWrite(pulseTriac, LOW);  //Stop Triac
  } else {
    Actif[0] = 0;
    LesActions[0].Actif = 0;
  }
  Gpio[0] = pulseTriac;
  LesActions[0].Gpio = pulseTriac;


  if (Horloge == 0) {  //heure par Internet}
    //Heure / Hour . A Mettre en priorité avant WIFI (exemple ESP32 Simple Time)
    //External timer to obtain the Hour and reset Watt Hour every day at 0h
    sntp_set_sync_interval(10800000);  //Synchro toutes les 3h
    sntp_set_time_sync_notification_cb(time_sync_notification);
    //sntp_servermode_dhcp(1);   Déprecié
    esp_sntp_servermode_dhcp(true);                                                        //Option
    configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", ntpServer1, ntpServer2);  //Voir Time-Zone:
  }

  //WIFI
  if (ModeWifi < 2) {
    Serial.println("ssid:" + ssid);
    Serial.println("password:" + password);
    if (ssid.length() > 0) {
      if (dhcpOn == 0) {  //Static IP
        IP2String(RMS_IP[0]);
        // Set youRMS_IP[0]c IP address
        IPAddress local_IP(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);
        // Set your Gateway IP address
        IP2String(Gateway);
        IPAddress gateway(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);
        // Set your masque/subnet IP address
        IP2String(masque);
        IPAddress subnet(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);
        // Set your DNS IP address
        IP2String(dns);
        IPAddress primaryDNS(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);  //optional
        IPAddress secondaryDNS(8, 8, 4, 4);                            //optional
        if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
          Serial.println("WIFI STA Failed to configure");
        }
      }
      StockMessage("Wifi Begin : " + ssid);
      WiFi.begin(ssid.c_str(), password.c_str());
      WiFi.setSleep(WifiSleep);
      while (WiFi.status() != WL_CONNECTED && (millis() - startMillis < 20000)) {  // Attente connexion au Wifi
        Serial.write('.');
        Gestion_LEDs();
        Serial.print(WiFi.status());
        delay(300);
      }
      Serial.println();
    }
  }
  if (WiFi.status() == WL_CONNECTED && ModeWifi < 2) {
    RMS_IP[0] = String2IP(WiFi.localIP().toString());
    StockMessage("Connecté, addresse IP : " + WiFi.localIP().toString() + " or <a href='http://" + hostname + ".local' >" + hostname + ".local</a>");
  } else {
    StockMessage("Pas de connexion WIFI. ESP32 en mode AP et STA.");
    // Go into software AP and STA modes.
    //WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    delay(10);
    WiFi.softAP(ap_default_ssid, ap_default_psk);
    Serial.println("Access Point Mode : " + hostname);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Par le port série vous pouvez définir le WIFI à utiliser par l'ESP32 en tapant les 3 commandes ci-dessous en remplaçant xxx par la bonne valeur :");
    Serial.println("ssid:xxx");
    Serial.println("password:xxx");
    Serial.println("restart");
  }



  Init_Server();
  Liste_des_Noms();

  // Modification du programme par le Wifi  - OTA(On The Air)
  //***************************************************
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();  //Mandatory

  //Adaptation à la Source
  Serial.println("Source : " + Source);

  if (Source == "UxI") {
    Setup_UxI();
  }

  if (Source == "Enphase") {
    Setup_Enphase();
  }


  if (Source == "Pmqtt") {
    GestionMQTT();
  }

  //Port Série si besoin
  if (pSerial > 0) {
    RXD2 = RXD2_2;
    TXD2 = TXD2_2;
    if (pSerial == 1) {
      RXD2 = RXD2_1;
      TXD2 = TXD2_1;
    }
    if (Source == "UxIx2") {
      Setup_UxIx2();
    }

    if (Source == "Linky") {
      Setup_Linky();
    }
  }

  if (Source == "Ext") {
  } else {
    Source_data = Source;
  }




  xTaskCreatePinnedToCore(  //Préparation Tâche Multi Coeur
    Task_LectureRMS,        /* Task function. */
    "Task_LectureRMS",      /* name of task. */
    10000,                  /* Stack size of task */
    NULL,                   /* parameter of the task */
    10,                     /* priority of the task */
    &Task1,                 /* Task handle to keep track of created task */
    0);                     /* pin task to core 0 */


  if (pTriac > 0) {
    //Interruptions du Triac et Timer interne
    attachInterrupt(zeroCross, currentNull, RISING);
  }

  //Hardware timer 100uS
  timer = timerBegin(1000000);  //Clock 1MHz
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 100, true, 0);  //Interrupt every 100  microsecond

  //Hardware timer 10ms
  timer10ms = timerBegin(1000000);  //Clock 1MHz
  timerAttachInterrupt(timer10ms, &onTimer10ms);
  timerAlarm(timer10ms, 10000, true, 0);  //Interrupt every 10ms


  //Timers
  previousWifiMillis = millis() - 25000;
  previousHistoryMillis = millis() - 280000;
  previousTimer2sMillis = millis();
  previousLoop = millis();
  previousTimeRMS = millis();
  previousMQTTenvoiMillis = millis();
  previousMQTTMillis = millis();
  previousETX = millis();
  previousOverProdMillis = millis();
  LastRMS_Millis = millis();
  previousActionMillis = millis();
  previousActionExterneMillis = millis();
  previousTempMillis = millis() - 110000;
  if (Nbr_DS18B20 > 0) LectureTemperature();
}

/* **********************
   * ****************** *
   * * Tâches Coeur 0 * *
   * ****************** *
   **********************
*/
void Task_LectureRMS(void *pvParameters) {

  if (Source == "UxIx3") {
    Setup_JSY333();            // init port série
    delay(100);                // pour s'assurer que l'init du port série est ok coté module
    PeriodeProgMillis = 1000;  // la première lecture aura lieu 1000ms plus tard
    Requete_JSY333();          // requête initiale au module. La première lecture aura lieu PeriodeProgMillis =1000ms plus tard.
                               // et les données seront déjà toutes dans le buffer de réception
  }
  for (;;) {
    unsigned long tps = millis();
    float deltaT = float(tps - previousTimeRMS);
    previousTimeRMS = tps;
    previousTimeRMSMin = min(previousTimeRMSMin, deltaT);
    previousTimeRMSMin = previousTimeRMSMin + 0.002;
    previousTimeRMSMax = max(previousTimeRMSMax, deltaT);
    previousTimeRMSMax = previousTimeRMSMax * 0.999;
    previousTimeRMSMoy = deltaT * 0.01 + previousTimeRMSMoy * 0.99;
    previousTimeRMSMin = min(previousTimeRMSMin, previousTimeRMSMoy);
    previousTimeRMSMax = max(previousTimeRMSMax, previousTimeRMSMoy);


    //Recupération des données RMS
    //******************************
    if (tps - LastRMS_Millis > PeriodeProgMillis) {  //Attention delicat pour eviter pb overflow
      LastRMS_Millis = tps;
      unsigned long ralenti = long(PuissanceS_M / 10);  // On peut ralentir échange sur Wifi si grosse puissance en cours
      if (Source == "UxI") {
        LectureUxI();
        PeriodeProgMillis = 40;
      }
      if (pSerial > 0) {
        if (Source == "UxIx2") {
          LectureUxIx2();
          PeriodeProgMillis = 400;
        }
        if (Source == "UxIx3") {
          Lecture_JSY333();
          PeriodeProgMillis = 1000;
        }
        if (Source == "Linky") {
          LectureLinky();
          PeriodeProgMillis = 2;
        }
      }
      if (Source == "Enphase") {
        LectureEnphase();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 600 + ralenti;  //On s'adapte à la vitesse réponse Envoy-S metered
      }
      if (Source == "SmartG") {
        LectureSmartG();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 200 + ralenti;  //On s'adapte à la vitesse réponse SmartGateways
      }
      if (Source == "ShellyEm") {
        LectureShellyEm();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 200 + ralenti;  //On s'adapte à la vitesse réponse ShellyEm
      }
      if (Source == "ShellyPro") {
        LectureShellyProEm();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 200 + ralenti;  //On s'adapte à la vitesse réponse ShellyProEm
      }

      if (Source == "Ext") {
        CallESP32_Externe();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 200 + ralenti;  //Après pour ne pas surchargé Wifi
      }
      if (Source == "Pmqtt") {
        PeriodeProgMillis = 600;
        LastRMS_Millis = millis();
        UpdatePmqtt();
      }
    }
    delay(2);
  }
}




/* **********************
   * ****************** *
   * * Tâches Coeur 1 * *
   * ****************** *
   **********************
*/
void loop() {
  //Estimation charge coeur
  unsigned long tps = millis();
  float deltaT = float(tps - previousLoop);
  previousLoop = tps;
  previousLoopMin = min(previousLoopMin, deltaT);
  previousLoopMin = previousLoopMin + 0.002;
  previousLoopMax = max(previousLoopMax, deltaT);
  previousLoopMax = previousLoopMax * 0.999;
  previousLoopMoy = deltaT * 0.01 + previousLoopMoy * 0.99;
  previousLoopMin = min(previousLoopMin, previousLoopMoy);
  previousLoopMax = max(previousLoopMax, previousLoopMoy);
  //Gestion des serveurs
  //********************
  ArduinoOTA.handle();
  server.handleClient();

  //Archivage et envois des mesures périodiquement
  //**********************************************
  if (EnergieActiveValide) {

    if (tps - previousHistoryMillis >= 300000) {  //Historique consommation par pas de 5mn
      previousHistoryMillis = tps;
      tabPw_Maison_5mn[IdxStockPW] = PuissanceS_M - PuissanceI_M;
      tabPw_Triac_5mn[IdxStockPW] = PuissanceS_T - PuissanceI_T;
      for (int c = 0; c < 4; c++) {
        if (temperature[c] > -50) {
          tabTemperature_5mn[c][IdxStockPW] = int(temperature[c] * 10.0);
        } else {
          tabTemperature_5mn[c][IdxStockPW] = 0;
        }
      }


      for (int i = 0; i < NbActions; i++) {
        if (Actif[i] > 0) {
          tab_histo_ouverture[i][IdxStockPW] = 100 - Retard[i];
        } else {
          tab_histo_ouverture[i][IdxStockPW] = 0;
        }
      }
      IdxStockPW = (IdxStockPW + 1) % 600;

      //Discovery message pour MQTT (if HA restart)
      Discovered = false;
    }


    if (tps - previousTimer2sMillis >= 2000) {
      unsigned long dt = tps - previousTimer2sMillis;
      previousTimer2sMillis = tps;
      tabPw_Maison_2s[IdxStock2s] = PuissanceS_M - PuissanceI_M;
      tabPw_Triac_2s[IdxStock2s] = PuissanceS_T - PuissanceI_T;
      tabPva_Maison_2s[IdxStock2s] = PVAS_M - PVAI_M;
      tabPva_Triac_2s[IdxStock2s] = PVAS_T - PVAI_T;
      IdxStock2s = (IdxStock2s + 1) % 300;
      JourHeureChange();
      EnergieQuotidienne();
      H_Ouvre_Equivalent(dt);
    }

    if (tps - previousOverProdMillis >= 200) {
      previousOverProdMillis = tps;
      GestionOverproduction();
    }
  }
  if (tps - previousMQTTMillis > PeriodeMQTTMillis) {

    previousMQTTMillis = tps;
    GestionMQTT();
  }
  if (tps - previousLEDsMillis >= 50) {
    previousLEDsMillis = tps;
    Gestion_LEDs();
  }
  //Suivi action externes
  if (tps - previousActionExterneMillis > 21001) {
    previousActionExterneMillis = tps;
    InfoActionExterne();
  }
  //Actions forcées et température
  if (tps - previousActionMillis > 60000) {
    previousActionMillis = tps;
    for (int i = 0; i < NbActions; i++) {
      if (LesActions[i].tOnOff > 0) LesActions[i].tOnOff -= 1;
      if (LesActions[i].tOnOff < 0) LesActions[i].tOnOff += 1;
    }
  }
  if (tps - previousTempMillis > 60001) {
    previousTempMillis = tps;
    //Temperature
    LectureTemperature();
    //Rafraichissement des noms si un a changé
    for (int i = 0; i < LesRouteursMax; i++) {
      RMS_Noms_idx = (RMS_Noms_idx + 1) % LesRouteursMax;
      if (RMS_IP[RMS_Noms_idx] > 0) {
        Liste_Noms(RMS_Noms_idx);
        i = LesRouteursMax;
      }
    }
  }
  //Vérification du WIFI et de la puissance
  //********************
  if (tps - previousWifiMillis > 30000) {  //Test présence WIFI toutes les 30s et autres
    previousWifiMillis = tps;
    JourHeureChange();
    Serial.println("\nDate : " + DATE);
    if (WiFi.getMode() == WIFI_STA) {
      if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
        StockMessage("WIFI Connection Failed! #" + String(WIFIbug));
        WIFIbug++;
      } else {
        WIFIbug = 0;
      }
      Serial.print("Niveau Signal WIFI :");
      Serial.println(WiFi.RSSI());
      Serial.print("IP address_: ");
      Serial.println(WiFi.localIP());
      Serial.print("WIFIbug : #");
      Serial.println(WIFIbug);
      if (WIFIbug > 2880) {  //24h sans WIFI Reset
        delay(5000);
        ESP.restart();
      }

      Call_RTE_data();
      int Ltarf = 0;  //Code binaire Tarif
      if (LTARF.indexOf("PLEINE") >= 0) Ltarf += 1;
      if (LTARF.indexOf("CREUSE") >= 0) Ltarf += 2;
      if (LTARF.indexOf("BLEU") >= 0) Ltarf += 4;
      if (LTARF.indexOf("BLANC") >= 0) Ltarf += 8;
      if (LTARF.indexOf("ROUGE") >= 0) Ltarf += 16;
      LTARFbin = Ltarf;

    } else {
      Serial.println("Access Point Mode : " + hostname);
      Serial.print("IP address: ");
      Serial.println(WiFi.softAPIP());
      Serial.println("Par le port série vous pouvez définir le WIFI à utiliser par l'ESP32 en tapant les 3 commandes ci-dessous en remplaçant xxx par la bonne valeur :");
      Serial.println("ssid:xxx");
      Serial.println("password:xxx");
      Serial.println("restart");
    }
    //Verification puissance reçue
    Serial.print("Puissance reçue : ");
    String OK = "Non";
    if (PuissanceRecue) {
      OK = "Oui";
      PuissanceValide = 5;
    }
    if (PuissanceValide > 0) {
      PuissanceValide = PuissanceValide - 1;
    } else {
      delay(5000);
      ESP.restart();
    }
    Serial.println(OK);
    Serial.println("Charge Lecture RMS (coeur 0) en ms - Min : " + String(int(previousTimeRMSMin)) + " Moy : " + String(int(previousTimeRMSMoy)) + "  Max : " + String(int(previousTimeRMSMax)));
    Serial.println("Charge Boucle générale (coeur 1) en ms - Min : " + String(int(previousLoopMin)) + " Moy : " + String(int(previousLoopMoy)) + "  Max : " + String(int(previousLoopMax)));
    Serial.println("Mémoire RAM libre actuellement: " + String(esp_get_free_internal_heap_size()) + " byte");
    Serial.println("Mémoire RAM libre minimum: " + String(esp_get_minimum_free_heap_size()) + " byte");
    float DureeOn = float(T_On_seconde) / 3600.0;
    Serial.println("ESP32 ON depuis : " + String(DureeOn) + " heures");
    //Test pulse Zc Triac
    if (ITmode < 0 && pTriac > 0) {
      if (!erreurTriac) StockMessage("Erreur : pas de signal Zc du gradateur/Triac");  //Pour ne pas répéter sans cesse
      erreurTriac = true;
    } else {
      erreurTriac = false;
    }
  }

  //Port Série
  LireSerial();
  delay(1);
}

// ************
// *  ACTIONS *
// ************
void GestionOverproduction() {
  float SeuilPw;
  float MaxTriacPw;
  float GainBoucle;
  int Type_En_Cours = 0;
  int LeCanalTemp;
  float laTemperature;
  bool forceOff;
  bool lissage = false;
  //Puissance est la puissance en entrée de maison. >0 si soutire. <0 si injecte
  //Cas du Triac. Action 0
  float Puissance = float(PuissanceS_M - PuissanceI_M);
  if (NbActions == 0) LissageLong = true;  //Cas d'un capteur seul et actions déporté sur autre ESP
  for (int i = 0; i < NbActions; i++) {
    Actif[i] = LesActions[i].Actif;     //0=Inactif,1=Decoupe ou On/Off, 2=Multi, 3= Train
    if (Actif[i] >= 2) lissage = true;  //En RAM
    forceOff = false;
    LeCanalTemp = LesActions[i].CanalTempEnCours(HeureCouranteDeci);
    float laTemperature = -120;
    if (LeCanalTemp >= 0) {
      if (TemperatureValide[LeCanalTemp] > 0) {  //La température de ce canal est valide
        laTemperature = temperature[LeCanalTemp];
      } else {
        forceOff = true;
      }
    }
    if (forceOff) {
      Type_En_Cours = 1;  //  on arrete
    } else {
      Type_En_Cours = LesActions[i].TypeEnCours(HeureCouranteDeci, laTemperature, LTARFbin, Retard[i]);  //0=NO,1=OFF,2=ON,3=PW,4=Triac
    }
    if (Actif[i] > 0 && Type_En_Cours > 1) {  // On ne traite plus le NO
      if (Type_En_Cours == 2) {
        RetardF[i] = 0;
      } else {  // 3 ou 4
        SeuilPw = float(LesActions[i].Valmin(HeureCouranteDeci));
        MaxTriacPw = float(LesActions[i].Valmax(HeureCouranteDeci));
        GainBoucle = float(LesActions[i].Reactivite);                              //Valeur stockée dans Port
        if (Actif[i] == 1 && i > 0) {                                              //Les relais en On/Off
          if (Puissance > MaxTriacPw) { RetardF[i] = 100; }                        //OFF
          if (Puissance < SeuilPw) { RetardF[i] = 0; }                             //On
        } else {                                                                   // le Triac ou les relais en sinus
          RetardF[i] = RetardF[i] + 0.0001;                                        //On ferme très légèrement si pas de message reçu. Sécurité
          RetardF[i] = RetardF[i] + (Puissance - SeuilPw) * GainBoucle / 10000.0;  // Gain de boucle de l'asservissement
          if (RetardF[i] < 100 - MaxTriacPw) { RetardF[i] = 100 - MaxTriacPw; }
          if (ITmode < 0 && i == 0) RetardF[i] = 100;  //Triac pas possible sur synchro interne
        }
        if (RetardF[i] < 0) { RetardF[i] = 0; }
        if (RetardF[i] > 100) { RetardF[i] = 100; }
      }
    } else {
      RetardF[i] = 100;
    }
    Retard[i] = int(RetardF[i]);  //Valeure entiere pour piloter le Triac et les relais
    if (Retard[i] == 100) {       // Force en cas d'arret des IT
      LesActions[i].Arreter();
      PulseOn[i] = 0;  //Stop Triac ou relais
    } else {

      switch (Actif[i]) {  //valeur en RAM du Mode de regulation
        case 1:            //Decoupe Sinus pour Triac ou On/Off pour relais
          if (i > 0) LesActions[i].RelaisOn();
          break;
        case 2:  // Multi Sinus
          PulseOn[i] = tabPulseSinusOn[100 - Retard[i]];
          PulseTotal[i] = tabPulseSinusTotal[100 - Retard[i]];
          break;
        case 3:  // Train de Sinus
          PulseOn[i] = 100 - Retard[i];
          PulseTotal[i] = 99;  //Nombre impair pour éviter courant continu
          break;
      }
    }
  }
  LissageLong = lissage;
}

void InitGpioActions() {
  for (int i = 1; i < NbActions; i++) {
    LesActions[i].InitGpio();
    Gpio[i] = LesActions[i].Gpio;
    OutOn[i] = LesActions[i].OutOn;
    OutOff[i] = LesActions[i].OutOff;
  }
}
// ***********************************
// * Calage Zéro Energie quotidienne * -
// ***********************************

void EnergieQuotidienne() {
  if (HeureValide && Source != "Ext") {
    if (Energie_M_Soutiree < EAS_M_J0 || EAS_M_J0 == 0) {
      EAS_M_J0 = Energie_M_Soutiree;
    }
    EnergieJour_M_Soutiree = Energie_M_Soutiree - EAS_M_J0;
    if (Energie_M_Injectee < EAI_M_J0 || EAI_M_J0 == 0) {
      EAI_M_J0 = Energie_M_Injectee;
    }
    EnergieJour_M_Injectee = Energie_M_Injectee - EAI_M_J0;
    if (Energie_T_Soutiree < EAS_T_J0 || EAS_T_J0 == 0) {
      EAS_T_J0 = Energie_T_Soutiree;
    }
    EnergieJour_T_Soutiree = Energie_T_Soutiree - EAS_T_J0;
    if (Energie_T_Injectee < EAI_T_J0 || EAI_T_J0 == 0) {
      EAI_T_J0 = Energie_T_Injectee;
    }
    EnergieJour_T_Injectee = Energie_T_Injectee - EAI_T_J0;
  }
}

void H_Ouvre_Equivalent(unsigned long dt) {
  float Dheure = float(dt) / 3600000.0;
  for (int i = 0; i < NbActions; i++) {
    if (Actif[i] > 0) {                                       //valeur en RAM du Mode de regulation
      if (i == 0 && Actif[i] == 1) {                          //Decoupe pour Triac
        float teta = 6.28318 * (100.0 - RetardF[i]) / 100.0;  //2*PI integral sin²
        LesActions[i].H_Ouvre += Dheure * (teta - sin(2.0 * teta) / 2.0) / 6.28318;
      } else {
        LesActions[i].H_Ouvre += Dheure * (100 - RetardF[i]) / 100.0;
      }
    }
  }
}

// **************
// * Heure DATE * -
// **************
void time_sync_notification(struct timeval *tv) {
  Serial.println("\nNotification de l'heure ( time synchronization event ) ");
  HeureValide = true;
  Serial.print("Sync time in ms : ");
  Serial.println(sntp_get_sync_interval());
  JourHeureChange();
  StockMessage("Réception de l'heure Internet");
}


//****************
//* Gestion LEDs *
//****************
void Gestion_LEDs() {
  int retard_min = 100;
  int retardI;
  cptLEDyellow++;
  if (WiFi.status() != WL_CONNECTED) {  // Attente connexion au Wifi
    if (WiFi.getMode() == WIFI_STA) {   // en  Station mode
      cptLEDyellow = (cptLEDyellow + 6) % 10;
      cptLEDgreen = cptLEDyellow;
    } else {  //AP Mode
      cptLEDyellow = cptLEDyellow % 10;
      cptLEDgreen = (cptLEDyellow + 5) % 10;
    }
  } else {
    for (int i = 0; i < NbActions; i++) {
      retardI = Retard[i];
      retard_min = min(retard_min, retardI);
    }
    if (retard_min < 100) {
      cptLEDgreen = int((cptLEDgreen + 1 + 8 / (1 + retard_min / 10))) % 10;
    } else {
      cptLEDgreen = 10;
    }
  }
  if (cptLEDyellow > 5) {
    digitalWrite(LedYellow, LOW);
  } else {
    digitalWrite(LedYellow, HIGH);
  }
  if (cptLEDgreen > 5) {
    digitalWrite(LedGreen, LOW);
  } else {
    digitalWrite(LedGreen, HIGH);
  }
}
//*************
//* Test Pmax *
//*************
float PfloatMax(float Pin) {
  float P = max(-PmaxReseau, Pin);
  P = min(PmaxReseau, P);
  return P;
}
int PintMax(int Pin) {
  int M = int(PmaxReseau);
  int P = max(-M, Pin);
  P = min(M, P);
  return P;
}
