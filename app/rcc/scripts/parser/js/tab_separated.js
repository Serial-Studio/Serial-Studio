/**
 * Tab-Separated Values (TSV) Parser
 *
 * Parses tab-separated data into an array of values.
 *
 * INPUT FORMAT: "a\tb\tc\td\te\tf"
 * OUTPUT ARRAY: ["a", "b", "c", "d", "e", "f"]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a TSV frame into an array of values.
 *
 * @param {string} frame - The TSV data from the data source
 * @returns {array} Array of string values
 */
function parse(frame) {
  return frame.split('\t');
}
