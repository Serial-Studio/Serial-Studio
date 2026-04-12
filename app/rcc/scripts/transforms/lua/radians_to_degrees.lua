-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Radians to Degrees
-- Converts an angle from radians to degrees: deg = rad * 180/pi.
--

function transform(value)
  return value * 180.0 / math.pi
end
