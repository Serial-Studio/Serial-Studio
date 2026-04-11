--
-- MessagePack Data Parser
--
-- Parses MessagePack binary format into an array of values.
--
-- INPUT FORMAT: Binary MessagePack data (byte table)
-- OUTPUT TABLE: {value1, value2, ...}
--

----------------------------------------------------------------------
-- Configuration
----------------------------------------------------------------------

local mode = "array"  -- "array" or "map"

-- For "map" mode: maps key names to output indices (1-based for Lua)
local keyToIndexMap = {
  temperature = 1,
  humidity    = 2,
  pressure    = 3,
  voltage     = 4,
}

local numItems = 8
local parsedValues = {}
for i = 1, numItems do parsedValues[i] = 0 end

----------------------------------------------------------------------
-- MessagePack Decoder
----------------------------------------------------------------------

local pos = 1
local data = {}

local function readByte()
  local b = data[pos]
  pos = pos + 1
  return b
end

local function readUint16()
  local hi = data[pos]
  local lo = data[pos + 1]
  pos = pos + 2
  return (hi << 8) | lo
end

local function readUint32()
  local b0 = data[pos]
  local b1 = data[pos + 1]
  local b2 = data[pos + 2]
  local b3 = data[pos + 3]
  pos = pos + 4
  return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3
end

local function readString(len)
  local chars = {}
  for i = 1, len do
    chars[i] = string.char(data[pos])
    pos = pos + 1
  end
  return table.concat(chars)
end

local function readFloat32()
  local b0 = data[pos]; local b1 = data[pos+1]
  local b2 = data[pos+2]; local b3 = data[pos+3]
  pos = pos + 4
  local sign = (b0 & 0x80) ~= 0 and -1 or 1
  local exp  = ((b0 & 0x7F) << 1) | (b1 >> 7)
  local mant = ((b1 & 0x7F) << 16) | (b2 << 8) | b3
  if exp == 0 then return sign * mant * 2.0^(-149) end
  if exp == 0xFF then return mant ~= 0 and (0/0) or sign * math.huge end
  return sign * (mant | 0x800000) * 2.0^(exp - 150)
end

local decode  -- forward declaration

function decode()
  if pos > #data then return nil end
  local b = readByte()

  -- Positive fixint (0x00-0x7f)
  if b <= 0x7f then return b end

  -- Negative fixint (0xe0-0xff)
  if b >= 0xe0 then return b - 256 end

  -- Fixstr (0xa0-0xbf)
  if b >= 0xa0 and b <= 0xbf then
    return readString(b & 0x1f)
  end

  -- Fixarray (0x90-0x9f)
  if b >= 0x90 and b <= 0x9f then
    local n = b & 0x0f
    local arr = {}
    for i = 1, n do arr[i] = decode() end
    return arr
  end

  -- Fixmap (0x80-0x8f)
  if b >= 0x80 and b <= 0x8f then
    local n = b & 0x0f
    local map = {}
    for _ = 1, n do
      local key = decode()
      map[key] = decode()
    end
    return map
  end

  if b == 0xc0 then return nil end       -- nil
  if b == 0xc2 then return false end      -- false
  if b == 0xc3 then return true end       -- true
  if b == 0xcc then return readByte() end -- uint8
  if b == 0xcd then return readUint16() end -- uint16
  if b == 0xce then return readUint32() end -- uint32
  if b == 0xd0 then                       -- int8
    local v = readByte()
    return v > 127 and v - 256 or v
  end
  if b == 0xd1 then                       -- int16
    local v = readUint16()
    return v > 32767 and v - 65536 or v
  end
  if b == 0xd2 then                       -- int32
    local v = readUint32()
    return v > 2147483647 and v - 4294967296 or v
  end
  if b == 0xca then return readFloat32() end -- float32
  if b == 0xd9 then return readString(readByte()) end   -- str8
  if b == 0xda then return readString(readUint16()) end  -- str16
  if b == 0xdc then                       -- array16
    local n = readUint16()
    local arr = {}
    for i = 1, n do arr[i] = decode() end
    return arr
  end
  if b == 0xde then                       -- map16
    local n = readUint16()
    local map = {}
    for _ = 1, n do
      local key = decode()
      map[key] = decode()
    end
    return map
  end

  return nil
end

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  data = frame
  pos = 1

  local decoded = decode()

  -- Array mode: return the decoded array directly (variable length)
  if mode == "array" then
    if type(decoded) == "table" then
      return decoded
    end
    return {decoded}
  end

  -- Map mode: project into a fixed-size array via keyToIndexMap
  for i = 1, numItems do parsedValues[i] = 0 end
  if type(decoded) == "table" then
    for key, idx in pairs(keyToIndexMap) do
      if decoded[key] ~= nil then
        parsedValues[idx] = decoded[key]
      end
    end
  end

  return parsedValues
end
