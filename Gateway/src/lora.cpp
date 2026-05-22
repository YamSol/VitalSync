#include "lora.h"

LoRaReceiver::LoRaReceiver() : serialLoRa(2), e32ttl(&serialLoRa, LORA_AUX_PIN, LORA_M0_PIN, LORA_M1_PIN), isInitialized(false) {
    // Construtor
}

bool LoRaReceiver::initLoRa() { 
    Serial.println("Iniciando configuração do módulo LoRa E32...");
    
    // Inicializa Serial para comunicação com E32 (igual ao código funcional)
    serialLoRa.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    Serial.println("Serial LoRa configurado (RX: " + String(LORA_RX_PIN) + ", TX: " + String(LORA_TX_PIN) + ")");
    
    // Inicializa o módulo E32
    e32ttl.begin();
    Serial.println("Módulo E32 inicializado");
    
    // Aguarda estabilização inicial
    Serial.println("Aguardando estabilização inicial...");
    delay(2000);
    
    configureLoRaModule();

    // Aguarda estabilização após configuração
    Serial.println("Aguardando estabilização após configuração...");
    delay(2000);

    // Imprime configuração atual
    printConfiguration();
    
    // Limpa buffer que pode ter dados residuais
    Serial.println("Limpando buffer de recepção...");
    while (e32ttl.available() > 0) {
        e32ttl.receiveMessage();
        delay(100);
    }
    
    isInitialized = true;
    Serial.println("Módulo LoRa E32 inicializado com sucesso!");
    Serial.println("Gateway pronto para receber dados do Transmitter TR-001");
    
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
        configuration.ADDL = 0x01; // Endereço baixo do Gateway
        configuration.ADDH = 0x00; // Endereço alto do Gateway
        configuration.CHAN = 23;      // Canal 23
        configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
        configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
        configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
        configuration.OPTION.transmissionPower = POWER_20;

        configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
        configuration.SPED.uartBaudRate = UART_BPS_9600;
        configuration.SPED.uartParity = MODE_00_8N1;

        // Aplica as configurações
        ResponseStatus rsConfig = e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
        Serial.print("Status da aplicação da configuração: ");
        Serial.println(rsConfig.getResponseDescription());

        if (rsConfig.code == 1)
        {
            Serial.println("Configurações aplicadas com sucesso!");
        }
        else
        {
            Serial.println("Erro ao aplicar configurações! Tentando novamente...");
            delay(1000);
            
            // Segunda tentativa
            ResponseStatus rsConfig2 = e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
            Serial.print("Status da segunda tentativa: ");
            Serial.println(rsConfig2.getResponseDescription());
            
            if (rsConfig2.code == 1) {
                Serial.println("Configurações aplicadas na segunda tentativa!");
            } else {
                Serial.println("ERRO: Falha nas duas tentativas de configuração!");
            }
        }
    }
    else
    {
        Serial.println("Erro ao obter configuração atual! Tentando reinicializar...");
        delay(1000);
        
        // Reinicializa e tenta novamente
        e32ttl.begin();
        delay(1000);
        
        ResponseStructContainer c2 = e32ttl.getConfiguration();
        if (c2.status.code == 1) {
            Serial.println("Configuração obtida na segunda tentativa!");
            // Aplica configuração na segunda tentativa...
        } else {
            Serial.println("ERRO CRÍTICO: Não foi possível comunicar com o módulo E32!");
        }
        c2.close();
    }

    c.close();
}

ReceivedData LoRaReceiver::listenForData() {
    ReceivedData emptyData = {"", 0, 0, 0.0};
    
    if (!isInitialized) {
        Serial.println("ERRO: Módulo LoRa não inicializado!");
        return emptyData;
    }
    
    // Verifica se há dados disponíveis
    if (e32ttl.available() > 0) {
        Serial.println("\n[GATEWAY] Dados recebidos via LoRa!");
        
        // Recebe a mensagem
        ResponseContainer rc = e32ttl.receiveMessage();
        
        Serial.print("Status da recepção: ");
        Serial.println(rc.status.getResponseDescription());
        
        if (rc.status.code == 1) { // Sucesso
            Serial.print("Dados brutos recebidos: ");
            Serial.println(rc.data);

            // Faz parse do JSON individual
            ReceivedData parsedData;
            int parseResult = parseJSON(rc.data, parsedData);
            
            if (parseResult == 0) {
                Serial.println(String("=").substring(0,40) + " DADOS VÁLIDOS " + String("=").substring(0,40));
                Serial.println("Device ID: " + parsedData.device_id);
                Serial.println("Heart Rate: " + String(parsedData.heart_rate) + " BPM");
                Serial.println("Oxygen Level: " + String(parsedData.oxygen_level) + "%");
                Serial.println("Temperature: " + String(parsedData.temperature) + "°C");
                Serial.println(String("=").substring(0,90));
                return parsedData;
            } else {
                Serial.println("Erro no parse do JSON recebido");
            }
        } else {
            Serial.println("Erro na recepção. Código: " + String(rc.status.code));
        }
    }
    
    return emptyData;
}

