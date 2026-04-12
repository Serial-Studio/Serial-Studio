// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Celsius to Fahrenheit
// Converts temperature from Celsius to Fahrenheit: F = C * 9/5 + 32.
//

function transform(value) {
  return value * 9.0 / 5.0 + 32.0;
}
