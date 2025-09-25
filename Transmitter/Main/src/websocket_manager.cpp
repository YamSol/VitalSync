#include "websocket_manager.h"

// Instância global
WebSocketManager wsManager;

WebSocketManager::WebSocketManager() : 
    server(80), 
    ws("/ws"),
    current_state(STATE_IDLE),
    current_session(""),
    session_start_time(0),
    session_ttl_ms(300000), // 5 minutos
    oxi_buffer(nullptr),
    temp_buffer(nullptr),
    oxi_count(0),
    temp_count(0),
    last_command_time(0),
    commands_in_window(0),
    rate_limit_window_ms(1000),
    max_commands_per_window(2)
{
    // Configuração padrão
    config.device_pin = "123456";
    config.device_name = "VitalSync-TX";
    config.ssid_prefix = "VitalSync";
    config.max_samples = 50;
    config.oxi_interval_ms = 30;
    config.temp_interval_ms = 50;
    config.firmware_version = "1.0.0";
}

WebSocketManager::~WebSocketManager() {
    if (oxi_buffer) {
        delete[] oxi_buffer;
    }
    if (temp_buffer) {
        delete[] temp_buffer;
    }
}

bool WebSocketManager::init() {
    // Inicializar sistema de arquivos
    if (!LittleFS.begin(true)) {
        Serial.println("[WS] Erro ao inicializar LittleFS");
        return false;
    }
    
    // Carregar configuração
    loadConfig();
    
    // Alocar buffers
    oxi_buffer = new TimestampedOxiData[config.max_samples];
    temp_buffer = new TimestampedTempData[config.max_samples];
    
    if (!oxi_buffer || !temp_buffer) {
        Serial.println("[WS] Erro ao alocar buffers");
        return false;
    }
    
    // Configurar Access Point
    String ssid = config.ssid_prefix + "-" + WiFi.macAddress().substring(9, 17);
    ssid.replace(":", "");
    
    WiFi.softAP(ssid.c_str());
    IPAddress ip = WiFi.softAPIP();
    
    Serial.printf("[WS] Access Point criado: %s\n", ssid.c_str());
    Serial.printf("[WS] IP: %s\n", ip.toString().c_str());
    
    // Configurar WebSocket
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onEvent(server, client, type, arg, data, len);
    });
    
    server.addHandler(&ws);
    
    // Servir página principal
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/index.html")) {
            request->send(LittleFS, "/index.html", "text/html");
        } else {
            // HTML básico de fallback caso o arquivo não exista
            String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>VitalSync Transmitter</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; text-align: center; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🩺 VitalSync Transmitter</h1>
        <p><strong>ATENÇÃO:</strong> Arquivo /index.html não encontrado!</p>
        <p>Para usar a interface completa, execute:</p>
        <pre>pio run --target uploadfs</pre>
        <p>Sistema funcionando apenas via WebSocket direto em <code>/ws</code></p>
    </div>
</body>
</html>
            )";
            request->send(200, "text/html", html);
        }
    });
    
    server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleReset(request);
    });
    
    server.begin();
    Serial.println("[WS] Servidor HTTP iniciado");
    
    setState(STATE_IDLE);
    return true;
}

void WebSocketManager::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("[WS] Cliente conectado: %u\n", client->id());
            break;
            
        case WS_EVT_DISCONNECT:
            Serial.printf("[WS] Cliente desconectado: %u\n", client->id());
            break;
            
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
            
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebSocketManager::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; // Null terminator
        
        // Rate limiting
        if (!checkRateLimit()) {
            sendError("RATE_LIMIT", "Muitas requisições");
            return;
        }
        
        // Parse JSON
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            sendError("BAD_JSON", "JSON inválido");
            return;
        }
        
        JsonObject cmd = doc.as<JsonObject>();
        
        // Processar comando
        AsyncWebSocketClient* client = ws.client((uint32_t)info->opcode); // Hack para passar client
        processCommand(cmd, nullptr); // Client será determinado internamente
    }
}

bool WebSocketManager::processCommand(const JsonObject& cmd, AsyncWebSocketClient* client) {
    if (!cmd.containsKey("cmd")) {
        sendError("BAD_JSON", "Campo 'cmd' obrigatório");
        return false;
    }
    
    String command = cmd["cmd"];
    Serial.printf("[WS] Comando recebido: %s\n", command.c_str());
    
    // Handshake não precisa de sessão válida
    if (command == "handshake") {
        return handleHandshake(cmd, client);
    }
    
    // Todos os outros comandos precisam de sessão
    if (!cmd.containsKey("session") || !validateSession(cmd["session"])) {
        sendError("NO_SESSION", "Sessão inválida ou expirada");
        return false;
    }
    
    // Verificar estado para comandos que não sejam close
    if (command != "close" && !isValidState(command)) {
        sendError("BAD_STATE", "Comando não permitido no estado atual: " + getStateString());
        return false;
    }
    
    // Processar comandos
    if (command == "start") return handleStart(cmd);
    else if (command == "read_oximeter") return handleReadOximeter(cmd);
    else if (command == "read_temp") return handleReadTemperature(cmd);
    else if (command == "send_data") return handleSendData(cmd);
    else if (command == "close") return handleClose(cmd);
    else {
        sendError("BAD_JSON", "Comando desconhecido: " + command);
        return false;
    }
}

