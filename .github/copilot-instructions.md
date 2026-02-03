# YaSolR - AI Coding Agent Instructions

## Project Overview

**YaSolR (Yet another Solar Router)** is an ESP32-based solar router firmware for optimizing solar energy usage. It manages AC dimmer outputs, relays, and sensors to route excess solar power to resistive loads (e.g., water heaters) with real-time monitoring and PID control.

### Core Functionality

The system continuously monitors grid power consumption and dynamically adjusts AC dimmers to consume excess solar production:

- **Grid measurement**: JSY power meter measures total home consumption (positive = grid import, negative = excess solar)
- **PID control**: Continuously adjusts dimmer duty cycles to maintain grid setpoint (typically 0W)
- **Multi-output**: Supports 2 independent outputs, each with dimmer + optional bypass relay + temperature sensor + power meter
- **Auto-bypass**: Automatically switches to full-power relay mode when conditions met (temperature, time windows, etc.)
- **Safety**: Temperature limits, timeouts, dimmer offline detection, ZCD monitoring

### Key Components

- **Outputs (1-2)**: Each output routes power to a resistive load via dimmer or bypass relay
- **Grid Monitor**: JSY (local/remote) or computed from PZEM measurements per output
- **PID Controller**: Maintains grid setpoint by adjusting dimmer duty cycles
- **Sensors**: DS18B20 temperature sensors for dimmer heatsink, outputs, and system monitoring
- **Dashboard**: Real-time web interface (ESPDash) with charts, controls, and configuration
- **MQTT/HA**: Home Assistant auto-discovery and telemetry publishing
- **Relays**: 2 additional generic relays for auxiliary loads (independent of routing)

## Build System (PlatformIO)

- **Build tool**: PlatformIO, not Arduino IDE
- **Build command**: `pio run -e <environment>` 
- **Default environments**: `pro-esp32`, `pro-esp32s3`, `pro-lilygo_eth_lite_s3`, `pro-wt32_eth01`
- **Upload**: `pio run -e <environment> -t upload`
- **Monitor**: `pio device monitor` (115200 baud, uses `esp32_exception_decoder` filter)

### Pre-build Scripts

Three critical Python scripts in `tools/` run before every build:

1. **`version.py`**: Auto-generates version from git branch/tag and commit hash
2. **`embed.py`**: Gzips web assets from `embed/` to `.pio/embed/*.gz`
3. **`cacerts.py`**: Downloads CA certificates bundle for HTTPS/MQTT TLS

### Build Models

Three distinct firmware variants controlled by `[oss]`, `[pro]`, `[trial]` sections in `platformio.ini`:

- **OSS**: Uses open-source ESP-DASH and WebSerial libraries
- **PRO**: Uses proprietary ESPDASHPro/WebSerialPro from `lib/` (define: `APP_MODEL_PRO`)
- **TRIAL**: PRO features with time-limited trial (define: `APP_MODEL_TRIAL`)

**Critical**: When editing dashboard/web code, check for `#ifdef APP_MODEL_PRO` blocks - features differ between models.

## Architecture

### Initialization Pattern

All modules follow a two-phase init pattern (see `src/main.cpp`):

1. **`yasolr_init_*()` functions**: Create objects, allocate resources, register in task manager
2. **`yasolr_configure_*()` functions**: Apply settings from config, enable/disable features based on config keys

**Order matters**: DS18 sensors init first (blocking I2C scan), then dimmers (ISR-sensitive), then PZEM/JSY (UART heavy).

### Module Organization

Each functional area has dedicated source files:

**Core Systems:**
- `yasolr_router.cpp` - Solar routing logic, dimmer/relay orchestration, calibration
- `yasolr_pid.cpp` - PID controller setup and tuning
- `yasolr_config.cpp` - NVS configuration management, defaults, validation
- `yasolr_system.cpp` - System tasks (restart, reset, safe boot), task managers

**Hardware Integration:**
- `yasolr_ds18.cpp` - Temperature sensor discovery and monitoring
- `yasolr_jsy.cpp` - Local JSY power meter (Modbus RTU over UART)
- `yasolr_jsy_remote.cpp` - Remote JSY data via UDP (for shared grid meter)
- `yasolr_pzem.cpp` - Output-level PZEM power meters
- `yasolr_relays.cpp` - Generic relay1/relay2 control
- `yasolr_lights.cpp` - RGB status LEDs (traffic light)
- `yasolr_display.cpp` - OLED display updates
- `yasolr_victron.cpp` - Victron Modbus integration

