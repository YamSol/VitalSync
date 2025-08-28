#include <Arduino.h>
#include <Wire.h>
#include <esp_adc_cal.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 2000

float leituraADC;
float tensao;
float temperaturaC;
esp_adc_cal_characteristics_t adc_chars;

PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
    Serial.println("B:1");
}

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3300, &adc_chars); // Calibra ADC para 3.3V
    Wire.begin();
    Wire.setClock(100000);

    if (!pox.begin(PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT)) {
        Serial.println("ERROR: Failed to initialize pulse oximeter");
        for(;;);
    } else {
        Serial.println("MAX30100 initialized successfully!");
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_4_4MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    pox.update();
    delay(10);

    // Média de 10 leituras com calibração
    uint32_t soma = 0;
    for (int i = 0; i < 10; i++) {
        soma += analogRead(36);
        delay(10);
    }
    leituraADC = soma / 10.0;
    uint32_t voltage_mV = esp_adc_cal_raw_to_voltage((uint32_t)leituraADC, &adc_chars);
    tensao = voltage_mV / 1000.0;
    temperaturaC = tensao * 100; // Para LM35. Para TMP36: temperaturaC = (tensao - 0.5) * 100;

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Frequência Cardíaca: ");
        Serial.print(pox.getHeartRate());
        Serial.println(" bpm");

        Serial.print("Temperatura: ");
        Serial.print(temperaturaC);
        Serial.println(" °C");

        Serial.print("Saturação de oxigênio: ");
        Serial.print(pox.getSpO2());
        Serial.println(" %");

        Serial.println("------------------------------------------------------------------------------");
        tsLastReport = millis();

        pox.shutdown();
        delay(10);
        pox.resume();
    }
}