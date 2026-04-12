/**
 * UBX Protocol (u-blox) Parser
 *
 * Parses UBX binary protocol messages from u-blox GNSS receivers.
 *
 * UBX is a proprietary binary protocol used by u-blox GPS/GNSS modules.
 * It provides comprehensive navigation data, satellite information, and
 * receiver configuration. More efficient than NMEA with higher update rates.
 *
 * INPUT FORMAT: Binary UBX message frame
 * OUTPUT ARRAY: Extracted navigation data based on message class and ID
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters (0xB5 0x62 sync characters) are automatically removed
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
 * Maps UBX message classes and IDs to parsing configurations.
 *
 * UBX messages are identified by Class + ID combination.
 *
 * Common UBX message types:
 *   0x01 0x07: NAV-PVT (Position Velocity Time Solution)
 *   0x01 0x35: NAV-SAT (Satellite Information)
 *   0x01 0x06: NAV-SOL (Navigation Solution)
 *   0x01 0x02: NAV-POSLLH (Position LLH)
 *   0x01 0x12: NAV-VELNED (Velocity NED)
 *   0x01 0x21: NAV-TIMEUTC (UTC Time Solution)
 *
 * Message 0x01 0x07 (NAV-PVT) is most comprehensive and commonly used.
 */
const messageMap = {
  "0x01-0x07": {
    // NAV-PVT: position, velocity, time in a single message
    indices: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
    parser: function(data) {
      var iTOW = extractUint32LE(data, 0);

      var longitude = extractInt32LE(data, 24) * 1e-7;
      var latitude  = extractInt32LE(data, 28) * 1e-7;
      var height    = extractInt32LE(data, 32) * 0.001;
      var hMSL      = extractInt32LE(data, 36) * 0.001;

      var velN    = extractInt32LE(data, 48) * 0.001;
      var velE    = extractInt32LE(data, 52) * 0.001;
      var velD    = extractInt32LE(data, 56) * 0.001;
      var gSpeed  = extractInt32LE(data, 60) * 0.001;
      var headMot = extractInt32LE(data, 64) * 1e-5;

      var numSV = data[23];

      return [latitude, longitude, hMSL, height, gSpeed, headMot,
              velN, velE, velD, numSV, iTOW];
    }
  },
  "0x01-0x35": {
    // NAV-SAT: satellite information summary
    indices: [11, 12, 13],
    parser: function(data) {
      var iTOW    = extractUint32LE(data, 0);
      var version = data[4];
      var numSvs  = data[5];
      return [iTOW, version, numSvs];
    }
  },
  "0x01-0x06": {
    // NAV-SOL: ECEF position, accuracy estimate, satellite count
    indices: [14, 15, 16, 17, 18],
    parser: function(data) {
      var iTOW  = extractUint32LE(data, 0);
      var ecefX = extractInt32LE(data, 12) * 0.01;
      var ecefY = extractInt32LE(data, 16) * 0.01;
      var ecefZ = extractInt32LE(data, 20) * 0.01;
      var pAcc  = extractUint32LE(data, 24) * 0.01;
      var numSV = data[47];
      return [ecefX, ecefY, ecefZ, pAcc, numSV];
    }
  },
  "0x01-0x02": {
    // NAV-POSLLH: geodetic position solution
    indices: [0, 1, 2, 3],
    parser: function(data) {
      var lon    = extractInt32LE(data, 4) * 1e-7;
      var lat    = extractInt32LE(data, 8) * 1e-7;
      var height = extractInt32LE(data, 12) * 0.001;
      var hMSL   = extractInt32LE(data, 16) * 0.001;
      return [lat, lon, hMSL, height];
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
 * Extracts a 16-bit unsigned integer from byte array (little-endian).
 * UBX protocol uses little-endian byte order.
 */
function extractUint16LE(bytes, offset) {
  return bytes[offset] | (bytes[offset + 1] << 8);
}

/**
 * Extracts a 32-bit unsigned integer from byte array (little-endian).
 */
function extractUint32LE(bytes, offset) {
  return bytes[offset] | (bytes[offset + 1] << 8) |
         (bytes[offset + 2] << 16) | (bytes[offset + 3] << 24);
}

/**
 * Extracts a 32-bit signed integer from byte array (little-endian).
 */
function extractInt32LE(bytes, offset) {
  var value = bytes[offset] | (bytes[offset + 1] << 8) |
              (bytes[offset + 2] << 16) | (bytes[offset + 3] << 24);
  return value > 2147483647 ? value - 4294967296 : value;
}

/**
 * Validates the UBX Fletcher's checksum.
 * Covers class, ID, length, and payload bytes.
 */
function validateChecksum(msgClass, msgId, length, payload, ckA, ckB) {
  var calcCkA = 0, calcCkB = 0;

  calcCkA = (calcCkA + msgClass) & 0xFF; calcCkB = (calcCkB + calcCkA) & 0xFF;
  calcCkA = (calcCkA + msgId) & 0xFF;    calcCkB = (calcCkB + calcCkA) & 0xFF;
  calcCkA = (calcCkA + (length & 0xFF)) & 0xFF;        calcCkB = (calcCkB + calcCkA) & 0xFF;
  calcCkA = (calcCkA + ((length >> 8) & 0xFF)) & 0xFF; calcCkB = (calcCkB + calcCkA) & 0xFF;

  for (var i = 0; i < payload.length; i++) {
    calcCkA = (calcCkA + payload[i]) & 0xFF;
    calcCkB = (calcCkB + calcCkA) & 0xFF;
  }

  return (calcCkA === ckA) && (calcCkB === ckB);
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a UBX message frame into the predefined array structure.
 *
 * UBX message structure:
 *   Bytes 0-1:  Sync characters (0xB5 0x62)
 *   Byte 2:     Message class
 *   Byte 3:     Message ID
 *   Bytes 4-5:  Payload length (little-endian)
 *   Bytes 6-n:  Payload
 *   Byte n+1:   Checksum A (CK_A)
 *   Byte n+2:   Checksum B (CK_B)
 *
 * Note: Serial Studio removes sync characters, so this parser receives:
 * [class][id][len_lo][len_hi][payload...][ck_a][ck_b]
 *
 * How it works:
 * 1. Extracts message class and ID
 * 2. Extracts payload length
 * 3. Validates checksum
 * 4. Looks up parser for this class+ID combination
 * 5. Calls appropriate parser
 * 6. Stores values at their mapped indices
 *
 * @param {array} frame - The binary UBX frame from the data source (byte array)
 * @returns {array} Array of values mapped according to messageMap
 */
function parse(frame) {
  if (frame.length < 6)
    return parsedValues;

  var msgClass     = frame[0];
  var msgId        = frame[1];
  var payloadLength = frame[2] | (frame[3] << 8);

  if (frame.length < 4 + payloadLength + 2) {
    console.error("Incomplete UBX frame");
    return parsedValues;
  }

  var payload = [];
  for (var i = 0; i < payloadLength; i++)
    payload.push(frame[4 + i]);

  var ckA = frame[4 + payloadLength];
  var ckB = frame[4 + payloadLength + 1];

  if (!validateChecksum(msgClass, msgId, payloadLength, payload, ckA, ckB)) {
    console.error("UBX checksum validation failed");
    return parsedValues;
  }

  var messageKey = "0x" + msgClass.toString(16).padStart(2, '0') +
                   "-0x" + msgId.toString(16).padStart(2, '0');

  if (!messageMap.hasOwnProperty(messageKey))
    return parsedValues;

  var config = messageMap[messageKey];

  try {
    var values = config.parser(payload);
    for (var i = 0; i < values.length && i < config.indices.length; i++)
      parsedValues[config.indices[i]] = values[i];
  }
  catch (e) {
    console.error("UBX parsing error:", e.message);
  }

  return parsedValues;
}
