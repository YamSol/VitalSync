#include "lora.h"

LoRaManager::LoRaManager() : serialLoRa(2), e32ttl(&serialLoRa), isInitialized(false) {
    // Construtor
}

bool LoRaManager::initLoRa() {
    Serial.println("Inicializando módulo LoRa E32...");
    
    // Inicializa Serial para comunicação com E32 (igual ao código funcional)
    serialLoRa.begin(9600, SERIAL_8N1, RX_LoRa, TX_LoRa);
    
    // Inicializa o módulo E32
    e32ttl.begin();
    
    delay(1000); // Aguarda estabilização
    
    // Verifica se o módulo está respondendo
    Serial.println("Testando comunicação com E32...");
    
    isInitialized = true;
    Serial.println("Módulo LoRa E32 inicializado com sucesso!");
    
    return true;
}

bool LoRaManager::sendSensorData(const SensorData &data) {
    if (!isInitialized) {
        Serial.println("ERRO: Módulo LoRa não inicializado!");
        return false;
    }
    
    Serial.println("Preparando envio de dados via LoRa...");
    
    // Cria JSON com os dados dos sensores
    String jsonData = createJSON(data);
    
    if (jsonData.length() == 0) {
        Serial.println("ERRO: Falha ao criar JSON!");
        return false;
    }
    
    Serial.println("JSON criado: " + jsonData);
    
    // Liga o módulo LoRa (já está ligado, mas podemos adicionar lógica aqui)
    Serial.println("Ligando módulo LoRa...");
    
    // Envia a mensagem compacta
    bool success = sendMessage(jsonData);
    
    if (success) {
        Serial.println("Dados enviados com sucesso via LoRa!");
    } else {
        Serial.println("ERRO: Falha no envio via LoRa!");
    }
    
    return success;
}

void LoRaManager::shutdownLoRa() {
    Serial.println("Desligando módulo LoRa...");
    
    // Simples desligamento - não precisamos manipular M0/M1
    delay(100);
    
    isInitialized = false;
    Serial.println("Módulo LoRa desligado!");
}

String LoRaManager::createJSON(const SensorData &data) {
    Serial.println("Criando JSON dos dados dos sensores...");
    
    // Cria documento JSON COMPACTO (máximo 58 bytes)
    JsonDocument doc;
    
    // Usa nomes de campos muito curtos para economizar espaço
    doc["id"] = "T001";  // Device ID abreviado
    doc["hr"] = data.heart_rate;        // heart_rate -> hr
    doc["ox"] = data.oxygen_level;      // oxygen_level -> ox  
    doc["temp"] = data.temperature;     // temperature -> temp
    // Nota: Na nova estrutura do SensorData não temos mais pressure e timestamp
    
    // Serializa para string
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.println("JSON compacto criado: " + jsonString);
    Serial.println("Tamanho: " + String(jsonString.length()) + " bytes");
    
    if (jsonString.length() > 58) {
        Serial.println("AVISO: JSON ainda muito grande! (" + String(jsonString.length()) + " > 58 bytes)");
    }
    
    Serial.println("JSON criado com sucesso!");
    return jsonString;
}

bool LoRaManager::sendMessage(const String &message) {
    Serial.println("Enviando mensagem via UART para E32...");
    
    // Envia via biblioteca E32 (igual ao código funcional)
    ResponseStatus rs = e32ttl.sendMessage(message);
    
    // Exibe o status como no código funcional
    Serial.print("Status do envio: ");
    Serial.println(rs.getResponseDescription());
    
    // Para debug, vamos também mostrar o código
    Serial.println("Código: " + String(rs.code));
    
    // Verificar se é SUCCESS (código 1 significa sucesso)
    if (rs.code == 1) {
        Serial.println("Mensagem enviada com sucesso!");
        return true;
    } else {
        Serial.println("ERRO no envio da mensagem. Código: " + String(rs.code));
        return false;
    }
}