bool WebSocketManager::handleHandshake(const JsonObject& cmd, AsyncWebSocketClient* client) {
    if (!cmd.containsKey("pin")) {
        sendError("BAD_JSON", "Campo 'pin' obrigatório");
        return false;
    }
    
    String pin = cmd["pin"];
    if (pin != config.device_pin) {
        sendError("PIN_INVALID", "PIN incorreto");
        return false;
    }
    
    // Gerar nova sessão
    current_session = generateSession();
    session_start_time = millis();
    
    setState(STATE_HANDSHAKE_OK);
    
    // Responder
    DynamicJsonDocument doc(512);
    doc["evt"] = "handshake_ok";
    doc["dev"] = config.device_name;
    doc["fw"] = config.firmware_version;
    doc["session"] = current_session;
    
    sendEvent("handshake_ok", doc.as<JsonObject>());
    return true;
}

bool WebSocketManager::handleStart(const JsonObject& cmd) {
    setState(STATE_SENSORS_ON);
    
    // Aqui seria o código para ligar os sensores
    // Por exemplo: sensorManager.powerOn();
    
    DynamicJsonDocument doc(256);
    doc["evt"] = "state";
    doc["value"] = "SENSORS_ON";
    
    sendEvent("state", doc.as<JsonObject>());
    return true;
}

bool WebSocketManager::handleReadOximeter(const JsonObject& cmd) {
    int count = cmd.containsKey("count") ? cmd["count"].as<int>() : config.max_samples;
    if (count > config.max_samples) count = config.max_samples;
    
    setState(STATE_READING_OXI);
    
    // Reset buffer
    oxi_count = 0;
    
    // Aqui seria iniciada a leitura do oxímetro em outra task/thread
    // A leitura efetiva será feita pela classe SensorManager
    // que chamará reportOximeterProgress() e reportOximeterDone()
    
    Serial.printf("[WS] Iniciando leitura oxímetro: %d amostras\n", count);
    return true;
}

bool WebSocketManager::handleReadTemperature(const JsonObject& cmd) {
    int count = cmd.containsKey("count") ? cmd["count"].as<int>() : config.max_samples;
    if (count > config.max_samples) count = config.max_samples;
    
    setState(STATE_READING_TEMP);
    
    // Reset buffer
    temp_count = 0;
    
    Serial.printf("[WS] Iniciando leitura temperatura: %d amostras\n", count);
    return true;
}

bool WebSocketManager::handleSendData(const JsonObject& cmd) {
    // Verificar se ambas leituras foram feitas
    if (oxi_count == 0 || temp_count == 0) {
        sendError("NOT_READY", "Leituras incompletas");
        return false;
    }
    
    setState(STATE_SENDING);
    
    // Desligar sensores e ligar LoRa será feito pela classe LoRaManager
    Serial.println("[WS] Iniciando envio de dados via LoRa");
    return true;
}

bool WebSocketManager::handleClose(const JsonObject& cmd) {
    setState(STATE_IDLE);
    
    // Limpar sessão e buffers
    current_session = "";
    session_start_time = 0;
    oxi_count = 0;
    temp_count = 0;
    
    // Desligar todos os periféricos
    // sensorManager.powerOff();
    // loraManager.shutdown();
    
    DynamicJsonDocument doc(128);
    doc["evt"] = "closed";
    
    sendEvent("closed", doc.as<JsonObject>());
    Serial.println("[WS] Sessão finalizada");
    return true;
}

void WebSocketManager::sendEvent(const String& event, const JsonObject& data) {
    DynamicJsonDocument doc(1024);
    
    if (data.isNull()) {
        doc["evt"] = event;
    } else {
        // Copiar dados do objeto fornecido
        for (JsonPair kv : data) {
            doc[kv.key()] = kv.value();
        }
    }
    
    String message;
    serializeJson(doc, message);
    
    ws.textAll(message);
    Serial.printf("[WS] Evento enviado: %s\n", message.c_str());
}

void WebSocketManager::sendError(const String& code, const String& detail) {
    DynamicJsonDocument doc(512);
    doc["evt"] = "error";
    doc["code"] = code;
    doc["detail"] = detail;
    
    String message;
    serializeJson(doc, message);
    
    ws.textAll(message);
    Serial.printf("[WS] Erro enviado: %s - %s\n", code.c_str(), detail.c_str());
}

bool WebSocketManager::validateSession(const String& session) const {
    if (session.isEmpty() || session != current_session) {
        return false;
    }
    
    // Verificar TTL
    uint32_t now = millis();
    if (now - session_start_time > session_ttl_ms) {
        return false;
    }
    
    return true;
}