**Network & Communication:**
- `yasolr_network.cpp` - WiFi/Ethernet using MycilaESPConnect
- `yasolr_mqtt.cpp` - MQTT client, subscriptions, publishing
- `yasolr_web_server.cpp` - AsyncWebServer, dashboard, WebSerial
- `yasolr_dashboard.cpp` - Dashboard UI (cards, charts, tabs, statistics)

**Utilities:**
- `yasolr_logging.cpp` - ESP-IDF logging configuration
- `yasolr_version.cpp` - Version checking against GitHub releases
- `yasolr_trial.cpp` - Trial enforcement (TRIAL model only)
- `yasolr_grid.cpp` - Grid frequency/voltage monitoring

Global objects declared in `include/yasolr.h`, defined in respective cpp files.

**Dependency graph (initialization order matters):**
```
yasolr_logging → yasolr_system → yasolr_config
                     ↓
yasolr_ds18 (blocking I2C scan)
                     ↓
yasolr_router (creates dimmers - ISR sensitive)
                     ↓
yasolr_pzem, yasolr_jsy (UART heavy, async)
                     ↓
yasolr_network → yasolr_mqtt
                     ↓
yasolr_dashboard (requires network)
```

### The Router Core

The `Mycila::Router` class (in `lib/MycilaRouter/`) is the heart of solar routing:

- Manages two **outputs** (`output1`, `output2`) - each can be a dimmer or relay
- Each output has optional bypass relay, temperature sensor, PZEM power meter
- Applies **PID control** to match dimmer power to available solar excess
- Features: auto-bypass on temperature limit, timeout safety, excess power limiting

**Key concept**: The router continuously adjusts dimmer duty cycles to consume grid excess power (measured by JSY) without importing from grid.

#### Router::Output Class

Represents a single routing output with these components:

**State machine** (`State` enum):
- `UNUSED`: No dimmer or relay configured
- `IDLE`: Configured but not actively routing (no excess or conditions not met)
- `ROUTING`: Dimmer actively routing partial power
- `BYPASS_MANUAL`: Full power via relay (manual user action)
- `BYPASS_AUTO`: Full power via relay (auto-triggered by temp/time/power)

**Configuration** (`Output::Config` struct):
- `calibratedResistance`: Measured load resistance (Ohms) from calibration
- `autoDimmer`: Enable automatic dimming based on available power
- `dimmerTempLimit`: Max dimmer temperature before shutdown (°C)
- `autoBypass`: Enable automatic relay bypass
- `bypassTimeoutSec`: Auto-off timeout for manual bypass
- `autoStartTemperature`: Trigger bypass when output temp reaches (°C)
- `autoStopTemperature`: Disable bypass when output temp reaches (°C)
- `autoStartTime`: Daily start time for auto-bypass (HH:MM format)
- `autoStopTime`: Daily stop time for auto-bypass (HH:MM format)
- `weekDays`: Days of week for time-based bypass (comma-separated)
- `excessPowerLimiter`: Cap power sent to output (Watts)
- `excessPowerRatio`: Share of excess power [0.0-1.0] for multi-output distribution

**Core methods:**
- `autoDivert(gridVoltage, availablePower)`: Automatically adjust dimmer to consume available power
- `setBypass(state)`: Manually control bypass relay
- `applyAutoBypass()`: Check conditions and auto-enable bypass if criteria met
- `applyTemperatureLimit()`: Shut down dimmer if over temp limit
- `applyBypassTimeout()`: Auto-disable manual bypass after timeout
- `setDimmerDutyCycle(duty)`: Set dimmer duty cycle [0.0-1.0]
- `updateMetrics(metrics)`: Update power measurements from PZEM/JSY

**Metrics sources** (priority order):
1. **PZEM**: Direct measurement from output's PZEM meter (most accurate for that output)
2. **JSY/JSY_REMOTE**: Shared grid meter (aggregates all outputs)
3. **COMPUTED**: Calculated from dimmer duty cycle and calibrated resistance

#### Router::Relay Class

Auto-switching relay for high-power loads:

- `setNominalLoad(watts)`: Set connected load power rating
- `computeLoad(gridVoltage)`: Calculate actual load at current voltage
- `autoSwitch(gridVoltage, gridPower, routedPower, setpoint)`: Auto-enable if enough excess power
- `setTolerance(ratio)`: Power tolerance for hysteresis (default 5%)

The relay automatically switches on when available power exceeds nominal load (within tolerance), avoiding rapid on/off cycling.

