# VitalSync Transmitter - WebSocket Interface

## 📋 Visão Geral

Este é o firmware do **VitalSync Transmitter** com interface WebSocket para controle remoto via tablet. O sistema implementa uma arquitetura de comunicação robusta que permite controle passo-a-passo da coleta de dados dos sensores e transmissão via LoRa.

## 🏗️ Arquitetura

### Componentes Principais

1. **WebSocket Manager**: Gerencia comunicação bidirecional com o tablet
2. **Sensor Manager**: Controla sensores de oximetria e temperatura
3. **LoRa Manager**: Gerencia transmissão de dados via LoRa
4. **State Machine**: Coordena o fluxo de estados do sistema

### Estados da Máquina

```
IDLE → HANDSHAKE_OK → SENSORS_ON → READING_OXI → SENSORS_ON → 
READING_TEMP → READY_TO_SEND → SENDING → IDLE
```

## 🔧 Hardware Necessário

### ESP32 DevKit v1
- **Microcontrolador**: ESP32-WROOM-32
- **Flash**: 4MB
- **RAM**: 520KB
- **Wi-Fi**: 802.11 b/g/n

### Sensores
- **MAX30100/MAX30102**: Oxímetro de pulso (I²C)
- **LM35**: Sensor de temperatura analógico (ADC)

### Módulo LoRa
- **E32-TTL-100**: Transceptor LoRa 433MHz
- **Interface**: UART (Serial)

### Pinos de Conexão

| Componente | Pino ESP32 | Função |
|------------|------------|---------|
| MAX30100 SDA | GPIO21 | I²C Data |
| MAX30100 SCL | GPIO22 | I²C Clock |
| LM35 OUT | GPIO36 | ADC Input |
| E32 TX | GPIO16 | UART RX |
| E32 RX | GPIO17 | UART TX |
| E32 M0 | GPIO5 | Mode Control |
| E32 M1 | GPIO4 | Mode Control |
| E32 AUX | GPIO2 | Auxiliary |
| Sensor Power | GPIO23 | Power Control |
| LoRa Power | GPIO22 | Power Control |

## 📦 Instalação e Compilação

### Pré-requisitos

1. **PlatformIO IDE** ou **PlatformIO CLI**
2. **Python 3.7+** (para PlatformIO)
3. **Git** (para clonar repositórios)

### Dependências (Automaticamente instaladas)

```ini
bblanchon/ArduinoJson@^7.0.0
xreef/EByte LoRa E32 library@^1.5.13
oxullo/MAX30100lib@^1.2.1
me-no-dev/ESP Async WebServer@^1.2.3
me-no-dev/AsyncTCP@^1.1.1
lorol/LittleFS_esp32@^1.0.6
```

### Compilação

1. **Clonar o projeto**:
```bash
cd VitalSync-Firmware/Transmitter/Main
```

2. **Compilar**:
```bash
pio build
```

3. **Upload do firmware**:
```bash
pio upload
```

4. **Upload do sistema de arquivos** (opcional):
```bash
pio run --target uploadfs
```

5. **Monitor serial**:
```bash
pio device monitor
```

## 🖥️ Interface Web

### Acesso
1. O ESP32 cria um Access Point: **VitalSync-XXXX**
2. Conecte o tablet/dispositivo ao Wi-Fi
3. Acesse: **http://192.168.4.1**

### Funcionalidades
- 🔐 **Autenticação**: PIN de 6 dígitos
- 📊 **Status em tempo real**: Estado da máquina e progresso
- 🎛️ **Controles passo-a-passo**: Botões para cada operação
- 📝 **Log detalhado**: Eventos com timestamps
- 🔄 **Reconexão automática**: WebSocket resiliente

## 📡 Protocolo de Comunicação

### Comandos do Tablet
```json
{"cmd": "handshake", "pin": "123456"}
{"cmd": "start", "session": "abc123..."}
{"cmd": "read_oximeter", "count": 50, "session": "abc123..."}
{"cmd": "read_temp", "count": 50, "session": "abc123..."}
{"cmd": "send_data", "session": "abc123..."}
{"cmd": "close", "session": "abc123..."}
```

### Eventos do ESP32
```json
{"evt": "handshake_ok", "dev": "VitalSync-TX", "fw": "1.0.0", "session": "..."}
{"evt": "state", "value": "SENSORS_ON"}
{"evt": "oximeter_progress", "i": 25, "n": 50}
{"evt": "oximeter_done", "n": 47}
{"evt": "temp_progress", "i": 30, "n": 50}
{"evt": "temp_done", "n": 50}
{"evt": "lora_tx", "ok": true, "rssi": -85, "snr": 8.2}
{"evt": "error", "code": "SENSOR_TIMEOUT", "detail": "MAX30100"}
{"evt": "closed"}
```

## ⚙️ Configuração

### Parâmetros (NVS)
- **PIN de segurança**: `123456` (padrão)
- **Nome do dispositivo**: `VitalSync-TX`
- **Máximo de amostras**: 50
- **Intervalos de amostragem**: 30ms (oxímetro), 50ms (temperatura)

