// ********************
// Gestion des Actions
// ********************
class Action {
private:
  int Idx;  //Index
  void CallExterne(String host, String url, int port);
  int T_LastAction = 0;
  int tempoTimer = 0;
  int16_t Tseuil = 2000;



public:
  Action();  //Constructeur par defaut
  Action(int aIdx);

  void Definir(String ligne);
  String Lire();
  void Activer(float Pw, int Heure, float Temperature, int Ltarfbin);
  void Arreter();
  void RelaisOn();
  void Prioritaire();

  int16_t CanalTempEnCours(int Heure);
  byte TypeEnCours(int Heure, float Temperature, int Ltarfbin, int Retard);
  byte SelActEnCours(int Heure);
  int Valmin(int Heure);
  int Valmax(int Heure);
  void InitGpio(int FreqPWM);
  byte Actif;  //0=Inactif,1=Decoupe ou On/Off, 2=Multi, 3= Train, 4=PWM (sauf Triac)
  int Port;
  int Repet;
  int Tempo;
  String Titre;
  String Host;
  String OrdreOn;
  String OrdreOff;
  int Gpio;
  int OutOn;
  int OutOff;
  int tOnOff;
  byte Reactivite;
  byte NbPeriode;
  byte ExtSelAct;  //Selection Action en cours
  int8_t ExtValide;   //Condition Action externe
  int16_t ExtHequiv;   //Duree heure *100 action externe
  int16_t ExtOuvert;   //Pourcent ouverture

  bool On;
  float H_Ouvre;
  byte Type[8];  //0=NO(pas utilisé),1=OFF,2=ON,3=PW,4=Triac
  int16_t Hdeb[8];
  int16_t Hfin[8];
  int16_t Vmin[8];  //Seuil Pw On ou decoupe
  int16_t Vmax[8];  //Seuil Pw Off ou ouverture max 
  int16_t Tinf[8];  //Temperarure * 10
  int16_t Tsup[8];
  int16_t Hmin[8];  //Heure deci *100 Min pour actif. 0=non utilisé
  int16_t Hmax[8];  //Heure deci *100 Max pour actif
  int16_t CanalTemp[8];
  byte SelAct[8];  //Ref ESP/Action. 255=pas exploité
  byte Ooff[8];    //Ouvre Min Action pour Actif. 0 non utilisé
  byte O_on[8];
  byte Tarif[8];
};


extern void StockMessage(String m);
