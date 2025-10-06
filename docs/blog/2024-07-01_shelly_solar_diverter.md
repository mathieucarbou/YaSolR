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
- [Shelly Solar Diverter](#shelly-solar-diverter-features)
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
  - [Creating a Auto Divert Virtual Switch to control the script remotely](#creating-a-auto-divert-virtual-switch-to-control-the-script-remotely)
  - [Solar Diverter Status](#solar-diverter-status)
  - [PID Control and Tuning](#pid-control-and-tuning)
  - [MQTT Grid source](#mqtt-grid-source)
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
- **Excess sharing amongst dimmers**
- **Bypass (force heating)** and automatically turn the dimmer off
- **Plus all the power of the Shelly ecosystem (rules, schedules, automations, etc)**

This solar diverter based on Shelly devices and a Shelly script can control remotely dimmers and could even be enhanced with relays.
Shelly's being remotely controllable, such system offers a very good integration with Shelly App and Home Automation Systems like Home Assistant.

It is possible to put some rules based on temperature, time, days, etc and control everything from the Shelly App or Home Assistant.

The Shelly script, when activated, automatically adjusts the dimmers to the grid import or export (solar production excess).

## Download

- **[Shelly Solar Diverter Script V1](../downloads/auto_diverter_v1.js)**: initial version

- **[Shelly Solar Diverter Script V2](../downloads/auto_diverter_v2.js)**: Fix: Shelly Dimmer does not accept floats anymore ([see here](https://forum-photovoltaique.fr/viewtopic.php?p=794582#p794582)), which might create a max routing inaccuracy of 1% of the nominal load

- **[Shelly Solar Diverter Script V3](../downloads/auto_diverter_v3.js)**: Updated PID parameters ([see here](https://forum-photovoltaique.fr/viewtopic.php?p=796194#p796194) and [here](https://yasolr.carbou.me/manual#pid) to have more info about how to tune the PID controller)

- **[Shelly Solar Diverter Script V4](../downloads/auto_diverter_v4.js)**: Fixed dimmer sharing feature to use Watts instead of %: using % based on PID output is wrong and will make the PID react to compensate. `EXCESS_POWER_LIMIT` can also be used to limit an output power to a specific value.

- **[Shelly Solar Diverter Script V5](../downloads/auto_diverter_v5.js)**: Introduced a LUT to more closely match the voltage and current sine wave when computing the dimmer duty cycle to apply to the LSA

- **[Shelly Solar Diverter Script V6](../downloads/auto_diverter_v6.js)**: Added `USE_POWER_LUT` config to switch between linear dimming and LUT based dimming. v6 includes both code from v4 and v5.

- **[Shelly Solar Diverter Script V7](../downloads/auto_diverter_v7.js)**: Reworked the routing script to improve the internal relay lifespan inside the Shelly dimmer. This breaking change requires that you have configured the dimmer correctly according to the manual in order to have no power sent to the resistive load at 1%. You can use `DIMMER_TURN_OFF_DELAY` to control the dimmer timeout to turn it off.

- **[Shelly Solar Diverter Script V8](../downloads/auto_diverter_v8.js)**: Set OUT_MIN to -1000W by default to avoid the PID to trigger routing when the grid consumption drops near 0W in the morning for example if a load is turned off.

- **[Shelly Solar Diverter Script V9](../downloads/auto_diverter_v9.js)**: Implemented ability to switch PID parameters when the grid power is near the setpoint. This is useful to avoid the PID to overreact when the grid power is near the setpoint. The script will switch between HIGH and LOW PID parameters when the grid power is within a certain range of the setpoint. This is controlled by the HIGH_LOW_SWITCH parameter.

- **[Shelly Solar Diverter Script V10](../downloads/auto_diverter_v10.js)**: Added support for MQTT as a grid source in order to read the grid power and voltage from an external source. This is useful when the script is installed on the dimmer itself and the grid power and voltage are read from MQTT. The script will subscribe to the MQTT topics defined in the configuration and will use the values to compute the power to divert. This setup does not require a Shelly EM Pro.

- **[Shelly Solar Diverter Script V11](../downloads/auto_diverter_v11.js)**: Add config for nominal voltage

- **[Shelly Solar Diverter Script V12](../downloads/auto_diverter_v12.js)**: Implement ability to use Shelly Components for almost anything in config (currently only numbers and boolean are supported), values are read each divert call
  By [@lexyan](https://gist.github.com/lexyan) ([https://gist.github.com/lexyan/d22b60a776bd0d8ebb677082113e8269#file-shelly_solar_diverter_v12-js](https://gist.github.com/lexyan/d22b60a776bd0d8ebb677082113e8269#file-shelly_solar_diverter_v12-js))

- **[Shelly Solar Diverter Script V13](../downloads/auto_diverter_v13.js)**: Fix PID integral term anti-windup (typo in variable name)

- **[Shelly Solar Diverter Script V14](../downloads/auto_diverter_v14.js)**: Code cleanup and fixed from v13 and moved dimmer related config in dimmer section

- **[Shelly Solar Diverter Script V15](../downloads/auto_diverter_v15.js)**: Added MIN / MAX brightness remapping + support for 3EM by [@Bormes](https://forum-photovoltaique.fr/viewtopic.php?p=840087&sid=f9f96caeac8a35eee320341a9c50b88d#p840087) + fixed teh behavior of Internal EM relay Switch used for bypass to correctly pause the dimmer when activating bypass

- **[Shelly Solar Diverter Script V16](../downloads/auto_diverter_v16.js)**: Added ability to use the internal relay of the Shelly Dimmer to do the bypass (full power) mode even when nothing is connected to the relay.

## Hardware

All the components can be bought at [https://www.shelly.com/](https://www.shelly.com/), except the voltage regulator, where you can find some links [on my website](../build#voltage-regulators)

| [Shelly Pro EM - 50](https://www.shelly.com/fr/products/shop/proem-1x50a) (optional) | [Shelly Dimmer 0/1-10V PM Gen3](https://www.shelly.com/products/shelly-0-1-10v-dimmer-pm-gen3) | [Shelly Plus Add-On](https://www.shelly.com/fr/products/shop/shelly-plus-add-on) (optional) | [Temperature Sensor DS18B20](https://www.shelly.com/fr/products/shop/temperature-sensor-ds18B20) (optional) | Voltage Regulator<br>- [Loncont LSA-H3P50YB](https://fr.aliexpress.com/item/32606780994.html)<br>- [LCTC DTY-220V40P1](https://fr.aliexpress.com/item/1005005008018888.html) |
| :----------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
|                  ![](../assets/img/hardware/Shelly_Pro_EM_50.jpeg)                   |                       ![](../assets/img/hardware/Shelly_Dimmer-10V.jpeg)                       |                        ![](../assets/img/hardware/Shelly_Addon.jpeg)                        |                                ![](../assets/img/hardware/Shelly_DS18.jpeg)                                 |                             ![](../assets/img/hardware/LSA-H3P50YB.jpeg)<br>![](../assets/img/hardware/LCTC_Voltage_Regulator_DTY-220V40P1.jpeg)                             |

Some additional hardware are required depending on the installation.
**Please select the amperage according to your needs.**

- A 2A breaker for the Shelly electric circuit
- A 16A or 20A breaker for your water tank (resistance) electric circuit
- A 25A relay or contactor for the bypass relay (to force a heating) for the water tank electric circuit
- A protection box for the Shelly

**IMPORTANT** / **WARNING**

The Shelly dimmer should be a `0/1-10V` and the voltage must be `Current sourcing`, NOT `Current sinking`.
This is very important!

Shelly also sells the [Shelly Pro Dimmer 0/1-10V PM](https://www.shelly.com/products/shelly-pro-dimmer-0-1-10v-pm) on DIN rail which is of type current sourcing 0/1-10V.

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
- Set **Operation mode** to 1-10V
- Set **Min brightness** on toggle" to 0
- Use **Min/max brightness** to remap the 0-100% to match the voltage regulator. In the case of the LSA and LCTC, I found that **I had to remap to 7% - 85%.**

**IMPORTANT**

After calibration, you need to test the dimmer.

- At 0% and 1%: you should see 0W sent to the resistance
- At 2% and more, you should start seeing some watts sent to the resistance

**This is very important to make sure that no load goes to the resistance at 1%!**

### Shelly Pro EM 50 Setup

_Shelly Pro EM 50 is optional: the script can be installed directly on the Shelly dimmer and use MQTT to read the power and voltage._

- Set static IP address
- Make sure to place the A clamp around the main phase entering the house in the right direction
- Add the `Shelly Solar Diverter` script to the Shelly Pro EM
- Configure the settings in the `CONFIG` object
- Start the script
- Activate `Run on startup`

**The second clamp measuring the router output is optional! You can use it to measure the solar power if you want, it is not used by the script.**

## How to use

### Configuration

Edit the `CONFIG` object and pay attention to the values, especially the resistance value which should be accurate, otherwise the routing precision will be bad.

```javascript
const CONFIG = {
  // Debug mode
  DEBUG: 0,
  // Configure the sources for the grid power and voltage.
  // By default, the script will use a Shelly EM to read the grid power and voltage.
  // But it can be installed directly on the dimmer and read the power and voltage from MQTT.
  GRID_SOURCE: {
    TYPE: "MQTT", // "EM" or "MQTT"

    // Grid Read Interval (s) for power and voltage (only used when GRID_SOURCE.TYPE is "EM")
    EM_READ_INTERVAL_S: 1,

    // MQTT Topic for Grid Power (only used when GRID_SOURCE.TYPE is "MQTT")
    MQTT_TOPIC_GRID_POWER: "homeassistant/states/sensor/grid_power/state",
    // MQTT Topic for Grid Voltage (only used when GRID_SOURCE.TYPE is "MQTT")
    MQTT_TOPIC_GRID_VOLTAGE: "homeassistant/states/sensor/grid_voltage/state",
  },
  // grid semi-period in micro-seconds
  SEMI_PERIOD: 10000,
  // If set to true, the calculation of the dimmer duty cycle will be done based on a power matching LUT table which considers the voltage and current sine wave.
  // This should be more precise than a linear dimming.
  // You can try it and set it to false to compare the results, there is no harm in that!
  USE_POWER_LUT: true,
  // If the dimmer is running at 0-1% for this duration, it will be turned off automatically.
  // This duration should be long enough in order to not turn off the internal dimmer relay too often
  // The value is in milliseconds, the default is 5 minutes.
  DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
  // PID
  // More information for tuning:
  // - https://forum-photovoltaique.fr/viewtopic.php?p=796194#p796194
  // - https://yasolr.carbou.me/manual#pid
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
    // At which power to switch between HIGH and LOW PID parameters
    HIGH_LOW_SWITCH: 200,
    // PID Parameters to use when grid power is far away from setpoint
    HIGH: {
      // PID Proportional Gain
      // Also try with 0.1, 0.2, 0.3
      KP: 0.2,
      // PID Integral Gain
      // Also try with 0.2, 0.3, 0.4, 0.5, 0.6 but keep it higher than Kp
      KI: 0.4,
      // PID Derivative Gain = keep it low
      KD: 0.05,
      // Output Minimum (W): should be below the SETPOINT value, like 300W below.
      OUT_MIN: -1000,
      // Output Maximum (W): should be above the nominal power of the load.
      // I set below, the the available power to divert will be limited to this value globally for all dimmers.
      OUT_MAX: 5000,
    },
    // PID Parameters to use when grid power is near the setpoint
    LOW: {
      KP: 0.1,
      KI: 0.2,
      KD: 0.05,
      OUT_MIN: -1000,
      OUT_MAX: 5000,
    },
  },
  // DIMMER LIST
  DIMMERS: {
    "192.168.125.98": {
      // Resistance (in Ohm) of the load connecter to the dimmer + voltage regulator
      // 0 will disable the dimmer
      RESISTANCE: 24.37,
      // Maximum excess power to reserve to this load.
      // The remaining power will be given to the next dimmers
      EXCESS_POWER_LIMIT: 0,
      // Set whether the Shelly EM with this script will be used to control the bypass relay to force a heating
      // When set to true, if you activate remotely the bypass to force a heating, then the script will detect it and turn the dimmer off
      BYPASS_CONTROLLED_BY_EM: true,
    },
    // "192.168.125.99": {
    //   RESISTANCE: 0,
    //   EXCESS_POWER_LIMIT: 0,
    //   BYPASS_CONTROLLED_BY_EM: false
    // }
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
    EXCESS_POWER_LIMIT: 200,
  },
  "192.168.125.94": {
    RESISTANCE: 53,
    EXCESS_POWER_LIMIT: 200,
  },
  "192.168.125.95": {
    RESISTANCE: 53,
    EXCESS_POWER_LIMIT: 0,
  }
}
```

- the first one will take up to 200W
- the second one 200W (if some excess power is left)
- the third one will take the remaining power (if any left)

### Start / Stop Automatic Divert

Once the script is uploaded and started, it will automatically manage the power sent to the resistive load according to the rules above.

You can start / stop the script manually from the interface or remotely by calling:

```
http://192.168.125.92/rpc/Script.Start?id=1
http://192.168.125.92/rpc/Script.Stop?id=1
```

- `192.168.125.92` begin the Shelly EM 50 static IP address.
- `1` being the script ID as seen in the Shelly interface

[![](../assets/img/screenshots/shelly_script_id.jpeg) ](../assets/img/screenshots/shelly_script_id.jpeg)

### Creating a Auto Divert Virtual Switch to control the script remotely

It is possible to create a virtual switch called `Auto Divert` in the Shelly App to control the start / stop of the script.

- Activate the addons
- Create a new virtual switch addon

[![](../assets/img/screenshots/Shelly_Pro_EM_Addon.png) ](../assets/img/screenshots/Shelly_Pro_EM_Addon.png)

- Then add actions to this addon

[![](../assets/img/screenshots/Shelly_Pro_EM_Addon_AutoDivert.png) ](../assets/img/screenshots/Shelly_Pro_EM_Addon_AutoDivert.png)
[![](../assets/img/screenshots/Shelly_Pro_EM_Addon_Action_Details.png) ](../assets/img/screenshots/Shelly_Pro_EM_Addon_Action_Details.png)

- The actions can also be set in the actions section if needed

[![](../assets/img/screenshots/Shelly_Pro_EM_Actions.png) ](../assets/img/screenshots/Shelly_Pro_EM_Actions.png)

Once done, the virtual switch appears in green in the Shelly App and you can also see its definition in the Shelly App (the steps above can also be done from the Shelly App).

| [![](../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Bouton.png) ](../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Bouton.png) | [![](../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Definition.png) ](../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Definition.png)

If you have Home Assistant, it will automatically discover the new virtual switch and add it to the Shelly EM Device:

[![](../assets/img/screenshots/Shelly_Pro_EM_Home_Assistant.png) ](../assets/img/screenshots/Shelly_Pro_EM_Home_Assistant.png)

Here, Auto Divert can start and stop the divert script and Bypass Relay controls the dry contact relay of the EM to force a heating.

### Solar Diverter Status

You can view the status of the script by going to the script `status` endpoint, which is only available when the script is running.

```
http://192.168.125.92/script/1/status
```

```json
{
  "config": {
    "DEBUG": 1,
    "READ_INTERVAL_S": 1,
    "SEMI_PERIOD": 10000,
    "USE_POWER_LUT": true,
    "DIMMER_TURN_OFF_DELAY": 300000,
    "PID": {
      "REVERSE": false,
      "P_MODE": "input",
      "D_MODE": "error",
      "IC_MODE": "advanced",
      "SETPOINT": 0,
      "HIGH_LOW_SWITCH": 200,
      "HIGH": {
        "KP": 0.2,
        "KI": 0.4,
        "KD": 0.05,
        "OUT_MIN": -1000,
        "OUT_MAX": 5000
      },
      "LOW": {
        "KP": 0.1,
        "KI": 0.2,
        "KD": 0.05,
        "OUT_MIN": -1000,
        "OUT_MAX": 5000
      }
    },
    "DIMMERS": {
      "192.168.125.98": {
        "RESISTANCE": 24.37,
        "EXCESS_POWER_LIMIT": 0,
        "BYPASS_CONTROLLED_BY_EM": true
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
    "sum": 0,
    "mode": "LOW",
    "kp": 0.1,
    "ki": 0.2,
    "kd": 0.05,
    "out_min": -1000,
    "out_max": 5000
  },
  "divert": {
    "lastTime": 1737027370748.083,
    "dimmers": {
      "192.168.125.98": {
        "powerToDivert": 0,
        "maxPower": 2246.86089454247,
        "dutyCycle": 0,
        "firingDelay": 10000,
        "powerFactor": 0,
        "dimmedVoltage": 0,
        "current": 0,
        "apparentPower": 0,
        "thdi": 0,
        "rpc": "pending",
        "lastActivation": 0
      }
    },
    "gridVoltage": 234,
    "gridPower": 0
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
You will find a lot of information in the [YaSolR manual](/manual#pid).

The Shelly routeur having a slower feeback loop for measurements (each second for EM and could be more feom MQTT), it includes 2 PID configurations: one used when the grid excess is by default higher than 200W and another one when the excess is below. the advantage is to limit the over shoots of the PID and make it slowdown its corrections when around the setpoint.

### MQTT Grid source

The script can be directly installed on the dimmer and read the grid power and voltage from MQTT.

When doing that, some features are not available like using bypass mode. 
The script can be installed though on a Shelly supporting a Switch if you want bypass mode to work.

This setup is ideal for simple router setups with only 1 dimmer and 1 LSA.

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
- [Discussions](https://github.com/mathieucarbou/YaSolR/discussions) (on GitHub)
- [Issues](https://github.com/mathieucarbou/YaSolR/issues) (on GitHub)
