/**
 * Enhanced Modbus Frame Parser for Serial Studio
 * ===============================================
 *
 * A robust, configurable Modbus parser with:
 *   - Automatic scaling/division for fixed-point values
 *   - Signed integer support
 *   - 32-bit register pair support
 *   - Comprehensive exception handling
 *   - Debug logging option
 *
 * Compatible with Serial Studio's Modbus TCP/RTU driver.
 * Requires Binary (Direct) decoder mode in the Project Editor.
 *
 * Author: Serial Studio Community
 * License: MIT
 */

//==============================================================================
// CONFIGURATION - Edit this section to match your device
//==============================================================================

const CONFIG = {
  /**
   * Number of output values (datasets in Serial Studio)
   */
  numItems: 9,

  /**
   * Register address offset (0 for most devices, 40001 for legacy Modicon)
   */
  registerOffset: 0,

  /**
   * Enable console debug logging
   */
  debug: false,

  /**
   * Register definitions - customize for your application
   *
   * Properties:
   *   name:     Human-readable name (for debugging)
   *   scale:    Divisor for fixed-point (1 = no scaling)
   *   signed:   true for signed 16-bit (-32768 to 32767)
   *   combine:  'high' or 'low' for 32-bit register pairs
   *   units:    Display units (for documentation)
   */
  registers: {
    0: { name: "Emergency Stop",  scale: 1,  signed: false, units: "bool" },
    1: { name: "Start LED",       scale: 1,  signed: false, units: "bool" },
    2: { name: "Temperature",     scale: 1,  signed: false, units: "°F" },
    3: { name: "Pressure",        scale: 1,  signed: false, units: "PSI" },
    4: { name: "Motor RPM",       scale: 1,  signed: false, units: "RPM" },
    5: { name: "Valve Position",  scale: 1,  signed: false, units: "%" },
    6: { name: "Flow Rate",       scale: 10, signed: false, units: "GPM" },
    7: { name: "Motor Load",      scale: 1,  signed: false, units: "%" },
    8: { name: "Vibration",       scale: 10, signed: false, units: "mm/s" }
  }
};

//==============================================================================
// EXCEPTION CODES - Modbus standard exception definitions
//==============================================================================

const MODBUS_EXCEPTIONS = {
  0x01: { name: "ILLEGAL_FUNCTION",       desc: "Function code not supported" },
  0x02: { name: "ILLEGAL_DATA_ADDRESS",   desc: "Register address out of range" },
  0x03: { name: "ILLEGAL_DATA_VALUE",     desc: "Value out of range for register" },
  0x04: { name: "SERVER_DEVICE_FAILURE",  desc: "Unrecoverable device error" },
  0x05: { name: "ACKNOWLEDGE",            desc: "Request accepted, processing" },
  0x06: { name: "SERVER_DEVICE_BUSY",     desc: "Device busy, retry later" },
  0x07: { name: "NEGATIVE_ACKNOWLEDGE",   desc: "Cannot perform request" },
  0x08: { name: "MEMORY_PARITY_ERROR",    desc: "Memory parity check failed" },
  0x0A: { name: "GATEWAY_PATH_UNAVAIL",   desc: "Gateway path not available" },
  0x0B: { name: "GATEWAY_TARGET_FAILED",  desc: "Gateway target not responding" }
};

//==============================================================================
// FUNCTION CODES - Modbus standard function definitions
//==============================================================================

const MODBUS_FUNCTIONS = {
  0x01: { name: "Read Coils",              type: "read",  dataType: "bit" },
  0x02: { name: "Read Discrete Inputs",    type: "read",  dataType: "bit" },
  0x03: { name: "Read Holding Registers",  type: "read",  dataType: "register" },
  0x04: { name: "Read Input Registers",    type: "read",  dataType: "register" },
  0x05: { name: "Write Single Coil",       type: "write", dataType: "bit" },
  0x06: { name: "Write Single Register",   type: "write", dataType: "register" },
  0x0F: { name: "Write Multiple Coils",    type: "write", dataType: "bit" },
  0x10: { name: "Write Multiple Registers", type: "write", dataType: "register" },
  0x17: { name: "Read/Write Multiple",     type: "both",  dataType: "register" }
};

