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

// Configurações de temporização
#define SLEEP_TIME_SECONDS 30  // Tempo em deep sleep (30 segundos para teste)
#define uS_TO_S_FACTOR 1000000ULL  // Fator de conversão de microssegundos para segundos

// Instâncias dos gerenciadores
SensorManager sensorManager;
LoRaManager loraManager;


void setup() {    
    pinMode(2, OUTPUT); // Configura o pino do LED embutido como saída
    blinkLED();
    delay(1000);
}

void loop() {
    SensorData data = sensorManager.readSensors();
    delay(1000);
    
    if (!loraManager.initLoRa()) {

        return;
    }
    
    SensorData data = {
        .temperature = 36.5,
        .heart_rate = 72,
        .oxygen_level = 98
    };
    bool sendSuccess = loraManager.sendSensorData(data);
    if (!sendSuccess) {
        for (;;){
            blinkLED();
        };
    }
    
    loraManager.shutdownLoRa();

    blinkLED();
    delay(1000);
}

void blinkLED() {
    const int duration = 300; // Duração do piscar em milissegundos
    const int pin = 2; // GPIO do LED embutido
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
}