#include <Arduino.h>
#include <ArduinoJson.h>
#include <MycilaJSY.h>
#include <MycilaLogger.h>

Mycila::JSY jsy;
Mycila::Logger logger;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  logger.setLevel(ARDUHAL_LOG_LEVEL_DEBUG);
  logger.forwardTo(&Serial);

  // read JSY on pins 17 (JSY RX / Serial TX) and 16 (JSY TX / Serial RX)
  jsy.begin(Serial2, 16, 17);
  if (jsy.isEnabled() && jsy.getBaudRate() != Mycila::JSYBaudRate::BAUD_38400)
    jsy.setBaudRate(Mycila::JSYBaudRate::BAUD_38400);
}

void loop() {
  jsy.read();

  if (jsy.isEnabled()) {
    Serial.println((int)jsy.getBaudRate());

    JsonDocument doc;
    jsy.toJson(doc.to<JsonObject>());
    serializeJson(doc, Serial);
    Serial.println();
  }
  delay(1000);
}
