let client;

const {
  MQTT_TOPIC_CMD,
  MQTT_TOPIC_MOD,
  MQTT_TOPIC_LUX,
  MQTT_TOPIC_STATUS_MOD,
  MQTT_TOPIC_LIMIAR,
} = process.env;

const init = (mqttInstance) => {
  client = mqttInstance;
  console.log("🚀 Cliente MQTT inicializado");
};

const isClientReady = () => {
  if (!client) {
    console.warn("MQTT client não inicializado");
    return false;
  }
  return true;
};

const publishCommand = (estado) => {
  if (!isClientReady()) return;
  client.publish(MQTT_TOPIC_CMD, estado);
  console.log(`Comando publicado: ${estado}`);
};

const publishMode = (modo) => {
  if (!isClientReady()) return;
  client.publish(MQTT_TOPIC_MOD, modo);
  console.log(`📤 Modo publicado: ${modo}`);
};

const publishLimiar = (valor) => {
  if (!isClientReady()) return;
  client.publish(MQTT_TOPIC_LIMIAR, valor);
  console.log(`Limiar publicado: ${valor}`);
};

const onSensorData = (callback) => {
  if (!isClientReady()) return;

  client.subscribe(MQTT_TOPIC_LUX);
  client.on("message", (topic, message) => {
    if (topic === MQTT_TOPIC_LUX) {
      const lux = parseFloat(message.toString());
      if (!isNaN(lux)) {
        callback(lux);
      } else {
        console.warn("Valor de lux inválido recebido");
      }
    }
  });
};

const onModeChange = (callback) => {
  if (!isClientReady()) return;

  client.subscribe(MQTT_TOPIC_STATUS_MOD);
  client.on("message", (topic, message) => {
    if (topic === MQTT_TOPIC_STATUS_MOD) {
      callback(message.toString());
    }
  });
};

const onLimiarChange = (callback) => {
  if (!isClientReady()) return;

  client.subscribe(MQTT_TOPIC_LIMIAR);
  client.on("message", (topic, message) => {
    if (topic === MQTT_TOPIC_LIMIAR) {
      callback(message.toString());
    }
  });
};

module.exports = {
  init,
  publishCommand,
  publishMode,
  publishLimiar,
  onSensorData,
  onModeChange,
  onLimiarChange,
};
