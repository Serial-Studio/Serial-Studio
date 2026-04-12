// 'value' is the live numeric reading for this dataset,
// updated on every incoming frame.
//
// Globals declared at the top level persist between frames, so
// filters, accumulators, and other stateful transforms can keep
// their state across calls — just like a frame parser script.
//
// Bit Extract
// Returns a single bit from an integer status word as 0 or 1.
// Useful for decoding packed flag registers (e.g. "bit 3 = motor
// enabled"). Change bitIndex to pick a different bit (0 = LSB).
//

var bitIndex = 0;

function transform(value) {
  return (Math.trunc(value) >> bitIndex) & 1;
}
