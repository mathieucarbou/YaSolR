_Date: 2024-07-01_

_I've put the YaSolR project in pause for a few days to work on this very cool and awesome Shelly integration..._

# Shelly Solar Diverter / Router

| | |
|---|---|
| [![](../../assets/img/hardware/shelly_solar_diverter_poc2.jpeg)](../../assets/img/hardware/shelly_solar_diverter_poc2.jpeg) | [![](../../assets/img/screenshots/shelly_solar_diverter.jpeg)](../../assets/img/screenshots/shelly_solar_diverter.jpeg) |

- [What is a Solar Router / Diverter ?](#what-is-a-solar-router-diverter)
- [Shelly Solar Diverter](#shelly-solar-diverter-features)
- [Download](#download)
- [Hardware](#hardware)
- [Wiring](#wiring)
    - [Shelly Add-On + DS18B20](#shelly-add-on-ds18b20)
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
    - [Start / Stop Automatic Divert](#start-stop-automatic-divert)
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

- ⚙️ **Core capabilities**

    - 🔌 Unlimited dimmers (outputs)
    - 🧭 PID controller (new algorithm — simpler and more efficient)
    - 🔁 Excess sharing amongst dimmers (proportional sharing with `POWER_RATIO`)
    - 🔥 Bypass (force heating) with automatic dimmer turn-off

- 🔧 **Per-output controls**

    - ⚡ Per-output power limiting (`POWER_LIMIT`)
    - 📊 Per-output min/max duty cycle support (MIN / MAX)
    - 🔢 Use LUT-based dimming (`USE_POWER_LUT`) for more accurate power matching

- 🌐 **Configuration & integrations**

    - 🧩 Simplified configuration (reorganized `PID` and `DIMMERS` sections)
    - 🌩️ Support for Shelly EM, 3EM and MQTT as measurement sources (`GRID_SOURCE.TYPE`)
    - 🧰 Support for Shelly virtual components (dynamic config values)

- 🔁 **Routing & responsiveness**

    - 🧠 Routing managed by the PID controller proportional to measurements
    - ⚡ Better responsiveness, especially with MQTT (closed-loop MQTT updates)

- 🛠️ **API & control**

    - 🛰️ REST endpoint to read status and change debug level (`/script/1/status?debug=...`)
    - 📴 REST endpoint to set a dimmer to `standby` or `auto` or `bypass` (e.g. `?boiler=standby`)
    - 🔄 Support for physical forced full-power via contactor or virtual forced mode (EM relay)

- 🌐 **Ecosystem**
    - 🔗 Plus all the power of the Shelly ecosystem (rules, schedules, automations, Home Assistant integration)

This solar diverter based on Shelly devices and a Shelly script can control remotely dimmers and could even be enhanced with relays.
Shelly's being remotely controllable, such system offers a very good integration with Shelly App and Home Automation Systems like Home Assistant.

It is possible to put some rules based on temperature, time, days, etc and control everything from the Shelly App or Home Assistant.

The Shelly script, when activated, automatically adjusts the dimmers to the grid import or export (solar production excess).

## Download

Latest version of the script is available here: [auto_diverter.js](../downloads/shelly_auto_diverter/auto_diverter.js)

All versions of the script are available here: [https://github.com/mathieucarbou/YaSolR/tree/main/docs/downloads/shelly_auto_diverter/](https://github.com/mathieucarbou/YaSolR/tree/main/docs/downloads/shelly_auto_diverter/)

## Hardware

All the components can be bought at [https://www.shelly.com/](https://www.shelly.com/), except the voltage regulator, where you can find some links [on my website](../build.md#voltage-regulators)

| [Shelly Pro EM - 50](https://www.shelly.com/fr/products/shop/proem-1x50a) (optional) | [Shelly Pro Dimmer 0/1-10V PM](https://www.shelly.com/fr/products/shelly-pro-dimmer-0-1-10v-pm) ou [Shelly Dimmer 0/1-10V PM Gen3](https://www.shelly.com/products/shelly-0-1-10v-dimmer-pm-gen3) | [Shelly Plus Add-On](https://www.shelly.com/fr/products/shop/shelly-plus-add-on) (optional) | [Temperature Sensor DS18B20](https://www.shelly.com/fr/products/shop/temperature-sensor-ds18B20) (optional) | Voltage Regulator<br>- [Loncont LSA-H3P50YB](https://fr.aliexpress.com/item/32606780994.html)<br>- [LCTC DTY-220V40P1](https://fr.aliexpress.com/item/1005005008018888.html) |
| :----------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------: | :-----------------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------------------: | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
|                  ![](../../assets/img/hardware/Shelly_Pro_EM_50.jpeg)                   |                                             ![](../../assets/img/hardware/Shelly_Pro_Dimmer-10V.jpeg) ![](../../assets/img/hardware/Shelly_Dimmer-10V.jpeg)                                             |                        ![](../../assets/img/hardware/Shelly_Addon.jpeg)                        |                                ![](../../assets/img/hardware/Shelly_DS18.jpeg)                                 |                             ![](../../assets/img/hardware/LSA-H3P50YB.jpeg)<br>![](../../assets/img/hardware/LCTC_Voltage_Regulator_DTY-220V40P1.jpeg)                             |

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

| | |
|---|---|
| [![](../../assets/img/hardware/Shelly_Addon_DS18.jpeg)](../../assets/img/hardware/Shelly_Addon_DS18.jpeg) | [![](../../assets/img/hardware/shelly_dimmer_with_addon.jpeg)](../../assets/img/hardware/shelly_dimmer_with_addon.jpeg) |

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

[![](../../assets/img/schemas/Solar_Router_Diverter.jpg)](../../assets/img/schemas/Solar_Router_Diverter.jpg)

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
- Use **Min/max brightness** to remap the 0-100% to match the voltage regulator. In the case of the LSA and LCTC, I found that **I had to remap to 4% - 96%.**

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

Edit the `CONFIG` object to match your installation. v18 simplified and reorganized the configuration:

- `GRID_SOURCE.TYPE`: "EM", "3EM" or "MQTT" (use `MQTT` when the script is installed directly on a dimmer and reads grid power/voltage from MQTT topics).
- `DIMMERS`: named dimmer entries (example uses `boiler`) with per-dimmer options such as `IP`, `RESISTANCE`, `POWER_RATIO`, `POWER_LIMIT`, `BYPASS_THROUGH_EM_RELAY`, `USE_POWER_LUT`, `DIMMER_TURN_OFF_DELAY`, `MIN`, `MAX`.

Example minimal default configuration for v18:

```javascript
const CONFIG = {
  // Debug mode: 0, 1 (info) or 2 (verbose)
  DEBUG: 1,

  GRID_SOURCE: {
    // Configure the API type to call to get grid power and voltage
    // EM will use Shelly EM API (EM1.GetStatus)
    // 3EM will use Shelly 3EM aggregate API (EM.GetStatus)
    // MQTT will use MQTT topics to read grid power and voltage
    TYPE: "EM", // "EM", "3EM" or "MQTT"

    // Enter your grid nominal voltage here. It will be used while waiting for the correct value from MQTT
    NOMINAL_VOLTAGE: 230,

    // MQTT Topic for Grid Power (only used when GRID_SOURCE.TYPE is "MQTT")
    MQTT_TOPIC_GRID_POWER: "homeassistant/states/sensor/grid_power/state",
    // MQTT Topic for Grid Voltage (only used when GRID_SOURCE.TYPE is "MQTT")
    MQTT_TOPIC_GRID_VOLTAGE: "homeassistant/states/sensor/grid_voltage/state",
  },

  // grid semi-period in micro-seconds
  SEMI_PERIOD: 10000,

  // Define how the bypass mode is done (== marche forcée)
  // Bypass is activated when we turn the Shelly EM switch on, and the goal is let let pass 100% power to the load.
  // - Set it to "virtual" if nothing is connected to the Shelly EM relay. The script will then turn the dimmer to 100% to do the bypass.
  // - Set it to "contactor" if you have wired the Shelly EM relay to a power contactor that will bypass the dimmer when activated. In this case, the script will turn off the dimmer when bypass is activated.
  // - Set it to "api" if you need to control the bypass of each dimmer independently through an API call to /script/1/status?dimmerName=full
  // Note: in both "virtual" and "contactor" modes, when the bypass is activated, all dimmers will be set to 100% power.
  BYPASS: "virtual",

  // PID
  // More information for tuning:
  // - https://forum-photovoltaique.fr/viewtopic.php?p=796194#p796194
  // - https://yasolr.carbou.me/manual#pid
  PID: {
    // Proportional Mode:
    // - "error" (proportional on error),
    // - "input" (proportional on measurement),
    P_MODE: "input",
    // Target Grid Power (W)
    SETPOINT: -100,
    // Output Minimum (W): should be below the SETPOINT value, like 300-400W below.
    OUT_MIN: -400,
    // Output Maximum (W): should be above the nominal power of the load, ideally your maximum possible excess power, like your solar production peak power
    // I set below, the the available power to divert will be limited to this value globally for all dimmers.
    OUT_MAX: 6000,
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
      OUT_MIN: -300,
      // Output Maximum (W): should be above the nominal power of the load, ideally your maximum possible excess power, like your solar production peak power
      // I set below, the the available power to divert will be limited to this value globally for all dimmers.
      OUT_MAX: 2000,
    },
    // PID Parameters to use when grid power is near the setpoint
    LOW: {
      KP: 0.1,         // half of HIGH.KP
      KI: 0.2,         // half of HIGH.KI
      KD: 0.05,        // same as HIGH.KD
      OUT_MIN: -300,   // same as HIGH.OUT_MIN
      OUT_MAX: 2000,   // same as HIGH.OUT_MAX
    }
  },

  // DIMMER LIST
  DIMMERS: {
    boiler: {
      IP: "192.168.123.173",
      // Resistance (in Ohm) of the load connecter to the dimmer + voltage regulator
      // 0 will disable the dimmer
      RESISTANCE: 85,
      // The ratio of the power that the PID has calculated to be diverted to the dimmers
      // For example if set to 0.5 (== 50%), then this dimmer will take 50% of the available power to divert and the remaining 50% will be given to the next dimmers
      POWER_RATIO: 1.0,
      // Maximum excess power in Watts to reserve to this load.
      // The remaining power will be given to the next dimmers
      POWER_LIMIT: 0,
      // If set to true, the calculation of the dimmer duty cycle will be done based on a power matching LUT table which considers the voltage and current sine wave.
      // This should be more precise than a linear dimming.
      // You can try it and set it to false to compare the results, there is no harm in that!
      USE_POWER_LUT: true,
      // If the dimmer is running at 0-1% for this duration, it will be turned off automatically.
      // This duration should be long enough in order to not turn off the internal dimmer relay too often
      // The value is in milliseconds, the default is 5 minutes.
      DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
      // Minimum duty cycle value to set on the dimmer when routing.
      // MIN matches the number where no power is sent to the load and when the dimmer will be turned off after DIMMER_TURN_OFF_DELAY.
      // Range: 0-100 %
      MIN: 0,
      // Maximum duty cycle value to set on the dimmer when routing.
      // Range: 0-100 %
      MAX: 100,
    },
    // pool_heater: {
    //   IP: "192.168.123.4",
    //   RESISTANCE: 85,
    //   POWER_RATIO: 1.0,
    //   POWER_LIMIT: 0,
    //   USE_POWER_LUT: true,
    //   DIMMER_TURN_OFF_DELAY: 5 * 60 * 1000,
    //   MIN: 0,
    //   MAX: 100,
    // }
  }
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
    POWER_LIMIT: 200,    // reserve up to 200 W for this dimmer
  },
  "192.168.125.94": {
    RESISTANCE: 53,
    POWER_LIMIT: 200,    // reserve up to 200 W for this dimmer
  },
  "192.168.125.95": {
    RESISTANCE: 53,
    POWER_LIMIT: 0,      // 0 means "take the remaining power"
  }
}
```

- the first one will take up to 200 W (because of `POWER_LIMIT`)
- the second one up to 200 W (if some excess power is left)
- the third one will take the remaining power (if any left)

Alternative: use `POWER_RATIO` to share proportionally. For example, two dimmers sharing equally:

```javascript
DIMMERS: {
  "heaterA": { RESISTANCE: 50, POWER_RATIO: 0.5 },
  "heaterB": { RESISTANCE: 50, POWER_RATIO: 0.5 }
}
```

Or combine `POWER_RATIO` with `POWER_LIMIT` to cap each dimmer:

```javascript
DIMMERS: {
  "heaterA": { RESISTANCE: 50, POWER_RATIO: 0.6, POWER_LIMIT: 800 },
  "heaterB": { RESISTANCE: 50, POWER_RATIO: 0.4, POWER_LIMIT: 500 }
}
```

### Start / Stop Automatic Divert

Once the script is uploaded and started, it will automatically manage the power sent to the resistive load according to the rules above.

You can start / stop the script manually from the interface or remotely by calling:

```
http://192.168.125.92/rpc/Script.Start?id=1
http://192.168.125.92/rpc/Script.Stop?id=1
```

- `192.168.125.92` begin the Shelly EM 50 static IP address.
- `1` being the script ID as seen in the Shelly interface

[![](../../assets/img/screenshots/shelly_script_id.jpeg)](../../assets/img/screenshots/shelly_script_id.jpeg)

### Creating a Auto Divert Virtual Switch to control the script remotely

It is possible to create a virtual switch called `Auto Divert` in the Shelly App to control the start / stop of the script.

- Activate the addons
- Create a new virtual switch addon

[![](../../assets/img/screenshots/Shelly_Pro_EM_Addon.png)](../../assets/img/screenshots/Shelly_Pro_EM_Addon.png)

- Then add actions to this addon

[![](../../assets/img/screenshots/Shelly_Pro_EM_Addon_AutoDivert.png)](../../assets/img/screenshots/Shelly_Pro_EM_Addon_AutoDivert.png)
[![](../../assets/img/screenshots/Shelly_Pro_EM_Addon_Action_Details.png)](../../assets/img/screenshots/Shelly_Pro_EM_Addon_Action_Details.png)

- The actions can also be set in the actions section if needed

[![](../../assets/img/screenshots/Shelly_Pro_EM_Actions.png)](../../assets/img/screenshots/Shelly_Pro_EM_Actions.png)

Once done, the virtual switch appears in green in the Shelly App and you can also see its definition in the Shelly App (the steps above can also be done from the Shelly App).

| | |
|---|---|
| [![](../../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Bouton.png)](../../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Bouton.png) | [![](../../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Definition.png)](../../assets/img/screenshots/Shelly_App_EM_Switch_Virtuel_Definition.png) |

If you have Home Assistant, it will automatically discover the new virtual switch and add it to the Shelly EM Device:

[![](../../assets/img/screenshots/Shelly_Pro_EM_Home_Assistant.png)](../../assets/img/screenshots/Shelly_Pro_EM_Home_Assistant.png)

Here, Auto Divert can start and stop the divert script and Bypass Relay controls the dry contact relay of the EM to force a heating.

### Solar Diverter Status

You can view and control some runtime parameters through the script HTTP endpoint (available while the script is running):

```
http://192.168.125.92/script/1/status
```

Examples:

- `GET /script/1/status` => returns a JSON object with `config`, `pid` and `divert` state
- `GET /script/1/status?debug=2` => sets debug level to verbose
- `GET /script/1/status?boiler=standby` => sets the dimmer named `boiler` to standby (force off)
- `GET /script/1/status?boiler=auto` => sets the dimmer named `boiler` to auto (normal operation)
- `GET /script/1/status?boiler=bypass` => sets the dimmer named `boiler` in bypass mode (100% power) - only work if `BYPASS` is set to `api`
- `GET /script/1/status?all=standby|auto|bypass` => sets all dimmers to standby, auto or bypass mode
- `GET /script/1/status?reset=1` => resets all dimmer states
- `GET /script/1/status?setpoint=<value>` => sets PID setpoint on the fly

Sample response (v18):

```json
{
  "config": {
    "DEBUG": 1,
    "GRID_SOURCE": {
      "TYPE": "MQTT",
      "NOMINAL_VOLTAGE": 230,
      "MQTT_TOPIC_GRID_POWER": "homeassistant/states/sensor/grid_power/state",
      "MQTT_TOPIC_GRID_VOLTAGE": "homeassistant/states/sensor/grid_voltage/state"
    },
    "SEMI_PERIOD": 10000,
    "PID": {
      "P_MODE": "input",
      "D_MODE": "input",
      "SETPOINT": 1500,
      "OUT_MIN": -400,
      "OUT_MAX": 6000,
      "KP": 0.1,
      "KI": 0.2,
      "KD": 0.05
    },
    "DIMMERS": {
      "boiler": {
        "IP": "192.168.123.173",
        "RESISTANCE": 85,
        "POWER_RATIO": 1,
        "POWER_LIMIT": 0,
        "USE_POWER_LUT": true,
        "DIMMER_TURN_OFF_DELAY": 300000,
        "MIN": 1,
        "MAX": 99
      }
    }
  },
  "pid": {
    "input": 1242.7,
    "output": 1225.53,
    "error": 257.3,
    "pTerm": -124.27,
    "iTerm": 1349.8,
    "dTerm": 0
  },
  "divert": {
    "gridVoltage": 237.5,
    "dimmers": {
      "boiler": {
        "powerToDivert": 0,
        "fullPower": false,
        "standby": false,
        "maxPower": 663.60294117647,
        "dutyCycle": 0,
        "firingDelay": 10000,
        "powerFactor": 0,
        "dimmedVoltage": 0,
        "current": 0,
        "apparentPower": 0,
        "thdi": 0,
        "rpc": "success",
        "lastActivation": 1760909103663.832
      }
    },
    "gridPower": 1242.7
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
You will find a lot of information in the [YaSolR manual](../manual.md#pid).

The Shelly routeur having a slower feeback loop for measurements (each second for EM and could be more feom MQTT), it includes 2 PID configurations: one used when the grid excess is by default higher than 200W and another one when the excess is below. the advantage is to limit the over shoots of the PID and make it slowdown its corrections when around the setpoint.

You can also use the [Online PID Simulator](https://mathieu.carbou.me/MycilaUtilities/simulator/).
This simulator was built for YaSolR router but the PID algorithm is the same.

[![](../../assets/img/screenshots/pid_simulator.jpeg)](../../assets/img/screenshots/pid_simulator.jpeg)

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

| | |
|---|---|
| [![](../../assets/img/hardware/shelly_solar_diverter_poc.jpeg)](../../assets/img/hardware/shelly_solar_diverter_poc.jpeg) | [![](../../assets/img/hardware/shelly_solar_diverter_poc2.jpeg)](../../assets/img/hardware/shelly_solar_diverter_poc2.jpeg) |

## Help and Support

- [Forum photovoltaïque](https://forum-photovoltaique.fr/viewtopic.php?t=72838)
- [Discussions](https://github.com/mathieucarbou/YaSolR/discussions) (on GitHub)
- [Issues](https://github.com/mathieucarbou/YaSolR/issues) (on GitHub)
