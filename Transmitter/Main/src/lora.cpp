#include "lora.h"
#include "websocket_manager.h"

LoRaManager::LoRaManager() : loraHardwareSerial(2), e32ttl(&loraHardwareSerial, LORA_AUX_PIN, LORA_M0_PIN, LORA_M1_PIN), isInitialized(false), isPowered(false) {
    // Construtor
}

void LoRaManager::powerOn() {
    // Ligar alimentação do LoRa
    digitalWrite(LORA_POWER_PIN, HIGH);
    delay(100); // Aguarda estabilização
    
    Serial.println("[LORA] Ligando módulo LoRa...");
    
    // Inicializar comunicação serial e módulo
    loraHardwareSerial.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    delay(100);
    
    e32ttl.begin();
    delay(200);
    
    configureLoRaModule();
    printConfiguration();
    
    isPowered = true;
    isInitialized = true;
    
    Serial.println("[LORA] Módulo LoRa inicializado e pronto");
}

void LoRaManager::powerOff() {
    if (isPowered) {
        digitalWrite(LORA_POWER_PIN, LOW);
        isPowered = false;
        isInitialized = false;
        Serial.println("[LORA] Módulo LoRa desligado");
    }
}

bool LoRaManager::sendAllData() {
    if (!isReady()) {
        Serial.println("[LORA] ERRO: Módulo não está pronto");
        wsManager.reportError("LORA_NOT_READY", "Módulo LoRa não inicializado");
        return false;
    }
    
    Serial.println("[LORA] Iniciando envio de dados...");
    
    // Criar payload compacto com todos os dados
    String payload = createCompactPayload();
    
    if (payload.length() == 0) {
        wsManager.reportError("LORA_PAYLOAD_ERROR", "Falha ao criar payload");
        return false;
    }
    
    if (payload.length() > 58) {
        Serial.printf("[LORA] ERRO: Payload muito grande (%d bytes)\n", payload.length());
        wsManager.reportError("LORA_PAYLOAD_SIZE", "Payload excede limite de 58 bytes");
        return false;
    }
    
    Serial.printf("[LORA] Enviando payload (%d bytes): %s\n", payload.length(), payload.c_str());
    
    // Tentar envio com retry
    bool success = false;
    int attempts = 2;
    
    for (int i = 0; i < attempts && !success; i++) {
        if (i > 0) {
            Serial.printf("[LORA] Tentativa %d/%d\n", i + 1, attempts);
            delay(1000);
        }
        
        ResponseStatus rs = e32ttl.sendFixedMessage(GATEWAY_ADDH, GATEWAY_ADDL, CHANNEL, payload);
        success = (rs.code == 1);
        
        if (!success) {
            Serial.printf("[LORA] Falha no envio: %s\n", rs.getResponseDescription().c_str());
        }
    }
    
    // Simular métricas (em um sistema real, isso viria do módulo LoRa)
    int rssi = getRSSI();
    float snr = getSNR();
    
    if (success) {
        Serial.printf("[LORA] Dados enviados com sucesso! RSSI: %d, SNR: %.1f\n", rssi, snr);
    } else {
        Serial.println("[LORA] Falha no envio após todas as tentativas");
    }
    
    // Reportar resultado
    wsManager.reportLoRaResult(success, rssi, snr);
    
    return success;
}

String LoRaManager::createCompactPayload() {
    TimestampedOxiData* oxiBuffer = wsManager.getOxiBuffer();
    TimestampedTempData* tempBuffer = wsManager.getTempBuffer();
    int oxiCount = wsManager.getOxiCount();
    int tempCount = wsManager.getTempCount();
    
    if (oxiCount == 0 && tempCount == 0) {
        Serial.println("[LORA] ERRO: Nenhum dado para enviar");
        return "";
    }
    
    // Criar payload JSON otimizado
    DynamicJsonDocument doc(1024);
    
    // Metadados
    doc["id"] = TRANSMITTER_ID;
    doc["v"] = 1; // versão do payload
    doc["ts"] = millis();
    
    // Dados do oxímetro (apenas últimas amostras válidas para economizar espaço)
    if (oxiCount > 0) {
        JsonArray oxi = doc.createNestedArray("o");
        
        // Enviar no máximo as últimas 5 amostras válidas
        int start = max(0, oxiCount - 5);
        for (int i = start; i < oxiCount; i++) {
            if (oxiBuffer[i].valid) {
                JsonArray sample = oxi.createNestedArray();
                sample.add(oxiBuffer[i].bpm);
                sample.add(oxiBuffer[i].spo2);
                // Omitir IR/RED para economizar espaço
            }
        }
    }
    
    // Dados de temperatura (média das últimas 10 amostras)
    if (tempCount > 0) {
        float tempSum = 0;
        int validCount = 0;
        
        int start = max(0, tempCount - 10);
        for (int i = start; i < tempCount; i++) {
            if (tempBuffer[i].valid) {
                tempSum += tempBuffer[i].temperature;
                validCount++;
            }
        }
        
        if (validCount > 0) {
            doc["t"] = round((tempSum / validCount) * 10) / 10.0; // 1 decimal
        }
    }
    
    // Serializar
    String payload;
    serializeJson(doc, payload);
    
    Serial.printf("[LORA] Payload criado: %s (%d bytes)\n", payload.c_str(), payload.length());
    return payload;
}

int LoRaManager::getRSSI() {
    // Em um sistema real, isso viria do módulo LoRa
    // Por agora, simular valores típicos
    return random(-120, -70);
}

