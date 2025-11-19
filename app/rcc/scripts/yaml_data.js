/**
 * YAML Data Parser
 *
 * Parses YAML-formatted data into an array of values.
 *
 * YAML (YAML Ain't Markup Language) is a human-readable data serialization
 * format commonly used for configuration files and data exchange. This parser
 * handles basic YAML structures with key-value pairs.
 *
 * INPUT FORMAT: "temperature: 25.5\nhumidity: 60\npressure: 1013"
 * OUTPUT ARRAY: [25.5, 60, 1013, ...]  (mapped to predefined indices)
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       This is a simplified YAML parser for basic key-value structures.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 7;

/**
 * Maps YAML key names to array indices.
 *
 * Example: If YAML contains "temperature: 25.5",
 * the value 25.5 will be placed at index 0.
 *
 * Customize this object to match your YAML keys and desired output order.
 * Format: "key_name": index_in_output_array
 */
const keyToIndexMap = {
  "temperature": 0,
  "humidity": 1,
  "pressure": 2,
  "voltage": 3,
  "current": 4,
  "altitude": 5,
  "speed": 6
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
 * Parses a YAML value string and converts to appropriate type.
 * Handles: numbers, booleans, null, strings
 */
function parseYAMLValue(valueStr) {
  // Trim whitespace
  valueStr = valueStr.trim();

  // Handle empty/null values
  if (valueStr === '' || valueStr === 'null' || valueStr === '~') {
    return null;
  }

  // Handle booleans
  if (valueStr === 'true' || valueStr === 'yes' || valueStr === 'on') {
    return true;
  }
  if (valueStr === 'false' || valueStr === 'no' || valueStr === 'off') {
    return false;
  }

  // Handle quoted strings (remove quotes)
  if ((valueStr.charAt(0) === '"' && valueStr.charAt(valueStr.length - 1) === '"') ||
      (valueStr.charAt(0) === "'" && valueStr.charAt(valueStr.length - 1) === "'")) {
    return valueStr.substring(1, valueStr.length - 1);
  }

  // Try to parse as number
  var numValue = parseFloat(valueStr);
  if (!isNaN(numValue)) {
    return numValue;
  }

  // Return as string
  return valueStr;
}

/**
 * Removes YAML comments (everything after '#').
 */
function removeComment(line) {
  var commentPos = line.indexOf('#');
  if (commentPos !== -1) {
    return line.substring(0, commentPos);
  }
  return line;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a YAML frame into the predefined array structure.
 *
 * Supported YAML formats:
 *   - Simple key-value: "key: value"
 *   - Strings: "key: 'value'" or 'key: "value"'
 *   - Numbers: "key: 123" or "key: 45.67"
 *   - Booleans: "key: true" or "key: false"
 *   - Comments: Lines starting with '#' or inline with '#'
 *   - Null values: "key: null" or "key: ~"
 *
 * Limitations:
 *   - Does not support nested structures (dictionaries/arrays)
 *   - Does not support multi-line values
 *   - Does not support anchors/aliases
 *   - Only parses top-level key-value pairs
 *
 * How it works:
 * 1. Splits the frame into individual lines
 * 2. For each line, removes comments
 * 3. Splits on ':' to get key and value
 * 4. Parses the value (number, boolean, string, etc.)
 * 5. Looks up the key in keyToIndexMap
 * 6. Stores the value at the mapped index
 *
 * Example:
 *   Input:  "temperature: 25.5\nhumidity: 60\nstatus: online"
 *   Output: [25.5, 60, 0, 0, ...]
 *
 * @param {string} frame - The YAML data from the data source
 * @returns {array} Array of values mapped according to keyToIndexMap
 */
function parse(frame) {
  // Trim whitespace
  frame = frame.trim();

  // Skip empty frames
  if (frame.length === 0) {
    return parsedValues;
  }

  // Split into lines
  var lines = frame.split('\n');

  // Process each line
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];

    // Remove comments
    line = removeComment(line);

    // Trim whitespace
    line = line.trim();

    // Skip empty lines
    if (line.length === 0) {
      continue;
    }

    // Skip YAML document separators
    if (line === '---' || line === '...') {
      continue;
    }

    // Split on ':' to get key and value
    var colonPos = line.indexOf(':');
    if (colonPos === -1) {
      continue; // No ':' found, skip this line
    }

    // Extract key and value
    var key = line.substring(0, colonPos).trim();
    var valueStr = line.substring(colonPos + 1).trim();

    // Skip if key is empty
    if (key.length === 0) {
      continue;
    }

    // Remove quotes from key if present
    if ((key.charAt(0) === '"' && key.charAt(key.length - 1) === '"') ||
        (key.charAt(0) === "'" && key.charAt(key.length - 1) === "'")) {
      key = key.substring(1, key.length - 1);
    }

    // Parse the value
    var value = parseYAMLValue(valueStr);

    // Look up the key and store the value at the mapped index
    if (keyToIndexMap.hasOwnProperty(key)) {
      var index = keyToIndexMap[key];

      // If value is null, keep previous value (or use 0)
      if (value !== null) {
        parsedValues[index] = value;
      }
    }
  }

  return parsedValues;
}
