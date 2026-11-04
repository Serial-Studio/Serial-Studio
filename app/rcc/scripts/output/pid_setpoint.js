/**
 * PID setpoint with 2-decimal precision.
 * For temperature controllers, pressure regulators, flow control valves.
 * The control's Min and Max hold the range; value arrives already inside it.
 */
function transmit(value) {
  var sp = Number(value);
  return "SP " + sp.toFixed(2) + "\r\n";
}
