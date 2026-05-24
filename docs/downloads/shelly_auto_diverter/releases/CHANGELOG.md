- **[Shelly Solar Diverter Script V1](../downloads/shelly_auto_diverter/releases/auto_diverter_v1.js)**: initial version

- **[Shelly Solar Diverter Script V2](../downloads/shelly_auto_diverter/releases/auto_diverter_v2.js)**: Fix: Shelly Dimmer does not accept floats anymore ([see here](https://forum-photovoltaique.fr/viewtopic.php?p=794582#p794582)), which might create a max routing inaccuracy of 1% of the nominal load

- **[Shelly Solar Diverter Script V3](../downloads/shelly_auto_diverter/releases/auto_diverter_v3.js)**: Updated PID parameters ([see here](https://forum-photovoltaique.fr/viewtopic.php?p=796194#p796194) and [here](https://yasolr.carbou.me/manual#pid) to have more info about how to tune the PID controller)

- **[Shelly Solar Diverter Script V4](../downloads/shelly_auto_diverter/releases/auto_diverter_v4.js)**: Fixed dimmer sharing feature to use Watts instead of %: using % based on PID output is wrong and will make the PID react to compensate. `EXCESS_POWER_LIMIT` can also be used to limit an output power to a specific value.
- **[Shelly Solar Diverter Script V4](../downloads/shelly_auto_diverter/releases/auto_diverter_v4.js)**: Fixed dimmer sharing feature to use Watts instead of %: using % based on PID output is wrong and will make the PID react to compensate. (Now replaced by `POWER_RATIO` and `POWER_LIMIT` in later versions.)

- **[Shelly Solar Diverter Script V5](../downloads/shelly_auto_diverter/releases/auto_diverter_v5.js)**: Introduced a LUT to more closely match the voltage and current sine wave when computing the dimmer duty cycle to apply to the LSA

- **[Shelly Solar Diverter Script V6](../downloads/shelly_auto_diverter/releases/auto_diverter_v6.js)**: Added `USE_POWER_LUT` config to switch between linear dimming and LUT based dimming. v6 includes both code from v4 and v5.

- **[Shelly Solar Diverter Script V7](../downloads/shelly_auto_diverter/releases/auto_diverter_v7.js)**: Reworked the routing script to improve the internal relay lifespan inside the Shelly dimmer. This breaking change requires that you have configured the dimmer correctly according to the manual in order to have no power sent to the resistive load at 1%. You can use `DIMMER_TURN_OFF_DELAY` to control the dimmer timeout to turn it off.

- **[Shelly Solar Diverter Script V8](../downloads/shelly_auto_diverter/releases/auto_diverter_v8.js)**: Set OUT_MIN to -1000W by default to avoid the PID to trigger routing when the grid consumption drops near 0W in the morning for example if a load is turned off.

- **[Shelly Solar Diverter Script V9](../downloads/shelly_auto_diverter/releases/auto_diverter_v9.js)**: Implemented ability to switch PID parameters when the grid power is near the setpoint. This is useful to avoid the PID to overreact when the grid power is near the setpoint. The script will switch between HIGH and LOW PID parameters when the grid power is within a certain range of the setpoint. This is controlled by the HIGH_LOW_SWITCH parameter.

- **[Shelly Solar Diverter Script V10](../downloads/shelly_auto_diverter/releases/auto_diverter_v10.js)**: Added support for MQTT as a grid source in order to read the grid power and voltage from an external source. This is useful when the script is installed on the dimmer itself and the grid power and voltage are read from MQTT. The script will subscribe to the MQTT topics defined in the configuration and will use the values to compute the power to divert. This setup does not require a Shelly EM Pro.

- **[Shelly Solar Diverter Script V11](../downloads/shelly_auto_diverter/releases/auto_diverter_v11.js)**: Add config for nominal voltage

- **[Shelly Solar Diverter Script V12](../downloads/shelly_auto_diverter/releases/auto_diverter_v12.js)**: Implement ability to use Shelly Components for almost anything in config (currently only numbers and boolean are supported), values are read each divert call
  By [@lexyan](https://gist.github.com/lexyan) ([https://gist.github.com/lexyan/d22b60a776bd0d8ebb677082113e8269#file-shelly_solar_diverter_v12-js](https://gist.github.com/lexyan/d22b60a776bd0d8ebb677082113e8269#file-shelly_solar_diverter_v12-js))

- **[Shelly Solar Diverter Script V13](../downloads/shelly_auto_diverter/releases/auto_diverter_v13.js)**: Fix PID integral term anti-windup (typo in variable name)

- **[Shelly Solar Diverter Script V14](../downloads/shelly_auto_diverter/releases/auto_diverter_v14.js)**: Code cleanup and fixed from v13 and moved dimmer related config in dimmer section

- **[Shelly Solar Diverter Script V15](../downloads/shelly_auto_diverter/releases/auto_diverter_v15.js)**: Added MIN / MAX brightness remapping + support for 3EM by [@Bormes](https://forum-photovoltaique.fr/viewtopic.php?p=840087&sid=f9f96caeac8a35eee320341a9c50b88d#p840087) + fixed teh behavior of Internal EM relay Switch used for bypass to correctly pause the dimmer when activating bypass

- **[Shelly Solar Diverter Script V16](../downloads/shelly_auto_diverter/releases/auto_diverter_v16.js)**: Added ability to use the internal relay of the Shelly Dimmer to do the bypass (full power) mode even when nothing is connected to the relay.

- **[Shelly Solar Diverter Script V17](../downloads/shelly_auto_diverter/releases/auto_diverter_v17.js)**: Introduced POWER_RATIO and POWER_LIMIT to better share the available power to divert between multiple dimmers and fixed issue with standby and full power modes impacting the power sharing

- **[Shelly Solar Diverter Script V18](../downloads/shelly_auto_diverter/releases/auto_diverter_v18.js)**:

    - Removed GRID_SOURCE.PHASES: use `GRID_SOURCE.TYPE` instead with "EM", "3EM" or "MQTT"
    - `OUT_MIN` and `OUT_MAX` moved to the `PID` section
    - New PID implementation (same algorithm used in YaSolR router): <https://mathieu.carbou.me/MycilaUtilities/pid>
    - Improved MQTT integration when using `GRID_SOURCE.TYPE = "MQTT"` (script forces a closed loop to get more frequent MQTT updates)
    - New HTTP API endpoint `/script/1/status` returning current `config`, `pid` and `divert` objects
        - `GET /script/1/status` => returns current status and configuration
        - `GET /script/1/status?debug=0|1|2` => set script debug level live
        - `GET /script/1/status?<dimmerName>=standby|auto` => set a dimmer mode to `standby` (force off) or `auto` (normal),
    - Bypass handling: the script reads a Shelly switch (config `SHELLY_SWITCH_ID`) to detect bypass activation; set a negative `SHELLY_SWITCH_ID` to disable reading the switch

- **[Shelly Solar Diverter Script V19](../downloads/shelly_auto_diverter/releases/auto_diverter_v19.js)**:

    - Reduce call count to avoid Uncaught Error: Too many calls in progress
    - Changed config for BYPASS mode and clarified its usage and operation
    - Clarified doc for GRID_SOURCE.TYPE
    - Fix MIN & MAX validation
    - Fix MIN-MAX ranges
    - Added new HTTP API endpoints:
        - `GET /script/1/status?reset=1` => resets all dimmer & PID states
        - `GET /script/1/status?setpoint=<value>` => sets PID setpoint on the fly

- **[Shelly Solar Diverter Script V20](../downloads/shelly_auto_diverter/releases/auto_diverter_v20.js)**:

    - Fix dimmer sharing with POWER_RATIO and POWER_LIMIT
    - Moved BYPASS to global config section
    - Added "api" BYPASS option
    - Removed SHELLY_SWITCH_ID (device is automatically detected now)
    - Added API: `/script/1/status?<dimmerName>=standby|auto|bypass` to set dimmer mode to standby, auto or bypass
    - Fix bug where divert was not called when grid power was 0
    - Improved doc and logging
    - Code refactoring

- **[Shelly Solar Diverter Script V21](../downloads/shelly_auto_diverter/releases/auto_diverter_v21.js)**:

    - Improved throttling function to avoid overlapping calls when execution time is longer than tick rate
    - Code cleanup
    - Added API: `/script/1/status?all=standby|auto|bypass` to set all dimmers to standby, auto or bypass

- **[Shelly Solar Diverter Script V22](../downloads/shelly_auto_diverter/releases/auto_diverter_v22.js)**:
    - Removed `PID.INTERVAL_S` config (not used anymore)
    - Fix: memory leak due to increasing call stack when using throttling
    - Fix: too many HTTP calls triggered due to not waiting for the HTTP calls to finish before starting a new one
    - Fix: incorrect support of "skipping" attribute when skipping dimmer calls

- **[Shelly Solar Diverter Script V23](../downloads/shelly_auto_diverter/releases/auto_diverter_v23.js)**:
    - Put back the same PID logic than in v17 with a threshold to switch between HIGH and LOW PID parameters
    - Removed `D_MODE` parameter because both input and error modes lead to the same result for the derivative PID factor
    - Simplified `callDimmers` function to add a state machine to avoid a stack overflow with several dimmers

- **[Shelly Solar Diverter Script V24](../downloads/shelly_auto_diverter/releases/auto_diverter_v24.js)** ([No LUT version](../downloads/shelly_auto_diverter/releases/auto_diverter_v24-no-lut.js)):
    - Reduced stack memory used (less variables stored in call stacks)

- **[Shelly Solar Diverter Script V25](../downloads/shelly_auto_diverter/releases/auto_diverter_v25.js)**:
    - Completely removed the callback recursion mechanism which was used to call dimmers one by one
    - Now calling all dimmers in parallel
    - Waiting globally for all dimmers to be processed before starting a new divert cycle (Measurement, PID, Divert)

- **[Shelly Solar Diverter Script V26](../downloads/shelly_auto_diverter/releases/auto_diverter_v26.js)**:
    - Fixed PID proportional term calculation (clamping in input mode)
    - Reset PID terms when no power is routed and there is some excess power to avoid integral windup
