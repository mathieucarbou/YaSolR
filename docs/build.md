---
layout: default
title: Build
description: Build
---

# How to build your router

- [Build Examples](#build-examples)
  - [The Recycler](#the-recycler)
  - [The Minimalist](#the-minimalist)
  - [The Elegant](#the-elegant)
  - [The Adventurer](#the-adventurer)
  - [The Elite](#the-elite)
  - [The Professional](#the-professional)
    * [Voltage Regulator + PWM to Analog Converter + ZCD](#voltage-regulator--pwm-to-analog-converter--zcd)
    * [Voltage Regulator + PWM to Analog Converter only](#voltage-regulator--pwm-to-analog-converter-only)
    * [Voltage Regulator + DAC only](#voltage-regulator--dac-only)
  - [Possible Upgrades](#possible-upgrades)
  - [Remote JSY](#remote-jsy)
  - [Alternative: The Shelly Solar Diverter](#alternative-the-shelly-solar-diverter)
  - [Alternative: Home Assistant Solar Diverter](#alternative-home-assistant-solar-diverter)
- [Selecting your Hardware](#selecting-your-hardware)
  - [ESP32 Boards](#esp32-boards)
  - [Dimmers: RobotDyn, Solid State Relay or Voltage Regulator ?](#dimmers-robotdyn-solid-state-relay-or-voltage-regulator-)
  - [Relays: Solid State Relay or Electromagnetic Relay ?](#relays-solid-state-relay-or-electromagnetic-relay-)
  - [How to choose a Solid State Relay ?](#how-to-choose-a-solid-state-relay-)
- [Where to buy ?](#where-to-buy-)
  - [ESP32 Boards](#esp32-boards-1)
  - [RobotDyn](#robotdyn)
  - [Random and Zero-Cross SSR](#random-and-zero-cross-ssr)
  - [Zero-Cross Detection Modules](#zero-cross-detection-modules)
  - [Voltage Regulators](#voltage-regulators)
  - [Electromagnetic Relay](#electromagnetic-relay)
  - [Measurement Devices](#measurement-devices)
  - [Temperature Sensors, LEDs, Buttons, Displays](#temperature-sensors-leds-buttons-displays)
  - [Mounting Accessories](#mounting-accessories)
- [Default GPIO pinout per board](#default-gpio-pinout-per-board)
- [Wiring](#wiring)
  - [How to wire RobotDyn dimmer](#how-to-wire-robotdyn-dimmer)
  - [How to wire Random Solid State Relay dimmer](#how-to-wire-random-solid-state-relay-dimmer)
  - [How to wire PWM controlled Voltage Regulator dimmer](#how-to-wire-pwm-controlled-voltage-regulator-dimmer)
  - [How to wire DAC controlled Voltage Regulator dimmer](#how-to-wire-dac-controlled-voltage-regulator-dimmer)
  - [How to wire Bypass Relay](#how-to-wire-bypass-relay)
  - [How to wire JSY and PZEM](#how-to-wire-jsy-and-pzem)
  - [How to wire accessories](#how-to-wire-accessories)
  - [How to wire Random SSR with JSY-MK-194G ZCD](#how-to-wire-random-ssr-with-jsy-mk-194g-zcd)

## Build Examples

If you already have a solar router like [F1ATB](https://f1atb.fr/fr/realisation-dun-routeur-photovoltaique-multi-sources-multi-modes-et-modulaire/) or ["Profes'Solaire"](https://sites.google.com/view/le-professolaire/routeur-professolaire), just flash the YaSolR firmware instead of the one you are using.
Most hardware used in these routers are compatible with YaSolR.

YaSolR supports many builds and routing algorithms.
Before building your router, you need to decide which type of hardware you want to use.
Here are below some examples:

- [The Recycler](#the-recycler): reuse your existing Shelly EM or Shelly 3EM to build a router
- [The Minimalist](#the-minimalist): the cheapest and easiest to build
- [The Elegant](#the-elegant): an improved version of the Minimalist using the new JSY-MK-194G integrated ZCD circuit
- [The Adventurer](#the-adventurer): for people who want to mitigate the flaws of the RobotDyn and do some improvements over the existing RobotDyn
- [The Elite](#the-elite): for people who want to use a Random SSR instead of a RobotDyn to safely dim more power and have a better Zero-Cross Detection circuit
- [The Professional](#the-professional): probably the best and safe solution out there but requires an additional power source
  * [Voltage Regulator + PWM to Analog Converter + ZCD](#voltage-regulator--pwm-to-analog-converter--zcd)
  * [Voltage Regulator + PWM to Analog Converter only](#voltage-regulator--pwm-to-analog-converter-only)
  * [Voltage Regulator + DAC only](#voltage-regulator--dac-only)
- [Possible Upgrades](#possible-upgrades): some additional components you can add to your router
- [Remote JSY](#remote-jsy): a standalone application to place in your electrical panel to send the JSY metrics through MycilaJSY App for remote installations
- [Alternative: The Shelly Solar Diverter](#alternative-the-shelly-solar-diverter): a limited Solar Diverter / Router with Shelly devices and a voltage regulator
- [Alternative: Home Assistant Solar Diverter](#alternative-home-assistant-solar-diverter)

### The Recycler

Reuse your existing Shelly EM or Shelly 3EM to build a router!

- RobotDyn is used for the dimming (phase control)
- RobotDyn is used for the Zero-Cross Detection
- Shelly EM or MQTT is used to measure the grid power and voltage

|                              ESP32                               |                       RobotDyn AC Dimmer 40A/800V                       |                          Shelly EM or 3EM                           |
| :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | :-----------------------------------------------------------------: |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/RobotDyn_40A.jpeg" style="width:150px"> | <img src="./assets/img/hardware/Shelly_EM.png" style="width:150px"> |

> ##### TIP
>
> - RobotDyn includes Zero-Cross Detection circuit
> - Supports **Phase Control** and **Cycle Stealing**
> - Reuse your Shelly EM or 3EM and send through MQTT grid power and voltage
> - You can also use a SSR and ZCD module instead of the RobotDyn
{: .block-tip }

> ##### WARNING
>
> - Advised load not more than 2000W
> - RobotDyn has poor quality heat sink, soldering and Zero-Cross pulse
> - Bypass mode will use the RobotDyn dimmer at 100% power
> - Not as precise as a JSY (MQTT delays)
> - No local measurement in place to measure the routed power (statistics will be empty)
{: .block-warning }

### The Minimalist

The _Minimalist_ build uses inexpensive and easy to use components to start a router.

- RobotDyn is used for the dimming (phase control)
- RobotDyn is used for the Zero-Cross Detection
- Grid power measurement can be done with a JSY (local or remote) or MQTT

|                              ESP32                               |                       RobotDyn AC Dimmer 40A/800V                       |             JSY-MK-163T, JSY-MK-193, JSY-MK-194T, JSY-MK-194G or JSY-MK-333           |
| :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | :----------------------------------------------------------------------: |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/RobotDyn_40A.jpeg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> |

> ##### TIP
>
> - RobotDyn includes Zero-Cross Detection circuit
> - Supports **Phase Control** and **Cycle Stealing**
{: .block-tip }

> ##### WARNING
>
> - Advised load not more than 2000W
> - RobotDyn has poor quality heat sink, soldering and Zero-Cross pulse
> - Bypass mode will use the RobotDyn dimmer at 100% power
{: .block-warning }

### The Elegant

This is an improved version of the _Minimalist_ build using the new JSY-MK-194G which has an integrated ZCD.

- A Random SSR is used for the dimming (phase control)
- JSY-MK-194G (local) is used for the Zero-Cross Detection
- JSY-MK-194G (local) is used to measure the grid power and voltage

|                              ESP32                               |                       Random Solid State Relay                        |             JSY-MK-194G                   |
| :----------------------------------------------------------------------: | :-------------------------------------------------------------------: | :----------------------------------------------------------------------: |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/Random_SSR.jpeg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194G_3.jpeg" style="width:150px"> |

> ##### TIP
>
> - JSY-MK-194G includes a Zero-Cross Detection circuit
> - Supports **Phase Control** and **Cycle Stealing**
{: .block-tip }

> ##### WARNING
>
> - Bypass mode will use the RobotDyn dimmer at 100% power
{: .block-warning }

### The Adventurer

The _Adventurer_ build is for people who are able to mitigate the flaws of the RobotDyn 24A to improve it.
The TRIAC can be changed to a BTA40-800B RD91 fixed directly on the heat sink, and the heat sink can be upgraded.
See the [RobotDyn](#robotdyn) section for more information.

- A custom TRIAC is used for the dimming (phase control)
- RobotDyn is used for the Zero-Cross Detection
- Grid power measurement can be done with a JSY (local or remote) or MQTT

|                              ESP32                               |                       RobotDyn AC Dimmer 24A/600V                       |                              Heat Sink                               |                         Triac BTA40-800B RD91                         | JSY-MK-163T, JSY-MK-193, JSY-MK-194T, JSY-MK-194G or JSY-MK-333                          |
| :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | :------------------------------------------------------------------: | :-------------------------------------------------------------------: | ------------------------------------------------------------------------ |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/RobotDyn_24A.jpeg" style="width:150px"> | <img src="./assets/img/hardware/Heat_Sink.jpeg" style="width:150px"> | <img src="./assets/img/hardware/BTA40-800B.jpeg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> |

> ##### TIP
>
> - RobotDyn includes Zero-Cross Detection circuit
> - Supports **Phase Control** and **Cycle Stealing**
{: .block-tip }

> ##### WARNING
>
> - Advised load not more than 2000W
> - RobotDyn has poor quality heat sink, soldering and Zero-Cross pulse
> - Bypass mode will use the RobotDyn dimmer at 100% power
> - Requires to unsolder the heat sink and triac and put a new triac on a new heat sink
{: .block-warning }

### The Elite

The _Elite_ build is for people who want to use a Random SSR instead of a RobotDyn to safely dim more power and have a better Zero-Cross Detection circuit more more precising routing.

- A Random SSR is used for the dimming (phase control)
- A dedicated ZCD circuit is used for the Zero-Cross Detection (it can be the JSY-MK-194G)
- Grid power measurement can be done with a JSY (local or remote) or MQTT

|                              ESP32                               |                       Random Solid State Relay                        |                              Heat Sink                               |                  Zero-Cross Detection Module                   | JSY-MK-163T, JSY-MK-193, JSY-MK-194T, JSY-MK-194G or JSY-MK-333                          |
| :----------------------------------------------------------------------: | :-------------------------------------------------------------------: | :------------------------------------------------------------------: | :------------------------------------------------------------: | ------------------------------------------------------------------------ |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/Random_SSR.jpeg" style="width:150px"> | <img src="./assets/img/hardware/Heat_Sink.jpeg" style="width:150px"> | <img src="./assets/img/hardware/ZCD.jpeg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> |

> ##### TIP
>
> - The **JSY-MK-194G has an integrated ZCD** and can be used with a Random SSR directly without the need of an external ZCD module.
> - Dedicated ZCD circuit with a good pulse
> - Dedicated Random SSR (models up to 100A)
> - Supports **Phase Control** and **Cycle Stealing**
> - Other types of Heat Sink are available: the image above is just an example. Pick one according to your load.
> - All the components can be easily attached onto a DIN rail
{: .block-tip }

> ##### WARNING
>
> - Bypass mode will use the SSR dimmer set at 100% power
{: .block-warning }

### The Professional

The _Professional_ build uses a Voltage Regulator to control the power routing.
This is probably the best reliable and efficient solution, but it is more complex to setup and wire.
Voltage regulators like the LSA or LCTC have an integrated ZCD circuit to trigger at the right time based on the input voltage control.

There are multiple ways to control a voltage regulator with YaSolR.

> ##### TIP
>
> - Dedicated hardware supporting high loads
> - Supports **Phase Control** and **Cycle Stealing**
> - Heat sink are bigger and better quality: bigger models are also available
{: .block-tip }

> ##### WARNING
>
> - Bypass mode will use the dimmer set at 100% power
{: .block-warning }

#### Voltage Regulator + PWM to Analog Converter + ZCD

This option is a good upgrade to a RobotDyn or SSR since it reuses the same components and just replaces the dimmer part with the LSA.
It even reuses the existing ZCD circuit, and a dimmer implementation based on a ZCD, as usual, like for the RobotDyn or Random SSR.
As such, the LSA behavior is controlled exactly like a RobotDyn or SSR would be controlled.

- A voltage regulator (LSA or LCTC) is used for the dimming (phase control)
- A dedicated **ZCD circuit** is used for the Zero-Cross Detection (it can be the JSY-MK-194G)
- Grid power measurement can be done with a JSY (local or remote) or MQTT
- A PWM to Analog Converter is used to control the voltage regulator (**requires a 12V input**)

|                              ESP32                               |                           Voltage Regulator                            |                                Heat Sink                                 |                        PWM to Analog Converter                         | JSY-MK-163T, JSY-MK-193, JSY-MK-194T, JSY-MK-194G or JSY-MK-333                          |
| :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | ------------------------------------------------------------------------ |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/LSA-H3P50YB.jpeg" style="width:150px"> | <img src="./assets/img/hardware/lsa_heat_sink.jpeg" style="width:150px"> | <img src="./assets/img/hardware/PWM_33_0-10.jpeg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> |

> ##### WARNING
>
> - Requires an additional 12V power supply (e.g. Mean Well HDR-15-15 12V DC)
{: .block-warning }

#### Voltage Regulator + PWM to Analog Converter only

This solution skips the need to have a dedicated ZCD and uses a specific implementation in YaSolR which is specific to this PWM to Analog Converter.

- A voltage regulator (LSA or LCTC) is used for the dimming (phase control)
- Grid power measurement can be done with a JSY (local or remote) or MQTT
- A PWM to Analog Converter is used to control the voltage regulator (**requires a 12V input**)

|                              ESP32                               |                           Voltage Regulator                            |                                Heat Sink                                 |                        PWM to Analog Converter                         | JSY-MK-163T, JSY-MK-193, JSY-MK-194T, JSY-MK-194G or JSY-MK-333                          |
| :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | ------------------------------------------------------------------------ |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/LSA-H3P50YB.jpeg" style="width:150px"> | <img src="./assets/img/hardware/lsa_heat_sink.jpeg" style="width:150px"> | <img src="./assets/img/hardware/PWM_33_0-10.jpeg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> |

> ##### WARNING
>
> - Requires an additional 12V power supply (e.g. Mean Well HDR-15-15 12V DC)
{: .block-warning }

#### Voltage Regulator + DAC only

**Definitely the BEST solution out there**

This solution does not need a dedicated ZCD and uses a specific implementation in YaSolR which is specific to a DFRobot DAC.

The big advantage of a DAC module is that is allows to control a LSA directly without the need of an external 12V supply like for the PWM to Analog Converter, and it also does not need a ZCD module.

- A voltage regulator (LSA or LCTC) is used for the dimming (phase control)
- Grid power measurement can be done with a JSY (local or remote) or MQTT
- A [DFRobot DAC](https://www.dfrobot.com/blog-13458.html) is used to control the voltage regulator: DFR0971 (GP8403), DFR1073 (GP8413), DFR1071 (GP8211S)

|                              ESP32                               |                           Voltage Regulator                            |                                Heat Sink                                 |                        [DFRobot DAC](https://www.dfrobot.com/blog-13458.html)                         | JSY-MK-163T, JSY-MK-193, JSY-MK-194T, JSY-MK-194G or JSY-MK-333                          |
| :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | :----------------------------------------------------------------------: | :--------------------------------------------------------------------: | ------------------------------------------------------------------------ |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/LSA-H3P50YB.jpeg" style="width:150px"> | <img src="./assets/img/hardware/lsa_heat_sink.jpeg" style="width:150px"> | <img src="./assets/img/hardware/DFR0971.jpg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> |

> ##### TIP
>
> - No ZCD circuit required
> - No additional 12V power supply required
{: .block-tip }

### Possible Upgrades

Here are below what you can add to upgrade your router:

|                                Hardware                                | Description                                                                                                               |
| :--------------------------------------------------------------------: | :------------------------------------------------------------------------------------------------------------------------ |
| <img src="./assets/img/hardware/DIN_2_Relay.jpeg" style="width:150px"> | A bypass relay to avoid using the dimmer when auto bypass is enabled, and an additional relay to control an external load |
|   <img src="./assets/img/hardware/DS18B20.jpeg" style="width:150px">   | A temperature sensor to measure the water tank temperature to automatically stop or start the water heating               |
| <img src="./assets/img/hardware/PushButton.jpeg" style="width:150px">  | A push button to restart the router easily                                                                                |
|    <img src="./assets/img/hardware/LEDs.jpeg" style="width:150px">     | LEDs to display the system status                                                                                         |
|   <img src="./assets/img/hardware/SH1106.jpeg" style="width:150px">    | A display to show the router information                                                                                  |
|  <img src="./assets/img/hardware/PZEM-004T.jpeg" style="width:150px">  | A PZEM to precisely measure the routed power for each output. Only useful if you have more than one output.               |

### Remote JSY

Here are the components below to build a [MycilaJSY App](https://github.com/mathieucarbou/MycilaJSYApp).
This is a standalone application that looks looks like this and will show all your JSY data, help you manage it, and also send the data through UDP.
The reading rate is about **20-25 messages per second** and sending rate is 3 messages per second (because the JSY exposes 3 new measurements every second).

![](https://github.com/mathieucarbou/MycilaJSY/assets/61346/3066bf12-31d5-45de-9303-d810f14731d0)

You can look in the [MycilaJSY App](https://github.com/mathieucarbou/MycilaJSYApp) project to find more information about how to setup remote JSY and the supported protocols.

|                        Mean Well HDR-15-5 5V DC                        |                              ESP32                               | JSY-MK-163T, JSY-MK-193, JSY-MK-194T, JSY-MK-194G or JSY-MK-333                          |
| :--------------------------------------------------------------------: | :----------------------------------------------------------------------: | ------------------------------------------------------------------------ |
| <img src="./assets/img/hardware/DIN_HDR-15-5.jpeg" style="width:80px"> | <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px"> | <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> |

### Alternative: The Shelly Solar Diverter

It is also possible to build a (limited) Solar Diverter / Router with Shelly devices and a voltage regulator.

| [![](./assets/img/hardware/shelly_solar_diverter_poc2.jpeg)](./assets/img/hardware/shelly_solar_diverter_poc2.jpeg) | [![](./assets/img/screenshots/shelly_solar_diverter.jpeg)](./assets/img/screenshots/shelly_solar_diverter.jpeg) |

See this blog post for more information: [Shelly Solar Diverter](./blog/2024-07-01_shelly_solar_diverter)

### Alternative: Home Assistant Solar Diverter

It is also possible to build a (limited) Solar Diverter / Router with Home Assistant and a voltage regulator + Shelly dimmer.

| [![](./assets/img/hardware/shelly_solar_diverter_poc2.jpeg)](./assets/img/hardware/shelly_solar_diverter_poc2.jpeg) | [![](./assets/img/screenshots/HA_Router_Mode_AUTO.jpeg)](./assets/img/screenshots/HA_Router_Mode_AUTO.jpeg) |

See this blog post for more information: [Home Assistant Solar Diverter](./blog/2024-09-05_ha_diverter.md)

## Selecting your Hardware

- [ESP32 Boards](#esp32-boards)
- [Dimmers: RobotDyn, Solid State Relay or Voltage Regulator ?](#dimmers-robotdyn-solid-state-relay-or-voltage-regulator-)
- [Relays: Solid State Relay or Electromagnetic Relay ?](#relays-solid-state-relay-or-electromagnetic-relay-)
- [How to choose a Solid State Relay ?](#how-to-choose-a-solid-state-relay-)

### ESP32 Boards

Here are the boards for which firmwares can be downloaded.
If yours in not listed, just ask: it may be possible to add it.

| **Board**                                                                                                                                                  | **UARTs** | **Ethernet** |
| :--------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------: | :----------: |
| [Denky D4](https://github.com/hallard/Denky-D4)                                                                                                            |     3     |              |
| [ESP-32S](https://www.es.co.th/Schemetic/PDF/ESP32.PDF)                                                                                                    |     2     |              |
| [ESP32-DevKitC](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html) (recommended - 3 UARTs)           |     3     |              |
| [ESP32-S3-DevKitC-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html) (recommended - 3 UARTs) |     3     |              |
| [Olimex ESP32-GATEWAY](https://www.olimex.com/Products/IoT/ESP32/ESP32-GATEWAY/open-source-hardware)                                                       |     3     |      ✅      |
| [Olimex ESP32-POE](https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/open-source-hardware)                                                               |     3     |      ✅      |
| [T-ETH-Lite ESP32 S3](https://www.lilygo.cc/products/t-eth-lite?variant=43120880779445) (recommended - 3 UARTs)                                            |     3     |      ✅      |
| [TinyPICO](https://www.tinypico.com)                                                                                                                       |     3     |              |
| [Waveshare ESP32-S3 ETH Board](https://www.waveshare.com/wiki/ESP32-S3-ETH)                                                                                |     3     |      ✅      |
| [WIPI 3](https://docs.pycom.io/datasheets/development/wipy3/)                                                                                              |     3     |              |
| [WT32-ETH01](https://en.wireless-tag.com/product-item-2.html)                                                                                              |     2     |      ✅      |

### Dimmers: RobotDyn, Solid State Relay or Voltage Regulator ?

Here are some pros and cons of each phase control system:

**RobotDyn (TRIAC):**

- Pros:
  - cheap and easy to wire
  - 40A model comes with a heat sink and fan
  - All in one device: phase control, ZCD, heat sink, fan
- Cons:
  - limited in load to 1/3 - 1/2 of the announced load
  - 16A / 24A models comes with heat sink which is too small for its supported maximum load
  - no solution ready to attach them on a DIN rail.
  - The heat sink often has to be upgraded, except for the one on the 40 model which is already good for small loads below 2000W.
  - The ZCD circuit [is less accurate](https://github.com/fabianoriccardi/dimmable-light/wiki/About-dimmer-boards) and pulses can be harder to detect [on some boards](https://github.com/fabianoriccardi/dimmable-light/wiki/Notes-about-specific-architectures#interrupt-issue)
  - You need to go over some modifications to ([improve wiring / soldering and heat sink](https://sites.google.com/view/le-professolaire/routeur-professolaire))
  - You might need to replace the Triac or move it

**Solid State Relays:**

- Pros:
  - cheap and easy to wire
  - support higher loads
  - can be attached to a DIN rail with standard SSR clips
  - lot of heat sink models available
- Cons:
  - limited in load to 1/3 - 1/2 of the announced load
  - require an external ZCD module, heat sink and/or fan

**Voltage Regulators:**

Voltage regulators include a ZCD module and a phase control system which can be controlled in many ways.
These are the best option: they are big and robust.
But they require an additional module to control them with an ESP32.

This is the option and also what is used in the [Shelly Solar Router](./blog/2024-07-01_shelly_solar_diverter).

**Heat Sink:**

In any case, do not forget to correctly dissipate the heat of your Triac / SSR using an appropriate heat sink.
RobotDyn heat sink is not enough and require some tweaking (like adding a flan or de-soldering the triac and heat sink and put the triac on a bigger heat sink).

It is best to take a vertical heat sink for heat dissipation.
In case of the RobotDyn 40A, you can install it vertically.

### Relays: Solid State Relay or Electromagnetic Relay ?

**For bypass relays used for outputs**

- For bypass relays, you can use electromagnetic relays because they will be used less frequently.
  Also, some electromagnetic relays boards have both a NO and NC output to better isolate the dimming circuit and bypass circuit.

**For external Relays**

- If you want to use the relays to automatically switch one of the resistance of the water tank, as described in the [recommendations to reduce harmonics and flickering](./overview#recommendations-to-reduce-harmonics-and-flickering), you must use a SSR because the relay will be switched on and off frequently.
- If you are not using the automatic relay switching, and you either control them manually or through a Home Automation system, you can use electromagnetic relays, providing the relays won't be switched on and off frequently.
- Use a Zero-Cross SSR for resistive loads
- Use a Random SSR for inductive loads (pump, motors)

**Also to consider:**

- You should not use electromagnetic relays to switch a load on and off frequently because they have a limited number of cycles before they fail and they can be stuck.
- Relays have to be controllable through a 3.3V DC signal.
- It is easier to find SSR supporting high loads that can be controlled by a 3.3V DC signal than electromagnetic relays.
- Also, SSR with a DIN Rail clip are easy to install.
- On the other hand, SSR can be more affected by harmonics than electromagnetic relays and they are more expensive.

### How to choose a Solid State Relay ?

- Make sure you add a **heat sink** to the SSR or pick one with a heat sink, especially if you use a Random SSR instead of a RobotDyn
- **Type of control**: DA: (DC Control AC)
- **Control voltage**: 3.3V should be in the range (example: 3-32V DC)
- Verify that the **output AC voltage** is in the range (example: 24-480V AC)
- Verify the **SSR amperage**: usually, it should be 2-3 times the nominal current of your resistive load (example: 40A SSR for a 3000W resistance).
  For induction loads, it should be 4-7 times the nominal current.
- **Zero Cross SSR** (which is the default for most SSR): for the bypass relay or external relays with resistive loads, or when using Cycle Stealing modulation routing algorithm
- **Random SSR**: if you chose to not use the RobotDyn but a Random SSR for Phase Control, or external relays with inductive loads (pump, motors)

## Where to buy ?

Here is the non exhaustive list where to find some hardware to build your router.
Links are provided for reference only, you can find them on other websites.

- [ESP32 Boards](#esp32-boards-1)
- [RobotDyn](#robotdyn)
- [Random and Zero-Cross SSR](#random-and-zero-cross-ssr)
- [Zero-Cross Detection Modules](#zero-cross-detection-modules)
- [Voltage Regulators](#voltage-regulators)
- [Electromagnetic Relay](#electromagnetic-relay)
- [Measurement Devices](#measurement-devices)
- [Temperature Sensors, LEDs, Buttons, Displays](#temperature-sensors-leds-buttons-displays)
- [Mounting Accessories](#mounting-accessories)

### ESP32 Boards

|                                                                              |                                                                                                                                                           |
| :--------------------------------------------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/ESP32_NodeMCU.jpeg" style="width:150px">     | ESP32-DevKitC ([ESPRESSIF Official Store](https://fr.aliexpress.com/item/1005004441541467.html)) - Any version will work: 32 NodeMCU, 32s, 32e, 32ue, etc |
| <img src="./assets/img/hardware/ESP32-S3.jpeg" style="width:150px">          | ESP32-S3-DevKitC-1/1U ([ESPRESSIF Official Store](https://fr.aliexpress.com/item/1005003979778978.html))                                                  |
| <img src="./assets/img/hardware/LILYGO-T-ETH-Lite.jpeg" style="width:150px"> | [LILYGO T-ETH-Lite ESP32-S3](https://www.lilygo.cc/products/t-eth-lite) (Ethernet)                                                                        |
| <img src="./assets/img/hardware/WT32-ETH01.jpeg" style="width:150px">        | [WT32-ETH01](https://fr.aliexpress.com/item/1005004436473683.html) v1.4 (Ethernet)                                                                        |
| <img src="./assets/img/hardware/Pigtail_Antenna.jpeg" style="width:150px">   | [WiFi Pigtail Antenna](https://fr.aliexpress.com/item/32957527411.html) for ESP32 boards supporting external WiFi antenna                                 |

### RobotDyn

|                                                                        |                                                                                                                                                                                                           |
| :--------------------------------------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/RobotDyn_24A.jpeg" style="width:150px"> | [RobotDyn AC Dimmer 24A/600V](https://www.rbdimmer.com/fr/) Includes ZCD, supports **Phase Control** and **Cycle Stealing**                                                           |
| <img src="./assets/img/hardware/RobotDyn_40A.jpeg" style="width:150px"> | [RobotDyn AC Dimmer 40A/800V](https://www.rbdimmer.com/fr/) Includes ZCD, supports **Phase Control** and **Cycle Stealing**                                                            |
| <img src="./assets/img/hardware/BTA40-800B.jpeg" style="width:150px">  | Triac BTA40-800B RD91 [here](https://fr.aliexpress.com/item/32892486601.html) or [here](https://fr.aliexpress.com/item/1005001762265497.html) if you want / need to replace the Triac inside your RobotDyn |
| <img src="./assets/img/hardware/Heat_Sink.jpeg" style="width:150px">   | [Heat Sink for Random SSR and Triac](https://fr.aliexpress.com/item/1005004879389236.html) (there are many more types available: take a big heat sink placed vertically)                                  |

> ##### IMPORTANT
>
> 1. It is possible to switch the TRIAC of an original RobotDyn AC Dimmer with a higher one, for example a [BTA40-800B BOITIER RD-91](https://fr.farnell.com/stmicroelectronics/bta40-800b/triac-40a-800v-boitier-rd-91/dp/9801731)<br/>
>    Ref: [Triacs gradateurs pour routeur photovoltaïque](https://f1atb.fr/fr/triac-gradateur-pour-routeur-photovoltaique/).
>
> 2. The heat sink must be chosen according to the SSR / Triac. Here is a good video about the theory: [Calcul du dissipateur pour le triac d'un routeur](https://www.youtube.com/watch?v=_zAx1Q2IvJ8) (from Pierre)
>
> 3. Make sure to [improve the RobotDyn wiring/soldering](https://sites.google.com/view/le-professolaire/routeur-professolaire)
>
{: .block-important }

### Random and Zero-Cross SSR

| **Random and Zero-Cross SSR**                                           |                                                                                                                                                                                                                                                                                                                                          |
| :---------------------------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/Random_SSR.jpeg" style="width:150px">   | [LCTC Random Solid State Relay (SSR) that can be controlled by a 3.3V DC signal](https://www.aliexpress.com/item/1005004084038828.html), ([Other LCTC vendor link](https://fr.aliexpress.com/item/1005004863817921.html)). Supports **Phase Control** and **Cycle Stealing**, See [How to choose a Solid State Relay ?](#how-to-choose-a-solid-state-relay-) below |
| <img src="./assets/img/hardware/SSR_40A_DA.jpeg" style="width:150px">   | [Zero-Cross Solid State Relay (SSR) that can be controlled by a 3.3V DC signal](https://fr.aliexpress.com/item/1005003216482476.html) Supports **Cycle Stealing**, See [How to choose a Solid State Relay ?](#how-to-choose-a-solid-state-relay-) below                                                                                                            |
| <img src="./assets/img/hardware/SSR_Heat_Sink.png" style="width:150px"> | [Heat Sink for SSR](https://fr.aliexpress.com/item/32739226601.html) (there are many more types available: take a big heat sink placed vertically)                                                                                                                                                                                       |

### Zero-Cross Detection Modules

Please note that the **JSY-MK-194G has an integrated ZCD** and can be used with a Random SSR directly without the need of an external ZCD module.

| **Zero-Cross Detection**                                                 | Very good and dedicated ZCD modules - Required to use a SSR                                                                                                                                                       |
| :----------------------------------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/ZCD.jpeg" style="width:150px">           | [ZCD module from Daniel S.](https://www.pcbway.com/project/shareproject/Zero_Cross_Detector_a707a878.html) and its [DIN Rail support](https://fr.aliexpress.com/item/4000272944733.html)                            |
| <img src="./assets/img/hardware/ZCD_DIN_Rail.jpeg" style="width:150px">  | [UPM-01 DIN Rail Mount for PCB 72mm x 20mm](https://fr.aliexpress.com/item/4000272944733.html) for the ZCD module above to mount on DIN Rail. [Alternative link](https://fr.aliexpress.com/item/32276247838.html) |

I often have some spare ZCD modules since I do some batch ordering on PCBWay.
If you are interested in one, please have a look at the availabilities in the [Pro](pro#zero-cross-detection-modules) page.

Also, **do not forget that ZCD circuits are already included in RobotDyn (or other brands) dimmer boards!** So a good idea is to use such board but only for the ZCd module.
For example, on AliExpress, you can buy a cheap RobotDyn dimmer board for 3 euros with an included ZCD circuit (https://fr.aliexpress.com/item/32802025086.html).
This ZCD circuit won't be as good as a specialized one, but it can be enough for most cases.

> ##### TIP
>
> If you have access to a 3D printer, you can also [print the DIN Rail mounts](https://www.thingiverse.com/thing:4185585).
>
{: .block-tip }

**Other SSR:**

- [Zero-Cross SSR DA](https://fr.aliexpress.com/item/1005002297502716.html)
- [Zero-Cross SSR DA + Heat Sink + Din Rail Clip](https://www.aliexpress.com/item/1005002503185415.html) (40A, 60A, very high - can prevent closing an electric box)
- [Zero-Cross SSR 120 DA](https://www.aliexpress.com/item/1005005020709764.html) (for very high load)

### Voltage Regulators

|                                                                                            |                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| :----------------------------------------------------------------------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/LSA-H3P50YB.jpeg" style="width:150px">                     | [Loncont LSA-H3P50YB](https://fr.aliexpress.com/item/32606780994.html) (also available in 70A and more). Includes ZCD, supports **Phase Control** and **Cycle Stealing**                                                                                                                                                                                                                                                                                               |
| <img src="./assets/img/hardware/LCTC_Voltage_Regulator_220V_40A.jpeg" style="width:150px"> | [LCTC Voltage Regulator 220V / 40A](https://fr.aliexpress.com/item/1005005008018888.html) or [more models without heat sink but 60A, 80A, etc](https://fr.aliexpress.com/item/20000003997748.html) (also available in 70A and more). Includes ZCD, supports **Phase Control** and **Cycle Stealing**                                                                                                                                                                   |
| <img src="./assets/img/hardware/PWM_33_0-10.jpeg" style="width:150px">                     | [3.3V PWM Signal to 0-10V Convertor ](https://fr.aliexpress.com/item/1005004859012736.html) or [this link](https://fr.aliexpress.com/item/1005006859312414.html) or [this link](https://fr.aliexpress.com/item/1005007211285500.html) or [this link](https://fr.aliexpress.com/item/1005006822631244.html). Required to use the voltage regulators to convert the ESP32 pulse signal on 3.3V to an analogic output from 0-10V (external 12V power supply required) |
| <img src="./assets/img/hardware/DFR0971.jpg" style="width:150px">                          | [DAC: DFR0971 (based on GP8403), DFR1073 (based on GP8413), DFR1071 (based on GP8211S)](https://www.dfrobot.com/blog-13458.html) |
| <img src="./assets/img/hardware/lsa_heat_sink.jpeg" style="width:150px">                   | [Heat Sink 10-60A for Voltage Regulators](https://fr.aliexpress.com/item/1005001541419957.html) or [this link](https://fr.aliexpress.com/item/32823649189.html) or [this link](https://fr.aliexpress.com/item/1005003670939186.html) or [this link](https://fr.aliexpress.com/item/1005001541419957.html)                                                                                                                                                                                                                                                                                                                                                                |

### Electromagnetic Relay

|                                                                        |                                                                                                                                                                                                                                                                                                                                                                                        |
| :--------------------------------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/DIN_1_Relay.jpeg" style="width:150px"> | 1-Channel 5V DC / 30A Electromagnetic Relay on DIN Rail Support: [here](https://fr.aliexpress.com/item/1005004908430389.html), [here](https://fr.aliexpress.com/item/32999654399.html), [here](https://fr.aliexpress.com/item/1005005870389973.html), [here](https://fr.aliexpress.com/item/1005005883440249.html)                                                                     |
| <img src="./assets/img/hardware/DIN_2_Relay.jpeg" style="width:150px"> | 2-Channel 5V DC / 30A Dual Electromagnetic Relays on DIN Rail Support: [here](https://fr.aliexpress.com/item/1005004899369193.html), [here](https://fr.aliexpress.com/item/32999654399.html), [here](https://fr.aliexpress.com/item/1005005870389973.html), [here](https://fr.aliexpress.com/item/1005005883440249.html), [here](https://fr.aliexpress.com/item/1005001543232221.html) |

### Measurement Devices

|                                                                          |                                                                                                                                                                                                                                                                                                                                                                                               |
| :----------------------------------------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> | [JSY-MK-163T with a clamp](https://www.aliexpress.com/item/1005006223854694.html) Used to measure the grid power only.                                                                                                                                                                                                                                                                        |
| <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> | [JSY-MK-194T or JSY-MK-194G with 2 clamps](https://fr.aliexpress.com/item/1005007371816324.html) Used to measure the grid power and total routed power. There is also another version with a tore and a clamp.                                                                                                                                                                                |
| <img src="./assets/img/hardware/JSY-MK-194T_2.jpeg" style="width:150px"> | [JSY-MK-333](https://fr.aliexpress.com/item/1005007912611177.html) Used to measure the grid power of a 3-phase installation                                                                                                                                                                                |
| <img src="./assets/img/hardware/PZEM-004T.jpeg" style="width:150px">     | Peacefair PZEM-004T V3 100A Openable (with clamp) [official store](https://peacefair.aliexpress.com/store/1773456), [v3.0 Open CT + USB 100A](https://fr.aliexpress.com/item/33045826345.html), [v4.0 Open CT + USB 100A](https://fr.aliexpress.com/item/1005009611591855.html): Can be used to measure each output individually and more precisely. Several PZEM-004T can be connected to the same Serial port. -                                         |
| <img src="./assets/img/hardware/Shelly_EM.png" style="width:150px">      | [Shelly EM](https://www.shelly.com/en-fr/products/product-overview/shelly-em-120a/shelly-em-2x-50a) (or any other alternative sending data to MQTT)                                                                                                                                                                                                                                           |

### Temperature Sensors, LEDs, Buttons, Displays

| **Temperature Sensors, LEDs, Buttons**                                |                                                                                                                                                                                     |
| :-------------------------------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/DS18B20.jpeg" style="width:150px">    | [DS18B20 Temperature Sensor + Adapter](https://fr.aliexpress.com/item/4000143479592.html) (easier to use to install in the water tank - take a long cable)                          |
| <img src="./assets/img/hardware/PushButton.jpeg" style="width:150px"> | Push Buttons [Amazon](https://www.amazon.fr/dp/B0C2Y46BK6) (16mm) [AliExpress](https://fr.aliexpress.com/item/4001081212556.html) (12mm) for restart, manual bypass, reset          |
| <img src="./assets/img/hardware/LEDs.jpeg" style="width:150px">       | Traffic lights Lights module for system status [AZ-Delivery](https://www.az-delivery.de/en/products/led-ampel-modul), [AliExpress](https://fr.aliexpress.com/item/32957515484.html) |

| **Screens**                                                        |                                                                                                 |
| :----------------------------------------------------------------- | :---------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/SSD1306.jpeg" style="width:150px"> | [SSD1306 OLED Display 4 pins 128x64 I2C](https://www.aliexpress.com/item/32638662748.html)      |
| <img src="./assets/img/hardware/SH1106.jpeg" style="width:150px">  | [SH1106 OLED Display 4 pins 128x64 I2C](https://www.aliexpress.com/item/1005001621782442.html)  |
| <img src="./assets/img/hardware/SH1107.jpeg" style="width:150px">  | [SSD1307 OLED Display 4 pins 128x64 I2C](https://www.aliexpress.com/item/1005003297480376.html) |

### Mounting Accessories

|                                                                                  |                                                                                                                                                                                                                                                                                                                                                                                                                            |
| :------------------------------------------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| <img src="./assets/img/hardware/Electric_Box.jpeg" style="width:150px">          | [Electric Box](https://www.amazon.fr/gp/product/B0BWFGVV4S)                                                                                                                                                                                                                                                                                                                                                                |
| <img src="./assets/img/hardware/Extension_Board.jpeg" style="width:150px">       | [Extension boards](https://www.amazon.fr/dp/B0BCWBW4SR) (pay attention to the distance between header, there are different models. This one fits the ESP32 NodeMCU above)                                                                                                                                                                                                                                                  |
| <img src="./assets/img/hardware/ESP32S_Din_Rail_Mount.jpeg" style="width:150px"> | [DIN Rail Mount for ESP32 NodeMCU Dev Kit C](https://fr.aliexpress.com/item/1005005096107275.html)                                                                                                                                                                                                                                                                                                                         |
| <img src="./assets/img/hardware/Distrib_DIN.jpeg" style="width:150px">           | [Distribution Module](https://fr.aliexpress.com/item/1005005996836930.html) / [More choice](https://fr.aliexpress.com/item/1005006039723013.html)                                                                                                                                                                                                                                                                          |
| <img src="./assets/img/hardware/DIN_SSR_Clip.png" style="width:150px">           | [DIN Rail Clips for SSR](https://fr.aliexpress.com/item/1005004396715182.html)                                                                                                                                                                                                                                                                                                                                             |
| <img src="./assets/img/hardware/DIN_HDR-15-5.jpeg" style="width:80px">           | AC-DC 5V 2.4A DIN Adapter HDR-15-5 [Amazon](https://www.amazon.fr/Alimentation-rail-Mean-Well-HDR-15-5/dp/B06XWQSJGW), [AliExpress](https://fr.aliexpress.com/item/4000513120668.html). Can be used to power the ESP when installed in an electric box on DIN rail. Also if you need, a 12V version s available: [HDR-15-15 12V DC version](https://www.amazon.fr/Alimentation-rail-Mean-Well-HDR-15-5/dp/B07942GFTH?th=1) |
| <img src="./assets/img/hardware/jsy-enclosure.jpeg" style="width:150px">         | [3D Print enclosure for JSY-MK-194T](https://www.thingiverse.com/thing:6003867). You can screw it on an SSR DIN Rail to place your JSY on a DIN Rail in this enclosure.                                                                                                                                                                                                                                                    |
| <img src="https://mathieu.carbou.me/MycilaJSY/jsy-box-3.jpeg" style="width:150px"> | [3D Print enclosure for JSY-MK-194G](https://mathieu.carbou.me/MycilaJSY/#boxes-and-3d-models). You can screw it on an SSR DIN Rail to place your JSY on a DIN Rail in this enclosure.                                                                                                                                                                                                                                                    |
| <img src="./assets/img/hardware/DupontWire.jpeg" style="width:150px">            | [Dupont Cable Kit](https://fr.aliexpress.com/item/1699285992.html)                                                                                                                                                                                                                                                                                                                                                         |
| <img src="./assets/img/hardware/RC_Snubber.jpeg" style="width:150px">            | [100 ohms 0.1uF RC Snubber](https://www.shelly.com/fr/products/rc-snubber) (for RobotDyn AC dimmer and Random SSR: can be placed at dimmer output)                                                                                                                                                                                                                                                              |

## Default GPIO pinout per board

The hardware and GPIO pinout are heavily inspired by [Routeur solaire PV monophasé Le Profes'Solaire](https://sites.google.com/view/le-professolaire/routeur-professolaire) from Anthony.
Please read all the information there first.
He did a very great job with some videos explaining the wiring.

Most of the features can be enabled or disabled through the app and the GPIO pinout can be changed also trough the app.

Here are below the default GPIO pinout for each board.

**Tested boards:**

| **FEATURE**                       | **ESP32** | **NodeMCU-32S** | **esp32s3** | **wt32_eth01** | **T-ETH-Lite** | **esp32-poe** | **esp32-gateway** | **wipy3**         | **tinypico**      | **denky_d4**      |
| :-------------------------------- | :-------: | :-------------: | :---------: | :------------: | :------------: | :-----------: | :---------------: | :---------------: | :---------------: | :---------------: |
| I2C SCL (Display, DFRobot, etc)   |    22     |       22        |      9      |       32       |       40       |      -1       |      -1           |      -1           |      -1           |      -1           |
| I2C SDA (Display, DFRobot, etc)   |    21     |       21        |      8      |       33       |       41       |      -1       |      -1           |      -1           |      -1           |      -1           |
| JSY Serial2 RX                    |    16     |       16        |     16      |       5        |       18       |      35       |      16           |       4           |       4           |      22           |
| JSY Serial2 TX                    |    17     |       17        |     17      |       17       |       17       |      33       |      17           |      25           |      25           |      21           |
| Light Feedback (Green)            |    23     |       -1        |     -1      |       -1       |       -1       |      -1       |      -1           |      -1           |      -1           |      -1           |
| Light Feedback (Red)              |    15     |       -1        |     -1      |       -1       |       -1       |      -1       |      -1           |      -1           |      -1           |      -1           |
| Light Feedback (Yellow)           |     2     |       -1        |     -1      |       -1       |       -1       |      -1       |      -1           |      -1           |      -1           |      -1           |
| OUTPUT #1 Bypass Relay            |    32     |       32        |     40      |       12       |       20       |      -1       |      -1           |      -1           |      -1           |      -1           |
| OUTPUT #1 Dimmer                  |    25     |       25        |     37      |       2        |       19       |      -1       |      -1           |      -1           |      -1           |      -1           |
| OUTPUT #1 Temperature Sensor      |    18     |       18        |     18      |       15       |       3        |      -1       |      -1           |      -1           |      -1           |      -1           |
| OUTPUT #2 Bypass Relay            |    33     |       33        |     33      |       -1       |       15       |      -1       |      -1           |      -1           |      -1           |      -1           |
| OUTPUT #2 Dimmer                  |    26     |       26        |     36      |       -1       |       7        |      -1       |      -1           |      -1           |      -1           |      -1           |
| OUTPUT #2 Temperature Sensor      |     5     |        5        |      5      |       -1       |       16       |      -1       |      -1           |      -1           |      -1           |      -1           |
| Push Button (restart)             |    EN     |       EN        |     EN      |       EN       |       EN       |      EN       |      EN           |      EN           |      EN           |      EN           |
| RELAY #1                          |    13     |       13        |     13      |       14       |       5        |      -1       |      -1           |      -1           |      -1           |      -1           |
| RELAY #2                          |    12     |       12        |     12      |       -1       |       6        |      -1       |      -1           |      -1           |      -1           |      -1           |
| System Temperature Sensor         |     4     |        4        |      4      |       4        |       4        |      -1       |      -1           |      -1           |      -1           |      -1           |
| ZCD (RobotDyn, ZCD, JSY-194G)     |    35     |       35        |     35      |       35       |       8        |      -1       |      -1           |      -1           |      -1           |      -1           |
| PZEM-004T v3 Serial1 RX           |    14     |       14        |     14      |       -1       |       -1       |      36       |      -1           |      26           |      26           |      25           |
| PZEM-004T v3 Serial1 TX           |    27     |       27        |     11      |       -1       |       -1       |       4       |      -1           |      27           |      27           |      33           |

`-1` means not mapped, either because this is not possible (for example some boards to not have enough UART to support at the same time JSY and PZEM), or because the mapping is just not defined by default and you need to go in YaSolR GPIO section to define to the pin you want.

The website display the pinout configured, the pinout layout that is live at runtime and also displays some potential issues like duplicate pins or wrong pin configuration.

[![](./assets/img/screenshots/app-gpio.jpeg)](./assets/img/screenshots/app-gpio.jpeg)

## Wiring

> ##### IMPORTANT
>
> Make sure to power the components from the ESP32 3.3V and not directly from the 5V supply.
>
> This is especially true for the JSY, PZEM, DS18, Display, DAC, ZCD, etc.
> 
> So please don't do exactly like the schemas below where they show all components powered from the same +5V.
>
{: .block-important }

- [How to wire RobotDyn dimmer](#how-to-wire-robotdyn-dimmer)
- [How to wire Random Solid State Relay dimmer](#how-to-wire-random-solid-state-relay-dimmer)
- [How to wire PWM controlled Voltage Regulator dimmer](#how-to-wire-pwm-controlled-voltage-regulator-dimmer)
- [How to wire DAC controlled Voltage Regulator dimmer](#how-to-wire-dac-controlled-voltage-regulator-dimmer)
- [How to wire Bypass Relay](#how-to-wire-bypass-relay)
- [How to wire JSY and PZEM](#how-to-wire-jsy-and-pzem)
- [How to wire accessories](#how-to-wire-accessories)
- [How to wire Random SSR with JSY-MK-194G ZCD](#how-to-wire-random-ssr-with-jsy-mk-194g-zcd)

### How to wire RobotDyn dimmer

The diagram below shows how to wire dimmers based on RobotDyn modules.

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_robotdyn.jpeg).

[![](./assets/img/schemas/yasolr_robotdyn.jpeg)](https://app.cirkitdesigner.com/project/5a348c44-3e70-4ced-9420-75cbab118a30?view=interactive_preview)

### How to wire Random Solid State Relay dimmer

The diagram below shows how to wire dimmers based on Random Solid State Relay.

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_random_ssr.jpeg).

[![](./assets/img/schemas/yasolr_random_ssr.jpeg)](https://app.cirkitdesigner.com/project/af394fa8-dc6e-4785-b0c6-e0a7136aa614?view=interactive_preview)

### How to wire PWM controlled Voltage Regulator dimmer

The diagram below shows how to wire the PWM->Analog 0-10V module to control a LSA or LCTC voltage regulator.

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_pwm_lsa.jpeg).

[![](./assets/img/schemas/yasolr_pwm_lsa.jpeg)](https://app.cirkitdesigner.com/project/2ea2eb65-dc90-4cab-9349-3e484f2bdbce?view=interactive_preview)

### How to wire DAC controlled Voltage Regulator dimmer

The diagram below shows how to wire the DFRobot DAC to control a LSA or LCTC voltage regulator:

- LSA / LCTC Voltage Regulators + DAC GP8211S (DFR1071)
- LSA / LCTC Voltage Regulators + DAC GP8403 (DFR0971)
- LSA / LCTC Voltage Regulators + DAC GP8413 (DFR1073)

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_dac_lsa.jpeg).

[![](./assets/img/schemas/yasolr_dac_lsa.jpeg)](https://app.cirkitdesigner.com/project/0fa9e465-3e88-4420-91d8-cf95e1011474?view=interactive_preview)

### How to wire Bypass Relay

The diagram below shows how to wire the bypass relays to force heating through the relay and not using the dimmer for this output feature,

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_bypass_relay.jpeg).

[![](./assets/img/schemas/yasolr_bypass_relay.jpeg)](https://app.cirkitdesigner.com/project/b62655da-245f-42e6-a682-452b8bcbe7d5?view=interactive_preview)

### How to wire JSY and PZEM

The diagram below shows how to wire the following optional YaSolR measurement devices and where to put the clamps:

- PZE-004T v3
- JSY

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_measurements.jpeg).

[![](./assets/img/schemas/yasolr_measurements.jpeg)](https://app.cirkitdesigner.com/project/cb69313f-3fe0-441f-9db1-0ca880810543?view=interactive_preview)

### How to wire accessories

The diagram below shows how to wire the following optional YaSolR accessories:

- push button
- LEDs
- temperature sensors for router, output 1 and output 2
- external relays 1 and 2
- screen

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_accessories.jpeg).

[![](./assets/img/schemas/yasolr_accessories.jpeg)](https://app.cirkitdesigner.com/project/d937d685-6328-4653-80a8-f2ac8015baee?view=interactive_preview)

### How to wire Random SSR with JSY-MK-194G ZCD

The diagram below shows how to wire the Elegant, which is composed of a Random SSR and a JSY-MK-194G which does both the measurement and the Zero-Cross Detection.

You can click on the diagram to open the interactive mode or [download the image](./assets/img/schemas/yasolr_elegant.jpeg).

[![](./assets/img/schemas/yasolr_elegant.jpeg)](https://app.cirkitdesigner.com/project/8792d83e-10e1-49ff-875c-e6a09ed7660d?view=interactive_preview)
