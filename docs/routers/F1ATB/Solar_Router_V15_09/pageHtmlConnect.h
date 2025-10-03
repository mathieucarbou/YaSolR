// **********************************************************
//  Page de connexion 'Acces Point' pour définir réseau WIFI
// **********************************************************
const char *ConnectAP_Html = R"====(
<!doctype html>
<html><head><meta charset="UTF-8">    
    <link rel="stylesheet" href="/commun.css">
    
    <style>
      #form-passe {display: none;padding:10px;text-align:center;margin:auto;width:100%;}
      label,input,.dB {display: table-cell;padding-left:10px;padding-right:5px;text-align:left;font-size:120%;}
      .l0{display:table-row;margin:auto;background-color:#333;padding:2px;}
      .l1{display:table-row;margin:auto;background-color:#666;padding:2px;}
      #ListeWifi{display:inline-block;margin:auto;}
      .tableWifi{display:table;margin:auto;}
      #envoyer{padding-top:20px;display:none;}
      #attente2,#lesOnglets,#pied{display:none;}
      .Bparametres{border:inset 10px azure;}
      .Bwifi{border:inset 4px azure;}
    </style>
    <title>Access Point F1ATB</title>
</head>
<body onload="init();">
<div id='lesOnglets'></div>
<h1 id="h1T">Routeur Solaire - RMS</h1><h2>Connexion au r&eacute;seau WIFI local</h2>
<div id="ListeWifi"></div><br><br>
<div id="scanReseau">
    <input class='bouton' type='button' onclick="ScanWIFI();" value='Scan réseaux WIFI' >
</div>
<br>
<div id='form-passe'>
  <div >Entrez le mot de passe du r&eacute;seau : <span id='nom_reseau'></span></div>
  <input type='password' name='passe' id='passe' >
</div>
<div id="envoyer" >
    <input class='bouton' type='button' onclick="Envoyer();" value='Envoyer' >
</div>
<br>
<div id="attente2">Attendez l'adresse IP attribuée à l'ESP 32</div>
<br>
<div id="pied"></div>

<script>
  var BordsInverse=[".Bparametres",".Bwifi"];
  function ScanWIFI(){
    GH("ListeWifi", "Patientez 2s");
    var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
             var LesWifi=this.responseText;
             var Wifi=LesWifi.split(GS); 
             var S="Sélectionnez un réseau Wifi<br><span class='fsize12'>Une nouvelle adresse IP sera probablement attribuée par la box gérant le réseau</span>";
             S +="<div class='tableWifi'>";
             S +="<div class='l1' style='font-weight: bold;'><div class='dB'>SSID</div><div class='dB'>Niveau</div><div class='dB'> Adresse MAC</div><div class='dB'> Canal</div><div class='dB'> </div></div>";
             for (var i=0;i<Wifi.length-1;i++) {
                var wifi=Wifi[i].split(RS); 
                var j=i%2;
                S +="<div class='l"+ j +"'>";
                S +="<label for='W" + i+"'>" + wifi[0] +" </label>";
                S +="<div class='dB'>" +wifi[1] +" dBm </div><div class='dB'> " + wifi[2] +"</div><div class='dB'> "  +wifi[3] +"</div> <input type='radio' name='Wifi' value='" +  wifi[0] +"' onclick='ChoixWifi(this.value);'>";
                S +="</div>";
              }
              S +="</div>";
              S +="<div><span class='fsize12'>Ne sont visibles que les réseaux scannés à la mise sous tension de l'ESP32 ou après un reset</span></div>";
              S +="<div><span class='fsize12'>Attention, l'adresse IP attribuée à l'ESP32 peut changer</span></div>";
             GH("ListeWifi", S);
          }         
        };
        xhttp.open('GET', '/AP_ScanWifi', true);
        xhttp.send();
        GID("form-passe").style.display = "none";
        GID("envoyer").style.display = "none";
        GID("attente2").style.display = "none";
  }
  function Envoyer(){
    var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
             var Resp=this.responseText;
             var Txt=Resp.split(RS); 
             GH("ListeWifi", Txt[1]);
             GID("attente2").style.display = "none";
             GID("form-passe").style.display = "none";
             GID("envoyer").style.display = "none";
             if (Txt[0] == "Ok") {
              GID("scanReseau").style.display = "none";      
             }
          }         
        };
        var adr ="/AP_SetWifi?ssid="+clean(ssid)+"&passe=" + clean(GID("passe").value);
        xhttp.open('GET', adr, true);
        xhttp.send();
        GID("form-passe").style.display = "none";
        GID("envoyer").style.display = "none";
        if (window.location.href.indexOf("192.168.4.1")==-1) {
          GH("attente2","Adresse IP reconduite ou pas suivant box internet");
        }
        GID("attente2").style.display = "block";
  }
  ssid="";
  function ChoixWifi(V){
      ssid = V;
      GH("nom_reseau",ssid);
      GID("envoyer").style.display = "inline-block";
      GID("form-passe").style.display = "inline-block";
  }
  function init(){
          SetHautBas(); 
          LoadParaRouteur();         
          GID("lesOnglets").style.display = "block";
          GID("onglets2").style.display = "block";
          GID("pied").style.display = "flex";
          GID("h1T").style.display = "none";
          LoadCouleurs();
  }
  function AdaptationSource(){ }
  function FinParaRouteur(){
    GID("Bheure").style.display= (Horloge>1) ? "inline-block": "none";
    GID("Bwifi").style.display= (ESP32_Type<10) ? "inline-block": "none";
  }
</script>
<script src="/ParaRouteurJS"></script>
<script src="/CommunCouleurJS"></script>
</body></html>
)====";
