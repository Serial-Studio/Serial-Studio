/**
 * Comma-Separated Values (CSV) Parser
 *
 * Parses comma-separated data into an array of values.
 *
 * INPUT FORMAT: "a,b,c,d,e,f"
 * OUTPUT ARRAY: ["a", "b", "c", "d", "e", "f"]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a CSV frame into an array of values.
 *
 * @param {string} frame - The CSV data from the data source
 * @returns {array} Array of string values
 */
function parse(frame) {
  return frame.split(',');
}
