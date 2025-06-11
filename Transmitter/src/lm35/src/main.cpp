#include <Arduino.h>

const int pinoLM35 = 36; // GPIO 36 (VP)
float temperaturaC;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int leituraADC = analogRead(pinoLM35);
  float tensao = (leituraADC / 4095.0) * 3.3; // ESP32 ADC 12 bits (0–4095), 3.3V
  temperaturaC = tensao * 100; // 10mV por °C

  Serial.print("Temperatura: ");
  Serial.print(temperaturaC);
  Serial.println(" °C");

  delay(1000);
}
