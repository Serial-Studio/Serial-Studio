--
-- JSON Data Parser
--
-- Parses JSON-formatted data into an array of values.
--
-- INPUT FORMAT: {"time":"12:05","speed":48,"angle":4.0,"distance":87}
-- OUTPUT ARRAY: {"12:05", 48, 4.0, 87}
--
-- Note: Frame delimiters are automatically removed by Serial Studio.
--       Values persist between frames unless updated by new data.
--

--------------------------------------------------------------------------------
-- Configuration
--------------------------------------------------------------------------------

-- Total number of values in the output array.
-- This should match the number of datasets you've configured in Serial Studio.
local numItems = 4

-- Maps JSON field names to array indices (1-based for Lua).
-- Customize this table to match your JSON structure and desired output order.
local fieldToIndexMap = {
  time     = 1,
  speed    = 2,
  angle    = 3,
  distance = 4
}

-- Array to hold all parsed values.
-- Values persist between frames unless updated by new data.
local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses a JSON frame into an array of values.
-- @param frame The JSON data from the data source
-- @return Array of values mapped according to fieldToIndexMap
function parse(frame)
  -- Extract "key":value pairs via regex (flat object, no nesting)
  for key, value in frame:gmatch('"([^"]+)"%s*:%s*([^,}]+)') do
    local idx = fieldToIndexMap[key]
    if idx then
      -- Strip surrounding quotes from string values
      local stripped = value:match('^%s*"(.*)"%s*$')
      if stripped then
        parsedValues[idx] = stripped
      else
        -- Try numeric conversion, fall back to trimmed literal
        local trimmed = value:match("^%s*(.-)%s*$")
        parsedValues[idx] = tonumber(trimmed) or trimmed
      end
    end
  end

  return parsedValues
end
