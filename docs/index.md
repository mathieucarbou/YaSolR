---
layout: default
title: Home
description: Home
---

# YaSolR ?

YaSolR is an ESP32 firmware for **Solar Router** compatible with nearly **any existing hardware components**.

If you already have a Solar Router at home based on ESP32, built by yourself or someone else, there is a good chance that YaSolR will be compatible.

| [![](./assets/img/screenshots/overview.jpeg)](./assets/img/screenshots/overview.jpeg) | [![](./assets/img/screenshots/output1.jpeg)](./assets/img/screenshots/output1.jpeg) |

YaSolR **is a software** that will run on your Solar Router.

YaSolR **does not come with hardware**.
But this website will help you pick and build your router.

## Benefits

YaSolR is one of the **most optimized and powerful** Solar Router firmware available:

- **12-bits resolution** with linear interpolation for a precise TRIAC control
- **25 measurements / second** with a local JSY
- **20 measurements / second** with a remote JSY
- **PID Controller** optimized and customizable
- **PID Tuning** web interface
- **RMT Peripheral** used for DS18 readings
- **Harmonics** can be lowered to comply with regulations thanks to several settings
- **Custom dimmer library** optimized for ESP32 (🚧)
- **MCPWM** (Motor Control Pulse Width Modulator) for phase control (🚧)
- **3-Phase** support

This is a big **Open-Source** project following **best development practices**.
YaSolR is:

- **flexible** by allowing you to pick the hardware you want
- **easy to use** with one of the **best easy-to-use and responsive Web Interface**, **REST** API and **MQTT** API
- compatible with **Home Assistant** and other home automation systems (Auto Discovery)
- compatible with **EV Charging Box** like OpenEVSE
- compatible with **110/230V 50/60Hz**
- compatible with **remote** dimmer, relay, temperature, measurement (**ESP-Now** / **UDP**)
- compatible with **many hardware components**:
  - ESP32 Dev Kit
  - ESP32 S3 Dev Kit
  - ESP32s
  - Lilygo T Eth Lite S3 (**Ethernet support**)
  - WT32-ETH01 (**Ethernet support**)
  - Random and Zero-Cross SSR
  - Standalone Zero-Cross Detection modules
  - Robodyn 24A / 40A
  - Voltage Regulators (Loncont LSA or LCTC)
  - DS18 Temperature Sensors
  - etc

## Detailed Features

