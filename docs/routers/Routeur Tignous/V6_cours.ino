//  V6 cours VERSION FRANCAISE SIMPLIFIEE UNIQUEMENT EN COMMANDE DE PHASE. du 27 / 06 /19 

// + CORRECTION du SM à faible PUISSANCE ROUTEE .
// + DECLENCHEMENT d'une 2eme CHARGE en ZERO CROSSING suivant un seuil haut et un seuil bas 
// à partir de l'info "Fd"
//  version fast 1000 joules

 /* Pour dériver du surplus solaire vers un chauffe eau utilisant un relai statique non passage à zero.
  *  
 PRINCIPE =Calculer une cinquantaine de fois par période la puissance instantannée
 au noend de raccordement;moyenner sur le cycle puis en déduire le paquet d'énergie à ajouter ou soustraire
 au "bucket".
  A l'intérieur d'une fourchette de 100 à 1000 joules ,un triac est activé plus ou moins tard
 dérivant vers une charge extérieure la quantité exacte d'énergie de façon à retorter
 ou exporter un minimum.
 Les échantillons doivent être décales (offset)et filtrés pour être numérisés par l'Atmel
 */
 
#include <avr/wdt.h>
#define POSITIVE 1
#define NEGATIVE 0
#define ON 1  // commande positive du relai statique ou triac
#define OFF 0
byte CdeCh1 = 2;// pin4 micro cde SSR1
byte CdeCh2 = 4;// 8 pin 14 micro cde SSR2 (ou 4 pin 6)
byte voltageSensorPin = A3;
byte currentSensorPin = A5; 
float SommeP  = 0;  //somme de P1 sur N periodes
int   SMC =-60; // Safety Margin Compensée
float ret = 0 ; // retard

float imaP = 0;  //image de P pour calcul de SMC
long cycleCount = 0;
long cptperiodes = 0;
int  samplesDuringThisMainsCycle = 0;
byte nextStateOfTriac;
float cyclesPerSecond = 50; // flottant pour plus de précision
int  seuilH; //seuil enclenchemen ch2
int  seuilB; //seuil déclenchemen ch2
int  CE ;   // puissance Chauffe eau 
int  KCE ; // coefficient puissance kCE = 8000 / CE 
byte polarityNow;

boolean flg2 = false ; // pulse cde 2 
boolean triggerNeedsToBeArmed = false;
boolean beyondStartUpPhase = false;
float Plissee = 0;
float energyInBucket = 0;  
float rPWRprec ; //realPower de la période précédente                                              
int capacityOfEnergyBucket = 1000; // AU LIEU DE 3600
int recovPWR = 500 ;// VOIR
int sampleV,sampleI;   // Les valeurs de tension et courant sont des entiers dans l'espace 0 ...1023 
int lastSampleV;     // valeur à la boucle précédente.         
float lastFilteredV,filteredV;  //  tension après filtrage pour retirer la composante continue
float prevDCoffset;          // <<--- pour filtre passe bas
float DCoffset;              // <<--- idem 
float cumVdeltasThisCycle;   // <<--- idem 
float sampleVminusDC;         // <<--- idem
float sampleIminusDC;         // <<--- idem
float lastSampleVminusDC;     // <<--- idem
float sumP;   //  Somme cumulée des puissances à l'intérieur d'un cycle.
// realpower est la puissance moyenne à l'intérieur d'une période

int PHASECAL; //correction de phase inutilisé = 1
float POWERCAL;  // pour conversion des valeur brutes V et I en joules.  
int VOLTAGECAL; // Pour determiner la tension mini de déclenchement du triac.                  // the trigger device can be safely armed

boolean firstLoopOfHalfCycle;
boolean phaseAngleTriggerActivated;
unsigned long Tc; // = To machine à chaque début de 1/2 periode 
unsigned long Fd; // firing delay 


void setup()
{ 
  
   wdt_enable(WDTO_8S);
  
  Serial.begin(500000); // pour tests
  pinMode(CdeCh1, OUTPUT); 
  pinMode(CdeCh2, OUTPUT); 

  
  //++++++++ PARAMETRES A MODIFIER SUIVANT INSTALL++++++
   
  CE = 2400 ;  // Puissance chauffe eau 
  seuilH =500; //seuil enclenchemen ch2 en W approximatifs 
  seuilB =100; //seuil déclenchemen ch2

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
   KCE = 8000 / (CE) ; // coefficient suivant la puissance du CE pour la définition des seuils
   
   POWERCAL = 0.18 ;   // org 0.12 à ajuster pour faire coincider la puissance vraie avec le realPwer.
                    // en utilisant le traceur serie.
                    // NON CRITIQUE car les valeur absolues s'annulent en phase de régulation         // retort and export flows are balanced.  
                    
  VOLTAGECAL = (float)679 / 471; // En volts par pas d'ADC.
                        // Utilisé pour déterminer quand la tension secteur est suffisante pour
                        // exciter le triac. noter les valeurs min et max de la mesure de tension
                        // par exemple 678.8 pic pic 
                        // La dynamique étant de 471 signifie une sous estimation de la tension 
                        // de 471/679.  VOLTAGECAL doit donc être multiplié par l'inverse
                        // de 679/471 soit 1.44
     PHASECAL = 1;      // NON CRITIQUE
     
     sumP = 0 ;
}  


