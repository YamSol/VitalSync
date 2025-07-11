#include "network.h"

NetworkManager::NetworkManager() : isWiFiConnected(false), connectionStartTime(0) {
    // Construtor
}

bool NetworkManager::connectWiFi() {
    Serial.println("\n[NETWORK] Conectando ao WiFi...");
    Serial.println("SSID: " + String(WIFI_SSID));
    
    connectionStartTime = millis();
    
    // Inicia conexão WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Aguarda conexão com timeout
    while (WiFi.status() != WL_CONNECTED && !isConnectionTimeout()) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isWiFiConnected = true;
        Serial.println("\n✅ WiFi conectado com sucesso!");
        Serial.println("IP: " + WiFi.localIP().toString());
        Serial.println("Signal: " + String(WiFi.RSSI()) + " dBm");
        return true;
    } else {
        isWiFiConnected = false;
        Serial.println("\n❌ Falha na conexão WiFi (timeout)!");
        return false;
    }
}

bool NetworkManager::sendDataToAPI(const ReceivedData &data) {
    if (!isWiFiConnected) {
        Serial.println("ERRO: WiFi não conectado!");
        return false;
    }
    
    Serial.println("\n[NETWORK] Enviando dados para API...");
    
    // Cria JSON para API (formato completo)
    String jsonPayload = createAPIJSON(data);
    
    Serial.println("JSON para API: " + jsonPayload);
    
    // Configura HTTPClient
    HTTPClient http;
    http.begin(API_ENDPOINT);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(HTTP_TIMEOUT_MS);
    
    // Envia POST request
    Serial.println("Enviando POST para: " + String(API_ENDPOINT));
    int httpResponseCode = http.POST(jsonPayload);
    
    // Verifica resposta
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Código HTTP: " + String(httpResponseCode));
        Serial.println("Resposta da API: " + response);
        
        if (httpResponseCode >= 200 && httpResponseCode < 300) {
            Serial.println("✅ Dados enviados com sucesso para API!");
            http.end();
            return true;
        } else {
            Serial.println("❌ Erro HTTP da API: " + String(httpResponseCode));
        }
    } else {
        Serial.println("❌ Erro na conexão HTTP: " + String(httpResponseCode));
        Serial.println("Erro: " + http.errorToString(httpResponseCode));
    }
    
    http.end();
    return false;
}

void NetworkManager::disconnectWiFi() {
    if (isWiFiConnected) {
        Serial.println("\n[NETWORK] Desconectando WiFi...");
        WiFi.disconnect(true);
        isWiFiConnected = false;
        Serial.println("WiFi desconectado!");
    }
}

String NetworkManager::createAPIJSON(const ReceivedData &data) {
    Serial.println("Criando JSON para API...");
    
    // Cria documento JSON no formato completo para API
    JsonDocument doc;
    
    // Adiciona timestamp atual
    unsigned long currentTime = millis() / 1000; // Timestamp Unix básico
    String timestamp = "2025-01-02T" + String(currentTime % 86400 / 3600) + ":00:00"; // Formato simplificado
    
    // Preenche dados no formato completo da API
    doc["device_id"] = data.device_id;
    doc["heart_rate"] = data.heart_rate;
    doc["oxygen_level"] = data.oxygen_level;
    
    // Objeto aninhado para pressão (formato original da API)
    JsonObject pressure = doc["pressure"].to<JsonObject>();
    pressure["systolic"] = data.systolic_pressure;
    pressure["diastolic"] = data.diastolic_pressure;
    
    doc["temperature"] = data.temperature;
    doc["timestamp"] = timestamp;
    
    // Serializa para string
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.println("JSON da API criado com sucesso!");
    return jsonString;
}

bool NetworkManager::isConnectionTimeout() {
    return (millis() - connectionStartTime) > WIFI_TIMEOUT_MS;
}
