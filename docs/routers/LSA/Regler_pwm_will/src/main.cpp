
/*
https://github.com/lorol/LITTLEFS/issues/43
LITTLEFSimpl::exists() bug with Platformio (ESP32 arduino framework) in Visual Studio Code
Fix by adding third argument
File f = open(path, "r", false);

line 44 in LITTLEFS.cpp

also found
https://github.com/espressif/arduino-esp32/pull/6179

With LittleFS the `fs.exists(path)` returns true also on folders. A `isDirectory()` call is required to set _isFile to false on directories.
This enables serving all files from a folder like : `server->serveStatic("/", LittleFS, "/", cacheHeader.c_str());
        File f = fs.open(path);
        _isFile = (f && (! f.isDirectory()));

line 1120
*/

// Includes: <Arduino.h> for Serial etc., WiFi.h for WiFi support
#include <Arduino.h>
#include <WiFi.h>

// for Smarty
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Webserver OTA

#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>

#include <ArduinoOTA.h>

// #include <WebServer.h>
#include <FS.h>
#include "LITTLEFS.h"

// PID
#include <QuickPID.h>
#include <analogWrite.h>

// stune

#include <sTune.h>

// Include the header for the ModbusClient TCP style
#include <ModbusClientTCPasync.h>

#include "credentials.h"

// #define BUILTIN_LED 2
#define Alarm 27
#define SSR_1 26
// #define SSR_1 13
#define SSR_2 22

///////////////////

// #include <LibPrintf.h>

#include "special_settings.h"
// switch on (1)or  of (0) serial.print = serial.print replaced by debug //
// #define DEBUG 1 in special special_settings.h

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#if DEBUGf == 1
#define debugf(x) Serial.printf(x)
#define debugfln(x) Serial.printfln(x)
#else
#define debugf(x)
#define debugfln(x)
#endif

#if MQTT_DEBUG == 1
#define mqtt_debug(x) Serial.print(x)
#define mqtt_debugln(x) Serial.println(x)
#else
#define mqtt_debug(x)
#define mqtt_debugln(x)
#endif

#if PID_DEBUG == 1
#define pid_debug(x) Serial.print(x)
#define pid_debugln(x) Serial.println(x)
#else
#define pid_debug(x)
#define pid_debugln(x)
#endif

#if SSR_DEBUG == 1
#define ssr_debug(x) Serial.print(x)
#define ssr_debugln(x) Serial.println(x)
#else
#define ssr_debug(x)
#define ssr_debugln(x)
#endif

#if SMARTY_DEBUG == 1
#define smarty_debug(x) Serial.print(x)
#define smarty_debugln(x) Serial.println(x)
#else
#define smarty_debug(x)
#define smarty_debugln(x)
#endif

#if MODBUS_DEBUG == 1
#define modbus_debug(x) Serial.print(x)
#define modbus_debugln(x) Serial.println(x)
#else
#define modbus_debug(x)
#define modbus_debugln(x)
#endif

#if I2C_DEBUG == 1
#define i2c -debug(x) Serial.print(x)
#define i2c_debugln(x) Serial.println(x)
#else
#define i2c_debug(x)
#define i2c_debugln(x)
#endif

#if PWM_DEBUG == 1
#define pwm_debug(x) Serial.print(x)
#define pwm_debugln(x) Serial.println(x)
#else
#define pwm_debug(x)
#define pwm_debugln(x)
#endif

#if DFPLAYER_DEBUG == 1
#define dfplayer_debug(x) Serial.print(x)
#define dfplayer_debugln(x) Serial.println(x)
#else
#define dfplayer_debug(x)
#define dfplayer_debugln(x)
#endif

////////////////////

uint16_t Kostal_Pow_L1;
uint16_t Kostal_Pow_L2;
uint16_t Kostal_Pow_L3;
uint16_t Kostal_Pow_tot;

uint16_t Kostal_Pow_L1_L2;
uint16_t Kostal_Pow_L1_L3;
uint16_t Kostal_Pow_L2_L3;

uint16_t Basic_load_L1;
uint16_t Basic_load_L2;
uint16_t Basic_load_L3;

uint16_t Basic_load_tot;
/*
int16_t int_Voltaik_Surplus_L1;
int16_t int_Voltaik_Surplus_L2;
int16_t int_Voltaik_Surplus_L3;
int16_t int_Voltaik_Surplus_tot;
*/

float float_Voltaik_Surplus_L1;
float float_Voltaik_Surplus_L2;
float float_Voltaik_Surplus_L3;
float float_Voltaik_Surplus_tot;

uint32_t maxInflightRequests = 1;
// Für Smarty
float var_Will = 0;
float var_Bezug_tot = 0;
float var_Einsp_tot = 0;
float var_Einsp_L1 = 0;
float var_Einsp_L2 = 0;
float var_Einsp_L3 = 0;

float val_Bezug_tot = 0;
float val_Einsp_tot = 0;
float val_Einsp_L1 = 0;
float val_Einsp_L2 = 0;
float val_Einsp_L3 = 0;

int val_Surplus = 0;
int val_inverter = 0;

float val_Excess_SSR_1 = 0;
float val_Excess_SSR_2 = 0;

// Warmwasser und Heizung Temperaturen
float var_lower_temp_1 = 0;
float var_upper_temp_1 = 0;
float var_max_temp_1 = 0;

float var_lower_temp_2 = 0;
float var_upper_temp_2 = 0;
float var_max_temp_2 = 0;

// map range

int min_range_1 = 10;
int max_range_1 = 20;

int min_range_2 = 10;
int max_range_2 = 20;

int val_SSR_1 = 0;
int val_SSR_2 = 0;

// Solid State Relais

float SSR_1_on;
float SSR_1_off;
float SSR_2_on;
float SSR_2_off;

// PWM
float Var_PWM_1;

// PID

float Setpoint_4;
float aggKp;
float aggKi;
float aggKd;
float consKp;
float consKi;
float consKd;
float Input_4;
float Output_4;

float Setpoint_1;
float aggKp_1;
float aggKi_1;
float aggKd_1;
float consKp_1;
float consKi_1;
float consKd_1;
float Input_1;
float Output_1;

float Setpoint_2;
float aggKp_2;
float aggKi_2;
float aggKd_2;
float consKp_2;
float consKi_2;
float consKd_2;
float Input_2;
float Output_2;

float Input, Output, Setpoint = 50, Kp, Ki, Kd;

QuickPID myPID_4(&Input_4, &Output_4, &Setpoint_4);
QuickPID myPID_1(&Input_1, &Output_1, &Setpoint_1);
QuickPID myPID_2(&Input_2, &Output_2, &Setpoint_2);

sTune tuner = sTune(&Input, &Output, tuner.ZN_PID, tuner.directIP, tuner.printOFF);

QuickPID myPID(&Input, &Output, &Setpoint);

// values for teting

float test_val_1;
float test_val_2;

// webpage Input

const char *PARAM_INPUT_1 = "input_1";
const char *PARAM_INPUT_2 = "input_2";
const char *PARAM_INPUT_3 = "input_3";
const char *PARAM_INPUT_4 = "input_4";

const char *PARAM_INT_1 = "inputInt_1";
const char *PARAM_INT_2 = "inputInt_2";
const char *PARAM_INT_3 = "inputInt_3";
const char *PARAM_INT_4 = "inputInt_4";

const char *PARAM_Setpoint = "Setpoint";
const char *PARAM_Setpoint_1 = "Setpoint_1";
const char *PARAM_Setpoint_2 = "Setpoint_2";

const char *PARAM_Kp = "Kp";
const char *PARAM_Ki = "Ki";
const char *PARAM_Kd = "Kd";

const char *PARAM_aggKp = "aggKp";
const char *PARAM_aggKi = "aggKi";
const char *PARAM_aggKd = "aggKd";

