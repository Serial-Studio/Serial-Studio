// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// ADC to Voltage
// Converts a raw ADC reading to voltage. For a 10-bit ADC with
// 3.3V reference: voltage = value * 3.3 / 1023.
// Adjust v_ref, resolution to match your sensor.
//

function transform(value) {
  var v_ref = 3.3;
  var resolution = 1024;
  return value * v_ref / resolution;
}
