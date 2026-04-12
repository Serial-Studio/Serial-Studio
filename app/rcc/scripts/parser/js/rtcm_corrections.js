/**
 * RTCM Corrections Parser
 *
 * Parses RTCM (Radio Technical Commission for Maritime Services) correction messages.
 *
 * RTCM is a standard protocol for transmitting differential GPS/GNSS corrections
 * to improve positioning accuracy from meter-level to centimeter-level precision.
 * Commonly used in surveying, precision agriculture, and autonomous vehicles.
 *
 * INPUT FORMAT: Binary RTCM3 message frame
 * OUTPUT ARRAY: Extracted correction data and metadata
 *
 * Note: This parser requires Binary (Direct) decoder mode in the Project Editor.
 *       Frame delimiters are automatically removed by Serial Studio.
 *       This is a simplified parser for common RTCM3 message types.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 */
const numItems = 16;

/**
 * Maps RTCM message types to parsing configurations.
 *
 * Common RTCM3 message types:
 *   1005: Stationary RTK Reference Station ARP
 *   1074: GPS MSM4 (Multi-Signal Message)
 *   1077: GPS MSM7 (Full measurements)
 *   1084: GLONASS MSM4
 *   1087: GLONASS MSM7
 *   1094: Galileo MSM4
 *   1097: Galileo MSM7
 *   1124: BeiDou MSM4
 *   1127: BeiDou MSM7
 *   1230: GLONASS Code-Phase Biases
 *
 * MSM (Multi-Signal Message) contains satellite observations and corrections.
 */
const messageTypeMap = {
  1005: {
    // Stationary RTK Reference Station ARP: station ID and ECEF coordinates
    indices: [0, 1, 2, 3],
    parser: function(data, bitPos) {
      var stationId = readBits(data, bitPos, 12); bitPos += 12;
      bitPos += 6;
      var ecefX = readBits(data, bitPos, 38, true) * 0.0001; bitPos += 38;
      var ecefY = readBits(data, bitPos, 38, true) * 0.0001; bitPos += 38;
      var ecefZ = readBits(data, bitPos, 38, true) * 0.0001;
      return [stationId, ecefX, ecefY, ecefZ];
    }
  },
  1077: {
    // GPS MSM7: station ID, epoch time, sync flag, satellite count
    indices: [4, 5, 6, 7],
    parser: function(data, bitPos) {
      var stationId     = readBits(data, bitPos, 12); bitPos += 12;
      var epochTime     = readBits(data, bitPos, 30); bitPos += 30;
      var syncFlag      = readBits(data, bitPos, 1);  bitPos += 1;
      var numSatellites = countBits(readBits(data, bitPos, 64));
      return [stationId, epochTime, syncFlag, numSatellites];
    }
  },
  1087: {
    // GLONASS MSM7: station ID, epoch time, sync flag, satellite count
    indices: [8, 9, 10, 11],
    parser: function(data, bitPos) {
      var stationId     = readBits(data, bitPos, 12); bitPos += 12;
      var epochTime     = readBits(data, bitPos, 27); bitPos += 27;
      var syncFlag      = readBits(data, bitPos, 1);  bitPos += 1;
      var numSatellites = countBits(readBits(data, bitPos, 64));
      return [stationId, epochTime, syncFlag, numSatellites];
    }
  }
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
 * Reads a specified number of bits from a byte array starting at a bit position.
 * RTCM uses big-endian bit ordering.
 */
function readBits(bytes, bitPos, numBits, signed) {
  var value = 0;

  for (var i = 0; i < numBits; i++) {
    var byteIndex = Math.floor((bitPos + i) / 8);
    var bitIndex  = 7 - ((bitPos + i) % 8);

    if (byteIndex < bytes.length)
      value = (value << 1) | ((bytes[byteIndex] >> bitIndex) & 1);
  }

  // Apply two's complement for signed values
  if (signed && numBits > 0) {
    var signBit = 1 << (numBits - 1);
    if (value & signBit)
      value = value - (1 << numBits);
  }

  return value;
}

/**
 * Counts the number of set bits in a value (population count).
 * Used to determine the number of satellites in a satellite mask.
 */
function countBits(value) {
  var count = 0;
  while (value) {
    count += value & 1;
    value >>= 1;
  }
  return count;
}

/**
 * Validates the RTCM CRC-24Q checksum.
 * RTCM3 uses the CRC-24Q polynomial 0x1864CFB.
 */
function validateCRC24(data) {
  if (data.length < 6)
    return false;

  var crcPolynomial = 0x1864CFB;
  var crc = 0;

  for (var i = 0; i < data.length - 3; i++) {
    crc ^= (data[i] << 16);
    for (var j = 0; j < 8; j++) {
      crc <<= 1;
      if (crc & 0x1000000)
        crc ^= crcPolynomial;
    }
  }

  crc &= 0xFFFFFF;

  var receivedCRC = (data[data.length - 3] << 16) |
                    (data[data.length - 2] << 8) |
                    data[data.length - 1];

  return crc === receivedCRC;
}

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses an RTCM3 message frame into the predefined array structure.
 *
 * RTCM3 frame structure:
 *   Byte 0:        Preamble (0xD3)
 *   Bytes 1-2:     Reserved (6 bits) + Message length (10 bits)
 *   Bytes 3-n:     Message payload
 *   Bytes n+1-n+3: CRC-24Q checksum
 *
 * Message payload structure:
 *   Bits 0-11:  Message type/number
 *   Bits 12+:   Message-specific data
 *
 * How it works:
 * 1. Validates the preamble (0xD3)
 * 2. Extracts message length
 * 3. Validates CRC-24Q checksum
 * 4. Extracts message type from payload
 * 5. Calls appropriate parser for that message type
 * 6. Stores values at their mapped indices
 *
 * @param {array} frame - The binary RTCM3 frame from the data source (byte array)
 * @returns {array} Array of values mapped according to messageTypeMap
 */
function parse(frame) {
  if (frame.length < 6)
    return parsedValues;

  if (frame[0] !== 0xD3) {
    console.error("Invalid RTCM3 preamble");
    return parsedValues;
  }

  var msgLength = ((frame[1] & 0x03) << 8) | frame[2];

  if (frame.length < 3 + msgLength + 3) {
    console.error("Incomplete RTCM3 frame");
    return parsedValues;
  }

  if (!validateCRC24(frame)) {
    console.error("RTCM3 CRC validation failed");
    return parsedValues;
  }

  var payload = [];
  for (var i = 3; i < 3 + msgLength; i++)
    payload.push(frame[i]);

  var messageType = readBits(payload, 0, 12);

  if (!messageTypeMap.hasOwnProperty(messageType))
    return parsedValues;

  var config = messageTypeMap[messageType];

  try {
    var values = config.parser(payload, 12);
    for (var i = 0; i < values.length && i < config.indices.length; i++)
      parsedValues[config.indices[i]] = values[i];
  }
  catch (e) {
    console.error("RTCM3 parsing error:", e.message);
  }

  return parsedValues;
}