const char *PARAM_Kp_1 = "Kp_1";
const char *PARAM_Ki_1 = "Ki_1";
const char *PARAM_Kd_1 = "Kd_1";

const char *PARAM_aggKp_1 = "aggKp_1";
const char *PARAM_aggKi_1 = "aggKi_1";
const char *PARAM_aggKd_1 = "aggKd_1";

const char *PARAM_Kp_2 = "Kp_2";
const char *PARAM_Ki_2 = "Ki_2";
const char *PARAM_Kd_2 = "Kd_2";

const char *PARAM_aggKp_2 = "aggKp_2";
const char *PARAM_aggKi_2 = "aggKi_2";
const char *PARAM_aggKd_2 = "aggKd_2";

const char *PARAM_SSR_1_on = "SSR_1_on";
const char *PARAM_SSR_2_on = "SSR_2_on";
const char *PARAM_SSR_1_off = "SSR_1_off";
const char *PARAM_SSR_2_off = "SSR_2_off";

const char *PARAM_SSR_1_method = "SSR_1_method";
const char *PARAM_SSR_2_method = "SSR_2_method";

const char *PARAM_SSR_1_mode = "SSR_1_mode";
const char *PARAM_SSR_2_mode = "SSR_2_mode";

const char *PARAM_min_range_1 = "min_range_1";
const char *PARAM_min_range_2 = "min_range_2";
const char *PARAM_max_range_1 = "max_range_1";
const char *PARAM_max_range_2 = "max_range_2";

const char *PARAM_select_Input_SSR_2 = "select_Input_SSR_2";
const char *PARAM_select_Input_SSR_1 = "select_Input_SSR_1";

const char *PARAM_PWM_Freq = "PWM_Freq";
const char *PARAM_PWM_Resolution = "PWM_Resolution";

const char *PARAM_SSR_1_setpoint_distance = "SSR_1_setpoint_distance";
const char *PARAM_SSR_1_PID_direction = "SSR_1_PID_direction";

const char *PARAM_STRING = "inputString";
const char *PARAM_INT = "inputInt";
const char *PARAM_FLOAT = "inputFloat";

const char *PARAM_upper_1 = "upper_1";
const char *PARAM_lower_1 = "lower_1";
const char *PARAM_max_1 = "max_1";

const char *PARAM_upper_2 = "upper_2";
const char *PARAM_lower_2 = "lower_2";
const char *PARAM_max_2 = "max_2";

const char *PARAM_test_val_1 = "test_val_1";
const char *PARAM_test_val_2 = "test_val_2";
// end webpage

// MQTT settings see credentials.h

// Smarty subscriptions see specialsettings.h

WiFiClient ESP32_Client;
PubSubClient MQTT_Client(ESP32_Client);
StaticJsonDocument<64> doc;

int msg = 0;
int msg_L1 = 0;
int msg_L2 = 0;
int msg_L3 = 0;
int msg_Exp = 0;

// Warmwater and heater subscriptions see specialsettings.h

int msg_upper_1 = 0;
int msg_lower_1 = 0;
int msg_max_1 = 0;

int msg_upper_2 = 0;
int msg_lower_2 = 0;
int msg_max_2 = 0;

// LITTLEFS
unsigned int totalBytes = LITTLEFS.totalBytes();
unsigned int usedBytes = LITTLEFS.usedBytes();

// Webserver for parameter data input

