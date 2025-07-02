# VitalSync - Transmitter Implementation

## 📋 Descrição
Implementação do dispositivo transmitter do sistema VitalSync, responsável por coletar dados de sensores de sinais vitais e transmiti-los via LoRa para o Gateway.

## 🔧 Arquivos Criados

### Estrutura Modular
- **`sensors.h/cpp`** - Gerenciamento dos sensores (temperatura, batimentos, SpO2, pressão)
- **`lora.h/cpp`** - Controle do módulo LoRa E32
- **`src/main.cpp`** - Lógica principal do fluxo do transmitter

## 📡 Fluxo Implementado

Conforme especificado no README.md principal, o transmitter segue este fluxo:

1. **Inicializa sensores** ✅
2. **Lê sinais vitais** ✅ (dados simulados)
3. **Gera JSON com os dados** ✅
4. **Liga o módulo LoRa** ✅
5. **Envia dados ao Gateway** ✅
6. **Desliga o módulo e entra em deepSleep** ✅

## 📊 Dados Simulados Atualmente

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
  "timestamp": "2025-01-02T123:00:00"
}
```

## 🔌 Conexões Físicas

### Módulo E32 LoRa
- **RX** → GPIO 16 (TXD2)
- **TX** → GPIO 17 (RXD2)  
- **M0** → GPIO 2
- **M1** → GPIO 4
- **AUX** → GPIO 25
- **VCC** → 3.3V
- **GND** → GND

## 📚 Bibliotecas Necessárias (Arduino IDE)

Instale as seguintes bibliotecas através do Library Manager:

1. **LoRa_E32** - para comunicação com módulo E32
2. **ArduinoJson** - para criação e parsing de JSON

## ⚙️ Configurações

### Temporização
- **Deep Sleep**: 30 segundos (configurável em `SLEEP_TIME_SECONDS`)
- **Baud Rate Serial**: 115200
- **Baud Rate E32**: 9600

### Validação de Dados
- Heart Rate: 40-200 BPM
- Oxygen Level: 70-100%
- Temperature: 30-45°C
- Blood Pressure: 70-200 (systolic), 40-120 (diastolic)

## 🔋 Economia de Energia

O sistema implementa economia de energia através de:
- **Deep Sleep**: ESP32 entra em deep sleep entre transmissões
- **Shutdown LoRa**: Módulo E32 é colocado em modo sleep
- **Ciclo otimizado**: Minimiza o tempo ativo do sistema

## 🔧 Como Usar

1. **Hardware**: Conecte o módulo E32 conforme o diagrama de pinos
2. **Software**: 
   - Abra o Arduino IDE
   - Instale as bibliotecas necessárias
   - Carregue o código no ESP32
3. **Monitor Serial**: Configure para 115200 baud para ver os logs

## 📈 Próximas Implementações

- [ ] Integração com sensores reais (DS18B20, MAX30100, etc.)
- [ ] Implementação de RTC para timestamps precisos
- [ ] Criptografia dos dados transmitidos
- [ ] Validação de entrega das mensagens
- [ ] Configuração dinâmica via WiFi

## 🐛 Debug

O sistema fornece logs detalhados via Serial Monitor:
- Status de inicialização
- Leitura dos sensores
- Criação do JSON
- Status do envio LoRa
- Informações de Deep Sleep

## 📝 Notas Importantes

- **Dados Simulados**: Atualmente usa dados estáticos com variação aleatória
- **Teste de Comunicação**: Verifique se o Gateway está recebendo os dados
- **Economia de Energia**: Sistema otimizado para operação com bateria
- **Modularidade**: Código estruturado para fácil manutenção e expansão
