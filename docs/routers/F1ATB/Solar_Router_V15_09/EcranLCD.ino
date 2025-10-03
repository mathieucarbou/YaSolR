#include <vector>
#include <SPI.h>
#include <cstdint>
#define LIGHT_ADC 34
#define LGFX_AUTODETECT  // Autodetect board
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

LGFX lcd;
unsigned long runtime_0 = 0, runtime_click = 0, runtime_On = 0;
int8_t NumPage = 0, NumPage0 = 0, NbrPage = 6;
int16_t ClickX = 0, ClickY = 0, ClickCount = 0;
int LigneTotalOld, LigneTotal, LigneIdx;
bool ReDraw = false, ScreenOn = true;
int16_t ForceIPidx = 0, ForceIdx = 0, ForceOnOff = 0;


unsigned long CoulTexte, CoulFond, CoulBouton, CoulBoutFond, CoulBoutBord, CoulW, CoulWh, CoulTabTexte, CoulTabFond, CoulTabBord;
unsigned long CoulSaisieTexte, CoulSaisieFond, CoulSaisieBord, CoulTemp, CoulGrTexte, CoulGrFond;
int H_Onglet = 30;
void Ecran_Init(void) {
  pinMode(17, OUTPUT);  //LED bleue non utilisée
  digitalWrite(17, 1);  //Extinction
  pinMode(35, INPUT);   //Entrée capteur Infra-Rouge
  pinMode(21, OUTPUT);  //LED bleue non utilisée
  lcd.init();
  // Réinitialiser le mode non miroir
  lcd.writecommand(0x36);  // Commande MADCTL
  lcd.writedata(0x00);     // Mode normal sans inversion
  lcd.setRotation(rotation);
  SetCouleurs();
  lcd.setFont(&fonts::AsciiFont8x16);
  if (Calibre[7] != 0) {
    lcd.setTouchCalibrate(Calibre);  //Ancienne calibration ecran
  }
  GoPage(0);
}
void Ecran_Loop() {
  if ((millis() - runtime_0) > 3000) {
    if (ScreenOn) {
      lcd.setTextColor(CoulTexte, CoulFond);
      switch (NumPage) {
        case 0:
          AccueilLoop();
          break;
        case 1:
          GrapheTrace(10);
          break;
        case 3:
          runtime_On = millis(); //Cas des messages de debug, on laisse allumé
          break;
        case 4:
          TraceReseau();
          break;
      }
    }

    runtime_0 = millis();
  }
  if ((millis() - runtime_On) > DurEcran) {
    lcd.clear(TFT_BLACK);
    ReDraw = true;
    ScreenOn = false;
    runtime_On = millis();
  }
  if (digitalRead(35) == 1) {
    ScreenOn = true;
    if (ReDraw) GoPage(NumPage);
    ReDraw = false;
    runtime_On = millis();
  }

  if (millis() - runtime_click > 200) {  //Anti-glitch
    runtime_click = millis();
    if (lcd.getTouch(&ClickX, &ClickY)) {
      runtime_On = millis();
      if (!ScreenOn) {
        ScreenOn = true;
        GoPage(NumPage);
      } else {

        ClickCount = ClickCount + 10;
        if (ClickCount > 200) TraceCalibr();  //Pression de 3s force la calibration
        if (Calibre[7] == 0) {                //Pas encore calibré
          TraceCalibr();
        } else {
          if (ClickY < H_Onglet + 10) {
            int W = (lcd.width() - 36) / 3;
            if (ClickX <= 18) NumPage0 = (NumPage0 - 1 + NbrPage) % NbrPage;
            if (ClickX > 18 && ClickX < 18 + W) NumPage = (0 + NumPage0) % NbrPage;
            if (ClickX > 18 + W && ClickX < 18 + 2 * W) NumPage = (1 + NumPage0) % NbrPage;
            if (ClickX > 18 + 2 * W && ClickX < 18 + 3 * W) NumPage = (2 + NumPage0) % NbrPage;
            if (ClickX >= lcd.width() - 18) NumPage0 = (NumPage0 + 1 + NbrPage) % NbrPage;

            GoPage(NumPage);

          } else {
            switch (NumPage) {
              case 0:
                AccueilClick();
                break;
              case 5:
                ClickPreCalibr();
                break;
              case 10:
                AccueilForceClick();
                break;
            }
          }
        }
      }
    }
    if (ClickCount > 0) ClickCount--;
  }
}

