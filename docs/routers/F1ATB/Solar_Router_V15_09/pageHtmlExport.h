//********************************************************
// Page HTML et Javascript Import / Export configuration *
//********************************************************
const char *ExportHtml = R"====(
  <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="/commun.css">
  <style>
    .Zone{width:100%;border:1px solid grey;border-radius:10px;margin-top:10px;background-color:rgba(30,30,30,0.3);} 
    .boldT{text-align:left;font-weight:bold;padding:10px;}
    .form {margin:auto;padding:10px;display: table;text-align:left;width:100%;}
    .ligne {display: table-row;padding:10px;}
    label{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;width:70%;}
    input,.cell{display: table-cell;margin: 5px;text-align:left;font-size:20px;height:25px;}
    input[type=file]{display:block;height:32px;border:none;}
    .liste{display:flex;justify-content:center;text-align:left;} 
    #onglets2{display:block;}
    .Bparametres{border:inset 10px azure;}
    .Bexport{border:inset 4px azure;}
  </style>
  <title>Import/Export</title>
  </head>
  <body onload="Init();">
    <div id='lesOnglets'></div>
    <h2 >Import / Export des paramètres</h2>
    <div class="Zone">
        <div class="boldT">Export / Sauvegarde des paramètres <br><span class='fsize12'>(Permet la sauvegarde des paramètres sur votre PC avant une mise à jour)</span></div>
        <div class="form"  >
          <div class='ligne'>
            <label for='ip_load'>Paramètres IP du routeur : </label><input type='checkbox' name='ip_load' id='ip_load' style='width:25px;' onclick="setConf();" checked>
          </div>
          <div class='ligne'>
            <label for='para_load'>Autres paramètres (Sources, Noms...) : </label><input type='checkbox' name='para_load' id='para_load' style='width:25px;' onclick="setConf();" checked>
          </div>
          <div class='ligne'>
            <label for='action_load'>Planning des Actions : </label><input type='checkbox' name='action_load' id='action_load' style='width:25px;' onclick="setConf();" checked>
          </div>
          <div class='ligne'>
            <label for='nom_f_para'>Nom du fichier de sauvegarde ( *.json) : </label><input type='text' name='nom_f_para' id='nom_f_para'  value="parametres.json" onchange="setNomPara();">
          </div>
          <div class='ligne'>
            <div class='cell'></div><div class='cell'><a href="/export_file" download="parametre.json" id="adr_export"><button class='bouton'>Télécharger paramètres</button></a></div>
          </div>

        </div>
    </div>
    <div class="Zone">
        <div class="boldT">Import des paramètres</div>
        <form method='POST' onsubmit="submit_para(event);" action="#" enctype='multipart/form-data' id='upload_form'>
          <div class="form">
            <div class='ligne'>
              <div class='cell'><input type='file' name='fichier_para_' id="fichier_para_"  class='bouton' accept='.json'></div>
              <input type='submit' value='Mettre à jour'  class='bouton'>
            </div>
            <div class="lds-dual-ring" id="attente"></div>
          </div>
        </form>
        <span class='fsize12'>Après un Import de paramètres, faites un Reset pour redémarrer avec les nouveaux.</span>
    </div>
    <input  class='bouton' type='button' onclick='Reset();' value='ESP32 Reset' >
    <script>
        var BordsInverse=[".Bparametres",".Bexport"]; 
        function Init(){
          SetHautBas();
          LoadParaRouteur();
          setConf();
        }
        
        function submit_para(event){
          GID("attente").style="visibility: visible;";
          event.preventDefault(); //Pour Firefox
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() { 
            if (this.readyState == 4 && this.status == 200) {
              var retour=this.responseText;
              console.log(retour);
              GID("attente").style="visibility: hidden;";
            }         
          };
          const fileInput = GID("fichier_para_");
          var file = fileInput.files[0];
          var data = new FormData();
          data.append("file", file);
          xhttp.open('POST', '/import', true);
          xhttp.send(data);
        }
        function setNomPara(){
          GID("adr_export").download= GID("nom_f_para").value;
        }
        function setConf(){
          let conf="/export_file?ip=" + GID("ip_load").checked + "&para=" + GID("para_load").checked + "&action=" + GID("action_load").checked
          GID("adr_export").href= conf;
        }
        function AdaptationSource(){}
        function FinParaRouteur(){
          GID("Bheure").style.display= (Horloge>1) ? "inline-block": "none";
          GID("Bwifi").style.display= (ESP32_Type<10) ? "inline-block": "none";
          LoadCouleurs();
        };
        
    </script>
    <br>
    <div id='pied'></div>
    <br>
    <script src="/ParaRouteurJS"></script>
    <script src="/CommunCouleurJS"></script>
</body></html>
 
 )====";