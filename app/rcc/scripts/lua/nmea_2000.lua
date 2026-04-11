--
-- NMEA 2000 Messages Parser
--
-- Parses NMEA 2000 (N2K) protocol messages from marine CAN bus networks.
--
-- NMEA 2000 is a CAN-based communication standard used in marine electronics
-- for navigation, engine data, and vessel monitoring. Unlike NMEA 0183 (ASCII),
-- NMEA 2000 uses binary messages over a CAN bus.
--
-- INPUT FORMAT: Binary NMEA 2000 CAN frame
--   Bytes 1-4:  CAN ID (29-bit extended, little-endian, contains PGN)
--   Byte  5:    Data length (0-8)
--   Bytes 6-13: Data payload (up to 8 bytes)
--
-- OUTPUT ARRAY: Extracted values based on PGN (Parameter Group Number)
--
-- Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
--       Frame delimiters are automatically removed by Serial Studio.
--

--------------------------------------------------------------------------------
-- Configuration
--------------------------------------------------------------------------------

-- Total number of values in the output array.
-- This should match the number of datasets you've configured in Serial Studio.
local numItems = 20

-- Array to hold all parsed values.
-- Values persist between frames unless updated by new data.
local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

-- Pre-computed radians-to-degrees conversion factor.
local RAD_TO_DEG = 180.0 / math.pi

--------------------------------------------------------------------------------
-- Helper Functions (zero-based offsets, matching JS reference)
--------------------------------------------------------------------------------

-- Extracts a 16-bit unsigned integer from a byte table (little-endian).
local function extractUint16LE(bytes, offset)
  local b0 = bytes[offset + 1] or 0
  local b1 = bytes[offset + 2] or 0
  return b0 | (b1 << 8)
end

-- Extracts a 16-bit signed integer from a byte table (little-endian).
local function extractInt16LE(bytes, offset)
  local v = extractUint16LE(bytes, offset)
  if v > 32767 then v = v - 65536 end
  return v
end

-- Extracts a 32-bit unsigned integer from a byte table (little-endian).
local function extractUint32LE(bytes, offset)
  local b0 = bytes[offset + 1] or 0
  local b1 = bytes[offset + 2] or 0
  local b2 = bytes[offset + 3] or 0
  local b3 = bytes[offset + 4] or 0
  return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)
end

-- Extracts a 32-bit signed integer from a byte table (little-endian).
local function extractInt32LE(bytes, offset)
  local v = extractUint32LE(bytes, offset)
  if v > 2147483647 then v = v - 4294967296 end
  return v
end

-- Extracts the PGN from a 29-bit CAN identifier.
-- NMEA 2000 uses extended CAN IDs with PGN encoded in bits 8-25.
local function extractPGN(canId)
  local pf = (canId >> 16) & 0xFF
  local ps = (canId >> 8)  & 0xFF
  local dp = (canId >> 24) & 0x01

  -- PDU1 (PF < 240): destination-specific, PGN excludes PS field
  if pf < 240 then
    return (dp << 16) | (pf << 8)
  end

  -- PDU2 (PF >= 240): broadcast, PGN includes PS field
  return (dp << 16) | (pf << 8) | ps
end

--------------------------------------------------------------------------------
-- PGN Parsers
--------------------------------------------------------------------------------

-- PGN 127257 Attitude: yaw, pitch, roll in radians × 10000 → degrees.
local function parseAttitude(data)
  local yaw   = extractInt16LE(data, 1) * 0.0001 * RAD_TO_DEG
  local pitch = extractInt16LE(data, 3) * 0.0001 * RAD_TO_DEG
  local roll  = extractInt16LE(data, 5) * 0.0001 * RAD_TO_DEG
  return {yaw, pitch, roll}
end

-- PGN 129025 Position Rapid Update: lat/lon in degrees × 1e-7.
local function parsePositionRapid(data)
  local latitude  = extractInt32LE(data, 0) * 1e-7
  local longitude = extractInt32LE(data, 4) * 1e-7
  return {latitude, longitude}
end

-- PGN 129026 COG & SOG Rapid Update: course radians → degrees, speed m/s → knots.
local function parseCogSog(data)
  local cogRef = (data[2] or 0) & 0x03
  local cog    = extractUint16LE(data, 2) * 0.0001 * RAD_TO_DEG
  local sog    = extractUint16LE(data, 4) * 0.01 * 1.94384
  return {cog, sog, cogRef}
end

-- PGN 128259 Speed (Water Referenced): m/s → knots.
local function parseSpeed(data)
  local waterSpeed  = extractUint16LE(data, 1) * 0.01 * 1.94384
  local groundSpeed = extractUint16LE(data, 3) * 0.01 * 1.94384
  return {waterSpeed, groundSpeed}
end

-- PGN 128267 Water Depth: depth and offset in metres.
local function parseWaterDepth(data)
  local depth  = extractUint32LE(data, 1) * 0.01
  local offset = extractInt16LE(data, 5) * 0.001
  return {depth, offset}
end

-- PGN 130310 Environmental Water: temperatures in Kelvin → Celsius.
local function parseEnvWater(data)
  local waterTemp   = extractUint16LE(data, 1) * 0.01 - 273.15
  local outsideTemp = extractUint16LE(data, 3) * 0.01 - 273.15
  return {waterTemp, outsideTemp}
end

-- PGN 130311 Environmental Atmospheric: pressure, air temp, humidity.
local function parseEnvAtmos(data)
  local pressure = extractUint16LE(data, 1)
  local airTemp  = extractUint16LE(data, 3) * 0.01 - 273.15
  local humidity = extractInt16LE(data, 5) * 0.004
  return {pressure, airTemp, humidity}
end

-- Maps NMEA 2000 PGNs to parser function and output indices (1-based for Lua).
local pgnToIndexMap = {
  [127257] = { indices = {1, 2, 3},     parser = parseAttitude      },
  [129025] = { indices = {4, 5},        parser = parsePositionRapid },
  [129026] = { indices = {6, 7, 8},     parser = parseCogSog        },
  [128259] = { indices = {9, 10},       parser = parseSpeed         },
  [128267] = { indices = {11, 12},      parser = parseWaterDepth    },
  [130310] = { indices = {13, 14},      parser = parseEnvWater      },
  [130311] = { indices = {15, 16, 17},  parser = parseEnvAtmos      }
}

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses an NMEA 2000 CAN frame into the predefined array structure.
-- @param frame Binary NMEA 2000 frame (byte table)
-- @return Array of values mapped according to pgnToIndexMap
function parse(frame)
  -- Need CAN ID (4) + length (1) + at least 1 data byte
  if #frame < 6 then
    return parsedValues
  end

  -- Reconstruct the 29-bit CAN ID from the first four little-endian bytes
  local canId = frame[1] | (frame[2] << 8) | (frame[3] << 16) | (frame[4] << 24)

  -- Clamp payload length to the 8-byte CAN frame maximum
  local dataLength = frame[5]
  if dataLength > 8 then dataLength = 8 end

  -- Copy the payload into a byte table addressable via the zero-based helpers
  local data = {}
  for i = 0, dataLength - 1 do
    if (6 + i) > #frame then break end
    data[i + 1] = frame[6 + i]
  end

  -- Decode the PGN and look up the matching parser
  local pgn = extractPGN(canId)
  local config = pgnToIndexMap[pgn]
  if not config then
    return parsedValues
  end

  -- Invoke the matching parser and store values at their mapped indices
  local values = config.parser(data)
  for i = 1, #values do
    local idx = config.indices[i]
    if idx then
      parsedValues[idx] = values[i]
    end
  end

  return parsedValues
end
