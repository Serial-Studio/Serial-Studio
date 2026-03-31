/**
 * NMEA 0183 sentence with XOR checksum.
 * For GPS/GNSS configuration (u-blox, SiRF), marine autopilots, AIS transponders.
 */
var TALKER = "GP";

function transmit(value) {
  var body;

  // TextField: raw sentence body (without $ and checksum)
  if (typeof value === "string")
    body = value;
  else
    body = TALKER + "CMD," + value;

  // Compute XOR checksum over the sentence body
  var chk = 0;
  for (var i = 0; i < body.length; i++)
    chk ^= body.charCodeAt(i);

  var hex = ("0" + chk.toString(16)).slice(-2).toUpperCase();
  return "$" + body + "*" + hex + "\r\n";
}
