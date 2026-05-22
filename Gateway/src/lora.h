#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E32.h>
#include <ArduinoJson.h>
#include <vector>

// Definições de pinos para E32
#define LORA_RX_PIN 16
#define LORA_TX_PIN 17
#define LORA_M0_PIN 5
#define LORA_M1_PIN 4
#define LORA_AUX_PIN 2

// Definições de comunicação LoRa
#define GATEWAY_ADDH 0x00    // Endereço alto do Gateway
#define GATEWAY_ADDL 0x01    // Endereço baixo do Gateway (0x0001)
#define CHANNEL 0x17         // Canal 23 (0x17 em hex)

// Estrutura para dados recebidos
struct ReceivedData {
    String device_id;      // Identificador único do dispositivo
    int heart_rate;
    int oxygen_level;
    float temperature;
};

class LoRaReceiver {
private:
    HardwareSerial serialLoRa;
    LoRa_E32 e32ttl;
    bool isInitialized;
    
public:
    LoRaReceiver();
    bool initLoRa();
    ReceivedData listenForData();
    std::vector<ReceivedData> listenForMultipleData();
    void printConfiguration();
    
    private:
    void configureLoRaModule();
    int parseJSON(const String &jsonData, ReceivedData &data);
    std::vector<ReceivedData> extractMultipleJSONs(const String &rawData);
};

#endif
