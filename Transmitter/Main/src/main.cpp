/*
 * VitalSync - Transmitter
 * Sistema de monitoramento remoto para pacientes crônicos
 * 
 * Fluxo:
 * 1. Inicializa sensores
 * 2. Lê sinais vitais
 * 3. Liga o módulo LoRa
 * 4. Envia dados ao Gateway
 * 5. Desliga o módulo LoRa
 */

#include <Arduino.h>
#include "sensors.h"
#include "lora.h"

// Instâncias dos gerenciadores
SensorManager sensorManager;
LoRaManager loraManager;

void setup() {    
    pinMode(2, OUTPUT); // Configura o pino do LED embutido como saída
    delay(1000);

    // if(!sensorManager.initSensors()) {
    //     for (;;){
    //         blinkLED();
    //     };
    //     return;
    // }
    Serial.begin(115200);
    Serial.println("Iniciando sistema VitalSync - Transmitter");
}

void loop() {
    // # Fluxo da aplicaçao, baseada em comandos recebidos do tablet
    // 1. Recebeu comando "ler e enviar dados" do tablet
    // 2. Liga sensores
    // 3. Lê os sensores N vezes e armazenar num buffer
    // 4. Desliga sensores
    // 5. Liga o módulo LoRa
    // 6. Envia os dados via LoRa
    // 7. Desliga o módulo LoRa
    // 8. Aguarda próximo comando do tablet

    // SensorData data = sensorManager.readSensors();
    
    if (!loraManager.initLoRa()) {
        Serial.println("ERRO: Falha ao inicializar LoRa!");
        for (;;);
        return;
    }
    Serial.println("Módulo LoRa inicializado com sucesso!");

    SensorData data = {
        .temperature = 36.5,
        .heart_rate = 72,
        .oxygen_level = 98
    };
    bool sendSuccess = loraManager.sendSensorData(data);
    if (!sendSuccess) {
        Serial.println("ERRO: Falha ao enviar dados via LoRa!");
        for (;;);
        return;
    }
    
    loraManager.shutdownLoRa();

    delay(1000);
}
