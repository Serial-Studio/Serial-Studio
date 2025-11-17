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
  // ATTITUDE message (ID 30): roll, pitch, yaw, rollspeed, pitchspeed, yawspeed
  30: {
    indices: [0, 1, 2],
    parser: function(payload) {
      // Extract roll, pitch, yaw (4 bytes each, float, little-endian)
      var roll = extractFloat32LE(payload, 4);
      var pitch = extractFloat32LE(payload, 8);
      var yaw = extractFloat32LE(payload, 12);
      return [roll, pitch, yaw];
    }
  },
  // VFR_HUD message (ID 74): airspeed, groundspeed, heading, throttle, alt, climb
  74: {
    indices: [3, 4, 5, 6],
    parser: function(payload) {
      var airspeed = extractFloat32LE(payload, 0);
      var groundspeed = extractFloat32LE(payload, 4);
      var heading = extractInt16LE(payload, 8);
      var throttle = extractUint16LE(payload, 10);
      return [airspeed, groundspeed, heading, throttle];
    }
  },
  // GLOBAL_POSITION_INT message (ID 33): lat, lon, alt, relative_alt, vx, vy, vz, hdg
  33: {
    indices: [7, 8, 9],
    parser: function(payload) {
      var lat = extractInt32LE(payload, 4) / 1e7;    // Latitude in degrees
      var lon = extractInt32LE(payload, 8) / 1e7;    // Longitude in degrees
      var alt = extractInt32LE(payload, 12) / 1000;  // Altitude in meters
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
  var buffer = [];
  for (var i = 0; i < 4; i++) {
    buffer.push(bytes[offset + i]);
  }

  // Convert bytes to IEEE 754 float (simplified)
  var value = 0;
  var sign = (buffer[3] & 0x80) ? -1 : 1;
  var exponent = ((buffer[3] & 0x7F) << 1) | (buffer[2] >> 7);
  var mantissa = ((buffer[2] & 0x7F) << 16) | (buffer[1] << 8) | buffer[0];

  if (exponent === 0) {
    value = sign * mantissa * Math.pow(2, -149);
  } else if (exponent === 0xFF) {
    value = mantissa ? NaN : sign * Infinity;
  } else {
    mantissa = mantissa | 0x800000;
    value = sign * mantissa * Math.pow(2, exponent - 150);
  }

  return value;
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
  // Check minimum frame length
  if (frame.length < 8) {
    return parsedValues;
  }

  // Check start marker
  var expectedMarker = (mavlinkVersion === 2) ? 0xFD : 0xFE;
  if (frame[0] !== expectedMarker) {
    return parsedValues;
  }

  // Extract payload length and message ID
  var payloadLength = frame[1];
  var messageId = frame[5];  // For MAVLink v1

  // Extract payload
  var payload = [];
  var payloadStart = 6;
  for (var i = 0; i < payloadLength && (payloadStart + i) < frame.length; i++) {
    payload.push(frame[payloadStart + i]);
  }

  // Look up parser for this message ID
  if (messageIdToIndexMap.hasOwnProperty(messageId)) {
    var config = messageIdToIndexMap[messageId];

    // Call the custom parser function
    var values = config.parser(payload);

    // Store values at mapped indices
    for (var i = 0; i < values.length && i < config.indices.length; i++) {
      var index = config.indices[i];
      parsedValues[index] = values[i];
    }
  }

  return parsedValues;
}
