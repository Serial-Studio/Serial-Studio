/**
 * Batched Sensor Data Parser Template
 *
 * Use this template for devices that send scalar metadata along with
 * batched sensor readings in a single packet.
 *
 * AUTOMATIC MULTI-FRAME EXPANSION
 * ================================
 * When you return a mixed scalar/vector array like:
 *   [scalar1, scalar2, [vec1, vec2, ..., vecN]]
 *
 * Serial Studio automatically generates N frames:
 *   Frame 1: [scalar1, scalar2, vec1]
 *   Frame 2: [scalar1, scalar2, vec2]
 *   ...
 *   Frame N: [scalar1, scalar2, vecN]
 *
 * The scalars are automatically repeated across all frames!
 *
 * COMMON USE CASES
 * ================
 * 1. BLE devices batching high-frequency sensor data
 * 2. Environmental sensors with periodic batch uploads
 * 3. Audio devices sending sample chunks
 * 4. GPS trackers with position history
 * 5. Industrial sensors with hourly batches
 *
 * EXAMPLE INPUT
 * =============
 * {
 *   "device_id": 42,
 *   "battery": 3.7,
 *   "temperature": 25.5,
 *   "samples": [1.23, 4.56, 7.89, 10.11, 12.34]
 * }
 *
 * EXAMPLE OUTPUT
 * ==============
 * [42, 3.7, 25.5, [1.23, 4.56, 7.89, 10.11, 12.34]]
 *
 * This generates 5 frames:
 *   [42, 3.7, 25.5, 1.23]
 *   [42, 3.7, 25.5, 4.56]
 *   [42, 3.7, 25.5, 7.89]
 *   [42, 3.7, 25.5, 10.11]
 *   [42, 3.7, 25.5, 12.34]
 *
 * CUSTOMIZATION GUIDE
 * ===================
 * 1. Parse your JSON/binary/text format
 * 2. Extract scalar values (device ID, battery, etc.)
 * 3. Extract vector arrays (sensor samples)
 * 4. Return array with scalars first, then vectors
 * 5. Serial Studio handles the rest!
 *
 * @param {string} frame - Raw frame data as string or byte array
 * @returns {Array} Mixed scalar/vector array
 */
function parse(frame) {
  // STEP 1: Parse incoming data format
  // Modify this based on your data format (JSON, CSV, binary, etc.)
  var data = JSON.parse(frame);

  // STEP 2: Extract scalar values
  // These will be repeated across all generated frames
  var device_id   = data.device_id   || 0;
  var battery     = data.battery     || 0.0;
  var temperature = data.temperature || 0.0;

  // STEP 3: Extract vector arrays (batched samples)
  // Each element becomes one frame
  var samples = data.samples || [];

  // Optional: Handle multiple sensor arrays
  // var accel_x = data.accel_x || [];
  // var accel_y = data.accel_y || [];
  // var accel_z = data.accel_z || [];

  // STEP 4: Return mixed scalar/vector array
  // Format: [scalar1, scalar2, ..., vector1, vector2, ...]
  return [
    device_id,    // Scalar - repeated in all frames
    battery,      // Scalar - repeated in all frames
    temperature,  // Scalar - repeated in all frames
    samples       // Vector - unzipped element-by-element
  ];

  // For multiple vectors, use:
  // return [device_id, battery, temperature, accel_x, accel_y, accel_z];
  //
  // If vectors have different lengths, Serial Studio automatically
  // extends shorter vectors by repeating their last value!
}

/**
 * EXAMPLE VARIATIONS
 * ==================
 */

// VARIATION 1: CSV format with batched values
// Input: "42,3.7,25.5;1.23,4.56,7.89"
/*
function parse(frame) {
  var parts    = frame.split(';');
  var metadata = parts[0].split(',');
  var samples  = parts[1].split(',').map(parseFloat);

  return [
    parseFloat(metadata[0]),  // device_id
    parseFloat(metadata[1]),  // battery
    parseFloat(metadata[2]),  // temperature
    samples                   // batched samples
  ];
}
*/

// VARIATION 2: Base64-encoded binary data
// Input: Base64 string with header + packed samples
/*
function parse(frame) {
  var bytes = atob(frame).split('').map(function(c) {
    return c.charCodeAt(0);
  });

  var device_id   = bytes[0];
  var battery     = bytes[1] / 100.0;
  var temperature = bytes[2] - 40;

  var samples = [];
  for (var i = 8; i < bytes.length; i += 2) {
    var value = (bytes[i] << 8) | bytes[i + 1];
    samples.push(value / 100.0);
  }

  return [device_id, battery, temperature, samples];
}
*/

// VARIATION 3: Multiple sensor arrays with different lengths
// Input: {"id": 42, "temp": 25.5, "accel": [1,2,3], "gyro": [4,5]}
/*
function parse(frame) {
  var data = JSON.parse(frame);

  return [
    data.id,
    data.temp,
    data.accel,  // 3 elements
    data.gyro    // 2 elements - will be extended to [4,5,5]
  ];

  // This generates 3 frames (length of longest vector):
  //   [42, 25.5, 1, 4]
  //   [42, 25.5, 2, 5]
  //   [42, 25.5, 3, 5]  <- gyro[1] repeated
}
*/
