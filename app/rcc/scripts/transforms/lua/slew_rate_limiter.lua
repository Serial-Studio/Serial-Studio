-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Slew-Rate Limiter
-- Caps how much the value can change between consecutive samples.
-- Useful for rejecting sudden spikes or enforcing safe ramp rates
-- on actuators. Set maxDelta to the largest change you allow per
-- frame (in the same units as the signal).
--

local maxDelta = 10
local prev

function transform(value)
  if prev == nil then
    prev = value
    return value
  end

  local delta = value - prev
  if delta > maxDelta then
    prev = prev + maxDelta
  elseif delta < -maxDelta then
    prev = prev - maxDelta
  else
    prev = value
  end

  return prev
end
