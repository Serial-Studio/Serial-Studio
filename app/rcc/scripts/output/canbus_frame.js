/**
 * CAN Bus frame using built-in protocol helpers.
 * Returns binary payload: [id_hi, id_lo, dlc, data...].
 *
 * Available helpers:
 *   canSendValue(id, value, bytes) - numeric value, big-endian (default 2 bytes)
 *   canSendFrame(id, payload)      - arbitrary payload (string or byte array)
 *
 * Note: These produce raw binary CAN frames suitable for SocketCAN,
 * PCAN, and other native CAN interfaces.
 */
var CAN_ID = 0x100;

function transmit(value) {
  // Send numeric value as 2-byte big-endian CAN frame
  return canSendValue(CAN_ID, value, 2);

  // Alternative: send arbitrary byte payload
  // return canSendFrame(CAN_ID, [0x01, value & 0xFF]);
}