std::vector<ReceivedData> LoRaReceiver::listenForMultipleData() {
    std::vector<ReceivedData> dataList;
    
    if (!isInitialized) {
        Serial.println("ERRO: Módulo LoRa não inicializado!");
        return dataList;
    }
    
    // Verifica se há dados disponíveis
    if (e32ttl.available() > 0) {
        Serial.println("\n[GATEWAY] Iniciando coleta de múltiplas mensagens LoRa...");
        
        // Coleta múltiplas mensagens durante um período
        unsigned long startTime = millis();
        const unsigned long COLLECT_TIMEOUT = 3000; // 3 segundos para coletar mensagens
        int messageCount = 0;
        
        while ((millis() - startTime) < COLLECT_TIMEOUT) {
            if (e32ttl.available() > 0) {
                messageCount++;
                Serial.println("\n[GATEWAY] Processando mensagem #" + String(messageCount));
                
                // Recebe cada mensagem individual
                ResponseContainer rc = e32ttl.receiveMessage();
                
                Serial.print("Status da recepção: ");
                Serial.println(rc.status.getResponseDescription());
                
                if (rc.status.code == 1) { // Sucesso
                    Serial.print("Dados recebidos: ");
                    Serial.println(rc.data);
                    
                    // DEBUG: Mostra dados em formato hex/decimal
                    Serial.print("Dados em HEX: ");
                    for (int i = 0; i < rc.data.length(); i++) {
                        Serial.print(String(rc.data.charAt(i), HEX) + " ");
                    }
                    Serial.println();
                    
                    Serial.print("Dados em ASCII: ");
                    for (int i = 0; i < rc.data.length(); i++) {
                        char c = rc.data.charAt(i);
                        if (c >= 32 && c <= 126) {
                            Serial.print(c);
                        } else {
                            Serial.print("[" + String((int)c) + "]");
                        }
                    }
                    Serial.println();

                    // Filtra dados válidos (deve começar com '{' para ser JSON)
                    if (rc.data.length() > 0 && (rc.data.charAt(0) == '{' || rc.data.indexOf("{") != -1)) {
                        // Processa se for dados concatenados ou mensagem única
                        if (rc.data.indexOf("}{") != -1) {
                            // Dados concatenados - usar extração múltipla
                            std::vector<ReceivedData> extractedList = extractMultipleJSONs(rc.data);
                            for (size_t i = 0; i < extractedList.size(); i++) {
                                dataList.push_back(extractedList[i]);
                            }
                        } else {
                            // Mensagem única - parsing direto
                            ReceivedData parsedData;
                            if (parseJSON(rc.data, parsedData) == 0) {
                                dataList.push_back(parsedData);
                                Serial.println("✅ Mensagem #" + String(messageCount) + " processada!");
                            } else {
                                Serial.println("❌ Erro no parse da mensagem #" + String(messageCount));
                            }
                        }
                        
                        // Reset do timeout - continua coletando se há mais dados
                        startTime = millis();
                    } else {
                        Serial.println("⚠️  Dados recebidos não são JSON válido, ignorando mensagem #" + String(messageCount));
                    }
                } else {
                    Serial.println("❌ Erro na recepção da mensagem #" + String(messageCount));
                }
            }
            delay(50); // Pequeno delay para dar tempo às mensagens chegarem
        }
        
        Serial.println("\n📋 RESUMO DA COLETA:");
        Serial.println("📨 Mensagens LoRa processadas: " + String(messageCount));
        Serial.println("✅ JSONs válidos coletados: " + String(dataList.size()));
        Serial.println("⏱️  Tempo de coleta: " + String(millis() - startTime + COLLECT_TIMEOUT) + "ms");
    }
    
    return dataList;
}

int LoRaReceiver::parseJSON(const String &jsonData, ReceivedData &data) {
    Serial.println("Fazendo parse do JSON recebido...");
    Serial.println("JSON para parse: " + jsonData);
    
    // Inicializa dados com valores padrão
    data.device_id = "";
    data.heart_rate = -1;
    data.oxygen_level = -1;
    data.temperature = 14.0;
    
    // Cria documento JSON para parse
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonData);
    
    if (error) {
        Serial.println("Erro no parse JSON: " + String(error.c_str()));
        return 1;
    }
    
    Serial.println("JSON parseado com sucesso!");
    
    // Extrai dados (formato compacto do transmitter)
    if (doc.containsKey("id")) {
        data.device_id = doc["id"].as<String>();
        Serial.println("ID encontrado: " + data.device_id);
    }
    
    if (doc.containsKey("hr")) {
        data.heart_rate = doc["hr"];
        Serial.println("Heart Rate encontrado: " + String(data.heart_rate));
    }
    
    if (doc.containsKey("ox")) {
        data.oxygen_level = doc["ox"];
        Serial.println("Oxygen Level encontrado: " + String(data.oxygen_level));
    }
    
    if (doc.containsKey("temp")) {
        data.temperature = doc["temp"];
        Serial.println("Temperature encontrada: " + String(data.temperature));
    }
    
    // Valida se todos os dados foram extraídos
    if (data.device_id.length() == 0 || data.heart_rate == -1 || 
        data.oxygen_level == -1 || data.temperature == 14.0) {
        Serial.println("ERRO: Dados incompletos após parse JSON!");
        return 1;
    }
    
    return 0;
}

