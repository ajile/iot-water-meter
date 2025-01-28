#include <vector>
#include <WiFi.h>
#include <MQTT.h>
#include <nlohmann/json.hpp>

#include "Sensor.cpp"

struct DeviceInfo
{
  std::string manufacturer;
  std::string model;
  std::string name;
  std::string uniqueId;
};

class Device
{
public:
  Device(WiFiClient wifi, MQTTClient mqtt, DeviceInfo deviceInfo) : wifi(wifi), mqtt(mqtt), deviceInfo(deviceInfo) {}

  /**
   * Добавляет новый сенсор в массив сенсоров.
   */
  void addSensor(Sensor *sensor)
  {
    // Добавляем новый сенсор в массив…
    this->sensors.push_back(sensor);

    // …и тут же его регистрируем.
    this->mqttRegisterSensor(sensor);
  }

  /**
   * Отправляет в MQTT сообщение о регистрации девайса и одного сенсора.
   */
  void mqttRegisterSensor(Sensor *sensor)
  {

    std::string name = "Alice";
    int age = 30;

    nlohmann::json availabilityJson = {
        {"topic", this->getTopicNameAvailability()},
        {"value_template", "{{ value_json.state }}}"},
    };

    nlohmann::json deviceJson = {
        {"identifiers", {"water_meter_" + this->deviceInfo.uniqueId}},
        {"manufacturer", this->deviceInfo.manufacturer},
        {"model", this->deviceInfo.model},
        {"name", this->deviceInfo.name},
    };

    nlohmann::json json = {
        {"availability", {availabilityJson}},
        {"device", deviceJson},
        {"name", "Water Meter Hot"},
        {"device_class", "water"},
        {"enabled_by_default", false},
        {"entity_category", "diagnostic"},
        {"object_id", "0x0000000000000002_water_meter_hot"},
        {"state_class", "total"},
        {"state_topic", "rasp2mqtt/0x0000000000000002"},
        {"unique_id", "0x0000000000000002_water_meter_hot_rasp2mqtt"},
        {"unit_of_measurement", "L"},
        {"value_template", "{{ value_json.water_hot }}"},
        {"availability_mode", "all"},
    };

    Serial.println(&json.dump()[0]);

    // this->mqtt.publish("homeassistant/sensor/0x0000000000000002/water_meter_hot/config", &json.dump()[0]);
  }

  /**
   * Отправляет в MQTT сообщение о регистрации девайса и одного сенсора.
   */
  void mqttRegisterSensors()
  {
    for (Sensor *sensor : this->sensors)
    {
      this->mqttRegisterSensor(sensor);
    }
  }

  void mqttSendSensorValue(Sensor *sensor)
  {
    Serial.println("mqttSendSensorValue");
  }

  // Отправляет в MQTT новые значения всех сенсоров
  void mqttSendSensorValues()
  {
    for (Sensor *sensor : this->sensors)
    {
      this->mqttSendSensorValue(sensor);
    }
  }

  // Отправляет в MQTT сигнал о доступности устройства
  void mqttSendSensorAvailability()
  {
    nlohmann::json json = {
        {"state", "online"},
    };

    Serial.println(&this->getTopicNameAvailability()[0]);
    Serial.println(&json.dump()[0]);

    // this->mqtt.publish(&this->getTopicNameAvailability()[0], &json.dump()[0]);
  }

  std::string getTopicNameAvailability()
  {
    return "rasp2mqtt/" + this->deviceInfo.uniqueId + "/availability";
  }

private:
  WiFiClient wifi;
  MQTTClient mqtt;
  DeviceInfo deviceInfo;
  std::vector<Sensor *> sensors;
};
