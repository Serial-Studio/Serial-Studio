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
// Implementation note: a parallel sorted view of the FIFO window is
// kept up to date with binary-search insert + shift on every sample
// (O(N) per call, no allocation). This avoids the array-slice + sort
// cost the naive copy-and-sort approach pays per call, and stays
// well under a microsecond per sample at small N.
//

var N = 5;
var fifo = new Array(N).fill(0);
var sorted = new Array(N).fill(0);
var idx = 0;
var count = 0;

function transform(value) {
  // Drop the oldest sample from the sorted view when the window is full
  if (count === N) {
    var old = fifo[idx];
    var lo = 0, hi = count;
    while (lo < hi) {
      var mid = (lo + hi) >> 1;
      if (sorted[mid] < old) lo = mid + 1;
      else hi = mid;
    }
    for (var i = lo; i < count - 1; i++)
      sorted[i] = sorted[i + 1];
    count--;
  }

  // Insert the new value into the sorted view at the correct position
  var lo = 0, hi = count;
  while (lo < hi) {
    var mid = (lo + hi) >> 1;
    if (sorted[mid] < value) lo = mid + 1;
    else hi = mid;
  }
  for (var i = count; i > lo; i--)
    sorted[i] = sorted[i - 1];
  sorted[lo] = value;
  count++;

  // Update the chronological FIFO buffer
  fifo[idx] = value;
  idx = (idx + 1) % N;

  // Median: middle element for odd N, lower-middle for even N
  return sorted[(count - 1) >> 1];
}
