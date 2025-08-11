require("dotenv").config();

const express = require("express");
const mqtt = require("mqtt");
const routes = require("./routes/routes");
const { automaticControl } = require("./controller/lampController");
const mqttService = require("./services/mqttService");

const app = express();
app.use(express.json());
app.use(express.static("public"));
app.use(routes);

const mqttClient = mqtt.connect("mqtt://broker.hivemq.com:1883");

mqttClient.on("connect", () => {
  console.log("✅ Conectado ao HiveMQ público");

  mqttService.init(mqttClient);

  automaticControl();
});

app.set("mqttClient", mqttClient);

app.get("/", (req, res) => {
  res.send("API de controle de lâmpada está rodando!");
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`🚀 Servidor rodando na porta ${PORT}`);
});

module.exports = app;
