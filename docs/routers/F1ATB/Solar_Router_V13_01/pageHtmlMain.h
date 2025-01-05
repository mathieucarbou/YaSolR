//************************************************
// Page principale HTML et Javascript 
//************************************************
const char *MainHtml = R"====(
 <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="commun.css">
  
  <style>
    .ri { text-align: right;}
    .ce { text-align: center;}
    .Wh { background-color:#ff8;}
    .W { background-color:#f88;}
    .VA { background-color:#0ff;}
    .deg { background-color:#fdf;}
    .temper { background-color:#8f8;}
    .foot {font-size:16px;}
    td { text-align: left;padding:4px;}
    th { text-align: center;padding:4px;}
    svg { border:10px inset azure;}
    #LED{position:absolute;top:4px;left:4px;width:0px;height:0px;border:5px solid red;border-radius:5px;}
    .Baccueil{border:inset 8px azure;}
    .jauge{background-color:#ff8;height:28px;text-align:left;overflow: visible;position:absolute;top:4px;left:4px;}
    .jaugeBack{background-color:aqua;width:208px;height:36px;position:relative;padding:4px;}
    .w100{width:100%;position:absolute;top:4px;left:4px;}
    .centrer{text-align:center;}
    .dispT,#SVG_PW48hT,#SVG_PW2sT,.SVG_Temp48h,#SVG_Ouvertures{display:none;}
    #donneeDistante{font-size:50%;color:white;text-align:center;margin-bottom:10px;display:none;}
    #info{position:absolute;border-left: 1px solid black;display:none;}
    #info_txt{position:absolute;background-color:rgba(120, 120, 120, 0.7);padding:4px;right:10px;border: 1px solid black;text-align: right;}
    #couleurTarif_jour,#couleurTarif_J1{font-size:8px;}
    #etatActions{display:none;}
    th,td{border:2px solid grey;}
  </style>
  </head>
  <body onload='Init();' >
    <div id='LED'></div>
    <div id='lesOnglets'></div>
    <div id='date'>DATE</div>
    <div><div class='tableau'><table >
      <tr><th></th><th colspan='2' id='nomSondeMobile'>Maison</th><th colspan='2' class='dispT' id='nomSondeFixe'>Fixe</th><th id='couleurTarif_jour'></th></tr>
      <tr><th></th><th>Soutirée</th><th>Injectée</th><th class='dispT'>Conso.</th><th class='dispT' id="produite">Produite</th><th id='couleurTarif_J1'></th></tr>
      <tr class='W'><td>Puissance Active <small>(Pw)</small></td><td id='PwS_M' class='ri'></td><td class='ri' id='PwI_M'></td><td class='dispT ri' id='PwS_T'></td><td class='dispT ri' id='PwI_T'></td><td>W</td></tr>
      <tr class='VA'  id='ligneVA'><td>Puissance Apparente</td><td id='PVAS_M' class='ri'></td><td class='ri'  id='PVAI_M'></td><td class='dispT ri' id='PVAS_T'></td><td class='dispT ri'  id='PVAI_T'></td><td>VA</td></tr>
      <tr class='Wh'><td>Energie Active du jour</td><td id='EAJS_M' class='ri'></td><td id='EAJI_M' class='ri'></td><td class='dispT ri' id='EAJS_T'></td><td class='dispT ri' id='EAJI_T'></td><td>Wh</td></tr>
      <tr class='Wh'><td>Energie Active Totale</td><td id='EAS_M' class='ri'></td><td id='EAI_M' class='ri'></td><td class='dispT ri' id='EAS_T'></td><td class='dispT ri' id='EAI_T'></td><td>Wh</td></tr>
    </table></div></div>
    <div id="donneeDistante">Données distantes</div>
    <div id='etatActions' class='tableau'></div>
    <p id='SVG_PW2sM' ></p>
    <p id='SVG_PW2sT' ></p>
    <p id='SVG_PW48hM' ></p>
    <p id='SVG_PW48hT' ></p>
    <p class='SVG_Temp48h' id='SVG_Temp48h0'></p><p class='SVG_Temp48h' id='SVG_Temp48h1'></p><p class='SVG_Temp48h' id='SVG_Temp48h2'></p><p class='SVG_Temp48h' id='SVG_Temp48h3'></p>
    <p id='SVG_Ouvertures' ></p>
    <p id='SVG_Wh1an' ></p>
    <div id='info'><div id='info_txt'></div></div>
    <br><br><div class='foot' >Donn&eacute;es  RMS<div id='source'></div></div>
    <div id='pied'></div>
    <script src="/ParaRouteurJS"></script>
    <script src='MainJS'></script>
    <script src="/CommunCouleurJS"></script>
    <br></body></html>
)====";

const char *MainJS = R"====(
    var tabPW2sM=[];
    var tabPW2sT=[];
    var initUxIx2=false;
    var biSonde=false;
    var TabVal = [];
    var TabCoul= [];
    var myTimeout;
    var myActionTimeout;
    var ActionForce =[];
    var Pva_valide =false;
    var BordsInverse=[".Baccueil"];
    function LoadData() {
      GID('LED').style='display:block;';
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
            var DuRMS=this.responseText;
            var groupes=DuRMS.split(GS);
            var G0=groupes[0].split(RS);
            var G1=groupes[1].split(RS);
            var G2=groupes[2].split(RS);
            GID('date').innerHTML = G0[1];
            Source_data= G0[2];
            if (!initUxIx2){
              initUxIx2=true;
              var d='none';
              if(groupes.length==4 && nomSondeFixe!=""){ // Cas pour les sources externes UxIx2 et Shelly monophasé Si deuxième canal nommé
                d="table-cell";
              }
              const collection = document.getElementsByClassName('dispT');
              for (let i = 0; i < collection.length; i++) {
                collection[i].style.display = d;
              }      
            }           
            GID('PwS_M').innerHTML = LaVal(G1[0]); //Maison
            GID('PwI_M').innerHTML = LaVal(G1[1]); //Maison
            GID('PVAS_M').innerHTML = LaVal(G1[2]); //Maison
            GID('PVAI_M').innerHTML = LaVal(G1[3]); //Maison
            GID('EAJS_M').innerHTML = LaVal(G1[4]);
            GID('EAJI_M').innerHTML = LaVal(G1[5]);
            GID('EAS_M').innerHTML = LaVal(G1[6]); 
            GID('EAI_M').innerHTML = LaVal(G1[7]); 
            tabPW2sM.shift(); //Enleve Pw Maison
            tabPW2sM.shift(); //Enleve PVA
            tabPW2sM.push(parseFloat(G1[0]-G1[1]));
            tabPW2sM.push(parseFloat(G1[2]-G1[3]));
            Plot('SVG_PW2sM',tabPW2sM,Koul[Coul_W][3],'Puissance Active '+GID("nomSondeMobile").innerHTML+' sur 10 mn en W',Koul[Coul_VA][3],'Puissance Apparente sur 10 mn en VA');  

            var Tarif=["NON_DEFINI","PLEINE","CREUSE","BLEU","BLANC","ROUGE"];
            var couleur=["#"+Koul[Coul_Tab][3],"#f00","#0f0","#00bfff","#fff","#f00"];
            var tarif=["","H.<br>pleine","H.<br>creuse","Tempo<br>Bleu","Tempo<br>Blanc","Tempo<br>Rouge"];
            var idx=0;
            for (i=0;i<6;i++){
              if ( G0[3].indexOf(Tarif[i])>=0){ //LTARF dans Link
                idx=i;
              }
            }
            GID('couleurTarif_jour').style.backgroundColor= couleur[idx];
            GID('couleurTarif_jour').innerHTML =tarif[idx];
            var tempo = parseInt(G0[4], 16); //Tempo lendemain et jour STGE
            tempo =Math.floor(tempo/4) ; //Tempo lendemain uniquement
            idx=-2;
            var txtJ = "";
            if (tempo>0){
              idx = tempo;
              txtJ = "Tempo<br>J+1";
            }
            GID('couleurTarif_J1').style.backgroundColor= couleur[idx+2];
            GID('couleurTarif_J1').innerHTML =txtJ;
            Pva_valide = (G0[6] == 1 ) ? true:false;
            
          if (groupes.length==4 && nomSondeFixe!="") { // La source_data des données est de type UxIx2 ou on est en shelly monophas avec un deuxièeme canal
            GID('PwS_T').innerHTML = LaVal(G2[0]); //Triac
            GID('PwI_T').innerHTML = LaVal(G2[1]); //Triac
            GID('PVAS_T').innerHTML = LaVal(G2[2]); //Triac
            GID('PVAI_T').innerHTML = LaVal(G2[3]); //Triac
            GID('EAJS_T').innerHTML = LaVal(G2[4]);
            GID('EAJI_T').innerHTML = LaVal(G2[5]);      
            GID('EAS_T').innerHTML = LaVal(G2[6]);
            GID('EAI_T').innerHTML = LaVal(G2[7]); 
            tabPW2sT.shift(); //Enleve Pw Triav
            tabPW2sT.shift(); //Enleve PVA
            tabPW2sT.push(parseFloat(G2[0]-G2[1]));
            tabPW2sT.push(parseFloat(G2[2]-G2[3]));
            Plot('SVG_PW2sT',tabPW2sT,Koul[Coul_W][3],'Puissance Active '+GID("nomSondeFixe").innerHTML+' sur 10 mn en W',Koul[Coul_VA][3],'Puissance Apparente sur 10 mn en VA'); 
            if (parseInt(G2[5])==0 && Source!="ShellyEm" && Source!="ShellyPro")  { //Il n'y a pas d'injecté normalement
              GID('produite').innerHTML='';
              GID('PwI_T').innerHTML='';
              GID('PVAI_T').innerHTML='';
              GID('EAJI_T').innerHTML='';
              GID('EAI_T').innerHTML='';
            }
            biSonde=true;
          } else{
            biSonde=false;
          } 
          if (!Pva_valide) { GID('ligneVA').style='display:none;';}   
          GID('LED').style='display:none;';
          setBoColorQuery("th","#"+Koul[Coul_Tab][5]);
          setBoColorQuery("td","#"+Koul[Coul_Tab][5]);
          setTimeout('LoadData();',2000);
        }
        
      };
      xhttp.open('GET', 'ajax_data', true);
      xhttp.send();
    }
    
    function LoadHisto10mn() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
          var retour=this.responseText;
          var groupes=retour.split(GS);
          tabPW2sM.splice(0,tabPW2sM.length);
          tabPW2sM=groupes[1].split(',');
          tabPW2sM.pop();
          Plot('SVG_PW2sM',tabPW2sM,Koul[Coul_W][3],'Puissance Active '+GID("nomSondeMobile").innerHTML+' sur 10 mn en W',Koul[Coul_VA][3],'Puissance Apparente sur 10 mn en VA');
          if (biSonde){
            tabPW2sT.splice(0,tabPW2sT.length);
            tabPW2sT=groupes[2].split(',');
            tabPW2sT.pop();
            GID('SVG_PW2sT').style.display="block";
            Plot('SVG_PW2sT',tabPW2sT,Koul[Coul_W][3],'Puissance Active '+GID("nomSondeFixe").innerHTML+' sur 10 mn en W',Koul[Coul_VA][3],'Puissance Apparente sur 10 mn en VA');
          }
          LoadHisto1an();
        }
        
      };
      xhttp.open('GET', 'ajax_data10mn', true);
      xhttp.send();
    }
    function LoadHisto48h() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
          var retour=this.responseText;
          var groupes=retour.split(GS);
          var tabPWM=groupes[1].split(',');
          tabPWM.pop();
          Plot('SVG_PW48hM',tabPWM,Koul[Coul_W][3],'Puissance Active '+GID("nomSondeMobile").innerHTML+' sur 48h en W','','');
          if (biSonde){
            var tabPWT=groupes[2].split(',');
            tabPWT.pop();
            GID('SVG_PW48hT').style.display="block";
            Plot('SVG_PW48hT',tabPWT,Koul[Coul_W][3],'Puissance Active '+GID("nomSondeFixe").innerHTML+' sur 48h en W','',''); 
          }
          groupes.shift();groupes.shift();groupes.shift();
          var lesTemp=groupes[0].split("|");
          for (var c=0;c<4;c++){
            var tabTemperature=lesTemp[c].split(',');
            if (parseFloat(tabTemperature[tabTemperature.length-1])> -100) {
              tabTemperature.pop();
              GID('SVG_Temp48h'+c).style.display="block";
              Plot('SVG_Temp48h'+c,tabTemperature,Koul[Coul_Temp + c][3],nomTemperature[c]+' sur 48h ','',''); 
            }
          }
          groupes.shift();
          if (groupes.length>0) {
            Plot_ouvertures(groupes);
          }
          setTimeout('LoadHisto48h();',300000);
        }
        
      };
      xhttp.open('GET', 'ajax_histo48h', true);
      xhttp.send();
    }
    function LoadHisto1an() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
          var retour=this.responseText;
          var tabWh=retour.split(',');
          tabWh.pop();
          
          Plot('SVG_Wh1an',tabWh,Koul[Coul_Wh][3],'Energie Active Wh / Jour sur 1an','','');
          LoadHisto48h();
        }
        
      };
      xhttp.open('GET', 'ajax_histo1an', true);
      xhttp.send();
    }
    function FinParaRouteur(){
      LoadCouleurs();
      LoadData();
      LoadHisto10mn();
      EtatActions(0,0);
    };

    function Plot(SVG,Tab,couleur1,titre1,couleur2,titre2){
        var Vmax=0;
        var Vmin=0;
        var TabY0=[];
        var TabY1=[];
        couleur1 = "#" +couleur1;
        couleur2 = "#" +couleur2;
        for (var i = 0; i < Tab.length; i++) {
              Tab[i]=Math.min(Tab[i],10000000);
              Tab[i]=Math.max(Tab[i],-10000000);
              Vmax = Math.max(Math.abs(Tab[i]), Vmax);       
        }    
        var cadrageMax=1;
        var cadrage1=1000000;
        var cadrage2=[10,8,5,4,2,1];
        for (var m=0;m<7;m++){
          for (var i=0;i<cadrage2.length;i++){
              var X=cadrage1*cadrage2[i];
              if ((Vmax)<=X) cadrageMax=X;
          }
          cadrage1=cadrage1/10;
        }
       
        var dX=900/Tab.length;
        const d = new Date();
        var dI=1;
        var label='heure';
        var pixelTic=72;
        var dTextTic=4;
        var moduloText=24;
        var H0=d.getHours()+d.getMinutes()/60;
        var H00= 4*Math.floor(H0/4);
        var X0=18*(H00-H0);
        var Y0=250;
        var Yamp=230;
        var dy=2;
        switch (SVG){
          case  'SVG_PW48hM':
            
          break;
          case  'SVG_PW48hT':
            
          break;
          case 'SVG_Temp48h':
            Y0=450;
            Yamp=430;
            dy=1;
          break;
          case  'SVG_PW2sM':
            label='mn';
            pixelTic=90;
            X0=0;
            dTextTic=1;
            moduloText=-100;
            H00= 0;
            dI=2; //2 courbes PW et PVA
          break;
          case  'SVG_PW2sT':
            label='mn';
            pixelTic=90;
            X0=0;
            dTextTic=1;
            moduloText=-100;
            H00= 0;
            dI=2; //2 courbes PW et PVA
          break;
          case  'SVG_Wh1an':
            label='Mois';
            pixelTic=dX*30.4375;//Mois moyen
            var dTextTic=1;
            moduloText=12;
            H00= d.getMonth();
            X0=dX*(1-d.getDate());
            var Mois=['Jan','Fev','Mars','Avril','Mai','Juin','Juil','Ao&ucirc;t','Sept','Oct','Nov','Dec'];
          break;
          
        }
        var c1='"' + couleur1 + '"';
        var c2='"' + couleur2 + '"';
        var cT="#"+Koul[Coul_Graphe][1];
        var style='background:linear-gradient(#' + Koul[Coul_Graphe][5] +',#' + Koul[Coul_Graphe][3] +',#' + Koul[Coul_Graphe][5] +');border-color:#' +Koul[Coul_Tab][5]+';' ;  
        var S= "<svg viewbox='0 0 1030 500' style='"+style+ "' height='500' width='100%' id='S_" + SVG +"' onmousemove ='DispVal(this,event);' >"; //   
        S += "<line x1='100' y1='20' x2='100' y2='480' style='stroke:" + cT +";stroke-width:2' />";
        S += "<line x1='100' y1='" + Y0 + "' x2='1000' y2='" + Y0 + "' style='stroke:" + cT +";stroke-width:2' />";
        
        for (var x=1000+X0;x>100;x=x-pixelTic){
          var X=x;
          var Y2=Y0+6;
          S +="<line x1='"+X+"' y1='" + Y0 + "' x2='"+X+"' y2='" + Y2 + "' style='stroke:" + cT +";stroke-width:2' />";
          X=X-8;
          Y2=Y0+22;
          if (SVG=='SVG_Wh1an') {
            X=X+8;
            S +="<text x='"+X+"' y='" + Y2 + "' style='font-size:16px;fill:" + cT +";'>"+Mois[H00]+"</text>";
          }else{
            S +="<text x='"+X+"' y='" + Y2 + "' style='font-size:16px;fill:" + cT +";'>"+H00+"</text>";
          }
          H00=(H00-dTextTic+moduloText)%moduloText;
        }
        Y2=Y0-3;
        S +="<text x='980' y='" + Y2 + "' style='font-size:14px;fill:" + cT +";'>"+label+"</text>";
        for (var y=-10 ;y<=10;y=y+dy){
          
          Y2=Y0-Yamp*y/10;
          if (Y2<=480){
            S +="<line x1='100' y1='"+Y2+"' x2='1000' y2='"+Y2+"' style='stroke:" + cT +";stroke-width:1;stroke-dasharray:2 10;' />";
            Y2=Y2+7;
            var T=cadrageMax*y/10;T=T.toString();
            var X=90-9*T.length;
            S +="<text x='"+X+"' y='"+Y2+"' style='font-size:16px;fill:" + cT +";'>"+T+"</text>";
          }
        }
        if (dI==2 && Pva_valide){ //Puissance apparente 
          S +="<text x='450' y='40' style='font-size:18px;fill:"+couleur2+";'>"+titre2+"</text>";
          S += "<polyline points='"; 
            var j=0;       
            for (var i = 1; i < Tab.length; i = i+dI) {
              var Y = Y0 - Yamp * Tab[i] / cadrageMax;
              var X = 100+dX * i;
              S += X + "," + Y + " ";
              TabY1[j]=parseFloat(Tab[i]);
              j++;
            }
          S += "' style='fill:none;stroke:"+couleur2+";stroke-width:4' />";
        }
        S +="<text x='450' y='18' style='font-size:18px;fill:"+couleur1+";'>"+titre1+"</text>";
        S += "<polyline points='";   
          var j=0;     
          for (var i = 0; i < Tab.length; i = i+dI) {
            var Y = Y0 - Yamp * Tab[i] / cadrageMax;
            var X = 100+dX * i;
            S += X + "," + Y + " ";
            TabY0[j]=parseFloat(Tab[i]);
            j++;
          }
        S += "' style='fill:none;stroke:"+couleur1+";stroke-width:4' />";
        S += "</svg>";
        GID(SVG).innerHTML = S;
        TabVal["S_" + SVG]=[TabY0,TabY1];; //Sauvegarde valeurs
        TabCoul["S_" + SVG]=[couleur1, couleur2];
        
    }
    
    function DispVal(t,evt){
      var ClientRect =  t.getBoundingClientRect();
      var largeur_svg=ClientRect.right-ClientRect.left-20; //20 pixels de marge
      var x= Math.round(evt.clientX - ClientRect.left-10);
      x=x*1030/largeur_svg; 
      var  p=Math.floor((x-100)*TabVal[t.id][0].length/900);
      if (p>=0 && p<TabVal[t.id][0].length){
        var S="";
        for (j=0;j<TabVal[t.id].length;j++) {
          if (TabVal[t.id][j].length>0) {
            S +="<div style='color:"+TabCoul[t.id][j]  + ";'>" + TabVal[t.id][j][p] + "</div>";
          }
        }
        x = evt.pageX;
        GID("info").style.left=x + "px";
        x = ClientRect.top+10 +window.scrollY;
        GID("info").style.top=x +"px";
        x=evt.pageY- x;
        GID("info_txt").style.top=x +"px";
        x = ClientRect.height-20;
        GID("info").style.height=x +"px";
        GH("info_txt",S);
        GID("info").style.display="block";
        if (myTimeout !=null) clearTimeout(myTimeout);
        myTimeout=setTimeout(stopAffiche, 5000);
      }
      
    }

    function stopAffiche(){
      GID("info").style.display="none";
    }
    function Plot_ouvertures(Gr){
      GID("SVG_Ouvertures").style.display="block";
      const d = new Date();
      var label='heure';
      var pixelTic=72;
      var dTextTic=4;
      var moduloText=24;
      var H0=d.getHours()+d.getMinutes()/60;
      var H00= 4*Math.floor(H0/4);
      var X0=18*(H00-H0);
      var Hmax=50+150*Gr.length;
      var Y0=Hmax-50;
      var Couls=["#"+Koul[Coul_Ouvre][3],"#"+Koul[Coul_Ouvre+1][3],"#"+Koul[Coul_Ouvre+2][3],"#"+Koul[Coul_Ouvre+3][3]];
      var LesVals =[];
      var LesCouls =[];
      var cT="#"+Koul[Coul_Graphe][1];
      var style='background:linear-gradient(#' + Koul[Coul_Graphe][5] +',#' + Koul[Coul_Graphe][3] +',#' + Koul[Coul_Graphe][5] +');border-color:#' +Koul[Coul_Tab][5]+';' ;  
      var S= "<svg viewbox='0 0 1030 " + Hmax + "' height='" + Hmax + "'  style='"+style+ "' width='100%' id='S_Ouvertures'  onmousemove ='DispVal(this,event);'>";
      S += "<line x1='100' y1='"+Y0+"' x2='1000' y2='"+Y0+"' style='stroke:" + cT +";stroke-width:2' />";
      for (var x=1000+X0;x>100;x=x-pixelTic){
        var X=x;
        var Y2=Y0+6;
        S +="<line x1='"+X+"' y1='"+Y0+"' x2='"+X+"' y2='"+Y2+"' style='stroke:" + cT +";stroke-width:2' />";
        X=X-8;
        Y2=Y0+22;
        S +="<text x='"+X+"' y='" + Y2 + "' style='font-size:16px;fill:" + cT +";'>"+H00+"</text>";
        H00=(H00-dTextTic+moduloText)%moduloText;
      }
      for (var i=0;i<Gr.length;i++){
        var tableau=Gr[i].split(RS);
        var Y00=(i+1)*150;
        var Y2=Y00-110;
        S +="<text x='450' y='" + Y2 + "' style='font-size:18px;fill:" + Couls[i%4] + ";'>"+ tableau.pop() +"</text>";
        S += "<line x1='100' y1='"+Y00+"' x2='1000' y2='"+Y00+"' style='stroke:" + cT +";stroke-width:1;' />";
        Y2= Y00 - 100;
        S += "<line x1='100' y1='"+Y2+"' x2='1000' y2='"+Y2+"' style='stroke:" + cT +";stroke-width:1;stroke-dasharray:5 10;' />";
        Y2= Y00 +7;
        S +="<text x='80' y='"+Y2+"' style='font-size:16px;fill:" + cT +";'>0</text>";
        Y2= Y00 -93;
        S +="<text x='65' y='"+Y2+"' style='font-size:16px;fill:" + cT +";'>100</text>";
        Y2 = Y00-100;
        S += "<line x1='100' y1='"+Y00+"' x2='100' y2='"+Y2+"' style='stroke:" + cT +";stroke-width:1;' />";
        S += "<polyline points='";      
          for (var j = 0; j < tableau.length; j++) {
            var Y = Y00 - tableau[j];
            var X = 100+1.5 * j;
            S += X + "," + Y + " ";
          }
        S += "' style='fill:none;stroke:" + Couls[i%4] + ";stroke-width:3' />";
        
        LesVals.push(tableau); //Sauvegarde valeurs
        LesCouls.push(Couls[i%4]);
      }
      S += "</svg>";
      TabVal["S_Ouvertures"] = LesVals;
      TabCoul["S_Ouvertures"] = LesCouls;
      GID("SVG_Ouvertures").innerHTML = S;
      
    }
    function EtatActions(Force,NumAction) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() { 
        if (this.readyState == 4 && this.status == 200) {
          var retour=this.responseText;
          var message=retour.split(GS);
          var LesTemp=message[0].split("|");       
          Source_data=message[1];
          var T="";
          for (var c=0;c<4;c++){
            if(LesTemp[c]>-100){
                var Temper=parseFloat(LesTemp[c]).toFixed(1);
                T +="<tr class='temper'><td>" + nomTemperature[c] +"</td><td class='centrer'>"+Temper+"°C</td><td colspan='4' style='visibility: hidden;'></td></tr>";
            }
          }
          var S="";
          if (message[3]>0){ //Nb Actions 
            ActionForce.splice(0, ActionForce.length);           
            for (var i=0;i<message[3];i++){ 
              var data=message[i+4].split(RS);
              ActionForce[i]= data[3];
              S+="<tr><td>"+data[1]+"</td>";
              if (data[2]=="On" || data[2]=="Off"){
                S+="<td><div class='centrer'>"+data[2]+"</div></td>";
              } else {
                var W=1+1.99*data[2];
                S+="<td><div class='jaugeBack'><div class='jauge' style='width:"+W+"px'></div><div class='centrer w100'>"+data[2]+"%</div></div></td>";
              }
              S+="<td><div class='centrer'>"+Hdeci2Hmn(data[4])+"</div></td>";
              var stOn=(ActionForce[i]>0) ? "style='background-color:#f66;'":"";
              var stOff=(ActionForce[i]<0) ? "style='background-color:#f66;'":"";
              var min=(ActionForce[i]==0) ? "&nbsp;&nbsp;":Math.abs(ActionForce[i]) +" min";
              S +="<td><input type='button' onclick='Force(" +data[0] +",1);' value='On' " +stOn+"></td><td><small>"+min+"</small></td><td><input type='button' onclick='Force(" +data[0] +",-1);' value='Off' " + stOff + "></td></tr>";
            }
          }
          S=S+T;
          if (S!=""){
            S="<div><table ><tr><th colspan='2'>Etat Action(s)</th><th>H<div class='fsize10'>ouverture équivalente</div></th><th colspan='3'> Forçage</th></tr>" +S + "</table></div>";
            GH("etatActions",S);
            setBoColorQuery("th","#"+Koul[Coul_Tab][5]);
            setBoColorQuery("td","#"+Koul[Coul_Tab][5]);
            GID("etatActions").style.display="inline-block";
          }
          myActionTimeout=setTimeout('EtatActions(0,0);',3500);
        }
        
      };
      xhttp.open('GET', 'ajax_etatActions?Force=' +Force + '&NumAction=' + NumAction, true);
      xhttp.send();
    }
    
    function LaVal(d){
        d=parseInt(d);
        d='           '+d.toString();
        return d.substr(-9,3)+' '+d.substr(-6,3)+' '+d.substr(-3,3);
    }
    function Force(NumAction,Force){
        if (myActionTimeout !=null) clearTimeout(myActionTimeout);
        EtatActions(Force,NumAction);
    }
    
    function AdaptationSource(){
      var d='none';
      if(biSonde){
        d="table-cell";
      }
      const collection = document.getElementsByClassName('dispT');
      for (let i = 0; i < collection.length; i++) {
        collection[i].style.display = d;
      } 
      
      var S='Source : ' 
      if(Source=="Ext"){  
        S +='ESP distant '+int2ip(RMSextIP);
        GID("donneeDistante").style.display="block";
      }else {
        S +='ESP local';
      }
      GH('source',S);
    }
    function Init(){
      SetHautBas();
      LoadParaRouteur();
    }
)====";
