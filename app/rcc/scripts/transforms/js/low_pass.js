// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Low-Pass Filter
// First-order IIR low-pass filter. Alpha = dt/(RC + dt). Smaller
// alpha = lower cutoff frequency = more smoothing. Good for removing
// high-frequency noise.
// Adjust alpha to match your sensor.
//

var alpha = 0.05;
var filtered;

function transform(value) {
  if (filtered === undefined)
    filtered = value;

  filtered = filtered + alpha * (value - filtered);
  return filtered;
}
