// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Moving Average
// Smooths noisy signals by averaging the last N samples. Uses a
// circular buffer for memory efficiency. Introduces N/2 frames of lag.
// Adjust N to match your sensor.
//
// Implementation note: the running sum is maintained in O(1) per
// sample by subtracting the value leaving the window and adding the
// new one. This keeps the transform cheap even at audio rates where
// N may be in the tens of thousands.
//

var N = 10;
var history = new Array(N).fill(0);
var idx = 0;
var count = 0;
var sum = 0;

function transform(value) {
  sum = sum - history[idx] + value;
  history[idx] = value;
  idx = (idx + 1) % N;
  if (count < N) count++;

  return sum / count;
}
