/**
 * Modbus ASCII Frame Parser
 *
 * Parses Modbus ASCII protocol frames into an array of values.
 *
 * Modbus ASCII is a serial communication protocol commonly used in industrial
 * automation. Data is transmitted as ASCII characters representing hexadecimal values.
 *
 * INPUT FORMAT: ":010300000002FA\r\n" (ASCII hex characters)
 * OUTPUT ARRAY: Extracted register values from the Modbus response
 *
 * Note: Frame delimiters (start ':' and end '\r\n') are automatically removed
 *       by Serial Studio. This parser handles Modbus ASCII response frames.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 16;

/**
 * Starting register address offset.
 * If your Modbus device uses register 40001 as address 0, set this to 40001.
 * Most devices use 0-based addressing, so the default is 0.
 */
const registerOffset = 0;

/**
 * Array to hold all parsed register values.
 * Each index corresponds to a Modbus register value.
 */
const parsedValues = new Array(numItems).fill(0);

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------

/**
 * Converts two ASCII hex characters to a byte value.
 * Example: "4A" -> 74
 */
function hexToByte(hex) {
  return parseInt(hex, 16);
}

/**
 * Calculates Modbus LRC (Longitudinal Redundancy Check).
 * Used to verify frame integrity.
 */
function calculateLRC(data) {
  var lrc = 0;
  for (var i = 0; i < data.length; i++) {
    lrc = (lrc + data[i]) & 0xFF;
  }
  return ((lrc ^ 0xFF) + 1) & 0xFF;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a Modbus ASCII response frame into an array of register values.
 *
 * Modbus ASCII frame structure (without ':' and '\r\n'):
 *   - 2 chars: Slave address
 *   - 2 chars: Function code
 *   - N chars: Data (depends on function code)
 *   - 2 chars: LRC checksum
 *
 * Common function codes:
 *   0x03 (03): Read Holding Registers
 *   0x04 (04): Read Input Registers
 *   0x06 (06): Write Single Register
 *   0x10 (16): Write Multiple Registers
 *
 * How it works:
 * 1. Removes ':' prefix if present
 * 2. Converts ASCII hex pairs to bytes
 * 3. Validates LRC checksum
 * 4. Extracts register values based on function code
 * 5. Stores values in the output array
 *
 * Example frame: "010304001E0003D9" (without : and \r\n)
 *   Slave: 01
 *   Function: 03 (Read Holding Registers)
 *   Byte count: 04
 *   Register 1: 001E (30 decimal)
 *   Register 2: 0003 (3 decimal)
 *   LRC: D9
 *
 * @param {string} frame - The Modbus ASCII frame from the data source
 * @returns {array} Array of register values
 */
function parse(frame) {
  // Remove any whitespace and the ':' prefix if present
  frame = frame.trim();
  if (frame.charAt(0) === ':') {
    frame = frame.substring(1);
  }

  // Check minimum frame length (address + function + LRC = 6 chars)
  if (frame.length < 6) {
    return parsedValues;
  }

  // Convert ASCII hex pairs to byte array
  var bytes = [];
  for (var i = 0; i < frame.length; i += 2) {
    if (i + 1 < frame.length) {
      var hexPair = frame.substring(i, i + 2);
      bytes.push(hexToByte(hexPair));
    }
  }

  // Validate LRC checksum
  var receivedLRC = bytes[bytes.length - 1];
  var dataBytes = bytes.slice(0, bytes.length - 1);
  var calculatedLRC = calculateLRC(dataBytes)
  if (receivedLRC !== calculatedLRC) {
    console.error("Modbus ASCII LRC checksum mismatch");
    return parsedValues;
  }

  // Extract fields
  var slaveAddress = bytes[0];
  var functionCode = bytes[1];

  // Parse based on function code
  if (functionCode === 0x03 || functionCode === 0x04) {
    // Read Holding Registers (0x03) or Read Input Registers (0x04)
    if (bytes.length < 3) {
      return parsedValues;
    }

    var byteCount = bytes[2];
    var registerCount = byteCount / 2;

    // Extract register values (16-bit big-endian)
    for (var i = 0; i < registerCount && i < numItems; i++) {
      var offset = 3 + (i * 2);
      if (offset + 1 < bytes.length) {
        var regValue = (bytes[offset] << 8) | bytes[offset + 1];
        parsedValues[i] = regValue;
      }
    }
  }

  // Write Single Register (0x06) - Echo response
  else if (functionCode === 0x06) {
    if (bytes.length >= 6) {
      var registerAddress = (bytes[2] << 8) | bytes[3];
      var registerValue = (bytes[4] << 8) | bytes[5];
      var index = registerAddress - registerOffset;

      if (index >= 0 && index < numItems) {
        parsedValues[index] = registerValue;
      }
    }
  }

  // Write Multiple Registers (0x10) - Response contains address and quantity
  else if (functionCode === 0x10) {
    if (bytes.length >= 6) {
      var startAddress = (bytes[2] << 8) | bytes[3];
      var quantity = (bytes[4] << 8) | bytes[5];
    }
  }

  // Exception response (function code + 0x80)
  else if (functionCode >= 0x80) {
    var exceptionCode = bytes[2];
    console.error("Modbus exception:", exceptionCode);
  }

  return parsedValues;
}
