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
const numItems = 8;

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
 * Decodes URL-encoded string (percent-encoding).
 *
 * Converts:
 *   - %XX to the corresponding character (e.g., %20 -> space)
 *   - '+' to space
 *   - Handles UTF-8 multi-byte sequences
 *
 * Example: "Hello%20World" -> "Hello World"
 */
function decodeURIComponent(str) {
  var result = "";
  var i = 0;

  while (i < str.length) {
    var c = str.charAt(i);

    if (c === '%') {
      // Percent-encoded character
      if (i + 2 < str.length) {
        var hex = str.substring(i + 1, i + 3);
        var charCode = parseInt(hex, 16);
        result += String.fromCharCode(charCode);
        i += 3;
      } else {
        // Invalid encoding, just add the %
        result += c;
        i++;
      }
    }
    else if (c === '+') {
      // '+' is space in URL encoding
      result += ' ';
      i++;
    }
    else {
      // Regular character
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
 * Example:
 *   Input:  "temp=25.5&humidity=60&status=OK"
 *   Output: [25.5, 60, 0, 0, ...]
 *
 * @param {string} frame - The URL-encoded data from the data source
 * @returns {array} Array of values mapped according to keyToIndexMap
 */
function parse(frame) {
  // Trim whitespace
  frame = frame.trim();

  // Skip empty frames
  if (frame.length === 0) {
    return parsedValues;
  }

  // Split into key=value pairs
  var pairs = frame.split('&');

  // Process each pair
  for (var i = 0; i < pairs.length; i++) {
    var pair = pairs[i];

    // Skip empty pairs
    if (pair.length === 0) {
      continue;
    }

    // Split on '=' to get key and value
    var equalPos = pair.indexOf('=');
    if (equalPos === -1) {
      continue; // No '=' found, skip this pair
    }

    // Extract key and value
    var key = pair.substring(0, equalPos);
    var valueStr = pair.substring(equalPos + 1);

    // Decode URL encoding
    key = decodeURIComponent(key);
    valueStr = decodeURIComponent(valueStr);

    // Try to parse as number, otherwise keep as string
    var value = parseFloat(valueStr);
    if (isNaN(value)) {
      value = valueStr; // Keep as string if not a number
    }

    // Look up the key and store the value at the mapped index
    if (keyToIndexMap.hasOwnProperty(key)) {
      var index = keyToIndexMap[key];
      parsedValues[index] = value;
    }
  }

  return parsedValues;
}
