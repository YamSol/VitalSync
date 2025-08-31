#include "lora.h"

LoRaManager::LoRaManager() : serialLoRa(2), e32ttl(&serialLoRa, LORA_M0_PIN, LORA_M1_PIN, LORA_AUX_PIN), isInitialized(false) {
    // Construtor
}
    // Modo normal (igual ao Gateway)
    digitalWrite(LORA_M0_PIN, LOW);
    digitalWrite(LORA_M1_PIN, LOW); 
        
    delay(1000); // Aguarda estabilização
    
bool LoRaManager::initLoRa() {
    // Inicializa Serial para comunicação com E32 (igual ao código funcional)
    serialLoRa.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    
    // Inicializa o módulo E32
    e32ttl.begin();

    // Modo normal (igual ao Gateway)
    digitalWrite(LORA_M0_PIN, LOW);
    digitalWrite(LORA_M1_PIN, LOW); 
        
    delay(1000); // Aguarda estabilização
    
    // Configurar o módulo
    // configureLoRaModule();
    // printConfiguration();
    
    isInitialized = true;
    
    return true;
}

bool LoRaManager::sendSensorData(const SensorData &data) {
    if (!isInitialized) {
        // Serial.println("ERRO: Módulo LoRa não inicializado!");
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
    bool success = sendMessage(jsonData);
    
    return success;
}

void LoRaManager::shutdownLoRa() {
    // Serial.println("Desligando módulo LoRa...");
    
    // Simples desligamento - não precisamos manipular M0/M1
    delay(100);
    
    isInitialized = false;
    // Serial.println("Módulo LoRa desligado!");
}

String LoRaManager::createJSON(const SensorData &data) {
    
    // Cria documento JSON COMPACTO (máximo 58 bytes)
    JsonDocument doc;
    
    // Usa nomes de campos muito curtos para economizar espaço
    doc["id"] = "T001";  // Device ID abreviado
    doc["hr"] = data.heart_rate;        // heart_rate -> hr
    doc["ox"] = data.oxygen_level;      // oxygen_level -> ox  
    doc["temp"] = data.temperature;     // temperature -> temp
    
    // Serializa para string
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Serial.println("JSON compacto criado: " + jsonString);
    // Serial.println("Tamanho: " + String(jsonString.length()) + " bytes");
    
    // if (jsonString.length() > 58) {
    //     Serial.println("AVISO: JSON ainda muito grande! (" + String(jsonString.length()) + " > 58 bytes)");
    // }
    
    // Serial.println("JSON criado com sucesso!");
    return jsonString;
}

bool LoRaManager::sendMessage(const String &message) {
    // Serial.println("Enviando mensagem via UART para E32...");
    
    // Envia via biblioteca E32 (igual ao código funcional)
    ResponseStatus rs = e32ttl.sendMessage(message);
    
    // Exibe o status como no código funcional
    // Serial.print("Status do envio: ");
    // Serial.println(rs.getResponseDescription());
    
    // Para debug, vamos também mostrar o código
    // Serial.println("Código: " + String(rs.code));
    
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
    // Configura o módulo
    e32ttl.begin();

    // Obtém configuração atual
    ResponseStructContainer c = e32ttl.getConfiguration();
    Configuration configuration = *(Configuration *)c.data;
    Serial.print("Status da configuração: ");
    Serial.println(c.status.getResponseDescription());

    if (c.status.code == 1)
    {
        Serial.println("Configuração atual obtida com sucesso!");

        // Define configurações específicas
        configuration.ADDL = 0x01; // Endereço baixo
        configuration.ADDH = 0x00; // Endereço alto
        configuration.CHAN = 23;   // Canal 23
        configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
        configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
        configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
        configuration.OPTION.transmissionPower = POWER_20;

        configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
        configuration.SPED.uartBaudRate = UART_BPS_9600;
        configuration.SPED.uartParity = MODE_00_8N1;

        // Aplica as configurações
        ResponseStatus rsConfig = e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
        Serial.print("Status da aplicação da configuração: ");
        Serial.println(rsConfig.getResponseDescription());

        if (rsConfig.code == 1)
        {
            Serial.println("Configurações aplicadas com sucesso!");
        }
        else
        {
            Serial.println("Erro ao aplicar configurações!");
        }
    }
    else
    {
        Serial.println("Erro ao obter configuração atual!");
    }

    c.close();
}

void LoRaManager::printConfiguration() {
    if (!isInitialized) {
        Serial.println("Módulo LoRa não inicializado!");
        return;
    }
    
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
