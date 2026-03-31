/**
 * G-code command for CNC machines and 3D printers.
 * For Marlin, Grbl, RepRap, LinuxCNC, laser cutters, engravers.
 */
var FEED_RATE = 600;

function transmit(value) {
  // TextField: send raw G-code line
  if (typeof value === "string")
    return value + "\n";

  // Toggle: spindle on/off
  if (value === 0 || value === 1)
    return value ? "M3 S1000\n" : "M5\n";

  // Slider/Knob: jog X axis (relative move)
  return "G91\nG1 X" + Number(value).toFixed(2) + " F" + FEED_RATE + "\nG90\n";
}
