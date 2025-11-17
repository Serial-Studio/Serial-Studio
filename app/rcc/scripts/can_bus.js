/**
 * CAN Bus Frames Parser
 *
 * Parses CAN bus data frames into an array of values.
 *
 * INPUT FORMAT: Binary CAN frame data (11-bit or 29-bit identifier + data bytes)
 * OUTPUT ARRAY: [value1, value2, value3, ...]  (mapped based on CAN ID)
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 *       Values are extracted and mapped based on canIdToIndexMap.
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
 * CAN frame format configuration.
 *
 * Standard CAN frame structure (assuming simple format):
 *   Byte 0-1: CAN ID (16-bit, can be 11-bit or 29-bit depending on implementation)
 *   Byte 2:   Data Length Code (DLC) - number of data bytes (0-8)
 *   Byte 3+:  Data bytes (up to 8 bytes)
 */
const useExtendedId = false;  // Set to true for 29-bit CAN IDs, false for 11-bit

/**
 * Maps CAN IDs to parsing configurations.
 *
 * Each CAN ID can have different data formats. This map defines:
 *   - indices: Array positions where extracted values should be placed
 *   - parser: Function to extract values from the 8 data bytes
 *
 * Example: CAN ID 0x100 might contain RPM in bytes 0-1 and temperature in bytes 2-3
 */
const canIdToIndexMap = {
  0x100: {
    indices: [0, 1],
    parser: function(data) {
      // Extract RPM from bytes 0-1 (16-bit big-endian)
      let rpm = (data[0] << 8) | data[1];
      // Extract temperature from byte 2
      let temp = data[2];
      return [rpm, temp];
    }
  },
  0x200: {
    indices: [2, 3],
    parser: function(data) {
      // Extract speed from bytes 0-1 (16-bit big-endian)
      let speed = (data[0] << 8) | data[1];
      // Extract gear from byte 2
      let gear = data[2];
      return [speed, gear];
    }
  },
  0x300: {
    indices: [4, 5, 6],
    parser: function(data) {
      // Extract battery voltage from bytes 0-1 (16-bit, scale by 0.01)
      let voltage = ((data[0] << 8) | data[1]) * 0.01;
      // Extract current from bytes 2-3 (16-bit, scale by 0.1)
      let current = ((data[2] << 8) | data[3]) * 0.1;
      // Extract SOC (State of Charge) from byte 4
      let soc = data[4];
      return [voltage, current, soc];
    }
  }
};

/**
 * Array to hold all parsed values.
 * Values persist between frames unless updated by new data.
 */
const parsedValues = new Array(numItems).fill(0);

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a CAN bus frame into the predefined array structure.
 *
 * How it works:
 * 1. Extracts the CAN ID from the frame
 * 2. Extracts the data length code (DLC)
 * 3. Extracts the data bytes (up to 8 bytes)
 * 4. Looks up the CAN ID in canIdToIndexMap
 * 5. Calls the custom parser function for that CAN ID
 * 6. Places the parsed values at their mapped indices
 *
 * @param {array} frame - The binary CAN frame data from the data source (byte array)
 * @returns {array} Array of values mapped according to canIdToIndexMap
 */
function parse(frame) {
  // Check minimum frame length (ID + DLC + at least some data)
  if (frame.length < 3) {
    return parsedValues;
  }

  // Extract CAN ID (assuming 16-bit big-endian in bytes 0-1)
  // For standard 11-bit ID, mask off upper bits
  let canId = (frame[0] << 8) | frame[1];
  if (!useExtendedId) {
    canId = canId & 0x7FF;  // Mask to 11 bits for standard CAN
  }

  // Extract Data Length Code (DLC) - number of data bytes
  let dlc = frame[2];
  if (dlc > 8) {
    dlc = 8;  // CAN frames have max 8 data bytes
  }

  // Extract data bytes
  let dataBytes = [];
  for (let i = 0; i < dlc && (3 + i) < frame.length; i++) {
    dataBytes.push(frame[3 + i]);
  }

  // Pad with zeros if we have fewer than 8 bytes
  while (dataBytes.length < 8) {
    dataBytes.push(0);
  }

  // Look up the parser configuration for this CAN ID
  if (canIdToIndexMap.hasOwnProperty(canId)) {
    let config = canIdToIndexMap[canId];

    // Call the custom parser function to extract values
    let values = config.parser(dataBytes);

    // Place the extracted values at their mapped indices
    for (let i = 0; i < values.length && i < config.indices.length; i++) {
      let index = config.indices[i];
      parsedValues[index] = values[i];
    }
  }

  // Return the complete array
  return parsedValues;
}