bool WebSocketManager::isValidState(const String& command) {
    if (command == "start") {
        return current_state == STATE_HANDSHAKE_OK;
    } else if (command == "read_oximeter") {
        return current_state == STATE_SENSORS_ON;
    } else if (command == "read_temp") {
        return current_state == STATE_SENSORS_ON;
    } else if (command == "send_data") {
        return current_state == STATE_READY_TO_SEND;
    }
    
    return true; // close pode ser chamado a qualquer momento
}

void WebSocketManager::setState(SystemState new_state) {
    current_state = new_state;
    Serial.printf("[WS] Estado alterado para: %s\n", getStateString().c_str());
    
    // Verificar se está pronto para enviar
    if (current_state == STATE_SENSORS_ON && oxi_count > 0 && temp_count > 0) {
        current_state = STATE_READY_TO_SEND;
        
        DynamicJsonDocument doc(256);
        doc["evt"] = "state";
        doc["value"] = "READY_TO_SEND";
        
        sendEvent("state", doc.as<JsonObject>());
    }
}

String WebSocketManager::getStateString() const {
    switch (current_state) {
        case STATE_IDLE: return "IDLE";
        case STATE_HANDSHAKE_OK: return "HANDSHAKE_OK";
        case STATE_SENSORS_ON: return "SENSORS_ON";
        case STATE_READING_OXI: return "READING_OXI";
        case STATE_READING_TEMP: return "READING_TEMP";
        case STATE_READY_TO_SEND: return "READY_TO_SEND";
        case STATE_SENDING: return "SENDING";
        case STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

String WebSocketManager::generateSession() {
    String session = "";
    for (int i = 0; i < 32; i++) {
        session += String(random(0, 16), HEX);
    }
    return session;
}

bool WebSocketManager::checkRateLimit() {
    uint32_t now = millis();
    
    if (now - last_command_time > rate_limit_window_ms) {
        // Nova janela
        commands_in_window = 1;
        last_command_time = now;
        return true;
    } else {
        commands_in_window++;
        return commands_in_window <= max_commands_per_window;
    }
}

bool WebSocketManager::loadConfig() {
    preferences.begin("vitalsync", true);
    
    config.device_pin = preferences.getString("pin", "123456");
    config.device_name = preferences.getString("name", "VitalSync-TX");
    config.max_samples = preferences.getInt("max_samples", 50);
    
    preferences.end();
    return true;
}

void WebSocketManager::saveConfig() {
    preferences.begin("vitalsync", false);
    
    preferences.putString("pin", config.device_pin);
    preferences.putString("name", config.device_name);
    preferences.putInt("max_samples", config.max_samples);
    
    preferences.end();
}

void WebSocketManager::loop() {
    ws.cleanupClients();
    
    // Verificar timeout da sessão
    if (!current_session.isEmpty()) {
        uint32_t now = millis();
        if (now - session_start_time > session_ttl_ms) {
            Serial.println("[WS] Sessão expirou");
            handleClose(JsonObject());
        }
    }
}

// Métodos para reportar eventos das classes externas
void WebSocketManager::reportOximeterProgress(int current, int total) {
    DynamicJsonDocument doc(256);
    doc["evt"] = "oximeter_progress";
    doc["i"] = current;
    doc["n"] = total;
    
    sendEvent("oximeter_progress", doc.as<JsonObject>());
}

void WebSocketManager::reportOximeterDone(int count) {
    oxi_count = count;
    setState(STATE_SENSORS_ON);
    
    DynamicJsonDocument doc(256);
    doc["evt"] = "oximeter_done";
    doc["n"] = count;
    
    sendEvent("oximeter_done", doc.as<JsonObject>());
}

void WebSocketManager::reportTemperatureProgress(int current, int total) {
    DynamicJsonDocument doc(256);
    doc["evt"] = "temp_progress";
    doc["i"] = current;
    doc["n"] = total;
    
    sendEvent("temp_progress", doc.as<JsonObject>());
}

void WebSocketManager::reportTemperatureDone(int count) {
    temp_count = count;
    setState(STATE_SENSORS_ON);
    
    DynamicJsonDocument doc(256);
    doc["evt"] = "temp_done";
    doc["n"] = count;
    
    sendEvent("temp_done", doc.as<JsonObject>());
}

void WebSocketManager::reportLoRaResult(bool success, int rssi, float snr) {
    setState(STATE_IDLE);
    
    DynamicJsonDocument doc(512);
    doc["evt"] = "lora_tx";
    doc["ok"] = success;
    
    if (success) {
        doc["rssi"] = rssi;
        doc["snr"] = snr;
    }
    
    sendEvent("lora_tx", doc.as<JsonObject>());
    
    // Limpar buffers após envio
    oxi_count = 0;
    temp_count = 0;
}

void WebSocketManager::reportError(const String& code, const String& detail) {
    setState(STATE_ERROR);
    sendError(code, detail);
}

void WebSocketManager::handleReset(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Reiniciando o dispositivo...");
    delay(500);  // Small delay to ensure the response is sent
    ESP.restart();
}
