#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WiFiClient wifi;
MQTTClient mqtt;

void connectToMQTT() {
  Serial.print("Checking wifi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nConnecting...");

  // This should be different for every device
  while (!mqtt.connect("wc2sensor", "wildcard", "")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");
}

std::string deviceManufacturer = "Milkov";
std::string deviceModel = "ESP32 W Water Meter";
std::string deviceName = "ESP32 W Water Meter";

json getSensorJson(std::string uniqueId, std::string slugName, std::string name, std::string valueName) {
  json sensorJson = json::parse(R"(
    {
      "availability": [
        {
          "topic": "rasp2mqtt/" + uniqueId + "/availability",
          "value_template": "{{ value_json.state }}"
        }
      ],
      "availability_mode": "all",
      "device":
        {
          "identifiers": ["water_meter_" + uniqueId],
          "manufacturer": deviceManufacturer,
          "model": deviceModel,
          "name": deviceName,
        },
      "name": name,
      "device_class": "water",
      "enabled_by_default": false,
      "entity_category": "diagnostic",
      "object_id": uniqueId + "_" + slugName,
      "state_class": "total",
      "state_topic": "rasp2mqtt/" + uniqueId,
      "unique_id": uniqueId + "_" + slugName + "_rasp2mqtt",
      "unit_of_measurement": "L",
      "value_template": "{{ value_json." + valueName + " }}",
    }
  )");

  return sensorJson;
}

void registerMQTTDevices() {
  std::string deviceId = "0x0000000000000002";

  std::string sensorNameHot = "homeassistant/sensor/" + deviceId + "/water_meter_hot/config";
  std::string sensorNameCold = "homeassistant/sensor/" + deviceId + "/water_meter_cold/config";
  
  mqtt.publish(&sensorNameHot[0], &getSensorJson("0x0000000000000002", "water_meter_hot", "Water Meter Hot", "water_hot").dump()[0]);
  mqtt.publish(&sensorNameCold[0], &getSensorJson("0x0000000000000002", "water_meter_cold", "Water Meter Cold", "water_cold").dump()[0]);
}
