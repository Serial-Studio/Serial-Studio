-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Map Range
-- Rescales a value from one range to another. Maps [inMin, inMax]
-- to [outMin, outMax] linearly. Classic for ADC-to-engineering-unit
-- conversion.
-- Adjust in_min, in_max, out_min, out_max to match your sensor.
--

function transform(value)
  local in_min = 0
  local in_max = 1023
  local out_min = 0
  local out_max = 5.0
  return (value - in_min) / (in_max - in_min) * (out_max - out_min) + out_min
end
