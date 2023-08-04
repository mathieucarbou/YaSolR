// *******************************************************
// * Recherche Info Tempo EDF pour les sources non Linky *
// *******************************************************


void Call_EDF_data() {

  const char* adr_EDF_Host = "particulier.edf.fr";
  String Host = String(adr_EDF_Host);
  String urlJSON = "/services/rest/referentiel/searchTempoStore?dateRelevant=" + DateEDF;
  String EDFdata = "";
  String line = "";
  int Hcour = HeureCouranteDeci / 2;  //Par pas de 72secondes pour faire 2 appels si un bug
  int LastH = LastHeureEDF / 2;

  if ((LastH != Hcour) && ( Hcour == 300 || Hcour == 310 || Hcour == 530 || Hcour == 560 || Hcour == 600 || Hcour == 900 || Hcour == 1150) || LastHeureEDF < 0) {
    if (TempoEDFon == 1) {
      // Use clientSecu class to create TCP connections
      clientSecuEDF.setInsecure();  //skip verification
      if (!clientSecuEDF.connect(adr_EDF_Host, 443)) {
        StockMessage("Connection failed to EDF server :" + Host);
      } else {
        String Request=String("GET ") + urlJSON + " HTTP/1.1\r\n" ;
        Request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n" ;
        Request += "Accept-Encoding: gzip, deflate, br, zstd\r\n" ;
        Request += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36\r\n" ;
        Request += "Host: " + Host + "\r\n" ;
        Request += "Connection: keep-alive\r\n\r\n";
        clientSecuEDF.print(Request);
        Serial.println("Request vers EDF Envoyé");
        unsigned long timeout = millis();
        while (clientSecuEDF.available() == 0) {
          if (millis() - timeout > 5000) {
            StockMessage(">>> clientSecuEDF EDF Timeout !");
            clientSecuEDF.stop();
            return;
          }
        }
        timeout = millis();
        // Lecture des données brutes distantes
        int fin = 0;
        while (clientSecuEDF.connected() && (millis() - timeout < 5000) && fin < 2) {
          line = clientSecuEDF.readStringUntil('\n');
          EDFdata += line;
          if (line == "\r") {
            StockMessage("EnTetes EDF reçues");
            EDFdata = "";
            fin = 1;
          }
          if (fin == 1 && line.indexOf("}") >= 0) fin = 2;
        }
        clientSecuEDF.stop();

        // C'est EDF qui donne la couleur
        String LTARFrecu = StringJson("couleurJourJ", EDFdata);  //Remplace code du Linky
        if (LTARFrecu.indexOf("TEMPO") >= 0) {
          LTARF = LTARFrecu;
          String couleurJourJ1 = StringJson("couleurJourJ1", EDFdata);
          line = "0";
          if (couleurJourJ1 == "TEMPO_BLEU") line = "4";
          if (couleurJourJ1 == "TEMPO_BLANC") line = "8";
          if (couleurJourJ1 == "TEMPO_ROUGE") line = "C";
          STGE = line;  //Valeur Hexa code du Linky
          StockMessage(DateEDF + " : " + EDFdata);
          EDFdata = "";
          LastHeureEDF = HeureCouranteDeci;  //Heure lecture Tempo EDF
        } else {
          StockMessage(DateEDF + " : Pas de données EDF valides");
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
