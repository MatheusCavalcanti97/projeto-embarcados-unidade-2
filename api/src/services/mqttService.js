let client; // Cliente MQTT será injetado via init()

const init = (mqttInstance) => {
  client = mqttInstance;
};

const publishCommand = (estado) => {
  if (client) {
    client.publish("ads/embarcados/unidade2/comando", estado);
  } else {
    console.warn("⚠️ MQTT client não inicializado: comando não enviado");
  }
};

const publishMode = (modo) => {
  if (client) {
    client.publish("ads/embarcados/unidade2/modo", modo);
  } else {
    console.warn("⚠️ MQTT client não inicializado: modo não enviado");
  }
};

const onSensorData = (callback) => {
  if (!client) return;

  client.subscribe("ads/embarcados/unidade2/lux");
  client.on("message", (topic, message) => {
    if (topic === "ads/embarcados/unidade2/lux") {
      const lux = parseFloat(message.toString());
      callback(lux);
    }
  });
};

const onModeChange = (callback) => {
  if (!client) return;

  client.subscribe("ads/embarcados/unidade2/status_modo");
  client.on("message", (topic, message) => {
    if (topic === "ads/embarcados/unidade2/status_modo") {
      callback(message.toString());
    }
  });
};

module.exports = {
  init,
  publishCommand,
  publishMode,
  onSensorData,
  onModeChange,
};
