---
layout: default
title: Blog
description: "Shelly Solar Diverter / Router: redirects the excess solar production to a water tank or heater"
---

_Date: 2024-07-01_

_I've put the YaSolR project in pause for a few days to work on this very cool and awesome Shelly integration..._

# Shelly Solar Diverter / Router

| [![](../assets/img/hardware/shelly_solar_diverter_poc2.jpeg)](../assets/img/hardware/shelly_solar_diverter_poc2.jpeg) | [![](../assets/img/screenshots/shelly_solar_diverter.jpeg)](../assets/img/screenshots/shelly_solar_diverter.jpeg) |

- [What is a Solar Router / Diverter ?](#what-is-a-solar-router--diverter-)
- [Shelly Solar Diverter](#shelly-solar-diverter-1)
- [Download](#download)
- [Hardware](#hardware)
- [Wiring](#wiring)
  - [Shelly Add-On + DS18B20](#shelly-add-on--ds18b20)
  - [Electric Circuit](#electric-circuit)
  - [RC Snubber](#rc-snubber)
  - [Add other dimmers](#add-other-dimmers)
- [Setup](#setup)
  - [Shelly Dimmer Setup](#shelly-dimmer-setup)
  - [Shelly Pro EM 50 Setup](#shelly-pro-em-50-setup)
- [How to use](#how-to-use)
  - [Configuration](#configuration)
  - [Several dimmers](#several-dimmers)
  - [Excess sharing amongst dimmers](#excess-sharing-amongst-dimmers)
  - [Start / Stop Automatic Divert](#start--stop-automatic-divert)
  - [Solar Diverter Status](#solar-diverter-status)
  - [PID Control and Tuning](#pid-control-and-tuning)
- [Future Improvements](#future-improvements)
- [Demos](#demos)
- [Help and Support](#help-and-support)

## What is a Solar Router / Diverter ?

A _Solar Router_ allows to redirect the solar production excess to some appliances instead of returning it to the grid.
The particularity of a solar router is that it will dim the voltage and power sent to the appliance in order to match the excess production, in contrary to a simple relay that would just switch on/off the appliance without controlling its power.

A _Solar Router_ is usually connected to the resistance of a water tank and will heat the water when there is production excess.

A solar router can also do more things, like controlling (on/off) the activation of other appliances (with the grid normal voltage and not the dimmed voltage) in case the excess reaches a threshold. For example, one could activate a pump, pool heater, etc if the excess goes above a specific amount, so that this appliance gets the priority over heating the water tank.

A router can also schedule some forced heating of the water tank to ensure the water reaches a safe temperature, and consequently bypass the dimmed voltage. This is called a bypass relay.

## Shelly Solar Diverter Features

- **Unlimited dimmers (output)**
- **PID Controller**
- **Excess sharing amongst dimmers with percentages**
- **Bypass (force heating)** and automatically turn the dimmer off
- **Plus all the power of the Shelly ecosystem (rules, schedules, automations, etc)**

This solar diverter based on Shelly devices and a Shelly script can control remotely dimmers and could even be enhanced with relays.
Shelly's being remotely controllable, such system offers a very good integration with Shelly App and Home Automation Systems like Home Assistant.

It is possible to put some rules based on temperature, time, days, etc and control everything from the Shelly App or Home Assistant.

The Shelly script, when activated, automatically adjusts the dimmers to the grid import or export (solar production excess).

## Download

- **[Shelly Solar Diverter Script](../downloads/auto_diverter_v1.js)**

## Hardware

All the components can be bought at [https://www.shelly.com/](https://www.shelly.com/), except the voltage regulator, where you can find some links [on my website](../build#voltage-regulators)

| [Shelly Pro EM - 50](https://www.shelly.com/fr/products/shop/proem-1x50a) | [Shelly Dimmer 0/1-10V PM Gen3](https://www.shelly.com/fr/products/shop/1xsd10pmgen3) | [Shelly Plus Add-On](https://www.shelly.com/fr/products/shop/shelly-plus-add-on) | [Temperature Sensor DS18B20](https://www.shelly.com/fr/products/shop/temperature-sensor-ds18B20) | Voltage Regulator<br>- [Loncont LSA-H3P50YB](https://fr.aliexpress.com/item/32606780994.html)<br>- [LCTC DTY-220V40P1](https://fr.aliexpress.com/item/1005005008018888.html) |
| :-----------------------------------------------------------------------: | :-----------------------------------------------------------------------------------: | :------------------------------------------------------------------------------: | :----------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
|             ![](../assets/img/hardware/Shelly_Pro_EM_50.jpeg)             |                  ![](../assets/img/hardware/Shelly_Dimmer-10V.jpeg)                   |                  ![](../assets/img/hardware/Shelly_Addon.jpeg)                   |                           ![](../assets/img/hardware/Shelly_DS18.jpeg)                           |                             ![](../assets/img/hardware/LSA-H3P50YB.jpeg)<br>![](../assets/img/hardware/LCTC_Voltage_Regulator_DTY-220V40P1.jpeg)                             |

Some additional hardware are required depending on the installation.
**Please select the amperage according to your needs.**

- A 2A breaker for the Shelly electric circuit
- A 16A or 20A breaker for your water tank (resistance) electric circuit
- A 25A relay or contactor for the bypass relay (to force a heating) for the water tank electric circuit
- A protection box for the Shelly

## Wiring

### Shelly Add-On + DS18B20

First the easy part: the temperature sensor and the Shelly Add-On, which has to be put behind the Shelly Dimmer.

| [![](../assets/img/hardware/Shelly_Addon_DS18.jpeg)](../assets/img/hardware/Shelly_Addon_DS18.jpeg) | [![](../assets/img/hardware/shelly_dimmer_with_addon.jpeg)](../assets/img/hardware/shelly_dimmer_with_addon.jpeg) |

### Electric Circuit

- Choose your breakers and wires according to your load
- Circuits can be split.
  For example, the Shelly EM can be inside the main electric box, and the Shelly Dimmer + Add-On can be in the water tank electric panel, while the contactor and dimmer can be placed neat the water tank.
  They communicate through the network.
- The dimmer will control the voltage regulator through the `COM` and `0-10V` ports
- The Shelly EM will control the relay or contactor through the `A2` ports.
- The B clamp around the wire going from the voltage regulator to the water tank is to measure the current going through the water tank resistance is optional and for information purposes only.
- The A clamp should be put around the main phase entering the house
- The relay / contactor is optional and is used to schedule some forced heating of the water tank to ensure the water reaches a safe temperature, and consequently bypass the dimmed voltage.
- The neutral wire going to the voltage regulator can be a small one; it is only used for the voltage and Zero-Crossing detection.
- Communication is done through WiFI: **make sure you have a good WiFi to reduce the connection time and improve the router speed.**

[![](../assets/img/schemas/Solar_Router_Diverter.jpg)](../assets/img/schemas/Solar_Router_Diverter.jpg)

### RC Snubber

If switching the contactor / relay causes the Shelly device to reboot, place a [RC Snubber](https://www.shelly.com/fr/products/shop/shelly-rc-snubber) between the A1 and A2 ports of the contactor / relay.

### Add other dimmers

If you want to control a second resistive load, it is possible to duplicate the circuit to add another dimmer and voltage regulator.

Modify the config accordingly to support many dimmers and they will be turned on/off sequentially to match the excess production.

## Setup

First make sure that your Shelly's are setup properly.

The script has to be installed inside the Shelly Pro EM 50, because this is where the measurements of the imported and exported grid power is done.
Also, this central place allows to control the 1, 2 or more dimmers remotely.

### Shelly Dimmer Setup

- Set static IP addresses
- Use the min/max settings to remap the 0-100% to match the voltage regulator. In the case of the LSA and LCTC, I found that **I had to remap to 10%-80%.**

### Shelly Pro EM 50 Setup

- Set static IP address
- Make sure to place the A clamp around the main phase entering the house in the right direction
- Add the `Shelly Solar Diverter` script to the Shelly Pro EM
- Configure the settings in the `CONFIG` object
- Start the script
- Activate `Run on startup`

## How to use

### Configuration

Edit the `CONFIG` object and pay attention to the values, especially the resistance value which should be accurate, otherwise the routing precision will be bad.

```javascript
const CONFIG = {
  // Debug mode
  DEBUG: 1,
  // Grid Power Read Interval (s)
  READ_INTERVAL_S: 1,
  PID: {
    // Reverse
    REVERSE: false,
    // Proportional Mode:
    // - "error" (proportional on error),
    // - "input" (proportional on measurement),
    // - "both" (proportional on 50% error and 50% measurement)
    P_MODE: "input",
    // Derivative Mode:
    // - "error" (derivative on error),
    // - "input" (derivative on measurement)
    D_MODE: "error",
    // Integral Correction
    // - "off" (integral sum not clamped),
    // - "clamp" (integral sum not clamped to OUT_MIN and OUT_MAX),
    // - "advanced" (advanced anti-windup algorithm)
    IC_MODE: "advanced",
    // Target Grid Power (W)
    SETPOINT: 0,
    // PID Proportional Gain
    KP: 0.3,
    // PID Integral Gain
    KI: 0.3,
    // PID Derivative Gain
    KD: 0.1,
    // Output Minimum (W)
    OUT_MIN: -10000,
    // Output Maximum (W)
    OUT_MAX: 10000,
  },
  DIMMERS: {
    "192.168.125.98": {
      // Resistance (in Ohm) of the load connecter to the dimmer + voltage regulator
      // 0 will disable the dimmer
      RESISTANCE: 24,
      // Percentage of the remaining excess power that will be assigned to this dimmer
      // The remaining percentage will be given to the next dimmers
      RESERVED_EXCESS_PERCENT: 100,
      // Set whether the Shelly EM with this script will be used to control the bypass relay to force a heating
      // When set to true, if you activate remotely the bypass to force a heating, then the script will detect it and turn the dimmer off
      BYPASS_CONTROLLED_BY_EM: true,
    },
    "192.168.125.97": {
      RESISTANCE: 0,
      RESERVED_EXCESS_PERCENT: 100,
      BYPASS_CONTROLLED_BY_EM: false,
    },
  },
};
```

### Several dimmers

The script can automatically control several dimmers.
Just add the IP address of the dimmer and the resistance of the load connected to it.

**How it works:**

If you have 2000W of excess, and 2 dimmers of 1500W each (nominal load), then the first ome will be set at 100% and will consume 1500W and the second one will consume the remaining 500W.

### Excess sharing amongst dimmers

It is possible to share the excess power amongst the dimmers.
Let's say you have 3 dimmers with this configuration:

```javascript
DIMMERS: {
  "192.168.125.93": {
    RESISTANCE: 53,
    RESERVED_EXCESS_PERCENT: 50,
  },
  "192.168.125.94": {
    RESISTANCE: 53,
    RESERVED_EXCESS_PERCENT: 25,
  },
  "192.168.125.95": {
    RESISTANCE: 53,
    RESERVED_EXCESS_PERCENT: 100,
  }
}
```

When you'll have 3000W of excess:

- the first one will take up to 50% of it (1500 W), but it will only take 1000 W because of the resistance. So 2000 W remains.
- the second one 25% of teh remaining (500 W)
- the third one will take the remaining 1000 W

### Start / Stop Automatic Divert

Once the script is uploaded and started, it will automatically manage the power sent to the resistive load according to the rules above.

You can start / stop the script manually from the interface or remotely by calling:

```
http://192.168.125.92/rpc/Script.Start?id=1
http://192.168.125.92/rpc/Script.Stop?id=1
```

- `192.168.125.92` begin the Shelly EM 50 static IP address.
- `1` being the script ID as seen in the Shelly interface

![](../assets/img/screenshots/shelly_script_id.jpeg)

### Solar Diverter Status

You can view the status of the script by going to the script `status` endpoint, which is only available when the script is running.

```
http://192.168.125.92/script/1/status
```

```json
{
  "config": {
    "DEBUG": 2,
    "READ_INTERVAL_S": 1,
    "PID": {
      "REVERSE": false,
      "P_MODE": "input",
      "D_MODE": "error",
      "IC_MODE": "advanced",
      "SETPOINT": 0,
      "KP": 0.3,
      "KI": 0.3,
      "KD": 0.1,
      "OUT_MIN": -10000,
      "OUT_MAX": 10000
    },
    "DIMMERS": {
      "192.168.125.98": {
        "RESISTANCE": 24,
        "RESERVED_EXCESS_PERCENT": 100
      },
      "192.168.125.97": {
        "RESISTANCE": 0,
        "RESERVED_EXCESS_PERCENT": 100
      }
    }
  },
  "pid": {
    "input": 0,
    "output": 0,
    "error": 0,
    "pTerm": 0,
    "iTerm": 0,
    "dTerm": 0,
    "sum": 0
  },
  "divert": {
    "lastTime": 1720281498691.629,
    "dimmers": {
      "192.168.125.98": {
        "divertPower": 0,
        "maximumPower": 2263.98374999999,
        "dutyCycle": 0,
        "powerFactor": 0,
        "dimmedVoltage": 0,
        "current": 0,
        "apparentPower": 0,
        "thdi": 0,
        "rpc": "success"
      }
    }
  }
}
```

### Automation ideas

- Stop the automatic divert when the temperature of the water tank reaches a specific value, and turn it back on when the temperature goes below a specific value.
- Schedule a force heating of the water tank based on days and hours
  - Either by turning on the bypass relay controlled by the Shelly EM
  - Or by disabling the auto-divert script, and then turning the dimmer on and set it to 100%

You may need to use Home Assistant or Jeedom depending on what you need to do because the Shelly App, at the time of writing, does not support a lot of actions.

### PID Control and Tuning

The script uses a complex PID controller that can be tuned to really obtain a very good routing precision.
The algorithm used and default parameters are the same as in the YaSolr project.
You will find a lot of information in the [YaSolR manual](/manual#pid-controller-section).

## Future Improvements

- Key Value storage to store the configuration and status of the script
- Virtual Components to expose script variables (also soon in [Home Assistant](https://github.com/home-assistant/core/pull/119932))

## Demos

Here is a demo video of the Shelly device reacting to EM measured power:

[![Shelly Solar Diverter Demo](https://img.youtube.com/vi/qDV0VZnWXWU/0.jpg)](https://www.youtube.com/watch?v=qDV0VZnWXWU "Shelly Solar Diverter Demo")

Here is a demo video where you can see the solar production and grid power of the home, and the Shelly device reacting to the excess production and setpoint defined.

[![Shelly Solar Diverter Demo](https://img.youtube.com/vi/cOfrMYWf8tY/0.jpg)](https://www.youtube.com/watch?v=cOfrMYWf8tY "Shelly Solar Diverter Demo")

Here is a PoC box I am using for my testing with all the components wired.
In this PoC on the left, I have used the LCTC voltage regulator which comes already pre-mounted on a heat sink.
On the right, with the LSA.
The Shelly Dimmer Gen 3 with the Shelly Addon are in the black enclosure, the voltage regulator on the right and the Shelly EM Pro at the top right.

| [![](../assets/img/hardware/shelly_solar_diverter_poc.jpeg)](../assets/img/hardware/shelly_solar_diverter_poc.jpeg) | [![](../assets/img/hardware/shelly_solar_diverter_poc2.jpeg)](../assets/img/hardware/shelly_solar_diverter_poc2.jpeg) |

## Help and Support

- [Forum photovoltaïque](https://forum-photovoltaique.fr/viewtopic.php?t=72838)
- [Discussions](https://github.com/mathieucarbou/YaSolR-OSS/discussions) (on GitHub)
- [Issues](https://github.com/mathieucarbou/YaSolR-OSS/issues) (on GitHub)
