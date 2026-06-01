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

function parse(frame)
  return frame:split(",")
end
