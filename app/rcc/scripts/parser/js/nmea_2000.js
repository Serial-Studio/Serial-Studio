/**
 * NMEA 2000 Messages Parser
 *
 * Parses NMEA 2000 (N2K) protocol messages from marine CAN bus networks.
 *
 * NMEA 2000 is a CAN-based communication standard used in marine electronics
 * for navigation, engine data, and vessel monitoring. Unlike NMEA 0183 (ASCII),
 * NMEA 2000 uses binary messages over a CAN bus.
 *
 * INPUT FORMAT: Binary NMEA 2000 CAN frame
 * OUTPUT ARRAY: Extracted values based on PGN (Parameter Group Number)
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
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
 * Maps NMEA 2000 PGNs (Parameter Group Numbers) to parsing configurations.
 *
 * Common PGNs:
 *   127250: Vessel Heading
 *   127251: Rate of Turn
 *   127257: Attitude (Roll, Pitch, Yaw)
 *   128259: Speed (Water referenced)
 *   128267: Water Depth
 *   129025: Position (Lat/Lon)
 *   129026: COG & SOG (Course and Speed over Ground)
 *   129029: GNSS Position Data
 *   130310: Environmental Parameters (Water temp, etc.)
 *   130311: Environmental Parameters (Atmospheric)
 *
 * Each entry defines which indices to populate with parsed values.
 */
const pgnToIndexMap = {
  127257: {
    // Attitude: yaw, pitch, roll in radians × 10000 → converted to degrees
    indices: [0, 1, 2],
    parser: function(data) {
      var yaw   = extractInt16LE(data, 1) * 0.0001 * 180 / Math.PI;
      var pitch = extractInt16LE(data, 3) * 0.0001 * 180 / Math.PI;
      var roll  = extractInt16LE(data, 5) * 0.0001 * 180 / Math.PI;
      return [yaw, pitch, roll];
    }
  },
  129025: {
    // Position Rapid Update: latitude and longitude in degrees × 10⁷
    indices: [3, 4],
    parser: function(data) {
      var latitude  = extractInt32LE(data, 0) * 1e-7;
      var longitude = extractInt32LE(data, 4) * 1e-7;
      return [latitude, longitude];
    }
  },
  129026: {
    // COG & SOG Rapid Update: course in radians → degrees, speed m/s → knots
    indices: [5, 6, 7],
    parser: function(data) {
      var cogRef = data[1] & 0x03;
      var cog    = extractUint16LE(data, 2) * 0.0001 * 180 / Math.PI;
      var sog    = extractUint16LE(data, 4) * 0.01 * 1.94384;
      return [cog, sog, cogRef];
    }
  },
  128259: {
    // Speed Water Referenced: water speed and ground speed, m/s → knots
    indices: [8, 9],
    parser: function(data) {
      var waterSpeed  = extractUint16LE(data, 1) * 0.01 * 1.94384;
      var groundSpeed = extractUint16LE(data, 3) * 0.01 * 1.94384;
      return [waterSpeed, groundSpeed];
    }
  },
  128267: {
    // Water Depth: depth in metres, offset in metres
    indices: [10, 11],
    parser: function(data) {
      var depth  = extractUint32LE(data, 1) * 0.01;
      var offset = extractInt16LE(data, 5) * 0.001;
      return [depth, offset];
    }
  },
  130310: {
    // Environmental Water: water temp and outside temp, Kelvin → Celsius
    indices: [12, 13],
    parser: function(data) {
      var waterTemp   = extractUint16LE(data, 1) * 0.01 - 273.15;
      var outsideTemp = extractUint16LE(data, 3) * 0.01 - 273.15;
      return [waterTemp, outsideTemp];
    }
  },
  130311: {
    // Environmental Atmospheric: pressure in hPa, air temp Celsius, humidity %
    indices: [14, 15, 16],
    parser: function(data) {
      var pressure = extractUint16LE(data, 1);
      var airTemp  = extractUint16LE(data, 3) * 0.01 - 273.15;
      var humidity = extractInt16LE(data, 5) * 0.004;
      return [pressure, airTemp, humidity];
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
 */
function extractUint16LE(bytes, offset) {
  return bytes[offset] | (bytes[offset + 1] << 8);
}

/**
 * Extracts a 16-bit signed integer from byte array (little-endian).
 */
function extractInt16LE(bytes, offset) {
  var value = bytes[offset] | (bytes[offset + 1] << 8);
  return value > 32767 ? value - 65536 : value;
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
 * Extracts the PGN from a 29-bit CAN identifier.
 * NMEA 2000 uses extended CAN IDs with PGN encoded in bits 8-25.
 */
function extractPGN(canId) {
  var pf = (canId >> 16) & 0xFF;
  var ps = (canId >> 8) & 0xFF;
  var dp = (canId >> 24) & 0x01;

  // PDU1 (PF < 240): destination-specific, PGN excludes PS field
  if (pf < 240)
    return (dp << 16) | (pf << 8);

  // PDU2 (PF >= 240): broadcast, PGN includes PS field
  return (dp << 16) | (pf << 8) | ps;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses an NMEA 2000 CAN frame into the predefined array structure.
 *
 * NMEA 2000 frame structure (assuming preprocessed CAN frame):
 *   Bytes 0-3:  CAN ID (29-bit extended, contains PGN)
 *   Byte 4:     Data length (0-8)
 *   Bytes 5-12: Data payload (up to 8 bytes)
 *
 * How it works:
 * 1. Extracts the CAN ID from the frame
 * 2. Extracts the PGN from the CAN ID
 * 3. Extracts the data payload
 * 4. Looks up the PGN in pgnToIndexMap
 * 5. Calls the appropriate parser for that PGN
 * 6. Stores values at their mapped indices
 *
 * @param {array} frame - The binary NMEA 2000 frame from the data source (byte array)
 * @returns {array} Array of values mapped according to pgnToIndexMap
 */
function parse(frame) {
  if (frame.length < 6)
    return parsedValues;

  var canId      = frame[0] | (frame[1] << 8) | (frame[2] << 16) | (frame[3] << 24);
  var dataLength = Math.min(frame[4], 8);

  var data = [];
  for (var i = 0; i < dataLength && (5 + i) < frame.length; i++)
    data.push(frame[5 + i]);

  var pgn = extractPGN(canId);

  if (!pgnToIndexMap.hasOwnProperty(pgn))
    return parsedValues;

  var config = pgnToIndexMap[pgn];

  try {
    var values = config.parser(data);
    for (var i = 0; i < values.length && i < config.indices.length; i++)
      parsedValues[config.indices[i]] = values[i];
  }
  catch (e) {
    console.error("NMEA 2000 parsing error:", e.message);
  }

  return parsedValues;
}
