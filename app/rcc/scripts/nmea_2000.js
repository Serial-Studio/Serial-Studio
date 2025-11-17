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
  127257: {  // Attitude
    indices: [0, 1, 2],
    parser: function(data) {
      // Yaw (heading), Pitch, Roll in radians * 10000
      var yaw = extractInt16LE(data, 1) * 0.0001;      // Convert to radians
      var pitch = extractInt16LE(data, 3) * 0.0001;
      var roll = extractInt16LE(data, 5) * 0.0001;

      // Convert to degrees
      yaw = yaw * 180 / Math.PI;
      pitch = pitch * 180 / Math.PI;
      roll = roll * 180 / Math.PI;

      return [yaw, pitch, roll];
    }
  },
  129025: {  // Position - Rapid Update
    indices: [3, 4],
    parser: function(data) {
      // Latitude and Longitude in degrees * 10000000
      var latitude = extractInt32LE(data, 0) * 1e-7;
      var longitude = extractInt32LE(data, 4) * 1e-7;
      return [latitude, longitude];
    }
  },
  129026: {  // COG & SOG - Rapid Update
    indices: [5, 6, 7],
    parser: function(data) {
      // Course Over Ground and Speed Over Ground
      var cogRef = data[1] & 0x03;  // 0=True, 1=Magnetic
      var cog = extractUint16LE(data, 2) * 0.0001;  // Radians
      var sog = extractUint16LE(data, 4) * 0.01;    // m/s

      // Convert COG to degrees
      cog = cog * 180 / Math.PI;
      // Convert SOG to knots
      sog = sog * 1.94384;

      return [cog, sog, cogRef];
    }
  },
  128259: {  // Speed - Water referenced
    indices: [8, 9],
    parser: function(data) {
      // Water speed and ground speed
      var waterSpeed = extractUint16LE(data, 1) * 0.01;  // m/s
      var groundSpeed = extractUint16LE(data, 3) * 0.01; // m/s

      // Convert to knots
      waterSpeed = waterSpeed * 1.94384;
      groundSpeed = groundSpeed * 1.94384;

      return [waterSpeed, groundSpeed];
    }
  },
  128267: {  // Water Depth
    indices: [10, 11],
    parser: function(data) {
      // Depth below transducer and offset
      var depth = extractUint32LE(data, 1) * 0.01;  // meters
      var offset = extractInt16LE(data, 5) * 0.001; // meters
      return [depth, offset];
    }
  },
  130310: {  // Environmental - Water
    indices: [12, 13],
    parser: function(data) {
      // Water temperature and outside temperature
      var waterTemp = extractUint16LE(data, 1) * 0.01;     // Kelvin
      var outsideTemp = extractUint16LE(data, 3) * 0.01;   // Kelvin

      // Convert Kelvin to Celsius
      waterTemp = waterTemp - 273.15;
      outsideTemp = outsideTemp - 273.15;

      return [waterTemp, outsideTemp];
    }
  },
  130311: {  // Environmental - Atmospheric
    indices: [14, 15, 16],
    parser: function(data) {
      // Atmospheric pressure, air temperature, humidity
      var pressure = extractUint16LE(data, 1) * 100;    // Pascals
      var airTemp = extractUint16LE(data, 3) * 0.01;    // Kelvin
      var humidity = extractInt16LE(data, 5) * 0.004;   // Percent

      // Convert Kelvin to Celsius
      airTemp = airTemp - 273.15;
      // Convert Pascals to hPa (millibars)
      pressure = pressure / 100;

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
 * Extracts PGN from 29-bit CAN identifier.
 * NMEA 2000 uses extended CAN IDs with PGN encoded in bits 8-25.
 */
function extractPGN(canId) {
  // PGN is in bits 8-25 of the 29-bit identifier
  var pf = (canId >> 16) & 0xFF;  // PDU Format
  var ps = (canId >> 8) & 0xFF;   // PDU Specific
  var dp = (canId >> 24) & 0x01;  // Data Page

  // PDU1 format (PF < 240): PGN = DP + PF + 00
  // PDU2 format (PF >= 240): PGN = DP + PF + PS
  if (pf < 240) {
    return (dp << 16) | (pf << 8);
  } else {
    return (dp << 16) | (pf << 8) | ps;
  }
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
  // Check minimum frame length (CAN ID + length + some data)
  if (frame.length < 6) {
    return parsedValues;
  }

  // Extract CAN ID (29-bit, little-endian in bytes 0-3)
  var canId = frame[0] | (frame[1] << 8) | (frame[2] << 16) | (frame[3] << 24);

  // Extract data length
  var dataLength = frame[4];
  if (dataLength > 8) {
    dataLength = 8;
  }

  // Extract data bytes
  var data = [];
  for (var i = 0; i < dataLength && (5 + i) < frame.length; i++) {
    data.push(frame[5 + i]);
  }

  // Extract PGN from CAN ID
  var pgn = extractPGN(canId);

  // Look up parser for this PGN
  if (pgnToIndexMap.hasOwnProperty(pgn)) {
    var config = pgnToIndexMap[pgn];

    try {
      // Call the custom parser function
      var values = config.parser(data);

      // Store values at mapped indices
      for (var i = 0; i < values.length && i < config.indices.length; i++) {
        var index = config.indices[i];
        parsedValues[index] = values[i];
      }
    }
    catch (e) {
      console.error("NMEA 2000 parsing error:", e.message);
    }
  }

  return parsedValues;
}
