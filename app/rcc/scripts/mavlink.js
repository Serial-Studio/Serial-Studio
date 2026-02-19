/**
 * MAVLink Messages Parser
 *
 * Parses MAVLink protocol messages (commonly used in drones and robotics).
 *
 * MAVLink is a lightweight messaging protocol for communicating with drones,
 * ground control stations, and onboard drone components.
 *
 * INPUT FORMAT: Binary MAVLink v1 or v2 frame
 * OUTPUT ARRAY: [value1, value2, value3, ...]  (extracted from message payloads)
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 *       Values are extracted based on message ID and mapped to array indices.
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
 * MAVLink protocol version to parse.
 * Set to 1 for MAVLink v1 (0xFE start marker)
 * Set to 2 for MAVLink v2 (0xFD start marker)
 */
const mavlinkVersion = 1;

/**
 * Maps MAVLink message IDs to parsing configurations.
 *
 * Common MAVLink message IDs:
 *   0:   HEARTBEAT
 *   24:  GPS_RAW_INT
 *   30:  ATTITUDE
 *   33:  GLOBAL_POSITION_INT
 *   74:  VFR_HUD
 *   147: BATTERY_STATUS
 *
 * Each entry defines:
 *   - indices: Array positions where extracted values should be placed
 *   - parser: Function to extract values from the message payload
 */
const messageIdToIndexMap = {
  // ATTITUDE (ID 30): roll, pitch, yaw angles in radians (4 bytes each, float, little-endian)
  30: {
    indices: [0, 1, 2],
    parser: function(payload) {
      var roll  = extractFloat32LE(payload, 4);
      var pitch = extractFloat32LE(payload, 8);
      var yaw   = extractFloat32LE(payload, 12);
      return [roll, pitch, yaw];
    }
  },
  // VFR_HUD (ID 74): airspeed, groundspeed, heading, throttle
  74: {
    indices: [3, 4, 5, 6],
    parser: function(payload) {
      var airspeed    = extractFloat32LE(payload, 0);
      var groundspeed = extractFloat32LE(payload, 4);
      var heading     = extractInt16LE(payload, 8);
      var throttle    = extractUint16LE(payload, 10);
      return [airspeed, groundspeed, heading, throttle];
    }
  },
  // GLOBAL_POSITION_INT (ID 33): lat/lon in 1e-7 degrees, alt in mm
  33: {
    indices: [7, 8, 9],
    parser: function(payload) {
      var lat = extractInt32LE(payload, 4) / 1e7;
      var lon = extractInt32LE(payload, 8) / 1e7;
      var alt = extractInt32LE(payload, 12) / 1000;
      return [lat, lon, alt];
    }
  }
};

/**
 * Array to hold all parsed values.
 * Values persist between frames unless updated by new data.
 */
const parsedValues = new Array(numItems).fill(0);

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------

/**
 * Extracts a 32-bit float from byte array (little-endian).
 */
function extractFloat32LE(bytes, offset) {
  var b0 = bytes[offset], b1 = bytes[offset + 1];
  var b2 = bytes[offset + 2], b3 = bytes[offset + 3];

  var sign     = (b3 & 0x80) ? -1 : 1;
  var exponent = ((b3 & 0x7F) << 1) | (b2 >> 7);
  var mantissa = ((b2 & 0x7F) << 16) | (b1 << 8) | b0;

  if (exponent === 0)
    return sign * mantissa * Math.pow(2, -149);

  if (exponent === 0xFF)
    return mantissa ? NaN : sign * Infinity;

  return sign * (mantissa | 0x800000) * Math.pow(2, exponent - 150);
}

/**
 * Extracts a 16-bit signed integer from byte array (little-endian).
 */
function extractInt16LE(bytes, offset) {
  var value = bytes[offset] | (bytes[offset + 1] << 8);
  return value > 32767 ? value - 65536 : value;
}

/**
 * Extracts a 16-bit unsigned integer from byte array (little-endian).
 */
function extractUint16LE(bytes, offset) {
  return bytes[offset] | (bytes[offset + 1] << 8);
}

/**
 * Extracts a 32-bit signed integer from byte array (little-endian).
 */
function extractInt32LE(bytes, offset) {
  var value = bytes[offset] | (bytes[offset + 1] << 8) |
              (bytes[offset + 2] << 16) | (bytes[offset + 3] << 24);
  return value > 2147483647 ? value - 4294967296 : value;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a MAVLink frame into the predefined array structure.
 *
 * MAVLink v1 frame structure:
 *   Byte 0:    Start marker (0xFE)
 *   Byte 1:    Payload length
 *   Byte 2:    Packet sequence
 *   Byte 3:    System ID
 *   Byte 4:    Component ID
 *   Byte 5:    Message ID
 *   Byte 6-n:  Payload
 *   Byte n+1-n+2: Checksum
 *
 * How it works:
 * 1. Validates the start marker (0xFE for v1, 0xFD for v2)
 * 2. Extracts the message ID
 * 3. Extracts the payload bytes
 * 4. Looks up the message ID in messageIdToIndexMap
 * 5. Calls the custom parser for that message type
 * 6. Stores values at their mapped indices
 *
 * @param {array} frame - The binary MAVLink frame from the data source (byte array)
 * @returns {array} Array of values mapped according to messageIdToIndexMap
 */
function parse(frame) {
  if (frame.length < 8)
    return parsedValues;

  var expectedMarker = (mavlinkVersion === 2) ? 0xFD : 0xFE;
  if (frame[0] !== expectedMarker)
    return parsedValues;

  var payloadLength = frame[1];
  var messageId = frame[5];

  var payload = [];
  for (var i = 0; i < payloadLength && (6 + i) < frame.length; i++)
    payload.push(frame[6 + i]);

  if (!messageIdToIndexMap.hasOwnProperty(messageId))
    return parsedValues;

  var config = messageIdToIndexMap[messageId];
  var values = config.parser(payload);

  for (var i = 0; i < values.length && i < config.indices.length; i++)
    parsedValues[config.indices[i]] = values[i];

  return parsedValues;
}
