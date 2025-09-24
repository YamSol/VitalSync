#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Verifica se estamos usando ESP32 para incluir bibliotecas específicas
#if defined(ESP32)
  #include <Wire.h>
  #include <esp_adc_cal.h>
#endif

// Inclui a biblioteca do oxímetro
#include "MAX30100_PulseOximeter.h"

// Definições para os sensores
#define TEMP_SENSOR_PIN 36       // Pino para o sensor de temperatura LM35
#define REPORTING_PERIOD_MS 2000 // Período para relatar os dados

// Pino de controle de energia apenas para LoRa
#define LORA_POWER_PIN 22        // Pino para controlar alimentação do LoRa

// ID único do transmitter
#define TRANSMITTER_ID "TR-001"

// Estrutura para armazenar dados dos sensores (compatibilidade)
struct SensorData {
    float temperature;
    int heart_rate;
    int oxygen_level;
};

class SensorManager {
public:
    SensorManager();
    bool initSensors();
    
    // Métodos de leitura integrados com WebSocket
    bool readOximeterSequence(int count);
    bool readTemperatureSequence(int count);
    
    // Métodos de leitura individuais (mantidos para compatibilidade)
    void readOximeter(SensorData &data);
    void readTemperature(SensorData &data);
    
    // Callback para detecção de batimento cardíaco
    static void onBeatDetected();
    
    // Estado dos sensores
    bool isInitialized() const { return sensors_initialized; }
    
private:
    PulseOximeter pox;              // Instância do oxímetro MAX30100
    esp_adc_cal_characteristics_t adc_chars;  // Características de calibração do ADC
    uint32_t tsLastReport;          // Timestamp do último relatório
    bool sensors_initialized;
    
    bool validateSensorData(const SensorData &data);  // Valida os dados dos sensores
    bool stabilizeOximeter();       // Aguarda estabilização do oxímetro
};

#endif
