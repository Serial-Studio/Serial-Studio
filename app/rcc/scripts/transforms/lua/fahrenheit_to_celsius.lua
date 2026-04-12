-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Fahrenheit to Celsius
-- Converts temperature from Fahrenheit to Celsius: C = (F - 32) * 5/9.
--

function transform(value)
  return (value - 32.0) * 5.0 / 9.0
end
