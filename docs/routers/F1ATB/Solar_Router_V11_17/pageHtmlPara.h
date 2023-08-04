//***************************************************
// Page HTML et Javascript de gestion des Paramètres
//***************************************************
const char *ParaHtml = R"====(
  <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="commun.css">
  <style>    
    body{color:white;}
    .form {margin:auto;padding:10px;display: table;text-align:left;width:100%;}
    .ligne {display: table-row;padding:10px;}
    label{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;width:70%;}
    input{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;}
    .source label{display: table-cell;margin: 5px;text-align:left;font-size:16px;height:25px;width:initial;}
    .boldT{text-align:left;font-weight:bold;padding:10px;}   
    .Bparametres{border:inset 10px azure;}
    .Bgeneraux{border:inset 4px azure;}
    #BoutonsBas{text-align:center;}    
    #ligneFixe,#ligneTemperature,#ligneExt,#ligneEnphaseUser,#ligneEnphasePwd,#ligneEnphaseSerial,#infoIP,#ligneTopicP,#ligneTopicT{display:none;}
    .Zone{width:100%;border 1px solid grey;border-radius:10px;margin-top:10px;background-color:rgba(30,30,30,0.3);} 
    #onglets2{display:block;}  
  </style>
  <script src="/ParaJS"></script>
  <script src="/ParaRouteurJS"></script>
  </head>
  <body onLoad="Init();">
    <div id="lesOnglets"></div>
    <h4>Param&egrave;tres généraux</h4>
    <div class="Zone">
        <div class="boldT">Source des mesures de puissance</div>
        <div class="form"  > 
          <div class="ligne source">
            <label for='UxI' style='text-align:right;'>UxI</label>
            <input type='radio' name='sources' id='UxI' value="UxI"  onclick="checkDisabled();">
            <label for='UxIx2' style='text-align:right;'>UxIx2</label>
            <input type='radio' name='sources' id='UxIx2' value="UxIx2"  onclick="checkDisabled();">   
            <label for='UxIx3' style='text-align:right;'>UxIx3</label>
            <input type='radio' name='sources' id='UxIx3' value="UxIx3"  onclick="checkDisabled();">       
            <label for='Linky' style='text-align:right;'>Linky</label>
            <input type='radio' name='sources' id='Linky' value="Linky"  onclick="checkDisabled();">
            <label for='Enphase' style='text-align:right;'>Enphase-Envoy</label>
            <input type='radio' name='sources' id='Enphase' value="Enphase"  onclick="checkDisabled();">
            <label for='SmartG' style='text-align:right;'>SmartGateways</label>
            <input type='radio' name='sources' id='SmartG' value="SmartG"  onclick="checkDisabled();">
            <label for='ShellyEm' style='text-align:right;'>Shelly Em</label>
            <input type='radio' name='sources' id='ShellyEm' value="ShellyEm"  onclick="checkDisabled();">
            <label for='ShellyPro' style='text-align:right;'>Shelly Pro Em</label>
            <input type='radio' name='sources' id='ShellyPro' value="ShellyPro"  onclick="checkDisabled();">
            <label for='Ext' style='text-align:right;'>ESP Externe</label>
            <input type='radio' name='sources' id='Ext' value="Ext"  onclick="checkDisabled();">
            <label for='Pmqtt' style='text-align:right;'>MQTT</label>
            <input type='radio' name='sources' id='Pmqtt' value="Pmqtt"  onclick="checkDisabled();">
          </div>
        </div>
        <div class="form"  >
          <div class='ligne' id="ligneExt">
            <label for='RMSextIP'>Adresse IP <span id='labExtIp'></span> externe (ex : 192.168.1.248) : </label>
            <input type='text' name='RMSextIP' id='RMSextIP' >
          </div>
          <div class='ligne' id="ligneEnphaseUser">
            <label for='EnphaseUser'>Enphase Envoy-S metered User : <span class='fsize10'><br>Pour firmvare Envoy-S V7 seulement</span></label>
            <input type='text' name='EnphaseUser' id='EnphaseUser' >
          </div>
          <div class='ligne' id="ligneEnphasePwd">
            <label for='EnphasePwd'>Enphase Envoy-S metered Password : <span class='fsize10'><br>Pour firmvare Envoy-S V7 seulement</span></label>
            <input type='password' name='EnphasePwd' id='EnphasePwd' >
          </div>
          <div class='ligne' id="ligneEnphaseSerial">
            <label for='EnphaseSerial' id="label_enphase_shelly"></label>
            <input type='text' name='EnphaseSerial' id='EnphaseSerial' onchange='checkDisabled();'>
          </div>
          <div class='ligne' id="ligneTopicP">
            <label for='TopicP' >MQTT Topic Puissance :</label>
            <input type='text' name='TopicP' id='TopicP' >
          </div>
          <div><span class='fsize10'>Nécessite un Reset de l'ESP32</span></div>
        </div> 
    </div>
    <div class="Zone">
        <div class="boldT">Source des mesures de température</div>
        <div class="form"  >
          <div class="ligne source">
            <label for='tempNo' style='text-align:right;'>Pas de mesure</label>
            <input type='radio' name='srcTemp' id='tempNo' value="tempNo"  onclick="checkDisabled();">
            <label for='tempInt' style='text-align:right;'>Capteur DS18B20 Interne</label>
            <input type='radio' name='srcTemp' id='tempInt' value="tempInt"  onclick="checkDisabled();">   
            <label for='tempExt' style='text-align:right;'>Capteur DS18B20 ESP Externe</label>
            <input type='radio' name='srcTemp' id='tempExt' value="tempExt"  onclick="checkDisabled();">       
            <label for='tempMqtt' style='text-align:right;'>MQTT</label>
            <input type='radio' name='srcTemp' id='tempMqtt' value="tempMqtt"  onclick="checkDisabled();">
          </div>
        </div>
        <div class="form" id='ligneTemperature' >
          <div class='ligne' >
            <label for='nomTemperature' >Nom Température : </label>
            <input type='text' name='nomTemperature' id='nomTemperature' >
          </div>
          <div class='ligne' id="ligneTopicT">
            <label for='TopicT' >MQTT Topic Température :</label>
            <input type='text' name='TopicT' id='TopicT' >
          </div>
          <div class='ligne' id="ligneIPtemp">
            <label for='IPtemp' >Adresse IP ESP-RMS externe (ex : 192.168.1.248) :</label>
            <input type='text' name='IPtemp' id='IPtemp' >
          </div>
          
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
            <label for='nomSondeMobile' >Nom Données courant Maison : </label>
            <input type='text' name='nomSondeMobile' id='nomSondeMobile' >
          </div>
          <div class='ligne' id='ligneFixe'>
            <label for='nomSondeFixe' >Nom Données courant seconde sonde : </label>
            <input type='text' name='nomSondeFixe' id='nomSondeFixe' >
          </div>         
          <div class='ligne'>
            <label for='TempoEDFon'>Affichage couleur Tempo EDF : </label>
            <input type='checkbox' name='TempoEDFon' id='TempoEDFon' style='width:25px;' >
          </div>
          <div class='ligne'>
            <label for='MQTTRepete'>Puissances et température envoyées au serveur MQTT<br>Période (s) répétition  (0= pas d'envoi) : </label>
            <input type='number' name='MQTTRepete' id='MQTTRepete'  onclick="checkDisabled();" >
          </div>
          <div class='ligne'>
            <label for='subMQTT'>Souscrire au forçage des Actions via le serveur MQTT: </label>
            <input type='checkbox' name='subMQTT' id='subMQTT' style='width:25px;' onclick='checkDisabled();' >
          </div>
          <div class='ligne'>
             <label for='WifiSleep'>Wifi Sleep/veille <small>(réduit la consommation mais ralenti la communication)</small>: </label>
             <input type='checkbox' name='WifiSleep' id='WifiSleep' style='width:25px;' ><span class='fsize10'>Nécessite un Reset de l'ESP32</span>
          </div>
        </div>
        <div class="form"  >
          <div class="ligne source">
            <label for='Serial0' style='text-align:right;'>Pas de port série</label>
            <input type='radio' name='pSerie' id='Serial0' value="0"  >
            <label for='Serial1' style='text-align:right;'>Port série RX=gpio 16, TX=gpio 17</label>
            <input type='radio' name='pSerie' id='Serial1' value="1"  >
            <label for='Serial2' style='text-align:right;'>Port série RX=gpio 26, TX=gpio 27</label>
            <input type='radio' name='pSerie' id='Serial2' value="2"  >
          </div>
        </div>
        <div class="form"  >
          <div class="ligne source">
            <label for='Triac0' style='text-align:right;'>Pas de Triac</label>
            <input type='radio' name='pTriac' id='Triac0' value="0"  >
            <label for='Triac1' style='text-align:right;'>Triac pulse=gpio 4, Zc=gpio 5</label>
            <input type='radio' name='pTriac' id='Triac1' value="1"  >
            <label for='Triac2' style='text-align:right;'>Triac pulse=gpio 22, Zc=gpio 23</label>
            <input type='radio' name='pTriac' id='Triac2' value="2"  >
          </div>
        </div>
    </div>
    <div class="Zone">
        <div class="boldT">Adresse IP de l'ESP32 du Routeur</div>
        <div class="form"  >
          <div class='ligne'>
            <label for='dhcp'>Adresse IP auto (DHCP) : </label>
            <input type='checkbox' name='dhcp' id='dhcp' style='width:25px;' onclick="checkDisabled();"><span class='fsize10'>Nécessite un Reset de l'ESP32</span>
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
            <input type='text' name='MQTTUser' id='MQTTUser' >
          </div>
          <div class='ligne'>
            <label for='MQTTpwd'>MQTT mot de passe : </label>
            <input type='password' name='MQTTpwd' id='MQTTpwd' >
          </div>
          <div class='ligne'>
            <label for='MQTTPrefix'>MQTT Prefix (1 seul mot ex : homeassistant ) : </label>
            <input type='text' name='MQTTPrefix' id='MQTTPrefix' >
          </div>
          <div class='ligne'>
            <label for='MQTTdeviceName'>MQTT Device Name (1 seul mot ex : routeur_rms ) : </label>
            <input type='text' name='MQTTdeviceName' id='MQTTdeviceName' >
          </div>
        </div>
    </div>
    <div class="Zone" id="Zcalib">
        <div class="boldT" id='Tui'>Calibration Mesures Ueff et Ieff</div>
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
    <div  id='BoutonsBas'>        
        <br><input  class='bouton' type='button' onclick="SendValues();" value='Sauvegarder' >
        <div class="lds-dual-ring" id="attente"></div>
        <input  class='bouton' type='button' onclick='Reset();' value='ESP32 Reset' >
    </div>
    <br>
    <div id='pied'></div>
    <br>
  </body></html>
)====";
const char *ParaJS = R"====(
  var LaTemperature = -100;
  function Init(){
    SetHautBas();
    LoadParametres();
    LoadParaRouteur();
  }
  function LoadParametres() {
    var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
             var LesParas=this.responseText;
             var Para=LesParas.split(RS);  
             GID("dhcp").checked = Para[0]==1  ? true:false;
             GID("adrIP").value=int2ip(Para[1]);
             GID("gateway").value=int2ip(Para[2]);
             GID("masque").value=int2ip(Para[3]);
             GID("dns").value=int2ip(Para[4]);
             GID(Para[5]).checked = true;
             GID("RMSextIP").value=int2ip(Para[6]);
             GID("EnphaseUser").value=Para[7];
             GID("EnphasePwd").value=Para[8];
             GID("EnphaseSerial").value=Para[9];
             GID("TopicP").value=Para[10]; 
             GID("MQTTRepete").value = Para[11];
             GID("MQTTIP").value=int2ip(Para[12]);
             GID("MQTTPort").value=Para[13];
             GID("MQTTUser").value=Para[14];
             GID("MQTTpwd").value=Para[15];
             GID("MQTTPrefix").value=Para[16];
             GID("MQTTdeviceName").value=Para[17];
             GID("subMQTT").checked = Para[18]==1  ? true:false; 
             GID("nomRouteur").value=Para[19];
             GID("nomSondeFixe").value=Para[20];
             GID("nomSondeMobile").value=Para[21];
             LaTemperature=parseInt(Para[22]);
             GID("nomTemperature").value=Para[23];
             GID(Para[24]).checked = true; 
             GID("TopicT").value=Para[25]; 
             GID("IPtemp").value=int2ip(Para[26]); 
             GID("CalibU").value=Para[27];
             GID("CalibI").value=Para[28];
             GID("TempoEDFon").checked = Para[29]==1  ? true:false;
             GID("WifiSleep").checked = Para[30]==1  ? true:false;
             GID("Serial" + Para[31]).checked = true;
             GID("Triac" + Para[32]).checked = true;
             
             checkDisabled();
          }         
        };
        xhttp.open('GET', 'ParaAjax', true);
        xhttp.send();
      }
  function SendValues(){
    GID("attente").style="visibility: visible;";
    var dhcp = GID("dhcp").checked ? 1:0;
    var TempoEDFon = GID("TempoEDFon").checked ? 1:0;
    var Source_new = document.querySelector('input[name="sources"]:checked').value;
    var Source_Temp = document.querySelector('input[name="srcTemp"]:checked').value;
    var subMQTT = GID("subMQTT").checked ? 1:0;
    var WifiSleep = GID("WifiSleep").checked ? 1:0;
    var Serial = document.querySelector('input[name="pSerie"]:checked').value;
    var pTriac = document.querySelector('input[name="pTriac"]:checked').value;
    var S=dhcp+RS+ ip2int(GID("adrIP").value)+RS+ ip2int(GID("gateway").value);
    S +=RS+ip2int(GID("masque").value)+RS+ ip2int(GID("dns").value)
    S +=RS+Source_new+RS+ ip2int(GID("RMSextIP").value)+ RS+GID("EnphaseUser").value.trim()+RS+GID("EnphasePwd").value.trim()+RS+GID("EnphaseSerial").value.trim() +RS+GID("TopicP").value.trim();
    S +=RS+GID("MQTTRepete").value +RS+ip2int(GID("MQTTIP").value) +RS+GID("MQTTPort").value +RS+GID("MQTTUser").value.trim()+RS+GID("MQTTpwd").value.trim();
    S +=RS+GID("MQTTPrefix").value.trim()+RS+GID("MQTTdeviceName").value.trim() + RS + subMQTT;
    S +=RS+GID("nomRouteur").value.trim()+RS+GID("nomSondeFixe").value.trim()+RS+GID("nomSondeMobile").value.trim();
    S +=RS+GID("nomTemperature").value.trim() +RS+Source_Temp  +RS+GID("TopicT").value.trim() + RS + ip2int(GID("IPtemp").value);
    S +=RS+GID("CalibU").value+RS+GID("CalibI").value + RS + TempoEDFon + RS + WifiSleep + RS + Serial + RS + pTriac;
    S="?lesparas="+clean(S);
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
  function checkDisabled(){ 
    GID("infoIP").style.display = GID("dhcp").checked ? "none" : "table";
    GID("Zmqtt").style.display = (GID("MQTTRepete").value != 0  || GID("Pmqtt").checked || GID("tempMqtt").checked || GID("subMQTT").checked) ? "table" : "none";
    GID('ligneTemperature').style.display = (GID("tempNo").checked) ? "none" : "table";
    GID('ligneTopicT').style.display = (GID("tempMqtt").checked) ? "table-row" : "none";
    GID('ligneIPtemp').style.display = (GID("tempExt").checked) ? "table-row" : "none";
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
 
  
  function Reset(){
      var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            GID('BoutonsBas').innerHTML=this.responseText;
            setTimeout(location.reload(),3000);
          }         
        };
        xhttp.open('GET', 'restart', true);
        xhttp.send();
  }
  function AdaptationSource(){
      GID('ligneFixe').style.display = (Source_data=='UxIx2' || ((Source_data=='ShellyEm' || Source_data=='ShellyPro') && GID("EnphaseSerial").value <3))? "table-row" : "none";
      GID('Zcalib').style.display=(Source_data=='UxI' && Source=='UxI' ) ? "table" : "none";
      var txtExt = "ESP-RMS";
      if (Source=='Enphase') txtExt = "Enphase-Envoy";
      if (Source=='SmartG') txtExt = "SmartGateways";
      var lab_enphaseShelly= "Numéro série passerelle IQ Enphase : <span class='fsize10'><br>Pour firmvare Envoy-S V7 seulement</span>";
      if (Source=='ShellyEm' || Source=='ShellyPro') {
        txtExt = "Shelly (Pro) Em ";
        lab_enphaseShelly="Monophasé : Numéro de voie (0 ou 1) mesurant l'entrée du courant maison<br>Triphasé : mettre 3";
      }
      GID('labExtIp').innerHTML = txtExt;
      GID('label_enphase_shelly').innerHTML = lab_enphaseShelly;
      GID('ligneExt').style.display = (Source=='Ext' || Source=='Enphase' || Source=='SmartG' || Source=='ShellyEm' || Source=='ShellyPro') ? "table-row" : "none";
      GID('ligneEnphaseUser').style.display = (Source=='Enphase') ? "table-row" : "none";
      GID('ligneEnphasePwd').style.display = (Source=='Enphase') ? "table-row" : "none";
      GID('ligneEnphaseSerial').style.display = (Source=='Enphase' || Source=='ShellyEm' || Source=='ShellyPro') ? "table-row" : "none"; //Numéro de serie ou voie
  }
)====";

