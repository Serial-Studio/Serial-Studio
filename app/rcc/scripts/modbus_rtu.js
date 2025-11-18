/**
 * Modbus RTU Frame Parser
 *
 * Parses Modbus RTU protocol frames into an array of values.
 *
 * Modbus RTU is a binary serial communication protocol commonly used in industrial
 * automation. Unlike Modbus ASCII, RTU transmits data in compact binary format.
 *
 * INPUT FORMAT: Binary Modbus RTU frame
 * OUTPUT ARRAY: Extracted register values from the Modbus response
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 *       This parser handles Modbus RTU response frames.
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
 * Calculates Modbus RTU CRC-16 checksum.
 * Uses the standard Modbus polynomial 0xA001.
 */
function calculateCRC16(data) {
  var crc = 0xFFFF;

  for (var i = 0; i < data.length; i++) {
    crc = crc ^ data[i];

    for (var j = 0; j < 8; j++) {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc = crc >> 1;
    }

    return crc;
  }

  //------------------------------------------------------------------------------
  // Frame Parser Function
  //------------------------------------------------------------------------------

  /**
 * Parses a Modbus RTU response frame into an array of register values.
 *
 * Modbus RTU frame structure:
 *   Byte 0:     Slave address
 *   Byte 1:     Function code
 *   Byte 2-n:   Data (depends on function code)
 *   Byte n+1:   CRC low byte
 *   Byte n+2:   CRC high byte
 *
 * Common function codes:
 *   0x03: Read Holding Registers
 *   0x04: Read Input Registers
 *   0x06: Write Single Register
 *   0x10: Write Multiple Registers
 *
 * How it works:
 * 1. Validates minimum frame length
 * 2. Extracts and validates CRC-16 checksum
 * 3. Parses function code
 * 4. Extracts register values based on function code
 * 5. Stores values in the output array
 *
 * Example frame (hex): [01 03 04 00 1E 00 03 B9 F4]
 *   Slave: 0x01
 *   Function: 0x03 (Read Holding Registers)
 *   Byte count: 0x04
 *   Register 1: 0x001E (30 decimal)
 *   Register 2: 0x0003 (3 decimal)
 *   CRC: 0xF4B9
 *
 * @param {array} frame - The binary Modbus RTU frame from the data source (byte array)
 * @returns {array} Array of register values
 */
  function parse(frame) {
    // Check minimum frame length (address + function + CRC = 4 bytes)
    if (frame.length < 4) {
      return parsedValues;
    }

    // Extract CRC from last two bytes (little-endian)
    var receivedCRC = frame[frame.length - 2] | (frame[frame.length - 1] << 8);

    // Calculate CRC for all bytes except the CRC itself
    var dataBytes = [];
    for (var i = 0; i < frame.length - 2; i++) {
      dataBytes.push(frame[i]);
    }

    // Validate CRC
    var calculatedCRC = calculateCRC16(dataBytes);
    if (receivedCRC !== calculatedCRC) {
      console.error("Modbus RTU CRC mismatch");
      return parsedValues;
    }

    // Extract fields
    var slaveAddress = frame[0];
    var functionCode = frame[1];

    // Parse based on function code
    if (functionCode === 0x03 || functionCode === 0x04) {
      // Read Holding Registers (0x03) or Read Input Registers (0x04)
      if (frame.length < 3) {
        return parsedValues;
      }

      // Extract register values (16-bit big-endian)
      var byteCount = frame[2];
      var registerCount = byteCount / 2;
      for (var i = 0; i < registerCount && i < numItems; i++) {
        var offset = 3 + (i * 2);
        if (offset + 1 < frame.length - 2) {
          var regValue = (frame[offset] << 8) | frame[offset + 1];
          parsedValues[i] = regValue;
        }
      }
    }

    // Write Single Register (0x06) - Echo response
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

    // Write Multiple Registers (0x10) - Response contains address and quantity
    else if (functionCode === 0x10) {
      if (frame.length >= 6) {
        var startAddress = (frame[2] << 8) | frame[3];
        var quantity = (frame[4] << 8) | frame[5];
      }
    }

    // Exception response (function code + 0x80)
    else if (functionCode >= 0x80) {
      if (frame.length >= 3) {
        var exceptionCode = frame[2];
        console.error("Modbus RTU exception:", exceptionCode);
      }
    }

    return parsedValues;
  }
