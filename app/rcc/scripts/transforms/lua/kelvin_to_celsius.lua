-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Kelvin to Celsius
-- Converts temperature from Kelvin to Celsius: C = K - 273.15.
--

function transform(value)
  return value - 273.15
end
