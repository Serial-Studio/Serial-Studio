/**
 * PWM duty cycle (0-255) with channel selection.
 * For motor drivers, LED dimmers, heater SSRs, fan speed controllers.
 * The control's Min and Max hold the range (set Max to 255); value arrives already inside it.
 */
var CHANNEL = 0;

function transmit(value) {
  var duty = Math.round(value);
  return "PWM " + CHANNEL + " " + duty + "\r\n";
}
