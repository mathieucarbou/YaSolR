//************************************************
// Page données RMS Brutes HTML et Javascript 
//************************************************
const char *PageBrute = R"====(
 <!doctype html>
   <html><head><meta charset="UTF-8">
   <link rel="stylesheet" href="/commun.css">
   <style>
    .ri { text-align: right;}
    .Wh { background-color:#fdd;}
    .A { background-color:#ddf;}
    .W { background-color:#f88;}
    .phi { background-color:#ffd;}
    .V { background-color:#ee8;}
    .VA { background-color:#dfd;}
    .Hz,.Enph { background-color:#eeb;}
    .titre{background-color:#ccc; text-align: center;font-weight: bold;}
    .dataIn{text-align:left;overflow:hidden;}
    td { text-align: left;padding:4px;}
    #LED{position:absolute;top:4px;left:4px;width:0px;height:0px;border:5px solid red;border-radius:5px;}
    .Bbrut{border:inset 8px azure;}
    .dispT{display:none;}
    .ce { text-align: center;position:relative;}
    svg { border:10px inset azure;}
    #infoUxIx2,#infoUxIx3,#infoUxI,#infoNotDef,#infoLinky,#infoEnphase,#infoSmartG,#infoHomeW,#infoShellyEm,#infoPmqtt{display:none;}
    #donneeDistante{font-size:50%;text-align:center;margin-bottom:10px;display:none;}
    .bloc a:link {color:#116;text-decoration: none;}
    .bloc a:visited {color:#226;text-decoration: none;}
  </style>
  <title>Data brute F1ATB</title>
  </head>
  <body  onload='SetHautBas();LoadParaRouteur();' >
    <div id='LED'></div>
    <div id="lesOnglets"></div>
    <div id='date'>Date</div><br><br>
    <div id='infoNotDef'>
      <div class='tableau'>Source des mesures de puissance non définie<br>A définir au bas de la page Paramètres<br><br>Données simulées</div> <br><br>    
    </div>
    <div id='infoUxI'>
      <div >Tension et Courant sur 20ms</div>
      <div  class='ce'><h3 style='position:absolute;top:20px;right:40px;'><span id='Ueff'>.</span>
      <span id='Ieff'></span><br>
      <span id='cosphi'></span></h3><p id='SVG'></p></div>
    </div>
    <div id='infoUxIx2'>
      <br><br><div>Donn&eacute;es brutes capteur JSY-MK-194T</div>
      <div id='tableau' class='tableau'></div>
    </div>
    <div id='infoUxIx3'>
      <div>Donn&eacute;es brutes capteur JSY-MK-333</div>
      <div id='dataUxIx3' class='tableau dataIn'></div>
    </div>
    <div id='infoEnphase'>
      <br><br><div>Donn&eacute;es Enphase Envoy-S Metered</div>
      <div id='tableauEnphase' class='tableau'></div>
    </div>
    <div id='infoSmartG'>
      <div>Donn&eacute;es SmartGateways</div>
      <div id='dataSmartG' class='tableau dataIn'></div>
    </div>
    <div id='infoHomeW'>
      <div>Donn&eacute;es HomeWizard</div>
      <div id='dataHomeW' class='tableau dataIn'></div>
    </div>
    <div id='infoShellyEm'>
      <div>Donn&eacute;es Shelly Em </div>
      <div id='dataShellyEm' class='tableau dataIn'></div>
    </div>
    <div id='infoPmqtt'>
      <div>Données puissances recues par MQTT</div>
      <div id='dataPmqtt' class='tableau dataIn'></div>
    </div>
    <div id='infoLinky'>
      <div id='dateLinky'></div>
      <div id='tableauLinky' class='tableau'></div>
      <br><br><div>Donn&eacute;es brutes Linky en mode standard</div>
      <div><div id='DataLinky' class='tableau dataIn' ></div></div>
    </div>
    <div id="donneeDistante">Données distantes</div>
    <div>Donn&eacute;es ESP32</div>
    <div><div id='DataESP32' class='tableau bloc'></div></div><br>
    <div id='pied'></div>
    <script src="/ParaRouteurJS"></script>
    <script src='BruteJS'></script>
    <script src="/CommunCouleurJS"></script>
    <br></body></html>
)====";

const char *PageBruteJS = R"====(
       var InitFait=false;
       var IdxMessage=0;
       var MessageLinky='';
       var BordsInverse=[".Bbrut"];
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
       function LoadDataESP32() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
              var dataESP=this.responseText;
              var Messages=dataESP.split(GS);
              var message=Messages[0].split(RS);
              var S='<table>';
              var H=parseInt(message[0]);
              H=H + (message[0]-H)*0.6;
              H=H.toFixed(2);
              H=H.replace(".", "h ")+"mn";
              var LaSource=Source;
              if (LaSource=='Ext') LaSource="Externe ("+Source_data+")<br>" +int2ip(RMSextIP);
              var typeESP32=["Non défini","Wroom seul","Carte 1 relais","Carte 4 relais","Ecran320*240","","","","","","ESP32-ETH01"];
              S+='<tr><td>ESP32 On depuis :</td><td>'+H+'</td></tr>';
              S+='<tr><td>ESP32 modèle :</td><td>'+typeESP32[message[1]]+'</td></tr>';
              S+='<tr><td>Source des mesures :</td><td>'+LaSource+'</td></tr>';
              if (ModeReseau<2){
                if (message[1]<10){ //WIFI
                  S+='<tr><td>Niveau WiFi :</td><td>'+message[2]+' dBm</td></tr>';
                  S+="<tr><td>Point d'acc&egrave;s WiFi :</td><td>"+message[3]+'</td></tr>';
                  S+='<tr><td>R&eacute;seau WiFi :</td><td>'+message[6]+'</td></tr>';
                  S+='<tr><td>Canal WiFi :</td><td>'+message[4]+'</td></tr>';
                }
                S+='<tr><td>Adresse MAC ESP32 :</td><td>'+message[5]+'</td></tr>';                
                S+='<tr><td>Adresse IP ESP32 :</td><td>'+message[7]+'</td></tr>';
                S+='<tr><td>Adresse passerelle :</td><td>'+message[8]+'</td></tr>';
                S+='<tr><td>Masque du r&eacute;seau :</td><td>'+message[9]+'</td></tr>';
              } else {
                S+="<tr><td>Adresse IP ESP32 (Point d'Accès) :</td><td>192.168.4.1</td></tr>";
              }
              S+='<tr><td>Charge coeur 0 (Lecture Puissance) Min, Moy, Max :</td><td>'+message[10]+' ms</td></tr>';
              S+='<tr><td>Charge coeur 1 (Calcul + Wifi) Min, Moy, Max :</td><td>'+message[11]+' ms</td></tr>';
              S+='<tr><td>Espace mémoire EEPROM utilisé :</td><td>'+message[12]+' %</td></tr>';
              S+='<tr><td>Mémoire RAM libre actuellement :</td><td>'+message[13]+' octet</td></tr>';
              S+='<tr><td>Mémoire RAM libre minimum :</td><td>'+message[14]+' octet</td></tr>';
              S+="<tr><td>Nombre d'interruptions en 15ms du Gradateur (signal Zc) : Filtrés/Brutes :</td><td>"+message[15]+'</td></tr>';
              S+='<tr><td>Synchronisation au Secteur ou asynchrone horloge ESP32</td><td>'+message[16]+'</td></tr>';
              var Stemp=message[17];
              if (message[17]>0) Stemp +='<span class="fsize10">' + message[18] +'</span>';
              S+="<tr><td>Nombre de capteurs de température DS18B20 :</td><td>"+Stemp+'</td></tr>';
              S +='<tr><td style="text-align:center;"><strong>Messages</strong></td><td></td></tr>';
              var message1=Messages[1].split(RS);
              for (var i=1;i<=10;i++){
                S +='<tr><td>'+message1[i]+'</td><td></td></tr>';
              }
              var message2=Messages[2].split(RS);
              if(message2.length>1){
                S +='<tr><td style="text-align:center;"><strong>Note échanges entre routeurs</strong></td><td></td></tr>';
                for (var i=0;i<message2.length-1;i++){
                    var Note=message2[i].split(ES);
                    S +='<tr><td>'+Note[0]+'</td><td>' +Note[1] +'</td></tr>';
                }
              }
              S+='</table>';
              GH('DataESP32', S);             
             setTimeout('LoadDataESP32();',5000);
          }
          
        };
        xhttp.open('GET', '/ajax_dataESP32', true);
        xhttp.send();
       }
       function FinParaRouteur(){
        LoadCouleurs();
        LoadData();
        LoadDataESP32();
       }

      function LoadData() {
        GID('LED').style='display:block;';
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            GID('LED').style='display:none;';
            var DuRMS=this.responseText;
            var groupes=DuRMS.split(GS);
            var G0=groupes[0].split(RS);
            GH('date',G0[0]);
            Source_data=G0[1];
            if (Source_data == "NotDef"){
              GID('infoNotDef').style.display="block";
            }
            if (Source_data == "UxI"){
              GID('infoUxI').style.display="block";
              GH('Ueff',"<span style='color:#" + Koul[Coul_W][3] + ";'>_" +parseInt(G0[2],10) + "<small> V</small></span>");
              GH('Ieff',"<span style='color:#" + Koul[Coul_VA][3] + ";'> _" + G0[3] + "<small> A</small></span>");
              GH('cosphi',"<span style='color:#"+Koul[Coul_Graphe][1] +";'> <small>Facteur de puissance : " + G0[4]+ "</small></span>");
              var volt=groupes[1].split(RS);
              var amp=groupes[2].split(RS);
              var cT="#"+Koul[Coul_Graphe][1];
              var style='background:linear-gradient(#' + Koul[Coul_Graphe][5] +',#' + Koul[Coul_Graphe][3] +',#' + Koul[Coul_Graphe][5] +');border-color:#' +Koul[Coul_Tab][5]+';' ;  
              var S= "<svg height='400' width='1000'  style='"+style+ "' >";
              S += "<line x1='0' y1='400' x2='0' y2='0' style='stroke:" + cT +";stroke-width:2' />";
              S += "<line x1='0' y1='197' x2='1000' y2='197' style='stroke:" + cT +";stroke-width:2' />";
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
              S += "' style='fill:none;stroke:#" + Koul[Coul_W][3] + ";stroke-width:6' />";
              S += "<polyline points='";
              for (var i = 0; i < 100; i++) {
                var Y = 197 - 185 * amp[i] / Imax;
                var X = 10 * i;
                S += X + ',' + Y + ' ';
              }
              S += "' style='fill:none;stroke:#" + Koul[Coul_VA][3] + ";stroke-width:6' />";
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
            if (Source_data == "HomeW"){
              GID('infoHomeW').style.display="block";
              groupes[1] = groupes[1].replaceAll('"','');
              var G1=groupes[1].split(",");
              var S="";              
              for (var i=0;i<G1.length;i++){
                    S +=G1[i]+"<br>";
              }
               GH('dataHomeW', S);
            }
            if (Source_data == "UxIx3"){
              GID('infoUxIx3').style.display="block";
               GH('dataUxIx3', groupes[1]);
            }
            if (Source_data == "ShellyEm" || Source_data == "ShellyPro"){
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
            setCouleur();
            setTimeout('LoadData();',2000);
          }  
        }
        xhttp.open('GET', '/ajax_dataRMS?idx='+IdxMessage, true);
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
        
      }
)====";

