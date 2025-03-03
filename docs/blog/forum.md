[size=200]Le routeur solaire YaS☀️lR[/size]

En tant que développeur de profession et spécialiste en librairies Arduino / ESP32 liées à l'énergie, je bosse depuis fin 2023 sur un projet de logiciel de routage Open-Source nouveau genre, appelé [b]YaSolR[/b] ([i]Yet Another Solar Router[/i]), que je vais vous décrire ici.

[list]
[*][b]Site web[/b]: https://yasolr.carbou.me
[*][b]Projet GitHub[/b]: https://github.com/mathieucarbou/YaSolR
[/list]

[img]https://yasolr.carbou.me/assets/img/screenshots/app-overview.jpeg[/img]

[size=150]Qui suis-je ?[/size]

Pour ceux qui ne me connaissent pas, je suis très actif dans le milieu Arduino / ESP32 / Home Assistant.

[b]Profil[/b]: https://github.com/mathieucarbou

Je suis notamment l'auteur des librairies (toutes utilisées par YaSolR):

[list]
[*] [b][url=https://mathieu.carbou.me/MycilaJSY]MycilaJSY[/url][/b]: librairie qui supporte les JSY en TTL et RS485 sur ESP32
[*] [b][url=https://mathieu.carbou.me/MycilaPZEM004Tv3]MycilaPZEM004Tv3[/url][/b]: librairie qui supporte le PZEM-004T v3 sur ESP32
[*] [b][url=https://mathieu.carbou.me/MycilaPulseAnalyzer]MycilaPulseAnalyzer[/url][/b]: librairie d'analyse de pulse de circuit Zero-Cross
[*] [b][url=https://mathieu.carbou.me/MycilaESPConnect]MycilaESPConnect[/url][/b]: librairie de gestion réseau pour ESP32 qui supporte l'Ethernet
[/list]

Et je suis également un des développeurs des fameuses librairies [b][url=https://github.com/ESP32Async]AsyncTCP et ESPAsyncWebServer[/url][/b].

Et pour finir, je suis également le [b]concepteur de ces 2 autres routeurs solaires[/b] que vous trouverez dans la section [url=https://yasolr.carbou.me/blogs]blog du site YaSolR[/url] et aussi [url=https://forum-photovoltaique.fr/viewtopic.php?t=72838]sur ce forum[/url]:

[list]
[*] [b]Routeur solaire basé sur Shelly Dimmer Gen 3[/b]
[*] [b]Routeur solaire basé sur Home Assistant[/b]
[/list]

[size=150]Pourquoi avoir fait YaSolR ?[/size]

J'ai analysé BEAUCOUP de code source de routeur solaire, et malheureusement, la plupart des solutions ont des lacunes. 
N'étant donc pas satisfait des solutions de routage solaire existantes (soit d'un point de vue fonctionnalité, précision, rapidité de réaction, ou qualité du code, ou license, ou autre), j'ai décidé de créer ma propre solution, Open-Source.

[b]Le but de YaSolR est donc de fournir une solution de routage solaire de qualité, précise, réactive et compatible avec la plupart du matériel existant et bien plus encore, comme le LSA avec un [url=https://www.dfrobot.com/blog-13458.html]DAC DFRobot[/url][/b]!

[b]YaSolR est UNIQUEMENT le logiciel[/b]: donc pour l'utiliser, vous devez être capable d'assembler votre propre routeur avec le matériel compatible, et installer le logiciel sur un ESP32.

Je ne fournis pas de routeur pré-monté pour des raisons de sécurité, normes, support après-vente, etc.

[size=150]Particularités de YaSolR:[/size]

[list]
[*] [b]Réactivité et rapidité des mesures[/b]: YaSolR est capable de faire une mesure toutes les 42 ms, et [b]ajuste le routage au moins 3 fois par seconde.[/b]
[*] [b]PID[/b]: YaSolR est le seul routeur à utiliser un [b]algorithme PID à proportionnelle aux mesures[/b] pour ajuster le routage, ce qui permet de contrôler les corrections avec plus de précisions et en évitant les à-coups (overshoot) qui provoquent des oscillation, sur-consommations ou sur-injections.
[*] [b]PID Tuning[/b]: YaSolR propose un écran de tuning PID pour ajuster les paramètres du PID en temps réel, sans recompiler le code.
[*] [b]Fréquence[/b]: YaSolR est capable de fonctionner sur une fréquence de [b]50 Hz et 60 Hz[/b], mais aussi sur une fréquence de 51 Hz (par exemple sur génératrice Enedis).
[*] [b]Résolution[/b]: YaSolR a une résolution de routage de [b]12 bits[/b], donc est capable d'être précis au watts prés avec une charge de plus de 4000W. Les routeurs ayant une telle précision sont rares. La plupart des routeurs ont une précision de 100 pas, soit 30W pour une charge de 3000W.
[*] [b]Analyse de pulse Zero-Cross[/b]: YaSolR analyse les pulses du circuit Zero-Cross pour détecter les positions du front montant et descendant, ce qui permet de synchroniser le déclenchement du TRIAC le plus précisément possible lors du vrai passage à zéro. La plupart des routeurs basé sur circuit ZC calculent une fausse valeur de déclenchement à partir du front montant du pulse, qui se trouve à être avant le passage à zéro.
[*] [b]Matériel[/b]: YaSolR est le seul routeur qui n'impose pas de matériel et offre une [b]grande compatibilité[/b].
[*] Supporte le concept de "[b]virtual grid power[/b]" qui permet une compatibilité avec un [b]second routeur ou une borne de recharge EV[/b]
[*] Fonctionnalités de contrôle de phase: remapping du dimmer (comme les Shelly Dimmer), limite de puissance, etc
[*] [b]Bypass[/b] (marche forcée) selon horaires et / ou température
[*] [b]Simplicité[/b]: YaSolR est l'un des rare routeurs à [b]supporter directement le régulateur de tension LSA[/b] via un DAC, sans passer par un module Zero-Cross, ni une second alimentation!
[*] [b]Configurable[/b]: YaSolR permet de reconfigurer ses GPIO afin d'être compatible avec les setup existant du prof solaire et F1ATB entee autres.
[/list]

[size=150]Matériel supportés:[/size]

[b]ESP32 boards[/b]

[list]
[*] Dev Kit boards, 
[*] S3 Dev Kit boards
[*] ESP32s
[*] WIPI 3
[*] Denky D4
[*] Lilygo T Eth Lite S3 boards ([i]Ethernet support[/i])
[*] WT32-ETH01 boards ([i]Ethernet support[/i])
[*] Olimex ESP32-POE boards ([i]Ethernet support[/i])
[*] Olimex ESP32 Gateway boards ([i]Ethernet support[/i])
[*] etc
[/list]

[b]Circuits Zero-Cross:[/b]

[list]
[*] basés sur JSY-MK-194G (ce JSY a un pin Zx pour détecter le Zero-Cross)
[*] basés sur BM1Z102FJ
[*] basé sur un pulse court comme RobotDyn ou le module ZCD de Daniel S. sur PCB Way
[/list]

[b]Outils de mesure: [/b]

[list]
[*] PZEM-004T v3
[*] JSY-MK-333 (tri-phasé)
[*] JSY-MK-193
[*] JSY-MK-194T
[*] JSY-MK-194G
[*] JSY-MK-163T
[*] JSY-MK-227
[*] JSY-MK-229
[*] JSY Remote par UDP
[*] MQTT
[/list]

[b]Dimmers supportés en contrôle de phase avec un module Zero-Cross:[/b]

[list]
[*] RobotDyn 24/40A
[*] Triac + détection Zero-Cross
[*] SSR Random + détection Zero-Cross
[*] Régulateur de tension LSA ou LCTC + module de conversion PWM->Analog 0-10V + détection Zero-Cross
[/list]

[b]Dimmers supportés en contrôle de phase qui fonctionnent via PWM (sans détection Zero-Cross):[/b]

[list]
[*] Régulateur de tension LSA ou LCTC + module de conversion PWM->Analog 0-10V
[/list]

[b]Dimmers supportés en contrôle de phase qui fonctionnent via DAC (sans détection Zero-Cross):[/b]

[list]
[*] Régulateur de tension LSA ou LCTC + DFRobot DAC GP8211S (DFR1071)
[*] Régulateur de tension LSA ou LCTC + DFRobot DAC GP8403 (DFR0971)
[*] Régulateur de tension LSA ou LCTC + DFRobot DAC GP8413 (DFR1073)
[/list]

[size=150]Exemples de configurations:[/size]

Le site YaSolR propose [url=https://yasolr.carbou.me/build]plusieurs exemples de configurations[/url], mais à vous de jouer pour créer la votre.
Par exemple. YaSolR est compatible avec le matériel de F1ATB, Prof solaire, etc. Donc il vous suffit d'installer et configurer les GPIO.

YaSolR est aussi l'un des rare routeurs à supporter directement le régulateur de tension LSA via un DAC, sans passer par un module Zero-Cross, ni une second alimentation!
Il est donc possible de construire un routeur solaire avec un ESP32, un DAC DFRobot et un LSA uniquement!

[size=150]YaSolR Pro[/size]

YaSolR est Open-Source et disponible gratuitement.

Cependant, une [url=https://yasolr.carbou.me/pro]version Pro[/url] est aussi disponible avec une interface graphique plus jolie et complète, qui se base sur [url=https://espdash.pro]ESP-DASH Pro[/url], une librairie graphique commerciale dont la license ne permet pas son intégration dans les projets Open-Source.
Je rends donc disponible cette version Pro aux utilisateurs qui soutiennent le projet via un don, ce qui permet d'aider à financer le développement, maintenance et achat de matériel afin que le projet puisse continuer à évoluer et son code rester accessible gratuitement.