void SetCouleurs() {
  if (Couleurs.length() > 6) {
    CoulTexte = ConvCouleur(Couleurs.substring(0, 6));
    CoulFond = ConvCouleur(Couleurs.substring(6, 12));
    CoulBouton = ConvCouleur(Couleurs.substring(18, 24));
    CoulBoutFond = ConvCouleur(Couleurs.substring(24, 30));
    CoulBoutBord = ConvCouleur(Couleurs.substring(30, 36));
    CoulSaisieTexte = ConvCouleur(Couleurs.substring(36, 42));
    CoulSaisieFond = ConvCouleur(Couleurs.substring(42, 48));
    CoulSaisieBord = ConvCouleur(Couleurs.substring(48, 54));
    CoulTabTexte = ConvCouleur(Couleurs.substring(54, 60));
    CoulTabFond = ConvCouleur(Couleurs.substring(60, 66));
    CoulTabBord = ConvCouleur(Couleurs.substring(66, 72));
    CoulW = ConvCouleur(Couleurs.substring(72, 78));
    CoulWh = ConvCouleur(Couleurs.substring(84, 90));
    CoulGrTexte = ConvCouleur(Couleurs.substring(114, 120));
    CoulGrFond = ConvCouleur(Couleurs.substring(120, 126));
    CoulTemp = ConvCouleur(Couleurs.substring(132, 138));
  } else {
    CoulTexte = TFT_BLACK;
    CoulFond = TFT_LIGHTGREY;
    CoulBouton = TFT_WHITE;
    CoulBoutFond = TFT_LIGHTGREY;
    CoulBoutBord = TFT_DARKGREY;
    CoulW = TFT_RED;
    CoulWh = TFT_YELLOW;
    CoulTabTexte = TFT_BLACK;
    CoulTabFond = TFT_LIGHTGREY;
    CoulTabBord = TFT_BLACK;
    CoulSaisieTexte = TFT_WHITE,
    CoulSaisieFond = TFT_DARKGREY;
    CoulSaisieBord = TFT_BLACK;
    CoulTemp = TFT_GREEN;
  }
  lcd.fillScreen(CoulFond);
  lcd.setTextColor(CoulTexte, CoulFond);
}
void GoPage(int N) {
  if (ScreenOn) {
    NumPage = N;
    lcd.fillScreen(CoulFond);
    OngletsTrace(NumPage0);
    lcd.setTextColor(CoulTexte, CoulFond);
    switch (NumPage) {
      case 0:
        AccueilTrace();
        break;
      case 1:
        GrapheTrace(10);
        break;
      case 2:
        GrapheTrace(48);
        break;
      case 3:
        TraceMessages();
        break;
      case 4:
        TraceReseau();
        break;
      case 5:
        TracePreCalibr();
        break;
    }
  }
}
void OngletsTrace(int8_t page) {
  String Titre[] = { "Accueil", "Gr.10mn", "Gr.48h", "Messages", "Réseau", "Calibr." };
  lcd.fillRect(0, 0, lcd.width(), H_Onglet, CoulBoutFond);
  int W = (lcd.width() - 36) / 3;
  for (int i = 0; i <= 3; i++) {
    int x = 18 + W * i;
    lcd.drawFastVLine(x, 0, H_Onglet, CoulBoutBord);
    lcd.drawFastVLine(x + 1, 0, H_Onglet, CoulBoutBord);
    if (NumPage != (i + page) % NbrPage) {
      lcd.drawFastHLine(x, H_Onglet, W, CoulBoutBord);
      lcd.drawFastHLine(x, H_Onglet - 1, W, CoulBoutBord);
      lcd.setTextColor(CoulBouton, CoulBoutFond);
    } else {
      lcd.fillRect(x, 0, W, H_Onglet, CoulFond);
      lcd.setTextColor(CoulBouton, CoulFond);
    }
    if (i < 3) PrintCentre(Ascii(Titre[(i + page) % NbrPage]), x + W / 2, 0, 1 + 0.4 * float(rotation % 2));
  }
  lcd.fillRect(0, 0, 18, H_Onglet, CoulBoutBord);
  lcd.fillRect(lcd.width() - 18, 0, 18, H_Onglet, CoulBoutBord);
  lcd.setTextColor(CoulBouton, CoulBoutBord);
  PrintCentre("<", 9, 0, 1.3);
  PrintCentre(">", lcd.width() - 9, 0, 1.3);
}
void AccueilTrace() {
  int WE, HE, W, W2, H, Hconso, H1, H2, H3, Hact, Hdelta, Hbas;
  float F0, F1, F2;
  String NomR, ActionR, ActionsR, TempR, TempsR, valeur0, valeur;
  Liste_NomsEtats(0);  //On rafraichi interne
  AccueilCadrage(WE, HE, W, W2, H, Hconso, H1, H2, H3, Hact, Hdelta, Hbas, F0, F1, F2);

  //Tableau consommations
  PrintCentre(Ascii("Soutirée"), W + W / 2, Hconso - 2, F0);
  PrintCentre(Ascii("Injectée"), 2 * W + W / 2, Hconso - 2, F0);
  lcd.fillRect(0, H1, 3 * W, H, CoulW);
  lcd.fillRect(0, H2, 3 * W, H, CoulWh);
  lcd.drawRect(0, H1, 3 * W, H, CoulTabBord);
  lcd.drawRect(0, H2, 3 * W, H, CoulTabBord);
  lcd.drawRect(W, Hconso, W, H2 + H - Hconso, CoulTabBord);
  lcd.drawRect(2 * W, Hconso, W, H2 + H - Hconso, CoulTabBord);
  lcd.setTextColor(CoulTabTexte, CoulW);
  PrintGauche("P (W)", 0, H1, F1);
  lcd.setTextColor(CoulTabTexte, CoulWh);
  PrintGauche("E (Wh)", 0, H2, F1);

  //Tableau Action puis température
  //Tableau actions en cours
  int LigneCount = 0;
  lcd.setTextColor(CoulTabTexte, CoulTabFond);
  for (int i = 0; i < LesRouteursMax; i++) {
    if (RMS_IP[i] > 0 && RMS_Note[i] > 0) {
      SplitS(RMS_NomEtat[i], NomR, US, ActionsR);
      SplitS(ActionsR, TempsR, US, ActionsR);
      while (ActionsR.length() > 1 && (Hact + H3) <= Hbas) {
        SplitS(ActionsR, ActionR, FS, ActionsR);
        if (LigneCount >= LigneIdx) {
          SplitS(ActionR, valeur, ES, ActionR);
          SplitS(ActionR, valeur0, ES, valeur);  //Nom
          lcd.fillRect(0, Hact, WE, H3, CoulTabFond);
          PrintGauche(Ascii(valeur0.substring(0, 16)), 0, Hact, F2);
          lcd.drawRect(0, Hact, 2 * W, H3, CoulTabBord);
          lcd.drawRect(2 * W, Hact, W2, H3, CoulTabBord);
          lcd.drawRect(2 * W + W2, Hact, W2, H3, CoulTabBord);
          Hact += H3;
        }
        LigneCount++;
      }
    }
  }
  Hact += Hdelta;
  //Tableau Températures
  lcd.setTextColor(CoulTabTexte, CoulTemp);
  for (int i = 0; i < LesRouteursMax; i++) {
    if (RMS_IP[i] > 0 && RMS_Note[i] > 0) {
      SplitS(RMS_NomEtat[i], NomR, US, ActionsR);
      SplitS(ActionsR, TempsR, US, ActionsR);
      while (TempsR.length() > 1 && (Hact + H3) <= Hbas) {
        SplitS(TempsR, TempR, FS, TempsR);
        if (TempR.indexOf("tempInt") > 0) {
          if (LigneCount >= LigneIdx) {
            SplitS(TempR, valeur, ES, TempR);
            SplitS(TempR, valeur0, ES, valeur);  //Nom
            lcd.fillRect(0, Hact, WE, H3, CoulTemp);
            PrintGauche(Ascii(valeur0.substring(0, 24)), 0, Hact, F2);
            lcd.drawRect(0, Hact, 2 * W + W2, H3, CoulTabBord);
            lcd.drawRect(2 * W + W2, Hact, W2, H3, CoulTabBord);
            Hact += H3;
          }
          LigneCount++;
        }
      }
    }
  }

  if (W2 < W) {  //On a beaucoup de ligne. Curseur vertical
    Hact = H2 + H;
    int16_t haut = Hbas - Hact;
    lcd.setTextColor(CoulBouton, CoulBoutBord);
    lcd.fillRect(WE - W2 / 2, Hact, W2, haut, CoulBoutBord);
    haut = haut / 2;
    lcd.drawRect(WE - W2 / 2, Hact, W2, haut, CoulTabBord);
    lcd.drawRect(WE - W2 / 2, Hact + haut, W2, haut, CoulTabBord);
    if (LigneIdx > 0) PrintCentre("<<", WE - W2 / 4, Hact + 10, 1.5);
    if (LigneIdx < LigneTotal - 1) PrintCentre(">>", WE - W2 / 4, Hbas - 40, 1.5);
  }
}
void AccueilClick() {
  int WE, HE, W, W2, H, Hconso, H1, H2, H3, Hact, Hdelta, Hbas;
  float F0, F1, F2;
  AccueilCadrage(WE, HE, W, W2, H, Hconso, H1, H2, H3, Hact, Hdelta, Hbas, F0, F1, F2);
  if (W2 < W && ClickX > WE - W2 / 2) {  //Boutons defilement
    int espace = (Hbas - Hact) / 2;
    if (ClickY < Hact + espace) {
      if (LigneIdx > 0) {
        LigneIdx--;
        GoPage(0);
      }
    } else {
      if (LigneIdx < LigneTotal - 1) {
        LigneIdx++;
        GoPage(0);
      }
    }
  } else {
    String NomR, NomAct, ActionR, ActionsR, TempR, TempsR, valeur, valeur0;
    int LigneCount = 0;
    //Tableau actions en cours
    for (int i = 0; i < LesRouteursMax; i++) {
      if (RMS_IP[i] > 0 && RMS_Note[i] > 0) {
        SplitS(RMS_NomEtat[i], NomR, US, ActionsR);
        SplitS(ActionsR, TempsR, US, ActionsR);
        while (ActionsR.length() > 1 && (Hact + H3) <= Hbas) {
          SplitS(ActionsR, ActionR, FS, ActionsR);
          if (LigneCount >= LigneIdx) {
            if (ClickY >= Hact && ClickY <= (Hact + H3)) {
              SplitS(ActionR, valeur0, ES, ActionR);
              ForceIdx = valeur0.toInt();  //Num Action
              ForceIPidx = i;
              SplitS(ActionR, NomAct, ES, valeur);
              SplitS(valeur, valeur0, ES, valeur);  //Ouverture
              SplitS(valeur, valeur0, ES, valeur);  //Hequiv
              SplitS(valeur, valeur0, ES, valeur);  //tOnOff
              NumPage = 10;
              lcd.setTextColor(CoulTabTexte, CoulTabFond);
              lcd.fillRect(0, H_Onglet, WE, HE - H_Onglet, CoulTabFond);
              lcd.drawRect(0, H_Onglet, WE, HE - H_Onglet, CoulTabBord);
              PrintCentre(Ascii("Forçage"), WE / 2, H_Onglet + 10, 1.5);
              PrintCentre(Ascii(NomR), WE / 2, H_Onglet + 50, 1.5);
              PrintCentre(Ascii(NomAct), WE / 2, H_Onglet + 90, 1.5);
              PrintCentre(valeur + " mn", WE / 2, H_Onglet + 135, 1.5);
              ForceOnOff = valeur.toInt();  //Duree
              uint16_t coul = CoulBoutFond;
              if (ForceOnOff > 0) coul = TFT_RED;
              lcd.fillRect(2, H_Onglet + 130, WE / 4, 40, coul);
              lcd.drawRect(2, H_Onglet + 130, WE / 4, 40, CoulBoutBord);
              lcd.setTextColor(CoulBouton, coul);
              PrintCentre("On", WE / 8 + 1, H_Onglet + 135, 1.5);
              coul = CoulBoutFond;
              if (ForceOnOff < 0) coul = TFT_RED;
              lcd.fillRect(WE - 2 - WE / 4, H_Onglet + 130, WE / 4, 40, coul);
              lcd.drawRect(WE - 2 - WE / 4, H_Onglet + 130, WE / 4, 40, CoulBoutBord);
              lcd.setTextColor(CoulBouton, coul);
              PrintCentre("Off", WE - WE / 8 - 1, H_Onglet + 135, 1.5);
              i = 100;
            }
            Hact += H3;
          }
          LigneCount++;
        }
      }
    }
  }
}
void ClickPreCalibr() {
  int Htop = H_Onglet + 60;
  int H = (lcd.height() - Htop) / 2;
  if (ClickY > Htop && ClickY < Htop + H) TraceCalibr();
  if (ClickY > Htop + H) {
    GoPage(0);
  }
}
void AccueilForceClick() {
  if (ClickY >= H_Onglet + 130 && ClickY <= H_Onglet + 170) {
    int WE = lcd.width();
    if (ClickX < WE / 2) {
      if (ForceOnOff < 0) {
        ForceOnOff = 0;
      } else {
        ForceOnOff += 30;
      }
    } else {
      if (ForceOnOff > 0) {
        ForceOnOff = 0;
      } else {
        ForceOnOff -= 30;
      }
    }
    lcd.setTextColor(CoulTabTexte, CoulTabFond);
    PrintCentre("        ", WE / 2, H_Onglet + 135, 1.5);
    PrintCentre(String(ForceOnOff) + " mn", WE / 2, H_Onglet + 135, 1.5);
    uint16_t coul = CoulBoutFond;
    if (ForceOnOff > 0) coul = TFT_RED;
    lcd.fillRect(2, H_Onglet + 130, WE / 4, 40, coul);
    lcd.drawRect(2, H_Onglet + 130, WE / 4, 40, CoulBoutBord);
    lcd.setTextColor(CoulBouton, coul);
    PrintCentre("On", WE / 8 + 1, H_Onglet + 135, 1.5);
    coul = CoulBoutFond;
    if (ForceOnOff < 0) coul = TFT_RED;
    lcd.fillRect(WE - 2 - WE / 4, H_Onglet + 130, WE / 4, 40, coul);
    lcd.drawRect(WE - 2 - WE / 4, H_Onglet + 130, WE / 4, 40, CoulBoutBord);
    lcd.setTextColor(CoulBouton, coul);
    PrintCentre("Off", WE - WE / 8 - 1, H_Onglet + 135, 1.5);
    if (ForceIPidx == 0) {
      LesActions[ForceIdx].tOnOff = ForceOnOff;
    } else {
      WiFiClient clientESP_RMS;
      String host = IP2String(RMS_IP[ForceIPidx]);
      if (!clientESP_RMS.connect(host.c_str(), 80)) {
        StockMessage("connection to ESP_RMS (Force)  : " + host + " failed");
        clientESP_RMS.stop();
        if (RMS_Note[ForceIPidx] > 0) RMS_Note[ForceIPidx]--;
        delay(2);
        return;
      }
      String url = "/ForceAction?Force=" + String(ForceOnOff) + "&NumAction=" + String(ForceIdx);
      clientESP_RMS.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while (clientESP_RMS.available() == 0) {
        if (millis() - timeout > 5000) {
          StockMessage("client ESP_RMS (Force) Timeout !" + host);
          clientESP_RMS.stop();
          RMS_Note[ForceIPidx] = false;
          return;
        }
      }
      timeout = millis();
      // Lecture des données brutes distantes
      while (clientESP_RMS.available() && (millis() - timeout < 5000)) {
        String Reponse = clientESP_RMS.readStringUntil('\r');
      }
      clientESP_RMS.stop();
    }
    RMS_Note[ForceIPidx] = true;
    Liste_NomsEtats(ForceIPidx);  //Pour rafraichir état
  }
}

