-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Auto-Zero / Tare
-- Averages the first N samples to determine a bias, then subtracts
-- that bias from every subsequent sample. Useful for zeroing out
-- sensor offsets on load cells, pressure sensors, and IMUs at
-- startup without needing a physical tare button.
-- Adjust warmupSamples to match how many samples you want to average.
--

local warmupSamples = 50
local bias = 0
local samplesCollected = 0

function transform(value)
  if samplesCollected < warmupSamples then
    bias = (bias * samplesCollected + value) / (samplesCollected + 1)
    samplesCollected = samplesCollected + 1
    return 0
  end

  return value - bias
end
