---
layout: default
title: Blog
description: YaSolR Solar Router
---

_Date: 2025-02-27_

# YaS☀️lR Solar Router

As a professional developer and specialist in Arduino / ESP32 libraries related to energy, I have been working since the end of 2023 on a new kind of Open-Source routing software project, called **YaSolR** (_Yet Another Solar Router_), which I will describe to you here.

- **Website**: [https://yasolr.carbou.me](https://yasolr.carbou.me)
- **GitHub Project**: [https://github.com/mathieucarbou/YaSolR](https://github.com/mathieucarbou/YaSolR)

![](https://yasolr.carbou.me/assets/img/screenshots/app-overview.jpeg)

## Who am I?

For those who don't know me, I am very active in the Arduino / ESP32 / Home Assistant community.

**Profile**: [https://github.com/mathieucarbou](https://github.com/mathieucarbou)

I am notably the author of the libraries (all used by YaSolR):

- **[MycilaJSY](https://mathieu.carbou.me/MycilaJSY)**: library that supports JSY in TTL and RS485 on ESP32
- **[MycilaPZEM004Tv3](https://mathieu.carbou.me/MycilaPZEM004Tv3)**: library that supports PZEM-004T v3 on ESP32
- **[MycilaPulseAnalyzer](https://mathieu.carbou.me/MycilaPulseAnalyzer)**: library for analyzing circuit pulses Zero-Cross
- **[MycilaESPConnect](https://mathieu.carbou.me/MycilaESPConnect)**: network management library for ESP32 that supports Ethernet

And I am also one of the developers of the famous libraries **[AsyncTCP and ESPAsyncWebServer](https://github.com/ESP32Async)**.

And finally, I am also the **designer of these 2 other solar routers** that you will find in the [blog section of the YaSolR website](https://yasolr.carbou.me/blogs) and also [on this forum](https://forum-photovoltaique.fr/viewtopic.php?t=72838):

- **Solar router based on Shelly Dimmer Gen 3**
- **Solar router based on Home Assistant**

## Why did I make YaSolR?

I have analyzed a LOT of solar router source code, and unfortunately, most of the solutions have gaps. So not being satisfied with existing solar routing solutions (either from a functionality, accuracy, speed of reaction, or code quality, or license, or other point of view), I decided to create my own, Open-Source solution.

**The goal of YaSolR is therefore to provide a quality solar routing solution, accurate, responsive and compatible with most existing hardware and much more**, such as the LSA voltage regulator with a [DFRobot DAC](https://www.dfrobot.com/blog-13458.html)!

**YaSolR is ONLY the software**: so to use it, you must be able to assemble your own router with compatible hardware, and install the software on an ESP32.

I do not provide a pre-assembled router for security reasons, standards, after-sales support, etc.

## YaSolR Features:

- **Reactivity and speed of measurements**: YaSolR is able to make a measurement every 42 ms, and **adjusts the routing at least 3 times per second.**
- **PID**: YaSolR is the only router to use a **PID algorithm proportional to the measurements** to adjust the routing, which allows to control the corrections with more precision and avoids overshoots that cause oscillations, over-consumptions or over-excess.
- **PID Tuning**: YaSolR offers a PID tuning screen to adjust the PID parameters in real time, without recompiling the code.
- **Frequency**: YaSolR is able to operate on a frequency of **50 Hz and 60 Hz**, but also on a frequency of 51 Hz (for example on an Enedis generator).
- **Resolution**: YaSolR has a routing resolution of **12 bits**, so is able to be precise to the nearest watt with a load of more than 4000W. Routers with such precision are rare. Most routers have a precision of 100 steps, or 30W for a load of 3000W.
- **Zero-Cross Pulse Analysis**: YaSolR analyzes the pulses of the Zero-Cross circuit to detect the positions of the rising and falling edges, which allows to synchronize the triggering of the TRIAC as precisely as possible during the true zero crossing. Most ZC-based routers calculate a false trigger value from the rising edge of the pulse, which happens to be before the zero crossing.
- **Hardware**: YaSolR is the only router that does not impose hardware and offers **wide compatibility**.
- Supports the concept of **virtual grid power** which allows compatibility with a **second router or an EV charging station**
- Phase control features: **dimmer remapping** (like Shelly Dimmer), **power limit**, etc.
- **Bypass** (forced operation) according to schedule and / or temperature
- **Simplicity**: YaSolR is one of the rare routers to **directly support the LSA voltage regulator** via a DAC, without the requirement of an external voltage
- **Configurable**: YaSolR allows you to reconfigure your **GPIO** to be compatible with existing setups of the solar prof and F1ATB among others.

## Supported hardware:

### ESP32 boards

- Dev Kit boards,
- S3 Dev Kit boards
- ESP32s
- WIPI 3
- Denky D4
- Lilygo T Eth Lite S3 boards (_Ethernet support_)
- WT32-ETH01 boards (_Ethernet support_)
- Olimex ESP32-POE boards (_Ethernet support_)
- Olimex ESP32 Gateway boards (_Ethernet support_)
- etc

### Zero-Cross Circuits

- based on JSY-MK-194G (this JSY has a Zx pin to detect the Zero-Cross)
- based on BM1Z102FJ
- based on a short pulse like RobotDyn or Daniel S.'s ZCD module on PCB Way

### Measurement tools

- PZEM-004T v3
- JSY-MK-333 (three-phase)
- JSY-MK-193
- JSY-MK-194T
- JSY-MK-194G
- JSY-MK-163T
- JSY-MK-227
- JSY-MK-229
- JSY Remote by UDP
- MQTT (raw value, Shelly JSON)

### Dimmers supported in phase control with a module Zero-Cross

- RobotDyn 24/40A
- Triac + Zero-Cross detection
- SSR Random + Zero-Cross detection
- LSA or LCTC voltage regulator + PWM->Analog 0-10V converter module + Zero-Cross detection

### Supported phase control dimmers that operate via PWM (without Zero-Cross detection)

- LSA or LCTC voltage regulator + PWM->Analog 0-10V converter module

### Supported phase control dimmers that operate via DAC (without Zero-Cross detection)

- LSA or LCTC voltage regulator + DFRobot DAC GP8211S (DFR1071)
- LSA or LCTC voltage regulator + DFRobot DAC GP8403 (DFR0971)
- LSA or LCTC voltage regulator + DFRobot DAC GP8413 (DFR1073)

### Configuration examples:

The YaSolR site offers [several configuration examples](https://yasolr.carbou.me/build), but it's up to you to create your own.
For example. YaSolR is compatible with F1ATB, Prof solaire, etc. hardware.
So you just need to install and configure the GPIOs.

YaSolR is also one of the rare routers to directly support the LSA voltage regulator via a DAC, without going through a Zero-Cross module or a second power supply!
It is therefore possible to build a solar router with an ESP32, a DFRobot DAC and an LSA only!

## YaSolR Pro

**YaSolR is Open-Source and available for free.**

However, a [Pro version](https://yasolr.carbou.me/pro) is also available with a prettier and more complete graphical interface, which is based on [ESP-DASH Pro](https://espdash.pro), a commercial graphical library whose license does not allow its integration into Open-Source projects.
I therefore make this Pro version available to users who support the project via a donation, which helps to finance the development, maintenance and purchase of hardware so that the project can continue to evolve and its code remains freely accessible.
