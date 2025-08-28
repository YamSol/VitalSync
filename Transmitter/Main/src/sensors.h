#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Estrutura para armazenar dados dos sensores
struct SensorData {
    String device_id;
    int heart_rate;
    int oxygen_level;
    struct {
        int systolic;
        int diastolic;
    } pressure;
    float temperature;
    String timestamp;
};

class SensorManager {
public:
    SensorManager();
    bool initSensors();
    SensorData readSensors();
    
private:
    String getCurrentTimestamp();
    bool validateSensorData(const SensorData &data);
};

#endif
