-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Kalman Filter (1D, scalar)
-- Optimal linear filter for a single scalar signal with Gaussian
-- noise. Tune two parameters:
--   Q = process noise  (higher = trusts new measurements more)
--   R = measurement noise (higher = smooths more aggressively)
-- Start with Q small and R close to your sensor's noise variance.
--

local Q = 0.01
local R = 1.0
local x
local P = 1.0

function transform(value)
  if x == nil then
    x = value
    return x
  end

  P = P + Q
  local K = P / (P + R)
  x = x + K * (value - x)
  P = (1 - K) * P

  return x
end
