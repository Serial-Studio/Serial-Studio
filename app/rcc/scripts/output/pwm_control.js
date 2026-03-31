/**
 * PWM duty cycle (0-255) with channel selection.
 * For motor drivers, LED dimmers, heater SSRs, fan speed controllers.
 */
var CHANNEL = 0;

function transmit(value) {
  var duty = Math.round(Math.max(0, Math.min(255, value)));
  return "PWM " + CHANNEL + " " + duty + "\r\n";
}