#### Router Class (Main)

- `addOutput(output)`: Register output for routing
- `divert(gridVoltage, powerToDivert)`: Distribute excess power across all enabled outputs
- `noDivert()`: Turn off all outputs (no excess power)
- `beginCalibration(outputIndex, callback)`: Start resistance calibration for output
- `continueCalibration()`: Advance calibration state machine (called periodically)
- `updateMetrics(metrics)`: Update grid-level measurements (from JSY)
- `readMeasurements(metrics)`: Aggregate measurements from outputs or JSY
- `computeMetrics(metrics, gridVoltage)`: Calculate expected metrics from dimmer states

**Calibration state machine** (10-second process):
1. **Prepare**: Turn off all outputs, wait for stable grid
2. **50% duty**: Run dimmer at 50%, measure power
3. **100% duty**: Run dimmer at 100%, measure power
4. **Calculate**: Compute resistance from P = V²/R at both duty cycles
5. **Cleanup**: Restore previous state, save to NVS

**Power distribution algorithm:**
```cpp
float Router::divert(float gridVoltage, float powerToDivert) {
  float totalRouted = 0;
  for (auto& output : outputs) {
    // Each output consumes its share
    float share = output.autoDivert(gridVoltage, powerToDivert - totalRouted);
    totalRouted += share;
    // Remaining power goes to next output
  }
  return totalRouted;
}
```

Each output respects its `excessPowerRatio` for sharing, and `excessPowerLimiter` for capping.

## Configuration System

- **Storage**: NVS (Non-Volatile Storage) via `Mycila::config::Config`
- **Keys**: Defined in `yasolr_macros.h` as `KEY_*` constants
- **Pattern**: `config.configure(KEY_NAME, defaultValue)` then `config.get<Type>(KEY_NAME)`
- **Reconfiguration**: Changes trigger async reconfigure tasks via `reconfigureQueue`

Example: Enabling Output1 dimmer checks `config.get<bool>(KEY_ENABLE_OUTPUT1_DIMMER)`

### Configuration Architecture

**Initialization** (`yasolr_config.cpp:yasolr_init_config()`):
1. Creates NVS storage backend: `Mycila::config::ConfigStorageNVS`
2. Declares all config keys with `config.configure(KEY, defaultValue)`
3. Loads values from NVS or uses defaults if not present
4. Validates config (e.g., UART conflicts, pin assignments)

**Key types and defaults:**
- **Boolean**: `config.configure<bool>(KEY_ENABLE_DEBUG, false)`
- **Integer**: `config.configure<uint16_t>(KEY_MQTT_PORT, 1883)`
- **Float**: `config.configure<float>(KEY_PID_KP, 0.3f)`
- **String**: `config.configure<const char*>(KEY_MQTT_SERVER, "")`

**Getters:**
```cpp
bool enabled = config.get<bool>(KEY_ENABLE_OUTPUT1_DIMMER);
uint16_t port = config.get<uint16_t>(KEY_MQTT_PORT);
std::string server = config.getString(KEY_MQTT_SERVER);
```

**Setters** (triggers reconfiguration):
```cpp
config.set<bool>(KEY_ENABLE_OUTPUT1_DIMMER, true);
config.set<float>(KEY_PID_KP, 0.5f);
// Changes are persisted to NVS and trigger reconfigure callbacks
```

**Key naming convention:**
- `KEY_ENABLE_*`: Feature enable/disable flags
- `KEY_*_PIN`: GPIO pin assignments
- `KEY_*_TEMPERATURE`: Temperature thresholds
- `KEY_OUTPUT*_*`: Output-specific settings (1 or 2)
- `KEY_RELAY*_*`: Generic relay settings (1 or 2)

**Reconfiguration queue:**
When config changes via web UI or MQTT, tasks are added to `reconfigureQueue` to apply changes without blocking:
```cpp
// In yasolr_dashboard.cpp or yasolr_mqtt.cpp
config.set<bool>(KEY_ENABLE_OUTPUT1_DIMMER, true);
// Automatically triggers yasolr_configure_output1_dimmer() via queue
```

**Config validation:**
- UART conflicts: JSY and PZEM cannot share same UART
- Pin conflicts: No two features on same GPIO
- Required dependencies: e.g., dimmer needs ZCD pin for certain types
- Value ranges: e.g., temperature limits 0-100°C, duty cycle 0-100%

## Library Architecture

YaSolR uses a mix of custom in-tree libraries and external dependencies.

