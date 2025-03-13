// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 * 
 * ======================================
 * CHANGELOG
 * 
 * - v1: Initial version
 * 
 * ======================================
 */
const scriptName = "auto_diverter";

// Config

const CONFIG = {
  // Debug mode
  DEBUG: 1,
  // Grid Power Read Interval (s)
  READ_INTERVAL_S: 1,
  PID: {
    // Reverse
    REVERSE: false,
    // Proportional Mode:
    // - "error" (proportional on error),
    // - "input" (proportional on measurement),
    // - "both" (proportional on 50% error and 50% measurement)
    P_MODE: "input",
    // Derivative Mode:
    // - "error" (derivative on error),
    // - "input" (derivative on measurement)
    D_MODE: "error",
    // Integral Correction
    // - "off" (integral sum not clamped),
    // - "clamp" (integral sum not clamped to OUT_MIN and OUT_MAX),
    // - "advanced" (advanced anti-windup algorithm)
    IC_MODE: "advanced",
    // Target Grid Power (W)
    SETPOINT: 0,
    // PID Proportional Gain
    KP: 0.3,
    // PID Integral Gain
    KI: 0.3,
    // PID Derivative Gain
    KD: 0.1,
    // Output Minimum (W)
    OUT_MIN: -10000,
    // Output Maximum (W)
    OUT_MAX: 10000,
  },
  DIMMERS: {
    "192.168.125.98": {
      // Resistance (in Ohm) of the load connecter to the dimmer + voltage regulator
      // 0 will disable the dimmer
      RESISTANCE: 24,
      // Percentage of the remaining excess power that will be assigned to this dimmer
      // The remaining percentage will be given to the next dimmers
      RESERVED_EXCESS_PERCENT: 100,
      // Set whether the Shelly EM with this script will be used to control the bypass relay to force a heating
      // When set to true, if you activate remotely the bypass to force a heating, then the script will detect it and turn the dimmer off
      BYPASS_CONTROLLED_BY_EM: true
    },
    "192.168.125.97": {
      RESISTANCE: 0,
      RESERVED_EXCESS_PERCENT: 100,
      BYPASS_CONTROLLED_BY_EM: false
    }
  }
};

// PID Controller 

let PID = {
  // PID Input
  input: 0,
  // PID Output
  output: 0,
  // current error value
  error: 0,
  // Proportional Term
  pTerm: 0,
  // Integral Term
  iTerm: 0,
  // Derivative Term
  dTerm: 0,
  // Sum
  sum: 0,
};

// Divert Control

let DIVERT = {
  lastTime: 0,
  dimmers: {}
};

function validateConfig(cb) {
  print(scriptName, ":", "Validating Config...");

  if (CONFIG.DIMMERS.length === 0) {
    print(scriptName, ":", "ERR: No dimmer configured");
    return;
  }

  for (let ip in CONFIG.DIMMERS) {
    if (CONFIG.DIMMERS[ip].RESISTANCE < 0) {
      print(scriptName, ":", "ERR: Dimmer resistance should be greater than 0");
      return;
    }

    if (CONFIG.DIMMERS[ip].RESISTANCE === 0) {
      print(scriptName, ":", "Dimmer", ip, "is disabled");
      continue;
    }

    print(scriptName, ":", "Dimmer", ip, "is enabled");
    DIVERT.dimmers[ip] = {
      divertPower: 0
    };
  }

  cb();
}

function constrain(value, min, max) { return Math.min(Math.max(value, min), max); }

// - https://github.com/Dlloydev/QuickPID
// - https://github.com/br3ttb/Arduino-PID-Library
function calculatePID(input) {
  const dInput = CONFIG.PID.REVERSE ? PID.input - input : input - PID.input;
  const error = CONFIG.PID.REVERSE ? input - CONFIG.PID.SETPOINT : CONFIG.PID.SETPOINT - input;
  const dError = error - PID.error;

  if (CONFIG.DEBUG > 1) {
    print(scriptName, ":", "Input:", input, "W, Error:", error, "W, dError:", dError, "W");
  }

  let peTerm = CONFIG.PID.KP * error;
  let pmTerm = CONFIG.PID.KP * dInput;
  switch (CONFIG.PID.P_MODE) {
    case "error":
      pmTerm = 0;
      break;
    case "input":
      peTerm = 0;
      break;
    case "both":
      peTerm *= 0.5;
      pmTerm *= 0.5;
      break;
    default:
      return PID.output;
  }

  // pTerm
  PID.pTerm = peTerm - pmTerm;

  // iTerm
  PID.iTerm = CONFIG.PID.KI * error;

  if (CONFIG.DEBUG > 1) {
    print(scriptName, ":", "pTerm:", PID.pTerm, "W, iTerm:", PID.iTerm, "W");
  }

  // anti-windup
  if (CONFIG.PID.IC_MODE == "advanced" && CONFIG.PID.KI) {
    const iTermOut = PID.pTerm + CONFIG.PID.KI * (PID.iTerm + error);
    if ((iTermOut > CONFIG.PID.OUT_MAX && dError > 0) || (iTermOut < CONFIG.PID.OUT_MIN && dError < 0)) {
      _iTerm = constrain(iTermOut, -CONFIG.PID.OUT_MAX, CONFIG.PID.OUT_MAX);
    }
  }

  // integral sum
  PID.sum = CONFIG.PID.IC_MODE == "off" ? (PID.sum + PID.iTerm - pmTerm) : constrain(PID.sum + PID.iTerm - pmTerm, CONFIG.PID.OUT_MIN, CONFIG.PID.OUT_MAX);

  // dTerm
  switch (CONFIG.PID.D_MODE) {
    case "error":
      PID.dTerm = CONFIG.PID.KD * dError;
      break;
    case "input":
      PID.dTerm = -CONFIG.PID.KD * dInput;
      break;
    default:
      return PID.output;
  }

  PID.output = constrain(PID.sum + peTerm + PID.dTerm, CONFIG.PID.OUT_MIN, CONFIG.PID.OUT_MAX);

  PID.input = input;
  PID.error = error;

  return PID.output;
}

