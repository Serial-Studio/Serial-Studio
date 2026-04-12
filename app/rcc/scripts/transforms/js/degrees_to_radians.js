// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Degrees to Radians
// Converts an angle from degrees to radians: rad = deg * pi/180.
//

function transform(value) {
  return value * Math.PI / 180.0;
}