void AccueilLoop() {

  int WE, HE, W, W2, H, Hconso, H1, H2, H3, Hact, Hdelta, Hbas;
  float F0, F1, F2;
  Liste_NomsEtats(0);  //On rafraichi interne
  AccueilCadrage(WE, HE, W, W2, H, Hconso, H1, H2, H3, Hact, Hdelta, Hbas, F0, F1, F2);
  lcd.setCursor(2, HE - 20);
  lcd.setTextSize(1);
  lcd.print(DATE);
  TraceTarif();
  lcd.fillRect(W + 1, H1 + 1, W - 2, H - 2, CoulW);
  lcd.fillRect(2 * W + 1, H1 + 1, W - 2, H - 2, CoulW);
  lcd.setTextColor(CoulTabTexte, CoulW);
  PrintDroite(String(PuissanceS_M), 2 * W, H1, F1);
  PrintDroite(String(PuissanceI_M), 3 * W, H1, F1);
  lcd.fillRect(W + 1, H2 + 1, W - 2, H - 2, CoulWh);
  lcd.fillRect(2 * W + 1, H2 + 1, W - 2, H - 2, CoulWh);
  lcd.setTextColor(CoulTabTexte, CoulWh);
  PrintDroite(String(EnergieJour_M_Soutiree), 2 * W, H2, F1);
  PrintDroite(String(EnergieJour_M_Injectee), 3 * W, H2, F1);
  uint16_t C = TFT_RED;
  float Angle = 0;
  if (PuissanceI_M > 0) {
    C = TFT_GREEN;
    Angle = float(PuissanceI_M) / 10;
  } else {
    Angle = float(PuissanceS_M) / 20;
  }
  Angle = min(float(360), Angle);
  Angle = Angle - 90;
  lcd.fillCircle(3.5 * W, H2 - 5, H + 1, TFT_WHITE);
  lcd.fillArc(3.5 * W, H2 - 5, H - 1, 0, -90, Angle, C);

  String NomR, ActionR, ActionsR, TempR, TempsR, valeur, valeur0;
  int LigneCount = 0;
  //Tableau actions en cours
  lcd.setTextColor(CoulTabTexte, CoulTabFond);

  for (int i = 0; i < LesRouteursMax; i++) {
    if (RMS_IP[i] > 0 && RMS_Note[i]) {
      SplitS(RMS_NomEtat[i], NomR, US, ActionsR);
      SplitS(ActionsR, TempsR, US, ActionsR);
      while (ActionsR.length() > 1 && (Hact + H3) <= Hbas) {
        SplitS(ActionsR, ActionR, FS, ActionsR);
        if (LigneCount >= LigneIdx) {
          SplitS(ActionR, valeur, ES, ActionR);
          SplitS(ActionR, valeur0, ES, valeur);
          SplitS(valeur, valeur0, ES, valeur);
          lcd.fillRect(2 * W + 1, Hact + 1, W2 - 2, H3 - 2, CoulTabFond);
          PrintDroite(valeur0 + "%", 2 * W + W2, Hact, F2);  //Ouverture
          SplitS(valeur, valeur0, ES, valeur);
          int heure = int(valeur0.toInt() / 100);
          String mn = "00" + String(int((valeur0.toInt() - 100 * heure) * 0.6));
          mn = String(heure) + ":" + mn.substring(mn.length() - 2);
          lcd.fillRect(2 * W + W2 + 1, Hact + 1, W2 - 2, H3 - 2, CoulTabFond);
          PrintDroite(mn, 2 * W + 2 * W2, Hact, F2);
          Hact += H3;
        }
        LigneCount++;
      }
    }
  }
  Hact += Hdelta;
  //Tableau Températures
  lcd.setTextColor(CoulTabTexte, CoulTemp);
  for (int i = 0; i < LesRouteursMax; i++) {
    if (RMS_IP[i] > 0 && RMS_Note[i]) {
      SplitS(RMS_NomEtat[i], NomR, US, ActionsR);
      SplitS(ActionsR, TempsR, US, ActionsR);
      while (TempsR.length() > 1 && (Hact + H3) <= Hbas) {
        SplitS(TempsR, TempR, FS, TempsR);
        if (TempR.indexOf("tempInt") > 0) {
          if (LigneCount >= LigneIdx) {
            SplitS(TempR, valeur, ES, TempR);
            SplitS(TempR, valeur0, ES, valeur);
            SplitS(valeur, valeur, ES, valeur0);
            lcd.fillRect(2 * W + W2 + 1, Hact + 1, W2 - 2, H3 - 2, CoulTemp);
            PrintDroite(valeur + String(char(248)) + "C", 2 * W + 2 * W2, Hact, F2);
            Hact += H3;
          }
          LigneCount++;
        }
      }
    }
  }
}
void AccueilCadrage(int& WE, int& HE, int& W, int& W2, int& H, int& Hconso, int& H1, int& H2, int& H3, int& Hact, int& Hdelta, int& Hbas, float& F0, float& F1, float& F2) {
  WE = lcd.width();
  HE = lcd.height();
  W = WE / 4;
  H = HE / (14 - 6 * float(rotation % 2));
  Hconso = H_Onglet + 4;
  H1 = Hconso + H / 2 + 2;
  H2 = H1 + H;
  H3 = 20;
  F0 = 0.8 + 0.2 * float(rotation % 2);  //Fonte
  F1 = 1.2 + 0.3 * float(rotation % 2);
  LigneTotal = 0;

  String NomR, ActionR, ActionsR, TempR, TempsR;
  for (int i = 0; i < LesRouteursMax; i++) {
    if (RMS_IP[i] > 0 && RMS_Note[i]) {
      SplitS(RMS_NomEtat[i], NomR, US, ActionsR);
      SplitS(ActionsR, TempsR, US, ActionsR);
      while (ActionsR.length() > 1) {
        SplitS(ActionsR, ActionR, FS, ActionsR);
        LigneTotal++;
      }
      while (TempsR.length() > 1) {
        SplitS(TempsR, TempR, FS, TempsR);
        if (TempR.indexOf("tempInt") > 0) LigneTotal++;
      }
    }
  }

  Hbas = HE - 30 + 10 * float(rotation % 2);
  int espace = Hbas - (H2 + H);
  Hdelta = 0;
  F2 = 1;
  if (espace > LigneTotal * H3) {
    Hdelta = (espace - LigneTotal * H3) / 3;
    LigneIdx = 0;
    W2 = W;
  } else {
    LigneIdx = max(LigneIdx, 0);
    LigneIdx = min(LigneTotal, LigneIdx);
    W2 = int(2 * W / 2.5);
  }
  Hact = H2 + H + Hdelta;
  if (LigneTotal != LigneTotalOld) ReDraw = true;
  LigneTotalOld = LigneTotal;
}
void GrapheTrace(int8_t gr) {
  int WE = lcd.width();
  int HE = lcd.height();
  int iS1, iS2, Y1, Y2, X1, X2;
  PrintCentre("Puissance W : " + Ascii(nomSondeMobile), WE / 2, H_Onglet + 10, 1.2);
  int Maxi = 0;
  if (gr == 10) {  //10mn
    for (int i = 0; i < 300; i++) {
      Maxi = max(Maxi, abs(tabPw_Maison_2s[i]));
    }
  } else {  //48h
    for (int i = 0; i < 600; i++) {
      Maxi = max(Maxi, abs(tabPw_Maison_5mn[i]));
    }
  }
  int step = 500;
  if (Maxi > 2000) step = 1000;
  Maxi = step * int(Maxi / step) + step;
  lcd.setTextColor(CoulGrTexte, CoulGrFond);
  lcd.fillRect(0, H_Onglet + 35, WE, HE - 35 - H_Onglet, CoulGrFond);
  int Hm = HE / 2 + 10 + H_Onglet / 2;
  lcd.drawFastHLine(40, Hm, WE - 50, CoulGrTexte);
  lcd.drawFastVLine(40, H_Onglet + 50, HE - 70 - H_Onglet, CoulGrTexte);
  int tick = Maxi / step;
  int delta = (HE - 70 - H_Onglet) / (2 * tick);
  for (int i = -tick; i <= tick; i++) {
    lcd.drawFastHLine(35, Hm - delta * i, 5, CoulGrTexte);
    PrintDroite(String(i * step), 35, Hm - delta * i - 10, 0.8);
  }
  if (gr == 10) {  //10mn
    delta = (WE - 50) / 10;
    for (int i = 1; i <= 10; i++) {
      lcd.drawFastVLine(40 + i * delta, Hm, 5, CoulGrTexte);
      if (i % 2 == 0) PrintCentre(String(i - 10), 40 + i * delta, Hm + 5, 0.8);
    }
    PrintCentre("mn", WE - 10, Hm - 20, 0.8);
    iS1 = IdxStock2s;
    delta = (HE - 70 - H_Onglet) / 2;
    for (int i = 0; i < 299; i++) {
      iS2 = (iS1 + 1) % 300;
      Y1 = Hm - delta * tabPw_Maison_2s[iS1] / Maxi;
      Y2 = Hm - delta * tabPw_Maison_2s[iS2] / Maxi;
      X1 = 40 + (WE - 50) * i / 300;
      X2 = 40 + (WE - 50) * (i + 1) / 300;
      lcd.drawLine(X1, Y1, X2, Y2, CoulW);
      iS1 = (iS1 + 1) % 300;
    }
  } else {
    delta = (WE - 50) / 48;
    int mn = (HeureCouranteDeci % 100) * delta;
    mn = WE - 10 - int(mn / 100);
    int h = int(HeureCouranteDeci / 100);
    for (int i = 0; i < 48; i++) {
      if (h % 6 == 0) {
        lcd.drawFastVLine(mn, Hm, 5, CoulGrTexte);
        PrintCentre(String(h), mn, Hm + 5, 0.8);
      }
      mn += -delta;
      h = (h + 23) % 24;
    }
    PrintCentre("h", WE - 10, Hm - 20, 0.8);
    iS1 = IdxStockPW;
    delta = (HE - 70 - H_Onglet) / 2;
    for (int i = 0; i < 599; i++) {
      iS2 = (iS1 + 1) % 600;
      Y1 = Hm - delta * tabPw_Maison_5mn[iS1] / Maxi;
      Y2 = Hm - delta * tabPw_Maison_5mn[iS2] / Maxi;
      X1 = 40 + (WE - 50) * i / 600;
      X2 = 40 + (WE - 50) * (i + 1) / 600;
      lcd.drawLine(X1, Y1, X2, Y2, CoulW);
      iS1 = (iS1 + 1) % 600;
    }
  }
}




