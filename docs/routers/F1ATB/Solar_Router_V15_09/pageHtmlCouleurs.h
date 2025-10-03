//********************************************************
// Page HTML et Javascript Mise à l'heure horloge interne *
//********************************************************
const char *CouleursHtml = R"====(
  <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="/commun.css">
  <style>
    .Zone{width:100%;border:1px solid grey;border-radius:10px;margin-top:10px;background-color:rgba(30,30,30,0.3);} 
    .boldT{text-align:left;font-weight:bold;padding:10px;}
    .form {margin:auto;padding:10px;display: table;text-align:left;width:100%;}
    .ligne {display: table-row;margin-top:5px;}
    .ligneB{font-weight: bold;}
    .ligne div{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;}
    .liste{display:flex;justify-content:center;text-align:left;} 
    #onglets2{display:block;}
    .Bparametres{border:inset 10px azure;}
    .Bcouleurs{border:inset 4px azure;}
    .les_boutons{display:flex;justify-content:space-between;}
  </style>
  <title>Colors</title>
  </head>
  <body onload="Init();">
    <div id='lesOnglets'></div>
    <h2 >Choix des couleurs</h2>
    <div class="Zone">
        <div class="form"  id="colors"></div>
    </div>
    
    <br>
    <div class='les_boutons'>
      <input  class='bouton' type='button' onclick="CouleurDefaut();CoulPage();setCouleur();" value='Couleurs par défaut' >
        <input  class='bouton' type='button' onclick="SendValues();" value='Sauvegarder' >
    </div>
    <small>Valide après 30s ou un Ctrl+F5 pour vider le cache du navigateur.</small>
    <script>
        var BordsInverse=[".Bparametres",".Bcouleurs"];
    
        function Init(){
          SetHautBas();
          SetCurseurs();
          LoadParaRouteur(); 
          LoadCouleurs();        
        }
        function SetCurseurs(){
            var S="<div class='ligne ligneB'><div>Champ</div><div>Texte</div><div>Fond / Courbe</div><div>Bordure</div></div>";
            for (var i=0;i<Koul.length;i++){
              S +=  "<div class='ligne'>";
                  S += "<div>"+ Koul[i][0] + "</div>";
                  S += "<div><input type='color' id='text_color"+ i +"' value='#000000'  onchange='readCouleur();'></div>";
                  S += "<div><input type='color' id='bg_color"+ i +"' value='#000000'  onchange='readCouleur();'></div>";
                  S += "<div><input type='color' id='bord_color"+ i +"' value='#000000'  onchange='readCouleur();'></div>";
              S +="</div>";
            }
            GH("colors",S);

        }
        function readCouleur(){
          for (var i=0;i<Koul.length;i++){
            if (Koul[i][1]) Koul[i][1]=GID("text_color"+ i).value.substr(1); //Texte
            if (Koul[i][3]) Koul[i][3]=GID("bg_color"+ i).value.substr(1); //Back ground
            if (Koul[i][5]) Koul[i][5]=GID("bord_color"+ i).value.substr(1);
          }
          CoulPage();
          setCouleur();  
        }
        function CoulPage(){
            setColorQuery("body","#" + Koul[0][1]);
            document.body.style.background="linear-gradient(#"+Koul[0][5]+",#"+ Koul[0][3]+",#"+Koul[0][5]+")";
        }
        function SendValues(){
          var S="?couleurs=";
          for (var i=0;i<Koul.length;i++){
            if (Koul[i][1]) S +=Koul[i][1]; //Texte
            if (Koul[i][3]) S +=Koul[i][3]; //Back ground
            if (Koul[i][5]) S +=Koul[i][5];
          }
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() { 
            if (this.readyState == 4 && this.status == 200) {
              var retour=this.responseText;
              location.reload();
            }         
          };
          xhttp.open('GET', '/CouleurUpdate'+S, true);
          xhttp.send();
 

        }
        function AdaptationSource(){}
        function FinParaRouteur(){
          GID("Bheure").style.display= (Horloge>1) ? "inline-block": "none";
          GID("Bwifi").style.display= (ESP32_Type<10) ? "inline-block": "none";
        }
         
        
    </script>
    <br>
    <div id='pied'></div>
    <br>
  <script src="/ParaRouteurJS"></script>
  <script src="/CommunCouleurJS"></script>
  </body></html>
 
 )====";


