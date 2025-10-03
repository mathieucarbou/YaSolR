#include "EcranLED.h"
LGFXoled oled;
void Init_LED_OLED(void) {
  //LEDs et OLRD
  //LEDgroupe = 0;   //0:pas de LED,1à 9 LEDs, 10 et 11 écran OLED SSD1306 , 12 et  13 OLED SH1106
  if (LEDgroupe > 0 && LEDgroupe < 10) {  //Simples LEDs
    pinMode(LEDyellow[LEDgroupe], OUTPUT);
    pinMode(LEDgreen[LEDgroupe], OUTPUT);
  }
  if (LEDgroupe >= 10) {
    bool SD = true;                                                     //SSD1306
    if (LEDgroupe > 11) SD = false;                                     //SH1106
    oled.LGFXoled_init(LEDyellow[LEDgroupe], LEDgreen[LEDgroupe], SD);  //SDA , SCL et SSD1306 ou SH1106
    // put your setup code here, to run once:
    oled.begin();
    oled.setRotation(2);
    oled.setBrightness(255);
    oled.setTextFont(1);
    oled.setTextColor(TFT_WHITE, TFT_BLACK);
    PrintCentreO("F1ATB", -1, 0, 2);
    oled.setScrollRect(0, 16, oled.width(), oled.height() - 16);
    oled.setTextScroll(true);
    delay(100);
  }
}

//****************
//* Gestion LEDs *
//****************
void Gestion_LEDs() {

  int retard_min = 100;
  int retardI;
  cptLEDyellow++;
  if ((WiFi.status() != WL_CONNECTED && ESP32_Type < 10) || (EthernetBug > 0 && ESP32_Type >= 10)) {  // Attente connexion au Wifi ou ethernet
    if (WiFi.getMode() == WIFI_STA) {                                                                 // en  Station mode
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


  if (LEDgroupe > 0 && LEDgroupe < 10) {
    int L = 0, H = 1;  //LED classique
    if (LEDgroupe == 2) {
      L = 1;
      H = 0;
    }
    if (cptLEDyellow > 5) {
      digitalWrite(LEDyellow[LEDgroupe], L);
    } else {
      digitalWrite(LEDyellow[LEDgroupe], H);
    }
    if (cptLEDgreen > 5) {
      digitalWrite(LEDgreen[LEDgroupe], L);
    } else {
      digitalWrite(LEDgreen[LEDgroupe], H);
    }
  }
  if (LEDgroupe >= 10) {

    if (cptLEDyellow == 5) {  //New puissance dispo
      oled.fillRect(0, 0, oled.width(), 16, TFT_BLACK);
      int Puissance = PuissanceS_M - PuissanceI_M;
      PrintCentreO(String(Puissance) + "W", -1, 0, 2);
      int teta = 360 * Puissance / 5000;
      if (Puissance >= 0) {
        oled.fillArc(8, 8, 0, 7, -90, teta - 90, TFT_WHITE);
      } else {
        oled.fillArc(8, 8, 0, 7, teta - 90, -90, TFT_WHITE);
      }
      PrintDroiteO(String(int(100 -retard_min )) + "%", oled.width(), 8, 1);
    }
    if (cptLEDyellow > 200) {
      oled.fillRect(0, 0, oled.width(), 16, TFT_BLACK);
      PrintCentreO("Puissance Inconnue", -1, 0, 2);
    }
  }
}

void PrintScroll(String m) {
  TelnetPrintln( m);
  if (LEDgroupe >= 10) {
    oled.setTextSize(1);
    oled.setTextScroll(true);
    oled.println(Ascii(m));
    oled.setTextScroll(false);
  }
  if (ESP32_Type == 4 && NumPage == 3) {  //Ecran LCD présent
    lcd.setTextSize(1);
    lcd.setTextScroll(true);
    lcd.println(Ascii(m));
    lcd.setTextScroll(false);
  }
}
void PrintCentreO(String S, int X, int Y, float Sz) {
  if (X < 0) X = oled.width() / 2;
  oled.setTextSize(Sz);
  int W = oled.textWidth(S);
  oled.drawString(S, X - W / 2, Y);
}
void PrintDroiteO(String S, int X, int Y, float Sz) {
  oled.setTextSize(Sz);
  int W = oled.textWidth(S);
  oled.drawString(S, X - W - 4, Y);
}