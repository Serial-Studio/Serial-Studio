--
-- Comma-Separated Values (CSV) Parser
--
-- Parses comma-separated data into an array of values.
--
-- INPUT FORMAT: "a,b,c,d,e,f"
-- OUTPUT ARRAY: {"a", "b", "c", "d", "e", "f"}
--
-- Note: Frame delimiters are automatically removed by Serial Studio.
--

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses a CSV frame into an array of values.
-- Empty fields (e.g. "a,,b") are preserved as empty strings.
function parse(frame)
  local result = {}
  local start = 1
  while true do
    local sep = frame:find(",", start, true)
    if sep then
      result[#result + 1] = frame:sub(start, sep - 1)
      start = sep + 1
    else
      result[#result + 1] = frame:sub(start)
      return result
    end
  end
end
