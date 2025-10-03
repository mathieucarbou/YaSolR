//***************************************************
// Page HTML et Javascript de gestion des Paramètres
//***************************************************
const char *ParaHtml = R"====(
  <!doctype html>
  <html lang="fr"><head><meta charset="UTF-8">
  <link rel="stylesheet" href="commun.css">
  <style>    
    .form {margin:auto;padding:5px;display: table;text-align:left;width:100%;}
    .form2 {margin:2px;padding:2px;display: block;width:100%;}
    .ligne {display: table-row;}
    .ligneF {display: table-row;background: rgba(50, 50, 50, 0.2);width:100%;}
    .ligne3 {display:flex;justify-content: center;padding:0px;margin: 0px;}
    .cadre {border-top:1px solid azure;}
    .canalT {display: table;}
    .canalTr {display: table-row;width:100%;}
    .canalTc {display: table-cell;padding:1px;text-align:center;width:100%;font-size:12px;}
    label,.nomR{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;width:70%;}
    input{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;}
    select{display: table-cell;margin: 5px;text-align:left;}
    .source label{display: table-cell;margin: 5px;text-align:left;font-size:12px;height:21px;width:initial;}
    .boldT{text-align:left;font-weight:bold;padding:10px;}   
    .Bparametres{border:inset 10px azure;}
    .Bgeneraux{border:inset 4px azure;}
    #BoutonsBas{text-align:center;}    
    #ligneFixe,.ligneTemperature,#ligneExt,#ligneEnphaseUser,#ligneEnphasePwd,#ligneEnphaseSerial,#infoIP,#ligneTopicP,#ligneTopicT{display:none;}
    .Zone,.generaux{width:100%;border 1px solid grey;border-radius:10px;margin-top:10px;background-color:rgba(30,30,30,0.3);} 
    #onglets2{display:block;} 
    .don{height:50px;display:block;text-align:center;} 
    #donEnv{display:inline-block;text-align:center;border:azure 3px outset; border-radius:18px;background-color:white;} 
    .donNone{display:none;} 
    .generaux{width:50%;text-align:center;margin:auto;}
    .generaux label{width:30%;}
    .fsize150{font-size:140%;}
    .topG{display:flex;justify-content: center;}
    .shem{font-size:12px;display:flex;}
    .tc{text-align:center;background: rgba(50, 50, 60, 0.2);padding:4px;}
  </style>
  <title>Params F1ATB</title>
  </head>
  <body onLoad="Init();">
    <div id="lesOnglets"></div>
    <div class="generaux">
      <div class="fsize150">Paramètres généraux</div>
      <div class="topG">
              <label for='ModeP0' style='text-align:right;'>Mode standard</label>
              <input type='radio' name='ModeP' id='ModeP0' value="0" checked onclick="checkDisabled();">
              <label for='ModeP1' style='text-align:right;'>Mode expert</label>
              <input type='radio' name='ModeP' id='ModeP1' value="1"  onclick="checkDisabled();">
      </div>
    </div>
    <div class="Zone">
        <div class="boldT">Routeur</div>
        <div class="form"  >
          <div class='ligne'>
            <label for='nomRouteur' >Nom du routeur : </label>
            <input type='text' name='nomRouteur' id='nomRouteur' >
          </div>
          <div class='ligne' id='ligneMobile'>
            <label for='nomSondeMobile' >Nom données puissance entrée de Maison : </label>
            <input type='text' name='nomSondeMobile' id='nomSondeMobile' >
          </div>
          <div class='ligne' id='ligneFixe'>
            <label for='nomSondeFixe' >Nom Données courant seconde sonde : </label>
            <input type='text' name='nomSondeFixe' id='nomSondeFixe' >
          </div>       
          <div class='ligne' id="l_wifi_0">
            <label for='TempoRTEon'>Affichage couleur Tempo RTE : </label>
            <input type='checkbox' name='TempoRTEon' id='TempoRTEon' style='width:25px;' ><span class='fsize10'>Nécessite un Reset de l'ESP32</span>
          </div>
          <div class='ligne' id="l_wifi_1">
            <label for='MQTTRepete'>Puissances et températures envoyées au serveur MQTT<br>Période (s) répétition  (0= pas d'envoi) : </label>
            <input type='number' name='MQTTRepete' id='MQTTRepete'  onclick="checkDisabled();" >
          </div>
          <div class='ligne' id="l_wifi_2">
            <label for='WifiSleep'>Wifi Sleep/veille <small>(réduit la consommation mais ralenti la communication)</small>: </label>
            <input type='checkbox' name='WifiSleep' id='WifiSleep' style='width:25px;' ><span class='fsize10'>Nécessite un Reset de l'ESP32</span>
          </div>
          <div class='ligne'>
             <label for='RAZdata'>Remise à zéro historique des mesures</label>
             <input  class='bouton'   id='RAZdata' type='button' onclick="SendRAZ();" value='RAZ' >
          </div>
        </div>
        <div>Configuration matérielle <span class='fsize10'>Nécessite un Reset de l'ESP32</span></div>
        <div class="form"  >
          <div class="ligne source">
            <label for='ESP0' style='text-align:right;'>ESP32 non défini</label>
            <input type='radio' name='pESP' id='ESP0' value="0" checked  onclick="checkDisabled();">
            <label for='ESP1' style='text-align:right;'>ESP32 Wroom seul</label>
            <input type='radio' name='pESP' id='ESP1' value="1"  onclick="checkDisabled();">
            <label for='ESP2' style='text-align:right;'>ESP32 carte 1 relais</label>
            <input type='radio' name='pESP' id='ESP2' value="2"  onclick="checkDisabled();">
            <label for='ESP3' style='text-align:right;'>ESP32 carte 4 relais</label>
            <input type='radio' name='pESP' id='ESP3' value="3"  onclick="checkDisabled();">
            <label for='ESP4' style='text-align:right;'>ESP32 Ecran 320*240</label>
            <input type='radio' name='pESP' id='ESP4' value="4"  onclick="checkDisabled();">
            <label for='ESP10' style='text-align:right;'>ESP32/WT32-ETH01</label>
            <input type='radio' name='pESP' id='ESP10' value="10"  onclick="checkDisabled();">
          </div>
          <div class="ligneF source" id="rotation">
            <label for='Rot0' style='text-align:right;'>Ecran portrait (0°)</label>
            <input type='radio' name='pRot' id='Rot0' value="0"  >
            <label for='Rot1' style='text-align:right;'>Ecran paysage (90°)</label>
            <input type='radio' name='pRot' id='Rot1' value="1"  >
            <label for='Rot2' style='text-align:right;'>Ecran portrait (180°)</label>
            <input type='radio' name='pRot' id='Rot2' value="2"  >
            <label for='Rot3' style='text-align:right;'>Ecran paysage (270°)</label>
            <input type='radio' name='pRot' id='Rot3' value="3" checked  >
          </div>
          <div class="ligneF source" id="dureeOn">
            <label for='Ec10000' style='text-align:right;'>Ecran On 10s</label>
            <input type='radio' name='pDurEcran' id='Ec10000' value="10000"  >
            <label for='Ec30000' style='text-align:right;'>Ecran On 30s</label>
            <input type='radio' name='pDurEcran' id='Ec30000' value="30000" checked >
            <label for='Ec300000' style='text-align:right;'>Ecran On 5mn</label>
            <input type='radio' name='pDurEcran' id='Ec300000' value="300000"  >
            <label for='Ec54000000' style='text-align:right;'>Ecran On 15h</label>
            <input type='radio' name='pDurEcran' id='Ec54000000' value="54000000"   >
          </div>
          
          <div class="ligne source">
            <label for='Serial0' style='text-align:right;'>Pas de port série 2</label>
            <input type='radio' name='pSerie' id='Serial0' value="0"  checked >
            <label for='Serial1' style='text-align:right;'>Port RX=gpio 16, TX=gpio 17</label>
            <input type='radio' name='pSerie' id='Serial1' value="1"  >
            <label for='Serial2' style='text-align:right;'>Port RX=gpio 26, TX=gpio 27</label>
            <input type='radio' name='pSerie' id='Serial2' value="2"  >
            <label for='Serial3' style='text-align:right;'>Port RX=gpio 18, TX=gpio 19</label>
            <input type='radio' name='pSerie' id='Serial3' value="3"  >
            <label for='Serial4' style='text-align:right;'>Port RX=gpio 5, TX=gpio 17</label>
            <input type='radio' name='pSerie' id='Serial4' value="4"  >
          </div>
          <div class="ligne source" id="Analog">
            <label for='pUxI0' style='text-align:right;'>Pas d'entrée Analogique</label>
            <input type='radio' name='pUxI' id='pUxI0' value="0"  checked >
            <label for='pUxI1' style='text-align:right;'>Commun(35), U(32) , I(33)</label>
            <input type='radio' name='pUxI' id='pUxI1' value="1"  >
            <label for='pUxI2' style='text-align:right;'>Commun(35), U(32) , I(34)</label>
            <input type='radio' name='pUxI' id='pUxI2' value="2"  >
            <label for='pUxI3' style='text-align:right;'>Commun(34), U(32) , I(33)</label>
            <input type='radio' name='pUxI' id='pUxI3' value="3"  >
            <label for='pUxI4' style='text-align:right;'>Commun(35), U(36) , I(39)</label>
            <input type='radio' name='pUxI' id='pUxI4' value="4"  >
          </div>
          <div class="ligne source">
            <label for='Triac0' style='text-align:right;'>Pas de Triac</label>
            <input type='radio' name='pTriac' id='Triac0' value="0" checked  >
            <label for='Triac1' style='text-align:right;'>Triac pulse=gpio 4, Zc=gpio 5</label>
            <input type='radio' name='pTriac' id='Triac1' value="1"  >
            <label for='Triac2' style='text-align:right;'>Triac pulse=gpio 22, Zc=gpio 23</label>
            <input type='radio' name='pTriac' id='Triac2' value="2"  >
            <label for='Triac3' style='text-align:right;'>Triac pulse=gpio 21, Zc=gpio 22</label>
            <input type='radio' name='pTriac' id='Triac3' value="3"  >
            <label for='Triac4' style='text-align:right;'>Triac pulse=gpio 12, Zc=gpio 14</label>
            <input type='radio' name='pTriac' id='Triac4' value="4"  >
          </div>
        </div>
        <div class="form"  >
          
          <div class="ligneF source">
            <label for='LED0' style='text-align:right;'>Pas de LED / OLED</label>
            <input type='radio' name='pLED' id='LED0' value="0"  >
            <label for='LED1' style='text-align:right;'>LEDs  gpio 18, gpio 19</label>
            <input type='radio' name='pLED' id='LED1' value="1"  checked>
            <label for='LED2' style='text-align:right;'>LEDs gpio 4, gpio 16</label>
            <input type='radio' name='pLED' id='LED2' value="2"   >
            <label for='LED3' style='text-align:right;'>LEDs gpio 2, gpio 4</label>
            <input type='radio' name='pLED' id='LED3' value="3"  >
          </div>
          <div class="ligneF source">
            <label for='LED10' style='text-align:right;'>SSD1306/1309 OLED<br>sda=gpio 18, scl=gpio 19</label>
            <input type='radio' name='pLED' id='LED10' value="10"  >
            <label for='LED11' style='text-align:right;'>SSD1306/1309 OLED<br>sda=gpio 4, scl=gpio 32</label>
            <input type='radio' name='pLED' id='LED11' value="11"  >
            <label for='LED12' style='text-align:right;'>SH1106 OLED<br>sda=gpio 18, scl=gpio 19</label>
            <input type='radio' name='pLED' id='LED12' value="12"  >
            <label for='LED13' style='text-align:right;'>SH1106 OLED<br>sda=gpio 4, scl=gpio 32</label>
            <input type='radio' name='pLED' id='LED13' value="13"  >
          </div>
        </div>
        <div class="form"  >
          <div class="ligne source" id="pTemp">
            <label for='pTemp0' style='text-align:right;'>Pas de capteur de température</label>
            <input type='radio' name='pTemp' id='pTemp0' value="0"  >
            <label for='pTemp1' style='text-align:right;'>DS18B20 gpio 13</label>
            <input type='radio' name='pTemp' id='pTemp1' value="1" checked   >
            <label for='pTemp2' style='text-align:right;'>DS18B20 gpio 27</label>
            <input type='radio' name='pTemp' id='pTemp2' value="2"  >
            <label for='pTemp3' style='text-align:right;'>DS18B20 gpio 33</label>
            <input type='radio' name='pTemp' id='pTemp3' value="3"  >
          </div>
        </div>
    </div>
    <div class="Zone">
        <div class="boldT">Accès réseau et adresse IP de l'ESP32 du Routeur</div>
        <div class="form"  >
          <div class="ligne source">           
              <label for='ModeW0' style='text-align:right;'>Accès Internet</label><input type='radio' name='ModeW' id='ModeW0' value="0"  checked onclick="checkDisabled();" >
              <label for='ModeW1' style='text-align:right;'>Accès réseau local uniquement</label><input type='radio' name='ModeW' id='ModeW1' value="1" onclick="checkDisabled();"  >
              <label for='ModeW2' style='text-align:right;'>Point d'Accès isolé</label><input type='radio' name='ModeW' id='ModeW2' value="2" onclick="checkDisabled();"  >
          </div>
        </div>
        <div class="form"  >
          <div class='ligne'>
            <label for='dhcp'>Adresse IP <span id='localIP'><span> : </label>
            <input type='checkbox' name='dhcp' id='dhcp' style='width:25px;' onclick="checkDisabled();"><span id="ipreset" class='fsize10'>Nécessite un Reset de l'ESP32</span>
          </div>
        </div>
        <div class="form"  id="infoIP">
          <div class='ligne'>
            <label for='adrIP'>Adresse IP si fixe (ex : 192.168.1.245) : <br><span class='fsize10'>N&eacute;cessite un Reset de l'ESP32</span></label>
            <input type='text' name='adrIP' id='adrIP' >
          </div>
          <div class='ligne'>
            <label for='gateway'>Passerelle / Gateway (ex : 192.168.1.254) :  <br><span class='fsize10'>En g&eacute;n&eacute;ral l'adresse de votre box internet</span></label>
            <input type='text' name='gateway' id='gateway' >
          </div>
          <div class='ligne'>
            <label for='masque'>Masque / Subnet (ex : 255.255.255.0) :  </label>
            <input type='text' name='masque' id='masque' >
          </div>
          <div class='ligne'>
            <label for='dns'>DNS (ex : 192.168.1.254) :  <br><span class='fsize10'>En g&eacute;n&eacute;ral l'adresse de votre box internet</span></label>
            <input type='text' name='dns' id='dns' >
          </div>
        </div>
    </div>
    <div class="Zone">
      <div class="boldT">Horloge du Routeur</div>
      <div class="form"  >
        <div class="ligne source">        
            <label for='Hor0' style='text-align:right;' id='Hor0L'>Internet</label><input type='radio' name='Horlo' id='Hor0' value="0" checked  onclick="checkDisabled();"  >
            <label for='Hor1' style='text-align:right;'>Linky</label><input type='radio' name='Horlo' id='Hor1' value="1" onclick="checkDisabled();"  >
            <label for='Hor2' style='text-align:right;'>Interne</label><input type='radio' name='Horlo' id='Hor2' value="2" onclick="checkDisabled();"  >
            <label for='Hor3' style='text-align:right;'>IT 10ms/100Hz (Triac)</label><input type='radio' name='Horlo' id='Hor3' value="3" onclick="checkDisabled();"  >
            <label for='Hor4' style='text-align:right;'>IT 20ms/50Hz</label><input type='radio' name='Horlo' id='Hor4' value="4" onclick="checkDisabled();"  >                 
        </div>
      </div>
    </div>
    <div class="Zone" id="SurvCom">
      <div class="boldT">Surveillance Communications du Routeur <span class="fsize10"> Reset si coupure de : </span></div>
      <div class="form"  >
        <div class="ligne source">        
            <label for='ComSurv6' style='text-align:right;' >3mn</label><input type='radio' name='ComSurv' id='ComSurv6' value="6" checked  >
            <label for='ComSurv60' style='text-align:right;'>30mn</label><input type='radio' name='ComSurv' id='ComSurv60' value="60" >
            <label for='ComSurv360' style='text-align:right;'>3h</label><input type='radio' name='ComSurv' id='ComSurv360' value="360" >
            <label for='ComSurv1440' style='text-align:right;'>12h</label><input type='radio' name='ComSurv' id='ComSurv1440' value="1440" >
            <label for='ComSurv2880' style='text-align:right;'>24h</label><input type='radio' name='ComSurv' id='ComSurv2880' value="2880" >                 
        </div>
      </div>
    </div>
    <div class="Zone" id="listerouteurs">
        <div class="boldT">Liste des Routeurs en réseau <span class="fsize10">Même version de routeur recommandée</span></div>
        <div class="form" id="Routeurs"></div>
    </div>
    <div class="Zone">
        <div class="boldT">Source des mesures de puissance</div>
        <div class="form"  > 
          <div class="ligne source">
            <label for='NotDef' style='text-align:right;'>Source non définie</label>
            <input type='radio' name='sources' id='NotDef' value="NotDef" checked   onclick="checkDisabled();">
            <label for='UxI' style='text-align:right;'>UxI</label>
            <input type='radio' name='sources' id='UxI' value="UxI"  onclick="checkDisabled();">
            <label for='UxIx2' style='text-align:right;'>UxIx2</label>
            <input type='radio' name='sources' id='UxIx2' value="UxIx2"  onclick="checkDisabled();">   
            <label for='UxIx3' style='text-align:right;'>UxIx3</label>
            <input type='radio' name='sources' id='UxIx3' value="UxIx3"  onclick="checkDisabled();">       
            <label for='Linky' style='text-align:right;'>Linky</label>
            <input type='radio' name='sources' id='Linky' value="Linky"  onclick="checkDisabled();">
            <span id="avec_wifi">
              <label for='Enphase' style='text-align:right;'>Enphase-Envoy</label>
              <input type='radio' name='sources' id='Enphase' value="Enphase"  onclick="checkDisabled();">
              <label for='SmartG' style='text-align:right;'>SmartGateways</label>
              <input type='radio' name='sources' id='SmartG' value="SmartG"  onclick="checkDisabled();">
              <label for='HomeW' style='text-align:right;'>HomeWizard</label>
              <input type='radio' name='sources' id='HomeW' value="HomeW"  onclick="checkDisabled();">
              <label for='ShellyEm' style='text-align:right;'>Shelly Em</label>
              <input type='radio' name='sources' id='ShellyEm' value="ShellyEm"  onclick="checkDisabled();">
              <label for='ShellyPro' style='text-align:right;'>Shelly Pro Em</label>
              <input type='radio' name='sources' id='ShellyPro' value="ShellyPro"  onclick="checkDisabled();">
              <label for='Ext' style='text-align:right;'>ESP Externe</label>
              <input type='radio' name='sources' id='Ext' value="Ext"  onclick="checkDisabled();">
              <label for='Pmqtt' style='text-align:right;' id='PmqttL'>MQTT</label>
              <input type='radio' name='sources' id='Pmqtt' value="Pmqtt"  onclick="checkDisabled();">
            </span>
          </div>
        </div>
        <div class="form"  >
          <div class='ligne' id="ligneExt">
            <label for='RMSextIP'>Adresse IP <span id='labExtIp'></span> externe (ex : 192.168.1.248) : </label>
            <input type='text' name='RMSextIP' id='RMSextIP' autocomplete='on'>
          </div>
          <div class='ligne' id="ligneEnphaseUser">
            <label for='EnphaseUser'>Enphase Envoy-S metered User : <span class='fsize10'><br>Pour firmvare Envoy-S V7 seulement</span></label>
            <input type='text' name='EnphaseUser' id='EnphaseUser' autocomplete='on'>
          </div>
          <div class='ligne' id="ligneEnphasePwd">
            <label for='EnphasePwd'>Enphase Envoy-S metered Password : <span class='fsize10'><br>Pour firmvare Envoy-S V7 seulement</span></label>
            <input type='password' name='EnphasePwd' id='EnphasePwd' autocomplete='on'>
          </div>
          <div class='ligne' id="ligneEnphaseSerial">
            <label for='EnphaseSerial' id="label_enphase_shelly"></label>
            <input type='text' name='EnphaseSerial' id='EnphaseSerial' onchange='checkDisabled();' autocomplete='on'>
          </div>
          <div class='ligne' id="ligneTopicP">
            <label for='TopicP' >MQTT Topic Puissance :</label>
            <input type='text' name='TopicP' id='TopicP' autocomplete='on'>
          </div>
          <div><span class='fsize10'>Nécessite un Reset de l'ESP32</span></div>
        </div>
        <div id="CACSI" class="form">             
            <div class='ligne'>
              <label for='EstimCACSI'>Estimateur injection si CACSI: </label>
              <input type='checkbox' name='EstimCACSI' id='EstimCACSI' style='width:25px;' onclick="checkDisabled();">
            </div>
        </div> 
    </div>
    <div class="Zone" id="LesSourcesTemp">
        <div class="boldT">Source des mesures de température</div>
        <div class="form"  >
          <div id="Sources_Temp"></div>
        </div>        
    </div>
    
    <div class="Zone" id="Zmqtt">
        <div class="boldT">Paramètres serveur MQTT <small>(Home Assistant , Domoticz ...)</small></div>
        <div class="form"  >  
          <div class='ligne'>
            <label for='MQTTIP'>Adresse IP host MQTT (ex : 192.168.1.18) : </label>
            <input type='text' name='MQTTIP' id='MQTTIP' >
          </div>
          <div class='ligne'>
            <label for='MQTTPort'> port (ex : 1883) : </label>
            <input type='number' name='MQTTPort' id='MQTTPort' >
          </div>
          <div class='ligne'>
            <label for='MQTTUser'>MQTT User nom : </label>
            <input type='text' name='MQTTUser' id='MQTTUser' autocomplete='on'>
          </div>
          <div class='ligne'>
            <label for='MQTTpwd'>MQTT mot de passe : </label>
            <input type='password' name='MQTTpwd' id='MQTTpwd'  autocomplete='on'>
          </div>
          <div class='ligne'>
            <label for='MQTTPrefix'>MQTT Préfixe de découverte (1 seul mot ex : homeassistant ) : </label>
            <input type='text' name='MQTTPrefix' id='MQTTPrefix' >
          </div>
          <div class='ligne'>
            <label for='MQTTPrefixEtat'>MQTT Préfixe de publication (1 seul mot ex : homeassistant ) : </label>
            <input type='text' name='MQTTPrefixEtat' id='MQTTPrefixEtat' >
          </div>
          <div class='ligne'>
            <label for='MQTTdeviceName'>MQTT Device Name / Nom routeur (1 seul mot ex : routeur_rms ) : </label>
            <input type='text' name='MQTTdeviceName' id='MQTTdeviceName' >
          </div>
          <div class='ligne'>
            <label for='subMQTT'>Souscrire au forçage des Actions via le serveur MQTT: </label>
            <input type='checkbox' name='subMQTT' id='subMQTT' style='width:25px;' onclick='checkDisabled();' >
          </div>
        </div>
    </div>
    <div class="Zone" id="Zcalib">
        <div class="boldT" id='Tui'>Calibration Mesures Ueff et Ieff <small>(UxI)</small></div>
        <div class="form"  >    
          <div class='ligne' id='CUi'>
            <label for='CalibU'>Coefficient multiplicateur Ueff (typique : 1000) : </label>
            <input type='number' name='CalibU' id='CalibU'   >
          </div>
          <div class='ligne' id='CuI'>
            <label for='CalibI'>Coefficient multiplicateur Ieff (typique : 1000) : </label>
            <input type='number' name='CalibI' id='CalibI'   >
          </div>
        </div>
    </div>
    <div class="Zone" >
        <div class="boldT" >Sécurité d'accès aux paramètres et Actions</div>
        <div class="form"  >    
          <div class='ligne'>
            <label for='CleAcces'>Définissez un mot de passe d'accès <span class='fsize10'>Pas de ";" Ne rien mettre si accès libre</span> : </label>
            <input type='text' name='CleAcces' id='CleAcces'>
          </div>
        </div>
    </div>
    <div  id='BoutonsBas'>        
        <br><input  class='bouton' type='button' onclick="SendValues();" value='Sauvegarder' >
        <div class="lds-dual-ring" id="attente"></div>
        <input  class='bouton' type='button' onclick='Reset();' value='ESP32 Reset' >
    </div>
    <br>
    <div id='pied'></div>
    <br>
    <div id="donEnv" >
      <form action="https://www.paypal.com/donate" method="post" target="_top">
        <input type="hidden" name="hosted_button_id" value="Z35E9D5D9N9DN" >
        <input class="don" type="image" src="https://pics.paypal.com/00/s/MGY1NzdhY2YtYTRkNi00YzIwLWI2YzQtNWI3YjM3ZmFiNWUx/file.PNG" style="border:0px;" name="submit" title="PayPal - The safer, easier way to pay online!" alt="Bouton Faites un don avec PayPal" >
        <img alt="" class="donNone" style="border:0px;" src="https://www.paypal.com/fr_FR/i/scr/pixel.gif" width="1" height="1" />
      </form>

    </div>
    <script src="/ParaRouteurJS"></script>
    <script src="/ParaJS"></script>
    <script src="/CommunCouleurJS"></script>
  </body></html>
)====";
const char *ParaJS = R"====(
  var refTempIP=[];
   var BordsInverse=[".Bparametres",".Bgeneraux"];
  function Init(){
    var S="";
    for (var i=0;i<4;i++){

        S +="<div class='form cadre'>";
          S +="<div class='canalT'><div class='canalTr'><div class='canalTc'>Canal : "+i+"</div></div></div>";
          S +="<div class='ligne source'>";
            S +="<label for='tempNo" + i + "' style='text-align:right;'>Pas de mesure</label>";
            S +="<input type='radio' name='srcTemp" + i + "' id='tempNo" + i + "' value='tempNo'  onclick='checkDisabled();' checked>";
            S +="<label for='tempInt" + i + "' style='text-align:right;'>Capteur DS18B20 Interne</label>";
            S +="<input type='radio' name='srcTemp" + i + "' id='tempInt" + i + "' value='tempInt'  onclick='checkDisabled();'>  ";
            S +="<span id='siWifi"+i+"'>";
              S +="<label for='tempExt" + i + "' style='text-align:right;'>Capteur DS18B20 ESP Externe</label>";
              S +="<input type='radio' name='srcTemp" + i + "' id='tempExt" + i + "' value='tempExt'  onclick='checkDisabled();'>";       
              S +="<label for='tempMqtt" + i + "' style='text-align:right;'>MQTT</label>";
              S +="<input type='radio' name='srcTemp" + i + "' id='tempMqtt" + i + "' value='tempMqtt'  onclick='checkDisabled();'>";
            S +="</span>";
          S +="</div>";
        S +="</div>";
        S +="<div class='form ligneTemperature' id='ligneTemperature" + i + "' >";
          S +="<div class='ligne' >";
            S +="<label for='nomTemperature" + i + "' >Nom Température : </label>";
            S +="<input type='text' name='nomTemperature" + i + "' id='nomTemperature" + i + "' >";
          S +="</div>";
          S +="<div class='ligne' id='ligneoffsetTemp" + i + "'>";
            S +="<label for='offsetTemp" + i + "' >Offset mesure température (°C) :</label>";
            S +="<input type='number' name='offsetTemp" + i + "' id='offsetTemp" + i + "' >";
          S +="</div>";
          S +="<div class='ligne' id='ligneTopicT" + i + "'>";
            S +="<label for='TopicT" + i + "' >MQTT Topic Température :</label>";
            S +="<input type='text' name='TopicT" + i + "' id='TopicT" + i + "' >";
          S +="</div>";
          S +="<div class='ligne' id='ligneIPtemp" + i + "'>";
            S +="<label for='refTempIP" + i + "' >Adresse IP ESP-RMS externe :</label>";
            S +="<select  id='refTempIP" + i + "' ></select>";
          S +="</div>";
          S +="<div class='ligne' id='canalEXTtemp" + i + "'>";
            S +="<label for='canalEXT" + i + "' >Canal Température ESP-RMS externe :</label>";
            S +="<select id='canalEXT" + i +"'><option value=0>Canal 0</option><option value=1>Canal 1</option><option value=2>Canal 2</option><option value=3>Canal 3</option></select>";
          S +="</div>";
          
        S +="</div>";

    }
    GH("Sources_Temp",S);
    S="<div class='ligne'><div class='nomR'><strong>Nom</strong></div><div class='nomR'><strong>Adresse IP</strong></div></div>";
    for (var i=0;i<8;i++){
        S +="<div class='ligne' id='Routeur_" + i + "' style='display:none;' >";
        var bg="";
        if (i==0) bg="style='background-color:#bbb;'";
          S +="<div id='RMS_Nom" + i +"'  class='nomR'></div><input type='text' id='RMS_IP" + i +"' " + bg + ">";
        S +="</div>";
    }
    GH("Routeurs",S);

    SetHautBas();
    LoadParametres();
    LoadParaRouteur();
    LoadCouleurs();
  }
  function LoadParametres() {
    //Cookie clé d'accès
    var name="CleAcces=";
    var decodedCookie = decodeURIComponent(document.cookie);
    var ca = decodedCookie.split(';');
    for(var i = 0; i < ca.length; i++) {
      var c = ca[i];
      while (c.charAt(0) == ' ') {
        c = c.substring(1);
      }
      if (c.indexOf(name) == 0)       GID("CleAcces").value = c.substring(name.length, c.length);
    }


    var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
             var BlocsParas=this.responseText;
             var LesParas=BlocsParas.split(GS);
             var Para=LesParas[0].split(RS);  
             GID("dhcp").checked = Para[0]==1  ? true:false;
             GID("adrIP").value=int2ip(Para[1]);
             GID("gateway").value=int2ip(Para[2]);
             GID("masque").value=int2ip(Para[3]);
             GID("dns").value=int2ip(Para[4]);
             GID("ModeP"+Para[5]).checked = true;
             ModePara = Para[5];
             GID("ModeW"+Para[6]).checked = true;
             GID("Hor"+Para[7]).checked = true;
             GID(Para[8]).checked = true;
             GID("RMSextIP").value=int2ip(Para[9]); 
             GID("EnphaseUser").value=Para[10];
             GID("EnphasePwd").value=Para[11];
             GID("EnphaseSerial").value=Para[12];
             GID("TopicP").value=Para[13]; 
             GID("MQTTRepete").value = Para[14];
             GID("MQTTIP").value=int2ip(Para[15]);
             GID("MQTTPort").value=Para[16];
             GID("MQTTUser").value=Para[17];
             GID("MQTTpwd").value=Para[18];
             GID("MQTTPrefix").value=Para[19];
             GID("MQTTPrefixEtat").value=Para[20];
             GID("MQTTdeviceName").value=Para[21];
             GID("subMQTT").checked = Para[22]==1  ? true:false; 
             GID("nomRouteur").value=Para[23];
             GID("nomSondeFixe").value=Para[24];
             GID("nomSondeMobile").value=Para[25];
             GID("CalibU").value=Para[26];
             GID("CalibI").value=Para[27];
             GID("TempoRTEon").checked = Para[28]==1  ? true:false;
             GID("WifiSleep").checked = Para[29]==1  ? true:false;
             GID("ComSurv" + Para[30]).checked = true;
             GID("Serial" + Para[31]).checked = true;
             GID("Triac" + Para[32]).checked = true;
             GID("ESP" + Para[33]).checked = true;
             GID("LED" + Para[34]).checked = true;
             GID("Rot" + Para[35]).checked = true;
             GID("Ec" + Para[36]).checked = true;
             GID("pUxI" + Para[37]).checked = true;
             GID("pTemp" + Para[38]).checked = true;
             GID("EstimCACSI").checked = Para[39]==1  ? true:false;
             for (var c=0;c<4;c++){
                var ParaT=LesParas[c+1].split(RS);
                GID("nomTemperature"+c).value=ParaT[0];
                GID(ParaT[1]+c).checked = true; 
                GID("TopicT"+c).value=ParaT[2]; 
                refTempIP[c]=ParaT[3];
                GID("canalEXT"+c).value=ParaT[4]; 
                GID("offsetTemp"+c).value=ParaT[5]/100; 
             }
             
             checkDisabled();
          }         
        };
        xhttp.open('GET', '/ParaAjax', true);
        xhttp.send();
      }
  function SendValues(){
    GID("attente").style="visibility: visible;";
    var dhcp = GID("dhcp").checked ? 1:0;
    var TempoRTEon = GID("TempoRTEon").checked ? 1:0;
    var Source_new = document.querySelector('input[name="sources"]:checked').value;
    var ComSurv = document.querySelector('input[name="ComSurv"]:checked').value;
    var subMQTT = GID("subMQTT").checked ? 1:0;
    var WifiSleep = GID("WifiSleep").checked ? 1:0;
    var Serial = document.querySelector('input[name="pSerie"]:checked').value;
    var pTriac = document.querySelector('input[name="pTriac"]:checked').value;
    ModePara = document.querySelector('input[name="ModeP"]:checked').value;
    ModeReseau = document.querySelector('input[name="ModeW"]:checked').value;
    Horloge = document.querySelector('input[name="Horlo"]:checked').value;
    var pESP  = document.querySelector('input[name="pESP"]:checked').value;
    var pLED  = document.querySelector('input[name="pLED"]:checked').value;
    var pRot  = document.querySelector('input[name="pRot"]:checked').value;
    var pDurEcran  = document.querySelector('input[name="pDurEcran"]:checked').value;
    var pUxI  = document.querySelector('input[name="pUxI"]:checked').value;
    var pTemp  = document.querySelector('input[name="pTemp"]:checked').value;
    var EstimCACSI = GID("EstimCACSI").checked ? 1:0;
    if (Source_new!="Linky") EstimCACSI=0;
    if (ModePara==0) { //Non Expert
        subMQTT=0; WifiSleep=1;
    }
    var S=dhcp+RS+ ip2int(GID("adrIP").value)+RS+ ip2int(GID("gateway").value);
    S +=RS+ip2int(GID("masque").value)+RS+ ip2int(GID("dns").value) +RS +ModePara + RS +ModeReseau+ RS +Horloge;
    S +=RS+Source_new+RS+ ip2int(GID("RMSextIP").value)+ RS+GID("EnphaseUser").value.trim()+RS+GID("EnphasePwd").value.trim()+RS+GID("EnphaseSerial").value.trim() +RS+GID("TopicP").value.trim();
    S +=RS+GID("MQTTRepete").value +RS+ip2int(GID("MQTTIP").value) +RS+GID("MQTTPort").value +RS+GID("MQTTUser").value.trim()+RS+GID("MQTTpwd").value.trim();
    S +=RS+GID("MQTTPrefix").value.trim()+RS+GID("MQTTPrefixEtat").value.trim()+RS+GID("MQTTdeviceName").value.trim() + RS + subMQTT;
    S +=RS+GID("nomRouteur").value.trim()+RS+GID("nomSondeFixe").value.trim()+RS+GID("nomSondeMobile").value.trim();
    S +=RS+GID("CalibU").value+RS+GID("CalibI").value + RS + TempoRTEon + RS + WifiSleep + RS + ComSurv + RS + Serial + RS + pTriac;
    S +=RS + pESP + RS + pLED +RS + pRot + RS + pDurEcran + RS + pUxI + RS + pTemp+ RS + EstimCACSI;
    for (var c=0;c<4;c++){
      var QS='input[name="srcTemp' + c + '"]:checked';
      var Source_Temp = (ModePara==0) ? "tempNo":document.querySelector(QS).value;
      S +=RS+GID("nomTemperature"+c).value.trim() + RS + Source_Temp  +RS + GID("TopicT"+c).value.trim() + RS + GID("refTempIP"+c).value + RS + GID("canalEXT"+c).value + RS + GID("offsetTemp"+c).value*100;
    }
    for (var i=0;i<8;i++){
        var Vip =(ModePara==0) ? 0:ip2int(GID("RMS_IP"+i).value.trim());
        S +=RS+ Vip;
    }
    S="?lesparas="+clean(S);
    document.cookie="CleAcces=" + GID("CleAcces").value.trim() ;
    if ((GID("dhcp").checked ||  checkIP("adrIP")&&checkIP("gateway"))   && (!GID("MQTTRepete").checked ||  checkIP("MQTTIP"))){
      var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            var retour=this.responseText;
            location.reload();
          }         
        };
        xhttp.open('GET', 'ParaUpdate'+S, true);
        xhttp.send();
    }
  }
  function SendRAZ(){
    if (confirm("Confirmez la remise à zéro des historiques") ) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            var retour=this.responseText;
          }         
        };
        xhttp.open('GET', '/ajaxRAZhisto', true);
        xhttp.send();
    } 
  }
  function checkDisabled(){ 
    GID("RMS_IP0").style.backgroundColor ="#888";
    GID("RMS_IP0").readOnly =true;
    ModeReseau = document.querySelector('input[name="ModeW"]:checked').value;
    ModePara = document.querySelector('input[name="ModeP"]:checked').value;
    Horloge = document.querySelector('input[name="Horlo"]:checked').value;
    var pESP = document.querySelector('input[name="pESP"]:checked').value;
    GID("Bheure").style.display= (Horloge>1) ? "inline-block": "none";
    GID("Bwifi").style.display= (ESP32_Type<10) ? "inline-block": "none";
    GID("infoIP").style.display = (GID("dhcp").checked || ModeReseau==2) ? "none" : "table";
    GID("dhcp").style.visibility= (ModeReseau==2) ? "hidden" : "visible";
    GID("ipreset").style.display= (ModeReseau==2) ? "none" : "inherit";
    GID("rotation").style.display= (pESP==4) ? "table-row" : "none";
    GID("dureeOn").style.display= (pESP==4) ? "table-row" : "none";
    GID("l_wifi_0").style.display= (ModeReseau==2) ? "none": "table-row";
    GID("l_wifi_1").style.display= (ModeReseau==2 || ModePara==0) ? "none": "table-row";
    GID("l_wifi_2").style.display= (ModeReseau==2 || ModePara==0 || ESP32_Type>=10) ? "none": "table-row";
    GID("Pmqtt").style.display= ( ModePara==0) ? "none": "table-cell";
    GID("PmqttL").style.display= ( ModePara==0) ? "none": "table-cell";
    GID("listerouteurs").style.display= (ModeReseau==2  || ModePara==0) ? "none": "block";
    GID("avec_wifi").style.visibility= (ModeReseau==2) ? "hidden" : "visible";
    for (var i=0;i<4;i++){
        GID("siWifi"+i).style.visibility= (ModeReseau==2) ? "hidden" : "visible";
    }
    GID("donEnv").style.display= (ModeReseau==2) ? "none": "inline-block;";
    if (ModeReseau==2){
      GH("localIP"," : 192.168.4.1");
    } else {
      GH("localIP","auto (DHCP) : " + LocalIP);
    }
    GID("Hor0L").style.textDecoration=(ModeReseau>0) ? "line-through" :"none";
    if (ModeReseau>0 && Horloge==0) {GID("Hor2").checked = true;Horloge=2;}
    GID("Zmqtt").style.display = ((GID("MQTTRepete").value != 0  || GID("Pmqtt").checked ||  GID("subMQTT").checked) && ModePara>0) ? "block" : "none";
    GID("LesSourcesTemp").style.display= (ModePara==0) ? "none": "block";
    GID("pTemp").style.display= (ModePara==0) ? "none": "table-row";
    GID("SurvCom").style.display= (ModePara==0) ? "none": "block";
    for (var c=0;c<4;c++){
      GID('ligneTemperature'+c).style.display = (GID("tempNo"+c).checked) ? "none" : "table";
      GID('ligneTopicT'+c).style.display = (GID("tempMqtt"+c).checked) ? "table-row" : "none";
      GID('ligneIPtemp'+c).style.display = (GID("tempExt"+c).checked) ? "table-row" : "none";  
      GID('canalEXTtemp'+c).style.display = (GID("tempExt"+c).checked) ? "table-row" : "none"; 
      GID('ligneoffsetTemp'+c).style.display = (GID("tempInt"+c).checked) ? "table-row" : "none"; 
      if (GID("tempMqtt"+c).checked )  GID("Zmqtt").style.display ="table"; 
    }
    GID('ligneTopicP').style.display = (GID("Pmqtt").checked) ? "table-row" : "none";
    Source = document.querySelector('input[name="sources"]:checked').value;
    if (Source !='Ext') Source_data=Source;
    AdaptationSource();
  }
  function checkIP(id){
    var S=GID(id).value;
    var Table=S.split(".");
    var valide=true;
    if (Table.length!=4) {
      valide=false;
    }else{
      for (var i=0;i<Table.length;i++){
        if (Table[i]>255 || Table[i]<0) valide=false;
      }
    }
    if (valide){
      GID(id).style.color="black";
    } else {
      GID(id).style.color="red";
    }
    return valide;
  }
 
  
  
  function AdaptationSource(){
      GID('ligneFixe').style.display = (Source_data=='UxIx2' || ((Source_data=='ShellyEm' || Source_data=='ShellyPro') && GID("EnphaseSerial").value !=3 ))? "table-row" : "none";
      GID('Zcalib').style.display=(Source_data=='UxI' && Source=='UxI' ) ? "table" : "none";
      GID('Analog').style.display=(Source_data=='UxI' && Source=='UxI' ) ? "table-row" : "none";
      GID('CACSI').style.display=( Source=='Linky' ) ? "table" : "none";
      var txtExt = "ESP-RMS";
      if (Source=='Enphase') txtExt = "Enphase-Envoy";
      if (Source=='SmartG') txtExt = "SmartGateways";
      if (Source=='HomeW') txtExt = "HomeWizard";
      var lab_enphaseShelly= "Numéro série passerelle IQ Enphase : <span class='fsize10'><br>Pour firmvare Envoy-S V7 seulement</span>";
      if (Source=='ShellyEm' || Source=='ShellyPro') {
        txtExt = "Shelly (Pro) Em ";
        lab_enphaseShelly="<div class='shem'><strong>Shelly (Pro) Em</strong><br>";
        lab_enphaseShelly +="Monophasé : Courant maison sur voie 0,1 ou 2<br>Triphasé : mettre 3";
        lab_enphaseShelly +="</div><div class='shem'><Strong>Shelly Em Gen3</strong><br>";
        lab_enphaseShelly +="Courant maison sur voie 0 = 30, voie 1 = 31</div>";
      }
      GID('labExtIp').innerHTML = txtExt;
      GID('label_enphase_shelly').innerHTML = lab_enphaseShelly;
      GID('ligneExt').style.display = (Source=='Ext' || Source=='Enphase' || Source=='SmartG' || Source=='HomeW'|| Source=='ShellyEm' || Source=='ShellyPro') ? "table-row" : "none";
      GID('ligneEnphaseUser').style.display = (Source=='Enphase') ? "table-row" : "none";
      GID('ligneEnphasePwd').style.display = (Source=='Enphase') ? "table-row" : "none";
      GID('ligneEnphaseSerial').style.display = (Source=='Enphase' || Source=='ShellyEm' || Source=='ShellyPro') ? "table-row" : "none"; //Numéro de serie ou voie
  }
  function FinParaRouteur(){
    var Soptions="";
    for (var c=0;c<nb_ESP;c++){
      GID("RMS_IP"+c).value =IP_RMS[c];
      GH("RMS_Nom"+c,nomRMS[c]);
      GID("Routeur_"+c).style="display: table-row";
      if (c>0){
        Soptions +="<option value=" +c +" >"+ IP_RMS[c] + " "+nomRMS[c] +"</option>";
      }
      var d=c+1;
      if (c<7) {
        GH("RMS_Nom"+d,"<small>Ajoutez une adresse IP de routeur ----></small>");
        GID("Routeur_"+d).style="display: table-row";
      }
    }
    
    for (var i=0;i<4;i++){      
      GH("refTempIP"+i,Soptions);
      GID("refTempIP"+i).value = refTempIP[i];
    }
     setTimeout('checkDisabled();',500);
                
  }
)====";

