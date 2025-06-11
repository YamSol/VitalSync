#include <Arduino.h>
#include <LoRa.h>

// Configuração dos pinos do LoRa
#define LORA_SCK    5    // Pino de SCK
#define LORA_MISO   19   // Pino de MISO
#define LORA_MOSI   27   // Pino de MOSI
#define LORA_CS     18   // Pino de Chip Select (NSS)
#define LORA_RST    14   // Pino de RESET
#define LORA_DIO0   2    // Pino DIO0 (para interrupções)

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

String receiveData() {
    int packetSize = LoRa.parsePacket();  // Verifica se há pacote recebido
    String receivedData = "";

    if (packetSize) {
        while (LoRa.available()) {  // Lê os dados recebidos
            receivedData += (char)LoRa.read();
        }
    }

    return receivedData;
}

String encryptData(String data) {
    // Implementação de criptografia AES (placeholder)
    String encryptedData = data;  // Substitua isso por uma lógica real de criptografia
    return encryptedData;
}

void loop() {
    displayMessage("Aguardando dados LoRa...");

    String receivedData = receiveData();  // Recebe os dados via LoRa

    if (receivedData != "") {
        displayMessage("Dados recebidos: " + receivedData);  // Exibe os dados recebidos
        // String encryptedData = encryptData(receivedData);  // Criptografa os dados (não implementado)
        // displayMessage("Dados criptografados: " + encryptedData);  // Exibe os dados criptografados
    }

    delay(100);  // Aguarda antes de verificar novamente
}
