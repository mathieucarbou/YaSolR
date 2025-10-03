//********************************************************
// Page HTML et Javascript Mise à l'heure horloge interne *
//********************************************************
const char *HeureHtml = R"====(
  <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="/commun.css">
  <style>
    .Zone{width:40%;border: 1px solid grey;border-radius:10px;margin:auto;background-color:rgba(30,30,30,0.3);} 
    .form {margin:auto;padding:10px;display: table;text-align:left;width:100%;}
    .ligne {display: table-row;padding:10px;}
    label{display: table-cell;margin: 5px;text-align:right;font-size:20px;height:25px;width:50%;}
    input{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;width:50%;}
    #onglets2{display:block;}
    .Bparametres{border:inset 10px azure;}
    .Bheure{border:inset 4px azure;}
    .bouton{width:auto;}
  </style>
  <title>Set hours F1ATB</title>
  </head>
  <body onload="Init();">
    <div id='lesOnglets'></div>
    <h2>Mise à l'heure horloge interne</h2>
    <h4>Heure actuelle : <span id='date'></span></h4>
    <div class="Zone">
        <div class="form"  >
          <div class='ligne'>
            <label for='New_H'>Nouvelle Heure h:mn  </label><input type='text' name='New_H' id='New_H' >
          </div>
        </div>
    </div>
    <div>        
        <input  class='bouton' type='button' onclick="SendHour();" value='Envoyer' >
        <div class="lds-dual-ring" id="attente"></div>
    </div>
    
    <script>    
        var BordsInverse=[".Bparametres",".Bheure"];
        function Init(){
          SetHautBas();
          LoadParaRouteur();
          LoadData();
          LoadCouleurs();
        }

        function AdaptationSource(){}
        function FinParaRouteur(){}
        function LoadData() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() { 
            if (this.readyState == 4 && this.status == 200) {
                var DuRMS=this.responseText;
                var groupes=DuRMS.split(GS);
                var G0=groupes[0].split(RS);
                GID('date').innerHTML = G0[1]; 
                setTimeout('LoadData();',2000);
            }
          };
          xhttp.open('GET', '/ajax_data', true);
          xhttp.send();
        }
        function SendHour(){
          var New_H= GID("New_H").value.trim() ;
          GID("attente").style="visibility: visible;";
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() { 
            if (this.readyState == 4 && this.status == 200) {
              var retour=this.responseText;
              GID("attente").style="visibility: hidden;";
            }         
          };
          xhttp.open('GET', '/HourUpdate?New_H='+clean(New_H), true);
          xhttp.send(); 
        }
    </script>
    <br>
    <div id='pied'></div>
    <br>
    <script src="/ParaRouteurJS"></script>
    <script src="/CommunCouleurJS"></script>
  </body></html>
 
 )====";