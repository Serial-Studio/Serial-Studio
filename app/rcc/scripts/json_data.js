/**
 * JSON Data Parser
 *
 * Parses JSON-formatted data into an array of values.
 *
 * INPUT FORMAT: {"time":"12:05","speed":48,"angle":4.0,"distance":87}
 * OUTPUT ARRAY: ["12:05", 48, 4.0, 87]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       Values persist between frames unless updated by new data.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 4;

/**
 * Maps JSON field names to array indices.
 * Customize this object to match your JSON structure and desired output order.
 */
const fieldToIndexMap = {
  "time": 0,
  "speed": 1,
  "angle": 2,
  "distance": 3
};

/**
 * Array to hold all parsed values.
 * Values persist between frames unless updated by new data.
 */
const parsedValues = new Array(numItems).fill(0);

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a JSON frame into an array of values.
 *
 * @param {string} frame - The JSON data from the data source
 * @returns {array} Array of values mapped according to fieldToIndexMap
 */
function parse(frame) {
  try {
    let data = JSON.parse(frame);
    for (let field in fieldToIndexMap) {
      if (fieldToIndexMap.hasOwnProperty(field) && data.hasOwnProperty(field))
        parsedValues[fieldToIndexMap[field]] = data[field];
    }

    return parsedValues;
  }
  catch (e) {
    console.error("JSON parse error:", e.message);
    return parsedValues;
  }
}
