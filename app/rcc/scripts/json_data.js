/**
 * JSON Data Parser
 *
 * Parses JSON-formatted data into an array of values.
 *
 * INPUT FORMAT: {"time":"12:05","speed":48,"angle":4.0,"distance":87}
 * OUTPUT ARRAY: ["12:05", 48, 4.0, 87]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Fields to extract from the JSON object.
 * Customize this array to match your JSON structure and desired output order.
 */
const fields = ["time", "speed", "angle", "distance"];

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a JSON frame into an array of values.
 *
 * @param {string} frame - The JSON data from the data source
 * @returns {array} Array of values extracted from the JSON object
 */
function parse(frame) {
  if (frame.length === 0) {
    return [];
  }

  try {
    let data = JSON.parse(frame);
    return fields.map(field => data[field]);
  }

  catch (e) {
    console.error("JSON parse error:", e.message);
    return [];
  }
}
