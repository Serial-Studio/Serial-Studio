/**
 * Relay or digital output toggle with channel support.
 * For relay boards, solenoid valves, contactors, safety interlocks.
 */
var CHANNEL = 0;

function transmit(value) {
  var state = value ? "ON" : "OFF";
  return "RELAY " + CHANNEL + " " + state + "\r\n";
}
