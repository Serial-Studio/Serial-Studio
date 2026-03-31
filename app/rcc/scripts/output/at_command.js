/**
 * Hayes AT command interface.
 * For modems, GSM/LTE modules (SIM800, Quectel), ESP-AT, HC-05 Bluetooth.
 */
function transmit(value) {
  if (typeof value === "string" && value.length > 0)
    return "AT+" + value + "\r\n";

  return "AT\r\n";
}
