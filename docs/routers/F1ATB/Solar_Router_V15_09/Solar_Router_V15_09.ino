#define Version "15.09"
#define HOSTNAME "RMS-ESP32-"
#define CLE_Rom_Init 912567899  //Valeur pour tester si ROM vierge ou pas. Un changement de valeur remet à zéro toutes les données. / Value to test whether blank ROM or not.

/*
  PV Router / Routeur Photovoltaïque 
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
  - Lecture compteur HomeWizard
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
    Choix de l'horloge :internet,Linky,Interne ou Secteur
    Choix paramétrage en mode standard ou expert.
  - V13.01
    Mystère du watchdog qui fait planter les ESP esclaves après quelques minutes, bien que plus présent. Il faut lui dire de ne pas s'activer avec un esp_task_wdt_deinit(); en début de programme
    RAZ du JSY-MK-194 quand on demande un RAZ dans la page paramètre
    Enrichissement des messages MQTT pour l'option Linky avec les énergies par index.
  - V13.02
    Rajout delai de 100ms après RAZ du JSY-MK-194
    Correction Shelly Pro Em
  - V13.03
    Bug corrigé : variable non initialisée en l'abscence de Triac
    Mise en cache du navigateur (5mn) de certaines pages pour accélerer le chargement
  - V14
    Carte ESP32 Wroom avec écran 320*240
    Envoi température CPU en MQTT
    Notes mesurant la qualité des échanges entre ESP32
    Correction bug calcul Energie avec Horloge Linky
  - V14.01
    Correction bug MesurePower UxI
  - V14.02
    Re-introduction du Watchdog avec une table de partition personalisé fichier : partitions.csv
    Correction bug absence lecture état actions
  - V14.03
    Forcer l'affichage normal, non miroir sur l'écran. Selection automatique de l'écran
  - V14.04
    Modif pour Shelly Pro Em ligne  245
    Retrait mode miroir pour les écrans
  - V14.10
    Modif pour Shelly Pro Em de Dash
    Introduction ESP32-ETH01 : Ethernet
  - V4.11
    Prise en compte des chips model D0WDQ6 qui fonctionne en WiFi bien que non V3
  - V14.20 
    Possibilité de remplacer les 2 LEDs par un mini écran SSD1306,SSD1309 ou SH1106
    Augmentation de la taille de l'identifiant ESP32 MQTT
    Source HomeWizard
    Correction Nom serveur si Ethernet
  - V14.21
    Shelly Em Gen3
    Courbe sur 10mn des ouvertures de Triac ou SSR
    Choix d'affichage des courbes de VA
    RAZ pour JSY-MK-333G
  - V14.22
    Distintinction des ESP32U en version "ESP32-D0WD" et WT-ETH01 (Ethernet)
  - V14.23
    MQTT : envoi facteur de puissance sans unité et envoi STGE du Linky
  - V14.24
    Bug affichage ouverture action 2s
    Bug affichage puissance HomeWizard. Modif ValJsonSG().
  - V14.25
    Affichage des autres routeurs en page d'accueil 
  - V15.00
    Retrait température CPU dans les données brutes. Plus défini par Espressif
    Reaction plus dynamique à choisir dans le cas d'un CACSI et légère surproduction 
    Si source de données de puissance externe, le nom du routeur s'affiche en plus de l'IP 
    Correction décodage Smart Gateway  ValJsonSG  
    Choix durée allumage écran LCD
    Affichage des puissances Max du jour 
    Sortie au format PWM pour les Actions
    Choix du Timeout en cas de coupure de la communication
    Pilotage des Actions par MQTT : tOnOff,Mode,SeuilOn,SeuilOff,OuvreMax,Periode (Topic=DeviceName/Nom_Action)
    Favicon
  - V15.01
    Nettoyage code html, javascript,css (Merci Michy)
    Connexion Wifi :extension du timeout et 2 tentatives avant de déclarer une erreur (Merci Lolo69)
  - V15.02
    Modifications proposée par Lolo69 sur les connexions WIFI avec le Shelly
    Rajout du nom du routeur dans le titre des pages HTML
  - V15.03
    Arrêt par stop() de toutes les connexions WIFI comme proposé par Lolo69
  - V15.04
    Correction conflit Wifi/OLed
    Fin message Shelly non plus sur Timeout mais chaine de caractères. A vérifier avec tous les modèles de Shelly. Codage d'après ChatGPT
  - V15.05
    Si plusieurs  AP même SSID, choix du niveau le plus élevé
    Telnet port 23 identique au port série USB si liaison ethernet/wifi
  - V15.06
    Correction affichage IP
  - V15.07
    Modif lecture Shelly Em
  - V15.08
    Possibilité de trouver le réseau WIFI par WPS. Modif proposée par SR19
    Amélioretion renouvellement token Enphase proposée par SR19
    Création d'une source de puissance non définie pour la première mise en route
  - V15.09
    Estimateur injection proposé par Ludovic35

  
  Les détails sont disponibles sur / Details are available here:
  https://f1atb.fr  Section Domotique / Home Automation

  
  F1ATB Septembre 2025

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
#include <EthernetESP32.h>
#include <esp_wps.h>  //Librairie WPS pour appairage automatique connexion WiFi //SR19

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

//Watchdog de 180 secondes. Le systeme se Reset si pas de dialoque avec le LINKY ou JSY-MK-194T/333 ou Enphase-Envoy pendant 180s
//Watchdog for 180 seconds. The system resets if no dialogue with the Linky or  JSY-MK-194T/333 or Enphase-Envoy for 180s
#define WDT_TIMEOUT 180


#define SER_BUF_SIZE 4096
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
String Source = "NotDef";
String Source_data = "NotDef";
String SerialIn = "";
String hostname = "";
byte dhcpOn = 1;
byte ModePara = 0;                                                     //0 = Minimal, 1= Expert
byte ModeReseau = 0;                                                   //0 = Internet, 1= LAN only, 2 =AP pas de réseau
byte Horloge = 0;                                                      //0=Internet, 1=Linky, 2=Interne, 3=IT 10ms/triac, 4=IT 20ms
byte ESP32_Type = 0;                                                   //0=Inconnu,1=Wroom seul,2=Wroom 1 relais,3=Wroom 4 relais,4=Wroom+Ecran320*240,10=ESP32-ETH01
byte LEDgroupe = 0;                                                    //0:pas de LED,1à9 pour les LED. 10 et 11 pour les écrans  OLED
byte LEDyellow[] = { 0, 18, 4, 2, 0, 0, 0, 0, 0, 0, 18, 4, 18, 4 };    //Ou SDA pour OLED
byte LEDgreen[] = { 0, 19, 16, 4, 0, 0, 0, 0, 0, 0, 19, 32, 19, 32 };  //ou SCL pour OLED
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



//Paramètres écran
byte rotation = 3;
uint16_t Calibre[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned long DurEcran = 30000;


//Paramètres électriques
bool EnergieActiveValide = false;
long EAS_T_J0 = 0;
long EAI_T_J0 = 0;
long EAS_M_J0 = 0;  //Debut du jour energie active
long EAI_M_J0 = 0;
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
int PuisMaxS_T = 0, PuisMaxS_M = 0, PuisMaxI_T = 0, PuisMaxI_M = 0;
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
int8_t tab_histo_2s_ouverture[LesActionsLength][300];
int16_t IdxStock2s = 0;
int16_t IdxStockPW = 0;
float PmaxReseau = 36000;  //Puissance Max pour eviter des débordements
bool LissageLong = false;
bool Pva_valide = false;


//Triac
bool erreurTriac = false;
byte pTriac = 0;  //Choix Pin Triac
int8_t pulseTriac = 0, zeroCross = -1;
int8_t PulseT[] = { 0, 4, 22, 21, 12 };
int8_t ZeroT[] = { -1, 5, 23, 22, 14 };

//Parameters for UxI
byte AnalogIn0 = 35;
byte AnalogIn1 = 32;
byte AnalogIn2 = 33;
unsigned int CalibU = 1000;  //Calibration Routeur UxI
unsigned int CalibI = 1000;
byte pUxI = 0;
byte Analog0[] = { 0, 35, 35, 34, 35 };
byte Analog1[] = { 0, 32, 32, 32, 36 };
byte Analog2[] = { 0, 33, 34, 33, 39 };
int value0;
int volt[100];
int amp[100];
float KV = 0.2083;  //Calibration coefficient for the voltage. Value for CalibU=1000 at startup
float KI = 0.0642;  //Calibration coefficient for the current. Value for CalibI=1000 at startup
float kV = 0.2083;  //Calibration coefficient for the voltage. Corrected value
float kI = 0.0642;  //Calibration coefficient for the current. Corrected value
float voltM[100];   //Voltage Mean value
float ampM[100];

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
String STGE = "";   //Status Linky
String STGEt = "";  //Status Tempo uniquement RTE
String NGTF = "";   //Calendrier tarifaire
String JourLinky = "";
int16_t Int_HeureLinky = 0;  //Heure interne
int16_t Int_MinuteLinky = 0;
int16_t Int_SecondeLinky = 0;
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
long LastwhDlvdCum = 0;             //Dernière valeur cumul Wh Soutire-injecté.
float EMI_Wh = 0;                   //Energie entrée Maison Injecté Wh
float EMS_Wh = 0;                   //Energie entrée Maison Soutirée Wh
unsigned long lastTokenUpdate = 0;  //interval de temps depuis dernier Token Enphase //SR19

//Paramètres for SmartGateways
String SG_dataBrute = "";

//Paramètres for HomeWizard
String HW_dataBrute = "";

//Paramètres for Shelly Em
String ShEm_dataBrute = "";
int ShEm_comptage_appels = 0;
float PwMoy2 = 0;  //Moyenne voie secondsaire
float pfMoy2 = 1;  //pf Voie secondaire
String Shelly_Name = "";
String Shelly_Profile = "";

//Paramètres pour puissance via MQTT
String P_MQTT_Brute = "";
float PwMQTT = 0;
float PvaMQTT = 0;
float PfMQTT = 1;

//Paramètres pour RTE
byte TempoRTEon = 0;
int LastHeureRTE = -1;
int LTARFbin = 0;  //Code binaire  des tarifs

//Paramètres pour Source Externe
int8_t RMSextIdx = 0;

//Actions
Action LesActions[LesActionsLength];  //Liste des actions
volatile int NbActions = 0;
byte ReacCACSI = 1;
unsigned int Fpwm = 500;  // Frequence signaux PWM en Hz



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



//Port Serie 2 - Remplace Serial2 qui bug
HardwareSerial MySerial(2);
byte pSerial = 0;             //Choix Pin port serie
int8_t RXD2 = -1, TXD2 = -1;  //Port serie
int8_t RX2_[] = { -1, 16, 26, 18, 5 };
int8_t TX2_[] = { -1, 17, 27, 19, 17 };

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
byte pTemp = 0;
byte pinTemp[] = { 0, 13, 27, 33 };
OneWire oneWire(17);  //Numero de pin bidon pour le constructor en attendant affectation reel à placer au debut du setup
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
int16_t WIFIbug = 0;
int16_t ComSurv = 6;  //Timeout sans Wifi par pas de 30s
WiFiClientSecure clientSecu;
WiFiClientSecure clientSecuRTE;
String Liste_AP = "";
uint8_t bestBSSID[6];  //Meilleur en dBm adresse MAC

//Ethernet
int16_t EthernetBug = 0;
EMACDriver driver(ETH_PHY_LAN8720, 23, 18, 16);

WebServer server(80);  // Simple Web Server on port 80

// === Serveur Telnet ===
WiFiServer telnetServer(23);  //Port Telnet 23
WiFiClient telnetClient;
bool dispPw = false;   //Affiche  Power sur serial et Telnet
bool dispAct = false;  //Affiche  Ouverture Actions sur serial et Telnet

// Routeurs du réseau
unsigned long RMS_IP[LesRouteursMax];  //RMS_IP[0] = adresse IP de cet ESP32
String RMS_NomEtat[LesRouteursMax];
int8_t RMS_Note[LesRouteursMax];
int8_t RMS_NbCx[LesRouteursMax];
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
      case 4:  //PWM ne depend pas IT 10ms

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

/*** WPS Configurations ***/                                          //SR19
#define ESP_WPS_MODE WPS_TYPE_PBC                                     //SR19
esp_wps_config_t wps_config = WPS_CONFIG_INIT_DEFAULT(ESP_WPS_MODE);  //SR19
bool isGOT_IP = false;                                                //true si IP reçue                                                            //SR19