std::vector<ReceivedData> LoRaReceiver::extractMultipleJSONs(const String &rawData) {
    std::vector<ReceivedData> dataList;
    Serial.println("\n🔍 Extraindo múltiplos JSONs da mensagem...");
    Serial.println("Dados brutos (" + String(rawData.length()) + " chars): " + rawData);
    
    // Primeiro, vamos limpar caracteres de controle mais agressivamente
    String cleanData = "";
    for (int i = 0; i < rawData.length(); i++) {
        char c = rawData.charAt(i);
        // Mantém apenas caracteres imprimíveis ASCII e alguns específicos do JSON
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t') {
            cleanData += c;
        }
    }
    
    Serial.println("Dados limpos (" + String(cleanData.length()) + " chars): " + cleanData);
    
    // Estratégia mais robusta: usar regex-like para encontrar padrões
    int pos = 0;
    int jsonCount = 0;
    
    while (pos < cleanData.length()) {
        // Procura pelo padrão {"id":"TR-001" mais específico
        int start = cleanData.indexOf("{\"id\":\"TR-001\"", pos);
        if (start == -1) {
            // Se não encontrar o padrão específico, tenta o genérico
            start = cleanData.indexOf("{\"id\":", pos);
            if (start == -1) break;
        }
        
        // Encontra a próxima ocorrência de {"id": para saber onde termina este JSON
        int nextJsonStart = cleanData.indexOf("{\"id\":", start + 1);
        
        // Extrai o JSON considerando o próximo início ou fim da string
        String possibleJson;
        if (nextJsonStart == -1) {
            // É o último JSON
            possibleJson = cleanData.substring(start);
        } else {
            // Há outro JSON depois
            possibleJson = cleanData.substring(start, nextJsonStart);
        }
        
        // Agora procura por um JSON válido dentro desta substring
        int braceCount = 0;
        int jsonEnd = -1;
        
        for (int i = 0; i < possibleJson.length(); i++) {
            char c = possibleJson.charAt(i);
            if (c == '{') {
                braceCount++;
            } else if (c == '}') {
                braceCount--;
                if (braceCount == 0) {
                    jsonEnd = i;
                    break;
                }
            }
        }
        
        if (jsonEnd != -1) {
            String jsonString = possibleJson.substring(0, jsonEnd + 1);
            jsonCount++;
            
            Serial.println("\n📄 JSON #" + String(jsonCount) + " extraído:");
            Serial.println("   Posição: " + String(start) + " - " + String(start + jsonEnd));
            Serial.println("   Conteúdo: " + jsonString);
            Serial.println("   Tamanho: " + String(jsonString.length()) + " chars");
            
            // Valida se parece com um JSON válido antes de tentar parse
            if (jsonString.indexOf("\"id\":") != -1 && 
                jsonString.indexOf("\"hr\":") != -1 && 
                jsonString.indexOf("\"ox\":") != -1 && 
                jsonString.indexOf("\"temp\":") != -1) {
                
                ReceivedData data;
                if (parseJSON(jsonString, data) == 0) {
                    dataList.push_back(data);
                    Serial.println("   ✅ Processado e adicionado com sucesso!");
                } else {
                    Serial.println("   ❌ Parse falhou");
                }
            } else {
                Serial.println("   ⚠️  JSON incompleto, pulando...");
            }
            
            pos = start + jsonEnd + 1;
        } else {
            // Se não conseguir encontrar fim válido, avança
            pos = start + 1;
        }
        
        // Proteção contra loop infinito
        if (pos <= start) {
            Serial.println("⚠️  Proteção contra loop infinito ativada");
            break;
        }
    }
    
    Serial.println("\n📊 Resultado final da extração:");
    Serial.println("   Tamanho original: " + String(rawData.length()) + " chars");
    Serial.println("   Tamanho limpo: " + String(cleanData.length()) + " chars");
    Serial.println("   JSONs encontrados: " + String(jsonCount));
    Serial.println("   JSONs válidos: " + String(dataList.size()));
    
    return dataList;
}


void LoRaReceiver::printConfiguration() {
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
