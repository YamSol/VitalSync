#include <Arduino.h>
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
        
        // LED indica sistema pronto
        digitalWrite(LED_STATUS, HIGH);
        delay(500);
        digitalWrite(LED_STATUS, LOW);
        delay(500);
        digitalWrite(LED_STATUS, HIGH);
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
    
    // [ETAPA 1] Escuta dados via LoRa
    ReceivedData receivedData = loraReceiver.listenForData();
    
    // Se dados válidos foram recebidos
    if (receivedData.heart_rate != 0 && receivedData.oxygen_level != 0 && receivedData.temperature != 0.0) {
        Serial.println("\n[ETAPA 2] Dados válidos recebidos!");
        
        // Pisca LED para indicar recepção
        blinkLED(3, 200);
        Serial.println("Dados recebidos:");
        Serial.println(" - Heart Rate: " + String(receivedData.heart_rate) +
                            " BPM\n - Oxygen Level: " + String(receivedData.oxygen_level) + 
                            "%\n - Temperature: " + String(receivedData.temperature) + "°C");
        
        // // [ETAPA 3] Conecta ao WiFi
        // Serial.println("\n[ETAPA 3] Conectando ao WiFi...");
        
        // if (networkManager.connectWiFi()) {
        //     Serial.println("✅ WiFi conectado!");
        //  
        //     // [ETAPA 4] Envia dados para API
        //     Serial.println("\n[ETAPA 4] Enviando dados para API...");
        //  
        //     if (networkManager.sendDataToAPI(receivedData)) {
        //         Serial.println("\n✅ SUCESSO: Dados enviados para API!");
        //         blinkLED(5, 100); // LED rápido = sucesso
        //     } else {
        //         Serial.println("\n❌ ERRO: Falha no envio para API!");
        //         blinkLED(10, 50); // LED muito rápido = erro
        //     }
        //    
        //     // [ETAPA 5] Desconecta WiFi
        //     Serial.println("\n[ETAPA 5] Desconectando WiFi...");
        //     networkManager.disconnectWiFi();
        //
        // } else {
        //     Serial.println("❌ Falha na conexão WiFi!");
        //     Serial.println("Tentativa de WiFi adiada por 30 segundos...");
        //     lastWiFiAttempt = millis();
        // }
        
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
