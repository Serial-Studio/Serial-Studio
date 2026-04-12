-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Logarithmic dB
-- Converts a linear amplitude to decibels: dB = 20 * log10(value).
-- Values <= 0 are clamped to -inf to avoid undefined results.
--

function transform(value)
  if value <= 0 then
    return -math.huge
  end

  return 20 * math.log(value) / math.log(10)
end
