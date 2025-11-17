/**
 * Raw Bytes Parser
 *
 * Parses raw binary data into an array of byte values (0-255).
 *
 * INPUT FORMAT: Binary frame data (QByteArray)
 * OUTPUT ARRAY: [0x12, 0x34, 0x56, 0x78, ...]
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a raw binary frame into an array of byte values.
 *
 * @param {array} frame - The raw binary data from the data source (byte array)
 * @returns {array} Array of byte values (0-255)
 */
function parse(frame) {
  let dataArray = [];

  for (let i = 0; i < frame.length; ++i) {
    let byte = frame[i];
    dataArray.push(byte);
  }

  return dataArray;
}
