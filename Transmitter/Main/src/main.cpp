/*
 * VitalSync - Transmitter com WebSocket
 * Sistema de monitoramento remoto para pacientes crônicos
 * 
 * # Novo Fluxo da aplicação (controlado via tablet)
 * 1. ESP32 cria Access Point (VitalSync-XXXX)
 * 2. Tablet conecta e faz handshake via WebSocket
 * 3. Tablet envia comandos passo-a-passo:
 *    - start: liga sensores
 *    - read_oximeter: lê N amostras do oxímetro 
 *    - read_temp: lê N amostras de temperatura
 *    - send_data: desliga sensores, liga LoRa, envia dados
 *    - close: finaliza sessão
 * 4. ESP32 responde com eventos de progresso e estado
 * 
 * # Estados da máquina:
 * IDLE → HANDSHAKE_OK → SENSORS_ON → READING_OXI → SENSORS_ON → 
 * READING_TEMP → READY_TO_SEND → SENDING → IDLE
 */

#include <Arduino.h>
#include <WiFi.h>
#include "websocket_manager.h"
#include "sensors.h"
#include "lora.h"

// Instâncias dos gerenciadores
SensorManager sensorManager;
LoRaManager loraManager;

// Task handles para operações assíncronas
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t loraTaskHandle = NULL;

// Forward declarations das tasks
void sensorReadTask(void *parameters);
void loraTransmitTask(void *parameters);
String getStateString(SystemState state);
void handleSensorsOnState();
void handleReadingOxiState();
void handleReadingTempState();
void handleSendingState();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("🩺 VitalSync Transmitter v1.0.0");
    Serial.println("Sistema de Monitoramento com WebSocket");
    Serial.println("========================================");
    
    // Configurar LED de status (opcional)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Inicializar gerenciador de sensores
    Serial.println("[SETUP] Inicializando sensores...");
    if (!sensorManager.initSensors()) {
        Serial.println("[SETUP] ERRO: Falha ao inicializar sensores");
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
    Serial.println("[SETUP] Sensores inicializados com sucesso");
    
    // Inicializar WebSocket Manager
    Serial.println("[SETUP] Inicializando WebSocket...");
    if (!wsManager.init()) {
        Serial.println("[SETUP] ERRO: Falha ao inicializar WebSocket");
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }
    Serial.println("[SETUP] WebSocket inicializado com sucesso");
    
    // LED indica sistema pronto
    digitalWrite(LED_BUILTIN, HIGH);
    
    Serial.println("========================================");
    Serial.println("✅ Sistema pronto e aguardando conexão");
    Serial.println("📱 Conecte o tablet ao WiFi: " + WiFi.softAPSSID());
    Serial.println("🌐 Acesse: http://" + WiFi.softAPIP().toString());
    Serial.println("========================================");
}

void loop() {
    // Processar WebSocket
    wsManager.loop();
    
    // Processar comandos baseado no estado atual
    SystemState currentState = wsManager.getState();
    
    static SystemState lastState = STATE_IDLE;
    if (currentState != lastState) {
        Serial.printf("[LOOP] Estado mudou: %s → %s\n", 
                     getStateString(lastState).c_str(), 
                     wsManager.getStateString().c_str());
        lastState = currentState;
    }
    
    // Executar ações baseadas no estado
    switch (currentState) {
        case STATE_SENSORS_ON:
            handleSensorsOnState();
            break;
            
        case STATE_READING_OXI:
            handleReadingOxiState();
            break;
            
        case STATE_READING_TEMP:
            handleReadingTempState();
            break;
            
        case STATE_SENDING:
            handleSendingState();
            break;
            
        default:
            // Outros estados não precisam de ação no loop principal
            break;
    }
    
    // Piscar LED para indicar que está vivo
    static uint32_t lastBlink = 0;
    if (millis() - lastBlink > (wsManager.hasValidSession() ? 2000 : 5000)) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        lastBlink = millis();
    }
    
    delay(10); // Pequena pausa para não sobrecarregar o processador
}
void handleSensorsOnState() {
    // Sensores estão sempre ligados, apenas verificar se leituras foram concluídas
    if (wsManager.getOxiCount() > 0 && wsManager.getTempCount() > 0) {
        // Estado será alterado automaticamente para READY_TO_SEND
        Serial.println("[MAIN] Ambas leituras concluídas, pronto para enviar");
    }
}

