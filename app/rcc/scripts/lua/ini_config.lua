--
-- INI/Config Format Parser
--
-- Parses INI-style key=value pairs into an array of values.
--
-- INPUT FORMAT: "temperature=25.5\nhumidity=60\npressure=1013"
-- OUTPUT TABLE: {25.5, 60, 1013, ...}  (mapped to predefined indices)
--

----------------------------------------------------------------------
-- Configuration
----------------------------------------------------------------------

local numItems = 5

local keyToIndexMap = {
  temperature = 1,
  humidity    = 2,
  pressure    = 3,
  battery     = 4,
  signal      = 5,
}

local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  for line in frame:gmatch("[^\n]+") do
    line = line:match("^%s*(.-)%s*$")

    -- Skip empty lines and comments
    if #line == 0 then goto continue end
    local first = line:sub(1, 1)
    if first == ";" or first == "#" then goto continue end

    -- Split at '=' to get key and value
    local key, valueStr = line:match("^([^=]+)=(.+)$")
    if not key then goto continue end

    key = key:match("^%s*(.-)%s*$")
    valueStr = valueStr:match("^%s*(.-)%s*$")

    local idx = keyToIndexMap[key]
    if idx then
      parsedValues[idx] = tonumber(valueStr) or valueStr
    end

    ::continue::
  end

  return parsedValues
end
