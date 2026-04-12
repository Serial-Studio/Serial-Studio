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

var N = 32;
var history = [];
var idx = 0;

function transform(value) {
  history[idx] = value * value;
  idx = (idx + 1) % N;

  var sumSq = 0;
  var count = Math.min(history.length, N);
  for (var i = 0; i < count; i++)
    sumSq += history[i];

  return Math.sqrt(sumSq / count);
}
