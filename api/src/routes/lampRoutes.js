const express = require("express");
const router = express.Router();

const {
  manualControl,
  setMode,
  getStatus,
  getLimiar,
  setLimiar,
} = require("../controller/lampController");

router.post("/manual", manualControl);
router.post("/modo", setMode);
router.get("/status", getStatus);
router.get("/limiar", getLimiar);
router.post("/limiar", setLimiar);

module.exports = router;
