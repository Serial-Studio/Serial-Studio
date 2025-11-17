/**
 * NMEA 0183 Sentences Parser
 *
 * Parses NMEA 0183 protocol sentences from GPS and marine navigation devices.
 *
 * NMEA 0183 is a standard communication protocol used by GPS receivers, chart
 * plotters, and other marine electronics. Data is transmitted as ASCII sentences.
 *
 * INPUT FORMAT: "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"
 * OUTPUT ARRAY: Extracted values based on sentence type
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       This parser handles common NMEA sentence types.
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
 * Maps NMEA sentence types to parsing configurations.
 *
 * Common NMEA sentence types:
 *   GPGGA: Global Positioning System Fix Data
 *   GPRMC: Recommended Minimum Specific GPS/Transit Data
 *   GPGLL: Geographic Position - Latitude/Longitude
 *   GPGSA: GPS DOP and Active Satellites
 *   GPGSV: GPS Satellites in View
 *   GPVTG: Track Made Good and Ground Speed
 *
 * Each entry defines which indices to populate with parsed values.
 */
const sentenceTypeMap = {
  "GPGGA": {
    indices: [0, 1, 2, 3, 4, 5],
    parser: function(fields) {
      // $GPGGA,time,lat,N/S,lon,E/W,quality,numSV,HDOP,alt,M,sep,M,diffAge,diffStation*cs
      var latitude = parseLatitude(fields[2], fields[3]);
      var longitude = parseLongitude(fields[4], fields[5]);
      var quality = parseInt(fields[6]) || 0;
      var numSatellites = parseInt(fields[7]) || 0;
      var altitude = parseFloat(fields[9]) || 0;
      var hdop = parseFloat(fields[8]) || 0;
      return [latitude, longitude, altitude, quality, numSatellites, hdop];
    }
  },
  "GPRMC": {
    indices: [0, 1, 6, 7],
    parser: function(fields) {
      // $GPRMC,time,status,lat,N/S,lon,E/W,speed,track,date,magvar,E/W*cs
      var latitude = parseLatitude(fields[3], fields[4]);
      var longitude = parseLongitude(fields[5], fields[6]);
      var speed = parseFloat(fields[7]) || 0;  // Speed in knots
      var track = parseFloat(fields[8]) || 0;  // Track angle in degrees
      return [latitude, longitude, speed, track];
    }
  },
  "GPGLL": {
    indices: [0, 1],
    parser: function(fields) {
      // $GPGLL,lat,N/S,lon,E/W,time,status*cs
      var latitude = parseLatitude(fields[1], fields[2]);
      var longitude = parseLongitude(fields[3], fields[4]);
      return [latitude, longitude];
    }
  },
  "GPVTG": {
    indices: [6, 7],
    parser: function(fields) {
      // $GPVTG,track,T,track,M,speed,N,speed,K,mode*cs
      var speedKnots = parseFloat(fields[5]) || 0;
      var speedKmh = parseFloat(fields[7]) || 0;
      return [speedKnots, speedKmh];
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
 * Parses NMEA latitude format to decimal degrees.
 * Format: DDMM.MMMM (degrees and minutes)
 * Example: "4807.038" with "N" -> 48.1173 degrees
 */
function parseLatitude(latStr, hemisphere) {
  if (!latStr || latStr.length < 4) {
    return 0;
  }

  var degrees = parseFloat(latStr.substring(0, 2));
  var minutes = parseFloat(latStr.substring(2));
  var decimal = degrees + (minutes / 60);

  // South is negative
  if (hemisphere === 'S') {
    decimal = -decimal;
  }

  return decimal;
}

/**
 * Parses NMEA longitude format to decimal degrees.
 * Format: DDDMM.MMMM (degrees and minutes)
 * Example: "01131.000" with "E" -> 11.5167 degrees
 */
function parseLongitude(lonStr, hemisphere) {
  if (!lonStr || lonStr.length < 5) {
    return 0;
  }

  var degrees = parseFloat(lonStr.substring(0, 3));
  var minutes = parseFloat(lonStr.substring(3));
  var decimal = degrees + (minutes / 60);

  // West is negative
  if (hemisphere === 'W') {
    decimal = -decimal;
  }

  return decimal;
}

/**
 * Validates NMEA checksum.
 * Checksum is XOR of all characters between $ and *.
 */
function validateChecksum(sentence) {
  var starPos = sentence.indexOf('*');
  if (starPos === -1) {
    return false;  // No checksum present
  }

  var data = sentence.substring(1, starPos);  // Between $ and *
  var checksumStr = sentence.substring(starPos + 1);
  var expectedChecksum = parseInt(checksumStr, 16);

  var calculatedChecksum = 0;
  for (var i = 0; i < data.length; i++) {
    calculatedChecksum ^= data.charCodeAt(i);
  }

  return calculatedChecksum === expectedChecksum;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses an NMEA 0183 sentence into the predefined array structure.
 *
 * NMEA sentence structure:
 *   $AACCC,field1,field2,...,fieldN*CS
 *
 *   $      - Start delimiter
 *   AA     - Talker ID (GP=GPS, GL=GLONASS, etc.)
 *   CCC    - Sentence formatter (GGA, RMC, etc.)
 *   fields - Comma-separated data fields
 *   *CS    - Checksum (XOR of all bytes between $ and *)
 *
 * How it works:
 * 1. Validates the sentence format (starts with $)
 * 2. Validates the checksum
 * 3. Extracts the sentence type
 * 4. Splits the sentence into fields
 * 5. Calls the appropriate parser for the sentence type
 * 6. Stores values at their mapped indices
 *
 * @param {string} frame - The NMEA sentence from the data source
 * @returns {array} Array of values mapped according to sentenceTypeMap
 */
function parse(frame) {
  // Trim whitespace
  frame = frame.trim();

  // Check if sentence starts with $
  if (frame.charAt(0) !== '$') {
    return parsedValues;
  }

  // Validate checksum if present
  if (frame.indexOf('*') !== -1) {
    if (!validateChecksum(frame)) {
      console.error("NMEA checksum validation failed");
      return parsedValues;
    }
  }

  // Remove checksum for parsing
  var starPos = frame.indexOf('*');
  var dataStr = (starPos !== -1) ? frame.substring(0, starPos) : frame;

  // Split into fields
  var fields = dataStr.split(',');

  // Extract sentence type (e.g., "GPGGA" from "$GPGGA,...")
  var sentenceId = fields[0].substring(1);  // Remove $

  // Look up parser for this sentence type
  if (sentenceTypeMap.hasOwnProperty(sentenceId)) {
    var config = sentenceTypeMap[sentenceId];

    try {
      // Call the custom parser function
      var values = config.parser(fields);

      // Store values at mapped indices
      for (var i = 0; i < values.length && i < config.indices.length; i++) {
        var index = config.indices[i];
        parsedValues[index] = values[i];
      }
    }
    catch (e) {
      console.error("NMEA parsing error:", e.message);
    }
  }

  return parsedValues;
}
