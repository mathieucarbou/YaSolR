---
layout: default
title: Blog
description: Remote JSY through UDP
---

_Date: 2024-06-25_

# Remote JSY

The free [JSY library](https://oss.carbou.me/MycilaJSY/) has been completed with 2 new examples to show how to use the JSY remotely through UDP.

The `Sender` program must be uploaded to an ESP32 connected to a JSY.
It sends through UDP broadcast the JSY data several times per second.
It can also be used as a standalone app to display the JSY data in real-time.

![](https://github.com/mathieucarbou/MycilaJSY/assets/61346/3066bf12-31d5-45de-9303-d810f14731d0)

There is also a `Listener` example which is the same app, bit not connected to the JSY, but will receive the data through UDP and display the metrics.

The 2 samples are made with ESP-DASH, ElegantOTA, WebSerial, MycilaESpConnect, etc.

=> [MycilaJSY RemoteUDP](https://github.com/mathieucarbou/MycilaJSY/tree/main/examples/RemoteUDP)
=> [MycilaJSY](https://oss.carbou.me/MycilaJSY/) project

## Remote JSY in YaSolR

The YaSolR project also support the JSY `Sender`. YaSolR will listen for these UDP packets and will use the remote JSY data if they are available.

You can have an ESP32 board installed in your electric box with a JSY and the `Sender` app, and YaSolr firmware installed next to the loads.

When using a remote JSY in YaSolR, the following rules apply:

- The voltage will always be read if possible from a connected JSY or PZEM, then from a remote JSY, then from MQTT.
- The grid power will always be read first from MQTT, then from a remote JSY, then from a connected JSY.

## Speed

The JSY Remote through UDP is nearly as fast as having the JSY wired to the ESP.
All changes to the JSY are immediately sent through UDP to the listener at a rate of about **20 messages per second**.
This is the rate at which the JSY usually updates its data.
