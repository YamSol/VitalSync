# 🩺 VitalSync  
**Sistema de Monitoramento Remoto de Sinais Vitais**

## 📘 Visão Geral

VitalSync é um sistema completo de monitoramento remoto para pacientes com doenças crônicas. Utiliza comunicação LoRa para transmitir sinais vitais coletados por sensores até um gateway, que envia os dados para uma API REST.

## 🏗️ Arquitetura do Sistema

```
[Paciente] → [Transmitter ESP32] ↔ [LoRa] ↔ [Gateway ESP32] → [WiFi] → [API Server]
```

## ⚙️ Componentes

### 📡 Transmitter (Dispositivo do Paciente)
- **ESP32 DevKit**
- **E32 TTL-100 (LoRa 433MHz)**
- **Sensores de Sinais Vitais:**
  - Temperatura (DS18B20)
  - Oximetria e Batimentos (MAX30100/30102)
  - Pressão Arterial (simulada)

### 🌐 Gateway (Receptor + Bridge WiFi)
- **ESP32 DevKit**
- **E32 TTL-100 (LoRa 433MHz)**
- **Conexão WiFi** para API
- **LED de Status**

### 🖥️ API Server (Backend)
- **FastAPI (Python)**
- **Endpoints REST**
- **Armazenamento de dados**
- **Interface Web (Swagger)**

## 🔄 Fluxo de Funcionamento

### 1. TRANSMITTER
1. ✅ Inicializa sensores de sinais vitais
2. ✅ Coleta dados (temperatura, oxigenação, batimentos)
3. ✅ Gera JSON compacto (respeitando limite de 58 bytes LoRa)
4. ✅ Transmite via LoRa para Gateway
5. ✅ Entra em Deep Sleep (economia de energia)

### 2. GATEWAY
1. ✅ Escuta dados via LoRa continuamente
2. ✅ Recebe e valida JSON compacto
3. ✅ Expande dados para formato completo da API
4. ✅ Conecta ao WiFi
5. ✅ Envia POST HTTP para API Server
6. ✅ Desconecta WiFi e retorna ao modo escuta

### 3. API SERVER
1. ✅ Recebe dados via HTTP POST
2. ✅ Valida ranges de sinais vitais
3. ✅ Armazena dados em arquivos JSON
4. ✅ Gera alertas para valores anômalos
5. ✅ Fornece interface web para visualização

## 📦 Formato dos Dados

### JSON Compacto (LoRa - 58 bytes max)
```json
{"id":"T001","hr":72,"ox":97,"ps":120,"pd":80,"temp":36.5}
```

### JSON Completo (API)
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

## 🚀 Como Executar

### 1. Transmitter
```bash
cd Transmitter/src/Main
pio run --target upload
```

### 2. Gateway
```bash
# Configure WiFi no platformio.ini
cd Gateway
pio run --target upload
```

### 3. API Server
```bash
cd Server
source venv/bin/activate.fish  # ou activate para bash
python -m uvicorn apirest.mainapi:app --reload --host 0.0.0.0 --port 8000
```

## 📊 Interfaces de Monitoramento

- **API Docs**: http://localhost:8000/docs
- **Logs do Transmitter**: Serial Monitor (115200 baud)
- **Logs do Gateway**: Serial Monitor (115200 baud)

## ✅ Status do Projeto

### Implementado
- [x] Transmitter com sensores simulados
- [x] Comunicação LoRa bi-direcional funcional
- [x] Gateway com validação de dados
- [x] API Server completa
- [x] Economia de energia (Deep Sleep)
- [x] JSON compacto para otimizar LoRa
- [x] Validação de ranges médicos
- [x] Sistema de alertas

### Em Desenvolvimento
- [ ] Integração com sensores reais
- [ ] Interface web para visualização
- [ ] Sistema de autenticação
- [ ] Criptografia LoRa
- [ ] Banco de dados persistente

## 📁 Estrutura do Projeto

```
VitalSync/
├── README.md                 # Este arquivo
├── TRANSMITTER.md           # Documentação do Transmitter
├── GATEWAY.md               # Documentação do Gateway  
├── SERVER.md                # Documentação da API
├── TODO.md                  # Lista de tarefas
├── Transmitter/             # Código ESP32 Transmitter
├── Gateway/                 # Código ESP32 Gateway
└── Server/                  # API REST Python
```

## 🔧 Requisitos Técnicos

### Hardware
- 2x ESP32 DevKit
- 2x E32-433T20D (LoRa)
- Sensores: DS18B20, MAX30100
- Antenas 433MHz

### Software
- PlatformIO
- Python 3.8+
- Arduino libraries: LoRa_E32, ArduinoJson

## 🚨 Segurança e Medicina

