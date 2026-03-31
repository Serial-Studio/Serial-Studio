/**
 * Plain text command with CR+LF terminator.
 * Suitable for serial consoles, Arduino, basic UART peripherals.
 */
function transmit(value) {
  if (typeof value === "string")
    return value + "\r\n";

  if (value === 1)
    return "ON\r\n";

  if (value === 0)
    return "OFF\r\n";

  return "SET " + value + "\r\n";
}
