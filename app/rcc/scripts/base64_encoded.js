/**
 * Base64-Encoded Data Parser
 *
 * Parses Base64-encoded data into an array of byte values (0-255).
 *
 * INPUT FORMAT: "SGVsbG8gV29ybGQ=" (Base64 string)
 * OUTPUT ARRAY: [72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100]
 *
 * Note: This parser requires Base64 decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 */

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------

/**
 * Decodes a Base64-encoded string into a binary string.
 *
 * @param {string} input - The Base64-encoded string
 * @returns {string} The decoded binary string
 * @throws {Error} If the input contains invalid Base64 characters
 */
function atob(input) {
  input = input.replace(/[\s=]+$/, '');
  const base64Chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
  let output = '';
  let buffer = 0, bits = 0;

  for (let i = 0; i < input.length; i++) {
    let value = base64Chars.indexOf(input[i]);
    if (value === -1) {
      throw new Error('Invalid character in Base64 string');
    }
    buffer = (buffer << 6) | value;
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      output += String.fromCharCode((buffer >> bits) & 0xFF);
    }
  }

  return output;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a Base64-encoded frame into an array of byte values.
 *
 * @param {string} frame - The Base64-encoded data from the data source
 * @returns {array} Array of byte values (0-255), or empty array on error
 */
function parse(frame) {
  try {
    let binaryString = atob(frame);
    let bytes = Array.from(binaryString).map(char => char.charCodeAt(0));
    return bytes;
  }
  catch (e) {
    console.error("Base64 decode error:", e.message);
    return [];
  }
}
