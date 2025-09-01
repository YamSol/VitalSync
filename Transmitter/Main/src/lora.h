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

// Definições de comunicação LoRa
#define GATEWAY_ADDH 0xFF    // Endereço alto do Gateway
#define GATEWAY_ADDL 0xFF    // Endereço baixo do Gateway (0xFFFF)
#define CHANNEL 0x17         // Canal 23 (0x17 em hex)

// Endereço do próprio Transmitter (único para cada device)
#define TRANSMITTER_ADDH 0x00  // Endereço alto do Transmitter
#define TRANSMITTER_ADDL 0x01  // Endereço baixo do Transmitter (0x0001)


class LoRaManager {
private:
    HardwareSerial loraHardwareSerial;
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
