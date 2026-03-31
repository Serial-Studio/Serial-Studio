/**
 * PID setpoint with range clamping and 2-decimal precision.
 * For temperature controllers, pressure regulators, flow control valves.
 */
var SP_MIN = 0.0;
var SP_MAX = 100.0;

function transmit(value) {
  var sp = Math.max(SP_MIN, Math.min(SP_MAX, Number(value)));
  return "SP " + sp.toFixed(2) + "\r\n";
}