AsyncWebServer httpServer(80);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char *path)
{
  // Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory())
  {
    debugln("- empty file or failed to open file");
    return String();
  }
  // debugln("- read from file:");
  String fileContent;
  while (file.available())
  {
    fileContent += String((char)file.read());
  }
  file.close();
  // debugln(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file)
  {
    debugln("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    debugln("- file written");
  }
  else
  {
    debugln("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values

String processor(const String &var)
{
  debugln(var);
  debug("var");
  debugln(var);
  if (var == "inputString")
  {
    return readFile(LITTLEFS, "/inputString.txt");
  }
  else if (var == "inputInt")
  {
    return readFile(LITTLEFS, "/inputInt.txt");
  }
  else if (var == "inputFloat")
  {
    return readFile(LITTLEFS, "/inputFloat.txt");
  }
  else if (var == "inputInt_1")
  {
    return readFile(LITTLEFS, "/inputInt_1.txt");
  }
  else if (var == "inputInt_2")
  {
    return readFile(LITTLEFS, "/inputInt_2.txt");
  }
  else if (var == "inputInt_3")
  {
    return readFile(LITTLEFS, "/inputInt_3.txt");
  }
  else if (var == "inputInt_4")
  {
    return readFile(LITTLEFS, "/inputInt_4.txt");
  }
  /////////////

  else if (var == "Setpoint")
  {
    return readFile(LITTLEFS, "/Setpoint.txt");
  }

  else if (var == "Setpoint_1")
  {
    return readFile(LITTLEFS, "/Setpoint_1.txt");
  }
  else if (var == "Setpoint_2")
  {
    return readFile(LITTLEFS, "/Setpoint_2.txt");
  }
  /////////
  else if (var == "Kp")
  {
    return readFile(LITTLEFS, "*/Kp.txt");
  }
  else if (var == "Ki")
  {
    return readFile(LITTLEFS, "Ki.txt");
  }
  else if (var == "Kd")
  {
    return readFile(LITTLEFS, "/Kd.txt");
  }
  else if (var == "aggKp")
  {
    return readFile(LITTLEFS, "/aggKp.txt");
  }
  else if (var == "aggKi")
  {
    return readFile(LITTLEFS, "/aggKi.txt");
  }
  else if (var == "aggKd")
  {
    return readFile(LITTLEFS, "/aggKd.txt");
  }

  /////////
  else if (var == "Kp_1")
  {
    return readFile(LITTLEFS, "/Kp_1.txt");
  }
  else if (var == "Ki_1")
  {
    return readFile(LITTLEFS, "/Ki_1.txt");
  }
  else if (var == "Kd_1")
  {
    return readFile(LITTLEFS, "/Kd_1.txt");
  }
  else if (var == "aggKp_1")
  {
    return readFile(LITTLEFS, "agg/Kp_1.txt");
  }
  else if (var == "aggKi_1")
  {
    return readFile(LITTLEFS, "/aggKi_1.txt");
  }
  else if (var == "aggKd_1")
  {
    return readFile(LITTLEFS, "/aggKd_1.txt");
  }
  ////////////
  else if (var == "Kp_2")
  {
    return readFile(LITTLEFS, "/Kp_2.txt");
  }
  else if (var == "Ki_2")
  {
    return readFile(LITTLEFS, "/Ki_2.txt");
  }
  else if (var == "Kd_2")
  {
    return readFile(LITTLEFS, "/Kd_2.txt");
  }

  else if (var == "aggKp_2")
  {
    return readFile(LITTLEFS, "/aggKp_2.txt");
  }
  else if (var == "aggKi_2")
  {
    return readFile(LITTLEFS, "/aggKi_2.txt");
  }
  else if (var == "aggKd_2")
  {
    return readFile(LITTLEFS, "/aggKd_2.txt");
  }
  /////////////////////////////
  else if (var == "SSR_1_method")
  {
    return readFile(LITTLEFS, "/SSR_1_method.txt");
  }
  else if (var == "SSR_2_method")
  {
    return readFile(LITTLEFS, "/SSR_2_method.txt");
  }

  /////////////////////////////
  else if (var == "SSR_1_mode")
  {
    return readFile(LITTLEFS, "/SSR_1_mode.txt");
  }
  else if (var == "SSR_2_mode")
  {
    return readFile(LITTLEFS, "/SSR_2_mode.txt");
  }

  else if (var == "select_Input_SSR_1")
  {
    return readFile(LITTLEFS, "/select_Input_SSR_1.txt");
  }
  else if (var == "select_Input_SSR_2")
  {
    return readFile(LITTLEFS, "/select_Input_SSR_2.txt");
  }
  /////////////////////////////

  //////////

  else if (var == "min_range_1")
  {
    return readFile(LITTLEFS, "/min_range_1.txt");
  }
  else if (var == "min_range_2")
  {
    return readFile(LITTLEFS, "/min_range_2.txt");
  }
  else if (var == "max_range_1")
  {
    return readFile(LITTLEFS, "/max_range_1.txt");
  }
  else if (var == "max_range_2")
  {
    return readFile(LITTLEFS, "/max_range_2.txt");
  }

  /////////////////////////////
  else if (var == "SSR_1_on")
  {
    return readFile(LITTLEFS, "/SSR_1_on.txt");
  }
  else if (var == "SSR_2_on")
  {
    return readFile(LITTLEFS, "/SSR_2_on.txt");
  }

  /////////////////////////////
  else if (var == "SSR_1_off")
  {
    return readFile(LITTLEFS, "/SSR_1_off.txt");
  }
  else if (var == "SSR_2_off")
  {
    return readFile(LITTLEFS, "/SSR_2_off.txt");
  }

  else if (var == "SSR_1_PID_direction")
  {
    return readFile(LITTLEFS, "/SSR_1_PID_direction.txt");
  }
  else if (var == "SSR_2_PID_direction")
  {
    return readFile(LITTLEFS, "/SSR_2_PID_direction.txt");
  }
  ///////////////
  else if (var == "PWM_Freq.txt")
  {
    return readFile(LITTLEFS, "/PWM_Freq.txt");
  }
  else if (var == "PWM_Resolution.txt")
  {
    return readFile(LITTLEFS, "/PWM_Resolution.txt");
  }

  else if (var == "action_1.txt")
  {
    return readFile(LITTLEFS, "/action_1.txt");
  }
  else if (var == "action_2.txt")
  {
    return readFile(LITTLEFS, "/action_2.txt");
  }
  else if (var == "gp_1.txt")
  {
    return readFile(LITTLEFS, "/gp_2.txt");
  }
  else if (var == "gp_2.txt")
  {
    return readFile(LITTLEFS, "/gp_2.txt");
  }

  //////////////////
  else if (var == "test_val_1")
  {
    return readFile(LITTLEFS, "/test_val_1");
  }
  else if (var == "test_val_2")
  {
    return readFile(LITTLEFS, "/test_val_2");
  }

  ////////////////

  return String();
}

// ModbusMessage DATA;

///////////////////

char ssid[] = MY_SSID; // SSID and ...
char pass[] = MY_PASS; // password for the WiFi network used

// Create a ModbusTCP client instance
ModbusClientTCPasync MB(Modbus_ip, Modbus_port);

// Define an onData handler function to receive the regular responses
// Arguments are Modbus server ID, the function code requested, the message data and length of it,
// plus a user-supplied token to identify the causing request
void handleData(ModbusMessage response, uint32_t token)
{
  // Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", response.getServerID(), response.getFunctionCode(), token, response.size());
  for (auto &byte : response)
  {
    //  Serial.printf("%02X ", byte);
  }
  // debugln("");

  switch (response.getServerID())
  {
  case 1:
    response.get(3, Kostal_Pow_L1);
    modbus_debug("Watt Kostal  L1 : ");
    modbus_debugln(Kostal_Pow_L1 / 1);
    break;

  case 2:
    response.get(3, Kostal_Pow_L2);
    modbus_debug("Watt Kostal  L2 : ");
    modbus_debugln(Kostal_Pow_L2 / 1);
    break;

  case 3:
    response.get(3, Kostal_Pow_L3);
    modbus_debug("Watt Kostal  L3 : ");
    modbus_debugln(Kostal_Pow_L3 / 1);
    break;

  case 4:
    response.get(3, Kostal_Pow_tot);
    modbus_debug("Watt Kostal total : ");
    modbus_debugln(Kostal_Pow_tot / 1);
  }

  Kostal_Pow_L1_L2 = (Kostal_Pow_L1 + Kostal_Pow_L2) / 1;
  modbus_debug("Watt Kostal  L1_L2 : ");
  modbus_debugln(Kostal_Pow_L1_L2);

  Kostal_Pow_L2_L3 = (Kostal_Pow_L2 + Kostal_Pow_L3) / 1;
  modbus_debug("Watt Kostal  L2_L3 : ");
  modbus_debugln(Kostal_Pow_L2_L3);

  Kostal_Pow_L1_L3 = (Kostal_Pow_L1 + Kostal_Pow_L3) / 1;
  modbus_debug("Watt Kostal  L1_L3 : ");
  modbus_debugln(Kostal_Pow_L1_L3);
}

// Define an onError handler function to receive error responses
// Arguments are the error code returned and a user-supplied token to identify the causing request
void handleError(Error error, uint32_t token)
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  Serial.printf("Error response: %02X - %s token: %d\n", (int)me, (const char *)me, token);
}

// Setup() - initialization happens here

void subscriptions()
{
  MQTT_Client.subscribe(Subscription_will);
  MQTT_Client.subscribe(Subscription_1);
  MQTT_Client.subscribe(Subscription_2);
  MQTT_Client.subscribe(Subscription_3);
  MQTT_Client.subscribe(Subscription_4);
  MQTT_Client.subscribe(Subscription_5);
  /* MQTT_Client.subscribe(Subscription_6);
   MQTT_Client.subscribe(Subscription_7);
   MQTT_Client.subscribe(Subscription_8);
   MQTT_Client.subscribe(Subscription_9);
   MQTT_Client.subscribe(Subscription_10);
   MQTT_Client.subscribe(Subscription_11);
   */
}

void mqtt_reconnect()
{ // Loop until reconnected
  while (!MQTT_Client.connected())
  {
    debug("Attempting MQTT connection...");
    if (MQTT_Client.connect(MQTT_CLIENT_ID))
    { // Attempt to connect
      debugln("connected");
      MQTT_Client.publish(MQTT_OUT_TOPIC, "connected");
      // MQTT_Client.subscribe(MQTT_IN_TOPIC);         // ... and resubscribe

      subscriptions();
    }
    else
    {
      debugln("failed, rc=" + String(MQTT_Client.state()) +
              " try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{

  mqtt_debug("Message arrived in topic: ");
  mqtt_debugln(topic);

  mqtt_debug("Subscribe payload:");
  for (int i = 0; i < length; i++)
  {
    mqtt_debug((char)payload[i]);
  }

  mqtt_debugln();
  mqtt_debugln("-----------------------");

  // Will force input to 0

  if (strcmp(topic, Subscription_will) == 0)
  {
    if (msg != 1)
    {
      char buff_p[length];
      for (int i = 0; i < length; i++)
      {
        buff_p[i] = (char)payload[i];
      }
      buff_p[length] = '\0';
      String msg_p = String(buff_p);
      msg = 1;

      var_Will = msg_p.toFloat(); // to float

      if (var_Will == 0)
      {
        var_Einsp_tot = 0.0; 
        var_Einsp_L1 = 0.0;
        var_Einsp_L2 = 0.0;
        var_Einsp_L3 = 0.0;
      }
    }
  }

  if (strcmp(topic, Subscription_1) == 0)
  {
    if (msg != 1)
    {
      char buff_p[length];
      for (int i = 0; i < length; i++)
      {
        buff_p[i] = (char)payload[i];
      }
      buff_p[length] = '\0';
      String msg_p = String(buff_p);
      msg = 1;

      var_Einsp_tot = msg_p.toFloat(); // to float
    }
  }
  if (strcmp(topic, Subscription_2) == 0)
  {
    if (msg_L1 != 1)
    {
      char buff_p[length];
      for (int i = 0; i < length; i++)
      {
        buff_p[i] = (char)payload[i];
      }
      buff_p[length] = '\0';
      String msg_p = String(buff_p);
      var_Einsp_L1 = msg_p.toFloat(); // to float
      msg_L1 = 1;
    }
  }
  if (strcmp(topic, Subscription_3) == 0)
  {
    if (msg_L2 != 1)
    {
      char buff_p[length];
      for (int i = 0; i < length; i++)
      {
        buff_p[i] = (char)payload[i];
      }
      buff_p[length] = '\0';
      String msg_p = String(buff_p);
      var_Einsp_L2 = msg_p.toFloat(); // to float
      msg_L2 = 1;
    }
  }
  if (strcmp(topic, Subscription_4) == 0)
  {
    if (msg_L3 != 1)
    {
      char buff_p[length];
      for (int i = 0; i < length; i++)
      {
        buff_p[i] = (char)payload[i];
      }
      buff_p[length] = '\0';
      String msg_p = String(buff_p);
      var_Einsp_L3 = msg_p.toFloat(); // to float
      msg_L3 = 1;
    }
  }
  if (strcmp(topic, Subscription_5) == 0)
  {
    if (msg_Exp != 1)
    {
      char buff_p[length];
      for (int i = 0; i < length; i++)
      {
        buff_p[i] = (char)payload[i];
      }
      buff_p[length] = '\0';
      String msg_p = String(buff_p);
      msg_Exp = 1;

      var_Einsp_tot = msg_p.toFloat(); // to float
    }
  }
  /*
   if (strcmp(topic, Subscription_6) == 0)
   {
     if (msg_upper_1 != 1)
     {
       char buff_p[length];
       for (int i = 0; i < length; i++)
       {
         buff_p[i] = (char)payload[i];
       }
       buff_p[length] = '\0';
       String msg_p = String(buff_p);
       var_upper_temp_1 = msg_p.toFloat(); // to float
       msg_upper_1 = 1;
     }
   }
   if (strcmp(topic, Subscription_7) == 0)
   {
     if (msg_lower_1 != 1)
     {
       char buff_p[length];
       for (int i = 0; i < length; i++)
       {
         buff_p[i] = (char)payload[i];
       }
       buff_p[length] = '\0';
       String msg_p = String(buff_p);
       var_lower_temp_1 = msg_p.toFloat(); // to float
       msg_lower_1 = 1;
     }
   }
   if (strcmp(topic, Subscription_8) == 0)
   {
     if (msg_max_1 != 1)
     {
       char buff_p[length];
       for (int i = 0; i < length; i++)
       {
         buff_p[i] = (char)payload[i];
       }
       buff_p[length] = '\0';
       String msg_p = String(buff_p);
       var_max_temp_1 = msg_p.toFloat(); // to float
       msg_max_1 = 1;
     }
   }
   if (strcmp(topic, Subscription_9) == 0)
   {
     if (msg_upper_2 != 1)
     {
       char buff_p[length];
       for (int i = 0; i < length; i++)
       {
         buff_p[i] = (char)payload[i];
       }
       buff_p[length] = '\0';
       String msg_p = String(buff_p);
       var_upper_temp_2 = msg_p.toFloat(); // to float
       msg_upper_2 = 1;
     }
   }
   if (strcmp(topic, Subscription_10) == 0)
   {
     if (msg_lower_2 != 1)
     {
       char buff_p[length];
       for (int i = 0; i < length; i++)
       {
         buff_p[i] = (char)payload[i];
       }
       buff_p[length] = '\0';
       String msg_p = String(buff_p);
       var_lower_temp_2 = msg_p.toFloat(); // to float
       msg_lower_2 = 1;
     }
   }
   if (strcmp(topic, Subscription_11) == 0)
   {
     if (msg_max_2 != 1)
     {
       char buff_p[length];
       for (int i = 0; i < length; i++)
       {
         buff_p[i] = (char)payload[i];
       }
       buff_p[length] = '\0';
       String msg_p = String(buff_p);
       var_max_temp_2 = msg_p.toFloat(); // to float
       msg_max_2 = 1;
     }
   }
   */
}

void printDirectory(File dir, int numTabs = 3);

void printDirectory(File dir, int numTabs)

{
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      debug('\t');
    }
    debug(entry.name());
    if (entry.isDirectory())
    {
      debugln("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void setup()

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

{

  // pinmode

  pinMode(SSR_1, OUTPUT);
  pinMode(SSR_2, OUTPUT);

  /// PWM

  ledcAttachPin(SSR_1, PWM_Channel_0);
  ledcSetup(PWM_Channel_0, PWM_Freq, PWM_Resolution);

  // Init Serial monitor

  Serial.begin(115200);

  while (!Serial)
  {
  }
  debugln("__ OK __");

  // stune

  tuner.Configure(inputSpan, outputSpan, outputStart, outputStep, testTimeSec, settleTimeSec, samples);
  tuner.SetEmergencyStop(tempLimit);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  delay(200);
  while (WiFi.status() != WL_CONNECTED)
  {
    debug(". ");
    delay(1000);
  }

  debugln(F("Inizializing FS..."));
  if (LITTLEFS.begin())
  {
    debugln(F("done."));
  }
  else
  {
    debugln(F("fail."));
  }
  debugln("File sistem info.");

  debug("Total space:      ");
  debug(totalBytes);
  debugln("byte");

  debug("Total space used: ");
  debug(usedBytes);
  debugln("byte");

  debug("Total space free: ");

  debugln("byte");

  debugln();

  // Open dir folder
  File dir = LITTLEFS.open("/");

  // Cycle all the content
  printDirectory(dir);

  // init_wifi();
  MQTT_Client.setServer(MQTT_SERVER, MQTT_PORT);
  MQTT_Client.setCallback(mqtt_callback);

  ////////////
  // Send web page with input fields to client

  httpServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(LITTLEFS, "/www/index.html", "text/html", false); });

  httpServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(LITTLEFS, "/www/index.html", "text/html", processor); });

  // Problems here with serveStatic

  // httpServer.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");//not working see bug ???
  // httpServer.serveStatic("/index1.html", LITTLEFS, "/index1.html");

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  httpServer.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
                {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();
      writeFile(LITTLEFS, "/inputString.txt", inputMessage.c_str());
    }
   
    else if (request->hasParam(PARAM_INT)) {
      inputMessage = request->getParam(PARAM_INT)->value();
      writeFile(LITTLEFS, "/inputInt.txt", inputMessage.c_str());
    }
    
    else if (request->hasParam(PARAM_FLOAT)) {
      inputMessage = request->getParam(PARAM_FLOAT)->value();
      writeFile(LITTLEFS, "/inputFloat.txt", inputMessage.c_str());
    }
  
    else if (request->hasParam(PARAM_INT_1)) {
      inputMessage = request->getParam(PARAM_INT_1)->value();
      writeFile(LITTLEFS, "/inputInt_1.txt", inputMessage.c_str());
    }
    
    else if (request->hasParam(PARAM_INT_2)) {
      inputMessage = request->getParam(PARAM_INT_2)->value();
      writeFile(LITTLEFS, "/inputInt_2.txt", inputMessage.c_str());
    }
    
    else if (request->hasParam(PARAM_INT_3)) {
      inputMessage = request->getParam(PARAM_INT_3)->value();
      writeFile(LITTLEFS, "/inputInt_3.txt", inputMessage.c_str());
    }
   
    else if (request->hasParam(PARAM_INT_4)) {
      inputMessage = request->getParam(PARAM_INT_4)->value();
      writeFile(LITTLEFS, "/inputInt_4.txt", inputMessage.c_str());
    }
     else if (request->hasParam(PARAM_Setpoint)) 
     {
   inputMessage = request->getParam(PARAM_Setpoint)->value();
   writeFile(LITTLEFS, "/Setpoint.txt", inputMessage.c_str());
   }
     

    else if (request->hasParam(PARAM_Setpoint_1)) {
      inputMessage = request->getParam(PARAM_Setpoint_1)->value();
      writeFile(LITTLEFS, "/Setpoint_1.txt", inputMessage.c_str());
    }

      else if (request->hasParam(PARAM_Setpoint_2)) {
     inputMessage = request->getParam(PARAM_Setpoint_2)->value();
      writeFile(LITTLEFS, "/Setpoint_2.txt", inputMessage.c_str());
    }
    
    else if (request->hasParam(PARAM_Kp)) {
      inputMessage = request->getParam(PARAM_Kp)->value();
       writeFile(LITTLEFS, "/Kp.txt", inputMessage.c_str());
    }
    
    
    else if (request->hasParam(PARAM_Ki)) {
      inputMessage = request->getParam(PARAM_Ki)->value();
       writeFile(LITTLEFS, "/Ki.txt", inputMessage.c_str());
    }
   
    else if (request->hasParam(PARAM_Kd)) {
      inputMessage = request->getParam(PARAM_Kd)->value();
      writeFile(LITTLEFS, "/Kd.txt", inputMessage.c_str());
     }
     
     else if(request->hasParam(PARAM_aggKp)) {
     inputMessage = request->getParam(PARAM_aggKp)->value();
      writeFile(LITTLEFS, "/aggKp.txt", inputMessage.c_str());
     }
 
    else if(request->hasParam(PARAM_aggKi)) {
    inputMessage = request->getParam(PARAM_aggKi)->value();
     writeFile(LITTLEFS, "/aggKi.txt", inputMessage.c_str());
    }
     else if(request->hasParam(PARAM_aggKd)) {
    inputMessage = request->getParam(PARAM_aggKd)->value();
    writeFile(LITTLEFS, "/aggKd.txt", inputMessage.c_str());
     }
    
       else if(request->hasParam(PARAM_Kp_1)) {
    inputMessage = request->getParam(PARAM_Kp_1)->value();
     writeFile(LITTLEFS, "/Kp_1.txt", inputMessage.c_str());
     }
    
     
      else if(request->hasParam(PARAM_Ki_1)) {
    inputMessage = request->getParam(PARAM_Ki_1)->value();
     writeFile(LITTLEFS, "/Ki_1.txt", inputMessage.c_str());
    }
 
   else if(request->hasParam(PARAM_Kd_1)) {
   inputMessage = request->getParam(PARAM_Kd_1)->value();
    writeFile(LITTLEFS, "/Kd_1.txt", inputMessage.c_str());
   }
   //////////
   else if(request->hasParam(PARAM_aggKp_1)) {
   inputMessage = request->getParam(PARAM_aggKp_1)->value();
    writeFile(LITTLEFS, "/aggKp_1.txt", inputMessage.c_str());
   }
    

   else if(request->hasParam(PARAM_aggKi_1)) {
   inputMessage = request->getParam(PARAM_aggKi_1)->value();
    writeFile(LITTLEFS, "/aggKi_1.txt", inputMessage.c_str());
   }

   else if(request->hasParam(PARAM_aggKd_1)) {
   inputMessage = request->getParam(PARAM_aggKd_1)->value();
   writeFile(LITTLEFS, "/aggKd_1.txt", inputMessage.c_str());
   }
  
   else if(request->hasParam(PARAM_Kp_2)) {
   inputMessage = request->getParam(PARAM_Kp_2)->value();
   writeFile(LITTLEFS, "/Kp_2.txt", inputMessage.c_str());
   }
    

   else if(request->hasParam(PARAM_Ki_2)) {
   inputMessage = request->getParam(PARAM_Ki_2)->value();
   writeFile(LITTLEFS, "/Ki_2.txt", inputMessage.c_str());
   }

   else if(request->hasParam(PARAM_Kd_2)) {
   inputMessage = request->getParam(PARAM_Kd_2)->value();
   writeFile(LITTLEFS, "/Kd_2.txt", inputMessage.c_str());
   }
 

 
   else if(request->hasParam(PARAM_aggKp_2)) {
   inputMessage = request->getParam(PARAM_aggKp_2)->value();
   writeFile(LITTLEFS, "/aggKp_2.txt", inputMessage.c_str());
   }
    
 
   else if(request->hasParam(PARAM_aggKi_2)) {
   inputMessage = request->getParam(PARAM_aggKi_2)->value();
   writeFile(LITTLEFS, "/aggKi_2.txt", inputMessage.c_str());
   }
 
   else if(request->hasParam(PARAM_aggKd_2)) {
   inputMessage = request->getParam(PARAM_aggKd_2)->value();
   writeFile(LITTLEFS, "/aggKd_2.txt", inputMessage.c_str());
   }

   else if(request->hasParam(PARAM_select_Input_SSR_1)) {
   inputMessage = request->getParam(PARAM_select_Input_SSR_1)->value();
   writeFile(LITTLEFS, "/select_Input_SSR_1.txt", inputMessage.c_str()); 
   }

   else if(request->hasParam(PARAM_select_Input_SSR_2)) {
    inputMessage = request->getParam(PARAM_select_Input_SSR_2)->value();
   writeFile(LITTLEFS, "/select_Input_SSR_2.txt", inputMessage.c_str());
   }

   
   else if(request->hasParam(PARAM_SSR_1_setpoint_distance)) {
   inputMessage = request->getParam(PARAM_SSR_1_setpoint_distance)->value();
   writeFile(LITTLEFS, "/SSR_1_setpoint_distance.txt", inputMessage.c_str());
   }



   else if(request->hasParam(PARAM_SSR_1_PID_direction)) {
   inputMessage = request->getParam(PARAM_SSR_1_PID_direction)->value();
   writeFile(LITTLEFS, "/SSR_1_PID_direction.txt", inputMessage.c_str());
   }


   else if(request->hasParam(PARAM_SSR_1_on)) {
   inputMessage = request->getParam(PARAM_SSR_1_on)->value();
   writeFile(LITTLEFS, "/SSR_1_on.txt", inputMessage.c_str());

   }
   else if(request->hasParam(PARAM_SSR_1_off)) {
   inputMessage = request->getParam(PARAM_SSR_1_off)->value();
   writeFile(LITTLEFS, "/SSR_1_off.txt", inputMessage.c_str());
   }
   else if(request->hasParam(PARAM_SSR_2_on)) {
   inputMessage = request->getParam(PARAM_SSR_2_on)->value();
   writeFile(LITTLEFS, "/SSR_2_on.txt", inputMessage.c_str());

   }
   else if(request->hasParam(PARAM_SSR_2_off)) {
   inputMessage = request->getParam(PARAM_SSR_2_off)->value();
   writeFile(LITTLEFS, "/SSR_2_off.txt", inputMessage.c_str());
   }
   



   else if(request->hasParam(PARAM_PWM_Freq)) {
    inputMessage = request->getParam(PARAM_PWM_Freq)->value();
   writeFile(LITTLEFS, "/PWM_Freq.txt", inputMessage.c_str());
   }

   else if(request->hasParam(PARAM_PWM_Resolution)) {
    inputMessage = request->getParam(PARAM_PWM_Resolution)->value();
   writeFile(LITTLEFS, "/PWM_Resolution.txt", inputMessage.c_str());
   }
 

   else if(request->hasParam(PARAM_SSR_1_method)) {
    inputMessage = request->getParam(PARAM_SSR_1_method)->value();
   writeFile(LITTLEFS, "/SSR_1_method.txt", inputMessage.c_str());
   }
   else if(request->hasParam(PARAM_SSR_2_method)) {
    inputMessage = request->getParam(PARAM_SSR_2_method)->value();
   writeFile(LITTLEFS, "/SSR_2_method.txt", inputMessage.c_str());
   }
   

 
   else if(request->hasParam(PARAM_SSR_1_mode)) {
    inputMessage = request->getParam(PARAM_SSR_1_mode)->value();
   writeFile(LITTLEFS, "/SSR_1_mode.txt", inputMessage.c_str());
   }
   else if(request->hasParam(PARAM_SSR_2_mode)) {
    inputMessage = request->getParam(PARAM_SSR_2_mode)->value();
   writeFile(LITTLEFS, "/SSR_2_mode.txt", inputMessage.c_str());
   }
  

   else if(request->hasParam(PARAM_min_range_1)) {
    inputMessage = request->getParam(PARAM_min_range_1)->value();
   writeFile(LITTLEFS, "/min_range_1.txt", inputMessage.c_str());
   }
    else if(request->hasParam(PARAM_min_range_2)) {
     inputMessage = request->getParam(PARAM_min_range_2)->value();
   writeFile(LITTLEFS, "/min_range_2.txt", inputMessage.c_str());
   }
    else if(request->hasParam(PARAM_max_range_1)) {
    inputMessage = request->getParam(PARAM_max_range_1)->value();
   writeFile(LITTLEFS, "/max_range_1.txt", inputMessage.c_str());
   }
    else if(request->hasParam(PARAM_max_range_2)) {
    inputMessage = request->getParam(PARAM_max_range_2)->value();
   writeFile(LITTLEFS, "/max_range_2.txt", inputMessage.c_str());
   
   }

    else if(request->hasParam(PARAM_upper_1)) {
    inputMessage = request->getParam(PARAM_upper_1)->value();
    writeFile(LITTLEFS, "/upper_1.txt", inputMessage.c_str());
    }

    else if(request->hasParam(PARAM_lower_1)) {
    inputMessage = request->getParam(PARAM_lower_1)->value();
    writeFile(LITTLEFS, "/lower_1.txt", inputMessage.c_str());
    }

     else if(request->hasParam(PARAM_max_1)) {
     inputMessage = request->getParam(PARAM_max_1)->value();
     writeFile(LITTLEFS, "/max_1.txt", inputMessage.c_str());
    }

     else if(request->hasParam(PARAM_upper_2)) {
    inputMessage = request->getParam(PARAM_upper_2)->value();
    writeFile(LITTLEFS, "/upper_2.txt", inputMessage.c_str());
    }
    else if(request->hasParam(PARAM_lower_2)) {
    inputMessage = request->getParam(PARAM_lower_2)->value();
    writeFile(LITTLEFS, "/lower_2.txt", inputMessage.c_str());
    }
    else if(request->hasParam(PARAM_max_2)) {
    inputMessage = request->getParam(PARAM_max_2)->value();
    writeFile(LITTLEFS, "/max_2.txt", inputMessage.c_str());
    }


    else if(request->hasParam(PARAM_test_val_1)) {
   inputMessage = request->getParam(PARAM_test_val_1)->value();
   writeFile(LITTLEFS, "/test_val_1.txt", inputMessage.c_str());
   }

   else if(request->hasParam(PARAM_test_val_2)) {
   inputMessage = request->getParam(PARAM_test_val_2)->value();
   writeFile(LITTLEFS, "/test_val_2.txt", inputMessage.c_str());
   }
     else {
      inputMessage = "No message sent";
    }
    debugln(inputMessage);
    request->send(200, "text/text", inputMessage); });

  httpServer.onNotFound(notFound);
  httpServer.begin();

  IPAddress wIP = WiFi.localIP();
  Serial.printf("WIFi IP address: %u.%u.%u.%u\n", wIP[0], wIP[1], wIP[2], wIP[3]);

  // Set up ModbusTCP client.
  // - provide onData handler function
  MB.onDataHandler(&handleData);
  // - provide onError handler function
  MB.onErrorHandler(&handleError);
  // Set message timeout to 2000ms and interval between requests to the same host to 200ms
  MB.setTimeout(2000);
  // Start ModbusTCP background task
  MB.setIdleTimeout(10000);
  MB.setMaxInflightRequests(maxInflightRequests);

  ////   OTA

  // ArduinoOTA.begin();

  // DFPlayer
}

// End Setup

// loop() - nothing done here today!
void loop()
{
  ArduinoOTA.handle();    // fuer Flashen ueber WLAN
  if (Modbus_on_off == 1) // Modbus ein und Aus schalten
  {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 5000)
    {
      lastMillis = millis();

      // Create request for
      // (Fill in your data here!)
      // - serverID = 1
      // - function code = 0x03 (read holding register)
      // - start address to read = word 40084
      // - number of words to read = 1
      // - token to match the response with the request. We take the current millis() value for it.
      //
      // If something is missing or wrong with the call parameters, we will immediately get an error code
      // and the request will not be issued
      Serial.printf("sending request with token %d\n", (uint32_t)lastMillis);
      Error error;
      error = MB.addRequest((uint32_t)lastMillis, Modbus_ID_1, READ_HOLD_REGISTER, Adresse_Modbus_Register_1, 1);     //  Abfrage 1
      error = MB.addRequest((uint32_t)lastMillis + 1, Modbus_ID_1, READ_HOLD_REGISTER, Adresse_Modbus_Register_2, 1); //  Abfrage 2
      error = MB.addRequest((uint32_t)lastMillis + 2, Modbus_ID_1, READ_HOLD_REGISTER, Adresse_Modbus_Register_3, 1); //  Abfrage 3
      error = MB.addRequest((uint32_t)lastMillis + 3, Modbus_ID_1, READ_HOLD_REGISTER, Adresse_Modbus_Register_4, 1); //  Abfrage 4
      if (error != SUCCESS)
      {
        ModbusError e(error);
        Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
      }

      // Else the request is processed in the background task and the onData/onError handler functions will get the result.
      //
      // The output on the Serial Monitor will be (depending on your WiFi and Modbus the data will be different):
      //     __ OK __
      //     . WIFi IP address: 192.168.178.74
      //     Response: serverID=20, FC=3, Token=0000056C, length=11:
      //     14 03 04 01 F6 FF FF FF 00 C0 A8
    }
  }
  if (!MQTT_Client.connected())
  {
    mqtt_reconnect();
  }

  MQTT_Client.loop();

  if (msg == 1)
  { // check if new callback message

    float_Voltaik_Surplus_tot = var_Einsp_tot; // to float
    // int_Voltaik_Surplus_tot = msg_t.toInt();     // to Int
    mqtt_debug("Excess Solar total: ");
    mqtt_debugln(float_Voltaik_Surplus_tot);
    mqtt_debugln("-----------------------");

    msg = 0; // reset message flag

    if (msg_L1 == 1)
    {
      float_Voltaik_Surplus_L1 = var_Einsp_L1;
      mqtt_debug("Excess Solar L1: ");
      mqtt_debugln(float_Voltaik_Surplus_L1);
      mqtt_debugln("-----------------------");
      msg_L1 = 0;
    }

    if (msg_L2 == 1)
    {
      float_Voltaik_Surplus_L2 = var_Einsp_L2;
      mqtt_debug("Excess L2: ");
      mqtt_debugln(float_Voltaik_Surplus_L2);
      mqtt_debugln("-----------------------");
      msg_L2 = 0;
    }

    if (msg_L3 == 1)
    {
      float_Voltaik_Surplus_L3 = var_Einsp_L3;
      mqtt_debug("Excess L3: ");
      mqtt_debugln(float_Voltaik_Surplus_L3);
      mqtt_debugln("-----------------------");
      msg_L3 = 0;
    }
  }

  //-----------------------SSR1-------------------------//

  select_Input_SSR_1 = readFile(LITTLEFS, "/select_Input_SSR_1.txt").toFloat();

  test_val_1 = readFile(LITTLEFS, "/test_val_1.txt").toFloat();
  test_val_2 = readFile(LITTLEFS, "/test_val_2.txt").toFloat();

  switch (select_Input_SSR_1)
  {
  case 0:
    ssr_debugln("input_1 case 0 ");
    val_Excess_SSR_1 = 0;
    ssr_debug("SSR 1 no value: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 1:
    ssr_debugln("input_1 case 1 ");
    val_Excess_SSR_1 = float_Voltaik_Surplus_tot;
    ssr_debug("SSR 1 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 2:
    ssr_debugln("input_1 case 2 ");
    val_Excess_SSR_1 = float_Voltaik_Surplus_L1;
    ssr_debug("SSR 1 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 3:
    ssr_debugln("input_1 case 3 ");
    val_Excess_SSR_1 = float_Voltaik_Surplus_L2;
    ssr_debug("SSR 1 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 4:
    ssr_debugln("input_1 case 4 ");
    val_Excess_SSR_1 = float_Voltaik_Surplus_L3;
    ssr_debug("SSR 1 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 5:
    ssr_debugln("input_1 case 5 ");
    val_Excess_SSR_1 = Kostal_Pow_L1;
    ssr_debug("SSR 1 Inverter L1: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 6:
    ssr_debugln("input_1 case 6 ");
    val_Excess_SSR_1 = Kostal_Pow_L2;
    ssr_debug("SSR 1 Inverter L2: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 7:
    ssr_debugln("input_1 case 7 ");
    val_Excess_SSR_1 = Kostal_Pow_L3;
    ssr_debug("SSR 1 Inverter L3: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 8:
    ssr_debugln("input_1 case 8 ");
    val_Excess_SSR_1 = Kostal_Pow_tot;
    ssr_debug("SSR 1 Inverter L1 L2 L3: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 9:
    ssr_debugln("input_1 case 9 ");
    val_Excess_SSR_1 = Kostal_Pow_L1_L2;
    ssr_debug("SSR 1 Inverter L1 L2 : ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 10:
    ssr_debugln("input_1 case 10 ");
    val_Excess_SSR_1 = Kostal_Pow_L1_L3;
    ssr_debug("SSR 1 Inverter L1 L3: ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 11:
    ssr_debugln("input_1 case 11 ");
    val_Excess_SSR_1 = Kostal_Pow_L2_L3;
    ssr_debug("SSR 1 Inverter L2 L3:  ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 12:
    ssr_debugln("input_1 case 12 ");
    val_Excess_SSR_1 = test_val_1;
    ssr_debug("SSR 1 test_val_1:  ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  case 13:
    ssr_debugln("input_1 case 13 ");
    val_Excess_SSR_1 = test_val_2;
    ssr_debug("SSR 1 test_val_2:  ");
    ssr_debugln(val_Excess_SSR_1);
    ssr_debugln("-----------------------");
    break;
  }
  ///////////////////////////////
  //-----------------------SSR2-------------------------//

  select_Input_SSR_2 = readFile(LITTLEFS, "/select_Input_SSR_2.txt").toFloat();
  test_val_1 = readFile(LITTLEFS, "/test_val_1.txt").toFloat();
  test_val_2 = readFile(LITTLEFS, "/test_val_2.txt").toFloat();

  switch (select_Input_SSR_2)
  {
  case 0:
    ssr_debugln("input_1 case 0 ");
    val_Excess_SSR_2 = 0;
    ssr_debug("SSR 2 no value: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 1:
    ssr_debugln("input_1 case 1 ");
    val_Excess_SSR_2 = float_Voltaik_Surplus_tot;
    ssr_debug("SSR 2 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 2:
    ssr_debugln("input_1 case 2 ");
    val_Excess_SSR_2 = float_Voltaik_Surplus_L1;
    ssr_debug("SSR 2 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 3:
    ssr_debugln("input_1 case 3 ");
    val_Excess_SSR_2 = float_Voltaik_Surplus_L2;
    ssr_debug("SSR 2 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 4:
    ssr_debugln("input_1 case 4 ");
    val_Excess_SSR_2 = float_Voltaik_Surplus_L3;
    ssr_debug("SSR 2 Excess Solar total: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 5:
    ssr_debugln("input_1 case 5 ");
    val_Excess_SSR_2 = Kostal_Pow_L1;
    ssr_debug("SSR 2 Inverter L1: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 6:
    ssr_debugln("input_1 case 6 ");
    val_Excess_SSR_2 = Kostal_Pow_L2;
    ssr_debug("SSR 2 Inverter L2: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 7:
    ssr_debugln("input_1 case 7 ");
    val_Excess_SSR_2 = Kostal_Pow_L3;
    ssr_debug("SSR 2 Inverter L3: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 8:
    ssr_debugln("input_1 case 8 ");
    val_Excess_SSR_2 = Kostal_Pow_tot;
    ssr_debug("SSR 2 Inverter L1 L2 L3: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 9:
    ssr_debugln("input_1 case 9 ");
    val_Excess_SSR_2 = Kostal_Pow_L1_L2;
    ssr_debug("SSR 2 Inverter L1 L2 : ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 10:
    ssr_debugln("input_1 case 10 ");
    val_Excess_SSR_2 = Kostal_Pow_L1_L3;
    ssr_debug("SSR 2 Inverter L1 L3: ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 11:
    ssr_debugln("input_1 case 11 ");
    val_Excess_SSR_2 = Kostal_Pow_L2_L3;
    ssr_debug("SSR 2 Inverter L2 L3:  ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 12:
    ssr_debugln("input_1 case 12 ");
    val_Excess_SSR_2 = test_val_1;

    ssr_debug("SSR 2 test_val_1:  ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  case 13:
    ssr_debugln("input_1 case 13 ");
    val_Excess_SSR_2 = test_val_2;
    ssr_debug("SSR 2 test_val_2:  ");
    ssr_debugln(val_Excess_SSR_2);
    ssr_debugln("-----------------------");
    break;
  }
  ///////////////////////////////

  //////////////////////////// SSR1  ///////////////////////

  SSR_1_on = readFile(LITTLEFS, "/SSR_1_on.txt").toFloat();
  SSR_1_off = readFile(LITTLEFS, "/SSR_1_off.txt").toFloat();
  SSR_1_mode = readFile(LITTLEFS, "/SSR_1_mode.txt").toFloat();
  PWM_Resolution = readFile(LITTLEFS, "/PWM_Resolution.txt").toFloat();

  if (SSR_1_mode == 0)
  {
    ledcSetup(PWM_Channel_0, PWM_Freq, 12);

    ssr_debugln("------------- SSR_1 - mode 0  ---------");

    if (val_Excess_SSR_1 < SSR_1_off)
    {
      Var_PWM_1 = 0;
      ssr_debugln("------------- SSR_1 ----------");
      ssr_debug("SSR 1 is switched on at value :");
      ssr_debugln(SSR_1_on);
      ssr_debug("SSR 1  is switched off at value : ");
      ssr_debugln(SSR_1_off);
      ssr_debugln("-----------------------");
      ssr_debugln("SSR 1 is off :");
      ssr_debug("value SSR_1 is : ");
      ssr_debugln(val_Excess_SSR_1);
    }

    if (val_Excess_SSR_1 > SSR_1_on)
    {
      Var_PWM_1 = 4095;

      ssr_debugln("-----------------------");
      ssr_debug("SSR 1 is switched on at value :");
      ssr_debugln(SSR_1_on);
      ssr_debug("SSR 1  is switched off at value : ");
      ssr_debugln(SSR_1_off);
      ssr_debugln("-----------------------");
      ssr_debugln("SSR 1 is on :");
      ssr_debug("val_Excess_SSR_1 is : ");
      ssr_debugln(val_Excess_SSR_1);
    }
    ledcWrite(PWM_Channel_0, Var_PWM_1);
  }
  if (SSR_1_mode == 1)
  {
    ledcSetup(PWM_Channel_0, PWM_Freq, 12);

    if ((val_Excess_SSR_1 >= SSR_1_off) && (val_Excess_SSR_1 <= SSR_1_on))
    {
      Var_PWM_1 = map(val_Excess_SSR_1, SSR_1_off, SSR_1_on, 0, 4095);
    }
    if (val_Excess_SSR_1 < SSR_1_off)
    {
      Var_PWM_1 = 0;
    }
    if (val_Excess_SSR_1 > SSR_1_on)
    {
      Var_PWM_1 = 4095;
    }

    ledcWrite(PWM_Channel_0, Var_PWM_1);
    pwm_debugln("SSR 1  is switched to mapped PWM mode : ");
    pwm_debug("SSR 1   PWM is : ");
    pwm_debugln(Var_PWM_1);
    pwm_debug("SSR 1   PWM Frequency  : ");
    pwm_debugln(PWM_Freq);
    pwm_debug("SSR 1   PWM Resolution : ");
    pwm_debugln(PWM_Resolution);
  }

  //////////Mode2///////

  if (SSR_1_mode == 2)
  {
    ledcSetup(PWM_Channel_0, PWM_Freq, PWM_Resolution);
    // PID 2
    pid_debugln("-------------");
    pid_debugln("SSR 1  is switched to PID PWM mede: ");
    SSR_1_PID_direction = readFile(LITTLEFS, "/SSR_1_PID_direction.txt").toFloat();
    PWM_Resolution = readFile(LITTLEFS, "/PWM_Resolution.txt").toFloat();

    switch (PWM_Resolution)
    {
    case 4:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 4 bit");
      PID_max = 15;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 5:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 5 bit");
      PID_max = 31;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 6:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 6 bit");
      PID_max = 63;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 7:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 7 bit");
      PID_max = 127;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 8:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 8 bit");
      PID_max = 255;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 9:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 9 bit");
      PID_max = 511;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;
    case 10:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 10 bit");
      PID_max = 1023;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;
    case 11:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 11 bit");
      PID_max = 2047;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;
    case 12:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 12 bit");
      PID_max = 4095;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;
    case 13:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 13 bit");
      PID_max = 8191;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 14:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 14 bit");
      PID_max = 16383;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 15:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 15 bit");
      PID_max = 32767;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;

    case 16:
      pid_debugln("-------------");
      pid_debugln("PID Resolution 16 bit");
      PID_max = 65535;
      pid_debug("PID Max Steps : 0 - ");
      pid_debugln(PID_max);
      break;
    }

    // turn the PID on
    myPID_4.SetMode(myPID.Control::automatic);
    myPID_4.SetOutputLimits(PID_min, PID_max);

    myPID_4.SetSampleTimeUs(SampleTimeUs);
    if (SSR_1_PID_direction == 1)
    {
      myPID_4.SetControllerDirection(myPID_4.Action::reverse);
      pid_debugln("PID direction  <<---- reverse");
    }
    else
    {
      myPID_4.SetControllerDirection(myPID_4.Action::direct);
      pid_debugln("PID direction  ----->> direct");
    }

    Input_4 = val_Excess_SSR_1;
    Setpoint_4 = readFile(LITTLEFS, "/Setpoint.txt").toFloat();
    SSR_1_setpoint_distance = readFile(LITTLEFS, "/SSR_1_setpoint_distance.txt").toFloat();
    consKp = readFile(LITTLEFS, "/Kp.txt").toFloat();
    consKi = readFile(LITTLEFS, "/Ki.txt").toFloat();
    consKd = readFile(LITTLEFS, "/Kd.txt").toFloat();
    aggKp = readFile(LITTLEFS, "/aggKp.txt").toFloat();
    aggKi = readFile(LITTLEFS, "/aggKi.txt").toFloat();
    aggKd = readFile(LITTLEFS, "/aggKd.txt").toFloat();
    float gap_4 = abs(Setpoint_4 - Input_4); // distance away from setpoint
    if (gap_4 < SSR_1_setpoint_distance)
    { // we're close to setpoint, use conservative tuning parameters
      myPID_4.SetTunings(consKp, consKi, consKd);
    }
    else
    {
      // we're far from setpoint, use aggressive tuning parameters
      myPID_4.SetTunings(aggKp, aggKi, aggKd);
    }

    myPID_4.Compute();

    pid_debugln("-------------");
    pid_debug("Setpoint :  ");
    pid_debugln(Setpoint_4);
    pid_debug("input: ");
    pid_debugln(Input_4);
    pid_debug("Setpoint distance: ");
    pid_debugln(SSR_1_setpoint_distance);
    pid_debug("gap: ");
    pid_debugln(gap_4);
    pid_debugln("-------------");
    pid_debug("SSR 1 PWM output: ");
    pid_debugln(Output_4);
    pid_debugln("-------------");
    pid_debug("consKp: ");
    pid_debugln(consKp);
    pid_debug("consKi: ");
    pid_debugln(consKi);
    pid_debug("consKd: ");
    pid_debugln(consKd);
    pid_debugln("-------------");
    pid_debug("aggKp: ");
    pid_debugln(aggKp);
    pid_debug("aggKi: ");
    pid_debugln(aggKi);
    pid_debug("aggcKd: ");
    pid_debugln(aggKd);
    pid_debugln("-------------");

    ledcWrite(PWM_Channel_0, Output_4);
    ssr_debugln("SSR 1  is switched to PID PWM : ");
    ssr_debug("SSR 1   PWM is : ");
    ssr_debugln(Output_4);
  }

  if (SSR_1_mode > 2)
  {
    ssr_debug("SSR 1 mode incompatible out of range  only 0, 1, 2: ");
  }

  //////////////////////////// SSR2  ///////////////////////
  SSR_2_on = readFile(LITTLEFS, "/SSR_2_on.txt").toFloat();
  SSR_2_off = readFile(LITTLEFS, "/SSR_2_off.txt").toFloat();
  SSR_2_mode = readFile(LITTLEFS, "/SSR_2_mode.txt").toFloat();

  if (SSR_2_mode == 0)
  {
    if (val_Excess_SSR_2 < SSR_2_off)
    {
      digitalWrite(SSR_2, LOW);
      ssr_debugln("-----------------------");
      ssr_debug("SSR 2 is switched on at value :");
      ssr_debugln(SSR_2_on);
      ssr_debug("SSR 2  is switched off at value : ");
      ssr_debugln(SSR_2_off);
      ssr_debugln("-----------------------");
      ssr_debugln("SSR 2 is off :");
      ssr_debug("value SSR_2 is : ");
      ssr_debugln(val_Excess_SSR_2);
    }
    if (val_Excess_SSR_2 > SSR_2_on)
    {
      digitalWrite(SSR_2, HIGH);
      ssr_debugln("-----------------------");
      ssr_debug("SSR 2 is switched on at value :");
      ssr_debugln(SSR_2_on);
      ssr_debug("SSR 2  is switched off at value : ");
      ssr_debugln(SSR_2_off);
      ssr_debugln("-----------------------");
      ssr_debugln("SSR 2 is on :");
      ssr_debug("val_Excess_SSR_2 is : ");
      ssr_debugln(val_Excess_SSR_2);
    }
  }
  if (SSR_2_mode == 1)
  {
    ssr_debug("SSR 2 mode PWM Mapped  is not implemented : ");
  }
  if (SSR_2_mode == 2)
  {
    ssr_debug("SSR 2 mode PWM PID is not implemented : ");
  }
  if (SSR_2_mode > 2)
  {
    ssr_debug("SSR 2 mode incompatible out of range  only 0, 1, 2: ");
  }

  if (DEBUG == 1)
  {
    delay(5000);
  }
  else if (DEBUGf == 1)
  {
    delay(5000);
  }
  else if (MQTT_DEBUG == 1)
  {
    delay(5000);
  }
  else if (SMARTY_DEBUG == 1)
  {
    delay(5000);
  }
  else if (MODBUS_DEBUG == 1)
  {
    delay(5000);
  }

  else if (PID_DEBUG == 1)
  {
    delay(5000);
  }
  else if (SSR_DEBUG == 1)
  {
    delay(5000);
  }
  else if (PWM_DEBUG == 1)
  {
    delay(5000);
  }
  else if (I2C_DEBUG == 1)
  {
    delay(5000);
  }

  else
  {
  }
}

// end loop