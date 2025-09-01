#include "lora.h"

LoRaReceiver::LoRaReceiver() : serialLoRa(2), e32ttl(&serialLoRa, LORA_M0_PIN, LORA_M1_PIN, LORA_AUX_PIN), isInitialized(false) {
    // Construtor
}

bool LoRaReceiver::initLoRa() { 
    // Inicializa Serial para comunicação com E32 (igual ao código funcional)
    serialLoRa.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    
    // Inicializa o módulo E32
    e32ttl.begin();
        
    configureLoRaModule();

    // Imprime configuração atual
    printConfiguration();
    
    isInitialized = true;
    Serial.println("Módulo LoRa E32 inicializado com sucesso!");
    
    return true;
}

void LoRaReceiver::configureLoRaModule()
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
        configuration.ADDL = GATEWAY_ADDL; // Endereço baixo do Gateway
        configuration.ADDH = GATEWAY_ADDH; // Endereço alto do Gateway
        configuration.CHAN = CHANNEL;      // Canal 23
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

ReceivedData LoRaReceiver::listenForData() {
    ReceivedData emptyData = {0, 0, 0.0};
    
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
            ReceivedData parsedData;
            parseJSON(rc.data, parsedData);
            
            Serial.println(String("=",10)+"Dados válidos recebidos!"+String("=",10));
            Serial.println("Heart Rate: " + String(parsedData.heart_rate) + " BPM");
            Serial.println("Oxygen Level: " + String(parsedData.oxygen_level) + "%");
            Serial.println("Temperature: " + String(parsedData.temperature) + "°C");
                
            return parsedData;
        } else {
            Serial.println("Erro na recepção. Código: " + String(rc.status.code));
        }
    }
    
    return emptyData;
}

int LoRaReceiver::parseJSON(const String &jsonData, ReceivedData &data) {
    Serial.println("Fazendo parse do JSON recebido...");
    
    // Cria documento JSON para parse
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonData);
    
    if (error) {
        Serial.println("Erro no parse JSON: " + String(error.c_str()));
        return 1;
    }
    
    // Extrai dados (formato compacto do transmitter)
    // if (doc.containsKey("id")) data.device_id = doc["id"].as<String>();
    if (doc.containsKey("hr")) data.heart_rate = doc["hr"];
    if (doc.containsKey("ox")) data.oxygen_level = doc["ox"];
    if (doc.containsKey("temp")) data.temperature = doc["temp"];
    
    return 0;
}


void LoRaReceiver::printConfiguration() {
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
