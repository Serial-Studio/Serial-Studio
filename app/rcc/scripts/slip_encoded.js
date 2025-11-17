/**
 * SLIP-Encoded Frames Parser
 *
 * Parses SLIP (Serial Line Internet Protocol) encoded binary data.
 *
 * SLIP is a framing protocol that encodes packets for transmission over serial
 * lines. It replaces special characters (END and ESC) with escape sequences,
 * allowing reliable framing without requiring specific timing between bytes.
 *
 * INPUT FORMAT: SLIP-encoded binary data
 * OUTPUT ARRAY: Decoded byte values
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters (0xC0 END bytes) are automatically removed by Serial Studio.
 *       This parser decodes the SLIP escape sequences to recover original data.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * SLIP protocol special bytes.
 * These values are defined by RFC 1055.
 */
const SLIP_END = 0xC0;      // Frame delimiter (192 decimal)
const SLIP_ESC = 0xDB;      // Escape character (219 decimal)
const SLIP_ESC_END = 0xDC;  // Escaped END (220 decimal)
const SLIP_ESC_ESC = 0xDD;  // Escaped ESC (221 decimal)

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Decodes a SLIP-encoded frame back to its original byte values.
 *
 * SLIP encoding rules (RFC 1055):
 * - END (0xC0) marks frame boundaries
 * - If data contains 0xC0, it's replaced with: 0xDB 0xDC (ESC ESC_END)
 * - If data contains 0xDB, it's replaced with: 0xDB 0xDD (ESC ESC_ESC)
 *
 * Decoding algorithm:
 * 1. Read each byte from the encoded frame
 * 2. If byte is SLIP_ESC (0xDB), read next byte:
 *    - If next is SLIP_ESC_END (0xDC), output 0xC0
 *    - If next is SLIP_ESC_ESC (0xDD), output 0xDB
 * 3. Otherwise, output the byte as-is
 * 4. Skip any END bytes (0xC0) that may remain
 *
 * Example:
 *   SLIP encoded: [0x01, 0x02, 0xDB, 0xDC, 0x03, 0xDB, 0xDD, 0x04]
 *   Decoded:      [0x01, 0x02, 0xC0, 0x03, 0xDB, 0x04]
 *
 * @param {array} frame - The SLIP-encoded binary data from the data source (byte array)
 * @returns {array} Array of decoded byte values
 */
function parse(frame) {
  var decoded = [];
  var i = 0;

  // Process the SLIP-encoded frame
  while (i < frame.length) {
    var byte = frame[i];
    i++;

    // Skip END bytes (frame delimiters that may remain)
    if (byte === SLIP_END) {
      continue;
    }

    // Handle escape sequences
    if (byte === SLIP_ESC) {
      // Read the next byte to determine what was escaped
      if (i < frame.length) {
        var nextByte = frame[i];
        i++;

        // ESC + ESC_END -> original END byte
        if (nextByte === SLIP_ESC_END) {
          decoded.push(SLIP_END);
        }

        // ESC + ESC_ESC -> original ESC byte
        else if (nextByte === SLIP_ESC_ESC) {
          decoded.push(SLIP_ESC);
        }

        // Invalid escape sequence, push both bytes
        else {
          decoded.push(SLIP_ESC);
          decoded.push(nextByte);
        }
      }

      // ESC at end of frame (incomplete escape sequence)
      // Just push the ESC byte
      else {
        decoded.push(SLIP_ESC);
      }
    }

    // Regular byte, no escaping needed
    else {
      decoded.push(byte);
    }
  }

  return decoded;
}