- [Routing Outputs](#routing-outputs)
  - [Dimmer (required)](#dimmer-required)
  - [Bypass Relay (optional)](#bypass-relay-optional)
  - [Temperature (optional)](#temperature-optional)
  - [Measurement device (optional)](#measurement-device-optional)
  - [Additional Features](#additional-output-features)
- [Grid Power Measurement](#grid-power-measurement)
  - [JSY-MK-194T](#jsy-mk-194t)
  - [Remote Grid Measurement](#remote-grid-measurement)
  - [3-Phase Support](#3-phase-support)
- [2x Relays](#2x-relays)
- [Monitoring and Management](#monitoring-and-management)
- [MQTT, REST API and Home Automation Systems](#mqtt-rest-api-and-home-automation-systems)
- [Online / Offline modes](#online--offline-modes)
- [PID Control and Tuning](#pid-control-and-tuning)
- [Remote Capabilities](#remote-capabilities)
- [Virtual Excess and EV Charger Compatibility](#virtual-excess-and-ev-charger-compatibility)

### Routing Outputs

YaSolR supports up to 2 outputs.
A routing output is connected to a resistive load and controls its power by dimming the voltage.
Each output is composed of the following components:

- [Dimmer (required)](#dimmer-required)
- [Bypass Relay (optional)](#bypass-relay-optional)
- [Temperature (optional)](#temperature-optional)
- [Measurement device (optional)](#measurement-device-optional)
- [Additional Output Features](#additional-output-features)

#### Dimmer (required)

A dimmer controls the power sent to the load.
Example of supported dimmers:

| Dimmer Type                                                                | `Phase Control` | `Burst Fire Control` (🚧) |
| :------------------------------------------------------------------------- | :-------------: | :-----------------------: |
| **Robodyn 24A**<br> ![](./assets/img/hardware/Robodyn_24A.jpeg)            |       ✅        |            ✅             |
| **Robodyn 40A**<br> ![](./assets/img/hardware/Robodyn_40A.jpeg)            |       ✅        |            ✅             |
| **Random SSR**<br> ![](./assets/img/hardware/Random_SSR.jpeg)              |       ✅        |            ✅             |
| **Zero-Cross SSR** (🚧)<br> ![](./assets/img/hardware/SSR_40A_DA.jpeg)     |       ❌        |            ✅             |
| **Voltage Regulator** (🚧)<br> ![](./assets/img/hardware/LSA-H3P50YB.jpeg) |       ✅        |            ✅             |

#### Bypass Relay (optional)

A bypass relay allows to force a heating at full power and bypass the dimmer at a given schedule or manually.
Keeping a dimmer `on` generates heat so a bypass relay can be installed to avoid using the dimmer.

**If a bypass relay is installed, the dimmer will be used instead and will be set to 0-100% to simulate the relay.**

|                      Electromagnetic Relay                       |                      Zero-Cross SSR                      |                      Random SSR                      |
| :--------------------------------------------------------------: | :------------------------------------------------------: | :--------------------------------------------------: |
| ![Electromagnetic Relay](./assets/img/hardware/DIN_2_Relay.jpeg) | ![Zero-Cross SSR](./assets/img/hardware/SSR_40A_DA.jpeg) | ![Random SSR](./assets/img/hardware/Random_SSR.jpeg) |

#### Temperature (optional)

Measuring the temperature of the water tanker is important to ve able to trigger automatic heating based on temperature thresholds, or stop the routing if the temperature i reached.

This can also be done:

- using a remote temperature sensor through MQTT
- using a remote temperature sensor through ESP-Now (🚧)
- using one of the supported sensor:

|                    DS18B20                     |
| :--------------------------------------------: |
| ![DS18B20](./assets/img/hardware/DS18B20.jpeg) |

#### Measurement device (optional)

Each output supports an optional measurement device to measure the power routed to the load.

- `JSY-MK-194T`: has 2 clamps, and is used to monitor the aggregated routed output power (sum of the two outputs) and the grid power with the second clamp.
- `PZEM-004T V3`: can monitor each output independently. **It cannot be used to measure the grid power.**

|                     PZEM-004T V3                      |                       JSY-MK-194T                        |
| :---------------------------------------------------: | :------------------------------------------------------: |
| ![PZEM-004T V3](./assets/img/hardware/PZEM-004T.jpeg) | ![JSY-MK-194T](./assets/img/hardware/JSY-MK-194T_2.jpeg) |

#### Additional Output Features

- `Bypass Automatic Control` / `Manual Control`: Automatically force a heating as needed based on days, hours, temperature range, or control it manually
- `Dimmer Automatic Control` / `Manual Control`: Automatically send the grid excess to the resistive load through the dimmer (or manually control the dimmer yourself if disabled)
- `Independent or Sequential Outputs with Grid Excess Sharing`: Outputs are sequential by default (second output activated after first one at 100%).
  **Independent outputs are also supported** thanks to the sharing feature to split the excess between outputs.
  This feature is available in `Dimmer Automatic Control` mode.
- `Dimmer Duty Limiter`: Set a limit to the dimmer power to avoid routing too much power
- `Dimmer Temperature Limiter`: Set a limit to the dimmer to stop it when a temperature is reached. This temperature can be different than the temperature used in auto bypass mode.
- `Statistics`: Harmonic information, power factor, energy, routed power, etc
- `Automatic Resistance Calibration` (🚧): if a JSY or PZEM is installed, automatically discover and save the resistance value of the connected load.

### Grid Power Measurement

Measuring the grid power is essential to know how much power is available to route.
YaSolR supports many ways to measure the grid power and voltage:

**Mono-phase**:

- `MQTT` (**Home Assistant**, **Jeedom**, `Shelly EM`, etc)
- `JSY-MK-194T` (25 measurements / second)
- Remote `JSY-MK-194T` through **UDP** (20 measurements / second)
- Remote `JSY-MK-194T` through **ESP-Now** (🚧)

**3-Phase**:

- `MQTT` (**Home Assistant**, **Jeedom**, `Shelly 3EM`, etc)
- `JSY-MK-333` (🚧)
- Remote `JSY-MK-333` through **UDP** (🚧)
- Remote `JSY-MK-333` through **ESP-Now** (🚧)

### Relays

YaSolR supports up to 2 relays to control external resistive loads or contactors.

- Supports `NO / NC` relay type
- `Automatic Control` / `Manual Control`: You can specify the resistive load power in watts connected to the relay.
  If you do so, the relay will be automatically activated based on the excess power.

### Monitoring and Management

**Dashboard**:

- `Charts`: Live charts (grid power, routed power, THDi, PID tuning, etc)
- `Health`: configuration mistakes are detected as much as possible and issues displayed when a component was found to not properly work.
- `Languages (i18n)`: en / fr
- `LEDs`: Support LEDs for visual alerts
- `Manual Override`: Override anything remotely (MQTT, REST, Website)
- `GPIO`: A GPIO section shows the view of configured pins and activated pins, to report issues, duplications or invalid pins.
- `Restart`, `Factory Reset`, `Config Backup & Restore`, `Debug Logging`
- `Statistics`: Harmonic information, power factor, energy, routed power, grid power, grid frequency and voltage, etc
- `Web console`: View ESP logs live from a Web interface
- `Web OTA Firmware Update`: Update firmware through the Web interface

**Hardware**:

- `Display`: Support I2C OLED Display (`SSD1307`, `SH1106`, `SH1107`)
- `Push Button`: Support a push button to restart the device
- `Temperature Sensor`: Support a temperature rensor for the router box (`DS18B20`)

### MQTT, REST API and Home Automation Systems

The router exposes a lot of statistics and information through MQTT and REST API and provides a very good integration with Home Assistant or other home automation systems.
The router can be completely controlled remotely through a Home Automation System.

- `REST API`: extensive REST API support
- `MQTT`: extensive MQTT API (with `TLS` support)
- [Home Assistant Integration](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery): Activate Home Assistant Auto Discovery to automatically create a YaSolR device in Home Assistant with all the sensors

### Online / Offline modes

- `Access Point Mode`: router can **work in AP mode without WiFi and Internet**
- `Admin Password`: to protect the website, API and Access Point
- `Captive Portal` a captive portal is started first time to help you connect the router
- `DNS & mDNS`: Discover the device on the network through mDNS (Bonjour) or DNS
- `Ethernet & Wifi`: **ESP32 boards with Ethernet and WiFi are supported**
- `NTP` support to synchronize time and date with Internet. If not activated, it is still possible to manually sync with your browser.
- `Offline Mode`: **The router can work without WiFi, even teh features requiring time and date.**

### PID Control and Tuning

The router uses a PID controller to control the dimmers and you have full control over the PID parameters to tune it.

Demo on Youtube:

[![PID Tuning in YaSolR (Yet Another Solar Router)](https://img.youtube.com/vi/ygSpUxKYlUE/0.jpg)](https://www.youtube.com/watch?v=ygSpUxKYlUE "PID Tuning in YaSolR (Yet Another Solar Router)")

### Virtual Excess and EV Charger Compatibility

Thanks to power measurement, the router also provides these features:

- `Virtual Excess`: Expose the virtual excess (MQTT, REST API, web) which is composed of the current excess plus the routing power
- `EV Charger Compatibility` (i.e OpenEVSE): Don't prevent an EV charge to start (router can have lower priority than an EV box to consume available production excess)

**A measurement device is required to use these features.**

## OSS vs PRO

OSS and Pro firmware are the same, except that the PRO version relies on commercial (paid) libraries and provides some additional features based on a better dashboard.

**The Pro version is only 25 euros** and gives access to all the perks of the Pro version below:

| Feature                          |                            OSS (Free)                            |                                                                                              PRO (Paid)                                                                                              |
| -------------------------------- | :--------------------------------------------------------------: | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
| Dashboard                        |                        Overview **only**                         |                                                                              Full Dashboard as seen in the screenshots                                                                               |
| Manual Dimmer Control            |              Home Assistant<br>MQTT API<br>REST API              |                                                                     **From Dashboard**<br>Home Assistant<br>MQTT API<br>REST API                                                                     |
| Manual Bypass Control            |              Home Assistant<br>MQTT API<br>REST API              |                                                                     **From Dashboard**<br>Home Assistant<br>MQTT API<br>REST API                                                                     |
| Manual Relay Control             |              Home Assistant<br>MQTT API<br>REST API              |                                                                     **From Dashboard**<br>Home Assistant<br>MQTT API<br>REST API                                                                     |
| Configuration                    |                        Debug Config Page                         |                                                                               **From Dashboard**<br>Debug Config Page                                                                                |
| Automatic Resistance Calibration |                                ❌                                |                                                                                                  ✅                                                                                                  |
| Energy Reset                     |                                ❌                                |                                                                                                  ✅                                                                                                  |
| GPIO Config and Health           |                                ❌                                |                                                                                                  ✅                                                                                                  |
| Hardware Config and Health       |                                ❌                                |                                                                                                  ✅                                                                                                  |
| Output Statistics                |                                ❌                                |                                                                                                  ✅                                                                                                  |
| PID Tuning View                  |                                ❌                                |                                                                                                  ✅                                                                                                  |
| PZEM Pairing                     |                                ❌                                |                                                                                                  ✅                                                                                                  |
| Help & Support                   |     [Facebook Group](https://www.facebook.com/groups/yasolr)     | [Facebook Group](https://www.facebook.com/groups/yasolr) <br> [Forum](https://github.com/mathieucarbou/YaSolR-OSS/discussions) <br> [Bug Report](https://github.com/mathieucarbou/YaSolR-OSS/issues) |
| Web Console                      | [WebSerial Lite](https://github.com/mathieucarbou/WebSerialLite) |                                                                              [WebSerial Pro](https://www.webserial.pro)                                                                              |
| Dashboard                        |      [ESP-DASH](https://github.com/ayushsharma82/ESP-DASH)       |                                                                                 [ESP-DASH Pro](https://espdash.pro)                                                                                  |
| OTA Firmware Update              |    [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)     |                                                                               [ElegantOTA Pro](https://elegantota.pro)                                                                               |

The money helps funding the hardware necessary to test and develop the firmware.

## Alternatives and Inspirations

This project was inspired by the following awesome Solar Router projects:

- [Routeur Solaire Apper](https://ota.apper-solaire.org) (Cyril P., _[xlyric](https://github.com/xlyric)_)
- [Routeur Solaire PVbrain](https://github.com/SeByDocKy/pvbrain2) (Sébastien P., _[SeByDocKy](https://github.com/SeByDocKy)_)
- [Routeur Solaire MK2 PV Router](https://www.mk2pvrouter.co.uk) (Robin Emley)
- [Routeur Solaire Mk2 PV Router](https://github.com/FredM67/Mk2PVRouter) (Frédéric M.)
- [Routeur Solaire Tignous](https://forum-photovoltaique.fr/viewtopic.php?f=110&t=40512) (Tignous)
- [Routeur Solaire MaxPV](https://github.com/Jetblack31/MaxPV) (Jetblack31)
- [Routeur solaire "Le Profes'Solaire"](https://sites.google.com/view/le-professolaire/routeur-professolaire) (Anthony G., _Le Profes'Solaire_)
- [Routeur solaire "Le Profes'Solaire"](https://github.com/benoit-cty/solar-router) (Adapation from Benoit C.)
- [Routeur solaire Multi-Sources Multi-Modes et Modulaire](https://f1atb.fr/fr/realisation-dun-routeur-photovoltaique-multi-sources-multi-modes-et-modulaire/) (André B., _[F1ATB](https://github.com/F1ATB)_)
- [Routeur solaire ESP Home](https://domo.rem81.com/index.php/2023/07/18/pv-routeur-solaire/) (Remy)
