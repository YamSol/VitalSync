# Guia de Conexões E32-433T20D com ESP32

## Problemas Comuns e Verificações

### 1. Conexões Físicas Corretas

#### Pinagem do E32-433T20D:
```
E32-433T20D     ESP32 DEVKIT V1
VCC      →      3.3V (NÃO usar 5V!)
GND      →      GND
TX       →      GPIO 17 (RX do ESP32)
RX       →      GPIO 16 (TX do ESP32)
M0       →      GPIO 4
M1       →      GPIO 5
AUX      →      GPIO 25
```

### 2. Verificações Importantes

#### A. Alimentação:
- ✅ **OBRIGATÓRIO**: Use 3.3V, NÃO 5V
- ✅ Verifique se o GND está conectado
- ✅ Corrente: O E32 pode consumir até 120mA na transmissão

#### B. Conexões Cruzadas TX/RX:
- ✅ TX do E32 → RX do ESP32 (GPIO 17)
- ✅ RX do E32 → TX do ESP32 (GPIO 16)

#### C. Antena:
- ✅ Verifique se a antena está conectada
- ✅ Antena deve ter ~17cm para 433MHz

### 3. Modos de Operação do E32

| M0 | M1 | Modo |
|----|----|----- |
| 0  | 0  | Normal (Transmissão/Recepção) |
| 0  | 1  | WOR (Wake on Radio) |
| 1  | 0  | Configuração |
| 1  | 1  | Sleep |

### 4. Possíveis Problemas

1. **Configuração de fábrica diferente entre módulos**
2. **Endereçamento incorreto**
3. **Canal/frequência diferentes**
4. **Potência de transmissão muito baixa**
5. **Interferência eletromagnética**

### 5. Como Usar o Código de Teste

1. Carregue o código de teste em AMBOS os ESP32
2. Abra o Serial Monitor de ambos (115200 baud)
3. Digite '1' no Transmitter para enviar teste
4. Digite '2' no Gateway para entrar em modo de escuta contínua
5. Digite '3' em qualquer um para configurar o módulo
