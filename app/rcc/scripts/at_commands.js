/**
 * AT Command Response Parser
 *
 * Parses AT command responses from modems and cellular modules.
 *
 * INPUT FORMAT: "+CSQ: 25,99" or "+CREG: 0,1" or "OK"
 * OUTPUT ARRAY: [25, 99, 0, 1, ...]  (mapped to predefined indices)
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 *       Responses are mapped to array indices based on commandToIndexMap.
 *       Missing commands retain their previous values between frames.
 */

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

/**
 * Total number of values in the output array.
 * This should match the number of datasets you've configured in Serial Studio.
 * Increase this if you need more parameters or commands.
 */
const numItems = 8;

/**
 * Maps AT command names to array indices.
 *
 * Example: If you receive "+CSQ: 25,99", this means:
 *   - Command name is "CSQ" (signal quality)
 *   - First parameter (25) goes to index 0
 *   - Second parameter (99) goes to index 1
 *
 * Customize this object to match your modem's commands and desired output order.
 * Format: "COMMAND_NAME": [index_for_param1, index_for_param2, ...]
 */
const commandToIndexMap = {
  // Signal quality: rssi → index 0, ber → index 1
  "CSQ": [0, 1],
  // Network registration: stat → index 2, lac → index 3
  "CREG": [2, 3],
  // GPRS attach status: state → index 4
  "CGATT": [4],
  // Operator selection: mode → index 5, format → index 6, oper → index 7
  "COPS": [5, 6, 7]
};

/**
 * Array to hold all parsed values.
 *
 * This array always has the same size and structure, which is what Serial Studio
 * needs to populate the dashboard consistently. Each index corresponds to a
 * specific dataset in your Serial Studio project.
 *
 * The array starts with all zeros, and gets updated when AT responses arrive.
 * Values persist between frames unless updated by new data.
 */
const parsedValues = new Array(numItems).fill(0);

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses an AT command response into the predefined array structure.
 *
 * How it works:
 * 1. Receives a frame like "+CSQ: 25,99"
 * 2. Splits it into command name ("CSQ") and parameters ("25,99")
 * 3. Looks up "CSQ" in commandToIndexMap to find [0, 1]
 * 4. Places 25 at parsedValues[0] and 99 at parsedValues[1]
 * 5. Returns the complete parsedValues array
 *
 * @param {string} frame - The AT command response from the data source
 * @returns {array} Array of values mapped according to commandToIndexMap
 */
function parse(frame) {
  frame = frame.trim();

  // Only process frames that contain a command with parameters
  if (!frame.includes(':'))
    return parsedValues;

  // Split at ':' to separate command name from parameters
  const parts = frame.split(':');

  // Remove the '+' prefix and whitespace: "+CSQ" → "CSQ"
  const command = parts[0].replace('+', '').trim();

  // Split parameters by comma and convert to numbers where possible
  const params = parts[1].trim().split(',').map(p => {
    const n = parseFloat(p.trim());
    return isNaN(n) ? p.trim() : n;
  });

  if (!commandToIndexMap.hasOwnProperty(command))
    return parsedValues;

  const indices = commandToIndexMap[command];
  for (let i = 0; i < params.length && i < indices.length; i++)
    parsedValues[indices[i]] = params[i];

  return parsedValues;
}