void handleReadingOxiState() {
    static bool taskStarted = false;
    
    if (!taskStarted) {
        // Criar task para leitura do oxímetro em paralelo
        xTaskCreate(
            sensorReadTask,
            "OximeterRead",
            4096,
            (void*)"oximeter",
            1,
            &sensorTaskHandle
        );
        taskStarted = true;
    }
    
    // Verificar se task terminou
    if (sensorTaskHandle == NULL) {
        taskStarted = false;
    }
}

void handleReadingTempState() {
    static bool taskStarted = false;
    
    if (!taskStarted) {
        // Criar task para leitura da temperatura em paralelo
        xTaskCreate(
            sensorReadTask,
            "TemperatureRead", 
            4096,
            (void*)"temperature",
            1,
            &sensorTaskHandle
        );
        taskStarted = true;
    }
    
    // Verificar se task terminou
    if (sensorTaskHandle == NULL) {
        taskStarted = false;
    }
}

void handleSendingState() {
    static bool taskStarted = false;
    
    if (!taskStarted) {
        // Criar task para transmissão LoRa
        xTaskCreate(
            loraTransmitTask,
            "LoRaTransmit",
            4096,
            NULL,
            1,
            &loraTaskHandle
        );
        taskStarted = true;
    }
    
    // Verificar se task terminou  
    if (loraTaskHandle == NULL) {
        taskStarted = false;
    }
}

// Task para leitura dos sensores
void sensorReadTask(void *parameters) {
    const char* sensorType = (const char*)parameters;
    const SystemConfig& config = wsManager.getConfig();
    
    Serial.printf("[TASK] Iniciando leitura de %s\n", sensorType);
    
    bool success = false;
    
    if (strcmp(sensorType, "oximeter") == 0) {
        success = sensorManager.readOximeterSequence(config.max_samples);
    } else if (strcmp(sensorType, "temperature") == 0) {
        success = sensorManager.readTemperatureSequence(config.max_samples);
    }
    
    if (!success) {
        Serial.printf("[TASK] ERRO na leitura de %s\n", sensorType);
        wsManager.reportError("SENSOR_READ_FAIL", sensorType);
    }
    
    Serial.printf("[TASK] Leitura de %s concluída\n", sensorType);
    
    // Cleanup
    sensorTaskHandle = NULL;
    vTaskDelete(NULL);
}

// Task para transmissão LoRa
void loraTransmitTask(void *parameters) {
    Serial.println("[TASK] Iniciando transmissão LoRa");
    
    // Ligar e configurar LoRa
    loraManager.powerOn();
    delay(500); // Aguardar inicialização
    
    if (!loraManager.isReady()) {
        Serial.println("[TASK] ERRO: LoRa não está pronto");
        wsManager.reportError("LORA_NOT_READY", "Falha na inicialização");
        loraManager.powerOff();
        loraTaskHandle = NULL;
        vTaskDelete(NULL);
        return;
    }
    
    // Enviar dados
    bool success = loraManager.sendAllData();
    
    // Desligar LoRa após transmissão
    delay(100);
    loraManager.powerOff();
    
    Serial.printf("[TASK] Transmissão LoRa %s\n", success ? "bem-sucedida" : "falhou");
    
    // Cleanup
    loraTaskHandle = NULL;
    vTaskDelete(NULL);
}

// Função auxiliar para converter estado em string (para debug)
String getStateString(SystemState state) {
    switch (state) {
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
