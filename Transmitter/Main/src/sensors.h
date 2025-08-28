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

// Estrutura para armazenar dados dos sensores
struct SensorData {;
    float temperature;
    int heart_rate;
    int oxygen_level;
};

class SensorManager {
public:
    SensorManager();
    bool initSensors();
    SensorData readSensors();
    
    // Callback para detecção de batimento cardíaco
    static void onBeatDetected();
    
private:
    PulseOximeter pox;              // Instância do oxímetro MAX30100
    esp_adc_cal_characteristics_t adc_chars;  // Características de calibração do ADC
    uint32_t tsLastReport;          // Timestamp do último relatório
    
    float readTemperature();        // Lê o sensor de temperatura LM35
    String getCurrentTimestamp();   // Obtém o timestamp atual
    bool validateSensorData(const SensorData &data);  // Valida os dados dos sensores
};

#endif
