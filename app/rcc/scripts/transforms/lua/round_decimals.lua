-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Round Decimals
-- Rounds the value to N decimal places. Set 'decimals' to control
-- precision (e.g., 2 = round to hundredths).
-- Adjust N to match your sensor.
--

function transform(value)
  local N = 2
  local factor = 10 ^ N
  return math.floor(value * factor + 0.5) / factor
end
