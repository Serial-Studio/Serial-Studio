/**
 * SLCAN (Lawicel) CAN bus ASCII command.
 * For USB-CAN adapters: CANable, USBtin, PCAN, Canable Pro, slcand.
 */
var CAN_ID  = 0x100;
var DLC     = 2;

function transmit(value) {
  // TextField: send raw SLCAN command
  if (typeof value === "string")
    return value + "\r";

  // Build standard 11-bit CAN frame: tIIILDD..DD\r
  var id  = ("00" + CAN_ID.toString(16)).slice(-3).toUpperCase();
  var val = Math.round(value) & 0xFFFF;

  // Pack value as big-endian 16-bit into DLC=2 data bytes
  var hi = ("0" + ((val >> 8) & 0xFF).toString(16)).slice(-2).toUpperCase();
  var lo = ("0" + (val & 0xFF).toString(16)).slice(-2).toUpperCase();

  return "t" + id + DLC + hi + lo + "\r";
}
