const {
  publishCommand,
  publishMode,
  onSensorData,
  onModeChange,
} = require("../services/mqttService");

let modoAtual = "manual";
let ultimoLux = 0;

const manualControl = (req, res) => {
  if (modoAtual !== "manual") {
    return res.status(403).send("Modo atual não permite controle manual");
  }

  const { estado } = req.body;

  if (estado !== "on" && estado !== "off") {
    return res.status(400).send("Estado inválido. Use 'on' ou 'off'.");
  }

  publishCommand(estado);
  res.send(`💡 Lâmpada ${estado === "on" ? "acesa" : "apagada"} manualmente`);
};

const automaticControl = () => {
  onModeChange((modo) => {
    modoAtual = modo.trim();
    console.log(`🔄 Modo atualizado pelo ESP32: ${modoAtual}`);
  });

  onSensorData((lux) => {
    ultimoLux = lux;

    if (modoAtual !== "automatico") return;

    const estado = lux < 300 ? "on" : "off";
    publishCommand(estado);
    console.log(`🌡️ Lux: ${lux} → Lâmpada: ${estado}`);
  });
};

const getStatus = (req, res) => {
  res.json({ modo: modoAtual, lux: ultimoLux });
};

const setMode = (req, res) => {
  const { modo } = req.body;

  if (modo !== "manual" && modo !== "automatico") {
    return res.status(400).send("Modo inválido. Use 'manual' ou 'automatico'.");
  }

  publishMode(modo);
  res.send(`Modo solicitado: ${modo}`);
};

module.exports = {
  manualControl,
  automaticControl,
  setMode,
  getStatus,
};
