--
-- Pipe-Delimited Values Parser
--
-- Parses pipe-delimited data into an array of values.
--
-- INPUT FORMAT: "a|b|c|d|e|f"
-- OUTPUT ARRAY: {"a", "b", "c", "d", "e", "f"}
--
-- Note: Frame delimiters are automatically removed by Serial Studio.
--

--------------------------------------------------------------------------------
-- Frame Parser Function
--------------------------------------------------------------------------------

-- Parses a pipe-delimited frame into an array of values.
-- Empty fields (e.g. "a||b") are preserved as empty strings.
-- Note: "|" is a literal here; frame:split treats the separator literally, not as a Lua pattern.
function parse(frame)
  return frame:split("|")
end