⚠️ **IMPORTANTE**: Este é um projeto educacional/prototipo. Para uso médico real:
- Implementar criptografia end-to-end
- Certificação médica dos dispositivos
- Validação clínica dos sensores
- Conformidade com regulamentações (ANVISA, FDA)

## 📞 Suporte

- **Logs detalhados** em cada componente
- **Documentação específica** em cada módulo
- **Testes de comunicação** implementados
- **Validação de dados** em todas as etapas

---

**Desenvolvido para monitoramento remoto de pacientes crônicos** 🏥

---

## ⚙️ Componentes

### 📡 Transmitter (Paciente)
- **ESP32 DevKit**
- **E32 TTL-100 (LoRa)**
- **Sensores:**
  - Temperatura (ex: DS18B20 ou MLX90614)
  - Batimentos Cardíacos (ex: MAX30100/30102)
  - Oxigenação (SpO2)
  - Pressão Arterial (opcional)

### 🖥️ Gateway (Receptor + Conexão com API)
- **ESP32 DevKit**
- **E32 TTL-100 (LoRa)**
- **WiFi** para envio HTTP
- (Opcional) LED ou Display para status

---

## 🔄 Fluxo de Funcionamento

### TRANSMITTER
1. Inicializa sensores
2. Lê sinais vitais
3. Gera JSON com os dados
4. (Opcional) Compacta/criptografa
5. Liga o módulo LoRa
6. Envia dados ao Gateway
7. Desliga o módulo e entra em deepSleep

### GATEWAY
1. Modo escuta via LoRa
2. Recebe JSON do Transmitter
3. Valida e faz parse dos dados
4. Conecta-se ao WiFi
5. Envia os dados via POST para API HTTP
6. Timeout → desliga WiFi e retorna ao modo LoRa

---

## 📦 Estrutura dos Dados (JSON)

\`\`\`json
{ 
  "device_id": "12345",              
  "heart_rate": 72,                   
  "oxygen_level": 97,                 
  "pressure": {"systolic": 120, "diastolic": 80},
  "temperature": 36.7,                
  "timestamp": "2025-06-10T10:00:00"  
}
\`\`\`

---

## 📂 Tarefas por Dispositivo

### ✅ Transmitter

- [ ] Inicializar sensores (temperatura, batimentos, SpO2, etc.)
- [ ] Ler valores e validar
- [ ] Criar JSON no formato correto
- [ ] (Opcional) Compactar ou criptografar
- [ ] Ligar módulo LoRa (E32)
- [ ] Enviar dados pela UART
- [ ] Desligar LoRa / entrar em deepSleep

### ✅ Gateway

- [ ] Inicializar E32 em modo Wake-up (M0=LOW, M1=LOW)
- [ ] Escutar UART do E32
- [ ] Receber string JSON
- [ ] Validar e converter JSON
- [ ] Conectar ao WiFi
- [ ] Enviar dados via POST para API
- [ ] Desconectar WiFi e voltar ao modo escuta

---

## 📋 Tabela Resumo

| Etapa                            | Transmitter                      | Gateway                                |
|----------------------------------|-----------------------------------|----------------------------------------|
| Inicialização                    | Sensores e LoRa desligado        | Modo Wake-up, LoRa pronto              |
| Leitura de sensores              | Temperatura, batimentos, etc     | —                                      |
| Montar mensagem JSON             | Inclui device_id e timestamp     | Recebe JSON                            |
| Enviar via LoRa                  | UART TX para módulo E32          | UART RX do módulo E32                  |
| Desligar LoRa                    | Após envio                       | Permanece ligado esperando             |
| Conectar WiFi                    | —                                | Ativa após receber dados               |
| Enviar para API HTTP            | —                                | Envia JSON via POST                    |
| Timeout                          | DeepSleep                        | Desliga WiFi após X segundos           |

---

## 🧱 Estrutura de Código Sugerida

- `sensors.cpp / .h` → Leitura dos sensores
- `lora.cpp / .h` → Controle do módulo E32
- `network.cpp / .h` → Envio de dados via HTTP
- `main.ino` → Lógica principal do fluxo

---

## 🔐 Segurança
- Ideal: usar HTTPS na API.
- Usar alguma criptografia entre o Gateway e o Transmitter
---

## ⏱️ Economia de Energia
- Transmitter deve usar `ESP.deepSleep()` entre ciclos.
- Gateway deve desligar WiFi após envio e permanecer no modo escuta LoRa.

---

## 🚀 Próximos Passos

### Para o Transmitter:
- [ ] Implementar leitura real dos sensores
- [ ] Formatar JSON conforme especificado
- [ ] Integrar com E32 e enviar dados

### Para o Gateway:
- [ ] Escutar LoRa UART e receber dados
- [ ] Validar e enviar JSON para a API
- [ ] Controlar WiFi dinamicamente
