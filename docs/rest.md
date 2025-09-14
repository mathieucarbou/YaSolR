---
layout: default
title: HTTP API
description: HTTP API
---

# Web Endpoints

- [`/api`](#api)
- [`/api/config`](#apiconfig)
- [`/api/config/backup`](#apiconfigbackup)
- [`/api/grid`](#apigrid)
- [`/api/router`](#apirouter)
- [`/api/system`](#apisystem)
- [`/api/system/reset`](#apisystemreset)
- [`/api/system/restart`](#apisystemrestart)
- [`/api/system/safeboot`](#apisystemsafeboot)

## `/api`

List all available endpoints

```bash
curl -X GET http://<esp-ip>/api
```

```json
{
  "config": "http://192.168.125.123/api/config",
  "config/backup": "http://192.168.125.123/api/config/backup",
  "grid": "http://192.168.125.123/api/grid",
  "router": "http://192.168.125.123/api/router",
  "system": "http://192.168.125.123/api/system",
  "system/reset": "http://192.168.125.123/api/system/reset",
  "system/restart": "http://192.168.125.123/api/system/restart"
}
```

## `/api/config`

Configuration view, update, backup and restore

```bash
curl -X GET http://<esp-ip>/api/config
```

```json
{
  "admin_pwd": "",
  "ap_mode_enable": "false",
  "debug_enable": "true",
  "disp_angle": "0",
  "disp_enable": "true",
  "disp_speed": "3",
  "disp_type": "SH1106",
  "ds18_sys_enable": "true",
  "grid_freq": "50",
  "grid_pow_mqtt": "",
  "grid_volt_mqtt": "",
  "ha_disco_enable": "true",
  "ha_disco_topic": "homeassistant",
  "jsy_enable": "true",
  "lights_enable": "true",
  "mqtt_enable": "true",
  "mqtt_port": "1883",
  "mqtt_pub_itvl": "5",
  "mqtt_pwd": "********",
  "mqtt_secure": "false",
  "mqtt_server": "192.168.125.90",
  "mqtt_topic": "yasolr_a1c48",
  "mqtt_user": "homeassistant",
  "ntp_server": "pool.ntp.org",
  "ntp_timezone": "Europe/Paris",
  "o1_ab_enable": "false",
  "o1_ad_enable": "false",
  "o1_days": "sun,mon,tue,wed,thu,fri,sat",
  "o1_dim_enable": "true",
  "o1_dim_max_d": "4095",
  "o1_dim_max_t": "60",
  "o1_ds18_enable": "true",
  "o1_excess_ratio": "100",
  "o1_pzem_enable": "true",
  "o1_relay_enable": "true",
  "o1_relay_type": "NO",
  "o1_resistance": "24",
  "o1_temp_start": "50",
  "o1_temp_stop": "60",
  "o1_time_start": "22:00",
  "o1_time_stop": "06:00",
  "o2_ab_enable": "false",
  "o2_ad_enable": "false",
  "o2_days": "sun,mon,tue,wed,thu,fri,sat",
  "o2_dim_enable": "true",
  "o2_dim_max_d": "4095",
  "o2_dim_max_t": "60",
  "o2_ds18_enable": "true",
  "o2_excess_ratio": "100",
  "o2_pzem_enable": "true",
  "o2_relay_enable": "true",
  "o2_relay_type": "NO",
  "o2_resistance": "85",
  "o2_temp_start": "50",
  "o2_temp_stop": "60",
  "o2_time_start": "22:00",
  "o2_time_stop": "06:00",
  "pid_dmode": "1",
  "pid_icmode": "2",
  "pid_kd": "0.1",
  "pid_ki": "0.3",
  "pid_kp": "0.3",
  "pid_out_max": "10000",
  "pid_out_min": "-10000",
  "pid_pmode": "2",
  "pid_setpoint": "0",
  "pid_view_enable": "false",
  "pin_disp_scl": "22",
  "pin_disp_sda": "21",
  "pin_ds18": "4",
  "pin_jsy_rx": "16",
  "pin_jsy_tx": "17",
  "pin_lights_g": "0",
  "pin_lights_r": "15",
  "pin_lights_y": "2",
  "pin_o1_dim": "25",
  "pin_o1_ds18": "18",
  "pin_o1_relay": "32",
  "pin_o2_dim": "26",
  "pin_o2_ds18": "5",
  "pin_o2_relay": "33",
  "pin_pzem_rx": "14",
  "pin_pzem_tx": "27",
  "pin_relay1": "13",
  "pin_relay2": "12",
  "pin_zcd": "35",
  "relay1_enable": "true",
  "relay1_load": "0",
  "relay1_type": "NO",
  "relay2_enable": "true",
  "relay2_load": "0",
  "relay2_type": "NO",
  "udp_port": "53964",
  "wifi_pwd": "",
  "wifi_ssid": "IoT",
  "zcd_enable": "true"
}
```

```bash
# Configuration Update:
curl -X POST \
  -F "hostname=foobarbaz" \
  -F "admin_password=" \
  -F "ntp_server=fr.pool.ntp.org" \
  -F "ntp_timezone=Europe/Paris" \
  [...]
  http://<esp-ip>/api/config
```

## `/api/config/backup`

```bash
# Backup configuration config.txt:
curl -X GET http://<esp-ip>/api/config/backup
```

```bash
# Restore configuration config.txt:
curl -X POST -F "data=@./path/to/config.txt" http://<esp-ip>/api/config/restore
```

## `/api/grid`

Display grid electricity information

```bash
curl -X GET http://<esp-ip>/api/grid
```

```json
{
  "apparent_power": -782.8499756,
  "current": 3.253999949,
  "energy": 407.8739929,
  "energy_returned": 21.01000023,
  "frequency": 50.04000092,
  "online": true,
  "power": -10.9598999,
  "power_factor": 0.014,
  "voltage": 234.0771942
}
```

## `/api/router`

Show the router information and allows to control the relays, dimmers and bypass

```bash
curl -X GET http://<esp-ip>/api/router
```

```json
{
  "lights": "🟢 ⚫ ⚫",
  "relay1": "off",
  "relay2": "off",
  "temperature": 24.5,
  "virtual_grid_power": -171.6903992,
  "measurements": {
    "apparent_power": 0,
    "current": 0,
    "energy": 0.810000002,
    "power": 0,
    "power_factor": 0,
    "resistance": 0,
    "thdi": 0,
    "voltage": 0,
    "voltage_dimmed": 0
  },
  "output1": {
    "state": "IDLE",
    "bypass": "off",
    "dimmer": "off",
    "duty_cycle": 0,
    "temperature": 24.25,
    "measurements": {
      "apparent_power": 0,
      "current": 0,
      "energy": 0,
      "power": 0,
      "power_factor": 0,
      "resistance": 0,
      "thdi": 0,
      "voltage": 0,
      "voltage_dimmed": 0
    }
  },
  "output2": {
    "state": "IDLE",
    "bypass": "off",
    "dimmer": "off",
    "duty_cycle": 0,
    "temperature": 0,
    "measurements": {
      "apparent_power": 0,
      "current": 0,
      "energy": 0,
      "power": 0,
      "power_factor": 0,
      "resistance": 0,
      "thdi": 0,
      "voltage": 0,
      "voltage_dimmed": 0
    }
  }
}
```

```bash
# Change relay state for a specific duration (duration is optional)
curl -X POST \
  -F "state=on" \
  http://<esp-ip>/api/router/relay1
```

```bash
# Set the duty cycle of the dimmer [0.0, 100.0]
curl -X POST \
  -F "duty_cycle=50.55" \
  http://<esp-ip>/api/router/output1/dimmer
```

```bash
# Change bypass relay state
curl -X POST \
  -F "state=on" \
  http://<esp-ip>/api/router/output1/bypass
```

## `/api/system`

System information: device, memory usage, network, application, router temperature, etc

```bash
curl -X GET http://<esp-ip>/api/system
```

```json
{
  "app": {
    "manufacturer": "Mathieu Carbou",
    "model": "Pro",
    "name": "YaSolR",
    "version": "main_7292f69_modified"
  },
  "device": {
    "boots": 540,
    "cores": 2,
    "cpu_freq": 240,
    "heap": {
      "total": 262380,
      "usage": 53.27999878,
      "used": 139788
    },
    "id": "A1C48",
    "model": "ESP32-D0WD",
    "revision": 301,
    "uptime": 42
  },
  "firmware": {
    "build": {
      "branch": "main",
      "hash": "7292f69",
      "timestamp": "2024-07-14T15:03:30.849885+00:00"
    },
    "debug": true,
    "filename": "YaSolR-main-pro-esp32-debug.bin"
  },
  "network": {
    "eth": {
      "ip_address": "0.0.0.0",
      "mac_address": ""
    },
    "hostname": "yasolr-a1c48",
    "ip_address": "192.168.125.123",
    "mac_address": "B0:B2:1C:0A:1C:48",
    "mode": "wifi",
    "ntp": "on",
    "wifi": {
      "bssid": "00:17:13:37:28:C0",
      "ip_address": "192.168.125.123",
      "mac_address": "B0:B2:1C:0A:1C:48",
      "quality": 100,
      "rssi": -30,
      "ssid": "IoT"
    }
  }
}
```

## `/api/system/reset`

```bash
# factory Reset
curl -X POST http://<esp-ip>/api/system/reset
```

## `/api/system/restart`

```bash
# System Reboot
curl -X POST http://<esp-ip>/api/system/restart
```

## `/api/system/safeboot`

```bash
# Reboot in SafeBoot mode
curl -X POST http://<esp-ip>/api/system/safeboot
```
