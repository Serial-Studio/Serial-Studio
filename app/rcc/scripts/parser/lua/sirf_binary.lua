--
-- SiRF Binary Protocol Parser
--
-- Parses SiRF binary protocol messages from GPS receivers.
--
-- SiRF binary protocol is a proprietary format used by SiRFstar GPS chipsets.
-- It provides more detailed navigation data than NMEA and updates at higher
-- rates. Common in older GPS modules and some consumer GPS devices.
--
-- INPUT FORMAT: Binary SiRF message frame
-- OUTPUT ARRAY: Extracted navigation data based on message ID
--
-- Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
--       Frame delimiters (0xA0 0xA2 ... 0xB0 0xB3) are automatically removed
--       by Serial Studio. The parser receives:
--         [length_hi][length_lo][msg_id][payload...][chk_hi][chk_lo]
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

--------------------------------------------------------------------------------
-- Helper Functions (zero-based offsets, matching JS reference)
--------------------------------------------------------------------------------

-- Extracts a 16-bit unsigned integer from a byte table (big-endian).
-- SiRF protocol uses big-endian byte order.
local function extractUint16BE(bytes, offset)
  local b0 = bytes[offset + 1] or 0
  local b1 = bytes[offset + 2] or 0
  return (b0 << 8) | b1
end

-- Extracts a 16-bit signed integer from a byte table (big-endian).
local function extractInt16BE(bytes, offset)
  local v = extractUint16BE(bytes, offset)
  if v > 32767 then v = v - 65536 end
  return v
end

-- Extracts a 32-bit unsigned integer from a byte table (big-endian).
local function extractUint32BE(bytes, offset)
  local b0 = bytes[offset + 1] or 0
  local b1 = bytes[offset + 2] or 0
  local b2 = bytes[offset + 3] or 0
  local b3 = bytes[offset + 4] or 0
  return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3
end

-- Extracts a 32-bit signed integer from a byte table (big-endian).
local function extractInt32BE(bytes, offset)
  local v = extractUint32BE(bytes, offset)
  if v > 2147483647 then v = v - 4294967296 end
  return v
end

-- Validates SiRF checksum: sum of payload bytes AND 0x7FFF.
local function validateChecksum(payload, receivedChecksum)
  local sum = 0
  for i = 1, #payload do
    sum = sum + payload[i]
  end
  return (sum & 0x7FFF) == receivedChecksum
end

--------------------------------------------------------------------------------
-- Message Parsers
--------------------------------------------------------------------------------

-- Geodetic Navigation Data: position, velocity, satellite count, HDOP.
-- Returns 9 values matching JS: lat, lon, altMSL, altEllipsoid, speed,
-- course, numSats, hdop, navValid.
local function parseGeodeticNav(data)
  local navValid = extractUint16BE(data, 0)
  local navType  = extractUint16BE(data, 2)

  local latitude     = extractInt32BE(data, 23) * 1e-7
  local longitude    = extractInt32BE(data, 27) * 1e-7
  local altEllipsoid = extractInt32BE(data, 31) * 0.01
  local altMSL       = extractInt32BE(data, 35) * 0.01

  local speedOverGround  = extractInt16BE(data, 40) * 0.01
  local courseOverGround = extractUint16BE(data, 42) * 0.01

  local numSatellites = data[89] or 0
  local hdop          = (data[90] or 0) * 0.2

  return {latitude, longitude, altMSL, altEllipsoid,
          speedOverGround, courseOverGround, numSatellites, hdop, navValid}
end

-- Navigation Library Measurement Data: ECEF position and satellite count.
local function parseMeasNavData(data)
  local xPos  = extractInt32BE(data, 1)
  local yPos  = extractInt32BE(data, 5)
  local zPos  = extractInt32BE(data, 9)
  local numSV = data[29] or 0
  return {xPos, yPos, zPos, numSV}
end

-- Measured Tracker Data: GPS week, time of week, tracked channels.
local function parseMeasTracker(data)
  local weekNumber  = extractUint16BE(data, 1)
  local timeOfWeek  = extractUint32BE(data, 3) * 0.01
  local numChannels = data[8] or 0
  return {weekNumber, timeOfWeek, numChannels}
end

-- Clock Status Data: GPS week, time of week, satellites used.
local function parseClockStatus(data)
  local weekNumber = extractUint16BE(data, 1)
  local timeOfWeek = extractUint32BE(data, 3)
  local numSV      = data[8] or 0
  return {weekNumber, timeOfWeek, numSV}
end

-- Maps SiRF message IDs to parser function and output indices.
local messageIdMap = {
  [41] = { indices = {1, 2, 3, 4, 5, 6, 7, 8, 9}, parser = parseGeodeticNav },
  [2]  = { indices = {11, 12, 13, 14},             parser = parseMeasNavData },
  [4]  = { indices = {15, 16, 17},                 parser = parseMeasTracker },
  [7]  = { indices = {18, 19, 20},                 parser = parseClockStatus }
}

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses a SiRF binary message frame into the predefined array structure.
-- @param frame The binary SiRF frame (byte table, start/end markers removed)
-- @return Array of values mapped according to messageIdMap
function parse(frame)
  -- Need at least length(2) + msgId + 2 checksum bytes
  if #frame < 5 then
    return parsedValues
  end

  -- Parse 15-bit payload length from first two bytes
  local payloadLength = ((frame[1] & 0x7F) << 8) | frame[2]

  -- Validate frame is long enough for declared payload + checksum
  if #frame < 2 + payloadLength + 2 then
    return parsedValues
  end

  -- Extract full payload (message ID + data bytes)
  local payload = {}
  for i = 1, payloadLength do
    payload[i] = frame[2 + i]
  end

  -- Parse checksum as 15-bit big-endian value
  local checksumOffset = 2 + payloadLength
  local receivedChecksum =
      ((frame[checksumOffset + 1] & 0x7F) << 8) | frame[checksumOffset + 2]

  -- Validate SiRF checksum over the full payload
  if not validateChecksum(payload, receivedChecksum) then
    return parsedValues
  end

  -- Look up parser config by message ID (first byte of payload)
  local messageId = payload[1]
  local config = messageIdMap[messageId]
  if not config then
    return parsedValues
  end

  -- Slice off message ID byte before passing to the parser
  local data = {}
  for i = 2, #payload do
    data[i - 1] = payload[i]
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
