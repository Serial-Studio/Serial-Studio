--
-- Base64-Encoded Data Parser
--
-- Parses Base64-encoded data into an array of byte values (0-255).
--
-- INPUT FORMAT: "SGVsbG8gV29ybGQ=" (Base64 string)
-- OUTPUT TABLE: {72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100}
--

----------------------------------------------------------------------
-- Helper Functions
----------------------------------------------------------------------

local b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
local b64lookup = {}
for i = 1, #b64chars do
  b64lookup[b64chars:byte(i)] = i - 1
end

local function b64decode(input)
  input = input:gsub("[%s=]+$", "")
  local result = {}
  local buffer, bits = 0, 0

  for i = 1, #input do
    local val = b64lookup[input:byte(i)]
    if val then
      buffer = (buffer << 6) | val
      bits = bits + 6
      if bits >= 8 then
        bits = bits - 8
        result[#result + 1] = (buffer >> bits) & 0xFF
      end
    end
  end

  return result
end

----------------------------------------------------------------------
-- Frame Parser Function
----------------------------------------------------------------------

function parse(frame)
  return b64decode(frame)
end
