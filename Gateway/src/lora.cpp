#include "lora.h"

LoRaReceiver::LoRaReceiver() : serialLoRa(2), e32ttl(&serialLoRa), isInitialized(false) {
    // Construtor
}

bool LoRaReceiver::initLoRa() {
    Serial.println("Inicializando módulo LoRa E32 (Gateway)...");
    
    // Inicializa Serial para comunicação com E32 (igual ao código funcional)
    serialLoRa.begin(9600, SERIAL_8N1, RXD2, TXD2); // RX=16, TX=17
    
    // Inicializa o módulo E32
    e32ttl.begin();
    
    delay(1000); // Aguarda estabilização
    
    isInitialized = true;
    Serial.println("Módulo LoRa E32 inicializado com sucesso!");
    Serial.println("Gateway pronto para receber dados...");
    
    return true;
}

ReceivedData LoRaReceiver::listenForData() {
    ReceivedData emptyData = {"", 0, 0, 0, 0, 0.0, false};
    
    if (!isInitialized) {
        Serial.println("ERRO: Módulo LoRa não inicializado!");
        return emptyData;
    }
    
    // Verifica se há dados disponíveis (igual ao código funcional)
    if (e32ttl.available() > 1) {
        Serial.println("\n[GATEWAY] Dados recebidos via LoRa!");
        
        // Recebe a mensagem
        ResponseContainer rc = e32ttl.receiveMessage();
        
        Serial.print("Status da recepção: ");
        Serial.println(rc.status.getResponseDescription());
        
        if (rc.status.code == 1) { // Sucesso
            Serial.print("Dados brutos recebidos: ");
            Serial.println(rc.data);
            
            // Faz parse do JSON
            ReceivedData parsedData = parseJSON(rc.data);
            
            if (parsedData.isValid) {
                Serial.println("✅ Dados válidos recebidos!");
                Serial.println("Device ID: " + parsedData.device_id);
                Serial.println("Heart Rate: " + String(parsedData.heart_rate) + " BPM");
                Serial.println("Oxygen Level: " + String(parsedData.oxygen_level) + "%");
                Serial.println("Blood Pressure: " + String(parsedData.systolic_pressure) + "/" + String(parsedData.diastolic_pressure));
                Serial.println("Temperature: " + String(parsedData.temperature) + "°C");
                
                return parsedData;
            } else {
                Serial.println("❌ Erro no parse dos dados recebidos!");
            }
        } else {
            Serial.println("❌ Erro na recepção. Código: " + String(rc.status.code));
        }
    }
    
    return emptyData;
}

ReceivedData LoRaReceiver::parseJSON(const String &jsonData) {
    ReceivedData data = {"", 0, 0, 0, 0, 0.0, false};
    
    Serial.println("Fazendo parse do JSON recebido...");
    
    // Cria documento JSON para parse
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonData);
    
    if (error) {
        Serial.println("Erro no parse JSON: " + String(error.c_str()));
        return data;
    }
    
    // Extrai dados (formato compacto do transmitter)
    if (doc.containsKey("id")) data.device_id = doc["id"].as<String>();
    if (doc.containsKey("hr")) data.heart_rate = doc["hr"];
    if (doc.containsKey("ox")) data.oxygen_level = doc["ox"];
    if (doc.containsKey("ps")) data.systolic_pressure = doc["ps"];
    if (doc.containsKey("pd")) data.diastolic_pressure = doc["pd"];
    if (doc.containsKey("temp")) data.temperature = doc["temp"];
    
    // Converte device_id abreviado para completo
    if (data.device_id == "T001") {
        data.device_id = "ESP32_TRANSMITTER_001";
    }
    
    // Valida os dados
    data.isValid = validateData(data);
    
    if (data.isValid) {
        Serial.println("Parse JSON realizado com sucesso!");
    } else {
        Serial.println("Dados inválidos após parse JSON!");
    }
    
    return data;
}

bool LoRaReceiver::validateData(const ReceivedData &data) {
    // Validações básicas
    if (data.device_id.length() == 0) {
        Serial.println("Validação falhou: device_id vazio");
        return false;
    }
    
    if (data.heart_rate < 30 || data.heart_rate > 200) {
        Serial.println("Validação falhou: heart_rate fora do range (30-200)");
        return false;
    }
    
    if (data.oxygen_level < 70 || data.oxygen_level > 100) {
        Serial.println("Validação falhou: oxygen_level fora do range (70-100)");
        return false;
    }
    
    if (data.systolic_pressure < 70 || data.systolic_pressure > 250) {
        Serial.println("Validação falhou: systolic_pressure fora do range (70-250)");
        return false;
    }
    
    if (data.diastolic_pressure < 40 || data.diastolic_pressure > 150) {
        Serial.println("Validação falhou: diastolic_pressure fora do range (40-150)");
        return false;
    }
    
    if (data.temperature < 30.0 || data.temperature > 45.0) {
        Serial.println("Validação falhou: temperature fora do range (30-45°C)");
        return false;
    }
    
    return true;
}
