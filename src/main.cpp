#include "Arduino.h"

#include "secrets.h"
#include "connection.cpp"
#include "Device.cpp"

#define WATER_METER_COLD_PIN 16
#define WATER_METER_HOT_PIN 17

boolean prevWaterMeterColdState = 0;
boolean prevWaterMeterHotState = 0;

Device *waterMeterDevice;
Sensor *cold;
Sensor *hot;

DeviceInfo deviceInfo = {
    .manufacturer = "Milkov",
    .model = "ESP32 Water Meter",
    .name = "ESP32 Water Meter",
    .uniqueId = "0x0000000000000002"};

void setup()
{
  Serial.begin(9600);

  // Set up purpose of GPIOs.
  pinMode(WATER_METER_COLD_PIN, INPUT_PULLDOWN);
  pinMode(WATER_METER_HOT_PIN, INPUT_PULLDOWN);

  char ssid[] = WLAN_SSID;
  char pass[] = WLAN_PASS;

  // Set up Wi-Fi
  WiFi.begin(ssid, pass);

  // Set up MQTT
  mqtt.begin(MQTT_SERVER, wifi);

  // Connect to Wi-Fi and MQTT
  connectToMQTT(&deviceInfo.uniqueId[0]);

  // Сразу после того как соединение по WiFi установлено и MQTT канал доступен,
  // инициализируем девайс и сенсоры к нему. В момент инициализации происходит
  // отстрел события в MQTT.
  if (mqtt.connected())
  {
    waterMeterDevice = new Device(wifi, mqtt, deviceInfo);
    waterMeterDevice->addSensor(cold = new Sensor("Water Meter Cold"));
    waterMeterDevice->addSensor(hot = new Sensor("Water Meter Hot"));
  }
}

void loop()
{
  mqtt.loop();

  // Periodically check the status of the Wi-Fi and MQTT connections
  // and recreate them if they are not established.
  if (!mqtt.connected())
    connectToMQTT(&deviceInfo.uniqueId[0]);

  // Whether the wires of cold or hot water meters are connected.
  boolean waterMeterColdState = digitalRead(WATER_METER_COLD_PIN);
  boolean waterMeterHotState = digitalRead(WATER_METER_HOT_PIN);

  waterMeterDevice->mqttSendSensorAvailability();

  delay(500);
}
