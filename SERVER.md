# 🖥️ VitalSync API Server

**Backend REST para recebimento e processamento de dados médicos**

## 📋 Descrição

O Server é uma API REST desenvolvida em FastAPI (Python) que recebe dados dos Gateways, processa sinais vitais, gera alertas médicos e armazena informações para análise posterior.

## 🏗️ Arquitetura

### Tecnologias
- **FastAPI**: Framework web moderno e rápido
- **Python 3.8+**: Linguagem de programação
- **Pydantic**: Validação de dados
- **Uvicorn**: Servidor ASGI
- **JSON**: Armazenamento de dados

### Estrutura do Projeto
```
Server/
├── apirest/
│   ├── __init__.py           # Módulo Python
│   ├── mainapi.py           # API principal
│   └── main.py              # Exemplo simples
├── data/                    # Dados armazenados (auto-criado)
├── venv/                    # Ambiente virtual Python
├── requirements.txt         # Dependências
├── setup.sh                # Script de configuração
└── README.md               # Documentação local
```

## 📡 Endpoints da API

### Gateway ESP32
```http
POST /gateway/device-data
Content-Type: application/json
x-api-key: expected-api-key

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

### Autenticação (Mock)
- `POST /auth/login` - Login de usuário
- `POST /auth/logout` - Logout de usuário  
- `GET /auth/profile` - Perfil do usuário

### Pacientes (CRUD)
- `GET /patients` - Listar pacientes
- `GET /patients/{id}` - Obter paciente específico
- `POST /patients` - Criar paciente
- `PUT /patients/{id}` - Atualizar paciente
- `DELETE /patients/{id}` - Deletar paciente

### Dados Médicos
- `GET /patients/{id}/medical-data` - Histórico médico
- `GET /patients/{id}/medical-data/latest` - Dados mais recentes

## 🔄 Fluxo de Processamento

```
[Gateway] → [HTTP POST] → [API Server]
                             │
                    ┌────────▼────────┐
                    │   Validação     │
                    │   dos Dados     │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │   Análise de    │
                    │   Alertas       │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  Armazenamento  │
                    │   em Arquivo    │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │   Resposta      │
                    │   HTTP 200      │
                    └─────────────────┘
```

## 🚨 Sistema de Alertas

### Validação de Ranges Médicos
```python
Heart Rate: 30-200 BPM
- < 60 BPM: ⚠️ Bradicardia detectada
- > 100 BPM: ⚠️ Taquicardia detectada

Oxygen Level: 70-100%
- < 95%: ⚠️ Saturação baixa

Temperature: 30.0-45.0°C
- > 37.5°C: ⚠️ Febre detectada
- < 35.0°C: ⚠️ Hipotermia detectada

Blood Pressure:
- Systolic > 140 ou Diastolic > 90: ⚠️ Hipertensão
- Systolic < 90 ou Diastolic < 60: ⚠️ Hipotensão
```

### Logs de Alertas
```
🚨 ALERTAS para ESP32_TRANSMITTER_001:
  ⚠️ Taquicardia detectada: 105 BPM
  ⚠️ Hipertensão: 150/95 mmHg
