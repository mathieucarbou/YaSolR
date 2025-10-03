//************************************************
// Page HTML et Javascript de gestion des Actions
//************************************************

const char *ActionsHtml = R"====(
   <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="/commun.css">
  
  <style>
   
    .Bactions{border:inset 8px azure;}
    .cadre{width:100%;max-width:1200px;margin:auto;}
    .form{width:100%;text-align:left;} 
    .form2 {margin:auto;padding:5px;display: table;text-align:left;width:100%;}
    .titre{display:flex;justify-content:center;cursor:pointer;color:black;font-weight:bold;font-size:110%;}
    .slideTriac{width:100%;position:relative;margin:4px;padding:4px;border:2px inset grey;background-color:#fff8f8;color:black;font-size:14px;}
    .slideTriacIn{display:flex;justify-content:center;width:100%;}
    .planning,#CACSI,#Freq_PWM{width:100%;position:relative;margin:4px;padding:2px;border:2px inset grey;background-color:white;color:black;border-radius:8px;}
    #CACSI,#Freq_PWM{background-color:#ffa;}
    #commun,#CACSI{display:none;}
    .periode{position:absolute;height:100%;border:outset 4px;border-radius:10px;display:flex;justify-content:space-between;align-items: center;color:white;cursor:ew-resize;}
    .curseur{position:relative;width:100%;height:30px;}
    .infoAction{position:relative;display:flex;width:100%;min-height:40px;font-size:20px;}
    .infoZone{position:relative;display:flex;justify-content: space-around;border:2px inset;align-items: center;text-align:center;background-color:#aaa;cursor:pointer;}
    .infoZone div{display:inline-block;}
    .infoZ{position:absolute;bottom:10px;left:10px;display:none;border:8px inset #555;border-radius:8px;background-color:#ddd;z-index:1000;font-size:16px;}
    .infoZ input{width:70px;}
    .radioC{border:3px outset grey;border-radius:6px;display:inline-block;text-align:left;width:220px;background-color:rgba(50,50,50,0.3);}
    .radioC input{width:20px;cursor:pointer;}
    .Hfin{position:absolute;bottom:2px;right:2px;}
    .zOff{background-color:#66f;margin:4px;padding:4px;border-radius:8px;}
    .zOn{background-color:#f66;margin:2px;padding:4px;border-radius:8px;}
    .zPw{background-color:#6f6;margin:2px;padding:4px;border-radius:8px;}
    .zTriac{background-color:#cc4;margin:2px;padding:4px;border-radius:8px;}
    .selectZ{background-color:#aaa;height:20px;font-size:16px;font-weight: bold;padding-left:6px;}
    .closeZ{background-color:black;border:outset 2px grey;color:white;position:absolute;top:0px;height:20px;width:20px;right:0px;cursor:pointer;}
    .fcontainer{display: flex;  justify-content: center;background-color:#eea;margin:2px;border-top:2px solid grey;}
    .fcontleft{display:flex;justify-content: space-around;flex-direction: column;}
    .tm{width:60px;text-align:left;}
    .tbut{width:40px;padding-left:10px;padding-right:10px;text-align:center;font-weight:bold;font-size:24px;cursor:pointer;display:inline-flex;}
    .ligne {display: table-row;padding:10px;} 
    tr {margin: 2px;text-align:left;font-size:20px;}
    h4{padding:2px;margin:0px;}
    h5{text-align:left;padding:2px;margin:0px;}
    label{text-align:right;}
    .source label{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:26px;width:initial;}
    input {margin: 5px;text-align:left;font-size:15px;max-width:150px;}
    #message{position:fixed;border:inset 4px grey;top:2px;right:2px;background-color:#333;color:white;font-size:16px;display:none;text-align:left;padding:5px;}
    .bord1px{border:solid 1px grey; margin:4px;padding:2px;border-radius:4px;}
    #mode,#option,.bouton_curseur,.les_select{display:flex;justify-content: space-between;font-size:20px;text-align:center;}
    #mode,#option,.les_select{background-color:#ddd;border-radius:4px;margin:4px;padding:3px;}
    .boutons{display:inline-flex;}
    .TitZone{font-size:12px;font-style: italic;font-weight: bold;}
    .minmax{display: flex;  justify-content: center;align-items: center;}
    .minmax div{margin-right:5px;margin-left:5px;}
  </style>
  <title>Actions</title>
  </head>
  <body onLoad="SetHautBas();Init();" onmouseup='mouseClick=false;' >
    <div class="cadre">
      <div id="lesOnglets"></div>
      <h4>Planning des Routages <small>(suivant <span id='nomSondeMobile'>sonde Maison</span>)</small></h4>
      <h5 id="TitrTriac" >Routage via Triac</h5>
      <div class="form"   >
        <div id="planning0" class="planning" ></div> 
      </div>
		  <h5>Routage via Relais</h5>
      <div class="form"   >
        <div id="plannings"></div>
      </div> 
      <div id="commun"> 
        <br>
        <h4>Paramètres communs aux Actions</h4>   
        <div id="CACSI"> 
          <div>Réactivité si CACSI <span class='fsize10'>Augmentée pour les valeurs de puissance inférieures au seuil Pw. Pour tous les SSR ou Triac.</span></div>
          <div class="form2"  >
            <div class="ligne source">
              <label for='CACSI1' style='text-align:right;'>Pas d'augmentation</label>
              <input type='radio' name='ReacCACSI' id='CACSI1' value="1" checked>
              <label for='CACSI2' style='text-align:right;'>Réactivité x 2</label>
              <input type='radio' name='ReacCACSI' id='CACSI2' value="2" >
              <label for='CACSI4' style='text-align:right;'>Réactivité x 4</label>
              <input type='radio' name='ReacCACSI' id='CACSI4' value="4" >
              <label for='CACSI8' style='text-align:right;'>Réactivité x 8</label>
              <input type='radio' name='ReacCACSI' id='CACSI8' value="8" >
            </div>
          </div>
        </div>
        <div id="Freq_PWM"> 
          <div>Fréquence signaux PWM <span class='fsize10'> Nécessite un Reset de l'ESP32</span></div>
          <div class="form2"  >
            <div class="ligne source">
              <label for='Fpwm5' style='text-align:right;'>5 Hz</label>
              <input type='radio' name='Fpwm' id='Fpwm5' value="5" >
              <label for='Fpwm50' style='text-align:right;'>50 Hz</label>
              <input type='radio' name='Fpwm' id='Fpwm50' value="50" >
              <label for='Fpwm500' style='text-align:right;'>500 Hz</label>
              <input type='radio' name='Fpwm' id='Fpwm500' value="500" checked>
              <label for='Fpwm5000' style='text-align:right;'>5000 Hz</label>
              <input type='radio' name='Fpwm' id='Fpwm5000' value="5000" >
              <label for='Fpwm50000' style='text-align:right;'>50000 Hz</label>
              <input type='radio' name='Fpwm' id='Fpwm50000' value="50000" >
            </div>
          </div>
        </div>
      </div>
      
      <div  id='BoutonsBas'>        
        <br><input  class='bouton' type='button' onclick="SendValues();" value='Sauvegarder' >
        <div class="lds-dual-ring" id="attente"></div>
        <input  class='bouton' type='button' onclick='Reset();' value='ESP32 Reset' >
      </div>
    </div>
    <div id="message"></div><br>
    <div id='pied'></div> 
    <script src="/ParaRouteurJS"></script>
    <script src="/ActionsJS"></script>
    <script src="/PinsActionsJS"></script>
    <script src="/CommunCouleurJS"></script>
  </body></html>
)====";
const char *ActionsJS = R"====(
  var LesActions = [];
  var mouseClick = false;
  var blockEvent = false;
  var LesTemperatures = [];
  var NomTemperatures = [];
  var ListeActions=[];
  var SelectActions="";
  var LTARFbin=0;
  var pTriac=0;
  var ReacCACSI=1;
  var IS="|"; //Input Separator
  var BordsInverse=[".Bactions"];
  function Init() {
      LoadActions();
      DispTimer();
      LoadCouleurs();      
  }
  function creerAction(aActif, aTitre, aHost, aPort, aOrdreOn, aOrdreOff, aRepet,aTempo,aReactivite, aPeriodes) {
      var S = {
          Actif: aActif,
          Titre: aTitre,
          Host: aHost,
          Port: aPort,
          OrdreOn: aOrdreOn,
          OrdreOff: aOrdreOff,
          Repet: aRepet,
          Tempo: aTempo,
          Reactivite: aReactivite,
          Periodes: aPeriodes     
      }
      return S;
  }
  function TracePlanning(iAct) {
      var Radio0 = "<div ><input type='radio' name='modeactif" + iAct +"' id='radio" + iAct +"-0' onclick='checkDisabled();'>Inactif</div>";
      var Radio1 = "<div ><input type='radio' name='modeactif" + iAct +"' id='radio" + iAct +"-1'  onclick='checkDisabled();'>Découpe sinus</div>";
      if (iAct > 0){Radio1 = "<div ><input type='radio' name='modeactif" + iAct +"' id='radio" + iAct +"-1'  onclick='checkDisabled();'>On/Off</div>";}
      Radio1 += "<div ><input type='radio' name='modeactif" + iAct +"' id='radio" + iAct +"-2'  onclick='checkDisabled();'>Multi-sinus</div>";
      Radio1 += "<div ><input type='radio' name='modeactif" + iAct +"' id='radio" + iAct +"-3'  onclick='checkDisabled();'>Train de sinus</div>";
      Radio1 += "<div id='Pwm" + iAct + "'><input type='radio' name='modeactif" + iAct +"' id='radio" + iAct +"-4'  onclick='checkDisabled();'>PWM</div>";
      var SelectPin="<div>Gpio <select id='selectPin"+iAct +"' onchange='checkDisabled();' onmousemove='Disp(this)'>";
      for (var i=0;i<Pins.length;i++){
        var v="gpio:"+Pins[i];
        if (Pins[i]==0) v="";
        if (Pins[i]==-1) v="Externe";
        SelectPin +="<option value="+Pins[i]+">"+v+"</option>";
      }
      SelectPin +="</select></div>";
      var SelectOut="<div id='SelectOut" + iAct +"'>Sortie 'On' <select id='selectOut"+iAct +"' onmousemove='Disp(this)'><option value=0>0V</option><option value=1 selected>3.3V</option></select></div>";
      
      var S = "<div class='titre'><span id ='titre" + iAct + "' onmousemove='Disp(this)' onclick='editTitre(" + iAct + ")'>Titre</span></div>";
      S +="<div  id='mode' onmousemove='Disp(this)'><div class='TitZone'>Mode</div>" +Radio0 + Radio1 + "</div>";
      S +="<div id='blocPlanning"+iAct+"' >";
        S += "<div class='les_select' id='sortie" +iAct +"'>";      
          S +=  "<div class='TitZone'>Sortie</div>" + SelectPin+SelectOut;
          S += "<div><span id='Tempo" + iAct + "'>Temporisation(s) <input type='number' class='tm' id='tempo" + iAct + "' onmousemove='Disp(this)'></span></div>";
        S += "</div><div class='les_select' id='ligne_bas" + iAct + "'><div class='TitZone'>Externe</div>";
          S += "<div><span id='Host" + iAct + "'>Host<br><input type='text' id='host" + iAct + "' onmousemove='Disp(this)' onchange='checkDisabled();' ></span></div>";   
          S += "<div><span id='Port" + iAct + "'>Port<br><input type='number' class='tm' id='port" + iAct + "' onmousemove='Disp(this)'></span></div>";
          S += "<div><span id='ordreon" + iAct +"'>Ordre On<br><input type='text' id='ordreOn" + iAct + "' onmousemove='Disp(this)'></span></div>";
          S += "<div><span id='ordreoff" + iAct +"'>Ordre Off<br><input type='text' id='ordreOff" + iAct + "' onmousemove='Disp(this)'></span></div>";
          S += "<div><span id='Repet" + iAct + "'>Répétition(s)<br><input type='number' id='repet" + iAct + "' class='tm' onmousemove='Disp(this)'></span></div>";
        S += "</div>";

        S +="<div  class='bouton_curseur' ><div class='boutons'><input id='adds' type='button' value='-' class='tbut'  onclick='AddSub(-1," + iAct + ")' onmousemove='Disp(this)' >";
        S +="<input id='adds' type='button' value='+' class='tbut' onclick='AddSub(1," + iAct + ")' onmousemove='Disp(this)'></div>";
        S +="<div class='slideTriac' id='fen_slide" + iAct +"'><div class='slideTriacIn'>";
              S +="<div>R&eacute;activit&eacute; lente ou charge importante</div>";
              S +="<input type='range' min='1' max='100' value='50' id='slider" + iAct + "' style='width:30%;' oninput=\"GH('sensi" + iAct +"',Math.floor(this.value));\" onmousemove='Disp(this)' >";
              S +="<div>R&eacute;activit&eacute; rapide ou charge faible</div><br>";
        S +="</div><div class='slideTriacIn'><strong><div id='sensi" + iAct + "'></div></strong></div>";
        S +="</div></div>";
        S += "<div id='infoAction" + iAct + "' class='infoAction'></div>";
        S += "<div id='curseurs" + iAct + "' class='curseur'  onmousedown='mouseClick=true;'  onmousemove='mouseMove(this,event," + iAct + ");'  ontouchstart='touchMove(this,event," + iAct + ");'  ontouchmove='touchMove(this,event," + iAct + ");' ></div>";
      S += "</div>";

      GH("planning" + iAct, S);
      GID("radio" + iAct +"-" +LesActions[iAct].Actif).checked = true;
      GH("titre" + iAct, LesActions[iAct].Titre);
      GV("host" + iAct, LesActions[iAct].Host);
      GV("port" + iAct, LesActions[iAct].Port);
      GV("ordreOn" + iAct, LesActions[iAct].OrdreOn);
      GV("ordreOff" + iAct, LesActions[iAct].OrdreOff);
      GV("repet" + iAct, LesActions[iAct].Repet);
      GV("tempo" + iAct, LesActions[iAct].Tempo);
      GV("slider" + iAct ,LesActions[iAct].Reactivite); 
      GH("sensi" + iAct ,LesActions[iAct].Reactivite);
      if(LesActions[iAct].OrdreOn.indexOf(IS)>0){
        var vals=LesActions[iAct].OrdreOn.split(IS);
        GID("selectPin"+iAct).value=vals[0];
        GID("selectOut" + iAct).value=vals[1];
      } else {      
        GID("selectPin"+iAct).value=-1;
        GID("selectOut" + iAct).value=1;
        if(LesActions[iAct].OrdreOn=="") GID("selectPin"+iAct).value=0;
      }
      TracePeriodes(iAct);
      
  }
  
  function TracePeriodes(iAct) {
      var S = "";
      var Sinfo = "";
      var SinfoClick = "";
      var left = 0;
      var H0 = 0;
      var colors = ["#666", "#66f", "#f66", "#6f6", "#cc4"]; //NO,OFF,ON,PW,Triac
      blockEvent = false;
      for (var i = 0; i < LesActions[iAct].Periodes.length; i++) {
        var w = (LesActions[iAct].Periodes[i].Hfin - H0) /24;
        left = H0 / 24;
        H0 = LesActions[iAct].Periodes[i].Hfin;
        var Type = LesActions[iAct].Periodes[i].Type;
        var color = colors[Type];
        var temperature="";
        if(LesActions[iAct].Periodes[i].CanalTemp>=0){
          if (LesTemperatures[LesActions[iAct].Periodes[i].CanalTemp]>-100) { // La sonde de température fonctionne          
            var Tsup=LesActions[iAct].Periodes[i].Tsup;
            if (Tsup>=0 && Tsup <=1000) temperature +="<div>T &ge;" + Tsup/10 + "°</div>";
            var Tinf=LesActions[iAct].Periodes[i].Tinf;
            if (Tinf>=0 && Tinf <=1000) temperature +="<div>T &le;" + Tinf/10 + "°</div>";
          }  
        }
        var H_Ouvert="";
        if(LesActions[iAct].Periodes[i].SelAct!=255){
          if (LesActions[iAct].Periodes[i].Hmin>0 ) H_Ouvert +="<div>H<span class='fsize8'>ouverture</span> &ge;" + Hdeci2Hmn(LesActions[iAct].Periodes[i].Hmin) + "</div>";
          if (LesActions[iAct].Periodes[i].Hmax>0 ) H_Ouvert +="<div>H<span class='fsize8'>ouverture</span> &le;" + Hdeci2Hmn(LesActions[iAct].Periodes[i].Hmax) + "</div>";
          if (LesActions[iAct].Periodes[i].Ooff>0 ) H_Ouvert +="<div>On à Off si &le;" + LesActions[iAct].Periodes[i].Ooff + "%</div>";
          if (LesActions[iAct].Periodes[i].O_on>0 ) H_Ouvert +="<div>Off à On si &ge;" + LesActions[iAct].Periodes[i].O_on + "%</div>";
        }
        var TxtTarif= "";
        if (LTARFbin>0)  {   
          TxtTarif= "Tarif : ";
          var Tarif_=LesActions[iAct].Periodes[i].Tarif;
          if (LTARFbin<=3) {
            TxtTarif += (Tarif_ & 1) ? "<span style='color:red;'>H. Pleine</span>":"" ; 
            TxtTarif += (Tarif_ & 2) ? "<span style='color:green;'> H. Creuse</span>":"" ; 
          } else {
            TxtTarif += (Tarif_ & 4) ? "Tempo<span style='color:blue;'>Bleu</span>":"" ; 
            TxtTarif += (Tarif_ & 8) ? "<span style='color:white;'> Blanc</span>":"" ; 
            TxtTarif += (Tarif_ & 16) ? "<span style='color:red;'> Rouge</span>":"" ; 
          }
          TxtTarif ="<div>" + TxtTarif +"</div>";
        }
        let TexteMinMax="";
        var condition= (temperature!="" || H_Ouvert!="" || TxtTarif !="" )?  "<div>Condition(s) :</div>" + temperature +H_Ouvert+ TxtTarif:""; 
        if (LesActions[iAct].Actif<=1 && iAct>0){
          LesActions[iAct].Periodes[i].Vmax=Math.max(LesActions[iAct].Periodes[i].Vmin,LesActions[iAct].Periodes[i].Vmax);
		      TexteMinMax="<div>Off si Pw&gt;"+LesActions[iAct].Periodes[i].Vmax+"W</div><div>On si Pw&lt;"+LesActions[iAct].Periodes[i].Vmin+"W</div>"+condition;
        } else {
          LesActions[iAct].Periodes[i].Vmax=Math.max(0,LesActions[iAct].Periodes[i].Vmax);
          LesActions[iAct].Periodes[i].Vmax=Math.min(100,LesActions[iAct].Periodes[i].Vmax);
          TexteMinMax="<div>Seuil Pw : "+LesActions[iAct].Periodes[i].Vmin+"W</div>" + "<div>Ouvre Max : "+LesActions[iAct].Periodes[i].Vmax+"%</div>"+condition;
        }
		    var TexteTriac="<div>Seuil Pw : "+LesActions[iAct].Periodes[i].Vmin+"W</div>" + "<div>Ouvre Max : "+LesActions[iAct].Periodes[i].Vmax+"%</div>"+condition;
        var paras = ["Pas de contr&ocirc;le", "OFF", "<div>ON</div>" +condition, TexteMinMax, TexteTriac];
        var para = paras[Type];
        S += "<div class='periode' style='width:" + w + "%;left:" + left + "%;background-color:" + color + ";'   ><div>&lArr;</div><div>&rArr;</div></div>";
        Hmn = Hdeci2Hmn(H0);
        fs = Math.max(8, Math.min(16, w/2)) + "px";
        Sinfo += "<div class='infoZone' style='width:" + w + "%;border-color:" + color + ";font-size:" + fs + "'  onclick='infoZclicK(" + i + "," + iAct + ")'  >"
        Sinfo += "<div class='Hfin'>" + Hmn + "</div>" + para + "</div>";
        SinfoClick +="<div id='info" + iAct + "Z" + i + "' class='infoZ' ></div>";
      }
      GH("curseurs" + iAct, S);
      GH("infoAction" + iAct, SinfoClick + Sinfo);
  }
  function touchMove(t, ev, iAct) {
      var leftPos = ev.touches[0].clientX - GID(t.id).getBoundingClientRect().left;
      NewPosition(t, leftPos, iAct);
  }
  function mouseMove(t, ev, iAct) {
      if (mouseClick) {
          var leftPos = ev.clientX - GID(t.id).getBoundingClientRect().left;
          NewPosition(t, leftPos, iAct);
      }
  }


  function NewPosition(t, leftPos, iAct) {
      var G = GID(t.id).style.left;
      //+ window.scrollX;
      var width = GID(t.id).getBoundingClientRect().width;
      var HeureMouse = leftPos * 2420 / width;
      var idxClick = 0;
      var deltaX = 999999;
      for (var i = 0; i < LesActions[iAct].Periodes.length - 1; i++) {
          let dist = Math.abs(HeureMouse - LesActions[iAct].Periodes[i].Hfin)
              if (dist < deltaX) {
                  idxClick = i;
                  deltaX = dist;
              }
      }
      var NewHfin = Math.max(0, Math.min(HeureMouse, 2400));
      if (idxClick == LesActions[iAct].Periodes.length - 1) NewHfin=2400;
      if (idxClick < LesActions[iAct].Periodes.length - 1)
          NewHfin = Math.min(NewHfin, LesActions[iAct].Periodes[idxClick + 1].Hfin);
      if (idxClick > 0)
          NewHfin = Math.max(NewHfin, LesActions[iAct].Periodes[idxClick - 1].Hfin);
      LesActions[iAct].Periodes[idxClick].Hfin = Math.floor(NewHfin);
      TracePeriodes(iAct);

  }
  

  function AddSub(v, iAct) {
      if (v == 1) {
          if (LesActions[iAct].Periodes.length<8){
            LesActions[iAct].Periodes.push({
                Hfin: 2400,
                Type: 1,
                Vmin:0,
                Vmax:100,
                Tinf:1600,
                Tsup:1600,
                Hmin:0,
                Hmax:0,
                CanalTemp:-1,
                SelAct:255,
                Ooff:0,
                O_on:0,
                Tarif:31
            }); //Tarif codé en bits
            var Hbas = 0;
            if (LesActions[iAct].Periodes.length > 2){
                Hbas = parseInt(LesActions[iAct].Periodes[LesActions[iAct].Periodes.length - 3].Hfin); 
            }      
            if (LesActions[iAct].Periodes.length > 1) {
                LesActions[iAct].Periodes[LesActions[iAct].Periodes.length - 2].Hfin = Math.floor((Hbas + 2400) / 2);              
            }               
          }
      } else {
        if (LesActions[iAct].Periodes.length>1){
          LesActions[iAct].Periodes.pop();
          if (LesActions[iAct].Periodes.length > 0)
              LesActions[iAct].Periodes[LesActions[iAct].Periodes.length - 1].Hfin = 2400;
        }
      }
      TracePeriodes(iAct);
      
  }
  function infoZclicK(i, iAct) {
      var capteurT=false;
      if (!blockEvent) {
          blockEvent = true;
          var Type = LesActions[iAct].Periodes[i].Type;
          var idZ = "info" + iAct + "Z" + i;
          var S = "<div class='selectZ'> S&eacute;lection Action<div class='closeZ' onclick='infoZclose(\"" + idZ + "\")'>X</div></div>";
          //On ne traite plus depuis version8 le cas "Pas de Contrôle". Inutile
          var check = (Type == 1) ? "checked" : "";
          S += "<div class='zOff'  onmousemove='Disp(\"zOff\")'><div class='radioC' ><input type='radio'  name='R" + idZ +"' onclick='selectZ(1," + i + "," + iAct + ");' " + check +">OFF</div></div>";
          S += "<div class='fcontainer'><div class='fcontleft'>";
              check = (Type == 2) ? "checked" : "";
              S += "<div  class='zOn'  onmousemove='Disp(\"zOn\")' ><div class='radioC' ><input type='radio'  name='R" + idZ +"' onclick='selectZ(2," + i + "," + iAct + ");' " + check +">ON <small>100%</small></div></div>";
              check = (Type > 2) ? "checked" : "";
              var Vmin=LesActions[iAct].Periodes[i].Vmin;
              var Vmax=LesActions[iAct].Periodes[i].Vmax;
              var Tinf=LesActions[iAct].Periodes[i].Tinf;
              var Tsup=LesActions[iAct].Periodes[i].Tsup;
              var TinfC=Tinf/10;
              var TsupC=Tsup/10;
              var Hmin=(LesActions[iAct].Periodes[i].Hmin>0)?Hdeci2Hmn(LesActions[iAct].Periodes[i].Hmin):"";
              var Hmax=(LesActions[iAct].Periodes[i].Hmax>0)?Hdeci2Hmn(LesActions[iAct].Periodes[i].Hmax):"";
              var Ooff=(LesActions[iAct].Periodes[i].Ooff>0)?LesActions[iAct].Periodes[i].Ooff:"";
              var O_on=(LesActions[iAct].Periodes[i].O_on>0)?LesActions[iAct].Periodes[i].O_on:"";
              if (Tinf>1500 || Tinf<-500) TinfC=""; //Temperature entre -50 et 150° représenté en dixième
              if (Tsup>1500 || Tsup<-500) TsupC=""; //Temperature entre -50 et 150
              if (iAct > 0) {
                  var Routage=["","Routage ON/Off","Routage Multi-sinus","Routage Train de Sinus","PWM"];
                  S += "<div class='zPw' ><div class='radioC' ><input type='radio'  name='R" + idZ +"' onclick='selectZ(3," + i + "," + iAct + ");' " + check +">" +Routage[LesActions[iAct].Actif] + "</div>";
                  if (LesActions[iAct].Actif<=1) {
                      S += "<div><small>On : &nbsp;</small>Pw &lt;<input id='Pw_min_"+idZ+"' onmousemove='Disp(this)' type='number' value='"+Vmin+"' onchange='NewVal(this)' >W</div>";
                      S += "<div><small>Off : </small>Pw &gt;<input id='Pw_max_"+idZ+"' onmousemove='Disp(this)' type='number' value='"+Vmax+"' onchange='NewVal(this)'>W</div>";
                      S += "<div><small>Puissance active en entrée de maison</small></div></div>";
                  } else {
                    S += "<div><small>Seuil Pw : &nbsp;</small><input id='Pw_min_"+idZ+"' onmousemove='Disp(this)' type='number' value='"+Vmin+"' onchange='NewVal(this)' >W</div>";
                    S += "<div><small>Puissance active en entrée de maison</small></div>";
                    S += "<div><small>Ouvre Max : </small><input id='Pw_max_"+idZ+"'  onmousemove='Disp(\"mxTr\")' type='number' value='"+Vmax+"' onchange='NewVal(this)'>%</div></div>";
                  }
                  
              } else {
                  var Routage=["","Routage Découpe Sinus","Routage Multi-sinus","Routage Train de Sinus"];
                  S += "<div  class='zTriac' ><div class='radioC' ><input type='radio'  name='R" + idZ +"' onclick='selectZ(4," + i + "," + iAct + ");' " + check +">" +Routage[LesActions[iAct].Actif] + "</div>";
                  S += "<div>Seuil Pw &nbsp;<input id='Pw_min_"+idZ+"' onmousemove='Disp(\"pwTr\")' type='number' value='"+Vmin+"' onchange='NewVal(this)'>W</div>";
                  S += "<div><small>Puissance active en entrée de maison</small></div>";
                  S += "<div>Ouvre Max <input id='Pw_max_"+idZ+"' onmousemove='Disp(\"mxTr\")' type='number' value='"+Vmax+"' onchange='NewVal(this)'>%</div></div>";
              }
            S += "</div>";
            var SelectT="<div>Canal de Température <select id='CanalTemp"+idZ +"' onmousemove='Disp(this)' onchange='NewVal(this)'><option value=-1 selected>Non exploité</option>";
            for (var c=0;c<4;c++){
              if (LesTemperatures[c]>-100){
                var Temper=parseFloat(LesTemperatures[c]).toFixed(1);
                SelectT +="<option value=" + c + " >" + NomTemperatures[c] + " (" +Temper + "°)" + "</option>";
                capteurT=true;
              }
            }
            SelectT +="</select></div>";
              var style=(ModePara==0) ? "none":"block";
              style ="style='display:" + style +"';";
              S += "<div>"; 
                  S +="<div class='TitZone' " + style + ">&nbsp;&nbsp;&nbsp;Conditions optionnelles pour activer</div>";
                  if (capteurT) {
                    S += "<div  class='bord1px' " + style + " onmousemove='Disp(\"tmpr\")'>";
                      S += SelectT;
                      S += "<div class='minmax'><div>T &ge;<input id='T_sup_"+idZ+"'  type='number' value='" + TsupC +"' onchange='NewVal(this)' >°</div>";
                      S += "<div>T &le;<input id='T_inf_"+idZ+"'  type='number' value='" + TinfC +"' onchange='NewVal(this)' >°</div></div>";
                      S += "<div><small>T en degré (-50.0 à 150.0) ou laisser vide</small></div>";
                    S += "</div>";
                  }
                  S += "<div  class='bord1px' " + style + " onmousemove='Disp(\"sAct\")'>";
                    S += "<div>Etat d'une Action <select id='SelAct" + idZ + "' onchange='NewVal(this)' >" + SelectActions + "</select></div>";
                    S += "<div class='minmax'><div>Durée : </div><div>H &ge;<input id='H_min_"+idZ+"'  type='text' value='" + Hmin +"' onchange='NewVal(this)' >h:mn</div>";
                    S += "<div>H &le;<input id='H_max_"+idZ+"'  type='text' value='" + Hmax +"' onchange='NewVal(this)' >h:mn</div></div>";
                    S += "<div class='minmax'><div>Seuil : </div><div>On à Off si &le;<input id='O_min_"+idZ+"'  type='number' value='" + Ooff +"' onchange='NewVal(this)' >%</div>";
                    S += "<div>Off à On si &ge;<input id='O_max_"+idZ+"'  type='number' value='" + O_on +"' onchange='NewVal(this)' >%</div></div>";
                    S += "<div><small>h:mn ou % ou laisser vide</small></div>";
                  S += "</div>";
              
                
                if (LTARFbin>0)  {   
                  S += "<div  class='bord1px' onmousemove='Disp(\"tarif\")'>";
                    S += "<div>Actif si tarif :</div>";
                    if (LTARFbin<=3) {
                      S += "<div id='PleineCreuse'><span style='color:red;'>Heure Pleine</span><input type='checkbox' checked id='TarifPl_"+idZ+"' onchange='NewVal(this)'> <span style='color:green;'>Heure Creuse</span><input type='checkbox' checked id='TarifCr_"+idZ+"' onchange='NewVal(this)'></div>";
                    } else {
                      S += "<div id='Tempo'>Tempo <span style='color:blue;'>Bleu</span><input type='checkbox' checked id='TarifBe_"+idZ+"' onchange='NewVal(this)'><span style='color:white;'> Blanc</span><input type='checkbox' checked id='TarifBa_"+idZ+"' onchange='NewVal(this)'><span style='color:red;'> Rouge</span><input type='checkbox' checked id='TarifRo_"+idZ+"' onchange='NewVal(this)'></div>";
                    } 
                  S += "</div>";
                }
              S += "</div>";
            
          S += "</div>";
          GH(idZ, S);
          if (capteurT) GID("CanalTemp"+idZ ).value=LesActions[iAct].Periodes[i].CanalTemp;
          GID("SelAct"+idZ ).value=LesActions[iAct].Periodes[i].SelAct;
          var Tarif_=LesActions[iAct].Periodes[i].Tarif;
          if (LTARFbin>0)  {
            if (LTARFbin<=3) {
              GID("TarifPl_" + idZ).checked = (Tarif_ & 1) ? 1:0 ; // H Pleine
              GID("TarifCr_" + idZ).checked = (Tarif_ & 2) ? 1:0 ;
            } else {
              GID("TarifBe_" + idZ).checked = (Tarif_ & 4) ? 1:0 ;
              GID("TarifBa_" + idZ).checked = (Tarif_ & 8) ? 1:0 ;
              GID("TarifRo_" + idZ).checked = (Tarif_ & 16) ? 1:0 ; //Rouge
            }
          }
          GID(idZ).style.display = "block";
      }
  }
  function infoZclose(idx) {
      var champs=idx.split("info");
	    var idx=champs[1].split("Z");
      S="TracePeriodes("+idx[0]+");";
      setTimeout(S, 100);
  }
  function selectZ(T, i, iAct) {
      if (LesActions[iAct].Periodes[i].Type != T) {
          LesActions[iAct].Periodes[i].Type = T;
          var idZ = "info" + iAct + "Z" + i;
          if (T <= 1)  {
              infoZclose(idZ);
              TracePeriodes(iAct);
          }
      }
  }

  function NewVal(t){
      var champs=t.id.split("info");
      var idx=champs[1].split("Z");   //Num Action, Num période
      if (champs[0].indexOf("Pw_min")>=0){
        LesActions[idx[0]].Periodes[idx[1]].Vmin=Math.floor(GID(t.id).value);
      }
      if (champs[0].indexOf("Pw_max")>=0){
        LesActions[idx[0]].Periodes[idx[1]].Vmax=Math.floor(GID(t.id).value);
        if (idx[0]==0){
          LesActions[idx[0]].Periodes[idx[1]].Vmax=Math.max(LesActions[idx[0]].Periodes[idx[1]].Vmax,5);
          LesActions[idx[0]].Periodes[idx[1]].Vmax=Math.min(LesActions[idx[0]].Periodes[idx[1]].Vmax,100);
        }
      }
      if (champs[0].indexOf("inf")>0){
        var V= GID(t.id).value;
        if (V=="") V=158;
        LesActions[idx[0]].Periodes[idx[1]].Tinf=Math.floor(V*10);
      }
      if (champs[0].indexOf("sup")>0){
        var V= GID(t.id).value;
        if (V=="") V=158;
        LesActions[idx[0]].Periodes[idx[1]].Tsup=Math.floor(V*10);
      }
      if (champs[0].indexOf("H_min")>=0){
        LesActions[idx[0]].Periodes[idx[1]].Hmin=Hmn2Hdeci(GID(t.id).value);
      }	
      if (champs[0].indexOf("H_max")>=0){
        LesActions[idx[0]].Periodes[idx[1]].Hmax=Hmn2Hdeci(GID(t.id).value);
      }	
      if (champs[0].indexOf("O_min")>=0){
        LesActions[idx[0]].Periodes[idx[1]].Ooff=Math.max(0,Math.min(100,Math.floor(GID(t.id).value)));
      }	
      if (champs[0].indexOf("O_max")>=0){
        LesActions[idx[0]].Periodes[idx[1]].O_on=Math.max(0,Math.min(100,Math.floor(GID(t.id).value)));
      }	
      if (champs[0].indexOf("Tarif")>=0){
        var idZ = "info" + champs[1];
        var Tarif_ = 0;
        if (LTARFbin<=3) {
          Tarif_ += GID("TarifPl_" + idZ).checked ? 1:0; //H pleine
          Tarif_ += GID("TarifCr_" + idZ).checked ? 2:0;
         } else {
          Tarif_ += GID("TarifBe_" + idZ).checked ? 4:0; //Bleu
          Tarif_ += GID("TarifBa_" + idZ).checked ? 8:0;
          Tarif_ += GID("TarifRo_" + idZ).checked ? 16:0; //Rouge
        }
        LesActions[idx[0]].Periodes[idx[1]].Tarif=Tarif_;
      }	
      if (champs[0].indexOf("CanalTemp")>=0) {
          LesActions[idx[0]].Periodes[idx[1]].CanalTemp=GID(t.id).value;
      }
      if (champs[0].indexOf("SelAct")>=0) {
          LesActions[idx[0]].Periodes[idx[1]].SelAct=GID(t.id).value;
      }
  }
  function editTitre(iAct) {
      if (GID("titre" + iAct).innerHTML.indexOf("<input") == -1) {
          GH("titre" + iAct, "<input type='text' value='" + GID("titre" + iAct).innerHTML + "' id='Etitre" + iAct + "'  onblur='TitreValid(" + iAct + ")' >");
          GID("Etitre" + iAct).focus();
      }
  }
  function TitreValid(iAct) {
      LesActions[iAct].Titre = GID("Etitre" + iAct).value.trim();
      GH("titre" + iAct, LesActions[iAct].Titre);
  }
  function checkDisabled(){
    GID("sortie0").style.display ="none";
    GID("Freq_PWM").style.display="none";
    GID("commun").style.display = (ModePara>0 && ReacCACSI<100 ) ? "block":"none";
    for (var iAct = 0; iAct < LesActions.length; iAct++) {
        for (var i=0;i<=4;i++){
            if( GID("radio" + iAct +"-"+ i).checked ) { LesActions[iAct].Actif =i;} //0=Inactif,1=Decoupe ou On/Off, 2=Multi, 3= Train, 4= PWM
        }
        if (GID("selectPin"+iAct).value==-1 && LesActions[iAct].Actif>1 && iAct>0){LesActions[iAct].Actif=1;GID("radio" + iAct +"-" +LesActions[iAct].Actif).checked = true;}  
        TracePeriodes(iAct);
        GID("planning0").style.display = (pTriac>0) ? "block" : "none";  // Si Pas de Triac 
        GID("TitrTriac").style.display = (pTriac>0) ? "block" : "none";  
        GID("blocPlanning"+iAct).style.display = (LesActions[iAct].Actif>0) ? "block" : "none";
        var visible = ( LesActions[iAct].Actif== 1) ? "block" : "none";
        GID("Tempo"+iAct).style.display =visible;
        var disable=true;
        var disp="block";
        if (GID("selectPin"+iAct).value>=0) { visible="none";disable=false;disp="none";} 
        GID("SelectOut"+iAct).style.display = (GID("selectPin"+iAct).value<=0) ? "none":"inline-block";       
        GID("Host"+iAct).style.display =visible;
        GID("Port"+iAct).style.display =visible;
        GID("Repet"+iAct).style.display =visible;   
        GID("radio" + iAct +"-2").disabled = disable; 
        GID("radio" + iAct +"-3").disabled = disable;
        GID("radio" + iAct +"-4").disabled = disable;
        GID("ordreoff"+iAct).style.display=disp;  
        GID("ordreon"+iAct).style.display =disp;
        if (GID("selectPin"+iAct).value==-1 && GID("ordreOn"+iAct).value.indexOf(IS)>0) GID("ordreOn"+iAct).value="";                   
        GID("ligne_bas"+iAct).style.display  =( LesActions[iAct].Actif==1 && GID("selectPin"+iAct).value<=0 && iAct>0) ?  "flex" :"none";
        GID("fen_slide"+iAct).style.display = (LesActions[iAct].Actif== 1 && iAct>0  ) ? "none" : "block";
        if( GID("radio" + iAct +"-4").checked) {
          GID("Freq_PWM").style.display="block";
          GID("commun").style.display =  "block";
        }
        
    }
    GID("Pwm0").style.display = "none"; //Pas de PWM sur la sortie Triac
  }

  function LoadActions() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
          if (this.readyState == 4 && this.status == 200) {
              var LeRetour = this.responseText;
              var Les_ACTIONS = LeRetour.split(GS);
              var LesParas = Les_ACTIONS[0].split(RS);
              LesTemperatures = LesParas[0].split("|");
              NomTemperatures = LesParas[1].split(US);
              LTARFbin = parseInt(LesParas[2]);
              pTriac = parseInt(LesParas[3]);
              ReacCACSI = LesParas[4]; //1,24 8 ou 100 pour EstimCACSI
              if (ReacCACSI<100) {
                GID("CACSI" + ReacCACSI).checked = true; //Reactivité CACSI et non Estimation
                GID("CACSI").style = "display:block;";
              }
              GID("Fpwm" + LesParas[5]).checked = true;
              LesActions.splice(0,LesActions.length);
              for (var iAct=1;iAct<Les_ACTIONS.length-1;iAct++){
                var champs=Les_ACTIONS[iAct].split(RS);
                var NbPeriodes=champs[9];
                var Periodes=[];
                var j=10;
                for (var i=0;i<NbPeriodes;i++){
                  Periodes[i]={Type:champs[j],Hfin:champs[j+1],Vmin:champs[j+2],Vmax:champs[j+3],Tinf:champs[j+4],Tsup:champs[j+5],Hmin:champs[j+6],Hmax:champs[j+7],CanalTemp:champs[j+8],SelAct:champs[j+9],Ooff:champs[j+10],O_on:champs[j+11],Tarif:champs[j+12]};
                  j=j+13;
                }
                LesActions[iAct-1]=creerAction(champs[0], champs[1], champs[2], champs[3], champs[4], champs[5], champs[6],champs[7],champs[8], Periodes);
              }    
              if (LesActions.length==0){  //Action Triac
                  LesActions.push( creerAction(0, "Titre Triac", "", 50, "", "","", 0,10, [{
                          Hfin: 2400,
                          Type: 4,
                          Vmin:0,
                          Vmax:100,
                          Tinf:1600,
                          Tsup:1600,
                          Hmin:0,
                          Hmax:0,
                          CanalTemp:-1,
                          SelAct:255,
                          Ooff:0,
                          O_on:0,
                          Tarif:31
                      }
                  ]));
              }
              LesActions.push( creerAction(0, "Titre Relais " + LesActions.length, "", 80, "", "", 240,0,10, [{
                      Hfin: 2400,
                      Type: 3,
                      Vmin:0,
                      Vmax:100,
                      Tinf:1600,
                      Tsup:1600,
                      Hmin:0,
                      Hmax:0,
                      CanalTemp:-1,
                      SelAct:255,
                      Ooff:0,
                      O_on:0,
                      Tarif:31
                  }
              ]));
              var S = "";
              for (var i = 1; i < LesActions.length; i++) {
                  S += "<div id='planning" + i + "' class='planning' ></div>";
              }
              GH("plannings", S);
              for (var iAct = 0; iAct < LesActions.length; iAct++) {
                  TracePlanning(iAct);
              }
              checkDisabled();
              LoadParaRouteur();
          }
      };
      xhttp.open('GET', '/ActionsAjax', true);
      xhttp.send();
  }
  
  
  function SendValues() {
      GID("attente").style="visibility: visible;";
      for (var iAct = 0; iAct < LesActions.length; iAct++) {
        for (var i=0;i<=4;i++){
            if( GID("radio" + iAct +"-"+ i).checked ) { LesActions[iAct].Actif =i;}
        }
        LesActions[iAct].Titre = GID("titre" + iAct).innerHTML.trim();
        LesActions[iAct].Host = GID("host" + iAct).value.trim();
        LesActions[iAct].Port = GID("port" + iAct).value;
        LesActions[iAct].OrdreOn = GID("ordreOn" + iAct).value.trim();
        LesActions[iAct].OrdreOff = GID("ordreOff" + iAct).value.trim();
        LesActions[iAct].Repet = GID("repet" + iAct).value;
        LesActions[iAct].Tempo = GID("tempo" + iAct).value;
        LesActions[iAct].Reactivite = GID("slider" + iAct).value;
        if (GID("selectPin"+iAct).value>=0) LesActions[iAct].OrdreOn=GID("selectPin"+iAct).value +IS + GID("selectOut"+iAct).value;
        if (iAct>0 && (GID("selectPin"+iAct).value==0 || LesActions[iAct].Titre=="")) LesActions[iAct].Actif=-1; //Action à effacer
      }
      var S="";
      for (var iAct = 0; iAct < LesActions.length; iAct++) {
        if (LesActions[iAct].Actif>=0 ){
            S +=LesActions[iAct].Actif+RS+LesActions[iAct].Titre+RS;
            S +=LesActions[iAct].Host+RS+LesActions[iAct].Port+RS;
            S +=LesActions[iAct].OrdreOn+RS+LesActions[iAct].OrdreOff+RS+LesActions[iAct].Repet+RS+LesActions[iAct].Tempo+RS;
            S +=LesActions[iAct].Reactivite + RS + LesActions[iAct].Periodes.length+RS;
            for (var i=0;i<LesActions[iAct].Periodes.length;i++){
              if(ModePara==0){ //Standard
                LesActions[iAct].Periodes[i].CanalTemp=-1;
                LesActions[iAct].Periodes[i].SelAct =255;
              }
              S +=LesActions[iAct].Periodes[i].Type+RS+Math.floor(LesActions[iAct].Periodes[i].Hfin)+RS;
              S +=Math.floor(LesActions[iAct].Periodes[i].Vmin)+RS+Math.floor(LesActions[iAct].Periodes[i].Vmax)+RS;  
              S +=Math.floor(LesActions[iAct].Periodes[i].Tinf)+RS+Math.floor(LesActions[iAct].Periodes[i].Tsup)+RS;
              S +=Math.floor(LesActions[iAct].Periodes[i].Hmin)+RS+Math.floor(LesActions[iAct].Periodes[i].Hmax)+RS;
              S +=Math.floor(LesActions[iAct].Periodes[i].CanalTemp)+RS+Math.floor(LesActions[iAct].Periodes[i].SelAct)+RS;
              S +=Math.floor(LesActions[iAct].Periodes[i].Ooff)+RS+Math.floor(LesActions[iAct].Periodes[i].O_on)+RS;  
              S +=LesActions[iAct].Periodes[i].Tarif + RS;  
            }
            S +=GS;
        }
      }
      if (ReacCACSI<100) ReacCACSI  = document.querySelector('input[name="ReacCACSI"]:checked').value; //Pas d'estimation
      var Fpwm  = document.querySelector('input[name="Fpwm"]:checked').value;
      S=clean(S);
      S = "?ReacCACSI=" + ReacCACSI + "&Fpwm=" + Fpwm +"&actions="+S+"|"; //On ne peut pas terminer par GS
      
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
          if (this.readyState == 4 && this.status == 200) {
              var retour = this.responseText;
             location.reload();
          }
      };
      xhttp.open('GET', '/ActionsUpdate' + S, true);
      xhttp.send();
      
  }

  function FinParaRouteur(){
    SelectActions="<option value=255>Non exploité</option>";
    for (esp=0;esp<nomRMS.length;esp++){ //Liste des actions par routeur
      for (var iAct = 0; iAct < nomActions[esp].length; iAct++){
        var v= esp*10 + parseInt(nomActions[esp][iAct][0]); //Nombre refletant la référence esp et action
        var T =  (esp==0  ) ? "" :  nomRMS[esp] + " / ";
        SelectActions += "<option value=" + v + ">" + T +nomActions[esp][iAct][1] + "</option>"; 
        ListeActions[v]= T +nomActions[esp][iAct][1];    
      }
    }
    checkDisabled();
  }
  
  function Disp(t) {
    if ( t!="zOn" && t!="zOff" && t!="pwTr" && t!="mxTr"  && t!="tmpr" && t!="tarif"  && t!="sAct" ) t=t.id.substr(0, 4);
      let m="";
      switch (t) { 
      case "mode":
          m = "D&eacute;sactivation du routage ou mode de découpe du secteur 230V.";
          break;
      case "opti":
          m = "Critères d'activation.";
          break;
      case "titr":
          m = "Nom ou Titre";
          break;
      case "slid":
          m = "Gain de la boucle d'asservissement. Faible, la r&eacute;gulation est lente mais stable. Elev&eacute;, la r&eacute;gulation est rapide mais risque d'oscillations. A ajuster suivant la charge branch&eacute;e au triac.";
          break;
      case "host":
          m = "Adresse IP machine sur réseau LAN, nom de domaine ou rien pour l'ESP32.<br>Ex : <b>192.168.1.25</b> ou <b>machine.local</b> .";
          break;
      case "port":
          m = "Port d'acc&egrave;s via le protocole http , uniquement pour machine distante. En g&eacute;n&eacute;ral <b>80</b>.";
          break;
      case "ordr":
          m = "Ordre à passer à la machine distante. <br>";
          m += "Ex. pour une machine sur le r&eacute;seau :<br><b>/commande?idx=23&position=on</b>. Se r&eacute;f&eacute;rer &agrave; la documentation constructeur.";
          break;
      case "sele":
          m = "Sélection du GPIO. <br>";
          m += "Choix de l'état de sortie haut ou bas quand l'action est 'On'";
          break;
      case "repe":
          m = "P&eacute;riode en s de r&eacute;p&eacute;tition/rafra&icirc;chissement de la commande. Uniquement pour les commandes vers l'extérieur.<br>";
          m += "0= pas de répétition.";
          break;
      case "temp":
          m = "Temporisation entre chaque changement d'état pour éviter des oscillations quand un appareil dans la maison consomme en dents de scie (Ex: un four).";
          break;
      case "adds":
          m = "Ajout ou retrait d'une p&eacute;riode horaire.";
          break;
      case "Pw_m":
          m = "Seuil de puissance pour activer ou désactiver le routage.";
          m +="Attention, en cas de mode On/Off la diff&eacute;rence, seuil sup&eacute;rieur moins  seuil inf&eacute;rieur doit &ecirc;tre sup&eacute;rieure &agrave; la consommation du dipositif pour &eacute;viter l'oscillation du relais de commande.";
          break;
      case "pwTr":
          m = "Seuil en W de r&eacute;gulation par le Triac de la puissance mesur&eacute;e Pw en entrée de la maison. Valeur typique : 0.";
          break;
      case "mxTr":
          m = "Ouverture maximum du triac ou du SSR entre 5 et 100%. Valeur typique : 100%";
          break;
      case "zOff":
          m = "Off forcé";
          break;
      case "zOn":
          m = "On forcé (si conditions optionnelles valides)";
          break;
      case "tmpr":
          m = "Définir la ou les températures qui permettent l'activation de la fonction On ou Routage.<br>Si condition non rempli ordre Off envoyé ou Triac se ferme.<br><br> Si seuil T>= est supérieur à seuil T<=,<br>activation de la fonction pour les valeurs inférieures de température<br>avec creation d'une hysteresis entre ces 2 seuils.<br><br>Ne rien mettre si pas d'activation en fonction de la température.";
          break;
      case "tarif":
          m = "Condition d'activation suivant la tarification.<br>Sinon ordre Off envoyé ou Triac se ferme.";
          break;
      case "sAct":
          m = "Définir des seuils :<br>- durée d'ouverture<br>- pourcentage d'ouverture";
          break;
      }
      GH("message", m);
      GID("message").style = "display:inline-block;";
      Timer = 10;
  }
  var Timer = 0;
  function DispTimer() {
      Timer = Timer - 1;
      if (Timer < 0) {
          GID('message').style = 'display:none;';
      }
      setTimeout("DispTimer();", 1000);
  }
  function AdaptationSource(){
    
  }
)====";