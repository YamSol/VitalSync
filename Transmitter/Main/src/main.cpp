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
#define button 18

// Instâncias dos gerenciadores
SensorManager sensorManager;
LoRaManager loraManager;
struct SensorData sensorDataBuffer[100];

void setup() {    
    pinMode(2, OUTPUT); // Configura o pino do LED embutido como saída
    pinMode(18, INPUT_PULLUP);
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

    if(digitalRead(18) == HIGH)
    {
        sensorManager.initSensors();

        // Aguardando estabilização do sensor
        for(int i = 0; i < 200; i++)
        {
            sensorManager.readSensors();
            delay(100);
        }

        // Fazendo a leitura dos dados
        for(int i = 0; i < 100; i++)
        {
            sensorDataBuffer[i] = sensorManager.readSensors();
            delay(100);
        }

    }
    
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
