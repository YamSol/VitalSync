/*
 * VitalSync - Transmitter
 * Sistema de monitoramento remoto para pacientes crônicos
 * 
 * Fluxo do Transmitter (conforme README.md):
 * 1. Inicializa sensores
 * 2. Lê sinais vitais
 * 3. Gera JSON com os dados
 * 4. Liga o módulo LoRa
 * 5. Envia dados ao Gateway
 * 6. Desliga o módulo e entra em deepSleep
 */

#include <Arduino.h>
#include "sensors.h"
#include "lora.h"

// Configurações de temporização
#define SLEEP_TIME_SECONDS 30  // Tempo em deep sleep (30 segundos para teste)
#define uS_TO_S_FACTOR 1000000ULL  // Fator de conversão de microssegundos para segundos

// Instâncias dos gerenciadores
SensorManager sensorManager;
LoRaManager loraManager;

// Variáveis de controle
bool systemInitialized = false;

// Declaração de funções
void enterDeepSleep();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
Serial.println(String('=', 50));
    Serial.println("VitalSync - Transmitter Iniciando...");
    Serial.println(String('=', 50));
    
    // 1. Inicializar sensores
    Serial.println("\n[ETAPA 1] Inicializando sensores...");
    if (!sensorManager.initSensors()) {
        Serial.println("ERRO CRÍTICO: Falha na inicialização dos sensores!");
        Serial.println("Entrando em deep sleep por segurança...");
        enterDeepSleep();
        return;
    }
    
    systemInitialized = true;
    Serial.println("Sistema inicializado com sucesso!\n");
}

void loop() {
    if (!systemInitialized) {
        Serial.println("Sistema não inicializado. Reiniciando...");
        ESP.restart();
        return;
    }
    
Serial.println("\n" + String('=', 50));
    Serial.println("Iniciando ciclo de transmissão...");
    Serial.println(String('=', 50));
    
    // 2. Ler sinais vitais
    Serial.println("\n[ETAPA 2] Lendo sinais vitais...");
    SensorData data = sensorManager.readSensors();
    
    // 3. Ligar módulo LoRa
    Serial.println("\n[ETAPA 4] Ligando módulo LoRa...");
    if (!loraManager.initLoRa()) {
        Serial.println("ERRO: Falha na inicialização do módulo LoRa!");
        Serial.println("Tentando novamente no próximo ciclo...");
        enterDeepSleep();
        return;
    }
    
    // 4. Serializa o Struct e Enviar dados ao Gateway
    Serial.println("\n[ETAPA 5] Enviando dados ao Gateway...");
    bool sendSuccess = loraManager.sendSensorData(data);
    
    if (sendSuccess) {
        Serial.println("\nSucesso | Dados enviados com sucesso!");
    } else {
        Serial.println("\nFalha | Falha no envio dos dados!");
    }
    
    // 6. Desligar LoRa e entrar em deepSleep
    Serial.println("\n[ETAPA 6] Desligando módulo LoRa...");
    loraManager.shutdownLoRa();
    
    Serial.println("\n[ETAPA 7] Entrando em Deep Sleep...");
    enterDeepSleep();
}

void enterDeepSleep() {
    Serial.println("\n" + String('-', 50));
    Serial.println("Preparando para Deep Sleep...");
    Serial.println("Tempo de sleep: " + String(SLEEP_TIME_SECONDS) + " segundos");
Serial.println(String('-', 50));
    
    // Configura o timer para despertar do deep sleep
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_SECONDS * uS_TO_S_FACTOR);
    
    Serial.println("Entrando em Deep Sleep agora...");
    Serial.flush(); // Garante que todas as mensagens sejam enviadas
    delay(100);    // Pequeno delay para garantir o flush
    // Entra em deep sleep
    esp_deep_sleep_start();
}
