--
-- Time-Series 2D Array Parser Template
--
-- Use this template for devices that send multiple complete data records
-- in a single packet, where each record is a separate frame.
--
-- AUTOMATIC MULTI-FRAME EXPANSION
-- ================================
-- When you return a 2D table like:
--   {{val1, val2, val3}, {val4, val5, val6}, {val7, val8, val9}}
--
-- Serial Studio automatically generates one frame per row:
--   Frame 1: {val1, val2, val3}
--   Frame 2: {val4, val5, val6}
--   Frame 3: {val7, val8, val9}
--
-- COMMON USE CASES
-- ================
-- 1. Data loggers uploading historical records
-- 2. Industrial systems with buffered measurements
-- 3. Network devices sending burst transmissions
-- 4. Batch processing results
-- 5. Replay/playback systems
--
-- EXAMPLE INPUT (JSON)
-- ====================
-- {
--   "records": [
--     {"timestamp": 1000, "temp": 25.5, "humidity": 60.0},
--     {"timestamp": 2000, "temp": 25.7, "humidity": 59.8},
--     {"timestamp": 3000, "temp": 25.9, "humidity": 59.5}
--   ]
-- }
--
-- EXAMPLE OUTPUT
-- ==============
-- {{1000, 25.5, 60.0}, {2000, 25.7, 59.8}, {3000, 25.9, 59.5}}
--
-- This generates 3 separate frames for visualization.
--

--------------------------------------------------------------------------------
-- Helper Functions
--------------------------------------------------------------------------------

-- Extracts a scalar numeric/string field from a flat JSON object substring.
local function jsonField(obj, key, default)
  local strVal = obj:match('"' .. key .. '"%s*:%s*"([^"]*)"')
  if strVal then return strVal end

  local rawVal = obj:match('"' .. key .. '"%s*:%s*([%-%d%.eE]+)')
  if rawVal then return tonumber(rawVal) or default end

  return default
end

-- Extracts every top-level {...} object from the value of a named JSON array.
-- Returns an array of raw object-body strings.
local function jsonObjectArray(frame, key)
  local body = frame:match('"' .. key .. '"%s*:%s*%[(.-)%]%s*[,}]')
  if not body then
    body = frame:match('"' .. key .. '"%s*:%s*%[(.*)%]')
  end
  if not body then return {} end

  local objects = {}
  local depth = 0
  local start = nil
  for i = 1, #body do
    local ch = body:sub(i, i)
    if ch == "{" then
      if depth == 0 then start = i end
      depth = depth + 1
    elseif ch == "}" then
      depth = depth - 1
      if depth == 0 and start then
        objects[#objects + 1] = body:sub(start, i)
        start = nil
      end
    end
  end
  return objects
end

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

function parse(frame)
  -- STEP 1: Parse incoming JSON data
  -- This helper walks the "records" array and extracts each {...} object.

  -- STEP 2: Extract array of records
  local records = jsonObjectArray(frame, "records")

  -- STEP 3: Convert each record to an array of values
  local result = {}
  for i = 1, #records do
    local record = records[i]
    result[#result + 1] = {
      jsonField(record, "timestamp", 0),
      jsonField(record, "temp",      0.0),
      jsonField(record, "humidity",  0.0)
    }
  end

  -- STEP 4: Return the 2D array
  return result
end

--------------------------------------------------------------------------------
-- EXAMPLE VARIATIONS
--------------------------------------------------------------------------------

-- VARIATION 1: CSV format with multiple rows
-- Input: "1000,25.5,60.0\n2000,25.7,59.8\n3000,25.9,59.5"
--[[
function parse(frame)
  local result = {}
  for line in frame:gmatch("[^\n]+") do
    local row = {}
    for field in line:gmatch("([^,]+)") do
      row[#row + 1] = tonumber(field) or field
    end
    if #row > 0 then result[#result + 1] = row end
  end
  return result
end
--]]

-- VARIATION 2: Fixed-width binary records
-- Input: Binary data with 5-byte records (1 id byte + 2 shorts BE)
--[[
function parse(frame)
  local result     = {}
  local recordSize = 5

  for i = 1, #frame, recordSize do
    if i + recordSize - 1 > #frame then break end
    local id       = frame[i]
    local temp     = ((frame[i + 1] << 8) | frame[i + 2]) / 10.0
    local humidity = ((frame[i + 3] << 8) | frame[i + 4]) / 10.0
    result[#result + 1] = {id, temp, humidity}
  end

  return result
end
--]]

-- VARIATION 3: Semicolon-separated rows, comma-separated values
-- Input: "1,25.5,60.0;2,25.7,59.8;3,25.9,59.5"
--[[
function parse(frame)
  local result = {}
  for row in frame:gmatch("[^;]+") do
    local values = {}
    for field in row:gmatch("([^,]+)") do
      values[#values + 1] = tonumber(field) or 0
    end
    result[#result + 1] = values
  end
  return result
end
--]]