```

## 💾 Armazenamento de Dados

### Estrutura do Arquivo JSON
```json
{
  "received_at": "2025-01-02T10:00:00Z",
  "api_version": "1.0.0",
  "data": {
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
}
```

### Nomenclatura de Arquivos
```
data/vitalsigns_{device_id}_{timestamp}.json

Exemplo:
data/vitalsigns_ESP32_TRANSMITTER_001_20250102_100000.json
```

## 🚀 Configuração e Execução

### 1. Instalação Automática
```bash
cd Server
chmod +x setup.sh
./setup.sh
```

### 2. Instalação Manual
```bash
cd Server

# Criar ambiente virtual
python3 -m venv venv

# Ativar ambiente (Linux/macOS)
source venv/bin/activate

# Ativar ambiente (Fish shell)
source venv/bin/activate.fish

# Instalar dependências
pip install -r requirements.txt
```

### 3. Execução
```bash
# Com ambiente virtual ativado
python -m uvicorn apirest.mainapi:app --reload --host 0.0.0.0 --port 8000
```

### 4. Acesso
- **API Docs**: http://localhost:8000/docs
- **ReDoc**: http://localhost:8000/redoc
- **Health Check**: http://localhost:8000/health

## 📊 Interfaces de Monitoramento

### Swagger UI (http://localhost:8000/docs)
- **Documentação interativa** da API
- **Teste de endpoints** diretamente no browser
- **Esquemas de dados** detalhados
- **Exemplos de requisições**

### Logs do Servidor
```
INFO:     📡 Dados recebidos do dispositivo: ESP32_TRANSMITTER_001
INFO:     💓 Heart Rate: 72 BPM
INFO:     🫁 Oxygen Level: 97%
INFO:     🩸 Blood Pressure: 120/80 mmHg
INFO:     🌡️ Temperature: 36.5°C
INFO:     ✅ Processamento concluído para ESP32_TRANSMITTER_001
```

## 🔒 Segurança

### Implementado
- **CORS**: Configurado para aceitar ESP32
- **Validação de dados**: Pydantic models
- **API Key**: Header obrigatório para Gateway
- **Error handling**: Tratamento robusto

### Configuração de Segurança
```python
# API Key para Gateway
API_KEY = "expected-api-key"  # Alterar em produção

# CORS
allow_origins = ["*"]  # Restringir em produção
```

## 🐛 Debug e Troubleshooting

### Problemas Comuns

#### 1. Porta em Uso
```bash
# Encontrar processo na porta 8000
sudo lsof -i :8000

# Matar processo
sudo kill -9 <PID>
```

#### 2. Dependências não Instaladas
```bash
# Verificar ambiente virtual
which python
# Deve mostrar caminho do venv

# Reinstalar dependências
pip install --upgrade -r requirements.txt
```

#### 3. Erro de Módulo
```bash
# Verificar estrutura
ls -la apirest/
# Deve conter __init__.py

# Executar do diretório correto
cd Server
python -m uvicorn apirest.mainapi:app
```

### Logs de Debug
```python
# Nível de logging
logging.basicConfig(level=logging.INFO)

# Logs disponíveis
DEBUG: Detalhes de processamento
INFO: Fluxo normal da aplicação
WARNING: Alertas médicos
ERROR: Erros de processamento
```

## 📈 Monitoramento e Estatísticas

### Endpoint de Estatísticas
```http
GET /stats

Response:
{
  "total_files": 150,
  "unique_devices": 3,
  "devices": ["ESP32_TRANSMITTER_001", "ESP32_TRANSMITTER_002"],
  "latest_files": ["vitalsigns_ESP32_TRANSMITTER_001_20250102_100000.json"]
}
```

### Health Check
```http
GET /health

Response:
{
  "status": "healthy",
  "timestamp": "2025-01-02T10:00:00Z",
  "data_directory": "data",
  "files_count": 150
}
```

## ⚙️ Configurações Avançadas

### Variáveis de Ambiente
```bash
# .env file (criar se necessário)
API_HOST=0.0.0.0
API_PORT=8000
API_KEY=your-secure-api-key
DATA_DIRECTORY=data
LOG_LEVEL=INFO
```

### Produção
```python
# Para produção, modificar:
1. API Key segura
2. CORS restrito
3. HTTPS obrigatório
4. Banco de dados real
5. Rate limiting
6. Autenticação JWT
```

## 🔮 Próximas Funcionalidades

### Banco de Dados
- [ ] **PostgreSQL**: Armazenamento persistente
- [ ] **MongoDB**: Dados não-relacionais
- [ ] **InfluxDB**: Dados de série temporal

### Análise de Dados
- [ ] **Dashboard**: Interface web
- [ ] **Gráficos**: Visualização temporal
- [ ] **Machine Learning**: Detecção de padrões
- [ ] **Relatórios**: Exportação PDF

### Integração
- [ ] **WebSocket**: Tempo real
- [ ] **MQTT**: Protocolo IoT
- [ ] **Notification API**: Alertas push
- [ ] **EMR Integration**: Sistemas hospitalares

## 📊 Status de Desenvolvimento

- [x] **API REST**: Implementada e funcional
- [x] **Validação**: Ranges médicos verificados
- [x] **Alertas**: Sistema de notificação
- [x] **Armazenamento**: Arquivos JSON
- [x] **Documentação**: Swagger integrada
- [x] **CORS**: Configurado para ESP32
- [ ] **Banco de dados**: Planejado
- [ ] **Interface web**: Futuro
- [ ] **Autenticação**: Planejado

## 🌟 Exemplos de Uso

### Teste Manual
```bash
# Testar endpoint do Gateway
curl -X POST "http://localhost:8000/gateway/device-data" \
  -H "Content-Type: application/json" \
  -H "x-api-key: expected-api-key" \
  -d '{
    "device_id": "ESP32_TRANSMITTER_001",
    "heart_rate": 75,
    "oxygen_level": 98,
    "pressure": {"systolic": 120, "diastolic": 80},
    "temperature": 36.7,
    "timestamp": "2025-01-02T10:00:00"
  }'
```

### Response Esperada
```json
{
  "success": true,
  "message": "Dados recebidos com sucesso do dispositivo ESP32_TRANSMITTER_001",
  "data": {
    "device_id": "ESP32_TRANSMITTER_001",
    "processed_at": "2025-01-02T10:00:00Z"
  },
  "received_at": "2025-01-02T10:00:00Z"
}
```

---

**Para mais informações sobre Transmitter e Gateway, consulte TRANSMITTER.md e GATEWAY.md**
