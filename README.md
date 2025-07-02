# 🩺 VitalSync  
**Atendimento remoto de pacientes crônicos**

## 📘 Visão Geral
VitalSync é um sistema de monitoramento remoto para pacientes com doenças crônicas. Utiliza comunicação LoRa para transmitir sinais vitais coletados por sensores até um gateway, que por sua vez envia os dados para uma API HTTP.

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