float LoRaManager::getSNR() {
    // Em um sistema real, isso viria do módulo LoRa
    // Por agora, simular valores típicos
    return random(-10, 15) + (random(0, 10) / 10.0);
}

// Método legado mantido para compatibilidade
bool LoRaManager::initLoRa() {
    powerOn();
    return isReady();
}

bool LoRaManager::sendSensorData(const SensorData &data) {
    if (!isInitialized) {
        Serial.println("Nao foi possível enviar dados");
        Serial.println("ERRO: Módulo LoRa não inicializado!");
        return false;
    }
    
    // Cria JSON com os dados dos sensores
    String jsonData = createJSON(data);
    
    if (jsonData.length() == 0) {
        return false;
    }
    Serial.println("JSON criado: " + jsonData);
    
    bool success = sendMessage(jsonData);    
    return success;
}

void LoRaManager::shutdownLoRa() {
    powerOff();
}

String LoRaManager::createJSON(const SensorData &data) {
    // Cria documento JSON COMPACTO (máximo 58 bytes)
    JsonDocument doc;
    
    doc["id"] = TRANSMITTER_ID;         // ID do transmitter definido no header
    doc["hr"] = data.heart_rate;        // heart_rate -> hr
    doc["ox"] = data.oxygen_level;      // oxygen_level -> ox  
    doc["temp"] = data.temperature;     // temperature -> temp
    
    // Serializa para string
    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("JSON compacto criado: " + jsonString + " (" + String(jsonString.length()) + " bytes)");
    return jsonString;
}

bool LoRaManager::sendMessage(const String &message) {
    if (message.length() > 58) {
        Serial.println("ERRO: Mensagem muito longa para transmissão LoRa!");
        return false;
    }

    ResponseStatus rs = e32ttl.sendFixedMessage(GATEWAY_ADDH, GATEWAY_ADDL, CHANNEL, message);

    return (rs.code == 1);
}

void LoRaManager::configureLoRaModule()
{

    // Obtém configuração atual
    ResponseStructContainer c = e32ttl.getConfiguration();
    Configuration configuration = *(Configuration *)c.data;
    Serial.print("Status da configuração: ");
    Serial.println(c.status.getResponseDescription());

    if (c.status.code != 1) {
        Serial.println("Erro ao obter configuração atual!");
        c.close();
        return;
    }
    Serial.println("Configuração atual obtida com sucesso!");

    // Define configurações específicas
    configuration.ADDH = 0x00; // Endereço alto do Transmitter
    configuration.ADDL = 0x02; // Endereço baixo do Transmitter
    configuration.CHAN = 23;
    configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
    configuration.OPTION.transmissionPower = POWER_20;
    configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
    configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.uartParity = MODE_00_8N1;

    // Aplica as configurações
    ResponseStatus rsConfig = e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
    Serial.print("Status da aplicação da configuração: ");
    Serial.println(rsConfig.getResponseDescription());

    if (rsConfig.code == 1) {
        Serial.println("Configurações aplicadas com sucesso!");
    }
    else {
        Serial.println("Erro ao aplicar configurações!");
    }

    c.close();
}

void LoRaManager::printConfiguration() {
    ResponseStructContainer c = e32ttl.getConfiguration();
    if (c.status.code == 1) {
        Configuration configuration = *(Configuration*) c.data;
        
        Serial.println("========== CONFIGURAÇÃO ATUAL ==========");
        Serial.println("Endereço Alto (ADDH): " + String(configuration.ADDH, HEX));
        Serial.println("Endereço Baixo (ADDL): " + String(configuration.ADDL, HEX));
        Serial.println("Canal (CHAN): " + String(configuration.CHAN));
        
        Serial.print("Taxa de dados do ar: ");
        switch(configuration.SPED.airDataRate) {
            case AIR_DATA_RATE_000_03: Serial.println("0.3 kbps"); break;
            case AIR_DATA_RATE_001_12: Serial.println("1.2 kbps"); break;
            case AIR_DATA_RATE_010_24: Serial.println("2.4 kbps"); break;
            case AIR_DATA_RATE_011_48: Serial.println("4.8 kbps"); break;
            case AIR_DATA_RATE_100_96: Serial.println("9.6 kbps"); break;
            case AIR_DATA_RATE_101_192: Serial.println("19.2 kbps"); break;
            default: Serial.println("Desconhecida");
        }
        
        Serial.print("Baud rate UART: ");
        switch(configuration.SPED.uartBaudRate) {
            case UART_BPS_1200: Serial.println("1200"); break;
            case UART_BPS_2400: Serial.println("2400"); break;
            case UART_BPS_4800: Serial.println("4800"); break;
            case UART_BPS_9600: Serial.println("9600"); break;
            case UART_BPS_19200: Serial.println("19200"); break;
            case UART_BPS_38400: Serial.println("38400"); break;
            case UART_BPS_57600: Serial.println("57600"); break;
            case UART_BPS_115200: Serial.println("115200"); break;
        }
        
        Serial.print("Potência de transmissão: ");
        switch(configuration.OPTION.transmissionPower) {
            case POWER_20: Serial.println("20 dBm"); break;
            case POWER_17: Serial.println("17 dBm"); break;
            case POWER_14: Serial.println("14 dBm"); break;
            case POWER_10: Serial.println("10 dBm"); break;
        }
        
        Serial.println("========================================");
    } else {
        Serial.println("Erro ao obter configuração: " + c.status.getResponseDescription());
    }
    
    c.close();
}
