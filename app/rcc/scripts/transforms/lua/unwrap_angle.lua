-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- Unwrap Angle (continuous)
-- Removes ±360° jumps from an angle signal so the output is a
-- continuous value instead of wrapping at the boundary. Useful for
-- compass headings or rotary encoders that report in [0, 360] or
-- [-180, 180] but you want a cumulative total (e.g. for plotting
-- full-turn rotation history).
--

local prev
local offset = 0

function transform(value)
  if prev == nil then
    prev = value
    return value
  end

  local delta = value - prev
  if delta > 180 then
    offset = offset - 360
  elseif delta < -180 then
    offset = offset + 360
  end

  prev = value
  return value + offset
end
