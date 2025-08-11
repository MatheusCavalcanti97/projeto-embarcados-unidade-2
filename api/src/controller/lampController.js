const {
  publishCommand,
  publishMode,
  publishLimiar,
  onSensorData,
  onModeChange,
  onLimiarChange,
} = require("../services/mqttService");

let modoAtual = "manual";
let ultimoLux = 0;
let limiarLux = 300;

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
    console.log(`Modo atualizado pelo ESP32: ${modoAtual}`);
  });

  onLimiarChange((valor) => {
    const novoLimiar = parseInt(valor, 10);
    if (!isNaN(novoLimiar) && novoLimiar >= 0 && novoLimiar <= 1000) {
      limiarLux = novoLimiar;
      console.log(`🔧 Limiar atualizado via MQTT: ${limiarLux} lux`);
    } else {
      console.warn("Valor de limiar inválido recebido via MQTT");
    }
  });

  onSensorData((lux) => {
    ultimoLux = lux;

    if (modoAtual !== "automatico") return;

    const estado = lux > limiarLux ? "on" : "off";
    publishCommand(estado);
    console.log(`🌡️ Lux: ${lux} → Lâmpada: ${estado} (limiar: ${limiarLux})`);
  });
};

const getStatus = (req, res) => {
  res.json({ modo: modoAtual, lux: ultimoLux });
};

const getLimiar = (req, res) => {
  res.json({ limiarLux });
};

const setLimiar = (req, res) => {
  const { limiar } = req.body;
  const valor = parseFloat(limiar);

  if (isNaN(valor) || valor < 0 || valor > 1000) {
    return res
      .status(400)
      .send("Limiar inválido. Use um valor entre 0 e 1000.");
  }

  publishLimiar(valor.toString());
  res.json({ mensagem: "Limiar publicado com sucesso", valor });
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
  getLimiar,
  setLimiar,
};
