// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 * 
 * ======================================
 * CHANGELOG
 * 
 * - v1: Initial version
 * - v2: Fix: Shelly Dimmer does not accept floats anymore (see here), which might create a max routing inaccuracy of 1% of the nominal load
 * - v3: Updated PID parameters (see here and here to have more info about how to tune the PID controller)
 * - v4: Fixed dimmer sharing feature to use Watts instead of %: using % based on PID output is wrong and will make the PID react to compensate. EXCESS_POWER_LIMIT can also be used to limit an output power to a specific value.
 * - v5: Introduced a 12-bits LUT to more closely match the voltage and current sine wave when computing the dimmer duty cycle to apply to the LSA
 * - v6: Added USE_POWER_LUT config to switch between linear dimming and LUT based dimming. v6 includes both code from v4 and v5.
 * 
 * ======================================
 */
const scriptName = "auto_diverter";

// Config

const CONFIG = {
  // Debug mode
  DEBUG: 0,
  // Grid Power Read Interval (s)
  READ_INTERVAL_S: 1,
  // grid semi-period in micro-seconds
  SEMI_PERIOD: 10000,
  // If set to true, the calculation of the dimmer duty cycle will be done based on a power matching LUT table which considers the voltage and current sine wave.
  // This should be more precise than a linear dimming.
  // You can try it and set it to false to compare the results, there is no harm in that!
  USE_POWER_LUT: true,
  // PID
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
    // Also try with 0.4, 0.5, 0.6
    // See: https://forum-photovoltaique.fr/viewtopic.php?p=796194#p796194
    KI: 0.4,
    // PID Derivative Gain
    KD: 0.1,
    // Output Minimum (W)
    OUT_MIN: -300,
    // Output Maximum (W)
    OUT_MAX: 5000,
  },
  // DIMMER LIST
  DIMMERS: {
    "192.168.125.98": {
      // Resistance (in Ohm) of the load connecter to the dimmer + voltage regulator
      // 0 will disable the dimmer
      RESISTANCE: 24.37,
      // Maximum excess power to reserve to this load.
      // The remaining power will be given to the next dimmers
      EXCESS_POWER_LIMIT: 0,
      // Set whether the Shelly EM with this script will be used to control the bypass relay to force a heating
      // When set to true, if you activate remotely the bypass to force a heating, then the script will detect it and turn the dimmer off
      BYPASS_CONTROLLED_BY_EM: true
    },
    // "192.168.125.99": {
    //   RESISTANCE: 0,
    //   EXCESS_POWER_LIMIT: 0,
    //   BYPASS_CONTROLLED_BY_EM: false
    // }
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
      powerToDivert: 0
    };
  }

  cb();
}

function constrain(value, min, max) { return Math.min(Math.max(value, min), max); }

const DIMMER_RESOLUTION = 12; // 12 bits
const DIMMER_MAX = (1 << DIMMER_RESOLUTION) - 1; // 4095
const TABLE_PHASE_LEN = 80;
const TABLE_PHASE_SCALE = (TABLE_PHASE_LEN - 1) * (1 << (16 - DIMMER_RESOLUTION)); // 1264
const TABLE_PHASE_DELAY = [0xefea, 0xdfd4, 0xd735, 0xd10d, 0xcc12, 0xc7cc, 0xc403, 0xc094, 0xbd6a, 0xba78, 0xb7b2, 0xb512, 0xb291, 0xb02b, 0xaddc, 0xaba2, 0xa97a, 0xa762, 0xa557, 0xa35a, 0xa167, 0x9f7f, 0x9da0, 0x9bc9, 0x99fa, 0x9831, 0x966e, 0x94b1, 0x92f9, 0x9145, 0x8f95, 0x8de8, 0x8c3e, 0x8a97, 0x88f2, 0x8750, 0x85ae, 0x840e, 0x826e, 0x80cf, 0x7f31, 0x7d92, 0x7bf2, 0x7a52, 0x78b0, 0x770e, 0x7569, 0x73c2, 0x7218, 0x706b, 0x6ebb, 0x6d07, 0x6b4f, 0x6992, 0x67cf, 0x6606, 0x6437, 0x6260, 0x6081, 0x5e99, 0x5ca6, 0x5aa9, 0x589e, 0x5686, 0x545e, 0x5224, 0x4fd5, 0x4d6f, 0x4aee, 0x484e, 0x4588, 0x4296, 0x3f6c, 0x3bfd, 0x3834, 0x33ee, 0x2ef3, 0x28cb, 0x202c, 0x1016];

