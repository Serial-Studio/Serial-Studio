/**
 * Control Script (setup / loop)
 *
 * setup() runs once when the device connects; loop() runs repeatedly while it
 * stays connected. Both can drive the connection through the SerialStudio SDK
 * (for example io.writeData(...)). Pace loop() with delay(ms), like an Arduino
 * sketch. See the Control Script help page for examples.
 */

/**
 * @brief Runs once, right after the device connects.
 */
function setup() {
}

/**
 * @brief Runs repeatedly while the device stays connected.
 */
function loop() {
  delay(1000);
}
