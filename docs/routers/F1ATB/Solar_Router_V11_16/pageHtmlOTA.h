//***************************************************
// Page HTML et Javascript mise à jour du code par OTA
//***************************************************
const char *OtaHtml = R"====(
  <!doctype html>
  <html><head><meta charset="UTF-8">
  <link rel="stylesheet" href="commun.css">
  <style>
    input {font-size:20px;}
    .liste{display:flex;justify-content:center;text-align:left;} 
    #onglets2{display:block;}
    .Bparametres{border:inset 10px azure;}
    .Bota{border:inset 4px azure;}
    body{color:white;}
  </style>
  <script src="/ParaRouteurJS"></script>
  <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>

  </head>
  <body onload="Init();">
    <div id='lesOnglets'></div>
    <h2 >Web OTA</h2>
    <h4>Mise à jour par Wifi</h4>
    <div class="liste">
      Votre version actuelle du routeur : <span id ="Version_actu"></span>
    </div>
    <br>
    <div class="liste">  
      Version(s) disponible(s) :
    </div>
    <div class="liste">
      <iframe src="https://f1atb.fr/web_tool/scan_dir_bin.php"  width=50% height=150px></iframe>
    </div>
    <div class="liste">
      <ul>
        <li>1 - <a href='/Export' >Sauvegardez vos paramètres</a> sur le PC</li>
        <li>2 - Téléchargez sur votre ordinateur, la version binaire du logiciel du routeur souhaitée <br>(Solar_Router_Vxx.xx.ino.bin) en cliquant dessus</li>
        <li>3 - Cliquez sur "Choisir un fichier" et sélectionnez ce binaire sur votre ordinateur</li>
        <li>4 - Cliquez sur "Mettre à jour"</li>
      </ul>
    </div>
    <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
      <input type='file' name='update' id="fichier">
      <input class='bouton' type='submit' value='Mettre à jour'>
    </form>
    <div id='prg'>progression: 0%</div>
    <script>
          $('form').submit(function(e){
            e.preventDefault();
            var form = $('#upload_form')[0];
            var data = new FormData(form);
            
            $.ajax({
              url: '/update',
              type: 'POST',
              data: data,
              contentType: false,
              processData:false,
              xhr: function() {
              var xhr = new window.XMLHttpRequest();
              xhr.upload.addEventListener('progress', function(evt) {
              if (evt.lengthComputable) {
              var per = evt.loaded / evt.total;
              $('#prg').html('progression: ' + Math.round(per*100) + '%');
              }
              }, false);
              return xhr;
              },
              success:function(d, s) {
              console.log('succès!')
              
            },
            error: function (a, b, c) {
            }
          });
        });
        function Init(){
          SetHautBas();
          LoadParaRouteur();
        }
        function AdaptationSource(){
          setTimeout('GH("Version_actu", GID("version").innerHTML)',1000);
        };
    </script>
    <br>
    <div id='pied'></div>
    <br>
</body></html>
 
 )====";