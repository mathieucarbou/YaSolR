#ifndef ARDUINO_LOOP_STACK_SIZE
  #define ARDUINO_LOOP_STACK_SIZE 4096
#endif

#ifndef CONFIG_ASYNC_TCP_MAX_ACK_TIME
  #define CONFIG_ASYNC_TCP_MAX_ACK_TIME 3000
#endif

#ifndef CONFIG_ASYNC_TCP_PRIORITY
  #define CONFIG_ASYNC_TCP_PRIORITY 10
#endif

#ifndef CONFIG_ASYNC_TCP_QUEUE_SIZE
  #define CONFIG_ASYNC_TCP_QUEUE_SIZE 128
#endif

#ifndef CONFIG_ASYNC_TCP_RUNNING_CORE
  #define CONFIG_ASYNC_TCP_RUNNING_CORE 1
#endif

#ifndef CONFIG_ASYNC_TCP_STACK_SIZE
  #define CONFIG_ASYNC_TCP_STACK_SIZE 4096
#endif

#ifndef DASH_JSON_SIZE
  #define DASH_JSON_SIZE 4096
#endif

#ifndef ELEGANTOTA_USE_ASYNC_WEBSERVER
  #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#endif

#ifndef MYCILA_JSON_SUPPORT
  #define MYCILA_JSON_SUPPORT 1
#endif

#ifndef MYCILA_LOGGER_SUPPORT
  #define MYCILA_LOGGER_SUPPORT 1
#endif

#ifndef WS_MAX_QUEUED_MESSAGES
  #define WS_MAX_QUEUED_MESSAGES 64
#endif

#ifndef WSL_HIGH_PERF
  #define WSL_HIGH_PERF 1
#endif

#ifndef YASOLR_JSY_SERIAL
  #define YASOLR_JSY_SERIAL Serial2
#endif

#ifndef YASOLR_JSY_RX
  #define YASOLR_JSY_RX 16
#endif

#ifndef YASOLR_JSY_TX
  #define YASOLR_JSY_TX 17
#endif

#ifndef YASOLR_ADMIN_PASSWORD
  #define YASOLR_ADMIN_PASSWORD ""
#endif

#define TAG                          "YaSolR"
#define YASOLR_APP_NAME              "JSY Remote UDP"
#define YASOLR_ADMIN_USERNAME        "admin"
#define YASOLR_UDP_PORT              54321
#define YASOLR_UDP_MSG_TYPE_JSY_DATA 0x01

#include <Arduino.h>
#include <ESPmDNS.h>

#include <ArduinoJson.h>       // https://github.com/bblanchon/ArduinoJson
#include <AsyncTCP.h>          // https://github.com/mathieucarbou/AsyncTCP
#include <ESPAsyncWebServer.h> // https://github.com/mathieucarbou/ESPAsyncWebServer
#include <ESPDash.h>           // https://github.com/mathieucarbou/ayushsharma82-ESP-DASH#dev
#include <ElegantOTA.h>        // https://github.com/mathieucarbou/ayushsharma82-ElegantOTA#dev
#include <MycilaESPConnect.h>  // https://github.com/mathieucarbou/MycilaESPConnect
#include <MycilaJSY.h>         // https://github.com/mathieucarbou/MycilaJSY
#include <MycilaLogger.h>      // https://github.com/mathieucarbou/MycilaLogger
#include <MycilaSystem.h>      // https://github.com/mathieucarbou/MycilaSystem
#include <MycilaTaskManager.h> // https://github.com/mathieucarbou/MycilaTaskMonitor
#include <MycilaTaskMonitor.h> // https://github.com/mathieucarbou/MycilaTaskMonitor
#include <WebSerialLite.h>     // https://github.com/mathieucarbou/WebSerialLite