const char *CommunCouleurJS = R"====(
  var Koul=[]; //Couleurs courantes
  var Coul_Page,Coul_Bout,Coul_W,Coul_VA,Coul_Wh,Coul_Tab,Coul_Graphe,Coul_Temp,Coul_Ouvre;//Index couleurs
  
  function LoadCouleurs() {
    var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() { 
          if (this.readyState == 4 && this.status == 200) {
            var Retour=this.responseText;
            if (Retour!=""){
              var L=Retour.length;
              var j=0;
              for (var i=0;i<Koul.length;i++){
                if (Koul[i][1] && (j+6)<=L) {Koul[i][1] = Retour.substring(j,j+6); j=j+6; }//Texte
                if (Koul[i][3] && (j+6)<=L) {Koul[i][3]= Retour.substring(j,j+6); j=j+6; } //Back ground
                if (Koul[i][5] && (j+6)<=L) {Koul[i][5]= Retour.substring(j,j+6); j=j+6; } //Border
              }
            }
            setCouleur();
          }         
        };
        xhttp.open('GET', '/CouleursAjax', true);
        xhttp.send();
  }
  
  function setCouleur(){
    for (var i=0;i<Koul.length;i++){
      if (Koul[i][1]){ //Texte     
        if (GID("text_color"+ i)) GID("text_color"+ i).value = "#" +Koul[i][1];
        if(Koul[i][2]) {
          for (var j=0;j<Koul[i][2].length;j++){
            setColorQuery(Koul[i][2][j],"#" + Koul[i][1]);
          }
        }
      } else {
        if (GID("text_color"+ i)) GID("text_color"+ i).style.display="none";
      }

      if (Koul[i][3]){ //Background
        if (GID("bg_color"+ i)) GID("bg_color"+ i).value = "#" +Koul[i][3];
        if(Koul[i][4]) {
          for (var j=0;j<Koul[i][4].length;j++){
            setBgColorQuery(Koul[i][4][j],"#" + Koul[i][3]);   
          }
        }
      } else {
        if (GID("bg_color"+ i)) GID("bg_color"+ i).style.display="none";
      }
      
      if (Koul[i][5]){ //Border
        if (GID("bord_color"+ i)) GID("bord_color"+ i).value = "#" +Koul[i][5];
        if (Koul[i][6]){
          for (var j=0;j<Koul[i][6].length;j++){
            setBoColorQuery(Koul[i][6][j],"#" + Koul[i][5]);   
          }
        }
      } else {
        if (GID("bord_color"+ i)) GID("bord_color"+ i).style.display="none";
      }       
    }
    
    //Inversion bouton haut page
    for (var i=0;i<BordsInverse.length;i++){
      var liste=document.querySelectorAll(BordsInverse[i]);
      for (var j=0;j<liste.length;j++){
        var rgb=liste[j].style.borderColor;
        rgb=rgb.substr(rgb.indexOf("(") +1);
        rgb=rgb.substr(0,rgb.indexOf(")"));
        var tmp = rgb.split(",");
        var hexColor="#";
        for(var k=0;k<3;k++){
            var c=Math.min(255,Math.floor(tmp[k]*1.8)); //eclaircir
            var H="00" +c.toString(16);
            hexColor += H.substr(-2);
        }
        liste[j].style.borderColor=hexColor;
      }
    }
    
  }
  
  function setColorQuery(S,C){
    var liste=document.querySelectorAll(S);
    for (var i=0;i<liste.length;i++){
      liste[i].style.color=C;
    }
  }
  function setBgColorQuery(S,C){
    var liste=document.querySelectorAll(S);
    for (var i=0;i<liste.length;i++){
      liste[i].style.background=C;
    }
  }
  function setBoColorQuery(S,C){
    var liste=document.querySelectorAll(S);
    for (var i=0;i<liste.length;i++){
      liste[i].style.borderColor=C;
    }
  }
  function CouleurDefaut(){  
    var Coul=[];
    //Format: nom,CoulTexte,QueryTexte,CoulFond,QueryFond,CoulBorder,QueryBorder
    Coul_Page = Coul.length;
    Coul.push(["Page","ffffff",,"77b5fe",,"000033",]);
    Coul_Bout = Coul.length;
    Coul.push(["Boutons haut des pages","ccccff",["a:visited","a:link"],"333333",[".Bonglet",".Bonglet2"],"808080",[".Bonglet",".Bonglet2"]]);
    Coul.push(["Champs de saisie","000000",["input"],"ffffff",["input"],"808080",["input"]]);
    Coul_Tab = Coul.length;
    Coul.push(["Tableaux","000000",[".tableau"],"ddddff",[".tableau"],"cccccc",[".tableau"]]);
    Coul_W = Coul.length;
    Coul.push(["Puissance Active en W",,,"ff4444",[".W"],,]);
    Coul_VA = Coul.length;
    Coul.push(["Puissance Apparente en VA",,,"00ffff",[".VA"],,]);
    Coul_Wh = Coul.length;
    Coul.push(["Energie Active en Wh",,,"ffff66",[".Wh"],,]);
    Coul.push(["Volt V",,,"ffaa88",[".V"],,]);
    Coul.push(["Ampère A",,,"aaffaa",[".A"],,]);
    Coul.push(["Cosinus Phi",,,"aaffee",[".phi"],,]);
    Coul.push(["Hertz et divers",,,"eeeebb",[".Hz",".Enph"],,]);
    Coul_Graphe = Coul.length;
    Coul.push(["Graphes","ffffff",,"666666",,"333333",]);
    Coul_Temp = Coul.length;
    Coul.push(["Temperature Canal 0",,,"00ff00",,,]);
    Coul.push(["Temperature Canal 1",,,"aaff00",,,]);
    Coul.push(["Temperature Canal 2",,,"00ffaa",,,]);
    Coul.push(["Temperature Canal 3",,,"aaffaa",,,]);
    Coul_Ouvre = Coul.length; 
    Coul.push(["Ouverture SSR/Triac 0",,,"ff8833",,,]);
    Coul.push(["Ouverture SSR/Triac 1",,,"33ffaa",,,]);
    Coul.push(["Ouverture SSR/Triac 2",,,"6688ff",,,]);
    Coul.push(["Ouverture SSR/Triac 3",,,"aaff11",,,]);
    Koul=Coul.slice(); //copie de travail
  }
  CouleurDefaut(); //Copie pour Couleurs courantes
 )====";