### In-Tree Custom Libraries (`lib/`)

#### ESPDASHPro (Proprietary - PRO model only)
- **Purpose**: Advanced web dashboard framework with Svelte-based frontend
- **Key features**: Real-time charts, cards, tabs, statistics, authentication
- **Frontend**: Built with Svelte + Vite + TypeScript in `portal/` directory
- **Backend**: C++ WebSocket server in `src/ESPDashPro.h`
- **Cards types**: Statistic, Energy, Chart (Line/Area/Bar), ToggleButton, IndicatorButton, Feedback, Slider
- **Usage pattern**: `dashboard.setTitle()`, create cards/tabs, `card.update(value)` to push data
- **Build**: Frontend pre-built and embedded in C++ header files
- **OSS alternative**: Uses ayushsharma82/ESP-DASH (feature-limited, different API)

#### WebSerialPro (Proprietary - PRO model only)  
- **Purpose**: Browser-based serial monitor/console
- **Key features**: Real-time logs, command input, persistent connection, filtering
- **Frontend**: Svelte + WebSocket in `portal/`
- **Backend**: WebSocket log streamer in `src/WebSerialPro.h`
- **Integration**: Hooks into ESP-IDF logging system to capture all `ESP_LOG*` output
- **OSS alternative**: Uses ayushsharma82/WebSerial

#### MycilaRouter
- **Purpose**: Core solar routing engine
- **Location**: `lib/MycilaRouter/MycilaRouter.h` + `MycilaRouter.cpp`
- **Key classes**:
  - `Router::Metrics`: Power measurement data (voltage, current, power, energy, PF, THDi, resistance)
  - `Router::Relay`: Auto-switching relay based on available power and nominal load
  - `Router::Output`: Represents one routing output (dimmer + optional bypass relay)
  - `Router`: Main router managing multiple outputs and calibration
  
**Router::Output states:**
- `UNUSED`: Output disabled (no dimmer/relay configured)
- `IDLE`: Enabled but not routing (no excess power or conditions not met)
- `ROUTING`: Actively routing via dimmer (partial power)
- `BYPASS_MANUAL`: Full power via relay (user-triggered)
- `BYPASS_AUTO`: Full power via relay (auto-triggered by temperature/time)

**Router workflow:**
1. Grid measurement provides available power to divert
2. Router distributes power across enabled outputs using `divert(gridVoltage, powerToDivert)`
3. Each output calculates duty cycle: `dutyCycle = powerToDivert / maxPower` where `maxPower = V²/R`
4. Outputs respect limits: excess power ratio, power limiter, temperature limits
5. Auto-bypass activates if temperature threshold reached or time window active

**Calibration process:**
- Measures actual load resistance by running dimmer at 50% and 100% duty
- Records power consumption at each level
- Calculates calibrated resistance from measurements
- Takes ~10 seconds, requires stable grid voltage
- Stored in NVS config as `KEY_OUTPUT*_RESISTANCE`

#### MycilaAppInfo
- **Purpose**: Application metadata and versioning
- **Global object**: `Mycila::AppInfo` singleton
- **Properties**: `name`, `version`, `model`, `id`, `manufacturer`, `buildHash`, `buildDate`, `latestVersion`
- **Version checking**: `isOutdated()` compares current version with latest from GitHub
- **Auto-generated**: Build scripts inject git info at compile time

#### MycilaVictron
- **Purpose**: Victron Energy device Modbus integration
- **Supported**: Solar chargers, inverters, battery monitors
- **Protocol**: Modbus RTU over UART
- **Usage**: Read solar production, battery state, etc. for advanced routing decisions
- **Status**: Optional feature, disabled by default

### External Mycila* Libraries (from PlatformIO registry)

These are maintained separately but designed to work together:

**Core Infrastructure:**
- `MycilaConfig`: NVS-backed configuration with validation, defaults, and change callbacks
- `MycilaTaskManager`: Cooperative task scheduler (alternative to FreeRTOS tasks for simple periodic work)
- `MycilaTaskMonitor`: Task performance monitoring and stack usage tracking
- `MycilaSystem`: System utilities (reboot, reset, safe boot, heap monitoring)

**Hardware Drivers:**
- `MycilaDimmer`: AC dimmer control (phase control, cycle stealing, SSR, DAC-based)
- `MycilaDS18`: Dallas DS18B20 temperature sensor wrapper with auto-search
- `MycilaJSY`: JSY-MK-194T/333 power meter communication (Modbus RTU)
- `MycilaPZEM`: PZEM-004T power meter communication (Modbus RTU)
- `MycilaRelay`: GPIO relay control with debouncing and switch counting
- `MycilaPulseAnalyzer`: Zero-crossing detection and frequency measurement

