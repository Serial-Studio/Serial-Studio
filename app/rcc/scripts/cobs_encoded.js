/**
 * COBS-Encoded Frames Parser
 *
 * Parses COBS-encoded binary data into an array of byte values.
 *
 * COBS (Consistent Overhead Byte Stuffing) is a framing algorithm that removes
 * zero bytes from data streams, allowing zero to be used as a frame delimiter.
 *
 * INPUT FORMAT: COBS-encoded binary data
 * OUTPUT ARRAY: [decoded_byte1, decoded_byte2, decoded_byte3, ...]
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters (0x00) are automatically removed by Serial Studio.
 *       This parser decodes the COBS encoding to recover the original data.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Decodes a COBS-encoded frame back to its original byte values.
 *
 * How COBS encoding works:
 * - Original data may contain zero bytes
 * - COBS replaces zeros with special overhead bytes
 * - This allows 0x00 to be used as a reliable frame delimiter
 *
 * Decoding algorithm:
 * 1. Read the first byte (overhead byte) - tells us how many non-zero bytes follow
 * 2. Copy that many bytes to output
 * 3. If we're not at the end, insert a zero byte
 * 4. Repeat until the entire frame is decoded
 *
 * Example:
 *   COBS encoded: [0x03, 0x11, 0x22, 0x02, 0x33]
 *   Decoded:      [0x11, 0x22, 0x00, 0x33]
 *
 * @param {array} frame - The COBS-encoded binary data from the data source (byte array)
 * @returns {array} Array of decoded byte values
 */
function parse(frame) {
  let decoded = [];
  let i = 0;

  // Process the COBS-encoded frame
  while (i < frame.length) {

    // Read the overhead byte (code byte)
    // This tells us how many non-zero bytes follow before the next zero
    let code = frame[i];
    i++;

    // If code is 0x00, we've reached the end (shouldn't happen if framing is correct)
    if (code === 0) {
      break;
    }

    // Copy (code - 1) bytes to the output
    // We subtract 1 because the code byte itself counts as one of the positions
    for (let j = 1; j < code && i < frame.length; j++) {
      decoded.push(frame[i]);
      i++;
    }

    // If code is less than 0xFF, insert a zero byte
    // (unless we're at the end of the frame)
    if (code < 0xFF && i < frame.length) {
      decoded.push(0x00);
    }
  }

  return decoded;
}