void TraceMessages() {
  lcd.setTextColor(CoulTexte, CoulFond);
  lcd.setScrollRect(0, H_Onglet, lcd.width(), lcd.height() - H_Onglet, CoulFond);
  lcd.setTextScroll(true);
  PrintCentre("Messages", -1, H_Onglet, 1.2);
  lcd.println("");
  lcd.setTextSize(1);
  int j = idxMessage;
  for (int i = 0; i < 10; i++) {
    lcd.println(Ascii(MessageH[j]));
    j = (j + 1) % 10;
  }
  lcd.setTextScroll(false);
}
void TraceReseau() {
  lcd.setTextColor(CoulTexte, CoulFond);
  lcd.fillRect(0, H_Onglet, lcd.width(), lcd.height() - H_Onglet, CoulFond);
  PrintCentre(Ascii("Réseau"), -1, H_Onglet, 1.2);
  lcd.println("");
  lcd.setTextSize(1);
  lcd.println("");
  if (WiFi.getMode() == WIFI_STA) {
    lcd.println("Niveau WiFi : " + String(WiFi.RSSI()) + " dBm");
    lcd.println(Ascii("Point d'accès WiFi : ") + WiFi.BSSIDstr());
    lcd.println("Adresse MAC : " + WiFi.macAddress());
    lcd.println(Ascii("Réseau WiFi : ") + ssid);
    lcd.println("Adresse IP ESP32 : " + WiFi.localIP().toString());
    lcd.println("Adresse passerelle : " + WiFi.gatewayIP().toString());
    lcd.println(Ascii("Masque du réseau : ") + WiFi.subnetMask().toString());
  } else {
    lcd.println("Connectez vous au WiFi : ");
    lcd.println(Ascii("Mode Point d'Accès : ") + hostname);
    lcd.print("Adresse IP ESP32 : ");
    lcd.println(WiFi.softAPIP());
  }
}
void TracePreCalibr() {
  PrintCentre("Recalibrer l'" + String(char(130)) + "cran ?", -1, H_Onglet + 10, 1 + 0.8 * float(rotation % 2));
  int Htop = H_Onglet + 60;
  int H = (lcd.height() - Htop) / 2;
  lcd.fillRect(0, Htop, lcd.width(), 2 * H, CoulSaisieFond);
  lcd.drawRect(0, Htop, lcd.width(), H, CoulSaisieBord);
  lcd.drawRect(0, Htop + H, lcd.width(), H, CoulSaisieBord);
  lcd.setTextColor(CoulSaisieTexte, CoulSaisieFond);
  PrintCentre("Oui", -1, Htop + H / 2 - 20, 2);
  PrintCentre("Non", -1, Htop + H + H / 2 - 20, 2);
}
void TraceCalibr() {
  lcd.fillScreen(CoulFond);
  lcd.setTextColor(CoulTexte, CoulFond);
  PrintCentre("Calibration", -1, lcd.height() / 2, 2.5);
  PrintCentre("Cliquez dans le coin", -1, lcd.height() / 2 + 50, 2);
  delay(1000);
  lcd.calibrateTouch(Calibre, TFT_MAGENTA, TFT_WHITE, 30);  // Runs a test that has you touch the corners of the screen
  lcd.setTouchCalibrate(Calibre);                           // setTouch actually implements the data form calibrateTouch
  lcd.fillScreen(CoulFond);
  EcritureEnROM();
  ClickCount = 0;
  GoPage(0);
}

