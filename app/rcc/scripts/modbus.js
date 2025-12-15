/**
 * Modbus Frame Parser for Serial Studio
 *
 * Parses Modbus protocol frames into an array of values.
 * Compatible with Serial Studio's Modbus TCP/RTU driver.
 *
 * INPUT FORMAT: Binary Modbus ADU (no CRC - handled by Qt/Serial Studio)
 *   [Slave Address][Function Code][Data...]
 *
 * OUTPUT ARRAY: Extracted register values from the Modbus response
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets configured in Serial Studio.
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
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a Modbus response frame into an array of register values.
 *
 * Frame structure (as emitted by Serial Studio Modbus driver):
 *   Byte 0:     Slave address
 *   Byte 1:     Function code
 *   Byte 2-n:   Data (depends on function code)
 *
 * Common function codes:
 *   0x01: Read Coils
 *   0x02: Read Discrete Inputs
 *   0x03: Read Holding Registers
 *   0x04: Read Input Registers
 *   0x05: Write Single Coil
 *   0x06: Write Single Register
 *   0x0F: Write Multiple Coils
 *   0x10: Write Multiple Registers
 *
 * Example frame (hex): [01 03 0C 00 01 00 00 00 2A 00 64 00 01 00 32]
 *   Slave: 0x01
 *   Function: 0x03 (Read Holding Registers)
 *   Byte count: 0x0C (12 bytes = 6 registers)
 *   Register 0: 0x0001 (1 - E-Stop)
 *   Register 1: 0x0000 (0 - Start Button)
 *   Register 2: 0x002A (42 - Temperature °C)
 *   Register 3: 0x0064 (100 - Pressure kPa)
 *   Register 4: 0x0001 (1 - Motor ON)
 *   Register 5: 0x0032 (50 - Valve 50%)
 *
 * @param {array} frame - Binary Modbus frame (byte array)
 * @returns {array} Array of register values
 */
function parse(frame) {
  // Check minimum frame length (address + function + data)
  if (frame.length < 3) {
    return parsedValues;
  }

  // Extract fields
  var slaveAddress = frame[0];
  var functionCode = frame[1];

  //----------------------------------------------------------------------------
  // Read Coils (0x01) or Read Discrete Inputs (0x02)
  //----------------------------------------------------------------------------
  if (functionCode === 0x01 || functionCode === 0x02) {
    var byteCount = frame[2];
    var bitIndex = 0;

    for (var byteIdx = 0; byteIdx < byteCount && bitIndex < numItems; byteIdx++) {
      var dataByte = frame[3 + byteIdx];
      for (var bit = 0; bit < 8 && bitIndex < numItems; bit++) {
        parsedValues[bitIndex] = (dataByte >> bit) & 0x01;
        bitIndex++;
      }
    }
  }

  //----------------------------------------------------------------------------
  // Read Holding Registers (0x03) or Read Input Registers (0x04)
  //----------------------------------------------------------------------------
  else if (functionCode === 0x03 || functionCode === 0x04) {
    var byteCount = frame[2];
    var registerCount = byteCount / 2;

    for (var i = 0; i < registerCount && i < numItems; i++) {
      var offset = 3 + (i * 2);
      if (offset + 1 < frame.length) {
        var regValue = (frame[offset] << 8) | frame[offset + 1];
        parsedValues[i] = regValue;
      }
    }
  }

  //----------------------------------------------------------------------------
  // Write Single Coil (0x05) - Echo response
  //----------------------------------------------------------------------------
  else if (functionCode === 0x05) {
    if (frame.length >= 6) {
      var coilAddress = (frame[2] << 8) | frame[3];
      var coilValue = (frame[4] << 8) | frame[5];
      var index = coilAddress - registerOffset;

      if (index >= 0 && index < numItems) {
        parsedValues[index] = (coilValue === 0xFF00) ? 1 : 0;
      }
    }
  }

  //----------------------------------------------------------------------------
  // Write Single Register (0x06) - Echo response
  //----------------------------------------------------------------------------
  else if (functionCode === 0x06) {
    if (frame.length >= 6) {
      var registerAddress = (frame[2] << 8) | frame[3];
      var registerValue = (frame[4] << 8) | frame[5];
      var index = registerAddress - registerOffset;

      if (index >= 0 && index < numItems) {
        parsedValues[index] = registerValue;
      }
    }
  }

  //----------------------------------------------------------------------------
  // Write Multiple Coils (0x0F) - Response contains address and quantity
  //----------------------------------------------------------------------------
  else if (functionCode === 0x0F) {
    if (frame.length >= 6) {
      var startAddress = (frame[2] << 8) | frame[3];
      var quantity = (frame[4] << 8) | frame[5];
      // Response only confirms write, no data returned
    }
  }

  //----------------------------------------------------------------------------
  // Write Multiple Registers (0x10) - Response contains address and quantity
  //----------------------------------------------------------------------------
  else if (functionCode === 0x10) {
    if (frame.length >= 6) {
      var startAddress = (frame[2] << 8) | frame[3];
      var quantity = (frame[4] << 8) | frame[5];
      // Response only confirms write, no data returned
    }
  }

  //----------------------------------------------------------------------------
  // Exception response (function code + 0x80)
  //----------------------------------------------------------------------------
  else if (functionCode >= 0x80) {
    if (frame.length >= 3) {
      var exceptionCode = frame[2];
      console.log("Modbus exception: 0x" + exceptionCode.toString(16));
    }
  }

  return parsedValues;
}
