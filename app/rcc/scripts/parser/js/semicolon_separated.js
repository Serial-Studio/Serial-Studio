/**
 * Semicolon-Separated Values Parser
 *
 * Parses semicolon-separated data into an array of values.
 *
 * INPUT FORMAT: "a;b;c;d;e;f"
 * OUTPUT ARRAY: ["a", "b", "c", "d", "e", "f"]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a semicolon-separated frame into an array of values.
 *
 * @param {string} frame - The semicolon-separated data from the data source
 * @returns {array} Array of string values
 */
function parse(frame) {
  return frame.split(';');
}
