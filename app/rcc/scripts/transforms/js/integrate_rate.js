// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Integrate Rate to Angle
// Converts an angular rate (deg/s) into an absolute angle by
// numerically integrating over time. Assumes a fixed sample rate
// — set sampleRateHz to match your sensor's output frequency.
// Wraps the result into [-180, 180] to avoid unbounded growth.
// Useful for gyroscope rate data when no absolute orientation is
// provided by the sensor.
//

var sampleRateHz = 100;
var dt = 1 / sampleRateHz;
var angle = 0;

function transform(value) {
  angle += value * dt;
  angle = ((angle + 180) % 360 + 360) % 360 - 180;
  return angle;
}
