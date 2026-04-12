--
-- YAML Data Parser
--
-- Parses simple YAML key-value pairs into an array of values.
--
-- INPUT FORMAT: "temperature: 25.5\nhumidity: 60\npressure: 1013"
-- OUTPUT TABLE: {25.5, 60, 1013, ...}  (mapped to predefined indices)
--

----------------------------------------------------------------------
-- Configuration
----------------------------------------------------------------------

local numItems = 7

local keyToIndexMap = {
  temperature = 1,
  humidity    = 2,
  pressure    = 3,
  voltage     = 4,
  current     = 5,
  altitude    = 6,
  speed       = 7,
}

local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

----------------------------------------------------------------------
-- Helper Functions
----------------------------------------------------------------------

local function parseYAMLValue(str)
  str = str:match("^%s*(.-)%s*$")
  if str == "" or str == "null" or str == "~" then return nil end
  if str == "true" or str == "yes" or str == "on" then return true end
  if str == "false" or str == "no" or str == "off" then return false end

  -- Strip surrounding quotes
  local first, last = str:sub(1, 1), str:sub(-1)
  if (first == '"' and last == '"') or (first == "'" and last == "'") then
    return str:sub(2, -2)
  end

  return tonumber(str) or str
end

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  frame = frame:match("^%s*(.-)%s*$")
  if #frame == 0 then
    return parsedValues
  end

  for line in frame:gmatch("[^\n]+") do
    -- Strip inline comments
    local commentPos = line:find("#", 1, true)
    if commentPos then
      line = line:sub(1, commentPos - 1)
    end

    line = line:match("^%s*(.-)%s*$")
    if #line == 0 or line == "---" or line == "..." then
      goto continue
    end

    -- Split at first colon
    local colonPos = line:find(":", 1, true)
    if not colonPos then goto continue end

    local key = line:sub(1, colonPos - 1):match("^%s*(.-)%s*$")
    local valueStr = line:sub(colonPos + 1)

    -- Strip surrounding quotes from key
    local first, last = key:sub(1, 1), key:sub(-1)
    if (first == '"' and last == '"') or (first == "'" and last == "'") then
      key = key:sub(2, -2)
    end

    local idx = keyToIndexMap[key]
    if idx then
      local value = parseYAMLValue(valueStr)
      if value ~= nil then
        parsedValues[idx] = value
      end
    end

    ::continue::
  end

  return parsedValues
end
