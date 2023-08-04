// switch on (1)or  of (0) serial.print = serial.print replaced by debug //
#define DEBUG 0
#define DEBUGf 0
#define MQTT_DEBUG 0
#define SMARTY_DEBUG 0
#define MODBUS_DEBUG 0

#define PID_DEBUG 0
#define SSR_DEBUG 0
#define PWM_DEBUG 0
#define I2C_DEBUG 0


// -- Web server
// const char *http_username = "admin";
// const char *http_password = "admin";
// AsyncWebServer httpServer(80);
// WebSocketsServer wsServer = WebSocketsServer(81);

// Set your Static IP address
IPAddress local_IP(192, 168, 178, 93);
// Set your Gateway IP address
IPAddress gateway(192, 168, 178, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 178, 1); // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

const char *OTA_host = "Voltaik_PWM_Control"; // Name unter welchem der virtuelle Port in der Arduino-IDE auftaucht

// Modbus Kostal (R)
IPAddress Modbus_ip = {192, 168, 178, 97}; // IP address of modbus server
uint16_t Modbus_port = 502;               // port of modbus server

// switch on (1)or  of (0) Modbus reading not yet implemented

#define Modbus_on_off 0 // switch on (1)or  of (0)

const int Modbus_ID_1 = 1; // Kostal Modbus ID
const int Modbus_ID_2 = 2; //  Modbus ID
const int Modbus_ID_3 = 3; //  Modbus ID
const int Modbus_ID_4 = 4; //  Modbus ID

int Adresse_Modbus_Register_1 = 30017;//Kostal Modbus Adresse Watt L1
int Adresse_Modbus_Register_2 = 30021;//Kostal Modbus Adresse Watt L2
int Adresse_Modbus_Register_3 = 30025;//Kostal Modbus Adresse Watt L3
int Adresse_Modbus_Register_4  = 30030;//Kostal Modbus Adresse Watt Total

  
// MQTT
const char *MQTT_SERVER = "192.168.178.100";
const char *MQTT_CLIENT_ID = "Smarty_esp_Voltaik";
const char *MQTT_OUT_TOPIC = "Voltaik_Surplus";

const short MQTT_PORT = 1883; // TLS=8883

const char *MQTT_IN_TOPIC = "#";

// Smarty 1

const char *Subscription_will = "smarty-1/lastwill/onlinestatus";
const char *Subscription_1 = "smarty-1/power_excess_solar_calc_W";
const char *Subscription_2 = "smarty-1/power_excess_solar_l1_calc_W";
const char *Subscription_3 = "smarty-1/power_excess_solar_l2_calc_W";
const char *Subscription_4 = "smarty-1/power_excess_solar_l3_calc_W";
const char *Subscription_5 = "smarty-1/act_pwr_exported_p_minus_kW";


// Boiler
//const char *Subscription_6 = "Warmwater-Mer/upper_temp";
/*
const char *Subscription_7 = "Warmwater-Mer/lower_temp";
const char *Subscription_8 = "Warmwater-Mer/max_temp";
const char *Subscription_9 = "Warmwater-Mer/test_1";
const char *Subscription_10 = "Warmwater-Mer/test_2";
const char *Subscription_11 = "Warmwater-Mer/Test_3";
*/

// SSR_1 SSR_2
//  values have to be set on webpage index.html

// SSR_1 PWM

int PWM_Freq = 1000; /* 1-3 KHz allowed */ 
int PWM_Channel_0 = 0;
int PWM_Resolution = 12;

//
//
int SSR_1_mode; // 0=digital 1=PWM mapped 2=Pwm  PID

// SSR_1 PID Range 0-4095
float PID_min = 0;
float PID_max = 4095;
float SSR_1_setpoint_distance = 10;
float SSR_1_PID_direction; // 0 direct (default) or 1 reverse

uint32_t SampleTimeUs = 15000000;

int SSR_2_mode; // 0=digital 1=PWM mapped 2=Pwm PID   attention only 0 implemented

int select_Input_SSR_1;
/*
 0 = no input
 1 = power_excess_solar_calc_W
 2 = power_excess_solar_l1_calc_W
 3 = power_excess_solar_l2_calc_W
 4 = power_excess_solar_l3_calc_W
 5 = production Kostal Surplus L1
 6 = production Kostal Surplus L2
 7 = production Kostal Surplus L3
 8 = production Kostal Surplus L1_L2_L3
 9 = production Kostal Surplus L1_L2
 10 = production Kostal Surplus L1_L3
 11 = production Kostal Surplus L2_L3
 12 = test_val_1
 13 = test_val_2
*/
int select_Input_SSR_2;
/*
 0 = no input
 1 = power_excess_solar_calc_W
 2 = power_excess_solar_l1_calc_W
 3 = power_excess_solar_l2_calc_W
 4 = power_excess_solar_l3_calc_W
 5 = production Kostal Surplus L1
 6 = production Kostal Surplus L2
 7 = production Kostal Surplus L3
 8 = production Kostal Surplus L1_L2_L3
 9 = production Kostal Surplus L1_L2
 10 = production Kostal Surplus L1_L3
 11 = production Kostal Surplus L2_L3
 12 = test_val_1
13 = test_val_2
*/


int offset_DAC_2 = 2000;// need for getting positive values in reverse
/*
Controller Action

If a positive error increases the controller’s output, the controller is said to be direct acting (i.e. heating process).
When a positive error decreases the controller’s output, the controller is said to be reverse acting (i.e. cooling process).
Since the PWM and ADC peripherals on microcontrollers only operate with positive values, QuickPID only uses positive values for Input, Output and Setpoint. 
When the controller is set to REVERSE acting, the sign of the error and dInput (derivative of Input) is internally changed. All operating ranges and limits remain the same. 
To simulate a REVERSE acting process from a process that’s DIRECT acting, the Input value needs to be “flipped”.
That is, if your reading from a 10-bit ADC with 0-1023 range, the input value used is (1023 - reading). See the example AutoTune_RC_Filter.ino for details.
*/
uint32_t SampleTimeUs_2 = 15000000;



//stune settimgs user settings
uint32_t settleTimeSec = 10;
uint32_t testTimeSec = 500;
const uint16_t samples = 500;
const float inputSpan = 15;
const float outputSpan = 4095;
float outputStart = 0;
float outputStep = 3;
float tempLimit = 75;

