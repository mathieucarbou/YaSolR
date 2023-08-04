---
layout: default
title: MQTT API
description: MQTT API
---

# MQTT Topics

- [`/status`](#status)
- [`/config`](#config)
- [`/grid`](#grid)
- [`/router`](#router)
- [`/router/outputX`](#routeroutputx)
- [`/system/app`](#systemapp)
- [`/system/device`](#systemdevice)
- [`/system/device/restart`](#systemdevicerestart)
- [`/system/device/heap`](#systemdeviceheap)
- [`/system/firmware`](#systemfirmware)
- [`/system/firmware/build`](#systemfirmwarebuild)
- [`/system/network`](#systemnetwork)
- [`/system/network/eth`](#systemnetworketh)
- [`/system/network/wifi`](#systemnetworkwifi)

Not everything MQTT topic will update frequently (5 sec by default).
Some topics, like configuration related, will only update when the configuration is changed.
These will have the retain flag set to true so that a subscriber coming after the data was published will still get the update.

## `/status`

This is the will topic which can be used to detect when the device is connected or gone.
It is also used by Home Assistant discovery.
It is set to `online` or `offline`

## `/config`

```properties
admin_pwd = ********
ap_mode_enable = false
debug_enable = true
disp_angle = 0
disp_enable = true
disp_speed = 3
disp_type = SH1106
ds18_sys_enable = true
grid_freq = 50
grid_pow_mqtt = homeassistant/states/sensor/grid_power/state
grid_volt = 230
grid_volt_mqtt =
ha_disco_enable = true
ha_disco_topic = homeassistant/discovery
jsy_enable = true
lights_enable = true
mqtt_enable = true
mqtt_port = 1883
mqtt_pub_itvl = 5
mqtt_pwd = ********
mqtt_secure = false
mqtt_server = 192.168.125.90
mqtt_topic = yasolr_a1c48
mqtt_user = homeassistant
ntp_server = pool.ntp.org
ntp_timezone = Europe/Paris
o1_ab_enable = false
o1_ad_enable = false
o1_days = sun,mon,tue,wed,thu,fri,sat
o1_dim_enable = true
o1_dim_limit = 100
o1_ds18_enable = true
o1_pzem_enable = true
o1_relay_enable = true
o1_relay_type = NO
o1_temp_start = 50
o1_temp_stop = 60
o1_time_start = 22:00
o1_time_stop = 06:00
o2_ab_enable = false
o2_ad_enable = false
o2_days = sun,mon,tue,wed,thu,fri,sat
o2_dim_enable = true
o2_dim_limit = 27
o2_ds18_enable = true
o2_pzem_enable = true
o2_relay_enable = false
o2_relay_type = NO
o2_temp_start = 50
o2_temp_stop = 60
o2_time_start = 22:00
o2_time_stop = 06:00
pin_disp_scl = 22
pin_disp_sda = 21
pin_ds18 = 4
pin_jsy_rx = 16
pin_jsy_tx = 17
pin_lights_g = 0
pin_lights_r = 15
pin_lights_y = 2
pin_o1_dim = 25
pin_o1_ds18 = 18
pin_o1_relay = 32
pin_o2_dim = 26
pin_o2_ds18 = 5
pin_o2_relay = 33
pin_pzem_rx = 14
pin_pzem_tx = 27
pin_relay1 = 13
pin_relay2 = 12
pin_zcd = 35
relay1_enable = true
relay1_load = 0
relay1_type = NO
relay2_enable = true
relay2_load = 0
relay2_type = NO
wifi_pwd =
wifi_ssid = IoT
zcd_enable = true
```

**Update**

```properties
# Set a configuration key to a new value
<prefix>/config/<key>/set
```

## `/grid`

```properties
apparent_power = 0
current = 0
energy = 0
energy_returned = 0
frequency = 49.97999954
online = true
power = 617.6273804
power_factor = 0
voltage = 234.0296021
```

## `/router`

```properties
apparent_power = 0
current = 0
energy = 0.067999996
lights = 🟢 🟡 ⚫
power = 239.4906921
power_factor = 0.495258361
temperature = 26.30999947
relay1 = off
relay2 = off
thdi = 1.550398946
virtual_grid_power = -503.7492371
```

**Update**

```properties
# Switch relay on or off or for a duration in milliseconds
<prefix>/router/relayX/set = "on"
<prefix>/router/relayX/set = "off"
```

## `/router/outputX`

```properties
state = Idle
bypass = on
dimmer = on
duty_cycle = 100
temperature = 26.3
```

**Update**

```properties
# Switch bypass on or off
<prefix>/router/outputX/bypass/set = "on"
<prefix>/router/outputX/bypass/set = "off"

# Update the dimmer duty cycle
<prefix>/router/outputX/duty_cycle/set = [0.0, 100.0]
```

## `/system/app`

```properties
manufacturer = Mathieu Carbou
model = Pro
name = YaSolR
version = main_0ed7d852_modified
```

## `/system/device`

```properties
boots = 1
cores = 2
cpu_freq = 240
id = A1C48
model = ESP32-D0WD-V3
uptime = 1675
```

## `/system/device/restart`

Restarts tge device

## `/system/device/heap`

```properties
total = 269404
usage = 47.63
used = 128316
```

## `/system/firmware`

```properties
debug = true
filename = YaSolR-main-pro-esp32-debug.bin
```

## `/system/firmware/build`

```properties
branch = main
hash = da49a3a
timestamp = 2024-06-08T12:14:30.915965+00:00
```

## `/system/network`

```properties
hostname = yasolr-a1c48
ip_address = 192.168.125.121
mac_address = B0:B2:1C:0A:1C:48
mode = wifi
ntp = on
```

## `/system/network/eth`

```properties
ip_address = 0.0.0.0
mac_address = B0:B2:1C:0A:1C:50
```

## `/system/network/wifi`

```properties
bssid = 00:17:13:37:28:C0
ip_address = 0.0.0.0
mac_address = B0:B2:1C:0A:1C:50
quality = 100
rssi = -21
ssid = IoT
```
