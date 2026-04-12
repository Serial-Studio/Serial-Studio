--
-- UBX Protocol (u-blox) Parser
--
-- Parses UBX binary protocol messages from u-blox GNSS receivers.
--
-- UBX is a proprietary binary protocol used by u-blox GPS/GNSS modules.
-- It provides comprehensive navigation data, satellite information, and
-- receiver configuration. More efficient than NMEA with higher update rates.
--
-- INPUT FORMAT: Binary UBX message frame
-- OUTPUT ARRAY: Extracted navigation data based on message class and ID
--
-- Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
--       Frame delimiters (0xB5 0x62 sync characters) are automatically removed
--       by Serial Studio. The parser receives:
--         [class][id][len_lo][len_hi][payload...][ck_a][ck_b]
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

-- Extracts a 16-bit unsigned integer from a byte table (little-endian).
-- UBX protocol uses little-endian byte order.
local function extractUint16LE(bytes, offset)
  local b0 = bytes[offset + 1] or 0
  local b1 = bytes[offset + 2] or 0
  return b0 | (b1 << 8)
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

-- Validates the UBX Fletcher's checksum.
-- Covers class, ID, length, and payload bytes.
local function validateChecksum(msgClass, msgId, length, payload, ckA, ckB)
  local calcA, calcB = 0, 0

  calcA = (calcA + msgClass) & 0xFF;           calcB = (calcB + calcA) & 0xFF
  calcA = (calcA + msgId)    & 0xFF;           calcB = (calcB + calcA) & 0xFF
  calcA = (calcA + (length & 0xFF)) & 0xFF;    calcB = (calcB + calcA) & 0xFF
  calcA = (calcA + ((length >> 8) & 0xFF)) & 0xFF; calcB = (calcB + calcA) & 0xFF

  for i = 1, #payload do
    calcA = (calcA + payload[i]) & 0xFF
    calcB = (calcB + calcA) & 0xFF
  end

  return (calcA == ckA) and (calcB == ckB)
end

--------------------------------------------------------------------------------
-- Message Parsers
--------------------------------------------------------------------------------

-- NAV-PVT (0x01 0x07): Position, Velocity, Time solution.
-- Returns 11 values matching JS: lat, lon, hMSL, height, gSpeed, headMot,
-- velN, velE, velD, numSV, iTOW.
local function parseNavPvt(payload)
  local iTOW      = extractUint32LE(payload, 0)
  local longitude = extractInt32LE(payload, 24) * 1e-7
  local latitude  = extractInt32LE(payload, 28) * 1e-7
  local height    = extractInt32LE(payload, 32) * 0.001
  local hMSL      = extractInt32LE(payload, 36) * 0.001
  local velN      = extractInt32LE(payload, 48) * 0.001
  local velE      = extractInt32LE(payload, 52) * 0.001
  local velD      = extractInt32LE(payload, 56) * 0.001
  local gSpeed    = extractInt32LE(payload, 60) * 0.001
  local headMot   = extractInt32LE(payload, 64) * 1e-5
  local numSV     = payload[24] or 0

  return {latitude, longitude, hMSL, height, gSpeed, headMot,
          velN, velE, velD, numSV, iTOW}
end

-- NAV-SAT (0x01 0x35): Satellite information summary.
-- Returns iTOW, version, numSvs.
local function parseNavSat(payload)
  local iTOW    = extractUint32LE(payload, 0)
  local version = payload[5] or 0
  local numSvs  = payload[6] or 0
  return {iTOW, version, numSvs}
end

-- NAV-SOL (0x01 0x06): ECEF position, accuracy estimate, satellite count.
local function parseNavSol(payload)
  local iTOW  = extractUint32LE(payload, 0)
  local ecefX = extractInt32LE(payload, 12) * 0.01
  local ecefY = extractInt32LE(payload, 16) * 0.01
  local ecefZ = extractInt32LE(payload, 20) * 0.01
  local pAcc  = extractUint32LE(payload, 24) * 0.01
  local numSV = payload[48] or 0
  return {ecefX, ecefY, ecefZ, pAcc, numSV}
end

-- NAV-POSLLH (0x01 0x02): Geodetic position solution.
local function parseNavPosllh(payload)
  local lon    = extractInt32LE(payload, 4)  * 1e-7
  local lat    = extractInt32LE(payload, 8)  * 1e-7
  local height = extractInt32LE(payload, 12) * 0.001
  local hMSL   = extractInt32LE(payload, 16) * 0.001
  return {lat, lon, hMSL, height}
end

-- Maps UBX message class+ID combinations to parser function and output indices.
local messageMap = {
  ["0x01-0x07"] = { indices = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, parser = parseNavPvt    },
  ["0x01-0x35"] = { indices = {12, 13, 14},                         parser = parseNavSat    },
  ["0x01-0x06"] = { indices = {15, 16, 17, 18, 19},                 parser = parseNavSol    },
  ["0x01-0x02"] = { indices = {1, 2, 3, 4},                         parser = parseNavPosllh }
}

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses a UBX message frame into the predefined array structure.
-- @param frame The binary UBX frame (byte table, sync chars removed)
-- @return Array of values mapped according to messageMap
function parse(frame)
  -- Need at least class + id + length(2) + 2 checksum bytes
  if #frame < 6 then
    return parsedValues
  end

  -- Extract header fields
  local msgClass      = frame[1]
  local msgId         = frame[2]
  local payloadLength = frame[3] | (frame[4] << 8)

  -- Validate frame is long enough for declared payload + checksum
  if #frame < 4 + payloadLength + 2 then
    return parsedValues
  end

  -- Copy payload into a zero-based-addressable table (indices 1..N)
  local payload = {}
  for i = 1, payloadLength do
    payload[i] = frame[4 + i]
  end

  -- Extract checksum bytes
  local ckA = frame[4 + payloadLength + 1]
  local ckB = frame[4 + payloadLength + 2]

  -- Validate UBX Fletcher checksum
  if not validateChecksum(msgClass, msgId, payloadLength, payload, ckA, ckB) then
    return parsedValues
  end

  -- Build the class+ID lookup key
  local messageKey = string.format("0x%02x-0x%02x", msgClass, msgId)
  local config = messageMap[messageKey]
  if not config then
    return parsedValues
  end

  -- Invoke the matching parser and store values at their mapped indices
  local values = config.parser(payload)
  for i = 1, #values do
    local idx = config.indices[i]
    if idx then
      parsedValues[idx] = values[i]
    end
  end

  return parsedValues
end
