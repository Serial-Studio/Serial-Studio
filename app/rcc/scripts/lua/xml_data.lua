--
-- XML Data Parser
--
-- Parses XML-formatted data into an array of values.
--
-- XML (eXtensible Markup Language) is a widely-used markup language for
-- structured data exchange. This parser extracts values from XML elements
-- based on tag names.
--
-- INPUT FORMAT: "<root><temp>25.5</temp><humidity>60</humidity></root>"
-- OUTPUT ARRAY: {25.5, 60, ...}  (mapped to predefined indices)
--
-- Note: Frame delimiters are automatically removed by Serial Studio.
--       This is a lightweight XML parser suitable for simple structures.
--

--------------------------------------------------------------------------------
-- Configuration
--------------------------------------------------------------------------------

-- Total number of values in the output array.
-- This should match the number of datasets you've configured in Serial Studio.
local numItems = 8

-- Maps XML tag names to array indices (1-based for Lua).
-- Example: If XML contains <temperature>25.5</temperature>,
-- the value 25.5 will be placed at index 1.
local tagToIndexMap = {
  temperature = 1,
  humidity    = 2,
  pressure    = 3,
  voltage     = 4,
  current     = 5,
  altitude    = 6,
  speed       = 7
}

-- Array to hold all parsed values.
-- Values persist between frames unless updated by new data.
local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

--------------------------------------------------------------------------------
-- Helper Functions
--------------------------------------------------------------------------------

-- Extracts all text content for a given XML tag name.
-- Returns a list of values (supports multiple occurrences of the same tag).
local function extractTagValue(xml, tagName)
  local values = {}
  local openTag  = "<" .. tagName .. ">"
  local closeTag = "</" .. tagName .. ">"
  local searchPos = 1

  while true do
    local startPos = xml:find(openTag, searchPos, true)
    if not startPos then break end

    local endPos = xml:find(closeTag, startPos, true)
    if not endPos then break end

    values[#values + 1] = xml:sub(startPos + #openTag, endPos - 1)
    searchPos = endPos + #closeTag
  end

  return values
end

-- Extracts values from tags with attributes or self-closing tags.
-- Handles: <tag attr="...">content</tag> and <tag value="..." />.
local function extractTagWithAttributes(xml, tagName)
  local values = {}

  -- Match <tag ...>content</tag> (content without '<')
  local pattern = "<" .. tagName .. "[^>]*>([^<]*)</" .. tagName .. ">"
  for content in xml:gmatch(pattern) do
    local trimmed = content:match("^%s*(.-)%s*$")
    if trimmed and #trimmed > 0 then
      values[#values + 1] = trimmed
    end
  end

  -- Match self-closing <tag ... value="..." /> and extract the value attribute
  local selfClosingPattern = "<" .. tagName .. "([^/>]*)/%s*>"
  for attrs in xml:gmatch(selfClosingPattern) do
    local v = attrs:match('value%s*=%s*"([^"]*)"')
          or attrs:match("value%s*=%s*'([^']*)'")
    if v then
      values[#values + 1] = v
    end
  end

  return values
end

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses an XML frame into the predefined array structure.
-- @param frame The XML data from the data source
-- @return Array of values mapped according to tagToIndexMap
function parse(frame)
  -- Trim whitespace
  frame = frame:match("^%s*(.-)%s*$") or ""
  if #frame == 0 then
    return parsedValues
  end

  -- Walk every configured tag and store its first occurrence
  for tagName, index in pairs(tagToIndexMap) do
    local values = extractTagValue(frame, tagName)

    -- Fall back to attribute/self-closing extraction
    if #values == 0 then
      values = extractTagWithAttributes(frame, tagName)
    end

    if #values > 0 then
      local valueStr = values[1]:match("^%s*(.-)%s*$") or ""
      parsedValues[index] = tonumber(valueStr) or valueStr
    end
  end

  return parsedValues
end