AsyncUDP udp;
AsyncWebServer webServer(80);
ESPDash dashboard = ESPDash(&webServer, "/dashboard", false);
Mycila::JSY jsy;
Mycila::Logger logger;
Mycila::TaskManager coreTaskManager("core");
Mycila::TaskManager ioTaskManager("io");
Mycila::TaskManager jsyTaskManager("jsy");

Card voltage = Card(&dashboard, GENERIC_CARD, "Voltage", "V");
Card activePower = Card(&dashboard, GENERIC_CARD, "Active Power", "W");
Card apparentPower = Card(&dashboard, GENERIC_CARD, "Apparent Power", "VA");
Card powerFactor = Card(&dashboard, GENERIC_CARD, "Power Factor");

Card current = Card(&dashboard, GENERIC_CARD, "Current", "A");
Card energy = Card(&dashboard, GENERIC_CARD, "Imported Energy", "kWh");
Card energyReturned = Card(&dashboard, GENERIC_CARD, "Exported Energy", "kWh");
Card frequency = Card(&dashboard, GENERIC_CARD, "Grid Frequency", "Hz");

Card restart = Card(&dashboard, BUTTON_CARD, "Restart");
Card energyReset = Card(&dashboard, BUTTON_CARD, "Reset JSY");
Card reset = Card(&dashboard, BUTTON_CARD, "Reset Device");

Statistic networkHostname = Statistic(&dashboard, "Network Hostname");
Statistic networkInterface = Statistic(&dashboard, "Network Interface");
Statistic networkAPIP = Statistic(&dashboard, "Network Access Point IP Address");
Statistic networkAPMAC = Statistic(&dashboard, "Network Access Point MAC Address");
Statistic networkEthIP = Statistic(&dashboard, "Network Ethernet IP Address");
Statistic networkEthMAC = Statistic(&dashboard, "Network Ethernet MAC Address");
Statistic networkWiFiIP = Statistic(&dashboard, "Network WiFi IP Address");
Statistic networkWiFiMAC = Statistic(&dashboard, "Network WiFi MAC Address");
Statistic networkWiFiSSID = Statistic(&dashboard, "Network WiFi SSID");
Statistic networkWiFiRSSI = Statistic(&dashboard, "Network WiFi RSSI");
Statistic networkWiFiSignal = Statistic(&dashboard, "Network WiFi Signal");
Statistic uptime = Statistic(&dashboard, "Uptime");

String hostname;
String ssid;

const size_t sendBufferSize = sizeof(Mycila::JSYData) + 1;

const String toDHHMMSS(uint32_t seconds);
String getEspId();

const Mycila::TaskDoneCallback LOG_EXEC_TIME = [](const Mycila::Task& me, const uint32_t elapsed) {
  logger.debug(TAG, "%s in %" PRIu32 " us", me.getName(), elapsed);
};

Mycila::Task jsyTask("JSY", [](void* params) { jsy.read(); });
Mycila::Task networkManagerTask("ESPConnect", [](void* params) { ESPConnect.loop(); });

Mycila::Task profilerTask("Profiler", [](void* params) {
  Mycila::TaskMonitor.log();
  coreTaskManager.log();
  ioTaskManager.log();
  jsyTaskManager.log();
});

Mycila::Task networkUpTask("Network UP", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Enable Network Services...");

  // Web server
  logger.info(TAG, "Enable Web Server...");
  webServer.begin();
  webServer.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404);
  });

  // mDNS
#ifndef ESPCONNECT_NO_MDNS
  logger.info(TAG, "Enable mDNS...");
  MDNS.addService("http", "tcp", 80);
#endif
});

Mycila::Task otaTask("OTA", Mycila::TaskType::ONCE, [](void* params) {
  logger.info(TAG, "Preparing OTA update...");
  jsy.end();
});

Mycila::Task restartTask("Restart", Mycila::TaskType::ONCE, [](void* params) {
  logger.warn(TAG, "Restarting " YASOLR_APP_NAME "...");
  Mycila::System.restart(500);
});

