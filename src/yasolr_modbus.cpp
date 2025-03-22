// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <yasolr.h>



static Mycila::Task* modbusTask = nullptr;

static WiFiClient theClient;     

float modbus_frequency = 0.0f;


// Create a ModbusTCP client instance
ModbusClientTCP* MB = nullptr;

// Define a function to parse a signed 16-bit integer from a Modbus message response
// (Modbus uses big-endian byte order)    
// The response is a Modbus message, which is a vector of bytes
// The function returns a signed 16-bit integer
// scale : 10.0f for x10scale, 1.0f for no scale, 0.1f for 1 decimal, 0.01f for 2 decimals
// signedValue : true if the value is signed, false if the value is unsigned 
int16_t parseSignedorUnsignedInt16(ModbusMessage response,int offset,float scale,bool signedValue) {

  int32_t value = 0;
  // Combine the two bytes of the response into a signed 16-bit integer
  // (Modbus uses big-endian byte order, so we have to swap the bytes)
  //  - response[3] is the high byte
  //  - response[4] is the low byte
  *((unsigned char *)&value + 1) = response[offset];
  *((unsigned char *)&value + 0) = response[offset+1];
  
  // Convert the 16-bit integer to a signed value
  // (Modbus uses two's complement for negative values)
  if (signedValue) {
    if (value>=32768) {
      value = value - 65536;
    }
  } 

  // Apply the scaling factor
  value = value * scale;
  return value;
}



void parseVictronAC(ModbusMessage response){
  // ======================== AC Input based on CCGX-Modbus-TCP-register-list-3.40 ==========================
  // | SLAVE_ID | Description           | Register | Type   | Offset | Scale | Range               | Unit   |
  // |----------|-----------------------|----------|--------|--------|-------|---------------------|--------|
  // | 228      | Input voltage phase 1 | 3        | uint16 | 3 (*)  | 0.1f  | 0 to 6553.5         | V      |
  // | 228      | Input voltage phase 2 | 4        | uint16 | 5      | 0.1f  | 0 to 6553.5         | V      |
  // | 228      | Input voltage phase 3 | 5        | uint16 | 7      | 0.1f  | 0 to 6553.5         | V      |
  // | 228      | Input current phase 1 | 6        | int16  | 9 (*)  | 0.1f  | -3276.8 to 3276.7   | A AC   |
  // | 228      | Input current phase 2 | 7        | int16  | 11     | 0.1f  | -3276.8 to 3276.7   | A AC   |
  // | 228      | Input current phase 3 | 8        | int16  | 13     | 0.1f  | -3276.8 to 3276.7   | A AC   |
  // | 228      | Input frequency 1     | 9        | int16  | 15 (*) | 0.01f | -327.68 to 327.67   | Hz     |
  // | 228      | Input frequency 2     | 10       | int16  | 17     | 0.01f | -327.68 to 327.67   | Hz     |
  // | 228      | Input frequency 3     | 11       | int16  | 19     | 0.01f | -327.68 to 327.67   | Hz     |
  // | 228      | Input power 1         | 12       | int16  | 21 (*) | 1f    | -32768.0 to 32767.0 | VA or W |
  // | 228      | Input power 2         | 13       | int16  | 23     | 1f    | -32768.0 to 32767.0 | VA or W |
  // | 228      | Input power 3         | 14       | int16  | 25     | 1f    | -32768.0 to 32767.0 | VA or W |
  // =========================================================================================================

  // Parse the response to get the input power
  float voltage1 = parseSignedorUnsignedInt16(response,3,0.1f,false);
  float current1 = parseSignedorUnsignedInt16(response,9,0.1f,true);
  modbus_frequency = parseSignedorUnsignedInt16(response,15,0.01f,true);
  float power1 = parseSignedorUnsignedInt16(response,21,1.0f,true);

  if (config.getBool(KEY_ENABLE_DEBUG)) {
    logger.info(TAG,"Victron AC input : %f V, %f A, %f Hz, %f W ", voltage1, current1, modbus_frequency, power1);
  } 
  

  grid.remoteMetrics().update({
    .apparentPower = power1,
    .current =  current1,
    .energy = static_cast<uint32_t>(0),
    .energyReturned = static_cast<uint32_t>(0),
    .power = power1 ,
    .powerFactor =  NAN,
    .voltage = voltage1 ,

  });

  if (grid.updatePower()) {
    yasolr_divert();
  }


}

