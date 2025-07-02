#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E32.h>
#include <ArduinoJson.h>

// Definições de pinos para E32
#define RXD2 16
#define TXD2 17

// Estrutura para dados recebidos
struct ReceivedData {
    String device_id;
    int heart_rate;
    int oxygen_level;
    int systolic_pressure;
    int diastolic_pressure;
    float temperature;
    bool isValid;
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
    
private:
    ReceivedData parseJSON(const String &jsonData);
    bool validateData(const ReceivedData &data);
};

#endif
