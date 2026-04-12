-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Median Filter (rolling window)
-- Replaces each sample with the median of the last N samples. More
-- robust than moving average at rejecting outlier spikes (salt and
-- pepper noise) without smearing them into neighboring samples.
-- Adjust N to match your sensor (use odd values).
--

local N = 5
local window = {}

function transform(value)
  table.insert(window, value)
  if #window > N then
    table.remove(window, 1)
  end

  local sorted = {}
  for i = 1, #window do
    sorted[i] = window[i]
  end
  table.sort(sorted)

  return sorted[math.floor(#sorted / 2) + 1]
end
