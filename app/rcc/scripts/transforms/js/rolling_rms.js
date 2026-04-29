// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Rolling RMS (root mean square)
// Computes the RMS value over the last N samples. Fundamental for
// AC signals, audio levels, vibration amplitude, and power
// measurements where the average of a zero-mean signal is zero but
// the RMS is meaningful.
// Adjust N to match your sensor.
//
// Implementation note: the running sum is maintained in O(1) per
// sample by subtracting the value leaving the window and adding the
// new one. This keeps the transform cheap even at audio rates where
// N may be in the tens of thousands.
//

var N = 32;
var history = new Array(N).fill(0);
var idx = 0;
var count = 0;
var sumSq = 0;

function transform(value) {
  var sq = value * value;
  sumSq = sumSq - history[idx] + sq;
  history[idx] = sq;
  idx = (idx + 1) % N;
  if (count < N) count++;

  // Guard against tiny negative drift from float cancellation
  if (sumSq < 0) sumSq = 0;

  return Math.sqrt(sumSq / count);
}