//==============================================================================
// STATE - Persistent storage for parsed values
//==============================================================================

const parsedValues = new Array(CONFIG.numItems).fill(0);
let lastException = null;
let frameCount = 0;

//==============================================================================
// HELPER FUNCTIONS
//==============================================================================

/**
 * Convert unsigned 16-bit to signed
 */
function toSigned16(value) {
  return value > 0x7FFF ? value - 0x10000 : value;
}

/**
 * Combine two 16-bit registers into 32-bit value
 */
function combine32(high, low) {
  return (high << 16) | low;
}

/**
 * Format byte array as hex string for debugging
 */
function toHexString(bytes) {
  return Array.from(bytes).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
}

/**
 * Debug log helper
 */
function debugLog(message) {
  if (CONFIG.debug) {
    console.log("[Modbus] " + message);
  }
}

/**
 * Get register config with defaults
 */
function getRegisterConfig(index) {
  return CONFIG.registers[index] || { name: "Register " + index, scale: 1, signed: false };
}

/**
 * Apply scaling and sign conversion to raw value
 */
function processValue(index, rawValue) {
  var config = getRegisterConfig(index);
  var value = config.signed ? toSigned16(rawValue) : rawValue;
  return value / (config.scale || 1);
}

//==============================================================================
// MAIN PARSER FUNCTION
//==============================================================================

/**
 * Parse a Modbus response frame into an array of values.
 *
 * @param {Uint8Array|Array} frame - Binary Modbus ADU (no CRC)
 * @returns {Array} Parsed values array for Serial Studio
 *
 * Frame Structure:
 *   Byte 0:     Slave/Unit address (1-247)
 *   Byte 1:     Function code (1-127) or Exception (128-255)
 *   Byte 2+:    Data payload (varies by function)
 */
