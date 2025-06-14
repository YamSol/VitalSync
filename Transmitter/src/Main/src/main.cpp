#include <Arduino.h>
#include <HardwareSerial.h>

// Configuração dos pinos do E32-433T20D
#define E32_TX      16   // Pino TX do ESP32 conectado ao RX do E32
#define E32_RX      17   // Pino RX do ESP32 conectado ao TX do E32
#define E32_M0      4    // Pino M0 do E32 (opcional, para configuração)
#define E32_M1      5    // Pino M1 do E32 (opcional, para configuração)
#define E32_AUX     2    // Pino AUX do E32 (opcional, para status)

// Cria uma instância de HardwareSerial para comunicação com o E32
HardwareSerial E32Serial(2);

// Dados fixos que simulam a leitura de sensores
String fixedData = "Temperatura: 37.0°C, Frequência Cardíaca: 72 bpm, Oxigenação: 98%";

void setup() {
    Serial.begin(115200);  // Inicia a comunicação Serial para debug
    while (!Serial);
    
    // Configura os pinos de controle do E32 (opcional)
    pinMode(E32_M0, OUTPUT);
    pinMode(E32_M1, OUTPUT);
    pinMode(E32_AUX, INPUT);
    
    // Configura o E32 em modo normal (M0=0, M1=0)
    digitalWrite(E32_M0, LOW);
    digitalWrite(E32_M1, LOW);
    
    // Inicializa a comunicação serial com o E32
    E32Serial.begin(9600, SERIAL_8N1, E32_RX, E32_TX);
    
    // Aguarda o módulo E32 ficar pronto
    delay(1000);
    
    Serial.println("Módulo E32-433T20D configurado. Pronto para transmissão!");
}

String encryptData(String data) {
    // Implementação simples de "criptografia" (substituição de caracteres)
    // Para uma aplicação real, use uma biblioteca de criptografia adequada
    String encryptedData = "";
    for (int i = 0; i < data.length(); i++) {
        encryptedData += char(data[i] + 1); // Simples deslocamento de caracteres
    }
    return encryptedData;
}

void transmitData(String data) {
    // Verifica se o módulo está pronto (opcional, usando pino AUX)
    // while(digitalRead(E32_AUX) == LOW) {! The chip need
    //     delay(10);
    // }
    
    // Envio de dados via E32-433T20D
    E32Serial.print(encryptData(data));
    
    // Exibe a transmissão no console serial para debug
    Serial.println("Transmitido: " + data);
    
    // Opcional: Adiciona um terminador de linha
    E32Serial.println(); // Envia quebra de linha
}

void loop() {
    // Transmite dados fixos
    transmitData(fixedData);  // Transmite os dados simulados
    
    delay(2500);  // Aguarda 2.5 segundos antes de transmitir novamente
}