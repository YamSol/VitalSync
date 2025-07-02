#!/bin/bash

# 🩺 VitalSync Server Setup Script
# Configura automaticamente o ambiente Python para o servidor

echo "🩺 VitalSync Server - Script de Configuração"
echo "=============================================="

# Verificar se Python está instalado
echo "🔍 Verificando Python..."
if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version 2>&1 | cut -d' ' -f2)
    echo "✅ Python encontrado: $PYTHON_VERSION"
    
    # Verificar versão mínima (3.8)
    REQUIRED_VERSION="3.8"
    if python3 -c "import sys; exit(0 if sys.version_info >= (3, 8) else 1)"; then
        echo "✅ Versão do Python compatível (≥3.8)"
    else
        echo "❌ Python 3.8+ requerido. Versão atual: $PYTHON_VERSION"
        exit 1
    fi
else
    echo "❌ Python3 não encontrado. Instale Python 3.8+ primeiro."
    exit 1
fi

# Criar ambiente virtual
echo ""
echo "📦 Configurando ambiente virtual..."
if [ -d "venv" ]; then
    echo "⚠️ Ambiente virtual já existe. Removendo..."
    rm -rf venv
fi

python3 -m venv venv
if [ $? -eq 0 ]; then
    echo "✅ Ambiente virtual criado com sucesso"
else
    echo "❌ Erro ao criar ambiente virtual"
    exit 1
fi

# Ativar ambiente virtual
echo ""
echo "⚡ Ativando ambiente virtual..."
source venv/bin/activate

# Verificar se activation funcionou
if [ "$VIRTUAL_ENV" != "" ]; then
    echo "✅ Ambiente virtual ativado: $VIRTUAL_ENV"
else
    echo "❌ Falha ao ativar ambiente virtual"
    exit 1
fi

# Atualizar pip
echo ""
echo "🔧 Atualizando pip..."
pip install --upgrade pip

# Instalar dependências
echo ""
echo "📚 Instalando dependências..."
if [ -f "requirements.txt" ]; then
    pip install -r requirements.txt
    if [ $? -eq 0 ]; then
        echo "✅ Dependências instaladas com sucesso"
    else
        echo "❌ Erro ao instalar dependências"
        exit 1
    fi
else
    echo "❌ Arquivo requirements.txt não encontrado"
    exit 1
fi

# Criar diretório de dados
echo ""
echo "📁 Criando diretório de dados..."
mkdir -p data
echo "✅ Diretório 'data' criado"

# Verificar se a API pode ser importada
echo ""
echo "🧪 Testando importação da API..."
if python -c "from apirest.mainapi import app; print('✅ API importada com sucesso')" 2>/dev/null; then
    echo "✅ Teste de importação passou"
else
    echo "❌ Erro na importação da API. Verifique o código."
fi

echo ""
echo "🎉 Configuração concluída com sucesso!"
echo ""
echo "📋 Próximos passos:"
echo "1. Ativar o ambiente virtual:"
echo "   source venv/bin/activate"
echo ""
echo "2. Executar o servidor:"
echo "   uvicorn apirest.mainapi:app --reload --host 0.0.0.0 --port 8000"
echo ""
echo "3. Acessar a documentação:"
echo "   http://localhost:8000/docs"
echo ""
echo "🔧 Para desativar o ambiente virtual:"
echo "   deactivate"
