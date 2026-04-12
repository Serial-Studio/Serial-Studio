-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Exponential Moving Average
-- Weights recent values more heavily. Alpha controls responsiveness:
-- closer to 1 = faster response (less smoothing), closer to 0 =
-- smoother (more lag).
-- Adjust alpha to match your sensor.
--

local alpha = 0.1
local ema

function transform(value)
  if ema == nil then
    ema = value
  end

  ema = alpha * value + (1 - alpha) * ema
  return ema
end
