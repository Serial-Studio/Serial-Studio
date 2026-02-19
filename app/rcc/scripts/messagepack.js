/**
 * MessagePack Data Parser
 *
 * Parses MessagePack binary serialization format into an array of values.
 *
 * MessagePack is an efficient binary serialization format similar to JSON
 * but more compact. It's commonly used in embedded systems and IoT devices.
 *
 * INPUT FORMAT: Binary MessagePack data
 * OUTPUT ARRAY: Extracted values from the MessagePack structure
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 *       This implementation supports basic MessagePack types.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * MessagePack parsing mode.
 *
 * "array": Expects MessagePack array and returns all elements
 * "map":   Expects MessagePack map and extracts values based on keyToIndexMap
 */
const parseMode = "array";

/**
 * For "map" mode: Maps MessagePack map keys to array indices.
 * Only used when parseMode is set to "map".
 */
const keyToIndexMap = {
  "temperature": 0,
  "humidity": 1,
  "pressure": 2,
  "voltage": 3
};

/**
 * Total number of values in the output array (for "map" mode).
 */
const numItems = 8;

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------

/**
 * Decodes a MessagePack value starting at the given offset.
 * Returns an object with the decoded value and the new offset.
 */
function decodeMessagePack(data, offset) {
  if (offset >= data.length)
    return { value: null, offset: offset };

  var byte = data[offset];
  offset++;

  // Positive fixint (0x00 - 0x7f)
  if (byte <= 0x7f)
    return { value: byte, offset: offset };

  // Fixmap (0x80 - 0x8f)
  if (byte >= 0x80 && byte <= 0x8f) {
    var size = byte & 0x0f;
    var map = {};
    for (var i = 0; i < size; i++) {
      var keyResult = decodeMessagePack(data, offset);
      offset = keyResult.offset;
      var valueResult = decodeMessagePack(data, offset);
      offset = valueResult.offset;
      map[keyResult.value] = valueResult.value;
    }
    return { value: map, offset: offset };
  }

  // Fixarray (0x90 - 0x9f)
  if (byte >= 0x90 && byte <= 0x9f) {
    var size = byte & 0x0f;
    var array = [];
    for (var i = 0; i < size; i++) {
      var result = decodeMessagePack(data, offset);
      offset = result.offset;
      array.push(result.value);
    }
    return { value: array, offset: offset };
  }

  // Fixstr (0xa0 - 0xbf)
  if (byte >= 0xa0 && byte <= 0xbf) {
    var length = byte & 0x1f;
    var str = "";
    for (var i = 0; i < length && offset < data.length; i++)
      str += String.fromCharCode(data[offset++]);
    return { value: str, offset: offset };
  }

  // Negative fixint (0xe0 - 0xff)
  if (byte >= 0xe0)
    return { value: byte - 256, offset: offset };

  // Other types
  switch (byte) {
    case 0xc0:
      return { value: null, offset: offset };

    case 0xc2:
      return { value: false, offset: offset };

    case 0xc3:
      return { value: true, offset: offset };

    case 0xcc:
      return { value: data[offset], offset: offset + 1 };

    case 0xcd: {
      var value = (data[offset] << 8) | data[offset + 1];
      return { value: value, offset: offset + 2 };
    }

    case 0xce: {
      var value = (data[offset] << 24) | (data[offset + 1] << 16) |
                  (data[offset + 2] << 8) | data[offset + 3];
      return { value: value >>> 0, offset: offset + 4 };
    }

    case 0xd0: {
      var value = data[offset];
      return { value: value > 127 ? value - 256 : value, offset: offset + 1 };
    }

    case 0xd1: {
      var value = (data[offset] << 8) | data[offset + 1];
      return { value: value > 32767 ? value - 65536 : value, offset: offset + 2 };
    }

    case 0xd2: {
      var value = (data[offset] << 24) | (data[offset + 1] << 16) |
                  (data[offset + 2] << 8) | data[offset + 3];
      return { value: value, offset: offset + 4 };
    }

    case 0xca: {
      // IEEE 754 single-precision float (big-endian)
      var b0 = data[offset], b1 = data[offset + 1];
      var b2 = data[offset + 2], b3 = data[offset + 3];
      var sign     = (b0 & 0x80) ? -1 : 1;
      var exponent = ((b0 & 0x7F) << 1) | (b1 >> 7);
      var mantissa = ((b1 & 0x7F) << 16) | (b2 << 8) | b3;
      var value    = sign * (1 + mantissa / 0x800000) * Math.pow(2, exponent - 127);
      return { value: value, offset: offset + 4 };
    }

    case 0xdc: {
      var size = (data[offset] << 8) | data[offset + 1];
      offset += 2;
      var array = [];
      for (var i = 0; i < size; i++) {
        var result = decodeMessagePack(data, offset);
        offset = result.offset;
        array.push(result.value);
      }
      return { value: array, offset: offset };
    }

    case 0xde: {
      var size = (data[offset] << 8) | data[offset + 1];
      offset += 2;
      var map = {};
      for (var i = 0; i < size; i++) {
        var keyResult = decodeMessagePack(data, offset);
        offset = keyResult.offset;
        var valueResult = decodeMessagePack(data, offset);
        offset = valueResult.offset;
        map[keyResult.value] = valueResult.value;
      }
      return { value: map, offset: offset };
    }

    default:
      return { value: null, offset: offset };
  }
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a MessagePack frame into an array of values.
 *
 * How it works (array mode):
 * 1. Decodes the MessagePack data
 * 2. If it's an array, returns all elements
 * 3. If it's a single value, returns array with that value
 *
 * How it works (map mode):
 * 1. Decodes the MessagePack data (expects a map/object)
 * 2. Extracts values based on keyToIndexMap
 * 3. Places values at their mapped indices
 *
 * @param {array} frame - The binary MessagePack data from the data source (byte array)
 * @returns {array} Array of decoded values
 */
function parse(frame) {
  var result = decodeMessagePack(frame, 0);
  var decoded = result.value;

  if (parseMode === "array") {
    if (decoded instanceof Array)
      return decoded;
    return [decoded];
  }

  var parsedValues = new Array(numItems).fill(0);

  if (decoded && typeof decoded === "object" && !(decoded instanceof Array)) {
    for (var key in decoded) {
      if (decoded.hasOwnProperty(key) && keyToIndexMap.hasOwnProperty(key))
        parsedValues[keyToIndexMap[key]] = decoded[key];
    }
  }

  return parsedValues;
}
