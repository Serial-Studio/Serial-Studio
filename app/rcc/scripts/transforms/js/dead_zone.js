// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Dead Zone
// Suppresses small values near zero. If |value| < threshold,
// returns 0. Use to eliminate sensor noise around the zero point.
// Adjust threshold to match your sensor.
//

function transform(value) {
  var threshold = 0.5;

  if (Math.abs(value) < threshold)
    return 0;

  return value;
}