//Paramètres du routeur et fonctions générales pour toutes les pages.
const char *ParaRouteurJS = R"====(
  var Source="";
  var Source_data="";
  var RMSextIP="";
  var GS=String.fromCharCode(29); //Group Separator
  var RS=String.fromCharCode(30); //Record Separator
  var nomSondeFixe="Sonde Fixe";
  var nomSondeMobile="Sonde Mobile";
  var nomTemperature="Temperature"; 
  function LoadParaRouteur() {
    var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
             var LesParas=this.responseText;
             var Para=LesParas.split(RS);
             Source=Para[0];
             Source_data=Para[1];
             RMSextIP= Para[6]; 
             AdaptationSource();  
             GH("nom_R",Para[2]);
             GH("version",Para[3]);
             GH("nomSondeFixe",Para[4]);
             GH("nomSondeMobile",Para[5]); 
             nomSondeFixe=Para[4];
             nomSondeMobile=Para[5];
             nomTemperature=Para[7]; 
               
          }         
        };
        xhttp.open('GET', 'ParaRouteurAjax', true);
        xhttp.send();
  }
  function GID(id) { return document.getElementById(id); };
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
      var S="<div class='onglets'><div class='Baccueil'><a href='/'>Accueil</a></div><div class='Bbrut'><a href='/Brute'>Donn&eacute;es brutes</a></div><div class='Bparametres'><a href='/Para'>Param&egrave;tres</a></div><div class='Bactions'><a href='/Actions'>Actions</a></div></div>";
      S +="<div id='onglets2'><div class='Bgeneraux'><a href='/Para'>Généraux</a></div><div class='Bexport'><a href='/Export'>Import / Export</a></div><div class='Bota'><a href='/OTA'>Mise à jour par OTA</a></div><div class='Bwifi'><a href='/Change_Wifi'>Modif. Wifi</a></div></div>";
      S +="<h2 id='nom_R'>Routeur Solaire - RMS</h2>";
      GH("lesOnglets",S);
      GH("pied","<div>Routeur Version : <span id='version'></span></div><div><a href='https:F1ATB.fr/fr' >F1ATB.fr</a></div>");
  }
  
)====";
const char *CommunCSS = R"====(
  * {box-sizing: border-box;}
    body {font-size:150%;text-align:center;width:100%;max-width:1000px;margin:auto;background: linear-gradient(#003,#77b5fe,#003);background-attachment:fixed;padding:10px;}
    h2{text-align:center;color:white;}
    h3,h4,h5{color:white;}
    a:link {color:#aaf;text-decoration: none;}
    a:visited {color:#ccf;text-decoration: none;}
    .onglets{margin-top:4px;left:0px;font-size:130%;}
    #onglets2{margin-top:10px;left:0px;font-size:80%;display:none;}
    .Baccueil,.Bbrut,.Bparametres,.Bactions{margin-left:20px;border:outset 4px grey;background-color:#333;border-radius:6px;padding-left:20px;padding-right:20px;display:inline-block;}
    .Bgeneraux,.Bexport,.Bota,.Bwifi{margin-left:20px;border:outset 2px grey;background-color:#333;border-radius:4px;padding-left:20px;padding-right:20px;display:inline-block;}
    #pied{display:flex;justify-content:space-between;font-size:14px;color:white;}
    .fsize10{font-size:10px;height:14px;}
    .lds-dual-ring {color: #cccc5b;visibility: hidden;}
    .lds-dual-ring,.lds-dual-ring:after {box-sizing: border-box;}
    .lds-dual-ring {display: inline-block;width: 80px;height: 80px;}
    .lds-dual-ring:after {content: " ";display: block;width: 64px;height: 64px;margin: 8px;border-radius: 50%;border: 6.4px solid currentColor;border-color: currentColor transparent currentColor transparent;animation: lds-dual-ring 1.2s linear infinite;}
    @keyframes lds-dual-ring {0% {transform: rotate(0deg);} 100% {transform: rotate(360deg);}}
    .bouton,input[type=file]::file-selector-button{margin: 5px;text-align:left;font-size:20px;height:28px;border:3px grey outset;border-radius:7px;}
)====";