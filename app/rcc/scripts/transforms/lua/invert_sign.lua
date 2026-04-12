-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Invert Sign
-- Negates the value. Positive becomes negative and vice versa. Use
-- when sensor polarity is reversed.
--

function transform(value)
  return -value
end
