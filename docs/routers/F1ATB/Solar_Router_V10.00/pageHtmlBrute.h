//************************************************
// Page données RMS Brutes HTML et Javascript 
//************************************************
const char *PageBrute = R"====(
 <!doctype html>
   <html><head><meta charset="UTF-8"><style>
    * {box-sizing: border-box;}
    body {background: linear-gradient(#003,#77b5fe,#003);background-attachment:fixed;font-size:150%;text-align:center;width:100%;max-width:1000px;margin:auto;}
    h2{text-align:center;color:white;}
    a:link {color:#ccf;text-decoration: none;}
    a:visited {color:#ccf;text-decoration: none;}
    .ri { text-align: right;}
    .Wh { background-color:#fdd;}
    .A { background-color:#ddf;}
    .W { background-color:#f88;}
    .phi { background-color:#ffd;}
    .V { background-color:#ee8;}
    .VA { background-color:#dfd;}
    .Hz,.Enph { background-color:#eeb;}
    .titre{background-color:#ccc; text-align: center;font-weight: bold;}
    .foot { color:white;}
    #date,#dateLinky { color:white;}
    #DataLinky,#dataSmartG,#dataShellyEm,#dataUxIx3,#dataPmqtt {border:10px inset azure; background-color:white;margin:auto;padding:4px;text-align:left;overflow:hidden;display:inline-block;}
    #tableau,#DataESP32,#tableauLinky,#tableauEnphase { background-color:white;display:inline-block;margin:auto;padding:4px;}
    table {border:10px inset azure;}
    td { text-align: left;padding:4px;}
    #LED{position:absolute;top:4px;left:4px;width:0px;height:0px;border:5px solid red;border-radius:5px;}
    .onglets{margin-top:4px;left:0px;font-size:130%;}
    .Baccueil,.Bbrut,.Bparametres,.Bactions{margin-left:20px;border:outset 4px grey;background-color:#333;border-radius:6px;padding-left:20px;padding-right:20px;display:inline-block;}
    .Bbrut{border:inset 8px azure;}
    .pied{display:flex;justify-content:space-between;font-size:14px;color:white;}
    .dispT{display:none;}
    .ce { text-align: center;position:relative;}
    svg { border:10px inset azure;background: linear-gradient(#333,#666,#333);}
    #infoUxIx2,#infoUxIx3,#infoUxI,#infoLinky,#infoEnphase,#infoSmartG,#infoShellyEm,#infoPmqtt{display:none;}
    #donneeDistante{font-size:50%;color:white;text-align:center;margin-bottom:10px;display:none;}
  </style></head>
  <body  onload='LoadParaRouteur();' >
    <div id='LED'></div>
    <div class='onglets'><div class='Baccueil'><a href='/'>Accueil</a></div><div class='Bbrut'><a href='/Brute'>Donn&eacute;es brutes</a></div><div class='Bparametres'><a href='/Para'>Param&egrave;tres</a></div><div class='Bactions'><a href='/Actions'>Actions</a></div></div>
    <h2 id='nom_R'>Routeur Solaire - RMS</h2>
    <div id='date'>Date</div><br><br>
    <div id='infoUxI'>
      <div class='foot' >Tension et Courant sur 20ms</div>
      <div  class='ce'><h3 style='position:absolute;top:20px;right:40px;'><span style='color:red;'>_<span id='Ueff'></span><small> V</small></span>
      <span style='color:lightgreen;'> _<span id='Ieff'></span><small> A</small></span><br>
      <span style='color:white;'> <small>Facteur de puissance : <span id='cosphi'></span></small></span></h3><p id='SVG'></p></div>
    </div>
    <div id='infoUxIx2'>
      <br><br><div class='foot' >Donn&eacute;es brutes capteur JSY-MK-194T</div>
      <div id='tableau'></div>
    </div>
    <div id='infoUxIx3'>
      <div class='foot' >Donn&eacute;es brutes capteur JSY-MK-333</div>
      <div id='dataUxIx3'></div>
    </div>
    <div id='infoEnphase'>
      <br><br><div class='foot' >Donn&eacute;es Enphase Envoy-S Metered</div>
      <div id='tableauEnphase'></div>
    </div>
    <div id='infoSmartG'>
      <div class='foot' >Donn&eacute;es SmartGateways</div>
      <div id='dataSmartG'></div>
    </div>
    <div id='infoShellyEm'>
      <div class='foot' >Donn&eacute;es Shelly Em </div>
      <div id='dataShellyEm'></div>
    </div>
    <div id='infoPmqtt'>
      <div class='foot' >Données puissances recues par MQTT</div>
      <div id='dataPmqtt'></div>
    </div>
    <div id='infoLinky'>
      <div id='dateLinky'></div>
      <div id='tableauLinky'></div>
      <br><br><div class='foot' >Donn&eacute;es brutes Linky en mode standard</div>
      <div><div id='DataLinky' ></div></div>
    </div>
    <div id="donneeDistante">Données distantes</div>
    <div class='foot' >Donn&eacute;es ESP32</div>
    <div><div id='DataESP32' ></div></div><br>
    <div class='pied'><div>Routeur Version : <span id='version'></span></div><div><a href='https:F1ATB.fr' >F1ATB.fr</a></div></div>
    <script src='BruteJS'></script>
    <script src="/ParaRouteurJS"></script>
    <br></body></html>
)====";

const char *PageBruteJS = R"====(
       var InitFait=false;
       var IdxMessage=0;
       var MessageLinky='';

       const M=[]; //Pour UxIx2
       M.push(['Tension_M','Tension efficace','V','V']);
       M.push(['Intensite_M','Courant efficace','A','A']);
       M.push(['PuissanceS_M','Puissance <small>(Pw)</small>','W','W']);
       M.push(['PowerFactor_M','Facteur de puissance','','phi']);
       M.push(['Energie_M_Soutiree','Energie active soutirée','Wh','Wh']);
       M.push(['Energie_M_Injectee','Energie active injectée','Wh','Wh']);
       M.push(['Tension_T','Tension efficace','V','V']);
       M.push(['Intensite_T','Courant efficace','A','A']);
       M.push(['PuissanceS_T','Puissance <small>(Pw)</small>','W','W']);
       M.push(['PowerFactor_T','Facteur de puissance','','phi']);
       M.push(['Energie_T_Soutiree','Energie active consommée','Wh','Wh']);
       M.push(['Energie_T_Injectee','Energie active produite','Wh','Wh']); 
       M.push(['Frequence','Fréquence','Hz','Hz']);
       const E=[]; //Pour Enphase
       E.push(['Tension_M','Tension efficace','V','V']);
       E.push(['Intensite_M','Courant efficace','A','A']);
       E.push(['PuissanceS_M','Puissance réseau public <small>(Pw)</small>','W','W']);
       E.push(['PowerFactor_M','Facteur de puissance','','phi']);
       E.push(['Energie_M_Soutiree','Energie active soutirée','Wh','Wh']);
       E.push(['Energie_M_Injectee','Energie active injectée','Wh','Wh']);
       E.push(['PactProd','Puissance produite <small>(Pw)</small>','W','W']);
       E.push(['PactConso_M','Puissance consommée <small>(Pw)</small>','W','W']);
       E.push(['SessionId','Session Id</small>','','Enph']);
       E.push(['Token_Enphase','Token','','Enph']);
       
       const L=[];
       L.push(['EAST','Energie active soutir&eacute;e',false,'Wh',0]);
       L.push(['EASF01','Energie active soutir&eacute;e Fournisseur,<br>index 01',true,'Wh',0]);
       L.push(['EASF02','Energie active soutir&eacute;e Fournisseur,<br>index 02',true,'Wh',0]);
       L.push(['EASF03','Energie active soutir&eacute;e Fournisseur,<br>index 03',true,'Wh',0]);
       L.push(['EASF04','Energie active soutir&eacute;e Fournisseur,<br>index 04',true,'Wh',0]);
       L.push(['EASF05','Energie active soutir&eacute;e Fournisseur,<br>index 05',true,'Wh',0]);
       L.push(['EASF06','Energie active soutir&eacute;e Fournisseur,<br>index 06',true,'Wh',0]);
       L.push(['EASF07','Energie active soutir&eacute;e Fournisseur,<br>index 07',true,'Wh',0]);
       L.push(['EASF08','Energie active soutir&eacute;e Fournisseur,<br>index 08',true,'Wh',0]);
       L.push(['EASF09','Energie active soutir&eacute;e Fournisseur,<br>index 09',true,'Wh',0]);
       L.push(['EASF10','Energie active soutir&eacute;e Fournisseur,<br>index 10',true,'Wh',0]);
       L.push(['EAIT','Energie active inject&eacute;e',false,'Wh',0]);
       L.push(['IRMS1','Courant efficace, phase 1',true,'A',0]);
       L.push(['IRMS2','Courant efficace, phase 2',true,'A',0]);
       L.push(['IRMS3','Courant efficace, phase 3',true,'A',0]);
       L.push(['URMS1','Tension efficace, phase 1',true,'V',0]);
       L.push(['URMS2','Tension efficace, phase 2',true,'V',0]);
       L.push(['URMS3','Tension efficace, phase 3',true,'V',0]);
       L.push(['SINSTS','Puissance app. Instantan&eacute;e soutir&eacute;e',false,'VA',0]);
       L.push(['SINSTS1','Puissance app. Instantan&eacute;e soutir&eacute;e phase 1',true,'VA',0]);
       L.push(['SINSTS2','Puissance app. Instantan&eacute;e soutir&eacute;e phase 2',true,'VA',0]);
       L.push(['SINSTS3','Puissance app. Instantan&eacute;e soutir&eacute;e phase 3',true,'VA',0]);
       L.push(['SMAXSN','Puissance app. max. soutir&eacute;e n',false,'VA',1]);
       L.push(['SMAXSN1','Puissance app. max. soutir&eacute;e n phase 1',true,'VA',1]);
       L.push(['SMAXSN2','Puissance app. max. soutir&eacute;e n phase 2',true,'VA',1]);
       L.push(['SMAXSN3','Puissance app. max. soutir&eacute;e n phase 3',true,'VA',1]);
       L.push(['SMAXSN-1','Puissance app. max. soutir&eacute;e n-1',false,'VA',1]);
       L.push(['SMAXSN1-1','Puissance app. max. soutir&eacute;e n-1 phase 1',true,'VA',1]);
       L.push(['SMAXSN2-1','Puissance app. max. soutir&eacute;e n-1 phase 2',true,'VA',1]);
       L.push(['SMAXSN3-1','Puissance app. max. soutir&eacute;e n-1 phase 3',true,'VA',1]);
       L.push(['SINSTI','Puissance app. Instantan&eacute;e inject&eacute;e',false,'VA',0]);
       L.push(['SMAXIN','Puissance app. max inject&eacute;e n',false,'VA',1]);
       L.push(['SMAXIN-1','Puissance app. max inject&eacute;e n-1',false,'VA',1]);
       L.push(['LTARF','Option Tarifaire',false,'',2]);

       function creerTableauUxIx2(){
        var S='<table>';
        for (var i=0;i<M.length;i++){
          if (i==0){
            S+='<tr  class="titre"><td class="titre" id="nomSondeMobile">Maison</td><td ></td><td></td></tr>';
          }
          if (i==6){
            S+='<tr  class="titre"><td class="titre" id="nomSondeFixe">Triac</td><td ></td><td></td></tr>';
          }
          S+='<tr  class="'+M[i][3]+'"><td>'+M[i][1]+'</td><td id="'+M[i][0]+'" class="ri"></td><td>'+M[i][2]+'</td></tr>';
        }
        S+='</table>';
       GH('tableau', S);
       }
       function creerTableauEnphase(){
        var S='<table>';
        for (var i=0;i<E.length;i++){
          if (i==0){
            S+='<tr  class="titre"><td class="titre" id="nomSondeMobile">Maison</td><td ></td><td></td></tr>';
          }
          S+='<tr  class="'+E[i][3]+'"><td>'+E[i][1]+'</td><td id="'+E[i][0]+'" class="ri"></td><td>'+E[i][2]+'</td></tr>';
        }
        S+='</table>';
       GH('tableauEnphase', S);
       }
       function creerTableauLinky(){
        var S='<table>';
        for (var i=0;i<L.length;i++){
          S+='<tr id="L'+L[i][0]+'" style="display:none;" class="'+L[i][3]+'"><td>'+L[i][1]+'</td><td id="'+L[i][0]+'" class="ri"></td><td>'+L[i][3]+'</td><td id="h'+L[i][0]+'" class="ri"></td></tr>';
        }
        S+='</table>';
        GH('tableauLinky', S);
      }
      function LoadData() {
        GID('LED').style='display:block;';
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            GID('LED').style='display:none;';
            var DuRMS=this.responseText;
            var groupes=DuRMS.split(GS)
            var G0=groupes[0].split(RS);
            GH('date',G0[0]);
            Source_data=G0[1];
            if (Source_data == "UxI"){
              GID('infoUxI').style.display="block";
              GH('Ueff',parseInt(G0[2],10));
              GH('Ieff',G0[3]);
              GH('cosphi',G0[4]);
              var volt=groupes[1].split(RS);
              var amp=groupes[2].split(RS);
              var S= "<svg height='400' width='1000' >";
              S += "<line x1='0' y1='400' x2='0' y2='0' style='stroke:rgb(0,0,0);stroke-width:2' />";
              S += "<line x1='0' y1='197' x2='1000' y2='197' style='stroke:rgb(0,0,0);stroke-width:2' />";
              var  Vmax = 500;
              var Imax = 500;
              for (var i = 0; i < 100; i++) {
                Vmax = Math.max(Math.abs(volt[i]), Vmax);
                Imax = Math.max(Math.abs(amp[i]), Imax);
              }

              S += "<polyline points='";
              for (var i = 0; i < 100; i++) {
                var Y = 197 - 185 * volt[i] / Vmax;
                var X = 10 * i;
                S += X + ',' + Y + ' ';
              }
              S += "' style='fill:none;stroke:red;stroke-width:6' />";
              S += "<polyline points='";
              for (var i = 0; i < 100; i++) {
                var Y = 197 - 185 * amp[i] / Imax;
                var X = 10 * i;
                S += X + ',' + Y + ' ';
              }
              S += "' style='fill:none;stroke:lightgreen;stroke-width:6' />";
              S += "</svg>";
              GH('SVG',S);
            }
            if (Source_data == "UxIx2"){
              GID('infoUxIx2').style.display="block";
              var G1=groupes[1].split(RS);
              if(!InitFait){
                  InitFait=true;
                  creerTableauUxIx2();
                  GH("nomSondeFixe",nomSondeFixe);
                  GH("nomSondeMobile",nomSondeMobile); 
              }
              for (var j=0;j<M.length;j++){
                    if( M[j][3] == 'Wh' ) {
                      GH(M[j][0], LaVal(G1[j])); 
                    } else {
                      GH(M[j][0], G1[j]); 
                    }    
              }
            }
            if (Source_data == "Enphase"){
              GID('infoEnphase').style.display="block";
              var G1=groupes[1].split(RS);
              if(!InitFait){
                  InitFait=true;
                  creerTableauEnphase();
                  GH("nomSondeMobile",nomSondeMobile); 
              }
              for (var j=0;j<E.length;j++){
                    if( E[j][3] == 'Wh' ) {
                      GH(E[j][0], LaVal(G1[j])); 
                    } else {
                      GH(E[j][0], G1[j]); 
                    }    
              }
            }
            if (Source_data == "SmartG"){
              GID('infoSmartG').style.display="block";
              groupes[1] = groupes[1].replaceAll('"','');
              var G1=groupes[1].split(",");
              var S="";              
              for (var i=0;i<G1.length;i++){
                    S +=G1[i]+"<br>";
              }
               GH('dataSmartG', S);
            }
            if (Source_data == "UxIx3"){
              GID('infoUxIx3').style.display="block";
               GH('dataUxIx3', groupes[1]);
            }
            if (Source_data == "ShellyEm"){
              GID('infoShellyEm').style.display="block";
              groupes[1] = groupes[1].replaceAll('"','');
              var G1=groupes[1].split(",");
              var S="";              
              for (var i=0;i<G1.length;i++){
                    S +=G1[i]+"<br>";
              }
               GH('dataShellyEm', S);
            }
            if (Source_data == "Pmqtt"){
              GID('infoPmqtt').style.display="block";
               GH('dataPmqtt', groupes[1]);
            }
            if (Source_data == "Linky"){
              GID('infoLinky').style.display="block";
              if(!InitFait){
                  InitFait=true;
                  creerTableauLinky();
              }
              MessageLinky +=groupes[1];
              var blocs=MessageLinky.split(String.fromCharCode(2));
              var lg=blocs.length;
              if (lg>2){
                  MessageLinky=String.fromCharCode(2)+blocs[lg-1];               
                  GH('DataLinky', '<pre>'+blocs[lg-2]+'</pre>');
                  var lignes=blocs[lg-2].split(String.fromCharCode(10));
                  for (var i=0;i<lignes.length;i++){
                    var colonnes=lignes[i].split(String.fromCharCode(9));
                    if (colonnes[0]=='DATE'){
                      GH('dateLinky', LaDate(colonnes[1]));
                    }
                    for (var j=0;j<L.length;j++){
                      if (colonnes[0]==L[j][0]){
                        if (!L[j][2] || parseInt(colonnes[1])>0){
                          GID('L'+L[j][0]).style.display="table-row";
                          switch (L[j][4]){
                            case 0:
                              GH(L[j][0], LaVal(colonnes[1]));
                              break;
                            case 1:
                              GH('h'+L[j][0],  LaDate(colonnes[1]));
                              GH(L[j][0], LaVal(colonnes[2]));
                              break;
                            case 2: //Texte
                              GH('h'+L[j][0], colonnes[1]);
                              break;
                          }
                        }
                      }
                    }
                  }
                  GID('LED').style='display:none;';
              }
              IdxMessage=groupes[2];
            }
            
             setTimeout('LoadData();',2000);
          }  
        };
        xhttp.open('GET', 'ajax_dataRMS?idx='+IdxMessage, true);
        xhttp.send();
      }
      function LoadDataESP32() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
              var dataESP=this.responseText;
              var message=dataESP.split(RS);
              var S='<table>';
              var H=parseInt(message[0]);
              H=H + (message[0]-H)*0.6;
              H=H.toFixed(2);
              H=H.replace(".", "h ")+"mn";
              var LaSource=Source;
              if (LaSource=='Ext') LaSource="Externe ("+Source_data+")<br>" +int2ip(RMSextIP);
              S+='<tr><td>ESP On depuis :</td><td>'+H+'</td></tr>';
              S+='<tr><td>Source des mesures :</td><td>'+LaSource+'</td></tr>';
              S+='<tr><td>Niveau WiFi :</td><td>'+message[1]+' dBm</td></tr>';
              S+="<tr><td>Point d'acc&egrave;s WiFi :</td><td>"+message[2]+'</td></tr>';
              S+='<tr><td>Adresse MAC ESP32 :</td><td>'+message[3]+'</td></tr>';
              S+='<tr><td>R&eacute;seau WiFi :</td><td>'+message[4]+'</td></tr>';
              S+='<tr><td>Adresse IP ESP32 :</td><td>'+message[5]+'</td></tr>';
              S+='<tr><td>Adresse passerelle :</td><td>'+message[6]+'</td></tr>';
              S+='<tr><td>Masque du r&eacute;seau :</td><td>'+message[7]+'</td></tr>';
              S+='<tr><td>Charge coeur 0 (Lecture Puissance) Min, Moy, Max :</td><td>'+message[8]+' ms</td></tr>';
              S+='<tr><td>Charge coeur 1 (Calcul + Wifi) Min, Moy, Max :</td><td>'+message[9]+' ms</td></tr>';
              S+='<tr><td>Espace mémoire EEPROM utilisé :</td><td>'+message[10]+' %</td></tr>';
              S+="<tr><td>Nombre d'interruptions en 15ms du Gradateur (signal Zc) : Filtrés/Brutes :</td><td>"+message[11]+'</td></tr>';
              S+='<tr><td>Synchronisation 10ms au Secteur ou asynchrone horloge ESP32</td><td>'+message[12]+'</td></tr>';
              S +='<tr><td style="text-align:center;"><strong>Messages</strong></td><td></td></tr>';
              for (var i=0;i<10;i++){
                S +='<tr><td>'+message[13+i]+'</td><td></td></tr>';
              }
              S+='</table>';
              GH('DataESP32', S);             
             setTimeout('LoadDataESP32();',5000);
          }
          
        };
        xhttp.open('GET', 'ajax_dataESP32', true);
        xhttp.send();
      }
      function LaDate(d){
          return d.substr(0,1)+' '+d.substr(5,2)+'/'+d.substr(3,2)+'/'+d.substr(1,2)+' '+d.substr(7,2)+'h '+d.substr(9,2)+'mn '+d.substr(11,2)+'s';
      }
      function LaVal(d){
          d=parseInt(d);
          d='           '+d.toString();
          return d.substr(-9,3)+' '+d.substr(-6,3)+' '+d.substr(-3,3);
      }
      function AdaptationSource(){
        if(Source=="Ext"){
          GID("donneeDistante").style.display="block";
        }
        LoadData();LoadDataESP32();
      }
)====";

