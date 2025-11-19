/**
 * Binary TLV (Tag-Length-Value) Parser
 *
 * Parses binary TLV-encoded data into an array of values.
 *
 * INPUT FORMAT: Binary data with TLV structure
 *   [Tag1][Length1][Value1][Tag2][Length2][Value2]...
 *
 * OUTPUT ARRAY: [value1, value2, value3, ...]  (mapped to predefined indices)
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 *       Values are mapped to array indices based on tagToIndexMap.
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
 * Maps TLV tag numbers to array indices.
 *
 * Example: If you receive a TLV with tag 0x01, the value will be placed at index 0.
 *
 * TLV Structure:
 *   - Tag: 1 byte identifying the data type
 *   - Length: 1 byte indicating how many bytes the value occupies
 *   - Value: N bytes of actual data
 *
 * Customize this object to match your TLV protocol.
 * Format: tag_number: index_in_output_array
 */
const tagToIndexMap = {
  0x01: 0,    // Tag 0x01 -> index 0 (e.g., temperature)
  0x02: 1,    // Tag 0x02 -> index 1 (e.g., humidity)
  0x03: 2,    // Tag 0x03 -> index 2 (e.g., pressure)
  0x04: 3,    // Tag 0x04 -> index 3 (e.g., battery voltage)
  0x05: 4     // Tag 0x05 -> index 4 (e.g., signal strength)
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
 * Parses a binary TLV frame into the predefined array structure.
 *
 * How it works:
 * 1. Reads Tag byte (identifies what type of data)
 * 2. Reads Length byte (how many bytes in the value)
 * 3. Reads Value bytes (the actual data)
 * 4. Looks up Tag in tagToIndexMap to find where to store the value
 * 5. Converts value bytes to a number and stores at the mapped index
 * 6. Repeats for all TLV entries in the frame
 *
 * @param {array} frame - The binary TLV data from the data source (byte array)
 * @returns {array} Array of values mapped according to tagToIndexMap
 */
function parse(frame) {
  let i = 0;

  // Loop through the frame, parsing each TLV entry
  while (i < frame.length - 1) {  // Need at least Tag and Length bytes

    // Read the Tag byte (identifies the data type)
    let tag = frame[i];
    i++;

    // Read the Length byte (number of bytes in the value)
    let length = frame[i];
    i++;

    // Make sure we have enough bytes left for the value
    if (i + length > frame.length) {
      break;  // Incomplete TLV entry, stop parsing
    }

    // Read the Value bytes
    let valueBytes = [];
    for (let j = 0; j < length; j++) {
      valueBytes.push(frame[i]);
      i++;
    }

    // Convert value bytes to a number
    // For single byte: just use the byte value
    // For multiple bytes: combine them (big-endian)
    let value = 0;
    for (let j = 0; j < valueBytes.length; j++) {
      value = (value << 8) | valueBytes[j];
    }

    // Look up where this tag's value should be stored
    if (tagToIndexMap.hasOwnProperty(tag)) {
      let index = tagToIndexMap[tag];
      parsedValues[index] = value;
    }
  }

  // Return the complete array
  return parsedValues;
}
