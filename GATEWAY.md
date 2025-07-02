# 🌐 VitalSync Gateway

**Receptor LoRa e ponte WiFi para API**

## 📋 Descrição

O Gateway é responsável por receber dados dos Transmitters via LoRa, validar os dados, conectar ao WiFi e encaminhar para a API REST. Funciona como bridge entre a rede LoRa e a internet.

## 🔧 Hardware Necessário

### Componentes Principais
- **ESP32 DevKit V1**
- **E32-433T20D (LoRa)**
- **LED de Status** (GPIO 2)
- **Antena 433MHz**

### Especificações
- **Alimentação**: 5V via USB ou 3.3V regulado
- **WiFi**: 2.4GHz integrado
- **LoRa**: 433MHz, alcance até 3km em campo aberto

## 🔌 Diagrama de Conexões

```
E32-433T20D     ESP32 DevKit V1
VCC      →      3.3V
GND      →      GND  
TX       →      GPIO 17 (RXD2)
RX       →      GPIO 16 (TXD2)

LED Status      ESP32 DevKit V1
Anodo    →      GPIO 2
Catodo   →      GND (através de resistor 330Ω)
```

## 📊 Arquitetura do Código

### Estrutura Modular
```
src/
├── main.cpp          # Fluxo principal do Gateway
├── lora.h/cpp        # Recepção LoRa e parsing
└── network.h/cpp     # Gerenciamento WiFi e HTTP
```

### Classes Principais
- **`LoRaReceiver`**: Escuta e processa dados LoRa
- **`NetworkManager`**: Gerencia WiFi e comunicação HTTP

## 🔄 Fluxo de Operação

```
┌─────────────────┐
│   [BOOT]        │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Inicializa      │
│ LoRa Receiver   │
└─────────┬───────┘
          │
┌─────────▼───────┐    Não
│ Dados LoRa   ┌──┴──────────┐
│ Recebidos?   │             │
└─────────┬────┘             │
          │ Sim              │
┌─────────▼───────┐          │
│ Valida e        │          │
│ Parse JSON      │          │
└─────────┬───────┘          │
          │                  │
┌─────────▼───────┐          │
│ Conecta WiFi    │          │
└─────────┬───────┘          │
          │                  │
┌─────────▼───────┐          │
│ Envia HTTP      │          │
│ POST para API   │          │
└─────────┬───────┘          │
          │                  │
┌─────────▼───────┐          │
│ Desconecta      │          │
│ WiFi            │          │
└─────────┬───────┘          │
          │                  │
          └──────────────────┘
```

## 📦 Processamento de Dados

### JSON Recebido (LoRa)
```json
{"id":"T001","hr":72,"ox":97,"ps":120,"pd":80,"temp":36.5}
```

### JSON Expandido (API)
```json
{
  "device_id": "ESP32_TRANSMITTER_001",
  "heart_rate": 72,
  "oxygen_level": 97,
  "pressure": {
    "systolic": 120,
    "diastolic": 80
  },
  "temperature": 36.5,
  "timestamp": "2025-01-02T10:00:00"
}
```

### Validação de Dados
```cpp
Heart Rate: 30-200 BPM
Oxygen Level: 70-100%
Temperature: 30.0-45.0°C
Systolic BP: 70-250 mmHg
Diastolic BP: 40-150 mmHg
```

## ⚙️ Configurações

### WiFi e API (platformio.ini)
```ini
build_flags = 
    -DWIFI_SSID='"SuaRedeWiFi"'
    -DWIFI_PASSWORD='"SuaSenha123"'
    -DAPI_ENDPOINT='"http://localhost:8000/gateway/device-data"'
    -DWIFI_TIMEOUT_MS=10000
    -DHTTP_TIMEOUT_MS=5000
```

### Comunicação
```cpp
Serial Baud Rate: 115200
LoRa Baud Rate: 9600
LoRa Pins: RX=16, TX=17
```

### Timeouts
```cpp
WiFi Connection: 10 segundos
HTTP Request: 5 segundos
LED Status: Diferentes padrões
```

## 🚦 Indicações LED

### Padrões de Status
- **2 piscadas rápidas**: Sistema inicializado
- **3 piscadas médias**: Dados LoRa recebidos
- **5 piscadas rápidas**: Dados enviados com sucesso
- **10 piscadas muito rápidas**: Erro no envio

## 🌐 Comunicação de Rede

### Endpoints da API
- **URL**: Configurável via build flags
- **Método**: POST
- **Content-Type**: application/json
- **Headers**: Padrão HTTP

### Gerenciamento WiFi
- **Conexão**: Sob demanda (quando há dados)
- **Desconexão**: Automática após envio
- **Timeout**: Proteção contra travamentos
- **Retry**: Sem retry automático (economia energia)

## 🚀 Como Configurar e Usar

### 1. Configuração Inicial
```bash
cd Gateway
```