void wpsStop() {                                                                                //SR19
  esp_err_t err = esp_wifi_wps_disable();                                                       //SR19
  if (err != ESP_OK) {                                                                          //SR19
    TelnetPrintln("WPS Disable Failed: " + String(err, HEX) + "h -> " + esp_err_to_name(err));  //SR19
  }                                                                                             //SR19
}  //SR19

//Evènements WPS/WiFi                                                                                //SR19
void WiFiEvent(WiFiEvent_t event) {                                                            //SR19
  switch (event) {                                                                             //SR19
    case ARDUINO_EVENT_WIFI_STA_START:                                                         //SR19
      TelnetPrintln("WiFi Démarré en Mode Station. Attente WPS Client...");                    //SR19
      break;                                                                                   //SR19
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:                                                        //SR19
      TelnetPrintln("WiFi : " + String(WiFi.SSID()) + " connecté via WPS!");                   //SR19
      ssid = (WiFi.SSID());                                                                    //Récup ssid                                                             //SR19
      TelnetPrintln("Récupération IP de " + hostname + " -> " + (WiFi.localIP().toString()));  //SR19
      TelnetPrintln("Récupération password -> " + String(WiFi.psk()));                         //SR19
      password = (WiFi.psk());                                                                 //Récup password                                                      //SR19
      isGOT_IP = true; /*IP reçue*/                                                            //SR19
      break;                                                                                   //SR19
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:                                                  //SR19
      TelnetPrintln("Déconnecté. Redémarrage WiFi...");                                        //SR19
      WiFi.disconnect();                                                                       //Forçage reconnexion normale                                               //SR19
      delay(10);                                                                               //SR19
      WiFi.reconnect();                                                                        //SR19
      delay(100);                                                                              //SR19
      isGOT_IP = false;                                                                        //Réinitialiser l'événement si déconnecté                                    //SR19
      break;                                                                                   //SR19
    case ARDUINO_EVENT_WPS_ER_SUCCESS:                                                         //SR19
      TelnetPrintln("WPS réussi! Stop WPS et connexion vers: " + String(WiFi.SSID()));         //SR19
      wpsStop();                                                                               //Must disable WPS before connecting                                                //SR19
      WiFi.begin();                                                                            //Connect using credentials from WPS                                             //SR19
      break;                                                                                   //SR19
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:                                                     //SR19
      TelnetPrintln("WiFi Reconnecté en Mode Station");                                        //SR19
      break;                                                                                   //SR19
    default:                                                                                   //SR19
      break;                                                                                   //SR19
  }                                                                                            //SR19
}

