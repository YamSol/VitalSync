
# Gateway - Sistema de Monitoramento Remoto

## Descrição do Projeto

O **Gateway** é responsável por receber os dados dos **Transmissores** (dispositivos de medição de sinais vitais) via **LoRa**, processar essas informações e encaminhá-las para o servidor. A comunicação LoRa será a tecnologia escolhida devido à sua longa distância de comunicação e baixo consumo de energia, ideal para pacientes em áreas remotas.

## 1. Tipo de LoRa a Ser Usado

A tecnologia escolhida para a comunicação entre o Gateway e o Transmissor é **LoRa SX1278** com o **módulo E32-433T20D**, que opera na frequência de 433 MHz.

### Características:
- **Frequência:** 433 MHz
- **Distância:** Até 20 km (em condições ideais)
- **Taxa de Transmissão:** Aproximadamente 300 bps a 10 kbps
- **Interface:** SPI

## 2. Dependências

Para a comunicação LoRa no **ESP32**, a biblioteca recomendada é a **LoRa** oficial.

### Instalação no PlatformIO:
```ini
[env:esp32dev]
platform = espressif32
framework = arduino
board = esp32dev
lib_deps = 
  bblanchon/ArduinoJson@^6.18.0
  Sandeep Mistry/LoRa@^1.8.0
```

## 3. Estrutura de Dados Enviados

Os dados enviados pelo **Transmissor** (sensores de sinais vitais) serão enviados no formato JSON compactado.

### Exemplo de Estrutura de Dados:
```json
{
  "device_id": "12345",              
  "heart_rate": 72,                   
  "oxygen_level": 97,                 
  "pressure": {"systolic": 120, "diastolic": 80},
  "temperature": 36.7,                
  "timestamp": "2025-06-10T10:00:00"  
}
```

### Criptografia:
Utilizaremos **AES-128** para criptografar os dados antes de enviá-los. A chave de criptografia será compartilhada entre o **Transmissor** e o **Gateway**.

## 4. Estrutura do Código Fonte

A estrutura de código do **Gateway** segue o padrão **MVC (Model-View-Controller)**, separando a lógica de controle, visualização e manipulação dos dados.

### Estrutura de Diretórios:
```plaintext
/src
  ├── /controller           
  │    ├── gatewayController.cpp
  │    └── gatewayController.h
  ├── /model                
  │    ├── loraModel.cpp
  │    └── loraModel.h
  ├── /view                 
  │    ├── gatewayView.cpp
  │    └── gatewayView.h
  └── main.cpp              
```

## 5. Considerações Finais

- **Conectividade LoRa:** O Gateway deve ser capaz de receber dados dos transmissores a distâncias de até 20 km (em condições ideais) com uso eficiente da largura de banda.
- **Criptografia:** A utilização de criptografia AES-128 garantirá a segurança dos dados transmitidos.
- **Uso de PlatformIO:** A plataforma PlatformIO e a biblioteca LoRa permitem uma integração rápida e eficiente com o ESP32 para a implementação de comunicação LoRa.

Se precisar de mais informações ou ajustes, estou à disposição!
