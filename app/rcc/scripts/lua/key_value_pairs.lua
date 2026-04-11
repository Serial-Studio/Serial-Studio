--
-- Key-Value Pairs Parser
--
-- Parses key-value formatted data into an array of values using a custom keymap.
--
-- INPUT FORMAT: "temperature=22.5,humidity=60.3,pressure=1013"
-- OUTPUT ARRAY: {22.5, 60.3, 1013}
--
-- Note: Frame delimiters are automatically removed by Serial Studio.
--       Values are mapped to array indices based on the keyToIndexMap.
--       Missing keys retain their previous values between frames.
--

--------------------------------------------------------------------------------
-- Configuration
--------------------------------------------------------------------------------

-- Maps key names to array indices (1-based for Lua).
-- Customize this table to match your data format and desired output order.
local keyToIndexMap = {
  temperature = 1,
  humidity    = 2,
  pressure    = 3
}

-- Count of mapped keys (used to size the output array).
local numItems = 0
for _ in pairs(keyToIndexMap) do numItems = numItems + 1 end

-- Array to hold parsed values with default initialization.
-- Values persist between frames unless updated by incoming data.
local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses a key-value frame into an array of values.
-- @param frame The key-value data from the data source
-- @return Array of values mapped according to keyToIndexMap
function parse(frame)
  -- Match "key=value" pairs where value is numeric (digits and dots)
  for key, value in frame:gmatch("([%w_]+)=([%d.]+)") do
    local idx = keyToIndexMap[key]
    if idx then
      parsedValues[idx] = tonumber(value) or 0
    end
  end

  return parsedValues
end
