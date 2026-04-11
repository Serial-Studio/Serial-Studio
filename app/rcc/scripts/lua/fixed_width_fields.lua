--
-- Fixed-Width Fields Parser
--
-- Splits space-separated fixed-width fields into an array of values.
--
-- INPUT FORMAT: "John      25  Engineer  ABC123"
-- OUTPUT TABLE: {"John", "25", "Engineer", "ABC123"}
--

function parse(frame)
  local result = {}
  for field in frame:gmatch("%S+") do
    result[#result + 1] = field
  end
  return result
end
