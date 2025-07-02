#include <Arduino.h>
#include <HardwareSerial.h>

// Configuração dos pinos do E32-433T20D
#define E32_TX      16   // Pino TX do ESP32 conectado ao RX do E32
#define E32_RX      17   // Pino RX do ESP32 conectado ao TX do E32
#define E32_M0      4    // Pino M0 do E32 (para configuração)
#define E32_M1      5    // Pino M1 do E32 (para configuração)
#define E32_AUX     25   // Pino AUX do E32

// Cria uma instância de HardwareSerial para comunicação com o E32
HardwareSerial E32Serial(2);

void setMode(int mode);
void printConfig();
void configureModule();
void sendTestMessage();
void receiveMessages();
void printMenu();

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    Serial.println("=== TESTE DE CONECTIVIDADE E32-433T20D ===");
    Serial.println("Inicializando...");
    
    // Configura pinos
    pinMode(E32_M0, OUTPUT);
    pinMode(E32_M1, OUTPUT);
    pinMode(E32_AUX, INPUT_PULLUP);
    
    // Coloca em sleep primeiro
    setMode(3); // Sleep mode
    delay(100);
    
    // Inicializa serial do E32
    E32Serial.begin(9600, SERIAL_8N1, E32_RX, E32_TX);
    delay(500);
    
    // Coloca em modo normal
    setMode(0); // Normal mode
    delay(1000);
    
    // Aguarda módulo ficar pronto
    int timeout = 0;
    while(digitalRead(E32_AUX) == LOW && timeout < 50) {
        delay(100);
        timeout++;
    }
    
    Serial.println("Status AUX: " + String(digitalRead(E32_AUX) ? "HIGH (Pronto)" : "LOW (Ocupado)"));
    Serial.println("Inicialização concluída!");
    
    printMenu();
}

void loop() {
    // Verifica comandos do Serial Monitor
    if (Serial.available()) {
        char command = Serial.read();
        Serial.flush(); // Limpa buffer
        
        switch(command) {
            case '1':
                Serial.println("\n--- ENVIANDO MENSAGEM DE TESTE ---");
                sendTestMessage();
                break;
                
            case '2':
                Serial.println("\n--- MODO ESCUTA CONTÍNUA ---");
                Serial.println("(Digite qualquer tecla para parar)");
                receiveMessages();
                break;
                
            case '3':
                Serial.println("\n--- CONFIGURANDO MÓDULO ---");
                configureModule();
                break;
                
            case '4':
                Serial.println("\n--- LENDO CONFIGURAÇÃO ---");
                printConfig();
                break;
                
            case '5':
                Serial.println("\n--- TESTE DE PINOS ---");
                testPins();
                break;
                
            case '?':
            case 'h':
                printMenu();
                break;
                
            default:
                // Ignora outros caracteres
                break;
        }
    }
    
    // Verifica se há dados chegando do E32
    if (E32Serial.available()) {
        String receivedData = "";
        unsigned long startTime = millis();
        
        while (E32Serial.available() || (millis() - startTime < 100)) {
            if (E32Serial.available()) {
                receivedData += char(E32Serial.read());
                startTime = millis(); // Reset timeout
            }
            delay(1);
        }
        
        if (receivedData.length() > 0) {
            Serial.println(">>> DADOS RECEBIDOS: " + receivedData);
            Serial.println(">>> Tamanho: " + String(receivedData.length()) + " bytes");
        }
    }
    
    delay(10);
}

void setMode(int mode) {
    switch(mode) {
        case 0: // Normal
            digitalWrite(E32_M0, LOW);
            digitalWrite(E32_M1, LOW);
            Serial.println("Modo: NORMAL (Transmissão/Recepção)");
            break;
        case 1: // WOR
            digitalWrite(E32_M0, LOW);
            digitalWrite(E32_M1, HIGH);
            Serial.println("Modo: WOR (Wake on Radio)");
            break;
        case 2: // Config
            digitalWrite(E32_M0, HIGH);
            digitalWrite(E32_M1, LOW);
            Serial.println("Modo: CONFIGURAÇÃO");
            break;
        case 3: // Sleep
            digitalWrite(E32_M0, HIGH);
            digitalWrite(E32_M1, HIGH);
            Serial.println("Modo: SLEEP");
            break;
    }
    delay(100);
}

