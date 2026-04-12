--
-- Raw Bytes Parser
--
-- Returns the byte values from a binary frame table as-is.
-- When the input is a string, returns each byte as its numeric value.
--
-- INPUT (binary): {0x48, 0x65, 0x6C, 0x6C, 0x6F}
-- OUTPUT TABLE:   {"72", "101", "108", "108", "111"}
--
-- INPUT (string): "Hello"
-- OUTPUT TABLE:   {"72", "101", "108", "108", "111"}
--

function parse(frame)
  local result = {}

  if type(frame) == "table" then
    for i = 1, #frame do
      result[i] = frame[i]
    end
  elseif type(frame) == "string" then
    for i = 1, #frame do
      result[i] = frame:byte(i)
    end
  end

  return result
end
