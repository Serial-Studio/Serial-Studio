--
-- COBS-Encoded Frame Parser
--
-- Decodes COBS (Consistent Overhead Byte Stuffing) binary data.
--
-- INPUT FORMAT: COBS-encoded binary data (byte table)
-- OUTPUT TABLE: {decoded_byte1, decoded_byte2, ...}
--

function parse(frame)
  local decoded = {}
  local i = 1

  while i <= #frame do
    -- Overhead byte: how many non-zero bytes follow before the next zero
    local code = frame[i]
    i = i + 1

    -- A zero code byte signals the end of the frame
    if code == 0 then
      break
    end

    -- Copy (code - 1) non-zero data bytes
    for j = 1, code - 1 do
      if i > #frame then break end
      decoded[#decoded + 1] = frame[i]
      i = i + 1
    end

    -- Insert a zero unless this block reaches maximum length (0xFF)
    if code < 0xFF and i <= #frame then
      decoded[#decoded + 1] = 0
    end
  end

  return decoded
end
