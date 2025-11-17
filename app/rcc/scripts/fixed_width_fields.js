/**
 * Fixed-Width Fields Parser
 *
 * Parses space-separated fixed-width fields into an array of values.
 *
 * INPUT FORMAT: "John      25  Engineer  ABC123"
 * OUTPUT ARRAY: ["John", "25", "Engineer", "ABC123"]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       Multiple consecutive spaces are treated as field separators.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a fixed-width format frame by splitting on whitespace.
 *
 * How it works:
 * 1. Splits the frame on one or more space characters
 * 2. Filters out any empty strings
 * 3. Returns array of extracted fields
 *
 * Example:
 *   Input:  "John      25  Engineer  ABC123"
 *   Output: ["John", "25", "Engineer", "ABC123"]
 *
 * @param {string} frame - The fixed-width data from the data source
 * @returns {array} Array of string values
 */
function parse(frame) {
  return frame.split(/\s+/).filter(value => value.length > 0);
}
