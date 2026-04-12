-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- 2nd-Order Polynomial
-- Applies a 2nd-order polynomial: y = a*x^2 + b*x + c. Use for
-- sensors with non-linear response curves (e.g., thermocouples,
-- pressure transducers).
-- Adjust a, b, c to match your sensor.
--

function transform(value)
  local a = 1.0
  local b = 0.0
  local c = 0.0
  return a * value * value + b * value + c
end
