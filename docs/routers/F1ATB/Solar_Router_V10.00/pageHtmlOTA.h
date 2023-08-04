//***************************************************
// Page HTML et Javascript mise à jour du code par OTA
//***************************************************
const char *OtaHtml = R"====(
  <!doctype html>
  <html><head><meta charset="UTF-8"><style>
    * {box-sizing: border-box;}
    body {font-size:150%;text-align:center;width:100%;max-width:1000px;margin:auto;background: linear-gradient(#003,#77b5fe,#003);background-attachment:fixed;color:white;}
    h2{text-align:center;color:white;}
    .onglets{margin-top:4px;left:0px;font-size:130%;}
    .Baccueil,.Bbrut,.Bparametres,.Bactions{margin-left:20px;border:outset 4px grey;background-color:#333;border-radius:6px;padding-left:20px;padding-right:20px;display:inline-block;}
    a:link {color:#aaf;text-decoration: none;}
    a:visited {color:#ccf;text-decoration: none;}
    input {font-size:20px;}
    .pied{display:flex;justify-content:space-between;font-size:14px;color:white;} 
    .liste{display:flex;justify-content:center;text-align:left;} 
    .lds-dual-ring {color: #cccc5b;visibility: hidden;}
    .lds-dual-ring,.lds-dual-ring:after {box-sizing: border-box;}
    .lds-dual-ring {display: inline-block;width: 80px;height: 80px;}
    .lds-dual-ring:after {content: " ";display: block;width: 64px;height: 64px;margin: 8px;border-radius: 50%;border: 6.4px solid currentColor;border-color: currentColor transparent currentColor transparent;animation: lds-dual-ring 1.2s linear infinite;}
    @keyframes lds-dual-ring {0% {transform: rotate(0deg);} 100% {transform: rotate(360deg);}}
  </style>
  <script src="/ParaRouteurJS"></script>
  <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>

  </head>
  <body onload="Init();">
    <div class='onglets'><div class='Baccueil'><a href='/'>Accueil</a></div><div class='Bbrut'><a href='/Brute'>Donn&eacute;es brutes</a></div><div class='Bparametres'><a href='/Para'>Param&egrave;tres</a></div><div class='Bactions'><a href='/Actions'>Actions</a></div></div>
    <h2 >Web OTA</h2>
    <h4 >Mise à jour par Wifi</h4>
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
        <li>1 - Téléchargez sur votre ordinateur, la version binaire du logiciel du routeur souhaitée (Solar_Router_Vxx.xx.ino.bin)</li>
        <li>2 - Cliquez sur "Choisir un fichier" et sélectionnez ce binaire sur votre ordinateur</li>
        <li>3 - Cliquez sur "Mettre à jour"</li>
      </ul>
    </div>
    <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
      <input type='file' name='update' id="fichier">
      <input type='submit' value='Mettre à jour'>
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
          LoadParaRouteur();
        }
        function AdaptationSource(){
          setTimeout('GH("Version_actu", GID("version").innerHTML)',1000);
        };
    </script>
    <br>
    <div class='pied'><div>Routeur Version : <span id='version'></span></div><div><a href='https:F1ATB.fr' >F1ATB.fr</a></div></div>
    <br>
</body></html>
 
 )====";