// Define an onData handler function to receive the regular responses
// Arguments are Modbus server ID, the function code requested, the message data and length of it, 
// plus a user-supplied token to identify the causing request
void handleData(ModbusMessage response, uint32_t token) 
{
  if (config.getBool(KEY_ENABLE_DEBUG)) {
    logger.info(TAG,"Response: serverID=%d, FC=%d, Token=%08X, length=%d ", response.getServerID(), response.getFunctionCode(), token, response.size());
    for (auto& byte : response) {
      logger.info(TAG,"%02X ", byte);
    }
    logger.info(TAG,"=====================");
   }
  
   parseVictronAC(response);



 

}

// Define an onError handler function to receive error responses
// Arguments are the error code returned and a user-supplied token to identify the causing request
void handleError(Error error, uint32_t token) 
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  logger.error(TAG,"Error response: %02X - %s token: %d", (int)me, (const char *)me, token);
}


// Initialize Modbus  TCP Client      
//   - create a ModbusTCP client instance
//   - set up the onData and onError handler functions  
void yasolr_init_modbus() {

  if (config.getBool(KEY_ENABLE_MODBUS)) {
    assert(!MB);
    assert(!modbusTask);

    const char* modbus_IP = config.getString(KEY_MODBUS_SERVER).c_str();
    const int modbus_port = config.getLong(KEY_MODBUS_PORT);

    logger.info(TAG, "Initialize ModBus Client TCP Async on %s:%i", modbus_IP,modbus_port);

    MB = new ModbusClientTCP(theClient,IPAddress(modbus_IP), modbus_port );

    // Set up ModbusTCP client.
    // - provide onData handler function
    MB->onDataHandler(&handleData);
    // - provide onError handler function
    MB->onErrorHandler(&handleError);
    // Set message timeout to 2000ms and interval between requests to the same host to 200ms
    MB->setTimeout(2000,200);
    // Start ModbusTCP background task
    MB->begin();

    modbusTask = new Mycila::Task("MODBUS", [](void* params) {
        // Create request for
        // - server ID = 228 
        // - function code = 0x03 (read holding register)
        // - start address to read = word 3 
        // - number of words to read = 2 (1 word = 2 bytes, therefore int16_t = 1 word)  
        // we read more than one register in one batch to save time, the VICTRON inverter has a lot of data to read
        // - REG : 3 Input voltage phase 1
        // - REG : 4 Input voltage phase 2
        // - REG : 5 Input voltage phase 3  
        // - REG : 6 Input current phase 1
        // - REG : 7 Input current phase 2  
        // - REG : 8 Input current phase 3
        // - REG : 9 Input frequency phase 1
        // - REG : 10 Input frequency phase 2
        // - REG : 11 Input frequency phase 3
        // - REG : 12 Input power power phase 1
        // - REG : 13 Input power power phase 2 
        // - REG : 14 Input power power phase 3
        
        // The request will be issued in the background task, with 12 words to read in a raw
        // - token to match the response with the request. We take the current millis() value for it.
        //
        // If something is missing or wrong with the call parameters, we will immediately get an error code 
        // and the request will not be issued
        unsigned long lastMillis = millis();
        Error err;
        err = MB->addRequest((uint32_t)lastMillis,228, READ_HOLD_REGISTER, 3 , 12);
        if (err != SUCCESS) {
            ModbusError e(err);
            logger.error(TAG,"Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
        // Else the request is processed in the background task and the onData/onError handler functions will get the result.
      });

    modbusTask->setInterval(500);
    if (config.getBool(KEY_ENABLE_DEBUG))
        modbusTask->enableProfiling();

    unsafeTaskManager.addTask(*modbusTask);


    }

}



