--
-- SLIP-Encoded Frame Parser
--
-- Decodes SLIP (Serial Line Internet Protocol, RFC 1055) binary data.
--
-- INPUT FORMAT: SLIP-encoded binary data (byte table)
-- OUTPUT TABLE: {decoded_byte1, decoded_byte2, ...}
--

local SLIP_END     = 0xC0
local SLIP_ESC     = 0xDB
local SLIP_ESC_END = 0xDC
local SLIP_ESC_ESC = 0xDD

function parse(frame)
  local decoded = {}
  local i = 1

  while i <= #frame do
    local byte = frame[i]
    i = i + 1

    -- Skip residual END bytes
    if byte == SLIP_END then
      goto continue
    end

    -- Handle escape sequences
    if byte == SLIP_ESC then
      if i > #frame then
        decoded[#decoded + 1] = SLIP_ESC
        goto continue
      end

      local nextByte = frame[i]
      i = i + 1

      if nextByte == SLIP_ESC_END then
        decoded[#decoded + 1] = SLIP_END
      elseif nextByte == SLIP_ESC_ESC then
        decoded[#decoded + 1] = SLIP_ESC
      else
        decoded[#decoded + 1] = SLIP_ESC
        decoded[#decoded + 1] = nextByte
      end

      goto continue
    end

    decoded[#decoded + 1] = byte

    ::continue::
  end

  return decoded
end