**Networking:**
- `MycilaESPConnect`: Unified WiFi/Ethernet manager with captive portal
- `MycilaMQTT`: Async MQTT client with TLS, reconnection, LWT
- `MycilaHADiscovery`: Home Assistant MQTT discovery message builder
- `MycilaNTP`: NTP time synchronization

**Utilities:**
- `MycilaGrid`: Grid frequency/voltage validation (50Hz vs 60Hz, 110V vs 230V)
- `MycilaPID`: PID controller implementation
- `MycilaUtilities`: String helpers, value clamping, conversions
- `MycilaCircularBuffer`: Fixed-size ring buffer for averaging
- `MycilaExpiringValue`: Value with expiration timestamp (auto-invalidation)
- `MycilaEasyDisplay`: OLED display abstraction (U8g2)
- `MycilaTrafficLight`: RGB LED status indicator

**Build Tools:**
- `MycilaTrial`: Time-limited trial enforcement (TRIAL model only)

### Key Design Patterns

**ExpiringValue<T>**: Used throughout for sensor/measurement caching
```cpp
ExpiringValue<float> temperature;
temperature.setExpiration(10000); // 10 seconds
temperature.update(25.5);
if (temperature.isPresent()) {
  float value = temperature.get();
  uint32_t age = temperature.getLastUpdateTime();
}
```

**Task-based execution**: Non-blocking operations via TaskManager
```cpp
Mycila::Task myTask("Name", []() {
  // task code
});
myTask.setType(Mycila::TaskType::FOREVER);
myTask.setInterval(1000); // ms
coreTaskManager.add(myTask);
```

**Config-driven initialization**: Two-phase pattern
```cpp
// Phase 1: yasolr_init_*() - create objects
dimmer = new Mycila::Dimmer(pin);

// Phase 2: yasolr_configure_*() - apply config
if (config.get<bool>(KEY_ENABLE_DIMMER)) {
  dimmer->begin();
}
```

When modifying core routing, PID, or power management, check `lib/MycilaRouter/` sources.

## Pin Configuration

Hardware pins are **compile-time constants** defined per board in `platformio.ini`:

```cpp
-D YASOLR_OUTPUT1_DIMMER_PIN=25
-D YASOLR_JSY_RX_PIN=16
-D YASOLR_ZCD_PIN=35  // Zero-crossing detection for AC dimmers
```

**Never hardcode pins** - use the `YASOLR_*_PIN` macros from build flags.

## Critical Conventions

### Macros & Constants

- **TAG**: Always `"YASOLR"` for ESP logging (`ESP_LOGI(TAG, ...)`)
- **Boolean strings**: Use `YASOLR_BOOL(x)` → "true"/"false", `YASOLR_STATE(x)` → "on"/"off"
- **Dimmer types**: Predefined string constants like `YASOLR_DIMMER_TRIAC`, not free text
- **Partition**: SafeBoot partition required at `YASOLR_SAFEBOOT_PARTITION_NAME`

### UART Handling

ESP32 supports 1-3 UARTs depending on chip. Code must handle both cases:

```cpp
#if SOC_UART_NUM > 2
  #define YASOLR_UART_CHOICES "N/A,Serial1,Serial2"
#else
  #define YASOLR_UART_CHOICES "N/A,Serial1"
#endif
```

JSY and PZEM can share/conflict on UART - check config before enabling both.

### Task Management

Uses `Mycila::TaskManager` (custom cooperative scheduler):

- Tasks defined as lambda-based `Mycila::Task` objects
- Two task managers: `coreTaskManager` (runs in `loop()`), `unsafeTaskManager` (async FreeRTOS task)
- **Pattern**: `task.setType(Mycila::TaskType::FOREVER)` then `task.setInterval(1000)` or `task.resume()`

Dashboard updates are tasks that `resume()` on config changes to refresh UI.

## Testing & Debugging

### Stack Traces

