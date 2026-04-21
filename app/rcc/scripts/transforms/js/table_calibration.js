// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Data-Table Calibration
// Reads the slope and offset from a shared data table instead of
// hard-coding them in the transform. Define a table called
// "calibration" in the Project Editor with two Constant registers
// named "slope" and "offset", and every dataset using this
// template will pick up the same values. Change the calibration
// in one place and it applies everywhere.
//

function transform(value) {
  var slope  = tableGet("calibration", "slope");
  var offset = tableGet("calibration", "offset");
  if (slope  === undefined) slope  = 1.0;
  if (offset === undefined) offset = 0.0;
  return slope * value + offset;
}