void sendTestMessage() {
    // Verifica se módulo está pronto
    if (digitalRead(E32_AUX) == LOW) {
        Serial.println("ERRO: Módulo não está pronto (AUX = LOW)");
        return;
    }
    
    String testMessage = "TESTE_" + String(millis());
    
    Serial.println("Enviando: " + testMessage);
    E32Serial.print(testMessage);
    
    // Aguarda transmissão
    delay(100);
    
    Serial.println("Mensagem enviada!");
    Serial.println("Status AUX após envio: " + String(digitalRead(E32_AUX) ? "HIGH" : "LOW"));
}

void receiveMessages() {
    Serial.println("Escutando... (pressione qualquer tecla para parar)");
    
    while (!Serial.available()) {
        if (E32Serial.available()) {
            String message = "";
            unsigned long startTime = millis();
            
            while (E32Serial.available() || (millis() - startTime < 200)) {
                if (E32Serial.available()) {
                    message += char(E32Serial.read());
                    startTime = millis();
                }
                delay(1);
            }
            
            if (message.length() > 0) {
                Serial.println("RECEBIDO: " + message);
            }
        }
        delay(10);
    }
    
    // Limpa buffer serial
    while (Serial.available()) Serial.read();
    Serial.println("Modo escuta finalizado.");
}

void configureModule() {
    Serial.println("Configurando módulo E32...");
    
    // Entra em modo de configuração
    setMode(2); // Config mode
    delay(100);
    
    // Aguarda entrar em modo config
    int timeout = 0;
    while(digitalRead(E32_AUX) == LOW && timeout < 50) {
        delay(10);
        timeout++;
    }
    
    if (timeout >= 50) {
        Serial.println("ERRO: Timeout ao entrar em modo configuração");
        setMode(0); // Volta para normal
        return;
    }
    
    // Comando para configurar: ADDH, ADDL, CHAN, etc.
    // Configuração padrão recomendada:
    uint8_t config[] = {0xC0, 0x00, 0x00, 0x1A, 0x17, 0x44}; // Configuração padrão
    
    Serial.println("Enviando configuração...");
    for (int i = 0; i < 6; i++) {
        E32Serial.write(config[i]);
    }
    
    delay(100);
    
    // Lê resposta
    String response = "";
    timeout = 0;
    while (timeout < 100) {
        if (E32Serial.available()) {
            response += String(E32Serial.read(), HEX) + " ";
        }
        delay(10);
        timeout++;
    }
    
    Serial.println("Resposta: " + response);
    
    // Volta para modo normal
    setMode(0);
    delay(100);
    
    Serial.println("Configuração concluída!");
}

void printConfig() {
    Serial.println("Lendo configuração atual...");
    
    // Entra em modo configuração
    setMode(2);
    delay(100);
    
    // Comando para ler configuração
    uint8_t readCmd[] = {0xC1, 0xC1, 0xC1};
    
    for (int i = 0; i < 3; i++) {
        E32Serial.write(readCmd[i]);
    }
    
    delay(100);
    
    // Lê configuração
    String config = "";
    int timeout = 0;
    while (timeout < 100) {
        if (E32Serial.available()) {
            uint8_t byte = E32Serial.read();
            config += String(byte, HEX) + " ";
        }
        delay(10);
        timeout++;
    }
    
    Serial.println("Configuração atual: " + config);
    
    // Volta para modo normal
    setMode(0);
}

void testPins() {
    Serial.println("=== TESTE DE PINOS ===");
    Serial.println("M0 (GPIO 4): " + String(digitalRead(E32_M0)));
    Serial.println("M1 (GPIO 5): " + String(digitalRead(E32_M1)));
    Serial.println("AUX (GPIO 25): " + String(digitalRead(E32_AUX)));
    
    Serial.println("\nTestando controle M0/M1...");
    for (int i = 0; i < 4; i++) {
        setMode(i);
        delay(200);
        Serial.println("Modo " + String(i) + " - AUX: " + String(digitalRead(E32_AUX)));
    }
    
    setMode(0); // Volta para normal
}

void printMenu() {
    Serial.println("\n=== MENU DE TESTES ===");
    Serial.println("1 - Enviar mensagem de teste");
    Serial.println("2 - Modo escuta contínua");
    Serial.println("3 - Configurar módulo");
    Serial.println("4 - Ler configuração");
    Serial.println("5 - Teste de pinos");
    Serial.println("? - Mostrar este menu");
    Serial.println("=======================");
}
