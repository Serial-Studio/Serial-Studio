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
 * Extracts all values for a given XML tag from the XML string.
 * Returns an array of values (supports multiple occurrences of same tag).
 *
 * Example: extractTagValue(xml, "temp")
 *   from "<data><temp>25</temp><temp>30</temp></data>"
 *   returns ["25", "30"]
 */
function extractTagValue(xml, tagName) {
  var values = [];
  var openTag = "<" + tagName + ">";
  var closeTag = "</" + tagName + ">";
  var searchPos = 0;

  while (true) {
    // Find opening tag
    var startPos = xml.indexOf(openTag, searchPos);
    if (startPos === -1) {
      break; // No more occurrences
    }

    // Find closing tag
    var endPos = xml.indexOf(closeTag, startPos);
    if (endPos === -1) {
      break; // Malformed XML, stop searching
    }

    // Extract value between tags
    var valueStart = startPos + openTag.length;
    var value = xml.substring(valueStart, endPos);
    values.push(value);

    // Continue searching after this closing tag
    searchPos = endPos + closeTag.length;
  }

  return values;
}

/**
 * Extracts value from self-closing tags or tags with attributes.
 * Example: <temperature value="25.5"/> or <temp>25.5</temp>
 */
function extractTagWithAttributes(xml, tagName) {
  var values = [];

  // Pattern for tags with attributes: <tagName attr="value" ... >content</tagName>
  // or self-closing: <tagName attr="value" ... />
  var tagPattern = new RegExp("<" + tagName + "([^>]*)>([^<]*)</" + tagName + ">", "g");
  var selfClosingPattern = new RegExp("<" + tagName + "([^/>]*)/\\s*>", "g");

  var match;

  // Try normal tags first
  while ((match = tagPattern.exec(xml)) !== null) {
    var content = match[2].trim();
    if (content.length > 0) {
      values.push(content);
    }
  }

  // Try self-closing tags with 'value' attribute
  while ((match = selfClosingPattern.exec(xml)) !== null) {
    var attributes = match[1];
    var valueMatch = attributes.match(/value\s*=\s*["']([^"']*)["']/);
    if (valueMatch) {
      values.push(valueMatch[1]);
    }
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
 * Example:
 *   Input:  "<data><temp>25.5</temp><humidity>60</humidity></data>"
 *   Output: [25.5, 60, 0, 0, ...]
 *
 * @param {string} frame - The XML data from the data source
 * @returns {array} Array of values mapped according to tagToIndexMap
 */
function parse(frame) {
  // Trim whitespace
  frame = frame.trim();

  // Skip empty frames
  if (frame.length === 0) {
    return parsedValues;
  }

  // Process each tag in the mapping
  for (var tagName in tagToIndexMap) {
    if (tagToIndexMap.hasOwnProperty(tagName)) {
      var index = tagToIndexMap[tagName];

      // Try to extract value for this tag
      var values = extractTagValue(frame, tagName);

      // If simple extraction didn't work, try with attributes
      if (values.length === 0) {
        values = extractTagWithAttributes(frame, tagName);
      }

      // Use the first value found (if any)
      if (values.length > 0) {
        var valueStr = values[0].trim();

        // Try to parse as number
        var value = parseFloat(valueStr);
        if (isNaN(value)) {
          value = valueStr;
        }

        parsedValues[index] = value;
      }
    }
  }

  return parsedValues;
}