Mycila::Task dashboardCards("Dashboard", [](void* params) {
  ESPConnectMode mode = ESPConnect.getMode();

  networkAPIP.set(ESPConnect.getIPAddress(ESPConnectMode::AP).toString().c_str());
  networkEthIP.set(ESPConnect.getIPAddress(ESPConnectMode::ETH).toString().c_str());
  networkInterface.set(mode == ESPConnectMode::AP ? "AP" : (mode == ESPConnectMode::STA ? "WiFi" : (mode == ESPConnectMode::ETH ? "Ethernet" : "")));
  networkWiFiIP.set(ESPConnect.getIPAddress(ESPConnectMode::STA).toString().c_str());
  networkWiFiRSSI.set((String(ESPConnect.getWiFiRSSI()) + " dBm").c_str());
  networkWiFiSignal.set((String(ESPConnect.getWiFiSignalQuality()) + " %").c_str());
  networkWiFiSSID.set(ESPConnect.getWiFiSSID().c_str());
  uptime.set(toDHHMMSS(Mycila::System.getUptime()).c_str());

  activePower.update(jsy.getPower2());
  apparentPower.update(jsy.getApparentPower2());
  current.update(jsy.getCurrent2());
  energy.update(jsy.getEnergy2());
  energyReturned.update(jsy.getEnergyReturned2());
  frequency.update(jsy.getFrequency());
  powerFactor.update(jsy.getPowerFactor2());
  voltage.update(jsy.getVoltage2());

  dashboard.sendUpdates();
});

Mycila::Task senderTask("Sender", [](void* params) {
  Mycila::JSYData jsyData;
  jsy.getData(jsyData);
  uint8_t buffer[sendBufferSize];
  buffer[0] = YASOLR_UDP_MSG_TYPE_JSY_DATA;
  memcpy(buffer + 1, &jsyData, sizeof(Mycila::JSYData));
  switch (ESPConnect.getMode()) {
    case ESPConnectMode::AP:
      udp.broadcastTo(buffer, sendBufferSize, YASOLR_UDP_PORT, tcpip_adapter_if_t::TCPIP_ADAPTER_IF_AP);
      break;
    case ESPConnectMode::STA:
      udp.broadcastTo(buffer, sendBufferSize, YASOLR_UDP_PORT, tcpip_adapter_if_t::TCPIP_ADAPTER_IF_STA);
      break;
    case ESPConnectMode::ETH:
      udp.broadcastTo(buffer, sendBufferSize, YASOLR_UDP_PORT, tcpip_adapter_if_t::TCPIP_ADAPTER_IF_ETH);
      break;
    default:
      break;
  }
});

