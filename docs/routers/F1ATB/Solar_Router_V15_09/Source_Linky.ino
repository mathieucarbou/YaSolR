// ****************************
// * Source de Mesures LINKY  *
// ****************************

//Evolutions pour les Linky avec CACSI proposée par LJ (septembre 2025)

float deltaWS = 0;
float deltaWI = 0;
int boucle_appel_Linky = 0;
void Setup_Linky() {
  delay(20);
  MySerial.setRxBufferSize(SER_BUF_SIZE);
  MySerial.begin(9600, SERIAL_7E1, RXD2, TXD2);  //  7-bit Even parity 1 stop bit pour le Linky
  delay(100);
}

int pIRMS1, pIRMS2, pIRMS3 = -1, pURMS1, pURMS2, pURMS3, pSINSTS = 0, pSINSTS1 = 0, pSINSTS2, pSINSTS3, pPuissance;
void LectureLinky() {  //Lecture port série du LINKY .
  int V = 0;
  long OldWh = 0;
  float deltaWh = 0;
  float Pmax = 0;
  float Pmin = 0;
  unsigned long Tm = 0;
  float deltaT = 0;
  boucle_appel_Linky++;
  if (boucle_appel_Linky > 4000) {
    boucle_appel_Linky = 0;
    MySerial.flush();
    MySerial.write("Ok");
    StockMessage("Attente Linky 4000 boucles = 8s");
  }
  while (MySerial.available() > 0) {
    boucle_appel_Linky = 0;
    V = MySerial.read();
    DataRawLinky[IdxDataRawLinky] = char(V);
    IdxDataRawLinky = (IdxDataRawLinky + 1) % 10000;
    switch (V) {
      case 2:  //STX (Start Text)
        break;
      case 3:  //ETX (End Text)
        previousETX = millis();
        cptLEDyellow = 4;
        LFon = false;
        break;
      case 10:  // Line Feed. Debut Groupe
        LFon = true;
        IdxBufDecodLinky = IdxDataRawLinky;
        break;
      case 13:       // Line Feed. Debut Groupe
        if (LFon) {  //Debut groupe OK
          LFon = false;
          int nb_tab = 0;
          String code = "";
          String val = "";
          int checksum = 0;
          int checkLinky = -1;

          while (IdxBufDecodLinky != IdxDataRawLinky) {
            if (DataRawLinky[IdxBufDecodLinky] == char(9)) {  //Tabulation
              nb_tab++;
            } else {
              if (nb_tab == 0) {
                code += DataRawLinky[IdxBufDecodLinky];
              }
              if (nb_tab == 1) {
                val += DataRawLinky[IdxBufDecodLinky];
              }
              if (nb_tab <= 1) {
                checksum += (int)DataRawLinky[IdxBufDecodLinky];
              }
            }
            IdxBufDecodLinky = (IdxBufDecodLinky + 1) % 10000;
            if (checkLinky == -1 && nb_tab == 2) {
              checkLinky = (int)DataRawLinky[IdxBufDecodLinky];
              checksum += 18;            //2 tabulations
              checksum = checksum & 63;  //0x3F
              checksum = checksum + 32;  //0x20
            }
          }
          if (code.indexOf("EAST") == 0 || code.indexOf("EAIT") == 0 || code == "SINSTS" || code.indexOf("SINSTI") == 0) {
            if (checksum != checkLinky) {
              StockMessage("Erreur checksum code : " + code + " " + String(checksum) + "," + String(checkLinky));
            } else {
              if (code.indexOf("EAST") == 0) {

                OldWh = Energie_M_Soutiree;
                if (OldWh == 0) { OldWh = val.toInt(); }
                Energie_M_Soutiree = val.toInt();
                Tm = millis();
                deltaT = float(Tm - TlastEASTvalide);
                deltaT = deltaT / float(3600000);
                if (Energie_M_Soutiree == OldWh) {  //Pas de resultat en Wh
                  Pmax = 1.3 / deltaT;
                  moyPWS = min(moyPWS, Pmax);
                } else {
                  TlastEASTvalide = Tm;
                  deltaWh = float(Energie_M_Soutiree - OldWh);
                  deltaWS = deltaWh / deltaT;
                  Pmin = (deltaWh - 1) / deltaT;
                  moyPWS = max(moyPWS, Pmin);  //saut à la montée en puissance
                }
                moyPWS = 0.05 * deltaWS + 0.95 * moyPWS;
                EASTvalid = true;
                if (!EAITvalid && Tm > 12000) {  //Cas des CACSI ou EAIT n'est jamais positionné
                  EAITvalid = true;
                }
              }
              if (code.indexOf("EAIT") == 0) {
                OldWh = Energie_M_Injectee;
                if (OldWh == 0) { OldWh = val.toInt(); }
                Energie_M_Injectee = val.toInt();
                Tm = millis();
                deltaT = float(Tm - TlastEAITvalide);
                deltaT = deltaT / float(3600000);
                if (Energie_M_Injectee == OldWh) {  //Pas de resultat en Wh
                  Pmax = 1.3 / deltaT;
                  moyPWI = min(moyPWI, Pmax);
                } else {
                  TlastEAITvalide = Tm;
                  deltaWh = float(Energie_M_Injectee - OldWh);
                  deltaWI = deltaWh / deltaT;
                  Pmin = (deltaWh - 1) / deltaT;
                  moyPWI = max(moyPWI, Pmin);  //saut à la montée en puissance
                }
                moyPWI = 0.05 * deltaWI + 0.95 * moyPWI;
                EAITvalid = true;
              }
              if (EASTvalid && EAITvalid) {
                EnergieActiveValide = true;
              }
              if (code == "SINSTS") {  //Puissance apparente soutirée. Egalité pour ne pas confondre avec SINSTS1 (triphasé)
                pSINSTS = val.toInt();
                PVAS_M = PintMax(val.toInt());
                moyPVAS = 0.05 * float(PVAS_M) + 0.95 * moyPVAS;
                moyPWS = min(moyPWS, moyPVAS);
                if (moyPVAS > 0) {
                  COSphiS = moyPWS / moyPVAS;
                  COSphiS = min(float(1.0), COSphiS);
                  PowerFactor_M = COSphiS;
                }
                PuissanceS_M = PintMax(int(COSphiS * float(PVAS_M)));
                Pva_valide = true;
              }
              if (code.indexOf("SINSTI") == 0) {  //Puissance apparente injectée
                if (ReacCACSI != 100) {           //Estimateur OFF, mode normal sans CACSI
                  PVAI_M = PintMax(val.toInt());
                  moyPVAI = 0.05 * float(PVAI_M) + 0.95 * moyPVAI;
                  moyPWI = min(moyPWI, moyPVAI);
                  if (moyPVAI > 0) {
                    COSphiI = moyPWI / moyPVAI;
                    COSphiI = min(float(1.0), COSphiI);
                    PowerFactor_M = COSphiI;
                  }
                  PuissanceI_M = PintMax(int(COSphiI * float(PVAI_M)));
                  Pva_valide = true;
 
                }
              }
            }
          }
          if (code.indexOf("DATE") == 0) {

            PuissanceRecue = true;  //Reset du Watchdog à chaque trame du Linky reçue
            if (Horloge == 1) {
              JourLinky = val.substring(5, 7) + "/" + val.substring(3, 5) + "/" + val.substring(1, 3);
              Int_HeureLinky = val.substring(7, 9).toInt();
              Int_MinuteLinky = val.substring(9, 11).toInt();
              Int_SecondeLinky = val.substring(11, 13).toInt();
              HeureValide = true;
            }
          }
          
          //LJ START
          if (ReacCACSI == 100) {  //Estimateur ON
            if (code.indexOf("IRMS1") == 0) {
              pIRMS1 = val.toInt();
            }
            if (code.indexOf("IRMS2") == 0) {
              pIRMS2 = val.toInt();
            }
            if (code.indexOf("IRMS3") == 0) {
              pIRMS3 = val.toInt();
            }
            if (code.indexOf("URMS1") == 0) {
              pURMS1 = val.toInt();
            }
            if (code.indexOf("URMS2") == 0) {
              pURMS2 = val.toInt();
            }
            if (code.indexOf("URMS3") == 0) {
              pURMS3 = val.toInt();
            }
            if (code.indexOf("SINSTS1") == 0) {
              pSINSTS1 = val.toInt();
            }
            if (code.indexOf("SINSTS2") == 0) {
              pSINSTS2 = val.toInt();
            }
            if (code.indexOf("SINSTS3") == 0) {
              pSINSTS3 = val.toInt();
            }
            if (code == "SMAXSN") {

              PuissanceI_M = 0;
              if (PuissanceS_M == 0) {                                            // estimation de la puissance d'injection si PuissanceS_M==0
                if (pIRMS3 != -1) {                                               // triphasé
                  pPuissance = 150 + (pSINSTS1 == 0 ? -1 : 1) * pURMS1 * pIRMS1;  // marge de 150W, en mono SINSTS1==0 si PuissanceS_M==0
                  pPuissance += (pSINSTS2 == 0 ? -1 : 1) * pURMS2 * pIRMS2;
                  pPuissance += (pSINSTS3 == 0 ? -1 : 1) * pURMS3 * pIRMS3;
                } else {
                  pPuissance = 150 + (pSINSTS == 0 ? -1 : 1) * pURMS1 * pIRMS1;  // marge de 150W, en mono SINSTS==0 si PuissanceS_M==0
                }
                if (pPuissance < 0) {  // estimation si l'écart est supérieur à 150W

                  PuissanceI_M = -pPuissance;  // "-" car on donne la valeur injectée
                  PVAI_M = PuissanceI_M;       //On egalise Pw et PVA
              
                }
              }
            }
          }
          //LJ STOP
          if (code.indexOf("URMS1") == 0) {
            Tension_M = val.toFloat();  //phase 1 uniquement
          }
          if (code.indexOf("IRMS1") == 0) {
            Intensite_M = val.toFloat();  //Phase 1 uniquement
          }
          if (code.indexOf("STGE") == 0) {
            STGE = val;  //Status
            STGE.trim();
          }
          if (TempoRTEon == 0) {  // On prend tarif sur Linky
            if (code.indexOf("LTARF") == 0) {
              LTARF = val;  //Option Tarifaire
              LTARF.trim();
            }
            if (code.indexOf("STGE") == 0) {
              STGE = val;  //Status
              STGE.trim();
              STGE = STGE.substring(1, 2);  //Tempo lendemain et jour sur 1 octet
            }
          }
          if (code.indexOf("NGTF") == 0) {
            NGTF = val;  //Calendrier Tarifaire
            NGTF.trim();
          }
          if (code.indexOf("EASF01") == 0) EASF01 = val.toInt();
          if (code.indexOf("EASF02") == 0) EASF02 = val.toInt();
          if (code.indexOf("EASF03") == 0) EASF03 = val.toInt();
          if (code.indexOf("EASF04") == 0) EASF04 = val.toInt();
          if (code.indexOf("EASF05") == 0) EASF05 = val.toInt();
          if (code.indexOf("EASF06") == 0) EASF06 = val.toInt();
          if (code.indexOf("EASF07") == 0) EASF07 = val.toInt();
          if (code.indexOf("EASF08") == 0) EASF08 = val.toInt();
          if (code.indexOf("EASF09") == 0) EASF09 = val.toInt();
          if (code.indexOf("EASF10") == 0) EASF10 = val.toInt();
        }
        break;
      default:
        break;
    }
  }
}