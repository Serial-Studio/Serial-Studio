-- 'value' is the live numeric reading for this dataset,
-- updated on every incoming frame.
--
-- Globals declared at the top level persist between frames, so
-- filters, accumulators, and other stateful transforms can keep
-- their state across calls — just like a frame parser script.
--
-- High-Pass Filter (1st order)
-- Passes fast changes and removes slow drift / DC offset. Alpha
-- closer to 1 = higher cutoff (more drift removed), closer to 0 =
-- lower cutoff (only very slow drift removed).
-- Adjust alpha to match your sensor.
--

local alpha = 0.9
local prevInput
local prevOutput = 0

function transform(value)
  if prevInput == nil then
    prevInput = value
    return 0
  end

  prevOutput = alpha * (prevOutput + value - prevInput)
  prevInput = value
  return prevOutput
end