// SETUP
//*******
void setup() {
  startMillis = millis();
  previousLEDsMillis = startMillis;


  //Ports Série ESP
  Serial.begin(115200);


  TelnetPrintln("Booting");

  //Watchdog initialisation
  esp_task_wdt_deinit();
  // Initialisation de la structure de configuration pour la WDT
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT * 1000,                 // Convertir le temps en millisecondes
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,  // Bitmask of all cores, https://github.com/espressif/esp-idf/blob/v5.2.2/examples/system/task_watchdog/main/task_watchdog_example_main.c
    .trigger_panic = true                             // Enable panic to restart ESP32
  };
  // Initialisation de la WDT avec la structure de configuration
  ESP32_ERROR = esp_task_wdt_init(&wdt_config);
  TelnetPrintln("Dernier Reset : " + String(esp_err_to_name(ESP32_ERROR)));
  esp_task_wdt_add(NULL);  //add current thread to WDT watch
  esp_task_wdt_reset();
  delay(1);  //VERY VERY IMPORTANT for Watchdog Reset

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
    RMS_Note[i] = 0;
    RMS_NbCx[i] = 0;
  }
  init_puissance();
  InitTemperature();



  INIT_EEPROM();


  //Lecture Clé pour identifier si la ROM a déjà été initialisée
  Cle_ROM = CLE_Rom_Init;
  unsigned long Rcle = LectureCle();
  TelnetPrintln("cle : " + String(Rcle));
  if (Rcle == Cle_ROM) {  // Programme déjà executé
    LectureEnROM();
    LectureConsoMatinJour();
  } else {
    RAZ_Histo_Conso();
  }
  TelnetPrintln("Chip Model: " + String(ESP.getChipModel()));
  delay(100);
  MessageCommandes();
  LireSerial();
  Ethernet.init(driver);
  if (String(ESP.getChipModel()) == "ESP32-D0WD") {  //certains ESP32U et WT32-ETH01
    TelnetPrintln("\nAncien modèle d'ESP32 que l'on trouve sur les cartes Ethernet WT32-ETH01 (branchez le câble) et certains ESP32U");
    if (Ethernet.begin() != 0) {  //C'est une carte WT-ETH01
      TelnetPrintln("Carte WT32-ETH01 qui Crash en Wifi. On force Ethernet.\n");
      ESP32_Type = 10;  //On force Ethernet
    }
  }
  TelnetPrintln("InitGPIO");
  delay(500);
  LireSerial();
  InitGPIOs();
  TelnetPrintln("ESP32_Type:" + String(ESP32_Type));
  delay(500);
  if (ESP32_Type == 4) Ecran_Init();

  IP2String(RMS_IP[0]);
  // Set youRMS_IP[0]c IP address
  IPAddress local_IP(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);
  TelnetPrint("Adresse IP en mémoire : ");
  TelnetPrintln(local_IP.toString());
  // Set your Gateway IP address
  IP2String(Gateway);
  IPAddress gateway(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);
  // Set your masque/subnet IP address
  IP2String(masque);
  IPAddress subnet(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);
  // Set your DNS IP address
  IP2String(dns);
  IPAddress primaryDNS(arrIP[3], arrIP[2], arrIP[1], arrIP[0]);  //optional
  IPAddress secondaryDNS(8, 8, 4, 4);
  hostname = String(HOSTNAME);
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  hostname += String(chipId);  //Add chip ID to hostname
  TelnetPrintln(hostname);     //optional
  bool bestWifi = false;
  if (ESP32_Type == 10) {  //Ethernet (avant Horloge)
    PrintScroll("Lancement de la liaison Ethernet");
    if (Ethernet.linkStatus() == LinkOFF) {
      PrintScroll("Câble Ethernet non connecté.");
    }
    //Ethernet.hostname(hostname);
    if (dhcpOn == 0) {  //Static IP
                        //optional
                        //Adresse IP eventuelles
                        //optional
      Ethernet.begin(local_IP, primaryDNS, gateway, subnet);
      delay(100);
      Ethernet.begin(local_IP, primaryDNS, gateway, subnet);  //On s'y prend 2 fois. Parfois ne reussi pas au premier coup
      delay(100);
      StockMessage("Adresse IP Ethernet fixe : : " + Ethernet.localIP().toString());
      RMS_IP[0] = String2IP(Ethernet.localIP().toString());
    } else {
      TelnetPrintln("Initialisation Ethernet par DHCP:");
      if (Ethernet.begin()) {
        StockMessage("Adresse IP Ethernet assignée par DHCP : " + Ethernet.localIP().toString());
        RMS_IP[0] = String2IP(Ethernet.localIP().toString());
      } else {
        TelnetPrintln("Failed to configure Ethernet using DHCP");
        delay(1);
      }
    }

  } else {  //ESP32 en WIFI
    TelnetPrintln("Lancement du Wifi");
    //Liste Wifi à faire avant connexion à un AP. Necessaire depuis biblio ESP32 3.0.1
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    bestWifi = Liste_WIFI();
    TelnetPrint("Version : ");
    TelnetPrintln(Version);
    LireSerial();
    // Configure WIFI
    // **************

    WiFi.hostname(hostname);
    ap_default_ssid = (const char *)hostname.c_str();
    // Check WiFi connection
    // ... check mode
    if (WiFi.getMode() != WIFI_STA) {
      WiFi.mode(WIFI_STA);
      delay(10);
    }
  }

  LireSerial();
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
  if (ESP32_Type < 10) {
    if (ModeReseau < 2) {
      TelnetPrintln("ssid:" + ssid);
      TelnetPrintln("password:" + password);
      if (ssid.length() > 0) {
        if (dhcpOn == 0) {  //Static IP
                            //Adresse IP eventuelles
                            //optional
          if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
            TelnetPrintln("WIFI STA Failed to configure");
          }
        }
        StockMessage("Wifi Begin : " + ssid);
        if (bestWifi) {
          WiFi.begin(ssid.c_str(), password.c_str(), 0, bestBSSID);  // Connexion forcée au BSSID choisi
        } else {
          WiFi.begin(ssid.c_str(), password.c_str());
        }

        WiFi.setSleep(WifiSleep);
        while (WiFi.status() != WL_CONNECTED && (millis() - startMillis < 20000)) {  // Attente connexion au Wifi
          TelnetPrint(".");
          Gestion_LEDs();
          TelnetPrint(String(WiFi.status()));
          LireSerial();
          delay(300);
        }
        TelnetPrintln("");
      }
    }
    if (WiFi.status() == WL_CONNECTED && ModeReseau < 2) {
      RMS_IP[0] = String2IP(WiFi.localIP().toString());
      StockMessage("Connecté par WiFi, addresse IP : " + WiFi.localIP().toString() + " or <a href='http://" + hostname + ".local' >" + hostname + ".local</a>");
    } else {
      /*** WPS SETUP ***/                                               //SR19
      if (WiFi.scanNetworks() != 0 && WiFi.RSSI(0) > -83) {             //WPS inutile si aucun signal WiFi > -83dBm                                                                        //SR19
        WiFi.disconnect(false, true);                                   //RAZ config.                                                                                                                             //SR19
        delay(100);                                                     //SR19
        WiFi.mode(WIFI_AP_STA);                                         //SR19
        delay(10);                                                      //SR19
        TelnetPrintln("Tentative de connexion via WPS...");             //SR19
        WiFi.onEvent(WiFiEvent);                                        //appel évènements WPS/WiFi depuis WiFiEvent(WiFiEvent_t event)                                                                               //SR19
        delay(10);                                                      //SR19
        esp_wifi_wps_enable(&wps_config);                               //SR19
        esp_wifi_wps_start(0);                                          //SR19
        startMillis = millis();                                         //SR19
        while (isGOT_IP != true && (millis() - startMillis < 20000)) {  //Attente évènement "GOT_IP" pour récupération: ssid, password et IP                                    //SR19
          Gestion_LEDs();                                               //SR19
          delay(300);                                                   //SR19
        }                                                               //SR19
        if (WiFi.status() == WL_CONNECTED && ModeReseau < 2) {          //SR19
          RMS_IP[0] = String2IP(WiFi.localIP().toString());             //SR19
          // Go into software AP and STA modes.                                                                                                                                //SR19
          StockMessage("Connecté par WiFi via WPS, IP : " + WiFi.localIP().toString() + " nom d'hôte : <a href='http://" + hostname + ".local'>" + hostname + ".local</a>"                                                                      //SR19
                                                                                                                                                              " Copiez/collez le nom d'hôte dans votre navigateur. ESP32 en mode AP et STA.");  //SR19
          LireSerial();                                                                                                                                                                                                                         //SR19
          WiFi.softAP(ap_default_ssid, ap_default_psk);                                                                                                                                                                                         //on entre en mode AP pour enregistrer et lancer le RMS                                                                //SR19
        } else {                                                                                                                                                                                                                                //SR19
          StockMessage("Echec de connexion WiFi via WPS. ESP32 en mode AP et STA.");                                                                                                                                                            //SR19
          // Go into software AP and STA modes.                                                                                                                                //SR19
          LireSerial();                                  //SR19
          WiFi.softAP(ap_default_ssid, ap_default_psk);  //SR19
          infoSerie();                                   //SR19
        }                                                //SR19
      } else {
        StockMessage("Pas de connexion WIFI. ESP32 en mode AP et STA.");
        // Go into software AP and STA modes.
        //WiFi.disconnect();
        delay(100);
        WiFi.mode(WIFI_AP_STA);
        delay(10);
        LireSerial();
        WiFi.softAP(ap_default_ssid, ap_default_psk);
        infoSerie();
      }
      WiFi.scanDelete();  //SR19
    }
  }

  // Lancer serveur Telnet
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  TelnetPrintln("Serveur Telnet actif sur le port 23");

  LireSerial();
  Init_Server();
  Liste_des_Noms();

  // Modification du programme par le Wifi  - OTA(On The Air)
  //***************************************************
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();  //Mandatory

  //Adaptation à la Source
  TelnetPrintln("Source : " + Source);

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

    if (Source == "UxIx2") {
      Setup_UxIx2();
    }

    if (Source == "Linky") {
      Setup_Linky();
    }
  }

  if (Source == "Ext") {
    IndexSource();
  } else {
    Source_data = Source;
  }
  LireSerial();



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
  previousTempMillis = millis() - 110000;
  if (Nbr_DS18B20 > 0) LectureTemperature();
  esp_task_wdt_reset();
  delay(1);  //VERY VERY IMPORTANT for Watchdog Reset
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
      if (Source == "NotDef") {
        LectureNotDef();
        PeriodeProgMillis = 600;
      }
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
      if (Source == "HomeW") {
        LectureHomeW();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 200 + ralenti;  //On s'adapte à la vitesse réponse HomeWizard
      }
      if (Source == "ShellyEm") {
        LectureShellyEm();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 100 + ralenti;  //On adapte la vitesse pour ne pas surchargé Wifi.La gestion overproduction est toujours à 200ms
      }
      if (Source == "ShellyPro") {
        LectureShellyProEm();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 100 + ralenti;  //On adapte  la vitesse pour ne pas surchargé Wifi
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
  // Vérifie si un nouveau client Telnet
  if (telnetServer.hasClient()) {
    if (telnetClient && telnetClient.connected()) {
      telnetClient.stop();  // Déconnecter ancien client
    }
    telnetClient = telnetServer.available();
    TelnetPrintln("Nouveau client : " + nomRouteur + " connecté !");
    telnetClient.println("Bienvenue !");
    MessageCommandes();
  }

  //Archivage et envois des mesures périodiquement
  //**********************************************
  if (EnergieActiveValide) {

    if (tps - previousHistoryMillis > 300000) {  //Historique consommation par pas de 5mn
      previousHistoryMillis += 300000;
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
      //Info Serial et Telnet
      MessageCommandes();
    }


    if (tps - previousTimer2sMillis > 2000) {
      unsigned long dt = tps - previousTimer2sMillis;
      previousTimer2sMillis += 2000;  //Pou caler exactement à 2s
      tabPw_Maison_2s[IdxStock2s] = PuissanceS_M - PuissanceI_M;
      tabPw_Triac_2s[IdxStock2s] = PuissanceS_T - PuissanceI_T;
      tabPva_Maison_2s[IdxStock2s] = PVAS_M - PVAI_M;
      tabPva_Triac_2s[IdxStock2s] = PVAS_T - PVAI_T;
      for (int i = 0; i < NbActions; i++) {
        if (Actif[i] > 0) {
          tab_histo_2s_ouverture[i][IdxStock2s] = 100 - Retard[i];
        } else {
          tab_histo_2s_ouverture[i][IdxStock2s] = 0;
        }
      }
      IdxStock2s = (IdxStock2s + 1) % 300;
      PuisMaxS_T = max(PuisMaxS_T, PuissanceS_T);
      PuisMaxS_M = max(PuisMaxS_M, PuissanceS_M);
      PuisMaxI_T = max(PuisMaxI_T, PuissanceI_T);
      PuisMaxI_M = max(PuisMaxI_M, PuissanceI_M);
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

  //Actions forcées et température
  if (tps - previousActionMillis > 60000) {
    previousActionMillis = tps;
    for (int i = 0; i < NbActions; i++) {
      if (LesActions[i].tOnOff > 0) LesActions[i].tOnOff -= 1;
      if (LesActions[i].tOnOff < 0) LesActions[i].tOnOff += 1;
    }
  }
  if (tps - previousTempMillis > 15001) {
    previousTempMillis = tps;
    //Temperature
    LectureTemperature();
    //Rafraichissement des noms et des états des actions externes
    for (int i = 0; i < LesRouteursMax; i++) {
      RMS_Noms_idx = (RMS_Noms_idx + 1) % LesRouteursMax;
      if (RMS_IP[RMS_Noms_idx] > 0) {
        Liste_NomsEtats(RMS_Noms_idx);
        i = LesRouteursMax;
      }
    }
    InfoActionExterne();
  }
  //Vérification Ethernet, WIFI et de la puissance
  //*********************************************
  if (tps - previousWifiMillis > 30000) {  //Test présence WIFI toutes les 30s et autres
    previousWifiMillis = tps;
    JourHeureChange();
    TelnetPrintln("\nDate : " + DATE);
    if (ESP32_Type < 10) {  //ESP32 en WIFI
      if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
        if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
          StockMessage("WIFI Connection Failed! #" + String(WIFIbug));
          WIFIbug++;
          WiFi.begin(ssid.c_str(), password.c_str());  //WIFI auto restart
        } else {
          WIFIbug = 0;
        }

        PrintScroll("Signal WiFi: " + String(WiFi.RSSI()) + "dBm");
        PrintScroll("IP :" + WiFi.localIP().toString());
        if (WIFIbug > 0) PrintScroll("WiFi Bug # :" + String(WIFIbug));
        if (WIFIbug > ComSurv) {  //Timeout sans WIFI =Reset
          TelnetPrintln("Timeout sans WIFI ==> Reset");
          delay(5000);
          ESP.restart();
        }

      } else {
        if (ModeReseau < 2) {  //Normalement connecté au réseau
          WIFIbug++;
          if (WIFIbug > ComSurv) {  // TimeOut en mode AP Reset
            TelnetPrintln("TimeOut en mode AP ==> Reset");
            delay(5000);
            ESP.restart();
          }
        }
        infoSerie();
      }
    } else {  //ESP32 Ethernet
      PrintScroll("IP :" + Ethernet.localIP().toString());
      if (Ethernet.linkStatus() == LinkOFF) {
        PrintScroll("Câble Ethernet non connecté.");
        EthernetBug++;
      } else {
        EthernetBug = 0;
      }
      if (EthernetBug > ComSurv) {  // TimeOut sans réseau
        TelnetPrintln("TimeOut sans réseau Ethernet => Reset ");
        delay(5000);
        ESP.restart();
      }
    }
    //Verification puissance reçue
    String OK = "Non";
    if (PuissanceRecue) {
      OK = "Oui";
      PuissanceValide = 4;
      PuissanceRecue = false;
      esp_task_wdt_reset();
      delay(1);  //VERY VERY IMPORTANT for Watchdog Reset to apply.
    }
    if (PuissanceValide > 0) {
      PuissanceValide = PuissanceValide - 1;
    } else {
      TelnetPrintln("Puissances non reçues => Reset ");
      delay(5000);
      ESP.restart();
    }
    TelnetPrintln("Puissance reçue : " + OK);
    TelnetPrintln("Charge Lecture RMS (coeur 0) en ms - Min : " + String(int(previousTimeRMSMin)) + " Moy : " + String(int(previousTimeRMSMoy)) + "  Max : " + String(int(previousTimeRMSMax)));
    TelnetPrintln("Charge Boucle générale (coeur 1) en ms - Min : " + String(int(previousLoopMin)) + " Moy : " + String(int(previousLoopMoy)) + "  Max : " + String(int(previousLoopMax)));
    TelnetPrintln("Mémoire RAM libre actuellement: " + String(esp_get_free_internal_heap_size()) + " byte");
    TelnetPrintln("Mémoire RAM libre minimum: " + String(esp_get_minimum_free_heap_size()) + " byte");
    float DureeOn = float(T_On_seconde) / 3600.0;
    TelnetPrintln("ESP32 ON depuis : " + String(DureeOn) + " heures");
    //RTE
    if (ModeReseau == 0) {  //Valabe pour Ethernet également
      Call_RTE_data();
      int Ltarf = 0;  //Code binaire Tarif
      if (LTARF.indexOf("PLEINE") >= 0) Ltarf += 1;
      if (LTARF.indexOf("CREUSE") >= 0) Ltarf += 2;
      if (LTARF.indexOf("BLEU") >= 0) Ltarf += 4;
      if (LTARF.indexOf("BLANC") >= 0) Ltarf += 8;
      if (LTARF.indexOf("ROUGE") >= 0) Ltarf += 16;
      LTARFbin = Ltarf;
      if (LTARF != "") PrintScroll(LTARF);
    }
    //Test pulse Zc Triac
    if (ITmode < 0 && pTriac > 0) {
      if (!erreurTriac) StockMessage("Erreur : pas de signal Zc du gradateur/Triac");  //Pour ne pas répéter sans cesse
      erreurTriac = true;
    } else {
      erreurTriac = false;
    }
    if (ESP32_Type == 0) StockMessage("! Carte ESP32 non définie !");
    if (pSerial == 0 && (Source == "UxIx2" || Source == "UxIx3" || Source == "Linky")) StockMessage("! Port série non défini !");
  }


  //Ecran
  if (ESP32_Type == 4) Ecran_Loop();
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
  float GainCACSI = float(ReacCACSI);
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
    Actif[i] = LesActions[i].Actif;                      //0=Inactif,1=Decoupe ou On/Off, 2=Multi, 3= Train , 4=PWM
    if (Actif[i] == 2 || Actif[i] == 3) lissage = true;  //En RAM
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
        GainBoucle = float(LesActions[i].Reactivite);                                                        //Valeur stockée dans Port
        if (Actif[i] == 1 && i > 0) {                                                                        //Les relais en On/Off
          if (Puissance > MaxTriacPw) { RetardF[i] = 100; }                                                  //OFF
          if (Puissance < SeuilPw) { RetardF[i] = 0; }                                                       //On
        } else {                                                                                             // le Triac ou les relais en sinus
          RetardF[i] = RetardF[i] + 0.0001;                                                                  //On ferme très légèrement si pas de message reçu. Sécurité
          if (Puissance < SeuilPw && ReacCACSI > 1 && ReacCACSI < 100) GainBoucle = GainBoucle * GainCACSI;  //On boost si besoin l'écart (*2, 4 ou 8)
          RetardF[i] = RetardF[i] + (Puissance - SeuilPw) * GainBoucle / 10000.0;                            // Gain de boucle de l'asservissement
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

    if (Retard[i] == 100) {  // Force en cas d'arret des IT
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
        case 4:  //PWM
          int Vout = int(RetardF[i] * 2.55);
          if (OutOn[i] == 1) Vout = 255 - Vout;
          ledcWrite(Gpio[i], Vout);
      }
    }
  }
  LissageLong = lissage;
  //Sortie vers port Série ou Telnet
  if (dispPw || dispAct) {
    String dispOut = "";
    if (dispPw) {
      dispOut += "Pw : " + String(PuissanceS_M - PuissanceI_M) + "W";
    }
    if (dispAct) {
      dispOut += " | ";
      for (int i = 0; i < NbActions; i++) {
        if (Actif[i] > 0) dispOut += String(100 - Retard[i]) + "% | ";
      }
    }
    TelnetPrintln(dispOut);
  }
}

