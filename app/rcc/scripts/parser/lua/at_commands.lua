--
-- AT Command Response Parser
--
-- Parses AT command responses from modems and cellular modules.
--
-- INPUT FORMAT: "+CSQ: 25,99" or "+CREG: 0,1" or "OK"
-- OUTPUT TABLE: {25, 99, 0, 1, ...}  (mapped to predefined indices)
--

----------------------------------------------------------------------
-- Configuration
----------------------------------------------------------------------

local numItems = 8

local commandToIndexMap = {
  CSQ   = {1, 2},
  CREG  = {3, 4},
  CGATT = {5},
  COPS  = {6, 7, 8},
}

local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  frame = frame:match("^%s*(.-)%s*$")

  -- Only process frames that contain a command with parameters
  if not frame:find(":") then
    return parsedValues
  end

  -- Split at ':' to separate command name from parameters
  local cmd, params_str = frame:match("^([^:]+):(.+)$")
  if not cmd or not params_str then
    return parsedValues
  end

  -- Remove the '+' prefix and whitespace
  cmd = cmd:gsub("%+", ""):match("^%s*(.-)%s*$")

  -- Look up command in the index map
  local indices = commandToIndexMap[cmd]
  if not indices then
    return parsedValues
  end

  -- Split parameters by comma and assign to mapped indices
  local i = 1
  for param in params_str:gmatch("([^,]+)") do
    if i > #indices then break end
    param = param:match("^%s*(.-)%s*$")
    parsedValues[indices[i]] = tonumber(param) or param
    i = i + 1
  end

  return parsedValues
end
