--
-- MAVLink Messages Parser
--
-- Parses MAVLink protocol messages (drones and robotics).
--
-- INPUT FORMAT: Binary MAVLink v1 or v2 frame (byte table)
-- OUTPUT TABLE: {value1, value2, ...}  (extracted from message payloads)
--

----------------------------------------------------------------------
-- Configuration
----------------------------------------------------------------------

local numItems = 16
local mavlinkVersion = 1

local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

----------------------------------------------------------------------
-- Helper Functions
----------------------------------------------------------------------

local function extractFloat32LE(bytes, offset)
  local b0 = bytes[offset]
  local b1 = bytes[offset + 1]
  local b2 = bytes[offset + 2]
  local b3 = bytes[offset + 3]

  local sign = (b3 & 0x80) ~= 0 and -1 or 1
  local exponent = ((b3 & 0x7F) << 1) | (b2 >> 7)
  local mantissa = ((b2 & 0x7F) << 16) | (b1 << 8) | b0

  if exponent == 0 then
    return sign * mantissa * 2.0 ^ (-149)
  end

  if exponent == 0xFF then
    return mantissa ~= 0 and (0/0) or sign * math.huge
  end

  return sign * (mantissa | 0x800000) * 2.0 ^ (exponent - 150)
end

local function extractInt16LE(bytes, offset)
  local value = bytes[offset] | (bytes[offset + 1] << 8)
  if value > 32767 then value = value - 65536 end
  return value
end

local function extractUint16LE(bytes, offset)
  return bytes[offset] | (bytes[offset + 1] << 8)
end

local function extractInt32LE(bytes, offset)
  local value = bytes[offset] | (bytes[offset + 1] << 8)
                | (bytes[offset + 2] << 16) | (bytes[offset + 3] << 24)
  if value > 2147483647 then value = value - 4294967296 end
  return value
end

----------------------------------------------------------------------
-- Message Parsers
----------------------------------------------------------------------

local messageParsers = {
  -- ATTITUDE (ID 30): roll, pitch, yaw in radians
  [30] = {
    indices = {1, 2, 3},
    parse = function(payload)
      return {
        extractFloat32LE(payload, 5),
        extractFloat32LE(payload, 9),
        extractFloat32LE(payload, 13),
      }
    end,
  },
  -- VFR_HUD (ID 74): airspeed, groundspeed, heading, throttle
  [74] = {
    indices = {4, 5, 6, 7},
    parse = function(payload)
      return {
        extractFloat32LE(payload, 1),
        extractFloat32LE(payload, 5),
        extractInt16LE(payload, 9),
        extractUint16LE(payload, 11),
      }
    end,
  },
  -- GLOBAL_POSITION_INT (ID 33): lat/lon in 1e-7 deg, alt in mm
  [33] = {
    indices = {8, 9, 10},
    parse = function(payload)
      return {
        extractInt32LE(payload, 5) / 1e7,
        extractInt32LE(payload, 9) / 1e7,
        extractInt32LE(payload, 13) / 1000,
      }
    end,
  },
}

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  if #frame < 8 then
    return parsedValues
  end

  local expectedMarker = mavlinkVersion == 2 and 0xFD or 0xFE
  if frame[1] ~= expectedMarker then
    return parsedValues
  end

  local payloadLength = frame[2]
  local messageId = frame[6]

  -- Extract payload bytes
  local payload = {}
  for i = 1, payloadLength do
    if 6 + i > #frame then break end
    payload[i] = frame[6 + i]
  end

  local config = messageParsers[messageId]
  if not config then
    return parsedValues
  end

  local values = config.parse(payload)
  for i = 1, #values do
    if i <= #config.indices then
      parsedValues[config.indices[i]] = values[i]
    end
  end

  return parsedValues
end
