/*
 * Teste simples do botão - Pino 18
 * Conecte um botão entre o pino 18 e GND
 */

#include <Arduino.h>

#define BUTTON_PIN 18
#define LED_PIN 2

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Ativa pull-up interno
    pinMode(LED_PIN, OUTPUT);           // LED embutido para feedback visual
    
    Serial.println("=== Teste do Botão - Pino 18 ===");
    Serial.println("Pressione o botão para testar!");
    Serial.println("LED acenderá quando botão for pressionado");
    Serial.println("=============================");
}

void loop() {
    // Lê o estado do botão
    bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;  // LOW = pressionado (pull-up)
    
    if (buttonPressed) {
        digitalWrite(LED_PIN, HIGH);    // Acende LED
        Serial.println("BOTÃO PRESSIONADO!");
        delay(200);                     // Debounce simples
    } else {
        digitalWrite(LED_PIN, LOW);     // Apaga LED
    }
    
    delay(50);  // Pequeno delay para evitar spam no serial
}
