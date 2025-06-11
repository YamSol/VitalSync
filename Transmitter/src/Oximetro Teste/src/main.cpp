#include <Arduino.h>
#include "MAX30100_PulseOximeter.h"
#include <Wire.h>

#define REPORTING_PERIOD_MS     1000

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("B:1");
}

void setup()
{
    Serial.begin(115200);

    // Initialize the PulseOximeter instance and register a beat-detected callback
    // The parameter passed to the begin() method changes the samples flow that
    // the library spews to the serial.
    // Options:
    //  * PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT : filtered samples and beat detection threshold
    //  * PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES : sampled values coming from the sensor, with no processing
    //  * PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES : sampled values after the DC removal filter

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin(PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT)) {
        Serial.println("ERROR: Failed to initialize pulse oximeter");
        for(;;);
    }

    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    // Make sure to call update as fast as possible
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("H:");
        Serial.println(pox.getHeartRate());

        Serial.print("O:");
        Serial.println(pox.getSpO2());

        tsLastReport = millis();
    }
}