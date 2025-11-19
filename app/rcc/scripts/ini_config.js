/**
 * INI/Config Format Parser
 *
 * Parses INI-style configuration data into an array of values.
 *
 * INPUT FORMAT: "key1=value1\nkey2=value2" or "temperature=25.5\nhumidity=60"
 * OUTPUT ARRAY: [25.5, 60, ...]  (mapped to predefined indices)
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       INI entries are mapped to array indices based on keyToIndexMap.
 *       Values persist between frames unless updated by new data.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 5;

/**
 * Maps INI key names to array indices.
 *
 * Example: If you receive "temperature=25.5", the value 25.5 will be placed at index 0.
 *
 * Customize this object to match your INI keys and desired output order.
 * Format: "key_name": index_in_output_array
 */
const keyToIndexMap = {
  "temperature": 0,
  "humidity": 1,
  "pressure": 2,
  "battery": 3,
  "signal": 4
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
 * Parses an INI-format frame into the predefined array structure.
 *
 * How it works:
 * 1. Splits the frame into individual lines
 * 2. For each line, splits on '=' to get key and value
 * 3. Looks up the key in keyToIndexMap
 * 4. Stores the value at the mapped index
 *
 * Supported formats:
 *   - "key=value" (standard INI)
 *   - "key = value" (spaces around equals sign)
 *   - Comments starting with ';' or '#' are ignored
 *   - Empty lines are ignored
 *
 * @param {string} frame - The INI-format data from the data source
 * @returns {array} Array of values mapped according to keyToIndexMap
 */
function parse(frame) {
  // Split frame into individual lines
  var lines = frame.split('\n');

  // Process each line
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i].trim();

    // Skip empty lines
    if (line.length === 0) {
      continue;
    }

    // Skip comment lines (starting with ; or #)
    if (line.charAt(0) === ';' || line.charAt(0) === '#') {
      continue;
    }

    // Split on '=' to get key and value
    var parts = line.split('=');
    if (parts.length < 2) {
      continue;  // Invalid line, skip it
    }

    // Extract key and value, trim whitespace
    var key = parts[0].trim();
    var valueStr = parts[1].trim();

    // Try to parse as number, otherwise keep as string
    var value = parseFloat(valueStr);
    if (isNaN(value)) {
      value = valueStr;  // Keep as string if not a number
    }

    // Look up the key and store the value at the mapped index
    if (keyToIndexMap.hasOwnProperty(key)) {
      var index = keyToIndexMap[key];
      parsedValues[index] = value;
    }
  }

  return parsedValues;
}
