# Remote JSY

You can install a JSY with an ESP32 on the electric panel and it will send its JSY data to the router remotely.

## Remote JSY with ESP-Now

The `Remote JSY with ESP-Now` firmware will communicate with YaSolR through the ESP-Now protocol.

ESP-Now is a low level protocol which is not using your WiFi (but has the same frequency and channel).
This protocol allows ESP32 devices to talk to each other without the need of a WiFi network.
It is fast and support long range communication.
This option is suitable if you cannot connect the ESP32 to your home WiFi network where the electric panel is located.

=> [JSYRemoteESPNow](JSYRemoteESPNow)

## Remote JSY with UDP

The `Remote JSY with UDP` firmware will communicate with YaSolR through UDP protocol to send the JSY data each 500 ms.
Both devices needs to be connected to the same WiFi network and UDP packets must be allowed.

=> [JSYRemoteUDP](JSYRemoteUDP)
