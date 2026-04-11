--
-- NMEA 0183 Sentences Parser
--
-- Parses NMEA 0183 protocol sentences from GPS and marine navigation devices.
--
-- NMEA 0183 is a standard communication protocol used by GPS receivers, chart
-- plotters, and other marine electronics. Data is transmitted as ASCII sentences.
--
-- INPUT FORMAT: "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"
-- OUTPUT ARRAY: Extracted values based on sentence type
--
-- Note: Frame delimiters are automatically removed by Serial Studio.
--       This parser handles common NMEA sentence types.
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
-- Helper Functions
--------------------------------------------------------------------------------

-- Parses NMEA latitude format to decimal degrees.
-- Format: DDMM.MMMM (degrees and minutes).
local function parseLatitude(latStr, hemisphere)
  if not latStr or #latStr < 4 then return 0 end

  local degrees = tonumber(latStr:sub(1, 2)) or 0
  local minutes = tonumber(latStr:sub(3))    or 0
  local decimal = degrees + (minutes / 60)

  if hemisphere == "S" then decimal = -decimal end
  return decimal
end

-- Parses NMEA longitude format to decimal degrees.
-- Format: DDDMM.MMMM (degrees and minutes).
local function parseLongitude(lonStr, hemisphere)
  if not lonStr or #lonStr < 5 then return 0 end

  local degrees = tonumber(lonStr:sub(1, 3)) or 0
  local minutes = tonumber(lonStr:sub(4))    or 0
  local decimal = degrees + (minutes / 60)

  if hemisphere == "W" then decimal = -decimal end
  return decimal
end

-- Validates NMEA checksum: XOR of all characters between $ and *.
local function validateChecksum(sentence)
  local starPos = sentence:find("*", 1, true)
  if not starPos then return false end

  local data        = sentence:sub(2, starPos - 1)
  local checksumStr = sentence:sub(starPos + 1)
  local expected    = tonumber(checksumStr, 16)
  if not expected then return false end

  local calculated = 0
  for i = 1, #data do
    calculated = calculated ~ data:byte(i)
  end

  return calculated == expected
end

--------------------------------------------------------------------------------
-- Sentence Parsers (return ordered value list, indexed by `indices` below)
--------------------------------------------------------------------------------

-- $GPGGA,time,lat,N/S,lon,E/W,quality,numSV,HDOP,alt,M,sep,M,diffAge,diffStation*cs
-- Returns: {latitude, longitude, altitude, quality, numSatellites, hdop}
local function parseGGA(fields)
  local latitude      = parseLatitude(fields[3], fields[4])
  local longitude     = parseLongitude(fields[5], fields[6])
  local quality       = tonumber(fields[7])  or 0
  local numSatellites = tonumber(fields[8])  or 0
  local altitude      = tonumber(fields[10]) or 0
  local hdop          = tonumber(fields[9])  or 0
  return {latitude, longitude, altitude, quality, numSatellites, hdop}
end

-- $GPRMC,time,status,lat,N/S,lon,E/W,speed,track,date,magvar,E/W*cs
-- Returns: {latitude, longitude, speedKnots, track}
local function parseRMC(fields)
  local latitude  = parseLatitude(fields[4], fields[5])
  local longitude = parseLongitude(fields[6], fields[7])
  local speed     = tonumber(fields[8]) or 0
  local track     = tonumber(fields[9]) or 0
  return {latitude, longitude, speed, track}
end

-- $GPGLL,lat,N/S,lon,E/W,time,status*cs
-- Returns: {latitude, longitude}
local function parseGLL(fields)
  local latitude  = parseLatitude(fields[2], fields[3])
  local longitude = parseLongitude(fields[4], fields[5])
  return {latitude, longitude}
end

-- $GPVTG,track,T,track,M,speed,N,speed,K,mode*cs
-- Returns: {speedKnots, speedKmh}
local function parseVTG(fields)
  local speedKnots = tonumber(fields[6]) or 0
  local speedKmh   = tonumber(fields[8]) or 0
  return {speedKnots, speedKmh}
end

-- Maps NMEA sentence types to parser + output indices.
local sentenceTypeMap = {
  GPGGA = { indices = {1, 2, 3, 4, 5, 6}, parser = parseGGA },
  GPRMC = { indices = {1, 2, 7, 8},       parser = parseRMC },
  GPGLL = { indices = {1, 2},             parser = parseGLL },
  GPVTG = { indices = {7, 8},             parser = parseVTG }
}

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses an NMEA 0183 sentence into the predefined array structure.
-- @param frame The NMEA sentence from the data source
-- @return Array of values mapped according to sentenceTypeMap
function parse(frame)
  -- Trim whitespace
  frame = frame:match("^%s*(.-)%s*$") or ""

  -- Reject frames that don't start with '$'
  if #frame == 0 or frame:sub(1, 1) ~= "$" then
    return parsedValues
  end

  -- Validate checksum when present
  if frame:find("*", 1, true) and not validateChecksum(frame) then
    return parsedValues
  end

  -- Strip checksum suffix for field parsing
  local starPos = frame:find("*", 1, true)
  local dataStr = starPos and frame:sub(1, starPos - 1) or frame

  -- Split fields on commas (preserve empty tokens)
  local fields = {}
  local start = 1
  while true do
    local commaPos = dataStr:find(",", start, true)
    if commaPos then
      fields[#fields + 1] = dataStr:sub(start, commaPos - 1)
      start = commaPos + 1
    else
      fields[#fields + 1] = dataStr:sub(start)
      break
    end
  end

  -- Extract sentence type: "$GPGGA" → "GPGGA"
  local sentenceId = fields[1]:sub(2)
  local config = sentenceTypeMap[sentenceId]
  if not config then
    return parsedValues
  end

  -- Invoke the matching parser and store values at their mapped indices
  local values = config.parser(fields)
  for i = 1, #values do
    local idx = config.indices[i]
    if idx then
      parsedValues[idx] = values[i]
    end
  end

  return parsedValues
end
