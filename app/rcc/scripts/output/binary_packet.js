/**
 * Binary packet with STX/ETX framing and XOR checksum.
 * Common in PLCs, industrial sensors, and embedded protocols.
 */
function transmit(value) {
  var STX = String.fromCharCode(0x02);
  var ETX = String.fromCharCode(0x03);
  var cmd = String.fromCharCode(0x01);
  var val = String.fromCharCode(Math.round(value) & 0xFF);

  // XOR checksum over cmd + value
  var chk = String.fromCharCode(0x01 ^ (Math.round(value) & 0xFF));
  return STX + cmd + val + chk + ETX;
}
