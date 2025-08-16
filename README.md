# Projeto Embarcados - Unidade 2

Este projeto faz parte da Unidade 2 da disciplina de Sistemas Embarcados e utiliza o **ESP-IDF** para comunicação via MQTT e conexão Wi-Fi.

---

## 📥 1. Realizar o download do arquivo no repositório do GitHub

Você pode baixar o projeto de duas formas:

### **Opção 1 – Via Navegador**

1. Acesse o repositório no GitHub:
   [projeto-embarcados-unidade-2](https://github.com/MatheusCavalcanti97/projeto-embarcados-unidade-2.git)
2. Clique no botão **Code** (verde).
3. Clique em **Download ZIP**.
4. Aguarde o download concluir.

### **Opção 2 – Via Terminal (Git instalado)**

```bash
git clone https://github.com/MatheusCavalcanti97/projeto-embarcados-unidade-2.git
```

---

## 📂 2. Descompactar e acessar a pasta do arquivo baixado

**Se baixou como ZIP:**

1. Localize o arquivo `.zip` (normalmente na pasta `Downloads`).
2. Clique com o botão direito sobre ele e selecione **Extrair aqui** ou **Extrair para...**.

**Se clonou via Git:**

- O projeto já estará disponível na pasta `projeto-embarcados-unidade-2`.

---

## 🖥️ 3. Abrir a pasta no VS Code

1. Abra o **VS Code**.
2. Vá em **File > Open Folder** e selecione a pasta do projeto.

---

## ⚙️ 4. Abrir o terminal do ESP-IDF no VS Code

1. Na parte inferior do VS Code, procure por **OPEN ESP-IDF TERMINAL**.
2. Clique para abrir o terminal específico do ESP-IDF.

---

## 🔧 5. Executar o menu de configuração

1. No terminal do ESP-IDF, digite:
   ```bash
   idf.py menuconfig
   ```

---

## 🌐 6. Configurar o MQTT Broker

1. Dentro do menu de configuração, procure por **Example Configuration**.
2. Verifique se existe o valor:
   ```
   mqtt://broker.hivemq.com:1883
   ```
3. Caso não exista, insira este valor.
4. Pressione **S** para salvar e **ESC** para voltar.

---

## 📡 7. Configurar Wi-Fi

1. Acesse **Example Connection Configuration**.
2. Caso não haja valores, insira:
   - **Wifi-SSID**: nome da sua rede Wi-Fi
   - **Wifi-Password**: senha da sua rede Wi-Fi
3. Pressione **S** para salvar e **ESC** para sair.

---

## 🚀 8. Compilar e executar o projeto

⚠️ **Importante:** Antes de prosseguir, verifique:

- O ESP-32 deve estar conectado ao computador via cabo USB.
- Confirme se está selecionada a porta correta no VS Code. Caso não esteja, clique no seletor de porta (ex: `COMx`) e escolha a correta.

1. No terminal do ESP-IDF, execute:
   ```bash
   idf.py build
   idf.py flash
   idf.py monitor
   ```
2. O dispositivo será programado e o monitor serial exibirá os logs.

---

## 🔌 9. Conectar a fonte de alimentação

Após a conclusão do processo:

1. Desconecte o cabo USB do ESP-32.
2. Conecte a fonte da lâmpada na tomada de energia.
3. Aguarde a luz **AZUL** do ESP acender — isso indicará que o dispositivo está conectado à internet.

## ⚠️ Observação Importante

Se o **ESP-32** estiver conectado via USB ao computador, **a fonte e a tomada da lâmpada não devem estar conectadas à energia** nesta fase para evitar riscos.

# 📡 API do Projeto

- A API está localizada dentro do projeto ESP na pasta:

```
   .\projeto-final-embarcados-un-2\api
```

---

# 🔧 Configuração

1.  Criar o arquivo .env:

- Na pasta api, crie um arquivo chamado .env e insira os seguintes valores:

# 🔌 Conexão com o broker MQTT

1. BROKER_URL=mqtt://broker.hivemq.com:1883

# 📤 Tópicos de publicação

1. MQTT_TOPIC_CMD=ads/embarcados/unidade2/comando
   - Comando para acender/apagar lâmpada.
2. MQTT_TOPIC_MOD=ads/embarcados/unidade2/modo
   - Modo solicitado (manual/automatico).

# 📥 Tópicos de assinatura

1. MQTT_TOPIC_LUX=ads/embarcados/unidade2/lux

   - Sensor de luminosidade.

2. MQTT_TOPIC_STATUS_MOD=ads/embarcados/unidade2/status_modo

   - Modo atual reportado pelo ESP32.

3. MQTT_TOPIC_LIMIAR=ads/embarcados/unidade2/limiar
   - Limiar de luminosidade.

1. MQTT_TOPIC_STATUS_LAMP=ads/embarcados/unidade2/status_lampada
   - Status da lâmpada (ligada/desligada).

---

---

# 🌐 Porta do servidor Express

1. PORT=3000

2. Instalar dependências:

- No terminal, dentro da pasta api:

```
npm install
```

3. Executar a API:

```
npm start
```

# 🧠 Estrutura da API

- A API é organizada em controller, routes e service:

## Controller (lampController.js):

### 1. manualControl(req, res)

- Permite acender/apagar a lâmpada manualmente, apenas se o modo atual for "manual".

- Valida o valor do campo estado no corpo da requisição (on ou off).

- Publica o comando via MQTT (publishCommand).

### 2. automaticControl()

- Escuta mudanças de modo (onModeChange), limiar (onLimiarChange) e sensor de luminosidade (onSensorData).

- Quando o modo é "automatico", decide acender/apagar a lâmpada conforme o valor de lux comparado ao limiar.

### 3. getStatus(req, res)

- Retorna o modo atual e o último valor de luminosidade registrado.

### 4. getLimiar(req, res)

- Retorna o valor atual do limiar de luminosidade.

### 5. setLimiar(req, res)

- Atualiza o limiar de luminosidade e publica via MQTT (publishLimiar).

### 6. setMode(req, res)

- Altera o modo de operação (manual ou automatico) e publica via MQTT (publishMode).