function lookupFiringDelay(dutyCycle) {
  if (dutyCycle == 0)
    return CONFIG.SEMI_PERIOD;
  if (dutyCycle == 1)
    return 0;
  const duty = dutyCycle * DIMMER_MAX;
  const slot = duty * TABLE_PHASE_SCALE + (TABLE_PHASE_SCALE >> 2);
  const index = slot >> 16;
  const a = TABLE_PHASE_DELAY[index];
  const b = TABLE_PHASE_DELAY[index + 1];
  const delay = a - (((a - b) * (slot & 0xffff)) >> 16); // interpolate a b
  return (delay * CONFIG.SEMI_PERIOD) >> 16; // scale to period
}

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
  let dimmerPercentage = CONFIG.USE_POWER_LUT ? (CONFIG.SEMI_PERIOD - dimmer.firingDelay) / CONFIG.SEMI_PERIOD : dimmer.dutyCycle;
  dimmerPercentage = Math.round(constrain(dimmerPercentage * 100, 0, 100));
  const url = "http://" + ip + "/rpc/Light.Set?id=0&on=" + (dimmerPercentage > 0 ? "true" : "false") + "&brightness=" + dimmerPercentage + "&transition_duration=0.5";
  if (CONFIG.DEBUG > 1)
    print(scriptName, ":", "Calling Dimmer: ", url);
  Shelly.call("HTTP.GET", { url: url, timeout: 5 }, callDimmerCallback, { dimmer: dimmer, cb: cb });
}

function callDimmers(cb) {
  function recallMe() {
    callDimmers(cb)
  }

  for (let ip in DIVERT.dimmers) {
    dimmer = DIVERT.dimmers[ip];

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
      dimmer = DIVERT.dimmers[ip];
      if (CONFIG.DIMMERS[ip].BYPASS_CONTROLLED_BY_EM) {
        print(scriptName, ":", "Bypass is ON, turning off dimmer", ip);
        dimmer.apparentPower = 0;
        dimmer.current = 0;
        dimmer.dimmedVoltage = 0;
        dimmer.powerToDivert = 0;
        dimmer.dutyCycle = 0;
        dimmer.firingDelay = 0;
        dimmer.powerFactor = 0;
        dimmer.thdi = 0;
      }
    }
  }
  callDimmers(throttleReadPower);
}

function divert(gridVoltage, gridPower) {
  let availablePowerToDivert = calculatePID(gridPower);

  if (CONFIG.DEBUG > 0)
    print(scriptName, ":", "Grid:", gridVoltage, "V,", gridPower, "W => To divert:", availablePowerToDivert, "W");

  for (let ip in DIVERT.dimmers) {
    dimmer = DIVERT.dimmers[ip];

    // calculate powerToDivert
    dimmer.maxPower = gridVoltage * gridVoltage / CONFIG.DIMMERS[ip].RESISTANCE;
    dimmer.powerToDivert = constrain(availablePowerToDivert, 0, dimmer.maxPower);
    if (CONFIG.DIMMERS[ip].EXCESS_POWER_LIMIT > 0)
      dimmer.powerToDivert = constrain(dimmer.powerToDivert, 0, CONFIG.DIMMERS[ip].EXCESS_POWER_LIMIT);

    // calculate dutyCycle
    dimmer.dutyCycle = dimmer.maxPower == 0 ? 0 : constrain(dimmer.powerToDivert / dimmer.maxPower, 0, 1);
    dimmer.firingDelay = lookupFiringDelay(dimmer.dutyCycle);

    // derive abstract (not real measured) metrics
    dimmer.powerFactor = Math.sqrt(dimmer.dutyCycle);
    dimmer.dimmedVoltage = dimmer.powerFactor * gridVoltage;
    dimmer.current = dimmer.dimmedVoltage / CONFIG.DIMMERS[ip].RESISTANCE;
    dimmer.apparentPower = dimmer.current * gridVoltage;
    dimmer.thdi = dimmer.dutyCycle === 0 ? 0 : Math.sqrt(1 / dimmer.dutyCycle - 1);
    dimmer.rpc = "pending";

    availablePowerToDivert -= dimmer.powerToDivert;

    if (CONFIG.DEBUG > 0)
      print(scriptName, ":", "Dimmer:", ip, " => ", dimmer.powerToDivert, "W,", dimmer.dutyCycle, "%");
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
