-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Running Maximum
-- Returns the largest value seen since the transform started.
-- Useful for tracking high peaks (maximum temperature, peak current,
-- vibration amplitude, etc). The value only increases — it never
-- resets.
--

local maxValue

function transform(value)
  if maxValue == nil or value > maxValue then
    maxValue = value
  end

  return maxValue
end
