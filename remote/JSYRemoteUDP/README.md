# Remote JSY with UDP

The `Remote JSY with UDP` firmware will communicate with YaSolR through UDP protocol to send the JSY data each 500 ms.
Both devices needs to be connected to the same WiFi network and UDP packets must be allowed.

![](https://github.com/mathieucarbou/YaSolR-OSS/assets/61346/b375bfda-6bc1-4576-afeb-26594b5c8649)
![](https://github.com/mathieucarbou/YaSolR-OSS/assets/61346/b1e893d1-a272-4006-9df6-ebbe091a0ce1)

## How to build

1. Just attach the JSY module to the ESP32 as described in the YaSolR [Build](https://yasolr.carbou.me/build#default-gpio-pinout-per-board) page.
2. Advise is to power the ESP32 thanks to a MeanWell HDR-15-5 DIN module which is placed just after a 2A breaker.
3. Connect L + N wires to the JSY and set teh clamp around the main phase.
4. Ideally, put everything in an isolated box or DIN rail: YaSolR [Build](https://yasolr.carbou.me/build) page contains references to DIN mounts for ESP32 and JSY.

## Installation with Arduino IDE

_I am not familiar with Arduino IDE because this is not a correct IDE for development._
_Please help contribute to this README if the explanation needs to be improved._

1. You need to install these Arduino dependencies from the library manager or by downloading them:

```c++
#include <ArduinoJson.h>       // https://github.com/bblanchon/ArduinoJson
#include <AsyncTCP.h>          // https://github.com/mathieucarbou/AsyncTCP
#include <ESPAsyncWebServer.h> // https://github.com/mathieucarbou/ESPAsyncWebServer
#include <ESPDash.h>           // https://github.com/mathieucarbou/ayushsharma82-ESP-DASH#dev
#include <ElegantOTA.h>        // https://github.com/mathieucarbou/ayushsharma82-ElegantOTA#dev
#include <MycilaESPConnect.h>  // https://github.com/mathieucarbou/MycilaESPConnect
#include <MycilaJSY.h>         // https://github.com/mathieucarbou/MycilaJSY
#include <MycilaLogger.h>      // https://github.com/mathieucarbou/MycilaLogger
#include <MycilaSystem.h>      // https://github.com/mathieucarbou/MycilaSystem
#include <MycilaTaskManager.h> // https://github.com/mathieucarbou/MycilaTaskMonitor
#include <MycilaTaskMonitor.h> // https://github.com/mathieucarbou/MycilaTaskMonitor
#include <WebSerialLite.h>     // https://github.com/mathieucarbou/WebSerialLite
```

The program also uses these Arduino libraries:

```
DNSServer
ESP32 Async UDP
ESPmDNS
FS
Networking
LittleFS
WiFi
Ticker
Update
```

2. Compile and deploy to your board.

## Installation with PlatformIO

This is as simple as running:

```bash
pio run -t upload -e <your-board>
```

## Usage

Quick start is similar to YaSolR.
The device will open an access point, you need to connect to it and choose the WiFI to join.

Once the device has joined the WiFi and is connected to a JSY, you will see the JSY data on the dashboard of this device, but also on the YaSolR dashboard: graphs will be updated in real-time.

- To look at the logs, go to: `http://<device-ip>/console`

- To update the firmware, go to: `http://<device-ip>/update`

- To add a password, change the line `#define YASOLR_ADMIN_PASSWORD ""`