### Timeouts
- **Sessão**: 5 minutos
- **Leitura de sensores**: 5 segundos
- **Transmissão LoRa**: 4 segundos

## 🔋 Gerenciamento de Energia

### Sequência de Energia Otimizada
1. **Estado idle**: Apenas Wi-Fi ativo
2. **Comando start**: Liga sensores (GPIO23)
3. **Leituras**: Sensores ativos, LoRa desligado
4. **Transmissão**: Desliga sensores, liga LoRa (GPIO22)
5. **Após envio**: Desliga LoRa, volta ao idle

### Consumo Estimado
- **Idle**: ~80mA (Wi-Fi + ESP32)
- **Sensores ativos**: ~120mA
- **Transmissão LoRa**: ~140mA (picos de 250mA)

## 🛠️ Desenvolvimento e Debug

### Logs Serial
```bash
pio device monitor --baud 115200
```

### Estrutura de Logs
```
[WS] WebSocket Manager
[SENSORS] Sensor Manager  
[LORA] LoRa Manager
[SETUP] Sistema
[LOOP] Loop principal
[TASK] Tasks assíncronas
```

### Build Flags
```ini
-DARDUINOJSON_USE_LONG_LONG=1  # Suporte a int64
-DCORE_DEBUG_LEVEL=3           # Debug verboso
-DBOARD_HAS_PSRAM             # Suporte PSRAM
```

## 🧪 Testes

### Teste Manual (Passo-a-passo)
1. **Setup**: Upload firmware e monitor serial
2. **Conexão**: Conectar tablet ao Wi-Fi do ESP32
3. **Handshake**: Inserir PIN na interface web
4. **Fluxo completo**:
   - Iniciar sessão → `SENSORS_ON`
   - Ler oxímetro 50x → progresso + `oximeter_done`
   - Ler temperatura 50x → progresso + `temp_done`
   - Enviar dados → `READY_TO_SEND` → `SENDING` → `lora_tx`
   - Finalizar → `IDLE`

### Teste de Erros
- PIN incorreto → `PIN_INVALID`
- Comando fora de estado → `BAD_STATE`
- Sensor desconectado → `SENSOR_TIMEOUT`
- Falha LoRa → `LORA_FAIL`

### Performance
- **Heap livre**: >150KB após inicialização
- **Stack usage**: <4KB por task
- **Latência WebSocket**: <50ms
- **Tempo de leitura**: ~1.5s (50 amostras cada sensor)

## 🔒 Segurança

### Autenticação
- PIN de 6 dígitos configurável
- Sessão efêmera com TTL de 5 minutos
- Rate limiting: 2 comandos/segundo

### Rede
- Access Point local (sem internet)
- Alcance limitado (~30m em campo aberto)
- Sem armazenamento de credenciais

## 📄 Payload LoRa

### Formato Otimizado (≤58 bytes)
```json
{
  "id": "TR-001",
  "v": 1,
  "ts": 1234567890,
  "o": [[72,98],[75,97],[73,96]],
  "t": 36.8
}
```

### Campos
- `id`: ID único do transmissor
- `v`: Versão do payload
- `ts`: Timestamp Unix
- `o`: Últimas 5 amostras oxímetro [BPM, SpO2]
- `t`: Temperatura média (°C)

## 🚀 Próximas Funcionalidades

### v1.1 (Planejado)
- [ ] Interface de configuração via web
- [ ] Histórico de sessões
- [ ] Calibração automática de sensores
- [ ] Modo de economia de energia avançado

### v1.2 (Futuro)
- [ ] OTA (Over-The-Air) updates
- [ ] Múltiplos perfis de usuário
- [ ] Relatórios em PDF
- [ ] Integração com apps móveis

## 📞 Suporte e Contribuição

### Problemas Comuns
1. **Sensor não detectado**: Verificar conexões I²C
2. **LoRa não transmite**: Verificar antena e alimentação
3. **WebSocket desconecta**: Verificar força do sinal Wi-Fi

### Como Contribuir
1. Fork do repositório
2. Criar branch para feature (`git checkout -b feature/nova-funcionalidade`)
3. Commit das alterações (`git commit -am 'Adiciona nova funcionalidade'`)
4. Push para branch (`git push origin feature/nova-funcionalidade`)
5. Abrir Pull Request

### Reportar Bugs
Use o sistema de Issues do GitHub com as seguintes informações:
- Versão do firmware
- Hardware utilizado
- Passos para reproduzir
- Logs do monitor serial

## 📜 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 👥 Autores

- **Equipe VitalSync** - *Desenvolvimento inicial*

## 🙏 Agradecimentos

- Comunidade ESP32
- Bibliotecas open-source utilizadas
- Colaboradores e testadores

---

**📧 Contato**: vitalsync@example.com  
**🌐 Website**: https://vitalsync.example.com  
**📱 Discord**: VitalSync Community
