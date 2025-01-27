#include "Arduino.h"
#include <WiFi.h>
#include <MQTT.h>

#include "secrets.h"

char ssid[] = WLAN_SSID;    // your network SSID (name)
char pass[] = WLAN_PASS;    // your network password (use for WPA, or use as key for WEP)

int PIN_WATER_METER_0 = 16;
int prevWaterMeterState0 = 0;
int currentWaterMeterState0 = 0;
int counter0 = 0;
int prevCounter0 = 0;

int PIN_WATER_METER_1 = 17;
int prevWaterMeterState1 = 0;
int currentWaterMeterState1 = 0;
int counter1 = 0;
int prevCounter1 = 0;

unsigned long availabilityCheckLastMillis = 0;
unsigned long valuesSendLastMillis = 0;
unsigned long reregisterLastMillis = 0;

WiFiClient wifi;
MQTTClient mqtt;

void connectToMQTT() {
  Serial.print("checking wifi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");

  // This should be different for every device
  while (!mqtt.connect("wc2sensor", "wildcard", "")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");
  mqtt.subscribe(MQTT_TOPIC_SET_VALUE_1);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  counter0 = payload.toInt();
}

void registerMQTTDevice() {
  mqtt.publish("homeassistant/sensor/0x0000000000000002/water_meter_hot/config", "{\"availability\": [{\"topic\": \"rasp2mqtt/0x0000000000000002/availability\", \"value_template\": \"{{ value_json.state }}\"}],\"availability_mode\": \"all\",\"device\": {\"identifiers\": [\"water_meter_0x0000000000000002\"],\"manufacturer\": \"Milkov\",\"model\": \"ESP32 W Water Meter\",\"name\": \"ESP32 W Water Meter\"},\"name\": \"Water Meter Hot\",\"device_class\": \"water\",\"enabled_by_default\": false,\"entity_category\": \"diagnostic\",\"object_id\": \"0x0000000000000002_water_meter_hot\",\"state_class\": \"total\",\"state_topic\": \"rasp2mqtt/0x0000000000000002\",\"unique_id\": \"0x0000000000000002_water_meter_hot_rasp2mqtt\",\"unit_of_measurement\": \"L\",\"value_template\": \"{{ value_json.water_hot }}\"}");
  mqtt.publish("homeassistant/sensor/0x0000000000000002/water_meter_cold/config", "{\"availability\": [{\"topic\": \"rasp2mqtt/0x0000000000000002/availability\", \"value_template\": \"{{ value_json.state }}\"}],\"availability_mode\": \"all\",\"device\": {\"identifiers\": [\"water_meter_0x0000000000000002\"],\"manufacturer\": \"Milkov\",\"model\": \"ESP32 W Water Meter\",\"name\": \"ESP32 W Water Meter\"},\"name\": \"Water Meter Cold\",\"device_class\": \"water\",\"enabled_by_default\": false,\"entity_category\": \"diagnostic\",\"object_id\": \"0x0000000000000002_water_meter_cold\",\"state_class\": \"total\",\"state_topic\": \"rasp2mqtt/0x0000000000000002\",\"unique_id\": \"0x0000000000000002_water_meter_cold_rasp2mqtt\",\"unit_of_measurement\": \"L\",\"value_template\": \"{{ value_json.water_cold }}\"}");
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, pass);

  mqtt.begin(MQTT_SERVER, wifi);
  mqtt.onMessage(messageReceived);

  connectToMQTT();

  if (mqtt.connected()) {
    // Register new device
    registerMQTTDevice();
  }

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_WATER_METER_0, INPUT_PULLDOWN);
  pinMode(PIN_WATER_METER_1, INPUT_PULLDOWN);
}

void loop() {
  mqtt.loop();

  if (!mqtt.connected()) {
    connectToMQTT();
  }

  currentWaterMeterState0 = digitalRead(PIN_WATER_METER_0);
  currentWaterMeterState1 = digitalRead(PIN_WATER_METER_1);

  if (currentWaterMeterState0 != prevWaterMeterState0) {
    if (currentWaterMeterState0 == 1) {
      digitalWrite(LED_BUILTIN, HIGH);
      counter0 += 10;
    }
    prevWaterMeterState0 = currentWaterMeterState0;
  }

  if (currentWaterMeterState1 != prevWaterMeterState1) {
    if (currentWaterMeterState1 == 1) {
      digitalWrite(LED_BUILTIN, HIGH);
      counter1 += 10;
    }
    prevWaterMeterState1 = currentWaterMeterState1;
  }

  if (millis() - availabilityCheckLastMillis > 30000) {
    availabilityCheckLastMillis = millis();
    mqtt.publish("rasp2mqtt/0x0000000000000002/availability", "{\"state\":\"online\"}");
  }

  if (millis() - valuesSendLastMillis > 3000) {
    valuesSendLastMillis = millis();

    if (counter0 != 0 || counter1 != 0) {
      mqtt.publish("rasp2mqtt/0x0000000000000002", "{\"water_cold\": " + String(counter0) + ", \"water_hot\": " + String(counter1) + "}");
      mqtt.publish("rasp2mqtt/0x0000000000000002", "{\"water_cold\": 0, \"water_hot\": 0}");
    }

    counter0 = counter1 = 0;
  }

  // Every 1 hour re-register device
  if (millis() - reregisterLastMillis > 3600000) {
    reregisterLastMillis = millis();
    registerMQTTDevice();
  }

  delay(5);
  digitalWrite(LED_BUILTIN, LOW);
}
