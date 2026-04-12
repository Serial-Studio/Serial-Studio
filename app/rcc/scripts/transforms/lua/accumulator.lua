-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Accumulator
-- Running sum (discrete integral) of all received values. Adds each
-- incoming value to a persistent total. Use for odometers, energy
-- counters, etc.
--

local total = 0

function transform(value)
  total = total + value
  return total
end
