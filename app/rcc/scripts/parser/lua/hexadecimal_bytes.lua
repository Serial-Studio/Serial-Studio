--
-- Hexadecimal Bytes Parser
--
-- Parses a hex string (with optional spaces or delimiters) into decimal
-- byte values.
--
-- INPUT FORMAT: "48 65 6C 6C 6F" or "48656C6C6F"
-- OUTPUT TABLE: {"72", "101", "108", "108", "111"}
--

function parse(frame)
  local result = {}

  -- Strip all non-hex characters and parse pairs
  local hex = frame:gsub("[^%x]", "")
  for i = 1, #hex - 1, 2 do
    local byte = tonumber(hex:sub(i, i + 1), 16)
    if byte then
      result[#result + 1] = byte
    end
  end

  return result
end