void InitGPIOs() {
  if (ESP32_Type > 0) {
    //En premier pour affecter le GPIO au constructeur OneWire
    for (int i = 1; i < NbActions; i++) {
      LesActions[i].InitGpio(Fpwm);
      Gpio[i] = LesActions[i].Gpio;
      OutOn[i] = LesActions[i].OutOn;
      OutOff[i] = LesActions[i].OutOff;
    }
  }
  //Triac init
  if (pTriac > 0) {
    if (ESP32_Type == 2 || ESP32_Type == 3) pTriac = 1;  //Obligatoire carte avec relais)
    if (ESP32_Type == 10) pTriac = 4;                    //Obligatoire carte ETH01)
    pulseTriac = PulseT[pTriac];
    zeroCross = ZeroT[pTriac];
    pinMode(zeroCross, INPUT_PULLUP);
    pinMode(pulseTriac, OUTPUT);
    digitalWrite(pulseTriac, LOW);  //Stop Triac
  } else {
    Actif[0] = 0;
    LesActions[0].Actif = 0;
  }
  Gpio[0] = pulseTriac;
  LesActions[0].Gpio = pulseTriac;

  Init_LED_OLED();

  if (pSerial > 0) {
    if (ESP32_Type == 2) pSerial = 2;  //Obligatoire carte 1 relais
    if (ESP32_Type == 4) pSerial = 3;  //Obligatoire carte écran
    RXD2 = RX2_[pSerial];              //Port serie
    TXD2 = TX2_[pSerial];
  }
  if (pTemp > 0) {
    TelnetPrint("InirTemp:");
    TelnetPrintln(String(pinTemp[pTemp]));
    oneWire.begin(pinTemp[pTemp]);
    ds18b20.begin();
    Nbr_DS18B20 = ds18b20.getDeviceCount();
  }
  //Entree Analogique UxI
  if (Source == "UxI" && pUxI == 0) pUxI = 1;
  AnalogIn0 = Analog0[pUxI];
  AnalogIn1 = Analog1[pUxI];
  AnalogIn2 = Analog2[pUxI];
}

void infoSerie() {
  TelnetPrintln("Access Point Mode : " + hostname);
  TelnetPrint("IP address: ");
  TelnetPrintln(WiFi.softAPIP().toString());
  TelnetPrintln("\nPar le port série vous pouvez définir le WIFI à utiliser par l'ESP32 en tapant les 3 commandes ci-dessous en remplaçant xxx par la bonne valeur :");
  TelnetPrintln("ssid:xxx");
  TelnetPrintln("password:xxx");
  TelnetPrintln("restart");
  TelnetPrintln("\nSi vous utilisez la carte ESP32-ETH01, forcez le mode Ethernet en tapant les 2 commandes ci dessous via le port série :");
  TelnetPrintln("ETH01");
  TelnetPrintln("restart\n");
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
  TelnetPrintln("\nNotification de l'heure ( time synchronization event ) ");
  HeureValide = true;
  TelnetPrint("Sync time in ms : ");
  TelnetPrintln(String(sntp_get_sync_interval()));
  JourHeureChange();
  StockMessage("Réception de l'heure Internet");
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
