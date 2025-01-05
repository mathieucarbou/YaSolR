// *******************************************************
// * Recherche Info Tempo RTE pour les sources non Linky *
// *******************************************************


void Call_RTE_data() {
  char buffer[MAX_SIZE_T];
  const char* adr_RTE_Host = "www.services-rte.com";
  String Host = String(adr_RTE_Host);
  String urlJSON = "/cms/open_data/v1/tempoLight";
  String RTEdata = "";
  String line = "";
  String DateRTE = "";                //an-mois-jour
  String DateRTE2 = "";               //an-mois-jour lendemain
  int Hcour = HeureCouranteDeci / 2;  //Par pas de 72secondes pour faire 2 appels si un bug
  int LastH = LastHeureRTE / 2;

// il faut éviter de questionner l'URL tempo light si on a déjà les infos qui nous intéressent...
  bool couleur_lendemain =  (STGE == "4" || STGE == "8" || STGE == "C" );              // true si couleur du lendemain connue
  bool couleur_jour      =  (LTARF == "TEMPO_BLEU" || LTARF == "TEMPO_BLANC" || LTARF == "TEMPO_ROUGE"); // true si couleur du jour connu

 // traitement du changement de couleur tempo à 6h00, avec ré-initialisation de la couleur du lendemain
 if ( (Hcour==300) && (LTARF!="") && (STGE!="") )   // à 6h00 précises, la couleur tempo du lendemain devient la couleur du jour. Celle du lendemain n'est à priori pas encore connue
                                                    // Cela permet de continuer à fonctionner de manière nominale même si le site RTE n'est pas encore renseigné
                                                    // && (LTARF!="") pour ne pas passer dans cette boucle au reset si fait à 6h00 du matin...
                                                    // && (STGE<>"")  pour éviter de passer plusieurs fois( 3 à 4 ) dans cette boucle à cause du Hcour/2
    {  
     if       (STGE == "4") LTARF = "TEMPO_BLEU";  
     else if  (STGE == "8") LTARF = "TEMPO_BLANC"; 
     else if  (STGE == "C") LTARF = "TEMPO_ROUGE";
     STGE=""; 
     couleur_lendemain=false; // on ne connait plus la couleur du lendemain. Cela forcera la lecture sur le site RTE 
     if (LTARF!="") {  
           StockMessage("Tempo depuis 6h00: " + LTARF + ",demain ? ");
      }
    }  


  if ((HeureValide) && (!(couleur_lendemain && couleur_jour ) ) && ((LastH != Hcour) && (Hcour == 302 || Hcour == 310 || Hcour == 530 || Hcour == 560 || Hcour == 600 || Hcour == 900 || Hcour == 1150) || LastHeureRTE < 0)) {
    if (TempoRTEon == 1 && ModeWifi==0) {
      // Use clientSecu class to create TCP connections
      clientSecuRTE.setInsecure();  //skip verification
      if (!clientSecuRTE.connect(adr_RTE_Host, 443)) {
        StockMessage("Connection failed to RTE server :" + Host);
      } else {
        time_t timestamp = time(NULL) - 21600;  //Decallage début période couleur  RTE de 6h.
        struct tm* pTime = localtime(&timestamp);
        strftime(buffer, MAX_SIZE_T, "%Y-%m-%d", pTime);
        DateRTE = String(buffer);
        time_t timestamp2 = time(NULL) + 64800;  //Decallage début période couleur  RTE de 18h pour le lendemainh.
        struct tm* pTime2 = localtime(&timestamp2);
        strftime(buffer, MAX_SIZE_T, "%Y-%m-%d", pTime2);
        DateRTE2 = String(buffer);
        Serial.print("DateRTE:");
        Serial.println(DateRTE);
        Serial.print("DateRTE lendemain:");
        Serial.println(DateRTE2);
        Serial.println(urlJSON);
        clientSecuRTE.print(String("GET ") + urlJSON + " HTTP/1.1\r\n" + "Host: " + Host + "\r\n" + "Connection: close\r\n\r\n");
        Serial.println("Request vers RTE Envoyé");
        unsigned long timeout = millis();
        while (clientSecuRTE.available() == 0) {
          if (millis() - timeout > 5000) {
            StockMessage(">>> clientSecuRTE RTE Timeout !");
            clientSecuRTE.stop();
            return;
          }
        }
        timeout = millis();
        // Lecture des données brutes distantes
        int fin = 0;
        while (clientSecuRTE.connected() && (millis() - timeout < 5000) && fin < 2) {
          line = clientSecuRTE.readStringUntil('\n');
          RTEdata += line;
          if (line.indexOf("}}") >= 0) fin = 2;
        }
        clientSecuRTE.stop();
        Serial.print("RTEdata:");
        Serial.println(RTEdata);
        // C'est RTE qui donne la couleur
        int p = RTEdata.indexOf("\"" + DateRTE + "\"");
        int q = RTEdata.indexOf("\"" + DateRTE2 + "\"");
        if (p > 0 || q > 0) {
          String LTARFrecu = StringJson(DateRTE, RTEdata);  //Remplace code du Linky
          if (LTARFrecu == "BLUE") LTARF = "TEMPO_BLEU";
          if (LTARFrecu == "WHITE") LTARF = "TEMPO_BLANC";
          if (LTARFrecu == "RED") LTARF = "TEMPO_ROUGE";
          line = "0";
          String lendemain = "NON_DEFINI";
          LTARFrecu = StringJson(DateRTE2, RTEdata);
          if (LTARFrecu.indexOf("BLUE") >= 0) {
            line = "4";
            lendemain = "TEMPO_BLEU";
          }
          if (LTARFrecu.indexOf("WHITE") >= 0) {
            line = "8";
            lendemain = "TEMPO_BLANC";
          }
          if (LTARFrecu.indexOf("RED") >= 0) {
            line = "C";
            lendemain = "TEMPO_ROUGE";
          }
          STGE = line;  //Valeur Hexa code du Linky
          StockMessage(DateRTE + " : " + LTARF + " | " + DateRTE2 + " : " + lendemain);
          RTEdata = "";
          LastHeureRTE = HeureCouranteDeci;  //Heure lecture Tempo RTE
        } else {
          StockMessage(DateRTE + " : Pas de données RTE valides");
        }
      }
    } else {
      if (Source != "Linky" && Source != "Ext") {
        LTARF = "";
        STGE = "0";
      }
    }
  }
}
