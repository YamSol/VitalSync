/*
 * VitalSync - Transmitter
 * Sistema de monitoramento remoto para pacientes crônicos
 * 
 * # Fluxo da aplicação
 * 1. Recebeu comando "ler e enviar dados" do tablet
 * 2. Liga sensores
 * 3. Lê os sensores N vezes e armazenar num buffer
 * 4. Desliga sensores
 * 5. Liga o módulo LoRa
 * 6. Envia os dados via LoRa
 * 7. Desliga o módulo LoRa
 * 8. Aguarda próximo comando do tablet
 */

#include <Arduino.h>
#include "sensors.h"
#include "lora.h"
#define BUTTON_PIN 18
#define DATA_BUFFER_SIZE 100

// Instâncias dos gerenciadores
SensorManager sensorManager;
LoRaManager loraManager;
struct SensorData sensorDataBuffer[DATA_BUFFER_SIZE];
bool isTransmitting = false;

void setup() {    
    pinMode(2, OUTPUT); // Configura o pino do LED embutido como saída
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(1000);

    Serial.begin(115200);
    Serial.println("Iniciando sistema VitalSync - Transmitter");
}

void loop() {
    bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;

    if(!buttonPressed || isTransmitting) {
        return;
    }
    isTransmitting = true;
    
    sensorManager.initSensors();

    // Aguardando estabilização do sensor
    Serial.println("Aguardando estabilização do sensor...");
    SensorData dataTemp;
    for(int i = 0; i < 200; i++)
    {
        sensorManager.readOximeter(dataTemp);
        delay(100);
    }

    // Fazendo a leitura dos dados
    Serial.println("Lendo dados do oxímetro...");

    for(int i = 0; i < DATA_BUFFER_SIZE; i++) {
        sensorManager.readOximeter(sensorDataBuffer[i]);
        delay(100);
    }
    
    Serial.println("Leitura do oxímetro concluída... 10 segundos até a leitura de temperatura");
    delay(10000);
    Serial.println("Lendo dados do termômetro...");
    for(int i = 0; i < DATA_BUFFER_SIZE; i++) {
        sensorManager.readTemperature(sensorDataBuffer[i]);
        delay(100);
    }

    Serial.println("Leitura de temperatura concluída.");

    for(int i = 0; i < DATA_BUFFER_SIZE; i++) {
        Serial.println("DADO N" + String(i) + "-------------------");
        Serial.println("Heart Rate: " + String(sensorDataBuffer[i].heart_rate) + " BPM");
        Serial.println("Oxygen Level: " + String(sensorDataBuffer[i].oxygen_level) + "%");
        Serial.println("Temperature: " + String(sensorDataBuffer[i].temperature) + "°C");
    }

    // TODO: Enviando os dados via LoRa

}
