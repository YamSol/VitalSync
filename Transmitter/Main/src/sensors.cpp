#include "sensors.h"

// Variável global estática para callback
static SensorManager* instance = nullptr;

// Callback para detecção de batimento cardíaco
void SensorManager::onBeatDetected() {
    Serial.println("Batimento cardíaco detectado!");
}

SensorManager::SensorManager() : tsLastReport(0) {
    // Construtor - inicialização básica
    instance = this; // Armazena a instância para uso no callback
}

bool SensorManager::initSensors() {
    Serial.println("Inicializando sensores...");
    
    // Inicializa comunicação I2C para o MAX30100
    Wire.begin();
    Wire.setClock(100000);
    
    // Configura ADC para leitura do sensor de temperatura
    analogReadResolution(12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3300, &adc_chars);
    
    // Inicializa o oxímetro MAX30100
    if (!pox.begin(PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT)) {
        Serial.println("ERRO: Falha ao inicializar o oxímetro MAX30100");
        return false;
    }
    
    // Configura o oxímetro
    pox.setIRLedCurrent(MAX30100_LED_CURR_4_4MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    
    Serial.println("Sensores inicializados com sucesso!");
    return true;
}

float SensorManager::readTemperature() {
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
    
    return temperaturaC;
}

SensorData SensorManager::readSensors() {
    Serial.println("Lendo valores dos sensores...");
    
    SensorData data;
    
    // Atualiza o oxímetro por algum tempo para obter leituras precisas
    uint32_t startTime = millis();
    while (millis() - startTime < REPORTING_PERIOD_MS) {
        pox.update();
        delay(10);
    }
    
    // Lê dados dos sensores reais
    data.heart_rate = (int)pox.getHeartRate();
    data.oxygen_level = (int)pox.getSpO2();
    data.temperature = readTemperature();
    
    // Pressão arterial ainda é simulada (precisaria de outro sensor)

    
    // Valida os dados antes de retornar
    if (validateSensorData(data)) {
        Serial.println("Dados dos sensores válidos!");
        Serial.println("Heart Rate: " + String(data.heart_rate) + " BPM");
        Serial.println("Oxygen Level: " + String(data.oxygen_level) + "%");
        Serial.println("Temperature: " + String(data.temperature) + "°C");
        
        // Reset do oxímetro para próxima leitura
        pox.shutdown();
        delay(10);
        pox.resume();
    } else {
        Serial.println("ERRO: Dados dos sensores inválidos!");
    }
    
    return data;
}

bool SensorManager::validateSensorData(const SensorData &data) {
    if (data.oxygen_level < 90 || data.oxygen_level > 100) return false;
    if (data.temperature < 33.0 || data.temperature > 40.0) return false;
    // não valida a frequência cardíaca
    return true;
}

String SensorManager::getCurrentTimestamp() {
    // Por enquanto retorna um timestamp fixo baseado no tempo desde a inicialização
    // Em um projeto real, poderíamos usar RTC ou NTP para obter a data/hora real
    return "T" + String(millis() / 1000);
}