void loop() // Une paire tension / courant est mesurée à chaque boucle (environ 54 par période)

  {
 wdt_reset();
    samplesDuringThisMainsCycle++;  // incrément du nombre d'échantillons par période secteur pour calcul de puissance.

  // Sauvegarde des échantillons précédents
  lastSampleV=sampleV;            // pour le filtre passe haut 
  lastFilteredV = filteredV;      // afin d'identifier le début de chaque période
  lastSampleVminusDC = sampleVminusDC;  // for phasecal calculation 
 
// Acquisition d'une nouvelle paire d'chantillons bruts. temps total :380µS

  sampleI = analogRead(currentSensorPin);   
  sampleV = analogRead(voltageSensorPin); 
 
  // Soustraction de la composante continue déterminée par le filtre passe bas
  sampleVminusDC = sampleV - DCoffset; 
  sampleIminusDC = sampleI - DCoffset;

  // Un filtre passe haut est utilisé pour déterminer le début de cycle.  
  filteredV = 0.996*(lastFilteredV+sampleV-lastSampleV);  // Sinus tension reconstituée 
  // lastFilteredV = zéro en début de cycle

 digitalWrite(CdeCh1, OFF); // 

      // Détection de la polarité de l'alternance
  byte polarityOfLastReading = polarityNow;
  
  if(filteredV >= 0) 
    polarityNow = POSITIVE; 
    else 
    polarityNow = NEGATIVE;

  if (polarityNow == POSITIVE)
    {
    if (polarityOfLastReading != POSITIVE)
    {
  // C'est le départ d'une nouvelle sinus positive juste après le passage à zéro
      
      cycleCount++; // incrément Nb de périodes
      cptperiodes++; // pour affichage toutes les 50 périodes de Power
      firstLoopOfHalfCycle = true;
      
  // mise à jour du filtre passe bas pour la soustraction de la composante continue
      prevDCoffset = DCoffset;
      DCoffset = prevDCoffset + (0.015 * cumVdeltasThisCycle); 

  //  Calcul la puissance réelle de toutes les mesures echantillonnées durant 
  //  le cycle précedent, et determination du gain (ou la perte) en energie.
      float realPower = POWERCAL * sumP / (float)samplesDuringThisMainsCycle;
      float realEnergy = realPower / cyclesPerSecond;//Pmoy X 0.02 en joules sur une période
      
      
      //**********cas de variation brusque et importante ******
      // pour eviter un pic d'injection , on repart à puissance moindre
      float  deltaP = realPower - rPWRprec  ; // puissance période - puissance période précédente
      rPWRprec = realPower ;  // mise à jour 
      if ( deltaP > recovPWR)       // valeur à ajuster
      energyInBucket = 300 ; // routage sur Pmax /3   
 
 
      if (beyondStartUpPhase == true)// > 2 secondes
      {  
   // Supposant que les filtres ont eu suffisamment de temps de se stabiliser    
   // ajout de cette energie d'une période à l'énergie du reservoir.
        energyInBucket += realEnergy;   
     
    // Reduction d'énergie dans le reservoir d'une quantité "safety margin"
    // Ceci permet au système un décalage pour plus d'injection ou + de soutirage 
        energyInBucket -= SMC / cyclesPerSecond;       

    // Limites dans la fourchette 0 ...1000 joules ;ne peut être négatif le 11 / 2 / 18
        if (energyInBucket > capacityOfEnergyBucket)
          energyInBucket = capacityOfEnergyBucket;  
        if (energyInBucket < 0)
          energyInBucket = 0;    
         }
          else
         {  
     // Après un reset attendre 100 périodes (2 secondes) le temps que la composante continue soit éliminée par le filtre        
        if(cycleCount > 100) // deux secondes
          beyondStartUpPhase = true; //croisière
      }
        triggerNeedsToBeArmed = true;   // déclenchement armé à chaque cycle
      
     // ********************************************************
      
     // determination du retard de déclenchement du triac

     // Ne pas allumer si l'énergie du bucket est trop basse (Fd = Firing Delay) 
      if (energyInBucket <= 100) 
          {Fd = 99999; 
          }
          else  
      
      if (energyInBucket >= 1000)
          { Fd = 200;// déclencher immediatement si le niveau d'energie est au dessus du max
        
          }
         
     // determination du bon point de déclenchement pour un niveau donné
     // algorithme simple  
     // Fd est le retard au déclenchement en microsecondes du début de la sinus .
     // pour Pmin = 10000 corrigé à 8500
     // pour Pmax = 0 corrigé à 200 
 
     else
       
        {
          Fd = 10 * (1020 - energyInBucket); 
   
          ret = (Fd);
     if (ret >= 8000)
          ret = 8000; // LIMITE BASSE 8000 soit 50W
          imaP = 8000 - (ret) ;

    SMC = -60 ; //Base de -60   
  
           if ((Fd > 7300)) // <200w compensation pour moindre soutirage à basse puissance 
    SMC = -30 ; // -30 par défaut  

        
     if (Fd > 8500) // pas de déclenchement
        {  Fd = 99999; 
            
          }                    
      }
     //******************************************************             
     // imaP des SEUILS  imaP est une image de la puissance routée (8000-(Fd))/N    environ 1500 W max   
 
     // SommeP tient compte de la puissance du Chauffe eau 
     // Plissee est SommeP moyennée sur N périodes 
 
     (SommeP += (imaP / (KCE)))  ; // Puissance routée par période secteur et incrémentée
     
      if (cptperiodes==250) //Pmoy lissée ;par exemple sur 250 secondes
  
         { Plissee = SommeP / 250 ; //moyenne sur N échantillons }
            if (Plissee >= seuilH) //seuil enclenchemen ch2
                 { flg2 = true ; 
                                     }
             if (Plissee <= seuilB)
                  { flg2 = false ; 
  
                  } // sinon ,on coupe
    
 // Réinitialisation avant un nouveau moyennage 
   
       cptperiodes = 0; 
       SommeP = 0; 
       
 // POUR tests 
 // Serial.print((Plissee), 0);
 // Serial.print ("\t\t");
 // Serial.print((KCE), 0) ;
 // Serial.print ("\t\t");
 // Serial.print ("\t\t");
 // Serial.println((imaP), 0) ;
       
    } 
      sumP = 0;// somme des puissances instantannées
      samplesDuringThisMainsCycle = 0;
      cumVdeltasThisCycle = 0;// somme des tensions instantannées filtrées
    } // Fin du processus spécifique au premier échantillon +ve d'un nouveau cycle secteur
   
 // suite du traitement des échantillons de tension POSITIFS ...
   
    } // Fin du processus sur la demi alternance positive (tension)
   else
    {
    if (polarityOfLastReading != NEGATIVE)
      {
        firstLoopOfHalfCycle = true;
      }
  }
 // Processus pour TOUS les échantillons, positifs et negatifs  54 fois par période 
  
  
   unsigned long To = micros(); // Nb de microsSec depuis le lancement du PG
     
     if (flg2 == true ) 
            { {digitalWrite(CdeCh2, ON) ;}}
        else 
            { {digitalWrite(CdeCh2, OFF) ;}}
    
if (firstLoopOfHalfCycle == true)   
          
 {  Tc = To; // mise à l'heure en début de 1/2 alternance
    firstLoopOfHalfCycle = false;
    phaseAngleTriggerActivated = false;
 // Autre que P max,annuler le déclenchement a la première boucle 
 // de chaque demi cycle pour être sur qu'il ne reste pas bloqué ON.  
    if(Fd > 200)      
    
    { digitalWrite(CdeCh1, OFF);} 
 }
   if (phaseAngleTriggerActivated == true)
  {
 // Sauf demande de puissance max,désarmer le déclenchement a chaque boucle 
 // après la  conduction de la demi période. durée du pulse 20000 / 54 = 370µS
    
    if (Fd > 200) 
               { digitalWrite(CdeCh1, OFF);    }
       }
  else
       {  
           if (To >= (Tc + Fd))
               {  digitalWrite(CdeCh1, ON); // at To + Firing delay
                phaseAngleTriggerActivated = true;  }
    
       }
  // Fin de la gestion de l'allumage du Triac
  //*******************************************************

   
  // Apply phase-shift to the voltage waveform to ensure that the system measures a
  // resistive load with a power factor of unity.
  float  phaseShiftedVminusDC = 
                lastSampleVminusDC + PHASECAL * (sampleVminusDC - lastSampleVminusDC);  
  float instP = phaseShiftedVminusDC * sampleIminusDC; // PUISSANCE derniers échantillons V x I filtrés 
  sumP +=instP;     // accumulation à chaque boucle des puissances instantanées dans une période

  cumVdeltasThisCycle += (sampleV - DCoffset); // pour usage avec filtre passe bas
  
 
} // end of loop()

/*Fd    Puissance sur charge 1500 W
  8,5   20
  8     50
  7,5   100
  7     200
  6.5   300
  6     430
  5,5   550
  5     750
  4,5   926
  4     1015
  3,5   1100
  3     1235
  2,5   1340
  2     1455
  0     1500
   // simple algorithm (with non-linear power response across the energy range)
//        firingDelayInMicros = 10 * (2300 - energyInBucket); 
        
        // complex algorithm which reflects the non-linear nature of phase-angle control.  
        firingDelayInMicros = (asin((-1 * (energyInBucket - 1800) / 500)) + (PI/2)) * (10000/PI);
        
*/
