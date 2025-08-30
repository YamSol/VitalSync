#include <Arduino.h>


void blinkLED() {
    const int duration = 500; // Duração do piscar em milissegundos
    const int pin = 2; // GPIO do LED embutido
    digitalWrite(pin, LOW);
    delay(duration);
    digitalWrite(pin, HIGH);
}

void setup() {
    pinMode(2, OUTPUT);
}

void loop() {
    blinkLED();
    delay(1000);
}