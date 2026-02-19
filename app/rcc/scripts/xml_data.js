/**
 * XML Data Parser
 *
 * Parses XML-formatted data into an array of values.
 *
 * XML (eXtensible Markup Language) is a widely-used markup language for
 * structured data exchange. This parser extracts values from XML elements
 * based on tag names.
 *
 * INPUT FORMAT: "<root><temp>25.5</temp><humidity>60</humidity></root>"
 * OUTPUT ARRAY: [25.5, 60, ...]  (mapped to predefined indices)
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       This is a lightweight XML parser suitable for simple structures.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 8;

/**
 * Maps XML tag names to array indices.
 *
 * Example: If XML contains <temperature>25.5</temperature>,
 * the value 25.5 will be placed at index 0.
 *
 * Customize this object to match your XML tag names and desired output order.
 * Format: "tag_name": index_in_output_array
 */
const tagToIndexMap = {
  "temperature": 0,
  "humidity": 1,
  "pressure": 2,
  "voltage": 3,
  "current": 4,
  "altitude": 5,
  "speed": 6
};

/**
 * Array to hold all parsed values.
 * Values persist between frames unless updated by new data.
 */
const parsedValues = new Array(numItems).fill(0);

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------

/**
 * Extracts all text content for a given XML tag name.
 * Returns an array of values (supports multiple occurrences of the same tag).
 *
 * Example: extractTagValue(xml, "temp")
 *   from "<data><temp>25</temp><temp>30</temp></data>"
 *   returns ["25", "30"]
 */
function extractTagValue(xml, tagName) {
  var values   = [];
  var openTag  = "<" + tagName + ">";
  var closeTag = "</" + tagName + ">";
  var searchPos = 0;

  while (true) {
    var startPos = xml.indexOf(openTag, searchPos);
    if (startPos === -1)
      break;

    var endPos = xml.indexOf(closeTag, startPos);
    if (endPos === -1)
      break;

    values.push(xml.substring(startPos + openTag.length, endPos));
    searchPos = endPos + closeTag.length;
  }

  return values;
}

/**
 * Extracts values from tags with attributes or self-closing tags.
 * Handles: <tag attr="...">content</tag> and <tag value="..." />
 */
function extractTagWithAttributes(xml, tagName) {
  var values = [];

  var tagPattern        = new RegExp("<" + tagName + "([^>]*)>([^<]*)</" + tagName + ">", "g");
  var selfClosingPattern = new RegExp("<" + tagName + "([^/>]*)/\\s*>", "g");

  var match;

  while ((match = tagPattern.exec(xml)) !== null) {
    var content = match[2].trim();
    if (content.length > 0)
      values.push(content);
  }

  while ((match = selfClosingPattern.exec(xml)) !== null) {
    var valueMatch = match[1].match(/value\s*=\s*["']([^"']*)["']/);
    if (valueMatch)
      values.push(valueMatch[1]);
  }

  return values;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses an XML frame into the predefined array structure.
 *
 * Supported XML formats:
 *   - Simple tags: <temp>25.5</temp>
 *   - Nested structures: <root><sensors><temp>25.5</temp></sensors></root>
 *   - Multiple values: First occurrence is used
 *   - Whitespace: Automatically trimmed from values
 *
 * How it works:
 * 1. For each tag name in tagToIndexMap
 * 2. Search for that tag in the XML string
 * 3. Extract the value between opening and closing tags
 * 4. Convert to number if possible
 * 5. Store at the mapped index
 *
 * @param {string} frame - The XML data from the data source
 * @returns {array} Array of values mapped according to tagToIndexMap
 */
function parse(frame) {
  frame = frame.trim();

  if (frame.length === 0)
    return parsedValues;

  for (var tagName in tagToIndexMap) {
    if (!tagToIndexMap.hasOwnProperty(tagName))
      continue;

    var index = tagToIndexMap[tagName];
    var values = extractTagValue(frame, tagName);

    if (values.length === 0)
      values = extractTagWithAttributes(frame, tagName);

    if (values.length === 0)
      continue;

    var valueStr = values[0].trim();
    var value    = parseFloat(valueStr);
    parsedValues[index] = isNaN(value) ? valueStr : value;
  }

  return parsedValues;
}
