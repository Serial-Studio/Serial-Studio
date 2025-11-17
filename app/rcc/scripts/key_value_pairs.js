/**
 * Key-Value Pairs Parser
 *
 * Parses key-value formatted data into an array of values using a custom keymap.
 *
 * INPUT FORMAT: "temperature=22.5,humidity=60.3,pressure=1013"
 * OUTPUT ARRAY: [22.5, 60.3, 1013]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       Values are mapped to array indices based on the keyToIndexMap.
 *       Missing keys retain their previous values between frames.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Maps key names to array indices.
 * Customize this object to match your data format and desired output order.
 */
const keyToIndexMap = {
  temperature: 0,
  humidity: 1,
  pressure: 2
};

/**
 * Array to hold parsed values with default initialization.
 * Values persist between frames unless updated by incoming data.
 */
const parsedValues = new Array(Object.keys(keyToIndexMap).length).fill(0);

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a key-value frame into an array of values.
 *
 * @param {string} frame - The key-value data from the data source
 * @returns {array} Array of values mapped according to keyToIndexMap
 */
function parse(frame) {
  // Regular expression to match "key=value" pairs
  const regex = /([\w]+)=([\d.]+)/g;
  let match;

  // Iterate over all matches in the frame
  while ((match = regex.exec(frame)) !== null) {
    const key = match[1];                    // Extract the key (e.g., "temperature")
    const value = parseFloat(match[2]);      // Extract the numeric value (e.g., 22.5)

    // Check if the key exists in the map
    if (keyToIndexMap.hasOwnProperty(key)) {
      const index = keyToIndexMap[key];      // Get the mapped index
      parsedValues[index] = value;           // Update the corresponding value
    }
  }

  return parsedValues;
}
