// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Running Minimum
// Returns the smallest value seen since the transform started.
// Useful for tracking low peaks (minimum temperature, minimum
// voltage, etc). The value only decreases — it never resets.
//

var minValue;

function transform(value) {
  if (minValue === undefined || value < minValue)
    minValue = value;

  return minValue;
}