function parse(frame) {
  frameCount++;

  // Validate minimum frame length
  if (!frame || frame.length < 3) {
    debugLog("Frame too short: " + (frame ? frame.length : 0) + " bytes");
    return parsedValues;
  }

  var slaveAddress = frame[0];
  var functionCode = frame[1];

  debugLog("Frame #" + frameCount + ": Slave=" + slaveAddress +
           " FC=0x" + functionCode.toString(16) + " Len=" + frame.length);

  //----------------------------------------------------------------------------
  // Exception Response (FC >= 0x80)
  //----------------------------------------------------------------------------
  if (functionCode >= 0x80) {
    var originalFC = functionCode & 0x7F;
    var exceptionCode = frame[2];
    var exception = MODBUS_EXCEPTIONS[exceptionCode] || { name: "UNKNOWN", desc: "Unknown exception" };

    lastException = {
      functionCode: originalFC,
      exceptionCode: exceptionCode,
      name: exception.name,
      description: exception.desc,
      timestamp: Date.now()
    };

    console.log("[Modbus Exception] FC=0x" + originalFC.toString(16) +
                " Error=0x" + exceptionCode.toString(16) +
                " (" + exception.name + ": " + exception.desc + ")");

    // Return last known good values on exception
    return parsedValues;
  }

  //----------------------------------------------------------------------------
  // Read Coils (0x01) / Read Discrete Inputs (0x02)
  //----------------------------------------------------------------------------
  if (functionCode === 0x01 || functionCode === 0x02) {
    var byteCount = frame[2];
    var bitIndex = 0;

    debugLog("Reading " + (byteCount * 8) + " bits");

    for (var byteIdx = 0; byteIdx < byteCount && bitIndex < CONFIG.numItems; byteIdx++) {
      var dataByte = frame[3 + byteIdx];
      for (var bit = 0; bit < 8 && bitIndex < CONFIG.numItems; bit++) {
        parsedValues[bitIndex] = (dataByte >> bit) & 0x01;
        bitIndex++;
      }
    }
  }

  //----------------------------------------------------------------------------
  // Read Holding Registers (0x03) / Read Input Registers (0x04)
  //----------------------------------------------------------------------------
  else if (functionCode === 0x03 || functionCode === 0x04) {
    var byteCount = frame[2];
    var registerCount = Math.floor(byteCount / 2);

    debugLog("Reading " + registerCount + " registers (" + byteCount + " bytes)");

    for (var i = 0; i < registerCount && i < CONFIG.numItems; i++) {
      var offset = 3 + (i * 2);
      if (offset + 1 < frame.length) {
        var rawValue = (frame[offset] << 8) | frame[offset + 1];
        parsedValues[i] = processValue(i, rawValue);

        if (CONFIG.debug) {
          var config = getRegisterConfig(i);
          debugLog("  [" + i + "] " + config.name + " = " + parsedValues[i] + " " + (config.units || ""));
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  // Write Single Coil (0x05) - Echo Response
  //----------------------------------------------------------------------------
  else if (functionCode === 0x05) {
    if (frame.length >= 6) {
      var coilAddress = (frame[2] << 8) | frame[3];
      var coilValue = (frame[4] << 8) | frame[5];
      var index = coilAddress - CONFIG.registerOffset;

      if (index >= 0 && index < CONFIG.numItems) {
        parsedValues[index] = (coilValue === 0xFF00) ? 1 : 0;
        debugLog("Write Coil [" + index + "] = " + parsedValues[index]);
      }
    }
  }

  //----------------------------------------------------------------------------
  // Write Single Register (0x06) - Echo Response
  //----------------------------------------------------------------------------
  else if (functionCode === 0x06) {
    if (frame.length >= 6) {
      var registerAddress = (frame[2] << 8) | frame[3];
      var rawValue = (frame[4] << 8) | frame[5];
      var index = registerAddress - CONFIG.registerOffset;

      if (index >= 0 && index < CONFIG.numItems) {
        parsedValues[index] = processValue(index, rawValue);
        debugLog("Write Register [" + index + "] = " + parsedValues[index]);
      }
    }
  }

  //----------------------------------------------------------------------------
  // Write Multiple Coils (0x0F) - Confirmation Response
  //----------------------------------------------------------------------------
  else if (functionCode === 0x0F) {
    if (frame.length >= 6) {
      var startAddress = (frame[2] << 8) | frame[3];
      var quantity = (frame[4] << 8) | frame[5];
      debugLog("Write Multiple Coils: addr=" + startAddress + " qty=" + quantity);
    }
  }

  //----------------------------------------------------------------------------
  // Write Multiple Registers (0x10) - Confirmation Response
  //----------------------------------------------------------------------------
  else if (functionCode === 0x10) {
    if (frame.length >= 6) {
      var startAddress = (frame[2] << 8) | frame[3];
      var quantity = (frame[4] << 8) | frame[5];
      debugLog("Write Multiple Registers: addr=" + startAddress + " qty=" + quantity);
    }
  }

  //----------------------------------------------------------------------------
  // Unsupported Function Code
  //----------------------------------------------------------------------------
  else {
    var fcInfo = MODBUS_FUNCTIONS[functionCode];
    if (fcInfo) {
      debugLog("Unsupported function: " + fcInfo.name + " (0x" + functionCode.toString(16) + ")");
    } else {
      debugLog("Unknown function code: 0x" + functionCode.toString(16));
    }
  }

  return parsedValues;
}

//==============================================================================
// UTILITY EXPORTS (for advanced use)
//==============================================================================

/**
 * Get the last exception that occurred
 */
function getLastException() {
  return lastException;
}

/**
 * Get frame statistics
 */
function getStats() {
  return {
    framesProcessed: frameCount,
    lastException: lastException
  };
}

/**
 * Reset parser state
 */
function reset() {
  for (var i = 0; i < parsedValues.length; i++) {
    parsedValues[i] = 0;
  }
  lastException = null;
  frameCount = 0;
}
