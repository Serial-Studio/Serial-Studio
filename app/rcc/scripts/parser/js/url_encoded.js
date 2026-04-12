/**
 * URL-Encoded Data Parser
 *
 * Parses URL-encoded (percent-encoded) data into an array of values.
 *
 * URL encoding (also called percent-encoding) is used to encode special characters
 * in URLs and form data. Characters are represented as %XX where XX is the hexadecimal
 * ASCII value. Spaces can be encoded as '+' or '%20'.
 *
 * INPUT FORMAT: "key1=value1&key2=value2" or "temp=25.5&humidity=60&pressure=1013"
 * OUTPUT ARRAY: [value1, value2, ...]  (mapped to predefined indices)
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       URL-encoded characters (like %20 for space) are automatically decoded.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 6;

/**
 * Maps URL parameter names to array indices.
 *
 * Example: If you receive "temperature=25.5&humidity=60",
 * the value 25.5 will be placed at index 0 and 60 at index 1.
 *
 * Customize this object to match your parameter names and desired output order.
 * Format: "parameter_name": index_in_output_array
 */
const keyToIndexMap = {
  "temperature": 0,
  "humidity": 1,
  "pressure": 2,
  "voltage": 3,
  "current": 4,
  "power": 5
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
 * Decodes a URL-encoded (percent-encoded) string.
 *
 * Converts:
 *   - %XX to the corresponding character (e.g., %20 → space)
 *   - '+' to space
 *
 * Example: "Hello%20World" → "Hello World"
 */
function decodeURIComponent(str) {
  var result = "";
  var i = 0;

  while (i < str.length) {
    var c = str.charAt(i);

    if (c === '%') {
      if (i + 2 < str.length) {
        result += String.fromCharCode(parseInt(str.substring(i + 1, i + 3), 16));
        i += 3;
      } else {
        // Truncated percent sequence; emit as-is
        result += c;
        i++;
      }
    }
    else if (c === '+') {
      result += ' ';
      i++;
    }
    else {
      result += c;
      i++;
    }
  }

  return result;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a URL-encoded frame into the predefined array structure.
 *
 * URL-encoded format:
 *   key1=value1&key2=value2&key3=value3...
 *
 * Special characters are encoded as:
 *   - Space: '+' or '%20'
 *   - Equals: '%3D'
 *   - Ampersand: '%26'
 *   - Other special chars: '%XX' (hex ASCII value)
 *
 * How it works:
 * 1. Splits the frame on '&' to get individual key=value pairs
 * 2. For each pair, splits on '=' to separate key and value
 * 3. Decodes percent-encoded characters
 * 4. Looks up the key in keyToIndexMap
 * 5. Stores the value at the mapped index
 *
 * @param {string} frame - The URL-encoded data from the data source
 * @returns {array} Array of values mapped according to keyToIndexMap
 */
function parse(frame) {
  frame = frame.trim();

  if (frame.length === 0)
    return parsedValues;

  var pairs = frame.split('&');

  for (var i = 0; i < pairs.length; i++) {
    var pair = pairs[i];

    if (pair.length === 0)
      continue;

    var equalPos = pair.indexOf('=');
    if (equalPos === -1)
      continue;

    var key      = decodeURIComponent(pair.substring(0, equalPos));
    var valueStr = decodeURIComponent(pair.substring(equalPos + 1));

    var value = parseFloat(valueStr);
    if (isNaN(value))
      value = valueStr;

    if (keyToIndexMap.hasOwnProperty(key))
      parsedValues[keyToIndexMap[key]] = value;
  }

  return parsedValues;
}
