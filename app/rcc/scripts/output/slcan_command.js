/**
 * SLCAN (Serial CAN) ASCII command.
 * Produces ASCII-framed CAN messages for SLCAN adapters.
 *
 * SLCAN format: tIIILDD..DD\r  (standard frame)
 *   t    = transmit standard frame
 *   III  = 3-hex-digit CAN ID (000-7FF)
 *   L    = data length (0-8)
 *   DD   = data bytes as hex pairs
 *
 * Extended frame: TIIIIIIIILDD..DD\r (8-hex-digit ID)
 */
var CAN_ID = 0x100;

function transmit(value) {
  // Build standard SLCAN frame with 2-byte big-endian value
  var id  = ("000" + CAN_ID.toString(16).toUpperCase()).slice(-3);
  var hi  = ("00" + ((value >> 8) & 0xFF).toString(16).toUpperCase()).slice(-2);
  var lo  = ("00" + (value & 0xFF).toString(16).toUpperCase()).slice(-2);
  return "t" + id + "2" + hi + lo + "\r";
}
