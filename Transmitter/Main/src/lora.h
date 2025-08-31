#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E32.h>

#include <ArduinoJson.h>
#include "sensors.h"

// Definições de pinos para conexao com E32
#define LORA_RX_PIN 16 // GPIO3 (RX2D) 
#define LORA_TX_PIN 17 // GPIO1 (TX2D)
#define LORA_M0_PIN 5 // GPO5 (M0)
#define LORA_M1_PIN 4 // GPO4 (M1)
#define LORA_AUX_PIN 2 // GPO2 (AUX)


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
    void configureLoRaModule();
    void printConfiguration();
    String createJSON(const SensorData &data);
    bool sendMessage(const String &message);
};

#endif
