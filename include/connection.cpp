#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WiFiClient wifi;
MQTTClient mqtt;

void connectToMQTT(std::string uniqueId)
{
  Serial.print("Checking wifi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nConnecting...");

  // This should be different for every device
  while (!mqtt.connect(&uniqueId[0], "wildcard", ""))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");
}
