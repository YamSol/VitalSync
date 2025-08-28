#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E32.h>

#include <ArduinoJson.h>
#include "sensors.h"

// Definições de pinos para E32
#define RXD2 16
#define TXD2 17

class LoRaManager {
private:
    HardwareSerial serialLoRa;
    LoRa_E32 e32ttl;
    bool isInitialized;
    
public:
    LoRaManager();
    bool initLoRa();
    bool sendSensorData(const SensorData &data);
    void shutdownLoRa();
    
private:
    String createJSON(const SensorData &data);
    bool sendMessage(const String &message);
};

#endif
