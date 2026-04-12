--
-- Batched Sensor Data Parser Template
--
-- Use this template for devices that send scalar metadata along with
-- batched sensor readings in a single packet.
--
-- AUTOMATIC MULTI-FRAME EXPANSION
-- ================================
-- When you return a mixed scalar/table array like:
--   {scalar1, scalar2, {vec1, vec2, ..., vecN}}
--
-- Serial Studio automatically generates N frames:
--   Frame 1: {scalar1, scalar2, vec1}
--   Frame 2: {scalar1, scalar2, vec2}
--   ...
--   Frame N: {scalar1, scalar2, vecN}
--
-- The scalars are automatically repeated across all frames!
--
-- COMMON USE CASES
-- ================
-- 1. BLE devices batching high-frequency sensor data
-- 2. Environmental sensors with periodic batch uploads
-- 3. Audio devices sending sample chunks
-- 4. GPS trackers with position history
-- 5. Industrial sensors with hourly batches
--
-- EXAMPLE INPUT
-- =============
-- {
--   "device_id": 42,
--   "battery": 3.7,
--   "temperature": 25.5,
--   "samples": [1.23, 4.56, 7.89, 10.11, 12.34]
-- }
--
-- EXAMPLE OUTPUT
-- ==============
-- {42, 3.7, 25.5, {1.23, 4.56, 7.89, 10.11, 12.34}}
--
-- This generates 5 frames:
--   {42, 3.7, 25.5, 1.23}
--   {42, 3.7, 25.5, 4.56}
--   {42, 3.7, 25.5, 7.89}
--   {42, 3.7, 25.5, 10.11}
--   {42, 3.7, 25.5, 12.34}
--

--------------------------------------------------------------------------------
-- Helper Functions
--------------------------------------------------------------------------------

-- Extracts a scalar numeric or string field from a flat JSON object.
-- Returns the parsed value or `default` if the key is missing.
local function jsonField(frame, key, default)
  -- Quoted string value
  local strVal = frame:match('"' .. key .. '"%s*:%s*"([^"]*)"')
  if strVal then return strVal end

  -- Unquoted numeric/boolean/null value
  local rawVal = frame:match('"' .. key .. '"%s*:%s*([%-%d%.eE]+)')
  if rawVal then return tonumber(rawVal) or default end

  return default
end

-- Extracts a numeric array field from a flat JSON object.
-- Example: jsonArray(frame, "samples") on '"samples":[1,2,3]' → {1, 2, 3}
local function jsonArray(frame, key)
  local body = frame:match('"' .. key .. '"%s*:%s*%[([^%]]*)%]')
  if not body then return {} end

  local values = {}
  for token in body:gmatch("[^,]+") do
    local trimmed = token:match("^%s*(.-)%s*$")
    values[#values + 1] = tonumber(trimmed) or trimmed
  end
  return values
end

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

function parse(frame)
  -- STEP 1: Parse incoming JSON data
  -- This helper handles flat JSON objects (no nested objects in metadata).
  -- For complex/nested JSON, use the JavaScript engine.

  -- STEP 2: Extract scalar metadata (repeated across all generated frames)
  local device_id   = jsonField(frame, "device_id",   0)
  local battery     = jsonField(frame, "battery",     0.0)
  local temperature = jsonField(frame, "temperature", 0.0)

  -- STEP 3: Extract vector arrays (each element becomes one frame)
  local samples = jsonArray(frame, "samples")

  -- Optional: handle multiple sensor arrays
  -- local accel_x = jsonArray(frame, "accel_x")
  -- local accel_y = jsonArray(frame, "accel_y")
  -- local accel_z = jsonArray(frame, "accel_z")

  -- STEP 4: Return mixed scalar/vector array
  -- Format: {scalar1, scalar2, ..., vector1, vector2, ...}
  return {
    device_id,    -- Scalar - repeated in all frames
    battery,      -- Scalar - repeated in all frames
    temperature,  -- Scalar - repeated in all frames
    samples       -- Vector - unzipped element-by-element
  }

  -- For multiple vectors, use:
  -- return {device_id, battery, temperature, accel_x, accel_y, accel_z}
  --
  -- If vectors have different lengths, Serial Studio automatically
  -- extends shorter vectors by repeating their last value.
end

--------------------------------------------------------------------------------
-- EXAMPLE VARIATIONS
--------------------------------------------------------------------------------

-- VARIATION 1: CSV format with batched values
-- Input: "42,3.7,25.5;1.23,4.56,7.89"
--[[
function parse(frame)
  local meta_str, samples_str = frame:match("^([^;]+);(.+)$")

  local meta = {}
  for field in meta_str:gmatch("([^,]+)") do
    meta[#meta + 1] = tonumber(field) or 0
  end

  local samples = {}
  for field in samples_str:gmatch("([^,]+)") do
    samples[#samples + 1] = tonumber(field) or 0
  end

  return {meta[1], meta[2], meta[3], samples}
end
--]]

-- VARIATION 2: Packed binary data (received as byte table)
-- Input: header (3 bytes) + packed 16-bit samples
--[[
function parse(frame)
  local device_id   = frame[1]
  local battery     = frame[2] / 100.0
  local temperature = frame[3] - 40

  local samples = {}
  for i = 9, #frame, 2 do
    local value = (frame[i] << 8) | frame[i + 1]
    samples[#samples + 1] = value / 100.0
  end

  return {device_id, battery, temperature, samples}
end
--]]

-- VARIATION 3: Multiple sensor arrays with different lengths
-- Input: '{"id":42,"temp":25.5,"accel":[1,2,3],"gyro":[4,5]}'
--[[
function parse(frame)
  local id    = jsonField(frame, "id",   0)
  local temp  = jsonField(frame, "temp", 0.0)
  local accel = jsonArray(frame, "accel")
  local gyro  = jsonArray(frame, "gyro")

  return {id, temp, accel, gyro}
  --
  -- This generates 3 frames (length of longest vector):
  --   {42, 25.5, 1, 4}
  --   {42, 25.5, 2, 5}
  --   {42, 25.5, 3, 5}  <- gyro[2] repeated
end
--]]
