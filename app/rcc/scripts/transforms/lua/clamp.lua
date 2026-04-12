-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Clamp
-- Restricts the output to a fixed range [min, max]. Values below
-- min become min; values above max become max. Use for safety
-- limits or display bounds.
-- Adjust min_val, max_val to match your sensor.
--

function transform(value)
  local min_val = 0
  local max_val = 100
  return math.max(min_val, math.min(max_val, value))
end
