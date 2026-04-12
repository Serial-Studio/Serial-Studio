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

var N = 10;
var history = [];
var idx = 0;

function transform(value) {
  history[idx] = value;
  idx = (idx + 1) % N;

  var sum = 0;
  var count = Math.min(history.length, N);
  for (var i = 0; i < count; i++)
    sum += history[i];

  return sum / count;
}
