/**
 * SiRF Binary Protocol Parser
 *
 * Parses SiRF binary protocol messages from GPS receivers.
 *
 * SiRF binary protocol is a proprietary format used by SiRFstar GPS chipsets.
 * It provides more detailed navigation data than NMEA and updates at higher rates.
 * Common in older GPS modules and some consumer GPS devices.
 *
 * INPUT FORMAT: Binary SiRF message frame
 * OUTPUT ARRAY: Extracted navigation data based on message ID
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters (0xA0 0xA2 ... 0xB0 0xB3) are automatically removed
 *       by Serial Studio.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 20;

/**
 * Maps SiRF message IDs to parsing configurations.
 *
 * Common SiRF message IDs:
 *   2:  Navigation Library Measurement Data
 *   4:  Measured Tracker Data
 *   7:  Clock Status Data
 *   41: Geodetic Navigation Data (most commonly used)
 *   52: 1PPS Time
 *
 * Message ID 41 (Geodetic Navigation Data) contains:
 *   - Latitude, Longitude, Altitude
 *   - Speed, Heading
 *   - Number of satellites
 *   - HDOP, fix mode
 */
const messageIdMap = {
  41: {  // Geodetic Navigation Data
    indices: [0, 1, 2, 3, 4, 5, 6, 7, 8],
    parser: function(data) {
      // Message ID 41 structure (all multi-byte values are big-endian)
      var navValid = extractUint16BE(data, 0);
      var navType = extractUint16BE(data, 2);

      // Position (WGS84)
      var latitude = extractInt32BE(data, 23) * 1e-7;   // degrees
      var longitude = extractInt32BE(data, 27) * 1e-7;  // degrees
      var altEllipsoid = extractInt32BE(data, 31) * 0.01; // meters
      var altMSL = extractInt32BE(data, 35) * 0.01;     // meters above mean sea level

      // Velocity
      var speedOverGround = extractInt16BE(data, 40) * 0.01; // m/s
      var courseOverGround = extractUint16BE(data, 42) * 0.01; // degrees

      // Satellites and accuracy
      var numSatellites = data[88];
      var hdop = data[89] * 0.2;  // HDOP * 5

      return [latitude, longitude, altMSL, altEllipsoid,
              speedOverGround, courseOverGround, numSatellites, hdop, navValid];
    }
  },
  2: {  // Navigation Library Measurement Data
    indices: [10, 11, 12, 13],
    parser: function(data) {
      // ECEF position
      var xPos = extractInt32BE(data, 1);
      var yPos = extractInt32BE(data, 5);
      var zPos = extractInt32BE(data, 9);

      // Number of satellites
      var numSV = data[28];

      return [xPos, yPos, zPos, numSV];
    }
  },
  4: {  // Measured Tracker Data
    indices: [14, 15, 16],
    parser: function(data) {
      // Week number and time of week
      var weekNumber = extractUint16BE(data, 1);
      var timeOfWeek = extractUint32BE(data, 3) * 0.01; // seconds

      // Number of channels (satellites being tracked)
      var numChannels = data[7];

      return [weekNumber, timeOfWeek, numChannels];
    }
  },
  7: {  // Clock Status Data
    indices: [17, 18, 19],
    parser: function(data) {
      // Week number and time of week
      var weekNumber = extractUint16BE(data, 1);
      var timeOfWeek = extractUint32BE(data, 3);

      // Number of satellites used
      var numSV = data[7];

      // Clock drift and bias
      var clockDrift = extractInt32BE(data, 8);

      return [weekNumber, timeOfWeek, numSV];
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
 * Extracts a 16-bit unsigned integer from byte array (big-endian).
 * SiRF protocol uses big-endian byte order.
 */
function extractUint16BE(bytes, offset) {
  return (bytes[offset] << 8) | bytes[offset + 1];
}

/**
 * Extracts a 16-bit signed integer from byte array (big-endian).
 */
function extractInt16BE(bytes, offset) {
  var value = (bytes[offset] << 8) | bytes[offset + 1];
  return value > 32767 ? value - 65536 : value;
}

/**
 * Extracts a 32-bit unsigned integer from byte array (big-endian).
 */
function extractUint32BE(bytes, offset) {
  return (bytes[offset] << 24) | (bytes[offset + 1] << 16) |
         (bytes[offset + 2] << 8) | bytes[offset + 3];
}

/**
 * Extracts a 32-bit signed integer from byte array (big-endian).
 */
function extractInt32BE(bytes, offset) {
  var value = (bytes[offset] << 24) | (bytes[offset + 1] << 16) |
              (bytes[offset + 2] << 8) | bytes[offset + 3];
  return value > 2147483647 ? value - 4294967296 : value;
}

/**
 * Validates SiRF checksum.
 * Checksum is sum of all payload bytes & 0x7FFF.
 */
function validateChecksum(payload, receivedChecksum) {
  var sum = 0;
  for (var i = 0; i < payload.length; i++) {
    sum += payload[i];
  }
  sum &= 0x7FFF;
  return sum === receivedChecksum;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a SiRF binary message frame into the predefined array structure.
 *
 * SiRF binary message structure:
 *   Bytes 0-1:    Start sequence (0xA0 0xA2)
 *   Bytes 2-3:    Payload length (big-endian, 15 bits)
 *   Byte 4:       Message ID
 *   Bytes 5-n:    Payload data
 *   Bytes n+1-n+2: Checksum (big-endian, 15 bits)
 *   Bytes n+3-n+4: End sequence (0xB0 0xB3)
 *
 * Note: Serial Studio removes the start/end sequences, so this parser
 * receives: [length_hi][length_lo][msg_id][payload...][chk_hi][chk_lo]
 *
 * How it works:
 * 1. Extracts payload length from first two bytes
 * 2. Extracts message ID
 * 3. Validates checksum
 * 4. Calls appropriate parser for the message ID
 * 5. Stores values at their mapped indices
 *
 * @param {array} frame - The binary SiRF frame from the data source (byte array)
 * @returns {array} Array of values mapped according to messageIdMap
 */
function parse(frame) {
  // Check minimum frame length (length + ID + checksum = 5 bytes)
  if (frame.length < 5) {
    return parsedValues;
  }

  // Extract payload length (first 2 bytes, big-endian, 15 bits)
  var payloadLength = ((frame[0] & 0x7F) << 8) | frame[1];

  // Validate frame length
  if (frame.length < 2 + payloadLength + 2) {
    console.error("Incomplete SiRF frame");
    return parsedValues;
  }

  // Extract message ID
  var messageId = frame[2];

  // Extract payload (message ID + data)
  var payload = [];
  for (var i = 2; i < 2 + payloadLength; i++) {
    payload.push(frame[i]);
  }

  // Extract checksum (last 2 bytes of expected frame, big-endian, 15 bits)
  var checksumOffset = 2 + payloadLength;
  var receivedChecksum = ((frame[checksumOffset] & 0x7F) << 8) | frame[checksumOffset + 1];

  // Validate checksum
  if (!validateChecksum(payload, receivedChecksum)) {
    console.error("SiRF checksum validation failed");
    return parsedValues;
  }

  // Look up parser for this message ID
  if (messageIdMap.hasOwnProperty(messageId)) {
    var config = messageIdMap[messageId];

    try {
      // Call the custom parser function (payload without message ID)
      var values = config.parser(payload.slice(1));

      // Store values at mapped indices
      for (var i = 0; i < values.length && i < config.indices.length; i++) {
        var index = config.indices[i];
        parsedValues[index] = values[i];
      }
    }
    catch (e) {
      console.error("SiRF parsing error:", e.message);
    }
  }

  return parsedValues;
}
