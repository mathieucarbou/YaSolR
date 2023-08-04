// ********************
// Gestion des Actions
// ********************
class Action {
private:
  int Idx;  //Index
  void CallExterne(String host,String url, int port);
  int T_LastAction=0;
  int tempoTimer=0;
  
  

public:
  Action();  //Constructeur par defaut
  Action(int aIdx);
  
  void Definir(String ligne);
  String Lire();  
  void Activer(float Pw, int Heure, float Temperature, int Ltarfbin);
  void Arreter();
  void RelaisOn();
  void Prioritaire();
 
 
  byte TypeEnCours(int Heure,float Temperature, int Ltarfbin);
  int Valmin(int Heure);
  int Valmax(int Heure);
  void InitGpio();
  byte Actif;
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
  bool On;
  byte Type[8];
  int Hdeb[8];
  int Hfin[8];
  int Vmin[8];
  int Vmax[8];
  int Tinf[8];
  int Tsup[8];
  byte Tarif[8];
  
};


 extern void StockMessage(String m);


