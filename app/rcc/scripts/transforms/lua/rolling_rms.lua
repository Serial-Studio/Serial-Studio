-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Rolling RMS (root mean square)
-- Computes the RMS value over the last N samples. Fundamental for
-- AC signals, audio levels, vibration amplitude, and power
-- measurements where the average of a zero-mean signal is zero but
-- the RMS is meaningful.
-- Adjust N to match your sensor.
--

local N = 32
local history = {}
local idx = 1
local count = 0

function transform(value)
  history[idx] = value * value
  idx = idx % N + 1
  if count < N then
    count = count + 1
  end

  local sumSq = 0
  for i = 1, count do
    sumSq = sumSq + history[i]
  end

  return math.sqrt(sumSq / count)
end
