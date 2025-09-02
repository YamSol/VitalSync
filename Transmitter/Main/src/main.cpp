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
#define DATA_BUFFER_SIZE 10

// Instâncias dos gerenciadores
SensorManager sensorManager;
LoRaManager loraManager;
struct SensorData sensorDataBuffer[DATA_BUFFER_SIZE];
bool isSendingData = false;

void setup() {    
    pinMode(2, OUTPUT); // Configura o pino do LED embutido como saída
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(1000);

    Serial.begin(115200);
    Serial.println("Iniciando sistema VitalSync - Transmitter");
}

void loop() {
    bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;

    if(!buttonPressed || isSendingData) {
        return;
    }
    isSendingData = true;
    
    // SENSORES
    Serial.println("Iniciando leitura dos sensores");

    sensorManager.initSensors();

    // Aguardando estabilização do sensor
    Serial.println("1. Aguardando estabilização do sensor");
    SensorData dataTemp;
    for(int i = 0; i < 200; i++)
    {
        sensorManager.readOximeter(dataTemp);
        delay(100);
    }

    // Fazendo a leitura dos dados
    Serial.println("2. Lendo dados do oxímetro");

    for(int i = 0; i < DATA_BUFFER_SIZE; i++) {
        sensorManager.readOximeter(sensorDataBuffer[i]);
        delay(100);
    }

    Serial.println("OK! Aguardando 10 segundos para leitura do termômetro");

    delay(10000);
    
    Serial.println("3. Lendo dados do termômetro");
    for(int i = 0; i < DATA_BUFFER_SIZE; i++) {
        sensorManager.readTemperature(sensorDataBuffer[i]);
        delay(100);
    }

    Serial.println("OK!");

    // LORA
    Serial.println("Iniciando transmissão via LoRa"); 

    // inicializando o módulo
    loraManager.initLoRa();

    // enviando os dados
    for(int i = 0; i < DATA_BUFFER_SIZE; i++) {
        Serial.println("DADO N" + String(i+1) + 
        ": Temp=" + String(sensorDataBuffer[i].temperature) + 
        "C, HR=" + String(sensorDataBuffer[i].heart_rate) + 
        "bpm, SpO2=" + String(sensorDataBuffer[i].oxygen_level) + "%");


        int res = loraManager.sendSensorData(sensorDataBuffer[i]);
        Serial.println(res ? "Enviado com sucesso!" : "Falha no envio!");
        delay(100);
    }

    isSendingData = false;
}
