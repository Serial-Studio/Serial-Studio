--
-- Binary TLV (Tag-Length-Value) Parser
--
-- Parses binary TLV-encoded data into an array of values.
--
-- INPUT FORMAT: Binary data with TLV structure
--   {Tag1, Length1, Value1..., Tag2, Length2, Value2...}
--
-- OUTPUT TABLE: {value1, value2, value3, ...}  (mapped to predefined indices)
--

----------------------------------------------------------------------
-- Configuration
----------------------------------------------------------------------

local numItems = 5

local tagToIndexMap = {
  [0x01] = 1,
  [0x02] = 2,
  [0x03] = 3,
  [0x04] = 4,
  [0x05] = 5,
}

local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  local i = 1

  -- Need at least Tag and Length bytes to process an entry
  while i <= #frame - 1 do
    local tag = frame[i]
    i = i + 1
    local length = frame[i]
    i = i + 1

    -- Stop if the value bytes would exceed the frame boundary
    if i + length - 1 > #frame then
      break
    end

    -- Read the value bytes and combine big-endian into a number
    local value = 0
    for j = 0, length - 1 do
      value = (value << 8) | frame[i]
      i = i + 1
    end

    local idx = tagToIndexMap[tag]
    if idx then
      parsedValues[idx] = value
    end
  end

  return parsedValues
end
