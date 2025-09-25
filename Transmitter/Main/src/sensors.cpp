#include "sensors.h"
#include "websocket_manager.h"
#include <WiFi.h>

// Variável global estática para callback
static SensorManager* instance = nullptr;

// Callback para detecção de batimento cardíaco
void SensorManager::onBeatDetected() {
    Serial.println("Batimento cardíaco detectado!");
}

SensorManager::SensorManager() : tsLastReport(0), sensors_initialized(false) {
    // Construtor - inicialização básica
    instance = this; // Armazena a instância para uso no callback
}

bool SensorManager::initSensors() {
    // Configurar apenas pino de controle do LoRa
    pinMode(LORA_POWER_PIN, OUTPUT);
    digitalWrite(LORA_POWER_PIN, LOW);    // LoRa desligado inicialmente
    
    // Inicializa comunicação I2C para o MAX30100
    Wire.begin();
    Wire.setClock(100000);
    
    // Configura ADC para leitura do sensor de temperatura
    analogReadResolution(12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3300, &adc_chars);
    
    // Inicializa o oxímetro MAX30100 imediatamente (sensores sempre ligados)
    if (!pox.begin()) {
        Serial.println("[SENSORS] ERRO: Falha ao inicializar o oxímetro MAX30100");
        return false;
    }
    
    // Configura o oxímetro
    pox.setIRLedCurrent(MAX30100_LED_CURR_4_4MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    
    sensors_initialized = true;
    Serial.println("[SENSORS] Sensores inicializados e prontos");
    
    return true;
}


bool SensorManager::readOximeterSequence(int count) {
    if (!sensors_initialized) {
        Serial.println("[SENSORS] ERRO: Sensores não inicializados");
        wsManager.reportError("SENSOR_NOT_POWERED", "Sensores desligados");
        return false;
    }
    
    Serial.printf("[SENSORS] Iniciando leitura de %d amostras do oxímetro\n", count);
    
    
    TimestampedOxiData* buffer = wsManager.getOxiBuffer();
    const SystemConfig& config = wsManager.getConfig();
    
    int successful_readings = 0;
    
    for (int i = 0; i < count; i++) {
        uint32_t start_time = millis();
        bool reading_valid = false;
        int attempts = 0;
        const int max_attempts = 5;
        
        while (!reading_valid && attempts < max_attempts) {
            pox.update();
            delay(config.oxi_interval_ms);
            
            uint8_t hr = (uint8_t)pox.getHeartRate();
            uint8_t spo2 = (uint8_t)pox.getSpO2();

            Serial.printf("[DEBUG][SENSORS] Amostra %d: HR=%d BPM, SpO2=%d%%\n", i + 1, hr, spo2);

            // Validar leitura
            if (hr >= 40 && hr <= 200 && spo2 >= 70 && spo2 <= 100) {
                buffer[successful_readings].timestamp = millis();
                buffer[successful_readings].bpm = hr; 
                buffer[successful_readings].spo2 = spo2;
                buffer[successful_readings].valid = true;
                
                reading_valid = true;
                successful_readings++;
            } else {
                attempts++;
                delay(20);
            }
        }
        
        if (!reading_valid) {
            Serial.printf("[SENSORS] TIMEOUT na amostra %d do oxímetro\n", i + 1);
            // Continua mesmo com falha para não abortar toda a sequência
        }
        
        // Reportar progresso
        wsManager.reportOximeterProgress(i + 1, count);
        
        // Pequena pausa entre leituras
        delay(10);
    }
    
    Serial.printf("[SENSORS] Leitura oxímetro concluída: %d/%d amostras válidas\n", successful_readings, count);
    
    wsManager.setOxiCount(successful_readings);
    wsManager.reportOximeterDone(successful_readings);
    
    return successful_readings > 0;
}

bool SensorManager::readTemperatureSequence(int count) {
    if (!sensors_initialized) {
        Serial.println("[SENSORS] ERRO: Sensores não inicializados");
        wsManager.reportError("SENSOR_NOT_INITIALIZED", "Sensores não inicializados");
        return false;
    }
    
    Serial.printf("[SENSORS] Iniciando leitura de %d amostras de temperatura\n", count);
    
    TimestampedTempData* buffer = wsManager.getTempBuffer();
    const SystemConfig& config = wsManager.getConfig();
    
    int successful_readings = 0;
    
    for (int i = 0; i < count; i++) {
        // Média de 10 leituras com calibração para melhor precisão
        uint32_t soma = 0;
        bool reading_valid = true;
        
        for (int j = 0; j < 10; j++) {
            uint32_t reading = analogRead(TEMP_SENSOR_PIN);
            if (reading == 0 || reading >= 4095) {
                reading_valid = false;
                break;
            }
            soma += reading;
            delay(2);
        }
        
        if (reading_valid) {
            float leituraADC = soma / 10.0;
            
            // Converte para tensão e temperatura
            uint32_t voltage_mV = esp_adc_cal_raw_to_voltage((uint32_t)leituraADC, &adc_chars);
            float tensao = voltage_mV / 1000.0;
            float temperaturaC = tensao * 100; // Para LM35
            buffer[successful_readings].timestamp = millis();
            buffer[successful_readings].temperature = temperaturaC;
            buffer[successful_readings].valid = true;
            successful_readings++;
            Serial.printf("[SENSORS] Amostra %d: %.2f °C\n", i + 1, temperaturaC);
        } else {
            Serial.printf("[SENSORS] Falha na leitura ADC da amostra %d\n", i + 1);
        }
        
        // Reportar progresso
        wsManager.reportTemperatureProgress(i + 1, count);
        
        delay(config.temp_interval_ms);
    }
    
    Serial.printf("[SENSORS] Leitura temperatura concluída: %d/%d amostras válidas\n", successful_readings, count);
    
    wsManager.setTempCount(successful_readings);
    wsManager.reportTemperatureDone(successful_readings);
    
    return successful_readings > 0;
}

// Métodos mantidos para compatibilidade
void SensorManager::readTemperature(SensorData &data) {
    // Média de 10 leituras com calibração para melhor precisão
    uint32_t soma = 0;
    for (int i = 0; i < 10; i++) {
        soma += analogRead(TEMP_SENSOR_PIN);
        delay(10);
    }
    float leituraADC = soma / 10.0;
    
    // Converte para tensão e temperatura
    uint32_t voltage_mV = esp_adc_cal_raw_to_voltage((uint32_t)leituraADC, &adc_chars);
    float tensao = voltage_mV / 1000.0;
    float temperaturaC = tensao * 100; // Para LM35 (para TMP36 seria: (tensao - 0.5) * 100)

    data.temperature = temperaturaC;
}

void SensorManager::readOximeter(SensorData &data) {
    // Atualiza o oxímetro para obter novos dados
    pox.update();
    delay(10);

    // Ler frequência cardíaca e nível de oxigênio
    data.heart_rate = (int)pox.getHeartRate();
    data.oxygen_level = (int)pox.getSpO2();

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        
        // Atualiza o timestamp do último relatório
        tsLastReport = millis();
        
        // Reset do oxímetro para próxima leitura
        pox.shutdown();
        delay(10);
        pox.resume();
    }
}

bool SensorManager::validateSensorData(const SensorData &data) {
    // Validação básica dos dados
    if (data.temperature < 30.0 || data.temperature > 45.0) return false;
    if (data.heart_rate < 40 || data.heart_rate > 200) return false;
    if (data.oxygen_level < 70 || data.oxygen_level > 100) return false;
    
    return true;
}

