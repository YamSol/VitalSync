#include <Arduino.h>
#include <HardwareSerial.h>

// Configuração dos pinos do E32-433T20D
#define E32_TX      16   // Pino TX do ESP32 conectado ao RX do E32
#define E32_RX      17   // Pino RX do ESP32 conectado ao TX do E32
#define E32_M0      4    // Pino M0 do E32 (opcional, para configuração)
#define E32_M1      5    // Pino M1 do E32 (opcional, para configuração)
#define E32_AUX     2    // Pino AUX do E32 (opcional, para status)


// Cria uma instância de HardwareSerial para comunicação com o E32
HardwareSerial E32Serial(2);

String receiveData();
String decryptData(String encryptedData);
void processReceivedData(String data);

void setup() {
    Serial.begin(115200);  // Inicia a comunicação Serial para debug
    while (!Serial);
    
    // Configura os pinos de controle do E32 (opcional)
    pinMode(E32_M0, OUTPUT);
    pinMode(E32_M1, OUTPUT);
    pinMode(E32_AUX, INPUT);

    // Configura o E32 em modo normal (M0=0, M1=0)
    digitalWrite(E32_M0, LOW);
    digitalWrite(E32_M1, LOW);
    
    // Inicializa a comunicação serial com o E32
    E32Serial.begin(9600, SERIAL_8N1, E32_RX, E32_TX);
    
    // Aguarda o módulo E32 ficar pronto
    delay(1000);
    
    Serial.println("Gateway E32-433T20D iniciado!");
    Serial.println("Aguardando dados LoRa...");
}

void loop() {
    String receivedData = receiveData();  // Recebe os dados via E32
    
    if (receivedData != "") {
        Serial.println("Dados recebidos: " + receivedData);
  
        // Opcional: Processar os dados recebidos
        processReceivedData(receivedData);
    }
    
    delay(100);  // Aguarda antes de verificar novamente
}

void processReceivedData(String data) {
    // Função para processar os dados recebidos
    // Aqui você pode extrair informações específicas, validar dados, etc.
    
    Serial.println("--- Processando dados ---");
    
    // Exemplo: Extrair informações dos dados recebidos
    if (data.indexOf("Temperatura") != -1) {
        int tempStart = data.indexOf("Temperatura: ") + 13;
        int tempEnd = data.indexOf("°C", tempStart);
        if (tempEnd != -1) {
            String temperatura = data.substring(tempStart, tempEnd);
            Serial.println("Temperatura extraída: " + temperatura + "°C");
        }
    }
    
    if (data.indexOf("Frequência Cardíaca") != -1) {
        int fcStart = data.indexOf("Frequência Cardíaca: ") + 21;
        int fcEnd = data.indexOf(" bpm", fcStart);
        if (fcEnd != -1) {
            String frequencia = data.substring(fcStart, fcEnd);
            Serial.println("Frequência Cardíaca extraída: " + frequencia + " bpm");
        }
    }
    
    if (data.indexOf("Oxigenação") != -1) {
        int oxStart = data.indexOf("Oxigenação: ") + 12;
        int oxEnd = data.indexOf("%", oxStart);
        if (oxEnd != -1) {
            String oxigenacao = data.substring(oxStart, oxEnd);
            Serial.println("Oxigenação extraída: " + oxigenacao + "%");
        }
    }
    
    Serial.println("--- Fim do processamento ---");
}

String decryptData(String encryptedData) {
    // Implementação simples de descriptografia (reverso da criptografia acima)
    String decryptedData = "";
    for (int i = 0; i < encryptedData.length(); i++) {
        decryptedData += char(encryptedData[i] - 1);
    }
    return decryptedData;
}

String receiveData() {
    String receivedData = "";
    
    // Verifica se há dados disponíveis na serial do E32
    if (E32Serial.available()) {
        // Lê todos os dados disponíveis
        while (E32Serial.available()) {
            char c = E32Serial.read();
            receivedData += c;
            delay(10); // Pequeno delay para garantir que todos os dados sejam lidos
        }
    }

    receivedData = decryptData(receivedData);  // Descriptografa os dados recebidos (opcional)
    
    return receivedData;
}