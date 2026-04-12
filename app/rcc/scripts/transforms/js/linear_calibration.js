// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Linear Calibration
// Applies y = slope * value + offset. Commonly used for sensor
// calibration where raw ADC/register values need linear conversion
// to physical units.
// Adjust slope, offset to match your sensor.
//

function transform(value) {
  var slope = 0.01;
  var offset = 273.15;
  return slope * value + offset;
}
