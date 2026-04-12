-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Schmitt Trigger (hysteresis comparator)
-- Outputs 1 when the input rises above highThreshold, and 0 when
-- it falls below lowThreshold. Values in between keep the previous
-- state. This prevents rapid 0/1 toggling when a noisy analog
-- signal hovers near a single threshold.
--

local highThreshold = 7
local lowThreshold = 3
local state = 0

function transform(value)
  if state == 0 and value > highThreshold then
    state = 1
  elseif state == 1 and value < lowThreshold then
    state = 0
  end

  return state
end
