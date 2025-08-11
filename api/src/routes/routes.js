const express = require("express");
const router = express.Router();

const lampRoutes = require("./lampRoutes");

router.use("/lampada", lampRoutes);

module.exports = router;