void TraceTarif() {
  String Tarif[5] = { "PLEINE", "CREUSE", "BLEU", "BLANC", "ROUGE" };
  uint16_t couleur[5] = { TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE, TFT_RED };
  int16_t i;
  int16_t W, H;
  W = 20 + 20 * float(rotation % 2);
  H = 30 - 10 * float(rotation % 2);
  if (LTARF != "") {
    lcd.drawRect(lcd.width() - W * 3, lcd.height() - H, 3 * W - 1, H - 1, CoulSaisieBord);
    for (i = 0; i < 5; i++) {
      if (LTARF.indexOf(Tarif[i]) >= 0) {
        lcd.fillRect(lcd.width() - W * 3 + 1, lcd.height() - H + 1, 3 * W - 2, H - 2, couleur[i]);
      }
    }
  }

  long lendemain = int(strtol(STGEt.c_str(), NULL, 16)) / 4;
  if (lendemain > 0) {
    lcd.drawRect(lcd.width() - W, lcd.height() - H, W - 1, H - 1, CoulSaisieBord);
    lcd.fillRect(lcd.width() - W + 1, lcd.height() - H + 1, W - 2, H - 2, couleur[lendemain + 1]);
  }
}
void PrintCentre(String S, int X, int Y, float Sz) {
  if (X < 0) X = lcd.width() / 2;
  lcd.setTextSize(Sz);
  int W = lcd.textWidth(S);
  lcd.setCursor(X - W / 2, Y + 3);
  lcd.print(S);
}
void PrintGauche(String S, int X, int Y, float Sz) {
  lcd.setTextSize(Sz);
  lcd.setCursor(X + 4, Y + 3);
  lcd.print(S);
}
void PrintDroite(String S, int X, int Y, float Sz) {
  if (X < 0) X = lcd.width() / 2;
  lcd.setTextSize(Sz);
  int W = lcd.textWidth(S);
  lcd.setCursor(X - W - 4, Y + 3);
  lcd.print(S);
}

String Ascii(String S) {
  S.replace("é", String(char(130)));
  S.replace("â", String(char(131)));
  S.replace("à", String(char(133)));
  S.replace("ç", String(char(135)));
  S.replace("ê", String(char(136)));
  S.replace("è", String(char(138)));
  S.replace("ù", String(char(151)));
  return S;
}
