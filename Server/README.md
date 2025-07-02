# 🩺 VitalSync Server

Backend API REST para receber dados de sinais vitais do Gateway ESP32.

## 📋 Requisitos

- **Python**: 3.8 ou superior
- **Sistema Operacional**: Linux, macOS ou Windows

## 🚀 Configuração do Ambiente

### 1. Verificar Python

```bash
python3 --version
# ou
python --version
```

**Versão mínima requerida**: Python 3.8+

### 2. Criar Ambiente Virtual

```bash
# Navegar para o diretório do Server
cd /path/to/VitalSync/Server

# Criar ambiente virtual
python3 -m venv venv

# Ativar ambiente virtual
# No Linux/macOS:
source venv/bin/activate

# No Windows:
# venv\Scripts\activate
```

### 3. Instalar Dependências

```bash
# Com o ambiente virtual ativado
pip install -r requirements.txt
```

### 4. Executar o Servidor

```bash
# Método 1: Usando uvicorn diretamente
uvicorn apirest.mainapi:app --reload --host 0.0.0.0 --port 8000

# Método 2: Usando o arquivo main.py (se disponível)
python apirest/mainapi.py
```

## 📚 Documentação da API

Após iniciar o servidor, acesse:

- **Swagger UI**: http://localhost:8000/docs
- **ReDoc**: http://localhost:8000/redoc
- **API Root**: http://localhost:8000/

## 🔧 Endpoints Principais

### Gateway ESP32
- `POST /gateway/device-data` - Recebe dados do Gateway
  - Requer header: `x-api-key: expected-api-key`

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

## 📊 Teste com Gateway

Para testar a comunicação com o Gateway ESP32, configure no `platformio.ini` do Gateway:

```ini
build_flags = 
    -DAPI_ENDPOINT='"http://localhost:8000/gateway/device-data"'
```

## 🐛 Solução de Problemas

### Erro de Porta em Uso
```bash
# Encontrar processo usando a porta 8000
sudo lsof -i :8000

# Matar processo
sudo kill -9 <PID>
```

### Dependências não Instaladas
```bash
# Verificar se o ambiente virtual está ativo
which python
# Deve mostrar o caminho do venv

# Reinstalar dependências
pip install --upgrade -r requirements.txt
```

### Erro de Permissão
```bash
# Executar com permissões adequadas
sudo uvicorn apirest.mainapi:app --reload --host 0.0.0.0 --port 8000
```

## 📝 Estrutura do Projeto

```
Server/
├── requirements.txt          # Dependências Python
├── README.md                # Esta documentação
├── apirest/
│   ├── mainapi.py           # API principal
│   └── main.py              # Exemplo simples
└── data/                    # Dados salvos (criado automaticamente)
```

## 🔒 Configurações de Segurança

Para produção, modifique:

1. **API Key**: Altere `"expected-api-key"` no código
2. **CORS**: Especifique origins permitidas
3. **HTTPS**: Configure certificados SSL

## 📞 Suporte

Para problemas ou dúvidas, verifique:
- Logs do servidor no terminal
- Documentação da API em `/docs`
- Arquivo de configuração do Gateway
