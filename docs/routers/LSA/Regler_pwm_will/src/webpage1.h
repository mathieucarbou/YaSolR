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
// end webpage

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

void writeFile(fs::FS &fs, const char *path, const char *message
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
    else if (var == "SSR_1_setpoint_distance")
    {
        return readFile(LITTLEFS, "/SSR_1_setpoint_distance.txt");
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
        return readFile(LITTLEFS, "/Kp.txt");
    }
    else if (var == "Ki")
    {
        return readFile(LITTLEFS, "/Ki.txt");
    }
    else if (var == "Kd")
    {
        return readFile(LITTLEFS, "/Kd.txt");
    }
    else if (var == "aggKp")
    {
        return readFile(LITTLEFS, "agg/Kp.txt");
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
    ////////////////////////////////////////
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

    return String();
}
  // Send web page with input fields to client

  httpServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                {
    request->send(LITTLEFS, "/index.html", "text/html", false); });

  httpServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                {
    request->send(LITTLEFS, "/index.html", "text/html", processor); });

  // httpServer.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");//not working see bug

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  httpServer.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
                {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING))
    {
        inputMessage = request->getParam(PARAM_STRING)->value();
        writeFile(LITTLEFS, "/inputString.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    else if (request->hasParam(PARAM_INT))
    {
        inputMessage = request->getParam(PARAM_INT)->value();
        writeFile(LITTLEFS, "/inputInt.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_FLOAT))
    {
        inputMessage = request->getParam(PARAM_FLOAT)->value();
        writeFile(LITTLEFS, "/inputFloat.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt_1=<inputMessage>
    else if (request->hasParam(PARAM_INT_1))
    {
        inputMessage = request->getParam(PARAM_INT_1)->value();
        writeFile(LITTLEFS, "/inputInt_1.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt_2=<inputMessage>
    else if (request->hasParam(PARAM_INT_2))
    {
        inputMessage = request->getParam(PARAM_INT_2)->value();
        writeFile(LITTLEFS, "/inputInt_2.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt_3=<inputMessage>
    else if (request->hasParam(PARAM_INT_3))
    {
        inputMessage = request->getParam(PARAM_INT_3)->value();
        writeFile(LITTLEFS, "/inputInt_3.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt_4=<inputMessage>
    else if (request->hasParam(PARAM_INT_4))
    {
        inputMessage = request->getParam(PARAM_INT_4)->value();
        writeFile(LITTLEFS, "/inputInt_4.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Setpoint_1))
    {
        inputMessage = request->getParam(PARAM_Setpoint_1)->value();
        writeFile(LITTLEFS, "/Setpoint_1.txt", inputMessage.c_str());
    }

    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Setpoint_2))
    {
        inputMessage = request->getParam(PARAM_Setpoint_2)->value();
        writeFile(LITTLEFS, "/Setpoint_2.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Kp))
    {
        inputMessage = request->getParam(PARAM_Kp)->value();
        writeFile(LITTLEFS, "/Kp.txt", inputMessage.c_str());
    }

    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Ki))
    {
        inputMessage = request->getParam(PARAM_Ki)->value();
        writeFile(LITTLEFS, "/Ki.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Kd))
    {
        inputMessage = request->getParam(PARAM_Kd)->value();
        writeFile(LITTLEFS, "/Kd.txt", inputMessage.c_str());
    }
    //////////
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Kp_1))
    {
        inputMessage = request->getParam(PARAM_Kp_1)->value();
        writeFile(LITTLEFS, "/Kp_1.txt", inputMessage.c_str());
    }

    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Ki_1))
    {
        inputMessage = request->getParam(PARAM_Ki_1)->value();
        writeFile(LITTLEFS, "/Ki_1.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Kd_1))
    {
        inputMessage = request->getParam(PARAM_Kd_1)->value();
        writeFile(LITTLEFS, "/Kd_1.txt", inputMessage.c_str());
    }
    //////////
    else if (request->hasParam(PARAM_aggKp_1))
    {
        inputMessage = request->getParam(PARAM_aggKp_1)->value();
        writeFile(LITTLEFS, "/aggKp_1.txt", inputMessage.c_str());
    }

    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_aggKi_1))
    {
        inputMessage = request->getParam(PARAM_aggKi_1)->value();
        writeFile(LITTLEFS, "/aggKi_1.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_aggKd_1))
    {
        inputMessage = request->getParam(PARAM_aggKd_1)->value();
        writeFile(LITTLEFS, "agg/Kd_1.txt", inputMessage.c_str());
    }
    //////////
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Kp_2))
    {
        inputMessage = request->getParam(PARAM_Kp_2)->value();
        writeFile(LITTLEFS, "/Kp_2.txt", inputMessage.c_str());
    }

    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Ki_2))
    {
        inputMessage = request->getParam(PARAM_Ki_2)->value();
        writeFile(LITTLEFS, "/Ki_2.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_Kd_2))
    {
        inputMessage = request->getParam(PARAM_Kd_2)->value();
        writeFile(LITTLEFS, "/Kd_2.txt", inputMessage.c_str());
    }
    //////////
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_aggKp_2))
    {
        inputMessage = request->getParam(PARAM_aggKp_2)->value();
        writeFile(LITTLEFS, "/aggKp_2.txt", inputMessage.c_str());
    }

    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_aggKi_2))
    {
        inputMessage = request->getParam(PARAM_aggKi_2)->value();
        writeFile(LITTLEFS, "/aggKi_2.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_aggKd_2))
    {
        inputMessage = request->getParam(PARAM_aggKd_2)->value();
        writeFile(LITTLEFS, "/aggKd_2.txt", inputMessage.c_str());
    }

    else if (request->hasParam(PARAM_select_Input_SSR_1))
    {
        inputMessage = request->getParam(PARAM_select_Input_SSR_1)->value();
        writeFile(LITTLEFS, "/Input_SSR_1.txt", inputMessage.c_str());
    }

    else if (request->hasParam(PARAM_select_Input_SSR_2))
    {
        inputMessage = request->getParam(PARAM_select_Input_SSR_2)->value();
        writeFile(LITTLEFS, "/Input_SSR_2.txt", inputMessage.c_str());
    }

 

    else if (request->hasParam(PARAM_SSR_1_setpoint_distance))
    {
        inputMessage = request->getParam(PARAM_SSR_1_setpoint_distance)->value();
        writeFile(LITTLEFS, "/SSR_1_setpoint_distance.txt", inputMessage.c_str());
    }
    
    else if (request->hasParam(PARAM_SSR_1_PID_direction))
    {
        inputMessage = request->getParam(PARAM_SSR_1_PID_direction)->value();
        writeFile(LITTLEFS, "/SSR_1_PID_direction.txt", inputMessage.c_str());
    }

  
    else if (request->hasParam(PARAM_SSR_1_on))
    {
        inputMessage = request->getParam(PARAM_SSR_1_on)->value();
        writeFile(LITTLEFS, "/SSR_1_on.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_SSR_1_off))
    {
        inputMessage = request->getParam(PARAM_SSR_1_off)->value();
        writeFile(LITTLEFS, "/SSR_1_off.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_SSR_2_on))
    {
        inputMessage = request->getParam(PARAM_SSR_2_on)->value();
        writeFile(LITTLEFS, "/SSR_2_on.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_SSR_2_off))
    {
        inputMessage = request->getParam(PARAM_SSR_2_off)->value();
        writeFile(LITTLEFS, "/SSR_2_off.txt", inputMessage.c_str());
    }
    

    /////////////////

    else if (request->hasParam(PARAM_PWM_Freq))
    {
        inputMessage = request->getParam(PARAM_PWM_Freq)->value();
        writeFile(LITTLEFS, "/PWM_Freq.txt", inputMessage.c_str());
    }

    else if (request->hasParam(PARAM_PWM_Resolution))
    {
        inputMessage = request->getParam(PARAM_PWM_Resolution)->value();
        writeFile(LITTLEFS, "/PWM_Resolution.txt", inputMessage.c_str());
    }
    ////////////

    else if (request->hasParam(PARAM_SSR_1_method))
    {
        inputMessage = request->getParam(PARAM_SSR_1_method)->value();
        writeFile(LITTLEFS, "/SSR_1_method.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_SSR_2_method))
    {
        inputMessage = request->getParam(PARAM_SSR_2_method)->value();
        writeFile(LITTLEFS, "/SSR_2_method.txt", inputMessage.c_str());
    }
  
    ////////////////////
    else if (request->hasParam(PARAM_SSR_1_mode))
    {
        inputMessage = request->getParam(PARAM_SSR_1_mode)->value();
        writeFile(LITTLEFS, "/SSR_1_mode.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_SSR_2_mode))
    {
        inputMessage = request->getParam(PARAM_SSR_2_mode)->value();
        writeFile(LITTLEFS, "/SSR_2_mode.txt", inputMessage.c_str());
    }
  

    ////////////////////

    else if (request->hasParam(PARAM_min_range_1))
    {
        inputMessage = request->getParam(PARAM_min_range_1)->value();
        writeFile(LITTLEFS, "/min_range_1.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_min_range_2))
    {
        inputMessage = request->getParam(PARAM_min_range_2)->value();
        writeFile(LITTLEFS, "/min_range_2.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_max_range_1))
    {
        inputMessage = request->getParam(PARAM_max_range_1)->value();
        writeFile(LITTLEFS, "/max_range_1.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_max_range_2))
    {
        inputMessage = request->getParam(PARAM_max_range_2)->value();
        writeFile(LITTLEFS, "/max_range_2.txt", inputMessage.c_str());
    }

    else
    {
        inputMessage = "No message sent";
    }
    debugln(inputMessage);
    request->send(200, "text/text", inputMessage); });
  httpServer.onNotFound(notFound);
  httpServer.begin();

  ///////////////

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
}