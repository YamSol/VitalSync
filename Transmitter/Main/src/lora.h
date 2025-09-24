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
#define GATEWAY_ADDH 0x00    // Endereço alto do Gateway
#define GATEWAY_ADDL 0x01    // Endereço baixo do Gateway (0x0001)
#define CHANNEL 0x17         // Canal 23 (0x17 em hex)

// Endereço do próprio Transmitter (único para cada device)
#define TRANSMITTER_ADDH 0x00  // Endereço alto do Transmitter
#define TRANSMITTER_ADDL 0x02  // Endereço baixo do Transmitter (0x0002)

class LoRaManager {
private:
    HardwareSerial loraHardwareSerial;
    LoRa_E32 e32ttl;
    bool isInitialized;
    bool isPowered;
    
public:
    LoRaManager();
    bool initLoRa();
    void powerOn();
    void powerOff();
    bool sendAllData();  // Novo método integrado com WebSocket
    
    // Métodos mantidos para compatibilidade
    bool sendSensorData(const SensorData &data);
    void shutdownLoRa();
    
    // Estado do LoRa
    bool isReady() const { return isInitialized && isPowered; }
    
private:
    void configureLoRaModule();
    void printConfiguration();
    String createJSON(const SensorData &data);
    String createCompactPayload();  // Novo método para payload otimizado
    bool sendMessage(const String &message);
    int getRSSI();  // Simulated RSSI reading
    float getSNR(); // Simulated SNR reading
};

#endif
