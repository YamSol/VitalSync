#include "sensors.h"

SensorManager::SensorManager() {
    // Construtor - inicialização básica
}

bool SensorManager::initSensors() {
    Serial.println("Inicializando sensores...");
    
    // TODO: Aqui seria a inicialização real dos sensores
    // Por enquanto, simulamos uma inicialização bem-sucedida
    
    delay(1000); // Simula tempo de inicialização
    Serial.println("Sensores inicializados com sucesso!");
    return true;
}

SensorData SensorManager::readSensors() {
    Serial.println("Lendo valores dos sensores...");
    
    SensorData data;
    
    // Dados estáticos simulando leituras dos sensores
    data.device_id = "ESP32_TRANSMITTER_001";
    data.heart_rate = 72 + random(-5, 6);        // Variação de 67-77 BPM
    data.oxygen_level = 97 + random(-2, 3);      // Variação de 95-99%
    data.pressure.systolic = 120 + random(-10, 11);   // Variação de 110-130
    data.pressure.diastolic = 80 + random(-5, 6);     // Variação de 75-85
    data.temperature = 36.5 + (random(-20, 21) / 10.0); // Variação de 34.5-38.5°C
    data.timestamp = getCurrentTimestamp();
    
    // Valida os dados antes de retornar
    if (validateSensorData(data)) {
        Serial.println("Dados dos sensores válidos!");
        Serial.println("Heart Rate: " + String(data.heart_rate) + " BPM");
        Serial.println("Oxygen Level: " + String(data.oxygen_level) + "%");
        Serial.println("Blood Pressure: " + String(data.pressure.systolic) + "/" + String(data.pressure.diastolic));
        Serial.println("Temperature: " + String(data.temperature) + "°C");
    } else {
        Serial.println("ERRO: Dados dos sensores inválidos!");
    }
    
    return data;
}

String SensorManager::getCurrentTimestamp() {
    // Por enquanto retorna um timestamp fixo
    // TODO: Implementar RTC ou sincronização com NTP
    return "2025-01-02T" + String(millis() / 1000) + ":00:00";
}

bool SensorManager::validateSensorData(const SensorData &data) {
    // Validação básica dos dados dos sensores
    if (data.heart_rate < 40 || data.heart_rate > 200) return false;
    if (data.oxygen_level < 70 || data.oxygen_level > 100) return false;
    if (data.temperature < 30.0 || data.temperature > 45.0) return false;
    if (data.pressure.systolic < 70 || data.pressure.systolic > 200) return false;
    if (data.pressure.diastolic < 40 || data.pressure.diastolic > 120) return false;
    
    return true;
}
