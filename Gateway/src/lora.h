#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E32.h>
#include <ArduinoJson.h>

// Definições de pinos para E32
#define Lora_rx_pin 16
#define Lora_tx_pin 17
#define Lora_m0_pin 5
#define Lora_m1_pin 4
#define Lora_aux_pin 2

// Estrutura para dados recebidos
struct ReceivedData {
    float temperature;
    int heart_rate;
    int oxygen_level;
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
    void printConfiguration(); // Novo método para debug
    
    private:
    void configureLoRaModule();
    ReceivedData parseJSON(const String &jsonData);
    bool validateData(const ReceivedData &data);
};

#endif