function callDimmerCallback(result, errCode, errMessage, data) {
  if (errCode) {
    print(scriptName, ":", "ERR callDimmerCallback:", errCode);
    data.dimmer.rpc = "failed";
  } else if (result.code !== 200) {
    const rpcResult = JSON.parse(result.body);
    print(scriptName, ":", "ERR", rpcResult.code, ":", rpcResult.message);
    data.dimmer.rpc = "failed";
  } else {
    data.dimmer.rpc = "success";
  }
  data.cb();
}

function callDimmer(ip, dimmer, cb) {
  const url = "http://" + ip + "/rpc/Light.Set?id=0&on=" + (dimmer.dutyCycle > 0 ? "true" : "false") + "&brightness=" + (dimmer.dutyCycle * 100) + "&transition_duration=0.5";
  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "Calling Dimmer: ", url);
  Shelly.call("HTTP.GET", { url: url, timeout: 5 }, callDimmerCallback, { dimmer: dimmer, cb: cb });
}

function callDimmers(cb) {
  function recallMe() {
    callDimmers(cb)
  }

  for (let ip in DIVERT.dimmers) {
    const dimmer = DIVERT.dimmers[ip];

    // ignore contacted dimmers
    if (dimmer.rpc !== "pending") {
      continue;
    }

    if (isNaN(dimmer.dutyCycle)) {
      dimmer.rpc = "failed";
      print(scriptName, ":", "ERR: Invalid duty cycle for dimmer", ip);
      continue;
    }

    // call dimmer
    callDimmer(ip, dimmer, recallMe);

    // exit the loop immediately to avoid multiple calls in case the yare made in parallel
    return;
  }

  // if we are here, all dimmers have been contacted
  cb();
}

function onSwitchGetStatus(result, errCode, errMessage, data) {
  if (errCode) {
    print(scriptName, ":", "ERR onSwitchGetStatus:", errCode);
    throttleReadPower();
    return;
  }
  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "onSwitchGetStatus:", JSON.stringify(result));
  if (result.output) {
    for (let ip in DIVERT.dimmers) {
      const dimmer = DIVERT.dimmers[ip];
      if (CONFIG.DIMMERS[ip].BYPASS_CONTROLLED_BY_EM) {
        print(scriptName, ":", "Bypass is ON, turning off dimmer", ip);
        dimmer.apparentPower = 0;
        dimmer.current = 0;
        dimmer.dimmedVoltage = 0;
        dimmer.divertPower = 0;
        dimmer.dutyCycle = 0;
        dimmer.powerFactor = 0;
        dimmer.thdi = 0;
      }
    }
  }
  callDimmers(throttleReadPower);
}

function divert(voltage, gridPower) {
  let newRoutingPower = calculatePID(gridPower);

  if (CONFIG.DEBUG > 0)
    print(scriptName, ":", "Grid:", voltage, "V,", gridPower, "W => To divert:", newRoutingPower, "W");

  for (let ip in DIVERT.dimmers) {
    const dimmer = DIVERT.dimmers[ip];
    dimmer.maximumPower = voltage * voltage / CONFIG.DIMMERS[ip].RESISTANCE;
    dimmer.divertPower = Math.min(Math.max(0, newRoutingPower) * CONFIG.DIMMERS[ip].RESERVED_EXCESS_PERCENT / 100, dimmer.maximumPower);
    dimmer.dutyCycle = dimmer.divertPower / dimmer.maximumPower;
    dimmer.powerFactor = Math.sqrt(dimmer.dutyCycle);
    dimmer.dimmedVoltage = dimmer.powerFactor * voltage;
    dimmer.current = dimmer.dimmedVoltage / CONFIG.DIMMERS[ip].RESISTANCE;
    dimmer.apparentPower = dimmer.current * voltage;
    dimmer.thdi = dimmer.dutyCycle === 0 ? 0 : Math.sqrt(1 / dimmer.dutyCycle - 1);
    dimmer.rpc = "pending";

    newRoutingPower -= dimmer.divertPower;

    if (CONFIG.DEBUG > 0)
      print(scriptName, ":", "Dimmer", ip, "=>", dimmer.divertPower, "W");
  }

  Shelly.call("Switch.GetStatus", { id: 0 }, onSwitchGetStatus);
}

function onEM1GetStatus(result, errCode, errMessage, data) {
  if (errCode) {
    print(scriptName, ":", "ERR onEM1GetStatus:", errCode);
    throttleReadPower();
    return;
  }
  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "EM1.GetStatus:", JSON.stringify(result));
  divert(result.voltage, result.act_power);
}

function readPower() {
  DIVERT.lastTime = Date.now();
  Shelly.call("EM1.GetStatus", { id: 0 }, onEM1GetStatus);
}

function throttleReadPower() {
  const now = Date.now();
  const diff = now - DIVERT.lastTime;
  if (diff > 1000) {
    readPower();
  } else {
    Timer.set(1000 - diff, false, readPower);
  }
}

// HTTP handlers

function onHttpGetStatus(request, response) {
  response.code = 200;
  response.headers = {
    "Content-Type": "application/json"
  };
  response.body = JSON.stringify({
    config: CONFIG,
    pid: PID,
    divert: DIVERT
  });
  response.send();
}

// Main

validateConfig(function () {
  print(scriptName, ":", "Starting Shelly Solar Diverter...");
  HTTPServer.registerEndpoint("status", onHttpGetStatus);
  readPower();
});
