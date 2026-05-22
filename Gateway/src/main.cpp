#include <Arduino.h>
#include <vector>
#include "lora.h"
#include "network.h"

// Instâncias dos gerenciadores
LoRaReceiver loraReceiver;
NetworkManager networkManager;

// Configurações
#define LED_STATUS 2
#define WIFI_RETRY_DELAY 30000 // 30 segundos entre tentativas de WiFi

unsigned long lastWiFiAttempt = 0;
bool systemReady = false;

void blinkLED(int times, int delayMs);

void setup() {
    Serial.begin(115200);
    Serial.println("\n" + String("=").substring(0,50));
    Serial.println("🩺 VitalSync Gateway Iniciando...");
    Serial.println("" + String("=").substring(0,50));
    
    // Configura LED de status
    pinMode(LED_STATUS, OUTPUT);
    digitalWrite(LED_STATUS, LOW);
    
    Serial.println("\n[ETAPA 1] Inicializando módulo LoRa...");
    
    // Inicializa receptor LoRa
    if (loraReceiver.initLoRa()) {
        Serial.println("✅ LoRa inicializado com sucesso!");
        systemReady = true;
        
        blinkLED(2, 500);
    } else {
        Serial.println("❌ Falha na inicialização do LoRa!");
        systemReady = false;
    }
    
    Serial.println("\n[GATEWAY] Sistema pronto - Modo escuta LoRa ativo");
    Serial.println("Aguardando dados do Transmitter...\n");
}

void loop() {
    if (!systemReady) {
        Serial.println("Sistema não está pronto. Tentando reinicializar...");
        delay(5000);
        ESP.restart();
        return;
    }
    
    // [ETAPA 1] Escuta dados via LoRa - agora processa múltiplos dados
    std::vector<ReceivedData> receivedDataList = loraReceiver.listenForMultipleData();

    // Se dados válidos foram recebidos
    if (!receivedDataList.empty()) {
        Serial.println("\n[ETAPA 2] " + String(receivedDataList.size()) + " conjunto(s) de dados válidos recebidos!");
        
        // Pisca LED para indicar recepção
        blinkLED(3, 300);
        
        // [ETAPA 3] Conecta ao WiFi
        Serial.println("\n[ETAPA 3] Conectando ao WiFi...");
        
        if (networkManager.connectWiFi()) {
            Serial.println("✅ WiFi conectado!");
         
            // [ETAPA 4] Envia todos os dados para API
            Serial.println("\n[ETAPA 4] Enviando " + String(receivedDataList.size()) + " conjunto(s) de dados para API...");
            
            int successCount = 0;
            int errorCount = 0;
            
            for (size_t i = 0; i < receivedDataList.size(); i++) {
                ReceivedData currentData = receivedDataList[i];
                
                Serial.println("\n--- ENVIANDO DADOS #" + String(i + 1) + " ---");
                Serial.println("Device ID: " + currentData.device_id);
                Serial.println("Heart Rate: " + String(currentData.heart_rate) + " BPM");
                Serial.println("Oxygen Level: " + String(currentData.oxygen_level) + "%");
                Serial.println("Temperature: " + String(currentData.temperature) + "°C");
                
                if (networkManager.sendDataToAPI(currentData)) {
                    Serial.println("✅ Dados #" + String(i + 1) + " enviados com sucesso!");
                    successCount++;
                } else {
                    Serial.println("❌ Erro no envio dos dados #" + String(i + 1));
                    errorCount++;
                }
                
                // Pequeno delay entre envios para não sobrecarregar a API
                delay(500);
            }
            
            Serial.println("\n📊 RESUMO DO ENVIO:");
            Serial.println("✅ Sucessos: " + String(successCount));
            Serial.println("❌ Erros: " + String(errorCount));
            Serial.println("📦 Total processado: " + String(receivedDataList.size()));
            
            if (successCount > 0) {
                blinkLED(5, 100); // LED rápido = sucesso
            }
            if (errorCount > 0) {
                blinkLED(10, 50); // LED muito rápido = erro
            }
           
            // [ETAPA 5] Desconecta WiFi
            Serial.println("\n[ETAPA 5] Desconectando WiFi...");
            networkManager.disconnectWiFi();
        
        } else {
            Serial.println("❌ Falha na conexão WiFi!");
            Serial.println("Tentativa de WiFi adiada por 30 segundos...");
            lastWiFiAttempt = millis();
        }
      
        Serial.println("\n[GATEWAY] Retornando ao modo escuta LoRa...");
        Serial.println("" + String("-").substring(0,50) + "\n");
    }
    
    // Pequeno delay para não sobrecarregar o sistema
    delay(100);
}

void blinkLED(int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_STATUS, HIGH);
        delay(delayMs);
        digitalWrite(LED_STATUS, LOW);
        delay(delayMs);
    }
}
