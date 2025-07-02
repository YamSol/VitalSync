# 📡 VitalSync Transmitter

**Dispositivo de coleta e transmissão de sinais vitais**

## 📋 Descrição

O Transmitter é o dispositivo usado pelo paciente para coletar sinais vitais e transmiti-los via LoRa para o Gateway. Implementa economia de energia através de Deep Sleep entre transmissões.

## 🔧 Hardware Necessário

### Componentes Principais
- **ESP32 DevKit V1**
- **E32-433T20D (LoRa)**
- **Sensores de Sinais Vitais**

### Sensores Implementados/Planejados
- ✅ **Temperatura**: Dados simulados (DS18B20 planejado)
- ✅ **Batimentos Cardíacos**: Dados simulados (MAX30100 planejado)
- ✅ **Oxigenação (SpO2)**: Dados simulados (MAX30100 planejado)
- ✅ **Pressão Arterial**: Dados simulados

## 🔌 Diagrama de Conexões

```
E32-433T20D     ESP32 DevKit V1
VCC      →      3.3V (IMPORTANTE: NÃO usar 5V!)
GND      →      GND
TX       →      GPIO 17 (RXD2)
RX       →      GPIO 16 (TXD2)
M0       →      GPIO 2  (removido na versão atual)
M1       →      GPIO 4  (removido na versão atual)
AUX      →      GPIO 25 (removido na versão atual)
```

## 📊 Arquitetura do Código

### Estrutura Modular
```
src/
├── main.cpp          # Fluxo principal do sistema
├── sensors.h/cpp     # Gerenciamento dos sensores
└── lora.h/cpp        # Controle do módulo LoRa
```

### Classes Principais
- **`SensorManager`**: Lê e valida dados dos sensores
- **`LoRaManager`**: Controla comunicação LoRa

## 🔄 Fluxo de Operação

```
┌─────────────────┐
│  [BOOT/WAKE]    │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Inicializa      │
│ Sensores        │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Lê Sinais       │
│ Vitais          │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Valida Dados    │
│ (ranges médicos)│
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Gera JSON       │
│ Compacto        │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Liga LoRa       │
│ E32             │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Transmite       │
│ Dados           │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Desliga LoRa    │
└─────────┬───────┘
          │
┌─────────▼───────┐
│ Deep Sleep      │
│ (30 segundos)   │
└─────────────────┘
```

## 📦 Formato de Dados

### JSON Compacto (LoRa - máx 58 bytes)
```json
{"id":"T001","hr":72,"ox":97,"ps":120,"pd":80,"temp":36.5}
```

### Mapeamento de Campos
- `id`: device_id compactado
- `hr`: heart_rate
- `ox`: oxygen_level  
- `ps`: pressure.systolic
- `pd`: pressure.diastolic
- `temp`: temperature

## ⚙️ Configurações

### Temporização
```cpp
#define SLEEP_TIME_SECONDS 30      // Deep Sleep entre transmissões
#define SENSOR_READ_TIMEOUT 5000   // Timeout para leitura de sensores
```

### Validação de Ranges Médicos
```cpp
Heart Rate: 30-200 BPM
Oxygen Level: 70-100%
Temperature: 30.0-45.0°C
Systolic BP: 70-250 mmHg
Diastolic BP: 40-150 mmHg
```

### Comunicação
```cpp
Serial Baud Rate: 115200
LoRa Baud Rate: 9600
LoRa Pins: RX=16, TX=17
```

## 🔋 Economia de Energia

### Estratégias Implementadas
1. **Deep Sleep**: ESP32 dorme entre transmissões
2. **LoRa Shutdown**: Módulo E32 desligado quando não usado
3. **Ciclo Otimizado**: Tempo ativo minimizado

### Consumo Estimado
- **Ativo**: ~100mA (durante transmissão)
- **Deep Sleep**: ~10µA
- **Autonomia**: Estimada em várias semanas com bateria adequada

## 🚀 Como Compilar e Usar

### 1. Configuração do Ambiente
```bash
# Instalar PlatformIO
cd Transmitter/src/Main
pio lib install "EByte LoRa E32 library"
pio lib install "ArduinoJson"
```

### 2. Compilação
```bash
pio run
```

### 3. Upload
```bash
pio run --target upload
```

### 4. Monitoramento
```bash
pio device monitor -b 115200
```

## 📈 Dados Simulados Atuais

### Valores Base (com variação aleatória)
```cpp
Heart Rate: 70 ± 10 BPM
Oxygen Level: 97 ± 3%
Temperature: 36.5 ± 1.5°C
Systolic: 120 ± 10 mmHg
Diastolic: 80 ± 8 mmHg
```

## 🔧 Debug e Troubleshooting

### Logs Detalhados
```
[ETAPA 1] Inicializando sensores...
[ETAPA 2] Lendo sinais vitais...
[ETAPA 3] Preparando dados para transmissão...
[ETAPA 4] Ligando módulo LoRa...
[ETAPA 5] Enviando dados ao Gateway...
[ETAPA 6] Desligando módulo LoRa...
[ETAPA 7] Entrando em Deep Sleep...
```

### Problemas Comuns
1. **Erro 14**: Mensagem muito grande (>58 bytes)
2. **Falha na transmissão**: Verificar conexões LoRa
3. **Dados inválidos**: Verificar ranges dos sensores

## 📝 Próximas Implementações

### Sensores Reais
- [ ] **DS18B20**: Temperatura corporal
- [ ] **MAX30100**: Oximetria e batimentos
- [ ] **Sensor de pressão**: Implementação com manguito

### Melhorias de Sistema
- [ ] **RTC**: Timestamps precisos
- [ ] **Criptografia**: Segurança dos dados
- [ ] **Confirmação**: ACK do Gateway
- [ ] **Configuração dinâmica**: Via WiFi

## 🚨 Importante

⚠️ **Uso Médico**: Este é um protótipo educacional. Para uso clínico real é necessário:
- Certificação médica dos dispositivos
- Validação clínica dos sensores
- Conformidade regulatória (ANVISA)

## 📊 Status de Desenvolvimento

- [x] **Estrutura básica**: Implementada
- [x] **Comunicação LoRa**: Funcional  
- [x] **Deep Sleep**: Implementado
- [x] **Dados simulados**: Funcionando
- [x] **JSON compacto**: Otimizado para LoRa
- [ ] **Sensores reais**: Em desenvolvimento
- [ ] **Criptografia**: Planejado
- [ ] **Interface de configuração**: Futuro

---

**Para mais informações sobre o Gateway e API, consulte GATEWAY.md e SERVER.md**
