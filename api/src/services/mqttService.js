let client;
let alreadyInitialized = false;

let lampStatusCallback = () => {};
let sensorDataCallback = () => {};
let modeChangeCallback = () => {};
let limiarChangeCallback = () => {};

const {
  MQTT_TOPIC_CMD,
  MQTT_TOPIC_MOD,
  MQTT_TOPIC_LUX,
  MQTT_TOPIC_STATUS_MOD,
  MQTT_TOPIC_LIMIAR,
  MQTT_TOPIC_STATUS_LAMP,
} = process.env;

const init = (mqttInstance) => {
  if (alreadyInitialized) return;
  alreadyInitialized = true;

  client = mqttInstance;
  console.log("Cliente MQTT inicializado");

  client.subscribe(MQTT_TOPIC_STATUS_LAMP);
  client.subscribe(MQTT_TOPIC_LUX);
  client.subscribe(MQTT_TOPIC_STATUS_MOD);
  client.subscribe(MQTT_TOPIC_LIMIAR);

  client.on("message", (topic, message) => {
    const payload = message.toString().trim();

    if (topic === MQTT_TOPIC_STATUS_LAMP && lampStatusCallback) {
      if (["on", "off"].includes(payload)) {
        lampStatusCallback(payload);
      } else {
        console.warn(`Payload inválido para status_lampada: ${payload}`);
      }
    }

    if (topic === MQTT_TOPIC_LUX && sensorDataCallback) {
      const lux = parseFloat(payload);
      if (!isNaN(lux)) {
        sensorDataCallback(lux);
      } else {
        console.warn("Valor de lux inválido recebido");
      }
    }

    if (topic === MQTT_TOPIC_STATUS_MOD && modeChangeCallback) {
      modeChangeCallback(payload);
    }

    if (topic === MQTT_TOPIC_LIMIAR && limiarChangeCallback) {
      limiarChangeCallback(payload);
    }
  });
};

const isClientReady = () => {
  if (!client) {
    console.warn("MQTT client não inicializado");
    return false;
  }
  return true;
};

const onLampStatusChange = (callback) => {
  lampStatusCallback = callback;
};

const onSensorData = (callback) => {
  sensorDataCallback = callback;
};

const onModeChange = (callback) => {
  modeChangeCallback = callback;
};

const onLimiarChange = (callback) => {
  limiarChangeCallback = callback;
};

const publishCommand = (estado) => {
  if (!isClientReady()) return;
  client.publish(MQTT_TOPIC_CMD, estado);
  console.log(`📤 Comando publicado: ${estado}`);
};

const publishMode = (modo) => {
  if (!isClientReady()) return;
  client.publish(MQTT_TOPIC_MOD, modo);
  console.log(`📤 Modo publicado: ${modo}`);
};

const publishLimiar = (valor) => {
  if (!isClientReady()) return;
  client.publish(MQTT_TOPIC_LIMIAR, valor);
  console.log(`📤 Limiar publicado: ${valor}`);
};

module.exports = {
  init,
  publishCommand,
  publishMode,
  publishLimiar,
  onSensorData,
  onModeChange,
  onLimiarChange,
  onLampStatusChange,
};
