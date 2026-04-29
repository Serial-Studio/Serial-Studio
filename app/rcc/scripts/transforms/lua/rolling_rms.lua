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
-- Implementation note: the running sum is maintained in O(1) per
-- sample by subtracting the value leaving the window and adding the
-- new one. This keeps the transform cheap even at audio rates where
-- N may be in the tens of thousands.
--

local N = 32
local history = {}
local idx = 1
local count = 0
local sumSq = 0

for i = 1, N do
  history[i] = 0
end

function transform(value)
  local sq = value * value
  sumSq = sumSq - history[idx] + sq
  history[idx] = sq
  idx = idx % N + 1
  if count < N then
    count = count + 1
  end

  -- Guard against tiny negative drift from float cancellation
  if sumSq < 0 then
    sumSq = 0
  end

  return math.sqrt(sumSq / count)
end
