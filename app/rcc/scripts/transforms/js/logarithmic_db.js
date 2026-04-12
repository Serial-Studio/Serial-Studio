// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Logarithmic dB
// Converts a linear amplitude to decibels: dB = 20 * log10(value).
// Values <= 0 are clamped to -Infinity to avoid undefined results.
//

function transform(value) {
  if (value <= 0)
    return -Infinity;

  return 20 * Math.log10(value);
}
