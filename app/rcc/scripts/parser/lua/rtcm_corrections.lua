--
-- RTCM Corrections Parser
--
-- Parses RTCM (Radio Technical Commission for Maritime Services) correction
-- messages.
--
-- RTCM is a standard protocol for transmitting differential GPS/GNSS corrections
-- to improve positioning accuracy from meter-level to centimeter-level precision.
-- Commonly used in surveying, precision agriculture, and autonomous vehicles.
--
-- INPUT FORMAT: Binary RTCM3 message frame
-- OUTPUT ARRAY: Extracted correction data and metadata
--
-- Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
--       Frame delimiters are automatically removed by Serial Studio.
--       This is a simplified parser for common RTCM3 message types.
--

--------------------------------------------------------------------------------
-- Configuration
--------------------------------------------------------------------------------

-- Total number of values in the output array.
-- This should match the number of datasets you've configured in Serial Studio.
local numItems = 16

-- Array to hold all parsed values.
-- Values persist between frames unless updated by new data.
local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

--------------------------------------------------------------------------------
-- Helper Functions (zero-based offsets, matching JS reference)
--------------------------------------------------------------------------------

-- Reads a specified number of bits from a byte table starting at a bit position.
-- RTCM uses big-endian bit ordering. Returns the read value; use the signed
-- argument to apply two's complement conversion.
local function readBits(bytes, bitPos, numBits, signed)
  local value = 0

  for i = 0, numBits - 1 do
    local byteIndex = ((bitPos + i) // 8) + 1
    local bitIndex  = 7 - ((bitPos + i) % 8)

    if byteIndex <= #bytes then
      value = (value << 1) | ((bytes[byteIndex] >> bitIndex) & 1)
    end
  end

  -- Apply two's complement for signed values
  if signed and numBits > 0 then
    local signBit = 1 << (numBits - 1)
    if (value & signBit) ~= 0 then
      value = value - (1 << numBits)
    end
  end

  return value
end

-- Counts the number of set bits in a value (population count).
-- Used to determine the number of satellites in a satellite mask.
local function countBits(value)
  local count = 0
  while value ~= 0 do
    count = count + (value & 1)
    value = value >> 1
  end
  return count
end

-- Validates the RTCM CRC-24Q checksum.
-- RTCM3 uses the CRC-24Q polynomial 0x1864CFB.
local function validateCRC24(data)
  if #data < 6 then return false end

  local crcPolynomial = 0x1864CFB
  local crc = 0

  -- Walk every byte except the 3 trailing CRC bytes
  for i = 1, #data - 3 do
    crc = crc ~ (data[i] << 16)
    for _ = 1, 8 do
      crc = crc << 1
      if (crc & 0x1000000) ~= 0 then
        crc = crc ~ crcPolynomial
      end
    end
  end

  crc = crc & 0xFFFFFF

  local receivedCRC = (data[#data - 2] << 16)
                    | (data[#data - 1] << 8)
                    | data[#data]

  return crc == receivedCRC
end

--------------------------------------------------------------------------------
-- Message Parsers (take payload + initial bit position)
--------------------------------------------------------------------------------

-- RTCM 1005 Stationary RTK Reference Station ARP.
-- Returns: {stationId, ecefX, ecefY, ecefZ}
local function parseMsg1005(data, bitPos)
  local stationId = readBits(data, bitPos, 12); bitPos = bitPos + 12
  bitPos = bitPos + 6
  local ecefX = readBits(data, bitPos, 38, true) * 0.0001; bitPos = bitPos + 38
  local ecefY = readBits(data, bitPos, 38, true) * 0.0001; bitPos = bitPos + 38
  local ecefZ = readBits(data, bitPos, 38, true) * 0.0001
  return {stationId, ecefX, ecefY, ecefZ}
end

-- RTCM 1077 GPS MSM7: station ID, epoch time, sync flag, satellite count.
local function parseMsg1077(data, bitPos)
  local stationId     = readBits(data, bitPos, 12); bitPos = bitPos + 12
  local epochTime     = readBits(data, bitPos, 30); bitPos = bitPos + 30
  local syncFlag      = readBits(data, bitPos, 1);  bitPos = bitPos + 1
  local numSatellites = countBits(readBits(data, bitPos, 64))
  return {stationId, epochTime, syncFlag, numSatellites}
end

-- RTCM 1087 GLONASS MSM7: station ID, epoch time (27-bit), sync, sat count.
local function parseMsg1087(data, bitPos)
  local stationId     = readBits(data, bitPos, 12); bitPos = bitPos + 12
  local epochTime     = readBits(data, bitPos, 27); bitPos = bitPos + 27
  local syncFlag      = readBits(data, bitPos, 1);  bitPos = bitPos + 1
  local numSatellites = countBits(readBits(data, bitPos, 64))
  return {stationId, epochTime, syncFlag, numSatellites}
end

-- Maps RTCM message types to parser function and output indices.
local messageTypeMap = {
  [1005] = { indices = {1, 2, 3, 4},      parser = parseMsg1005 },
  [1077] = { indices = {5, 6, 7, 8},      parser = parseMsg1077 },
  [1087] = { indices = {9, 10, 11, 12},   parser = parseMsg1087 }
}

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses an RTCM3 message frame into the predefined array structure.
-- @param frame Binary RTCM3 frame (byte table)
-- @return Array of values mapped according to messageTypeMap
function parse(frame)
  -- Need preamble + length(2) + msg type(2) + CRC(3)
  if #frame < 6 then
    return parsedValues
  end

  -- Verify RTCM3 preamble byte
  if frame[1] ~= 0xD3 then
    return parsedValues
  end

  -- Extract 10-bit message length
  local msgLength = ((frame[2] & 0x03) << 8) | frame[3]

  -- Validate frame is long enough for declared payload + CRC
  if #frame < 3 + msgLength + 3 then
    return parsedValues
  end

  -- Validate the CRC-24Q checksum over the whole frame
  if not validateCRC24(frame) then
    return parsedValues
  end

  -- Copy the message payload (indices 1..msgLength)
  local payload = {}
  for i = 1, msgLength do
    payload[i] = frame[3 + i]
  end

  -- Message type lives in the first 12 bits of the payload
  local messageType = readBits(payload, 0, 12)
  local config = messageTypeMap[messageType]
  if not config then
    return parsedValues
  end

  -- Invoke the matching parser and store values at their mapped indices
  local values = config.parser(payload, 12)
  for i = 1, #values do
    local idx = config.indices[i]
    if idx then
      parsedValues[idx] = values[i]
    end
  end

  return parsedValues
end