void setup() {
  Serial.begin(115200);

#if ARDUINO_USB_CDC_ON_BOOT
  Serial.setTxTimeoutMs(0);
  delay(100);
#else
  while (!Serial)
    yield();
#endif

  // hostname
  hostname = "yasolr-jsy-" + getEspId();
  hostname.toLowerCase();
  ssid = "YaSolR-JSY-" + getEspId();

  // logging
  esp_log_level_set("*", static_cast<esp_log_level_t>(ARDUHAL_LOG_LEVEL_DEBUG));
  logger.setLevel(ARDUHAL_LOG_LEVEL_DEBUG);
  logger.forwardTo(&Serial);
  logger.warn(TAG, "Booting " YASOLR_APP_NAME "...");

  logger.warn(TAG, "Initializing " YASOLR_APP_NAME "...");

  // system
  Mycila::System.begin();

  // tasks
  dashboardCards.setEnabledWhen([]() { return ESPConnect.isConnected() && dashboard.hasClient() && !dashboard.isAsyncAccessInProgress(); });
  dashboardCards.setInterval(751 * Mycila::TaskDuration::MILLISECONDS);
  dashboardCards.setManager(coreTaskManager);
  jsyTask.setEnabledWhen([]() { return jsy.isEnabled(); });
  jsyTask.setManager(jsyTaskManager);
  Mycila::TaskManager::configureWDT();
  networkManagerTask.setManager(coreTaskManager);
  networkUpTask.setManager(coreTaskManager);
  otaTask.setManager(coreTaskManager);
  profilerTask.setInterval(10 * Mycila::TaskDuration::SECONDS);
  profilerTask.setManager(coreTaskManager);
  restartTask.setManager(coreTaskManager);
  senderTask.setEnabledWhen([]() { return jsy.isEnabled() && ESPConnect.isConnected(); });
  senderTask.setInterval(500 * Mycila::TaskDuration::MILLISECONDS);
  senderTask.setManager(ioTaskManager);

  // profiling
  dashboardCards.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  jsyTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  otaTask.setCallback(LOG_EXEC_TIME);
  profilerTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);
  profilerTask.setCallback(LOG_EXEC_TIME);
  senderTask.enableProfiling(10, Mycila::TaskTimeUnit::MILLISECONDS);

  // Task Monitor
  Mycila::TaskMonitor.begin();
  Mycila::TaskMonitor.addTask("arduino_events");          // Network
  Mycila::TaskMonitor.addTask("async_tcp");               // AsyncTCP
  Mycila::TaskMonitor.addTask("async_udp");               // AsyncUDP
  Mycila::TaskMonitor.addTask("wifi");                    // WiFI
  Mycila::TaskMonitor.addTask(coreTaskManager.getName()); // YaSolR
  Mycila::TaskMonitor.addTask(ioTaskManager.getName());   // YaSolR
  Mycila::TaskMonitor.addTask(jsyTaskManager.getName());  // YaSolR

  // WebSerial
  WebSerial.setAuthentication(YASOLR_ADMIN_USERNAME, YASOLR_ADMIN_PASSWORD);
  WebSerial.begin(&webServer, "/console");
  logger.forwardTo(&WebSerial);

  // ElegantOTA
  ElegantOTA.setAutoReboot(false);
  ElegantOTA.onStart([]() { otaTask.resume(); });
  ElegantOTA.onEnd([](bool success) {
    if (success) {
      logger.info(TAG, "OTA Update Success! Restarting...");
    } else {
      logger.error(TAG, "OTA Failed! Restarting...");
    }
    restartTask.resume();
  });
  ElegantOTA.begin(&webServer, YASOLR_ADMIN_USERNAME, YASOLR_ADMIN_PASSWORD);

  // Dashboard
  webServer.rewrite("/", "/dashboard").setFilter([](AsyncWebServerRequest* request) { return ESPConnect.getState() != ESPConnectState::PORTAL_STARTED; });
  restart.attachCallback([](int32_t value) { restartTask.resume(); });
  reset.attachCallback([](int32_t value) {
    ESPConnect.clearConfiguration();
    restartTask.resume();
  });
  energyReset.attachCallback([](int32_t value) {
    jsy.resetEnergy();
    energyReset.update(0);
    dashboard.refreshCard(&energyReset);
  });
  dashboard.onBeforeUpdate([](bool changes_only) {
    if (!changes_only) {
      logger.debug(TAG, "Dashboard refresh requested");
      networkAPMAC.set(ESPConnect.getMACAddress(ESPConnectMode::AP).c_str());
      networkEthMAC.set(ESPConnect.getMACAddress(ESPConnectMode::ETH).isEmpty() ? "N/A" : ESPConnect.getMACAddress(ESPConnectMode::ETH).c_str());
      networkHostname.set(hostname.c_str());
      networkWiFiMAC.set(ESPConnect.getMACAddress(ESPConnectMode::STA).c_str());
    }
  });
  dashboard.setAuthentication(YASOLR_ADMIN_USERNAME, YASOLR_ADMIN_PASSWORD);
  dashboard.refreshLayout();
  dashboardCards.forceRun();

  // jsy
  jsy.begin(YASOLR_JSY_SERIAL, YASOLR_JSY_RX, YASOLR_JSY_TX);
  if (jsy.isEnabled() && jsy.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400)
    jsy.setBaudRate(Mycila::JSYBaudRate::BAUD_38400);

  // Network Manager
  ESPConnect.setAutoRestart(true);
  ESPConnect.setBlocking(false);
  ESPConnect.listen([](ESPConnectState previous, ESPConnectState state) {
    logger.debug(TAG, "NetworkState: %s => %s", ESPConnect.getStateName(previous), ESPConnect.getStateName(state));
    switch (state) {
      case ESPConnectState::NETWORK_DISABLED:
        logger.warn(TAG, "Disabled Network!");
        break;
      case ESPConnectState::AP_STARTING:
        logger.info(TAG, "Starting Access Point %s...", ESPConnect.getAccessPointSSID().c_str());
        break;
      case ESPConnectState::AP_STARTED:
        logger.info(TAG, "Access Point %s started with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        networkUpTask.resume();
        break;
      case ESPConnectState::NETWORK_CONNECTING:
        logger.info(TAG, "Connecting to network...");
        break;
      case ESPConnectState::NETWORK_CONNECTED:
        logger.info(TAG, "Connected with IP address %s", ESPConnect.getIPAddress().toString().c_str());
        networkUpTask.resume();
        break;
      case ESPConnectState::NETWORK_TIMEOUT:
        logger.warn(TAG, "Unable to connect!");
        break;
      case ESPConnectState::NETWORK_DISCONNECTED:
        logger.warn(TAG, "Disconnected!");
        break;
      case ESPConnectState::NETWORK_RECONNECTING:
        logger.info(TAG, "Trying to reconnect...");
        break;
      case ESPConnectState::PORTAL_STARTING:
        logger.info(TAG, "Starting Captive Portal %s for %" PRIu32 " seconds...", ESPConnect.getAccessPointSSID().c_str(), ESPConnect.getCaptivePortalTimeout());
        break;
      case ESPConnectState::PORTAL_STARTED:
        logger.info(TAG, "Captive Portal started at %s with IP address %s", ESPConnect.getWiFiSSID().c_str(), ESPConnect.getIPAddress().toString().c_str());
        break;
      case ESPConnectState::PORTAL_COMPLETE: {
        if (ESPConnect.hasConfiguredAPMode()) {
          logger.info(TAG, "Captive Portal: Access Point configured");
        } else {
          logger.info(TAG, "Captive Portal: WiFi configured");
        }
        break;
      }
      case ESPConnectState::PORTAL_TIMEOUT:
        logger.warn(TAG, "Captive Portal: timed out.");
        break;
      default:
        break;
    }
  });
  ESPConnect.begin(webServer, hostname, ssid, YASOLR_ADMIN_PASSWORD);

  // start tasks
  assert(coreTaskManager.asyncStart(1024 * 4, 1, 1, 100, true)); // NOLINT
  assert(ioTaskManager.asyncStart(1024 * 4, 2, 1, 100, false));  // NOLINT
  assert(jsyTaskManager.asyncStart(1024 * 3, 5, 0, 100, true));  // NOLINT

  logger.info(TAG, "Started " YASOLR_APP_NAME "!");
}

// Destroy default Arduino async task
void loop() { vTaskDelete(NULL); }

const String toDHHMMSS(uint32_t seconds) {
  const uint8_t days = seconds / 86400;
  seconds = seconds % (uint32_t)86400;
  const uint8_t hh = seconds / 3600;
  seconds = seconds % (uint32_t)3600;
  const uint8_t mm = seconds / 60;
  const uint8_t ss = seconds % (uint32_t)60;
  char buffer[14];
  snprintf(buffer, sizeof(buffer), "%" PRIu8 "d %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8, days % 1000, hh % 100, mm % 100, ss % 100);
  return buffer;
}

String getEspId() {
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i += 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  String espId = String(chipId, HEX);
  espId.toUpperCase();
  return espId;
}
