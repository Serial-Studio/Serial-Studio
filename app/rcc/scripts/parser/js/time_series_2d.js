/**
 * Time-Series 2D Array Parser Template
 *
 * Use this template for devices that send multiple complete data records
 * in a single packet, where each record is a separate frame.
 *
 * AUTOMATIC MULTI-FRAME EXPANSION
 * ================================
 * When you return a 2D array like:
 *   [[val1, val2, val3], [val4, val5, val6], [val7, val8, val9]]
 *
 * Serial Studio automatically generates one frame per row:
 *   Frame 1: [val1, val2, val3]
 *   Frame 2: [val4, val5, val6]
 *   Frame 3: [val7, val8, val9]
 *
 * COMMON USE CASES
 * ================
 * 1. Data loggers uploading historical records
 * 2. Industrial systems with buffered measurements
 * 3. Network devices sending burst transmissions
 * 4. Batch processing results
 * 5. Replay/playback systems
 *
 * EXAMPLE INPUT (JSON)
 * ====================
 * {
 *   "records": [
 *     {"timestamp": 1000, "temp": 25.5, "humidity": 60.0},
 *     {"timestamp": 2000, "temp": 25.7, "humidity": 59.8},
 *     {"timestamp": 3000, "temp": 25.9, "humidity": 59.5}
 *   ]
 * }
 *
 * EXAMPLE OUTPUT
 * ==============
 * [[1000, 25.5, 60.0], [2000, 25.7, 59.8], [3000, 25.9, 59.5]]
 *
 * This generates 3 separate frames for visualization.
 *
 * CUSTOMIZATION GUIDE
 * ===================
 * 1. Parse your data format (JSON, CSV, binary, etc.)
 * 2. Extract array of records
 * 3. Convert each record to a 1D array of values
 * 4. Return as 2D array (array of arrays)
 * 5. Serial Studio processes each row as a frame!
 *
 * @param {string} frame - Raw frame data as string or byte array
 * @returns {Array<Array>} 2D array where each row is one frame
 */
function parse(frame) {
  // STEP 1: Parse incoming data format
  var data = JSON.parse(frame);

  // STEP 2: Extract array of records
  var records = data.records || [];

  // STEP 3: Convert each record to array of values
  var result = [];
  for (var i = 0; i < records.length; i++) {
    var record = records[i];
    result.push([
      record.timestamp || 0,
      record.temp      || 0.0,
      record.humidity  || 0.0
    ]);
  }

  // STEP 4: Return 2D array
  return result;
}

/**
 * EXAMPLE VARIATIONS
 * ==================
 */

// VARIATION 1: CSV format with multiple rows
// Input: "1000,25.5,60.0\n2000,25.7,59.8\n3000,25.9,59.5"
/*
function parse(frame) {
  var lines  = frame.trim().split('\n');
  var result = [];

  for (var i = 0; i < lines.length; i++) {
    var values = lines[i].split(',').map(parseFloat);
    result.push(values);
  }

  return result;
}
*/

// VARIATION 2: Fixed-width binary records
// Input: Binary data with 3-byte records (1 byte + 2 shorts)
/*
function parse(frame) {
  var result     = [];
  var recordSize = 5;

  for (var i = 0; i < frame.length; i += recordSize) {
    if (i + recordSize > frame.length) break;

    var id       = frame[i];
    var temp     = ((frame[i + 1] << 8) | frame[i + 2]) / 10.0;
    var humidity = ((frame[i + 3] << 8) | frame[i + 4]) / 10.0;

    result.push([id, temp, humidity]);
  }

  return result;
}
*/

// VARIATION 3: Array of arrays directly in JSON
// Input: {"data": [[1,2,3], [4,5,6], [7,8,9]]}
/*
function parse(frame) {
  var data = JSON.parse(frame);
  return data.data || [];
}
*/

// VARIATION 4: Semicolon-separated rows, comma-separated values
// Input: "1,25.5,60.0;2,25.7,59.8;3,25.9,59.5"
/*
function parse(frame) {
  var rows   = frame.split(';');
  var result = [];

  for (var i = 0; i < rows.length; i++) {
    var values = rows[i].split(',').map(parseFloat);
    result.push(values);
  }

  return result;
}
*/

// VARIATION 5: Historical data with timestamps
/*
function parse(frame) {
  var data    = JSON.parse(frame);
  var history = data.history || [];
  var result  = [];

  for (var i = 0; i < history.length; i++) {
    var entry = history[i];
    result.push([entry.time, entry.sensor1, entry.sensor2, entry.sensor3]);
  }

  return result;
}
*/

// VARIATION 6: GPS track with multiple position samples
/*
function parse(frame) {
  var data   = JSON.parse(frame);
  var fixes  = data.gps_track || [];
  var result = [];

  for (var i = 0; i < fixes.length; i++) {
    var fix = fixes[i];
    result.push([fix.timestamp, fix.latitude, fix.longitude,
                 fix.altitude, fix.speed, fix.satellites]);
  }

  return result;
}
*/

// VARIATION 7: Industrial PLC batch data
/*
function parse(frame) {
  var data     = JSON.parse(frame);
  var readings = data.batch || [];
  var result   = [];

  for (var i = 0; i < readings.length; i++) {
    var r = readings[i];
    result.push([r.sequence_number, r.pressure, r.temperature,
                 r.flow_rate, r.quality_flag]);
  }

  return result;
}
*/
