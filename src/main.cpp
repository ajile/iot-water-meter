#include "Arduino.h"
#include <WiFi.h>
#include <MQTT.h>

#include "secrets.h"

#define WATER_METER_COLD_PIN 16
#define WATER_METER_HOT_PIN 17

WiFiClient wifi;
MQTTClient mqtt;

boolean prevWaterMeterColdState = 0;
boolean prevWaterMeterHotState = 0;

int counterCold = 0;
int counterHot = 0;

unsigned long availabilityCheckLastMillis = 0;
unsigned long valuesSendLastMillis = 0;
unsigned long reregisterLastMillis = 0;

void registerMQTTDevice() {
  mqtt.publish("homeassistant/sensor/0x0000000000000002/water_meter_hot/config", "{\"availability\": [{\"topic\": \"rasp2mqtt/0x0000000000000002/availability\", \"value_template\": \"{{ value_json.state }}\"}],\"availability_mode\": \"all\",\"device\": {\"identifiers\": [\"water_meter_0x0000000000000002\"],\"manufacturer\": \"Milkov\",\"model\": \"ESP32 W Water Meter\",\"name\": \"ESP32 W Water Meter\"},\"name\": \"Water Meter Hot\",\"device_class\": \"water\",\"enabled_by_default\": false,\"entity_category\": \"diagnostic\",\"object_id\": \"0x0000000000000002_water_meter_hot\",\"state_class\": \"total\",\"state_topic\": \"rasp2mqtt/0x0000000000000002\",\"unique_id\": \"0x0000000000000002_water_meter_hot_rasp2mqtt\",\"unit_of_measurement\": \"L\",\"value_template\": \"{{ value_json.water_hot }}\"}");
  mqtt.publish("homeassistant/sensor/0x0000000000000002/water_meter_cold/config", "{\"availability\": [{\"topic\": \"rasp2mqtt/0x0000000000000002/availability\", \"value_template\": \"{{ value_json.state }}\"}],\"availability_mode\": \"all\",\"device\": {\"identifiers\": [\"water_meter_0x0000000000000002\"],\"manufacturer\": \"Milkov\",\"model\": \"ESP32 W Water Meter\",\"name\": \"ESP32 W Water Meter\"},\"name\": \"Water Meter Cold\",\"device_class\": \"water\",\"enabled_by_default\": false,\"entity_category\": \"diagnostic\",\"object_id\": \"0x0000000000000002_water_meter_cold\",\"state_class\": \"total\",\"state_topic\": \"rasp2mqtt/0x0000000000000002\",\"unique_id\": \"0x0000000000000002_water_meter_cold_rasp2mqtt\",\"unit_of_measurement\": \"L\",\"value_template\": \"{{ value_json.water_cold }}\"}");
}

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

void setup() {
  Serial.begin(9600);

  const char ssid[] = WLAN_SSID;
  const char pass[] = WLAN_PASS;

  // Set up Wi-Fi
  WiFi.begin(ssid, pass);

  // Set up MQTT
  mqtt.begin(MQTT_SERVER, wifi);

  // Connect to Wi-Fi and MQTT
  connectToMQTT();

  // Registering a new device and the events it produces via MQTT will allow
  // Home Assistant to effectively manage and respond to these events.
  if (mqtt.connected()) registerMQTTDevice();

  // Set up purpose of GPIOs.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(WATER_METER_COLD_PIN, INPUT_PULLDOWN);
  pinMode(WATER_METER_HOT_PIN, INPUT_PULLDOWN);
}

void loop() {
  mqtt.loop();

  // Periodically check the status of the Wi-Fi and MQTT connections
  // and recreate them if they are not established.
  if (!mqtt.connected()) connectToMQTT();

  // Whether the wires of cold or hot water meters are connected.
  boolean waterMeterColdState = digitalRead(WATER_METER_COLD_PIN);
  boolean waterMeterHotState = digitalRead(WATER_METER_HOT_PIN);

  // Everytime when value changes to positive the counter increases.
  if (waterMeterColdState != prevWaterMeterColdState) {
    if (waterMeterColdState) {
      digitalWrite(LED_BUILTIN, HIGH);
      counterCold += 10;
    }

    prevWaterMeterColdState = waterMeterColdState;
  }

  // Everytime when value changes to positive the counter increases.
  if (waterMeterHotState != prevWaterMeterHotState) {
    if (waterMeterHotState) {
      digitalWrite(LED_BUILTIN, HIGH);
      counterHot += 10;
    }

    prevWaterMeterHotState = waterMeterHotState;
  }

  if (millis() - availabilityCheckLastMillis > 30000) {
    availabilityCheckLastMillis = millis();
    mqtt.publish("rasp2mqtt/0x0000000000000002/availability", "{\"state\":\"online\"}");
  }

  if (millis() - valuesSendLastMillis > 3000) {
    valuesSendLastMillis = millis();

    if (counterCold != 0 || counterHot != 0) {
      mqtt.publish("rasp2mqtt/0x0000000000000002", "{\"water_cold\": " + String(counterCold) + ", \"water_hot\": " + String(counterHot) + "}");
      mqtt.publish("rasp2mqtt/0x0000000000000002", "{\"water_cold\": 0, \"water_hot\": 0}");
    }

    counterCold = counterHot = 0;
  }

  // Every 1 hour re-register device
  if (millis() - reregisterLastMillis > 3600000) {
    reregisterLastMillis = millis();
    registerMQTTDevice();
  }

  delay(5);
  digitalWrite(LED_BUILTIN, LOW);
}