//Paramètres du routeur et fonctions générales pour toutes les pages.
const char *ParaRouteurJS = R"====(
  var Source="";
  var Source_data="";
  var RMSextIP="";
  var ES=String.fromCharCode(27); //Escape Separator
  var FS=String.fromCharCode(28); //File Separator
  var GS=String.fromCharCode(29); //Group Separator
  var RS=String.fromCharCode(30); //Record Separator
  var US=String.fromCharCode(31); //Unit Separator
  var nomSondeFixe="Sonde Fixe";
  var nomSondeMobile="Sonde Mobile";
  var nb_ESP = 0;
  var nomRMS=[];
  var IP_RMS=[];
  var nomTemperature=[];
  var nomActions=[];
  var ModeReseau=0;
  var ModePara=0;
  var Horloge=0;
  var LocalIP ="";
  var ESP32_Type =0;
    
  function LoadParaRouteur() {
    var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
             var LesParas=this.responseText;
             var Para=LesParas.split(GS);
             Source=Para[0];
             Source_data=Para[1];
             LocalIP=Para[2];
             RMSextIP= Para[7]; 
             AdaptationSource();  
             GH("nom_R",Para[3]);
             GH("version",Para[4]);
             GH("nomSondeFixe",Para[5]);
             GH("nomSondeMobile",Para[6]); 
             nomSondeFixe=Para[5];
             nomSondeMobile=Para[6];
             ModeReseau=Para[8];
             ModePara=Para[9];
             Horloge=Para[10];
             ESP32_Type=Para[11];
             var IpNoms=Para[12].split(RS);
             nb_ESP=IpNoms.length - 1;
             for (var c=0;c<nb_ESP;c++){
              var ip_nom=IpNoms[c].split(US);
              IP_RMS[c]=int2ip(ip_nom[0]); 
              nomRMS[c]=ip_nom[1];
              if (c==0) {
                var LesTemp=ip_nom[2].split(FS); //Noms temperature ESP local
                for (var i=0;i<LesTemp.length -1;i++){
                  var LaTemp=LesTemp[i].split(ES);
                  nomTemperature[LaTemp[0]] = LaTemp[1];
                }
              }
              var lesNomsActions=ip_nom[3].split(FS);
              nomActions[c]=[]; 
              for (var i=0;i<lesNomsActions.length -1;i++){
                  var Nact=lesNomsActions[i].split(ES);
                  nomActions[c][i]=Nact;
              }
             }
             IP_RMS[0] = Para[2]; 
             document.title=nomRMS[0] +" - " + document.title;
             nomRMS[0] = nomRMS[0] +" (local)";
             FinParaRouteur();
          
          }         
        };
        xhttp.open('GET', '/ParaRouteurAjax', true);
        xhttp.send();
  }
  function GID(id) { return document.getElementById(id); }
  function GH(id, T) {
    if ( GID(id)){
     GID(id).innerHTML = T; }
    }
  function GV(id, T) { GID(id).value = T; }
  function clean(S){ //Remplace & et ? pour les envois au serveur
    let res=S.replace(/\%/g,"%25");
    res = res.replace(/\&/g, "%26");
    res = res.replace(/\#/g, "%23");
    res = res.replace(/\+/g, "%2B");
    res=res.replace(/amp;/g,"");
    return res.replace(/\?/g,"%3F");
  }
  function int2ip (V) {
    var ipInt=parseInt(V);
    return ( (ipInt>>>24) +'.' + (ipInt>>16 & 255) +'.' + (ipInt>>8 & 255) +'.' + (ipInt & 255) );
  }
  function ip2int(ip) {
    ip=ip.trim();
    return ip.split('.').reduce(function(ipInt, octet) { return (ipInt<<8) + parseInt(octet, 10)}, 0) >>> 0;
  }
  function SetHautBas(){
      var S="<div class='onglets'><div class='Bonglet Baccueil'><a href='/'>Accueil</a></div><div class='Bonglet Bbrut'><a href='/Brute'>Donn&eacute;es brutes</a></div><div class='Bonglet Bparametres'><a href='/Para'>Param&egrave;tres</a></div><div class='Bonglet Bactions'><a href='/Actions'>Actions</a></div></div>";
      S +="<div id='onglets2'><div class='Bonglet2 Bgeneraux'><a href='/Para'>Généraux</a></div><div class='Bonglet2 Bexport'><a href='/Export'>Import / Export</a></div><div class='Bonglet2 Bota'><a href='/OTA'>Mise à jour par OTA</a></div><div id='Bwifi' class='Bonglet2 Bwifi'><a href='/Wifi'>WIFI</a></div>";
      S +="<div class='Bonglet2 Bheure' id='Bheure'><a href='/Heure'>Heure</a></div><div class='Bonglet2 Bcouleurs'><a href='/Couleurs'>Couleurs</a></div></div>";
      S +="<h2 id='nom_R'>Routeur Solaire - RMS</h2>";
      GH("lesOnglets",S);
      GH("pied","<div>Routeur Version : <span id='version'></span></div><div><a href='https:F1ATB.fr/fr' >F1ATB.fr</a></div>");
  }
  function Hdeci2Hmn(H){
    var HI=parseInt(H);
    return Math.floor(HI / 100) + ":" + ("0" + Math.floor(0.6 * (HI +0.4 - 100 * Math.floor(HI / 100)))).substr(-2, 2);
  }
  function Hmn2Hdeci(H){
    var separ=":";
    if (H.indexOf(".")>0) separ=".";
    if (H.indexOf("h")>0) separ="h";
    var val=H.split(separ);
    var h = Math.floor(100*parseInt(val[0]) + 0.4 + 100*parseInt(val[1])/60);
    h=Math.max(0,h);h=Math.min(2400,h);
    return h;  
  }
  function Reset(){
      GID("attente").style="visibility: visible;";
      var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            GID('BoutonsBas').innerHTML=this.responseText;
            GID("attente").style="visibility: hidden;";
            setTimeout(location.reload(),1000);
          }         
        };
        xhttp.open('GET', '/restart', true);
        xhttp.send();
  }

)====";
const char *CommunCSS = R"====(
    a:link {color:#aaf;text-decoration: none;}
    a:visited {color:#ccf;text-decoration: none;}
    .onglets{margin-top:4px;left:0px;font-size:130%;}
    #onglets2{margin-top:10px;left:0px;font-size:80%;display:none;}
    .Bonglet{margin-left:20px;border:outset 4px grey;background-color:#333;border-radius:6px;padding-left:20px;padding-right:20px;display:inline-block;}
    .Bonglet2{margin-left:20px;border:outset 2px grey;background-color:#333;border-radius:4px;padding-left:20px;padding-right:20px;display:inline-block;}
    #pied{display:flex;justify-content:space-between;font-size:14px;}
    .fsize12{font-size:12px;height:16px;}
    .fsize10{font-size:10px;height:14px;}
    .fsize8{font-size:8px;}
    .tableau { background-color:white;display:inline-block;margin:auto;padding:4px;color:black;border:10px inset azure;}
    table{border-collapse:collapse;}
    .lds-dual-ring {color: #cccc5b;visibility: hidden;}
    .lds-dual-ring,.lds-dual-ring:after {box-sizing: border-box;}
    .lds-dual-ring {display: inline-block;width: 80px;height: 80px;}
    .lds-dual-ring:after {content: " ";display: block;width: 64px;height: 64px;margin: 8px;border-radius: 50%;border: 6.4px solid currentColor;border-color: currentColor transparent currentColor transparent;animation: lds-dual-ring 1.2s linear infinite;}
    @keyframes lds-dual-ring {0% {transform: rotate(0deg);} 100% {transform: rotate(360deg);}}
    .bouton,input[type=file]::file-selector-button{margin: 5px;text-align:left;font-size:20px;height:28px;border:3px grey outset;border-radius:7px;cursor:pointer;}
)====";
const char *ParaCleHtml = R"====(
  <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="/commun.css">  
  <style>    
    body{color:white;}
    .form {margin:auto;padding:10px;display: table;text-align:left;width:100%;}
    .ligne {display: table-row;padding:10px;}
    .cadre {border-top:1px solid azure;}
    label,.nomR{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;width:70%;}
    input{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;}
    .boldT{text-align:left;font-weight:bold;padding:10px;}   
    .Bparametres{border:inset 10px azure;}
    .Bgeneraux{border:inset 4px azure;}
    #BoutonsBas{text-align:center;}    
    .Zone{width:100%;border 1px solid grey;border-radius:10px;margin-top:10px;background-color:rgba(30,30,30,0.3);} 
    #onglets2{display:none;} 
  </style>
  <script>
    var BordsInverse=[".Bparametres"];
    function SendCle(){
        document.cookie="CleAcces=" + GID("CleAcces").value.trim() ;
        
        GID("attente").style="visibility: visible;";
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            var retour=this.responseText;
            location.reload();
          }         
        };
        xhttp.open('GET', '/CleUpdate', true);
        xhttp.send(); 
    }
    function init(){
      SetHautBas();
      LoadParaRouteur();
      LoadCouleurs();
      document.cookie="CleAcces=" + GID("CleAcces").value.trim() ;
    }
    function   AdaptationSource(){}  ;
    function FinParaRouteur(){};
  </script>
  <title>Pass Acces F1ATB</title>
  </head>
  <body onload="init()";>
    <div id="lesOnglets"></div>
    <h4>Mot de passe d'accès</h4>
    <div class="Zone" >
        <div class="boldT" >Sécurité d'accès aux paramètres et Actions</div>
        <div class="form"  >    
          <div class='ligne'>
            <label for='CleAcces'>Entrez le mot de passe d'accès : </label>
            <input type='text' name='CleAcces' id='CleAcces'>
          </div>
        </div>
    </div>
    <div  id='BoutonsBas'>        
        <br><input  class='bouton' type='button' onclick="SendCle();" value='Envoyer' >
        <div class="lds-dual-ring" id="attente"></div>
    </div>
    <br>
    <div id='pied'></div>
    <script src="/ParaRouteurJS"></script>
    <script src="/CommunCouleurJS"></script>
   </body></html>
)====";