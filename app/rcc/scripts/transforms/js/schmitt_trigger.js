// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Schmitt Trigger (hysteresis comparator)
// Outputs 1 when the input rises above highThreshold, and 0 when
// it falls below lowThreshold. Values in between keep the previous
// state. This prevents rapid 0/1 toggling when a noisy analog
// signal hovers near a single threshold.
//

var highThreshold = 7;
var lowThreshold = 3;
var state = 0;

function transform(value) {
  if (state === 0 && value > highThreshold)
    state = 1;
  else if (state === 1 && value < lowThreshold)
    state = 0;

  return state;
}
