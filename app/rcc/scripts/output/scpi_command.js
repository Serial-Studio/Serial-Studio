/**
 * SCPI (IEEE 488.2) instrument command interface.
 * For power supplies, DMMs, oscilloscopes, signal generators (Keysight, Rigol, R&S, Siglent).
 */
var CHANNEL = 1;

function transmit(value) {
  // TextField: send raw SCPI command
  if (typeof value === "string")
    return value + "\n";

  // Toggle: enable/disable output channel
  if (value === 0 || value === 1)
    return "OUTP" + CHANNEL + " " + (value ? "ON" : "OFF") + "\n";

  // Slider/Knob: set voltage on channel
  return "SOUR" + CHANNEL + ":VOLT " + Number(value).toFixed(3) + "\n";
}
