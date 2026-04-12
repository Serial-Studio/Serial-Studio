-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Absolute Value
-- Returns the absolute (unsigned) magnitude of the value. Negative
-- values become positive.
--

function transform(value)
  return math.abs(value)
end
