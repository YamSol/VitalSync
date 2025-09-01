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
    // Inicializa comunicação I2C para o MAX30100
    Wire.begin();
    Wire.setClock(100000);
    
    // Configura ADC para leitura do sensor de temperatura
    analogReadResolution(12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3300, &adc_chars);
    
    // Inicializa o oxímetro MAX30100
    // if (!pox.begin(PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT)) {
    if (!pox.begin()) {
        Serial.println("ERRO: Falha ao inicializar o oxímetro MAX30100");
        return false;
    }
    
    // Configura o oxímetro
    pox.setIRLedCurrent(MAX30100_LED_CURR_4_4MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
    
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
    SensorData data;
    
    // Atualiza o oxímetro para obter novos dados
    pox.update();
    delay(10);
    
    // Ler temperatura
    data.temperature = readTemperature();

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
    return data;
}
