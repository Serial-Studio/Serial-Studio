-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Rate of Change
-- Computes the difference between the current and previous value
-- (discrete derivative). Returns how much the signal changed since
-- the last frame.
--

local prev

function transform(value)
  if prev == nil then
    prev = value
  end

  local delta = value - prev
  prev = value
  return delta
end