- Build generates `.elf` files alongside `.bin` firmwares
- Use [esp-stacktrace-decoder](https://maximeborges.github.io/esp-stacktrace-decoder/) with `.elf` to decode crashes
- Stack usage analysis: Uncomment `-fstack-usage` flag to generate `.su` files

### Common Issues

1. **Dimmer ISR conflicts**: Dimmers use hardware interrupts for ZCD - avoid heavy operations during dimmer init
2. **NVS corruption**: Factory reset wipes config - check KEY_ENABLE_DEBUG_BOOT persistence
3. **UART conflicts**: JSY/PZEM share Serial1/Serial2 - validate config prevents overlap

## Development Workflow

1. **Local build**: `pio run -e pro-esp32`
2. **Upload OTA**: Use web interface `/update` or `pio run -t upload` via USB
3. **Logs**: Serial monitor at 115200 baud OR WebSerial interface in browser
4. **Config changes**: Modify via web UI → triggers reconfigure queue → dashboard refresh

### Adding a Feature

1. Declare config key in `yasolr_macros.h` (if needs persistence)
2. Configure default in `yasolr_init_config()` in `yasolr_config.cpp`
3. Create `yasolr_init_<feature>()` and `yasolr_configure_<feature>()` in new/existing file
4. Add declarations to `include/yasolr.h`
5. Call from `src/main.cpp` in proper initialization order
6. Add dashboard UI in `yasolr_dashboard.cpp` with `#ifdef APP_MODEL_PRO` guards if needed

## Home Assistant Integration

- Auto-discovery via MQTT using `Mycila::HADiscovery`
- Discovery topic: `homeassistant` (configurable via `KEY_HA_DISCOVERY_TOPIC`)
- Entities: sensors (power, temp), switches (relays), numbers (dimmer limits)
- Status published to `<mqtt_topic>/status` with LWT

### MQTT Architecture

**Connection management** (`yasolr_mqtt.cpp`):
- Async MQTT client using `Mycila::MQTT` (based on AsyncMQTT)
- TLS support with custom server cert or CA bundle
- Auto-reconnection with configurable keepalive (default 60s)
- Last Will Testament (LWT) for connection status

**Topics structure:**
```
<base_topic>/                    # e.g., "yasolr"
  ├── status                     # online/offline (LWT)
  ├── config/<key>/set          # Config updates
  ├── router/
  │   ├── output1/set           # Control output1 dimmer/bypass
  │   ├── output2/set           # Control output2 dimmer/bypass
  │   ├── relay1/set            # Control relay1
  │   └── relay2/set            # Control relay2
  └── data/                      # Telemetry (JSON payloads)
      ├── grid                   # Grid measurements
      ├── output1                # Output1 state and metrics
      ├── output2                # Output2 state and metrics
      ├── system                 # System info
      └── ...
```

**Home Assistant Discovery:**
Automatically publishes discovery messages to `homeassistant/<component>/<device_id>/<entity>/config`:

**Sensor entities:**
- Grid power, voltage, current, PF, frequency
- Output power, temperature, duty cycle, state
- System temperature, heap usage, uptime
- MQTT message rate, JSY remote rate

**Binary sensor entities:**
- Dimmer online/offline status
- Grid connected status
- Relay states

**Switch entities:**
- Relay1, Relay2 on/off
- Output bypass on/off

**Number entities:**
- Output duty cycle limits
- PID parameters (Kp, Ki, Kd)
- Temperature limits

**Button entities:**
- Restart system
- Factory reset
- Calibration triggers

**Publishing patterns:**
```cpp
// Static config (published once on connect)
mqttPublishConfigTask->resume();

// Periodic telemetry (every 1-5 seconds based on config)
mqttPublishTask->resume();

// Home Assistant discovery (on connect + config changes)
haDiscoveryTask->resume();
```

**Subscription handling:**
Commands from MQTT subscriptions trigger actions immediately:
```cpp
mqtt->subscribe(baseTopic + "/router/output1/set", [](const std::string& topic, const std::string_view& payload) {
  // Parse payload: "on", "off", "bypass=on", "duty=0.5"
  // Apply to output1
});
```

**Message expiration:**
Measurements marked stale after `YASOLR_MQTT_MEASUREMENT_EXPIRATION` (60s) if not updated.

## Language Support

Multi-language via compile-time defines:

- `YASOLR_LANG=YASOLR_LANG_EN` or `YASOLR_LANG_FR`
- Translation strings in `include/i18n/en.h` and `include/i18n/fr.h`
- Use `YASOLR_LBL_*` macros for all user-facing strings

## Special Notes

- **LittleFS**: Web assets, config HTML, logos embedded at build time and served from flash
- **Partitions**: Custom partition tables in `tools/partitions-*` for SafeBoot support
- **OTA**: Two firmware types - `.FACTORY.bin` (full flash) and `.OTA.bin` (app partition only)
