const express = require("express");
const router = express.Router();
const {
  manualControl,
  setMode,
  getStatus,
} = require("../controller/lampController");

router.post("/manual", manualControl);
router.post("/modo", setMode);
router.get("/status", getStatus);

module.exports = router;
