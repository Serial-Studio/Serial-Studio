// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Median Filter (rolling window)
// Replaces each sample with the median of the last N samples. More
// robust than moving average at rejecting outlier spikes (salt and
// pepper noise) without smearing them into neighboring samples.
// Adjust N to match your sensor (use odd values).
//

var N = 5;
var window = [];

function transform(value) {
  window.push(value);
  if (window.length > N)
    window.shift();

  var sorted = window.slice().sort(function (a, b) { return a - b; });
  return sorted[Math.floor(sorted.length / 2)];
}
