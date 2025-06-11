#include <Arduino.h>
#include <LoRa.h>

// Configuração dos pinos do LoRa
#define LORA_SCK    5    // Pino de SCK
#define LORA_MISO   19   // Pino de MISO
#define LORA_MOSI   27   // Pino de MOSI
#define LORA_CS     18   // Pino de Chip Select (NSS)
#define LORA_RST    14   // Pino de RESET
#define LORA_DIO0   2    // Pino DIO0 (para interrupções)

// Dados fixos que simulam a leitura de sensores
String fixedData = "Temperatura: 37.0°C, Frequência Cardíaca: 72 bpm, Oxigenação: 98%";

void setup() {
    Serial.begin(115200);  // Inicia a comunicação Serial
    while (!Serial);

    // Inicializa o LoRa
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);  // Configura os pinos de comunicação do LoRa

    if (!LoRa.begin(433E6)) {  // Inicia o LoRa na frequência 433 MHz
        Serial.println("Falha na inicialização do LoRa!");
        while (1);
    }
    Serial.println("LoRa iniciado com sucesso!");
}

void displayMessage(String message) {
    Serial.println(message);  // Exibe a mensagem no console serial
}

void transmitData(String data) {
    LoRa.beginPacket();  // Inicia o pacote de transmissão
    LoRa.print(data);    // Adiciona os dados ao pacote
    LoRa.endPacket();    // Finaliza o pacote e envia
    displayMessage("Transmitido: " + data);  // Exibe a transmissão no console serial
}

void loop() {
    // Transmite dados fixos
    transmitData(fixedData);  // Transmite os dados simulados (leitura fixa de sensores)

    delay(5000);  // Aguarda 5 segundos antes de transmitir novamente
}
