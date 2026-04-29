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
-- Implementation note: a parallel sorted view of the FIFO window is
-- kept up to date with binary-search insert + shift on every sample
-- (O(N) per call, no allocation). This avoids the table-allocation +
-- table.sort cost the naive copy-and-sort approach pays per call,
-- and stays well under a microsecond per sample at small N.
--

local N = 5
local fifo = {}
local sorted = {}
local idx = 1
local count = 0

for i = 1, N do
  fifo[i] = 0
  sorted[i] = 0
end

function transform(value)
  -- Drop the oldest sample from the sorted view when the window is full
  if count == N then
    local old = fifo[idx]
    local lo, hi = 1, count
    while lo < hi do
      local mid = (lo + hi) // 2
      if sorted[mid] < old then
        lo = mid + 1
      else
        hi = mid
      end
    end
    for i = lo, count - 1 do
      sorted[i] = sorted[i + 1]
    end
    count = count - 1
  end

  -- Insert the new value into the sorted view at the correct position
  local lo, hi = 1, count + 1
  while lo < hi do
    local mid = (lo + hi) // 2
    if sorted[mid] < value then
      lo = mid + 1
    else
      hi = mid
    end
  end
  for i = count, lo, -1 do
    sorted[i + 1] = sorted[i]
  end
  sorted[lo] = value
  count = count + 1

  -- Update the chronological FIFO buffer
  fifo[idx] = value
  idx = idx % N + 1

  -- Median: middle element for odd N, lower-middle for even N
  return sorted[(count + 1) // 2]
end
