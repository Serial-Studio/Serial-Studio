/**
 * Default transmit function for output widgets.
 *
 * @param value - Button: 1, Slider/Knob: number, Toggle: 0|1, TextField: string
 * @return String or byte array to send over the active connection.
 */
function transmit(value) {
  return "CMD " + value + "\r\n";
}
