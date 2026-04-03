/**
 * CAN bus binary frame using built-in protocol helpers.
 * Returns binary payload: [id_hi, id_lo, dlc, data...].
 *
 * Available helpers:
 *   canSendValue(id, value, bytes) - numeric value, big-endian (default 2 bytes)
 *   canSendFrame(id, payload)      - arbitrary payload (string or byte array)
 *
 * Note: These produce raw binary CAN frames.
 * For SLCAN ASCII framing (tIIILDD..DD\r), frame manually.
 */
var CAN_ID = 0x100;

function transmit(value) {
  // TextField: send raw string
  if (typeof value === "string")
    return value + "\r";

  // Send numeric value as 2-byte big-endian CAN frame
  return canSendValue(CAN_ID, value, 2);

  // Alternative: send arbitrary byte payload
  // return canSendFrame(CAN_ID, [0x01, value & 0xFF]);
}
