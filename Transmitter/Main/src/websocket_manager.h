#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <LittleFS.h>

// Estados da máquina de estados
enum SystemState {
    STATE_IDLE,
    STATE_HANDSHAKE_OK,
    STATE_SENSORS_ON,
    STATE_READING_OXI,
    STATE_READING_TEMP,
    STATE_READY_TO_SEND,
    STATE_SENDING,
    STATE_ERROR
};

// Estrutura de dados dos sensores com timestamp
struct TimestampedOxiData {
    uint32_t timestamp;
    uint8_t bpm;
    uint8_t spo2;
    bool valid;
};

struct TimestampedTempData {
    uint32_t timestamp;
    float temperature;
    bool valid;
};

// Configurações do sistema
struct SystemConfig {
    String device_pin;
    String device_name;
    String ssid_prefix;
    int max_samples;
    int oxi_interval_ms;
    int temp_interval_ms;
    String firmware_version;
};

class WebSocketManager {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    Preferences preferences;
    
    SystemState current_state;
    SystemConfig config;
    String current_session;
    uint32_t session_start_time;
    uint32_t session_ttl_ms;
    
    // Buffers de dados
    TimestampedOxiData* oxi_buffer;
    TimestampedTempData* temp_buffer;
    int oxi_count;
    int temp_count;
    
    // Rate limiting
    uint32_t last_command_time;
    int commands_in_window;
    uint32_t rate_limit_window_ms;
    int max_commands_per_window;
    
    // Métodos privados
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    bool processCommand(const JsonObject& cmd, AsyncWebSocketClient* client);
    void sendEvent(const String& event, const JsonObject& data = JsonObject());
    void sendError(const String& code, const String& detail = "");
    bool validateSession(const String& session) const;
    bool isValidState(const String& command);
    void setState(SystemState new_state);
    String generateSession();
    bool loadConfig();
    void saveConfig();
    bool checkRateLimit();
    
    // Handlers de comandos
    bool handleHandshake(const JsonObject& cmd, AsyncWebSocketClient* client);
    bool handleStart(const JsonObject& cmd);
    bool handleReadOximeter(const JsonObject& cmd);
    bool handleReadTemperature(const JsonObject& cmd);
    bool handleSendData(const JsonObject& cmd);
    bool handleClose(const JsonObject& cmd);
    
public:
    WebSocketManager();
    ~WebSocketManager();
    
    bool init();
    void loop();
    
    // Getters para estado atual
    SystemState getState() const { return current_state; }
    String getStateString() const;
    bool hasValidSession() const { return !current_session.isEmpty() && validateSession(current_session); }
    
    // Interface para classes externas
    void reportOximeterProgress(int current, int total);
    void reportOximeterDone(int count);
    void reportTemperatureProgress(int current, int total);
    void reportTemperatureDone(int count);
    void reportLoRaResult(bool success, int rssi = 0, float snr = 0.0);
    void reportError(const String& code, const String& detail = "");
    
    // Acesso aos buffers (para classes de sensores/LoRa)
    TimestampedOxiData* getOxiBuffer() { return oxi_buffer; }
    TimestampedTempData* getTempBuffer() { return temp_buffer; }
    int getOxiCount() const { return oxi_count; }
    int getTempCount() const { return temp_count; }
    void setOxiCount(int count) { oxi_count = count; }
    void setTempCount(int count) { temp_count = count; }
    
    // Configurações
    const SystemConfig& getConfig() const { return config; }
    void handleReset(AsyncWebServerRequest *request);
};

// Instância global
extern WebSocketManager wsManager;

#endif