### 2. Editar Configurações WiFi
Edite o arquivo `platformio.ini`:
```ini
build_flags = 
    -DWIFI_SSID='"MinhaRedeWiFi"'
    -DWIFI_PASSWORD='"MinhaSenha"'
    -DAPI_ENDPOINT='"http://192.168.1.100:8000/gateway/device-data"'
```

### 3. Compilação
```bash
pio run
```

### 4. Upload
```bash
pio run --target upload
```

### 5. Monitoramento
```bash
pio device monitor -b 115200
```

## 📊 Logs do Sistema

### Inicialização
```
🩺 VitalSync Gateway Iniciando...
[ETAPA 1] Inicializando módulo LoRa...
✅ LoRa inicializado com sucesso!
[GATEWAY] Sistema pronto - Modo escuta LoRa ativo
```

### Recepção de Dados
```
[GATEWAY] Dados recebidos via LoRa!
Status da recepção: Success
Dados brutos recebidos: {"id":"T001","hr":72,"ox":97,"ps":120,"pd":80,"temp":36.5}
✅ Dados válidos recebidos!
Device ID: ESP32_TRANSMITTER_001
Heart Rate: 72 BPM
Oxygen Level: 97%
Blood Pressure: 120/80
Temperature: 36.5°C
```

### Envio para API
```
[NETWORK] Conectando ao WiFi...
✅ WiFi conectado com sucesso!
IP: 192.168.1.50
[NETWORK] Enviando dados para API...
✅ Dados enviados com sucesso para API!
[NETWORK] Desconectando WiFi...
```

## 🔧 Debug e Troubleshooting

### Problemas Comuns

#### 1. WiFi não Conecta
- Verificar SSID e senha
- Verificar sinal WiFi
- Timeout configurado adequadamente

#### 2. API não Responde
- Verificar se servidor está rodando
- Verificar endpoint URL
- Verificar firewall/portas

#### 3. LoRa não Recebe
- Verificar conexões físicas
- Verificar antena
- Verificar se Transmitter está enviando

### Diagnósticos
```cpp
// Teste de WiFi
Status WiFi: CONECTADO/DESCONECTADO
IP obtido: 192.168.1.50
Signal strength: -45 dBm

// Teste de API
HTTP Response: 200 OK
Response time: 150ms

// Teste LoRa
Dados recebidos: SIM/NÃO
JSON válido: SIM/NÃO
```

## 📈 Otimizações Implementadas

### Economia de Energia
- **WiFi**: Ligado apenas quando necessário
- **HTTP**: Timeout otimizado
- **LoRa**: Modo escuta contínua eficiente

### Performance
- **JSON parsing**: Otimizado para dados compactos
- **Validação**: Ranges médicos verificados
- **Memory**: Gerenciamento eficiente de strings

## 🔒 Segurança

### Implementado
- **Validação de dados**: Ranges médicos
- **Timeout protection**: Evita travamentos
- **Error handling**: Tratamento robusto de erros

### Planejado
- [ ] **HTTPS**: Comunicação criptografada
- [ ] **API Key**: Autenticação na API
- [ ] **Certificate validation**: Verificação SSL

## 📝 Configurações Avançadas

### Build Flags Disponíveis
```ini
-DWIFI_SSID='"..."'           # SSID da rede WiFi
-DWIFI_PASSWORD='"..."'       # Senha da rede WiFi  
-DAPI_ENDPOINT='"..."'        # URL da API
-DWIFI_TIMEOUT_MS=10000       # Timeout WiFi (ms)
-DHTTP_TIMEOUT_MS=5000        # Timeout HTTP (ms)
```

### Exemplo para Produção
```ini
build_flags = 
    -DWIFI_SSID='"VitalSync_Network"'
    -DWIFI_PASSWORD='"SecurePass123"'
    -DAPI_ENDPOINT='"https://api.vitalsync.com/gateway/device-data"'
    -DWIFI_TIMEOUT_MS=15000
    -DHTTP_TIMEOUT_MS=8000
```

## 📊 Status de Desenvolvimento

- [x] **Recepção LoRa**: Implementada e testada
- [x] **Parsing JSON**: Funcionando com validação
- [x] **WiFi Management**: Dinâmico e otimizado
- [x] **HTTP POST**: Comunicação com API
- [x] **Error handling**: Robusto
- [x] **LED Status**: Indicação visual
- [ ] **HTTPS**: Planejado
- [ ] **Authentication**: Futuro
- [ ] **Configuration UI**: Futuro

## 🌟 Próximas Funcionalidades

### Comunicação
- [ ] **MQTT**: Protocolo alternativo
- [ ] **LoRaWAN**: Protocolo padrão LoRa
- [ ] **5G/LTE**: Conectividade celular

### Interface
- [ ] **Display**: Status visual local
- [ ] **Web interface**: Configuração via browser
- [ ] **Mobile app**: Monitoramento remoto

---

**Para mais informações sobre o Transmitter e API, consulte TRANSMITTER.md e SERVER.md**
