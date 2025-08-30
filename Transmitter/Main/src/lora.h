#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E32.h>

#include <ArduinoJson.h>
#include "sensors.h"

// Definições de pinos para conexao com E32
#define RX_LoRa 3 // GPIO3 (RXD) 
#define TX_LoRa 1 // GPIO1 (TXD)

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
