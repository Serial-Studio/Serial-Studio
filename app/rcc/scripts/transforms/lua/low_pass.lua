-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Low-Pass Filter
-- First-order IIR low-pass filter. Alpha = dt/(RC + dt). Smaller
-- alpha = lower cutoff frequency = more smoothing. Good for removing
-- high-frequency noise.
-- Adjust alpha to match your sensor.
--

local alpha = 0.05
local filtered

function transform(value)
  if filtered == nil then
    filtered = value
  end

  filtered = filtered + alpha * (value - filtered)
  return filtered
end
