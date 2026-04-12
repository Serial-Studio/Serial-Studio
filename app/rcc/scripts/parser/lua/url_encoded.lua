--
-- URL-Encoded Data Parser
--
-- Parses URL-encoded (percent-encoded) data into an array of values.
--
-- INPUT FORMAT: "temp=25.5&humidity=60&pressure=1013"
-- OUTPUT TABLE: {25.5, 60, 1013, ...}  (mapped to predefined indices)
--

----------------------------------------------------------------------
-- Configuration
----------------------------------------------------------------------

local numItems = 6

local keyToIndexMap = {
  temperature = 1,
  humidity    = 2,
  pressure    = 3,
  voltage     = 4,
  current     = 5,
  power       = 6,
}

local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

----------------------------------------------------------------------
-- Helper Functions
----------------------------------------------------------------------

local function urlDecode(str)
  str = str:gsub("+", " ")
  str = str:gsub("%%(%x%x)", function(hex)
    return string.char(tonumber(hex, 16))
  end)
  return str
end

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  frame = frame:match("^%s*(.-)%s*$")
  if #frame == 0 then
    return parsedValues
  end

  for pair in frame:gmatch("([^&]+)") do
    local key, valueStr = pair:match("^([^=]+)=(.*)$")
    if key then
      key = urlDecode(key)
      valueStr = urlDecode(valueStr)

      local idx = keyToIndexMap[key]
      if idx then
        parsedValues[idx] = tonumber(valueStr) or valueStr
      end
    end
  end

  return parsedValues
end
