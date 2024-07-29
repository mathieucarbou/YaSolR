//************************************************
// Page HTML et Javascript de gestion des Actions
//************************************************

const char *ActionsHtml = R"====(
   <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="commun.css">
  <style>
    body{color:white;}
    .Bactions{border:inset 8px azure;}
    .cadre{width:100%;max-width:1200px;margin:auto;}
    .form{width:100%;text-align:left;}
    .titre{display:flex;justify-content:center;cursor:pointer;color:black;}
    .slideTriac{width:100%;position:relative;margin:4px;padding:4px;border:2px inset grey;background-color:#fff8f8;color:black;font-size:14px;}
    .slideTriacIn{display:flex;justify-content:center;width:100%;}
    .planning{width:100%;position:relative;margin:4px;padding:4px;border:2px inset grey;background-color:#fff8f8;color:black;}
    .periode{position:absolute;height:100%;border:outset 4px;border-radius:10px;display:flex;justify-content:space-between;align-items: center;color:white;cursor:ew-resize;}
    .curseur{position:relative;width:100%;height:30px;}
    .infoAction{position:relative;width:100%;height:40px;font-size:20px;}
    .infoZone{position:absolute;height:100%;display:flex;justify-content: space-around;border:2px inset;align-items: center;text-align:center;background-color:#aaa;cursor:pointer;}
    .infoZ{position:absolute;bottom:2px;left:2px;display:none;border:2px inset grey;background-color:#ddd;}
    .infoZ input{width:70px;}
    .Hfin{position:absolute;bottom:2px;right:2px;}
    .zNo{background-color:#666;margin:4px;}
    .zOff{background-color:#66f;margin:4px;}
    .zOn{background-color:#f66;margin:4px;}
    .zPw{background-color:#6f6;margin:4px;padding:2px;}
    .bInset{border:8px inset #f00;}
    .bOutset{border:4px outset grey;cursor:pointer;}
    .zTriac{background-color:#cc4;margin:4px;padding:2px;}
    .selectZ{background-color:#aaa;height:20px;font-size:16px;font-weight: bold;}
    .closeZ{background-color:black;border:outset 2px grey;color:white;position:absolute;top:0px;height:20px;width:20px;right:0px;cursor:pointer;}
    .fcontainer{display: flex;  justify-content: center;background-color:#eea;border:solid 3px #772;margin:6px;}
    .tm{width:100px;text-align:center;padding-left:10px;padding-right:10px;}
    .tbut{width:40px;padding-left:10px;padding-right:10px;text-align:center;font-weight:bold;font-size:24px;cursor:pointer;display:inline-flex;}
    .ligne {display: table-row;padding:10px;}
    .tableAct{width:100%;}
    tr {margin: 2px;text-align:left;font-size:20px;}
    h4{padding:2px;margin:0px;}
    h5{text-align:left;padding:2px;margin:0px;}
    label{text-align:right;}
    input {margin: 5px;text-align:left;font-size:15px;max-width:150px;}
    #message{position:fixed;border:inset 4px grey;top:2px;right:2px;background-color:#333;color:white;font-size:16px;display:none;text-align:left;padding:5px;}
    .bord1px{border:solid 1px grey; margin:2px;}
    #mode,.bouton_curseur{display:flex;justify-content: space-around;font-size:20px;}
    .boutons{display:inline-flex;}
    .triacNone0{display:none;}
    #plannings h4,#planning0 h4{color:black};{color:black};
  </style>
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
      <div  style='text-align:right;padding-top:20px;'>
        <input class='bouton' type='button' value='Sauvegarder' onclick="SendValues();">
        <div class="lds-dual-ring" id="attente"></div>
      </div>
    </div>
    <div id="message"></div><br>
    <div id='pied'></div> 
    <script src="ActionsJS"></script>
    <script src="ActionsJS2"></script>
    <script src="ActionsJS3"></script>
    <script src="/ParaRouteurJS"></script>
  </body></html>
)====";
const char *ActionsJS = R"====(
  var LesActions = [];
  var mouseClick = false;
  var blockEvent = false;
  var temperatureDS=-127;
  var LTARFbin=0;
  var pTriac=0;
  var IS="|"; //Input Separator
  function Init() {
      LoadActions();
      DispTimer();
      LoadParaRouteur();
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
      var Pins=[0,4,5,14,16,17,21,22,23,25,26,27,-1];
      var SelectPin="Gpio <select id='selectPin"+iAct +"' onchange='checkDisabled();' onmousemove='Disp(this)'>";
      for (var i=0;i<Pins.length;i++){
        var v="gpio:"+Pins[i];
        if (Pins[i]==0) v="";
        if (Pins[i]==-1) v="Externe";
        SelectPin +="<option value="+Pins[i]+">"+v+"</option>";
      }
      SelectPin +="</select>";
      var SelectOut="<span id='SelectOut" + iAct +"'>&nbsp;&nbsp;&nbsp;Sortie 'On' <select id='selectOut"+iAct +"' onmousemove='Disp(this)'><option value=0>0V</option><option value=1 selected>3.3V</option></select></span>";
      var S = "<div class='titre'><h4 id ='titre" + iAct + "' onmousemove='Disp(this)' onclick='editTitre(" + iAct + ")'>Titre</h4></div>";
      S +="<div  id='mode' onmousemove='Disp(this)'>" +Radio0 + Radio1 + "</div>";

      S +="<div id='blocPlanning"+iAct+"' class='triacNone0' >";
        S += "<table class='tableAct triacNone"+ iAct +"'><tr>";       
        S += "<td><div id='Host" + iAct + "'>Host <input type='text' id='host" + iAct + "' onmousemove='Disp(this)' onchange='checkDisabled();' ></div></td>";
        S += "<td>"+SelectPin+SelectOut+"</td>";
        S += "<td><div id='ordreon" + iAct +"'>Ordre On <input type='text' id='ordreOn" + iAct + "' onmousemove='Disp(this)'></div></td><td class='tm' id='Repet" + iAct + "'>Répétition(s)</td>";
        S += "<td class='tm' id='Tempo" + iAct + "'>Temporisation(s)</td>";
        S += "</tr><tr id='ligne_bas"  +iAct + "'>";
        S += "<td><div id='Port" + iAct + "'>Port <input type='number' id='port" + iAct + "' onmousemove='Disp(this)'></div></td>";
        S += "<td></td>";
        S += "<td><div id='ordreoff" + iAct +"'>Ordre Off <input type='text' id='ordreOff" + iAct + "' onmousemove='Disp(this)'></div></td>";
        S += "<td class='tm'><input type='number' id='repet" + iAct + "' class='tm' onmousemove='Disp(this)'></td>";
        S += "<td class='tm'><input type='number' id='tempo" + iAct + "' class='tm' onmousemove='Disp(this)'></td>";
        S += "</tr></table>";

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
        if (temperatureDS>-100) { // La sonde de température fonctionne          
          var Tsup=LesActions[iAct].Periodes[i].Tsup;
          if (Tsup>=0 && Tsup <=1000) temperature +="<div> <small> si </small> T &ge;" + Tsup/10 + "°</div>";
          var Tinf=LesActions[iAct].Periodes[i].Tinf;
          if (Tinf>=0 && Tinf <=1000) temperature +="<div> <small> si </small> T &le;" + Tinf/10 + "°</div>";
        }  
        var TxtTarif= "";
        if (LTARFbin>0)  {   
          TxtTarif= " <small> si </small>Tarif : ";
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
        if (LesActions[iAct].Actif<=1 && iAct>0){
          LesActions[iAct].Periodes[i].Vmax=Math.max(LesActions[iAct].Periodes[i].Vmin,LesActions[iAct].Periodes[i].Vmax);
		      var TexteMinMax="<div>Off si Pw&gt;"+LesActions[iAct].Periodes[i].Vmax+"W</div><div>On si Pw&lt;"+LesActions[iAct].Periodes[i].Vmin+"W</div>"+temperature + TxtTarif;
        } else {
          LesActions[iAct].Periodes[i].Vmax=Math.max(0,LesActions[iAct].Periodes[i].Vmax);
          LesActions[iAct].Periodes[i].Vmax=Math.min(100,LesActions[iAct].Periodes[i].Vmax);
          var TexteMinMax="<div>Seuil Pw : "+LesActions[iAct].Periodes[i].Vmin+"W</div>"+ temperature + "<div>Ouvre Max : "+LesActions[iAct].Periodes[i].Vmax+"%</div>" + TxtTarif;
        }
		    var TexteTriac="<div>Seuil Pw : "+LesActions[iAct].Periodes[i].Vmin+"W</div>"+temperature + "<div>Ouvre Max : "+LesActions[iAct].Periodes[i].Vmax+"%</div>"+TxtTarif;
        var paras = ["Pas de contr&ocirc;le", "OFF", "ON" + temperature + TxtTarif, TexteMinMax, TexteTriac];
        var para = paras[Type];
        S += "<div class='periode' style='width:" + w + "%;left:" + left + "%;background-color:" + color + ";'   ><div>&lArr;</div><div>&rArr;</div></div>";
        Hmn = Math.floor(H0 / 100) + ":" + ("0" + Math.floor(0.6 * (H0 - 100 * Math.floor(H0 / 100)))).substr(-2, 2);
        fs = Math.max(8, Math.min(16, w/2)) + "px";
        Sinfo += "<div class='infoZone' style='width:" + w + "%;left:" + left + "%;border-color:" + color + ";font-size:" + fs + "'  onclick='infoZclicK(" + i + "," + iAct + ")'  >"
        Sinfo += "<div class='Hfin'>" + Hmn + "</div><div id='info" + iAct + "Z" + i + "' class='infoZ' ></div>" + para + "</div>";
      }
      GH("curseurs" + iAct, S);
      GH("infoAction" + iAct, Sinfo);
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
          var dist = Math.abs(HeureMouse - LesActions[iAct].Periodes[i].Hfin)
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
)====";


const char *ActionsJS2 = R"====(
  function AddSub(v, iAct) {
      if (v == 1) {
          if (LesActions[iAct].Periodes.length<8){
            LesActions[iAct].Periodes.push({
                Hfin: 2400,
                Type: 1,
                Vmin:0,
                Vmax:100,
                Tinf:1500,
                Tsup:1500,
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
      if (!blockEvent) {
          blockEvent = true;
          var Type = LesActions[iAct].Periodes[i].Type;
          var idZ = "info" + iAct + "Z" + i;
          var S = "<div class='selectZ'>S&eacute;lection Action<div class='closeZ' onclick='infoZclose(\"" + idZ + "\")'>X</div></div>";
          //On ne traite plus depuis version8 le cas "Pas de Contrôle". Inutile
          c = (Type == 1) ? "bInset" : "bOutset";
          S += "<div class='zOff " + c + "' onclick='selectZ(1," + i + "," + iAct + ");' onmousemove='Disp(\"zOff\")'>OFF</div>";
          S += "<div class='fcontainer'><div >";
              c = (Type == 2) ? "bInset" : "bOutset";
              S += "<div  class='zOn " + c + "' onclick='selectZ(2," + i + "," + iAct + ");' onmousemove='Disp(\"zOn\")' >ON</div>";
              c = (Type > 2) ? "bInset" : "bOutset";
              var Vmin=LesActions[iAct].Periodes[i].Vmin;
              var Vmax=LesActions[iAct].Periodes[i].Vmax;
              var Tinf=LesActions[iAct].Periodes[i].Tinf;
              var Tsup=LesActions[iAct].Periodes[i].Tsup;
              var TinfC=Tinf/10;
              var TsupC=Tsup/10;
              if (Tinf>1000 || Tinf<0) TinfC=""; //Temperature entre 0 et 100° représenté en dixième
              if (Tsup>1000 || Tsup<0) TsupC=""; //Temperature entre 0 et 100
              if (iAct > 0) {
                  var Routage=["","Routage ON/Off","Routage Multi-sinus","Routage Train de Sinus"];
                  S += "<div class='zPw " + c + "' onclick='selectZ(3," + i + "," + iAct + ");'><div><small>" +Routage[LesActions[iAct].Actif] + "</small></div>";
                  if (LesActions[iAct].Actif<=1) {
                      S += "<div><small>On : &nbsp;</small>Pw &lt;<input id='Pw_min_"+idZ+"' onmousemove='Disp(this)' type='number' value='"+Vmin+"' onchange='NewVal(this)' >W</div>";
                      S += "<div><small>Off : </small>Pw &gt;<input id='Pw_max_"+idZ+"' onmousemove='Disp(this)' type='number' value='"+Vmax+"' onchange='NewVal(this)'>W</div>";
                      S += "<div><small>Puissance active en entrée de maison</small></div></div>";
                  } else {
                    S += "<div><small>Seuil Pw : &nbsp;</small><input id='Pw_min_"+idZ+"' onmousemove='Disp(this)' type='number' value='"+Vmin+"' onchange='NewVal(this)' >W</div>";
                    S += "<div><small>Puissance active en entrée de maison</small></div>";
                    S += "<div><small>Ouvre Max : </small><input id='Pw_max_"+idZ+"' onmousemove='Disp(this)' type='number' value='"+Vmax+"' onchange='NewVal(this)'>%</div></div>";
                  }
                  
              } else {
                  var Routage=["","Routage Découpe Sinus","Routage Multi-sinus","Routage Train de Sinus"];
                  S += "<div  class='zTriac " + c + "' onclick='selectZ(4," + i + "," + iAct + ");'><div><small>" +Routage[LesActions[iAct].Actif] + "</small></div>";
                  S += "<div>Seuil Pw &nbsp;<input id='Pw_min_"+idZ+"' onmousemove='Disp(\"pwTr\")' type='number' value='"+Vmin+"' onchange='NewVal(this)'>W</div>";
                  S += "<div><small>Puissance active en entrée de maison</small></div>";
                  S += "<div>Ouvre Max <input id='Pw_max_"+idZ+"' onmousemove='Disp(\"mxTr\")' type='number' value='"+Vmax+"' onchange='NewVal(this)'>%</div></div>";
              }
            S += "</div>";
            S += "<div>";
              if (temperatureDS>-100) {
                S += "<div  class='bord1px' onmousemove='Disp(\"tmpr\")'>";
                S += "<div>Actif si température :</div>";
                S += "<div>T &ge;<input id='T_sup_"+idZ+"'  type='number' value='" + TsupC +"' onchange='NewVal(this)' >°</div>";
                S += "<div>T &le;<input id='T_inf_"+idZ+"'  type='number' value='" + TinfC +"' onchange='NewVal(this)' >°</div>";
                S += "<div><small>T en degré (0.0 à 100.0) ou laisser vide</small></div>";
                S += "</div>";
              }
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
      S="TracePeriodes("+idx[0]+");"
      setTimeout(S, 100);
  }
  function selectZ(T, i, iAct) {
      if (LesActions[iAct].Periodes[i].Type != T) {
          LesActions[iAct].Periodes[i].Type = T;
          var idZ = "info" + iAct + "Z" + i;
          if (T <= 2)
              infoZclose(idZ);
          TracePeriodes(iAct);
      }
  }

  function NewVal(t){
      var champs=t.id.split("info");
      var idx=champs[1].split("Z");   //Num Action, Num période
      if (champs[0].indexOf("min")>0){
        LesActions[idx[0]].Periodes[idx[1]].Vmin=Math.floor(GID(t.id).value);
      }
      if (champs[0].indexOf("max")>0){
        LesActions[idx[0]].Periodes[idx[1]].Vmax=Math.floor(GID(t.id).value);
        if (idx[0]==0){
          LesActions[idx[0]].Periodes[idx[1]].Vmax=Math.max(LesActions[idx[0]].Periodes[idx[1]].Vmax,5);
          LesActions[idx[0]].Periodes[idx[1]].Vmax=Math.min(LesActions[idx[0]].Periodes[idx[1]].Vmax,100);
        }
      }
      if (champs[0].indexOf("inf")>0){
        var V= GID(t.id).value;
        if (V=="") V=128;
        LesActions[idx[0]].Periodes[idx[1]].Tinf=Math.floor(V*10);
      }
      if (champs[0].indexOf("sup")>0){
        var V= GID(t.id).value;
        if (V=="") V=128;
        LesActions[idx[0]].Periodes[idx[1]].Tsup=Math.floor(V*10);
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
    for (var iAct = 0; iAct < LesActions.length; iAct++) {
        for (var i=0;i<=3;i++){
            if( GID("radio" + iAct +"-"+ i).checked ) { LesActions[iAct].Actif =i;}
        }
        TracePeriodes(iAct);
        GID("planning0").style.display = (pTriac>0) ? "block" : "none";  // Si Pas de Triac 
        GID("TitrTriac").style.display = (pTriac>0) ? "block" : "none";  
        GID("blocPlanning"+iAct).style.display = (LesActions[iAct].Actif>0) ? "block" : "none";
        var visible = ( LesActions[iAct].Actif== 1) ? "visible" : "hidden";
        GID("Tempo"+iAct).style.visibility =visible;
        GID("tempo"+iAct).style.visibility =visible;
        var disable=true;
        var disp="block";
        if (GID("selectPin"+iAct).value>=0) { visible="hidden";disable=false;disp="none";} 
        GID("SelectOut"+iAct).style.display = (GID("selectPin"+iAct).value<=0) ? "none":"inline-block";       
        GID("Host"+iAct).style.visibility =visible;
        GID("host"+iAct).style.visibility =visible;
        GID("Port"+iAct).style.visibility =visible;
        GID("port"+iAct).style.visibility =visible;
        GID("Repet"+iAct).style.visibility =visible;
        GID("repet"+iAct).style.visibility =visible;   
        GID("radio" + iAct +"-2").disabled = disable; 
        GID("radio" + iAct +"-3").disabled = disable;
        GID("ordreoff"+iAct).style.display=disp;  
        GID("ordreon"+iAct).style.display =disp;
        if (GID("selectPin"+iAct).value==-1 && GID("ordreOn"+iAct).value.indexOf(IS)>0) GID("ordreOn"+iAct).value="";            
        GID("ligne_bas"+iAct).style.display  =( LesActions[iAct].Actif> 1) ?  "none" :"table-row";
        GID("fen_slide"+iAct).style.visibility = (LesActions[iAct].Actif== 1 && iAct>0  ) ? "hidden" : "visible";
    }
  }

)====";


const char *ActionsJS3 = R"====(
  function LoadActions() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
          if (this.readyState == 4 && this.status == 200) {
              var LeRetour = this.responseText;
              var Les_ACTIONS = LeRetour.split(GS);
              var LesParas = Les_ACTIONS[0].split(RS);
              temperatureDS=LesParas[0];
              LTARFbin = parseInt(LesParas[1]);
              pTriac = parseInt(LesParas[2]);
              LesActions.splice(0,LesActions.length);
              for (var iAct=1;iAct<Les_ACTIONS.length-1;iAct++){
                var champs=Les_ACTIONS[iAct].split(RS);
                var NbPeriodes=champs[9];
                var Periodes=[];
                var j=10;
                for (var i=0;i<NbPeriodes;i++){
                  Periodes[i]={Type:champs[j],Hfin:champs[j+1],Vmin:champs[j+2],Vmax:champs[j+3],Tinf:champs[j+4],Tsup:champs[j+5],Tarif:champs[j+6]};
                  j=j+7;
                }
                LesActions[iAct-1]=creerAction(champs[0], champs[1], champs[2], champs[3], champs[4], champs[5], champs[6],champs[7],champs[8], Periodes);
              }    
              if (LesActions.length==0){  //Action Triac
                  LesActions.push( creerAction(0, "Titre Triac", "", 50, "", "","", 0,50, [{
                          Hfin: 2400,
                          Type: 4,
                          Vmin:0,
                          Vmax:100,
                          Tinf:1500,
                          Tsup:1500,
                          Tarif:31
                      }
                  ]));
              }
              LesActions.push( creerAction(0, "Titre Relais " + LesActions.length, "", 80, "", "", 240,0,50, [{
                      Hfin: 2400,
                      Type: 3,
                      Vmin:0,
                      Vmax:100,
                      Tinf:1500,
                      Tsup:1500,
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
              
          }
      };
      xhttp.open('GET', 'ActionsAjax', true);
      xhttp.send();
  }
  
  
  function SendValues() {
      GID("attente").style="visibility: visible;";
      for (var iAct = 0; iAct < LesActions.length; iAct++) {
        for (var i=0;i<=3;i++){
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
        if (GID("selectPin"+iAct).value==0 && iAct>0) LesActions[iAct].Actif=-1; //Action à effacer
      }
      var S="";
      for (var iAct = 0; iAct < LesActions.length; iAct++) {
        if (LesActions[iAct].Actif>=0){
            S +=LesActions[iAct].Actif+RS+LesActions[iAct].Titre+RS;
            S +=LesActions[iAct].Host+RS+LesActions[iAct].Port+RS;
            S +=LesActions[iAct].OrdreOn+RS+LesActions[iAct].OrdreOff+RS+LesActions[iAct].Repet+RS+LesActions[iAct].Tempo+RS;
            S +=LesActions[iAct].Reactivite + RS + LesActions[iAct].Periodes.length+RS;
            for (var i=0;i<LesActions[iAct].Periodes.length;i++){
              S +=LesActions[iAct].Periodes[i].Type+RS+Math.floor(LesActions[iAct].Periodes[i].Hfin)+RS;
              S +=Math.floor(LesActions[iAct].Periodes[i].Vmin)+RS+Math.floor(LesActions[iAct].Periodes[i].Vmax)+RS;  
              S +=Math.floor(LesActions[iAct].Periodes[i].Tinf)+RS+Math.floor(LesActions[iAct].Periodes[i].Tsup)+RS;  
              S +=LesActions[iAct].Periodes[i].Tarif + RS;  
            }
            S +=GS;
        }
      }
      S=clean(S);
      S = "?actions="+S+"|"; //On ne peut pas terminer par GS
      
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
          if (this.readyState == 4 && this.status == 200) {
              var retour = this.responseText;
             location.reload();
          }
      };
      console.log(S)
      xhttp.open('GET', 'ActionsUpdate' + S, true);
      xhttp.send();
      
  }
  
  
  function Disp(t) {
    if (t!="zNo" && t!="zOn" && t!="zOff" && t!="pwTr" && t!="mxTr"  && t!="tmpr" && t!="tarif" ) t=t.id.substr(0, 4);
      switch (t) { 
      case "mode":
          var m = "D&eacute;sactivation du routage ou mode de découpe du secteur 230V."
              break;
      case "titr":
          var m = "Nom ou Titre";
          break;
      case "slid":
          var m = "Gain de la boucle d'asservissement. Faible, la r&eacute;gulation est lente mais stable. Elev&eacute;, la r&eacute;gulation est rapide mais risque d'oscillations. A ajuster suivant la charge branch&eacute;e au triac.";
          break;
      case "host":
          var m = "Adresse IP machine sur réseau LAN, nom de domaine ou rien pour l'ESP32.<br>Ex : <b>192.168.1.25</b> ou <b>machine.local</b> .";
          break;
      case "port":
          var m = "Port d'acc&egrave;s via le protocole http , uniquement pour machine distante. En g&eacute;n&eacute;ral <b>80</b>.";
          break;
      case "ordr":
          var m = "Ordre à passer à la machine distante. <br>";
          m += "Ex. pour une machine sur le r&eacute;seau :<br><b>/commande?idx=23&position=on</b>. Se r&eacute;f&eacute;rer &agrave; la documentation constructeur."
          break;
      case "sele":
          var m = "Sélection du GPIO. <br>";
          m += "Choix de l'état de sortie haut ou bas quand l'action est 'On'"
          break;
      case "repe":
          var m = "P&eacute;riode en s de r&eacute;p&eacute;tition/rafra&icirc;chissement de la commande. Uniquement pour les commandes vers l'extérieur.<br>";
          m += "0= pas de répétition.";
          break;
      case "temp":
          var m = "Temporisation entre chaque changement d'état pour éviter des oscillations quand un appareil dans la maison consomme en dent de scie (Ex: un four)."
          break;
      case "adds":
          var m = "Ajout ou retrait d'une p&eacute;riode horaire."
              break;
      case "Pw":
          var m = "Seuil inf&eacute;rieur  de puissance mesur&eacute;e Pw &lt; pour d&eacute;marrer le routage  et seuil sup&eacute;rieur de puissance  &gt; pour l'arr&ecirc;ter.<br> ";
      m +="Attention, la diff&eacute;rence, seuil sup&eacute;rieur moins  seuil inf&eacute;rieur doit &ecirc;tre sup&eacute;rieure &agrave; la consommation du dipositif pour &eacute;viter l'oscillation du relais de commande."
          break;
      case "pwTr":
          var m = "Seuil en W de r&eacute;gulation par le Triac de la puissance mesur&eacute;e Pw en entrée de la maison. Valeur typique : 0.";
          break;
      case "mxTr":
          var m = "Ouverture maximum du triac entre 5 et 100%. Valeur typique : 100%";
          break;
      case "zNo":
            var m = "Pas d'action On ou Off de routage";
            break;
      case "zOff":
            var m = "Off forcé";
            break;
      case "zOn":
            var m = "On forcé (si règle température valide)";
            break;
      case "tmpr":
            var m = "Définir la ou les températures qui permettent l'activation de la fonction On ou Routage.<br>Sinon ordre Off envoyé ou Triac se ferme.<br>Ne rien mettre si pas d'activation en fonction de la température.";
            break;
      case "tarif":
            var m = "Condition d'activation suivant la tarification.<br>Sinon ordre Off envoyé ou Triac se ferme.";
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