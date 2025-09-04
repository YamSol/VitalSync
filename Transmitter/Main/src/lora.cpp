#include "lora.h"

LoRaManager::LoRaManager() : loraHardwareSerial(2), e32ttl(&loraHardwareSerial, LORA_AUX_PIN, LORA_M0_PIN, LORA_M1_PIN), isInitialized(false) {
    // Construtor
}

bool LoRaManager::initLoRa() {
    loraHardwareSerial.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    e32ttl.begin();
    
    Serial.println("START - Configurando modulo");
    configureLoRaModule();
    Serial.println("END - Configurando modulo");
    
    
    printConfiguration();
    
    isInitialized = true;
    return true;
}

bool LoRaManager::sendSensorData(const SensorData &data) {
    if (!isInitialized) {
        Serial.println("Nao foi possível enviar dados");
        Serial.println("ERRO: Módulo LoRa não inicializado!");
        return false;
    }
    
    // Serial.println("Preparando envio de dados via LoRa...");
    
    // Cria JSON com os dados dos sensores
    String jsonData = createJSON(data);
    
    if (jsonData.length() == 0) {
        // Serial.println("ERRO: Falha ao criar JSON!");
        return false;
    }
    // Serial.println("JSON criado: " + jsonData);
    
    // Envia a mensagem compacta
    // jsonData = jsonData + '\n';
    bool success = sendMessage(jsonData);
    
    return success;
}

void LoRaManager::shutdownLoRa() {
    isInitialized = false;
    Serial.println("Módulo LoRa desligado!");
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
    // Serial.println("Enviando mensagem via UART para E32...");
    
    // Envia via biblioteca E32 (igual ao código funcional)
    // ResponseStatus rs = e32ttl.sendMessage(message);
    ResponseStatus rs = e32ttl.sendFixedMessage(GATEWAY_ADDH, GATEWAY_ADDL, CHANNEL, message);

    // Verificar se é SUCCESS (código 1 significa sucesso)
    if (rs.code == 1) {
        // Serial.println("Mensagem enviada com sucesso!");
        return true;
    } else {
        // Serial.println("ERRO no envio da mensagem. Código: " + String(rs.code));
        return false;
    }
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
    configuration.ADDL = 0x01; // Endereço baixo do Transmitter
    configuration.CHAN = 23;
    // configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
    configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
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
    // if (!isInitialized) {
    //     Serial.println("Módulo LoRa não inicializado!");
    //     return;
    // }
    
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
