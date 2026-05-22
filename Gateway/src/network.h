#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "lora.h"

// Configurações WiFi e API vêm dos build flags do platformio.ini
// Valores padrão caso não sejam definidos
#ifndef WIFI_SSID
#define WIFI_SSID "CONFIGURE_NO_PLATFORMIO_INI"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "CONFIGURE_NO_PLATFORMIO_INI"
#endif
#ifndef API_ENDPOINT
#define API_ENDPOINT "https://localhost/vitals"
#endif
#ifndef API_KEY
#define API_KEY "CONFIGURE_IN_PLATFORMIO_INI"
#endif
#ifndef WIFI_TIMEOUT_MS
#define WIFI_TIMEOUT_MS 10000
#endif
#ifndef HTTP_TIMEOUT_MS
#define HTTP_TIMEOUT_MS 5000
#endif

class NetworkManager {
private:
    bool isWiFiConnected;
    unsigned long connectionStartTime;
    
public:
    NetworkManager();
    bool connectWiFi();
    bool sendDataToAPI(const ReceivedData &data);
    void disconnectWiFi();
    
private:
    String createAPIJSON(const ReceivedData &data);
    bool isConnectionTimeout();
};

#endif