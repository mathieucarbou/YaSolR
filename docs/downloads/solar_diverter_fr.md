Le Routeur Solaire Shelly

Bonjour la communauté!

Je souhaitais vous partager un petit projet que j'ai réalisé ces derniers jours dans le cadre du _"Shelly IoT Innovation Challenge"_ et qui va sûrement vous intéresser: un routeur solaire assez innovant car très simple et basé sur... les Shelly!

J'ai toujours souhaité trouvé un moyen **simple** de contrôler le régulateur de tension **Loncont LSA-H3P50YB**, qui est intéressant car en plus d'être costaud, il comprend un module Zero-Detection et se contrôle donc très facilement par variation de tension ou PWM.
On peut notamment le contrôler par un ESP32 mais à l'aide d'un module externe et une alimentation de 12V supplémentaire, ce qui n'est pas très pratique.

Il y a quelques jours, Mathieu Hertz m'a fait remarqué que Shelly ont des dimmers 0-10V, plage de tension justement requise au contrôle de ce régulateur, mais pour que cela fonctionne, il faut que le dimmer soit de type "sourcing current" et non "sinking current".

Hors... Shelly vient justement de sortie un nouveau dimmer depuis très peu: le **Shelly Dimmer 0/1-10V PM Gen3**, qui est justement de type "sourcing current"!

Quel bonheur!

J'ai donc décidé d'écrire un **script Shelly** qui permet de contrôler automatiquement ce genre de régulateur de tension via le Shelly Dimmer 0/1-10V PM Gen3, en fonction de la l'injection ou consommation lue à partir du Shelly EM Pro (acheté chez Quintium), qui lit aussi la puissance en sortie du régulateur avec sa 2ème pince ampèremétrique.

Je n'ai pas pu tester encore sur du long terme, mais les petits tests effectués pour le moment montrent que ça fonctionne, dans la limite possible de la précision qu'on peut avoir en passant par les scripts Shelly: lecture des mesure à chaque seconde et appel pour régler le dimmer de suite après.

C'est donc une solution prometteuse pour le moment, et qui reste facile à améliorer.

Également, les dimmer ont tendance à chauffer, donc je soupçonne qu'il faille l'installer espacé et dans un endroit aéré.

Fonctionnalités et avantages de ce routeur:

- Les composants Shelly et LSA Loncont, sont robustes, aux normes, et utilisés en industrie
- Très facile à mettre en place (script Shelly)
- Gestion automatique du routage via un contrôleur PID qui supporte plusieurs modes de proportionnelles et dérivées
- Supporte un contacteur pour la Marche forcée, qui va automatiquement couper les dimmer si mis en marche
- Supporte une sonde de température DS1820 via le Shelly Add-on pour avoir la température du ballon
- Support jusqu'à N dimmers, avec un partage possible du surplus entre dimmers
- Et bien sûr, tout ce qu'on peut avoir avec le contrôle à distance des Shelly via l'app Shelly, Home Assistant / Jeedom, etc.

À vous ensuite d'écrite vos automatismes Shelly pour programmer la marche forcée, démarrer ou arrêter le routage automatique à distance, etc.
Plein de possibilité avec Shelly!

Le script peut être téléchargé et modifié à votre guise (il est sous license MIT), et il se trouve sur le blog du site YaSolR, le logiciel de routage sur lequel je travaille depuis quelques mois:

https://yasolr.carbou.me/blog/2024-07-01_shelly_solar_diverter

Bonne lecture!
