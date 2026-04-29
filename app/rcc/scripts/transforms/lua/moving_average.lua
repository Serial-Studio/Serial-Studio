-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Moving Average
-- Smooths noisy signals by averaging the last N samples. Uses a
-- circular buffer for memory efficiency. Introduces N/2 frames of lag.
-- Adjust N to match your sensor.
--
-- Implementation note: the running sum is maintained in O(1) per
-- sample by subtracting the value leaving the window and adding the
-- new one. This keeps the transform cheap even at audio rates where
-- N may be in the tens of thousands.
--

local N = 10
local history = {}
local idx = 1
local count = 0
local sum = 0

for i = 1, N do
  history[i] = 0
end

function transform(value)
  sum = sum - history[idx] + value
  history[idx] = value
  idx = idx % N + 1
  if count < N then
    count = count + 1
  end

  return sum / count
end
