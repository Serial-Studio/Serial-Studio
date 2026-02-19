/**
 * Hexadecimal Bytes Parser
 *
 * Parses hexadecimal-encoded data into an array of byte values (0-255).
 *
 * INPUT FORMAT: "1A2B3C4D5E6F" or "1A 2B 3C 4D 5E 6F"
 * OUTPUT ARRAY: [26, 43, 60, 77, 94, 111]
 *
 * Note: This parser requires Hexadecimal decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 *       Spaces between hex pairs are automatically handled.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a hexadecimal string into an array of byte values.
 *
 * @param {string} frame - The hexadecimal data from the data source
 * @returns {array} Array of byte values (0-255)
 */
function parse(frame) {
  let dataArray = [];
  let hexString = frame.replace(/\s+/g, '');

  for (let i = 0; i < hexString.length; i += 2)
    dataArray.push(parseInt(hexString.substring(i, i + 2), 16));

  return dataArray;
}
