/**
 * Grbl $-prefixed and real-time commands.
 * For Grbl, FluidNC, CNC shields, laser engravers, DIY CNC routers.
 */
var JOG_FEED  = 500;
var JOG_AXIS  = "X";

function transmit(value) {
  // TextField: send raw Grbl command ($H, $$, $X, etc.)
  if (typeof value === "string")
    return value + "\n";

  // Toggle: hold / resume (cycle start)
  if (value === 0)
    return "!";
  if (value === 1)
    return "~";

  // Slider/Knob: incremental jog
  return "$J=G91 " + JOG_AXIS + Number(value).toFixed(2) + " F" + JOG_FEED + "\n";
}
