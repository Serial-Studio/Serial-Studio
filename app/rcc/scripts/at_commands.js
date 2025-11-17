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
  "CSQ": [0, 1],      // Signal quality command: param1->index 0, param2->index 1
  "CREG": [2, 3],     // Network registration: param1->index 2, param2->index 3
  "CGATT": [4],       // GPRS attach status: param1->index 4
  "COPS": [5, 6, 7]   // Operator selection: param1->index 5, param2->index 6, param3->index 7
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
  // Remove any extra spaces at the beginning or end
  frame = frame.trim();

  // Check if this is a command with parameters (contains ':')
  // Example: "+CSQ: 25,99" contains ':', but "OK" does not
  if (frame.includes(':')) {

    // Split the frame into two parts at the ':'
    // Example: "+CSQ: 25,99" becomes ["+CSQ", " 25,99"]
    let parts = frame.split(':');

    // Get the command name and remove the '+' prefix and spaces
    // Example: "+CSQ" becomes "CSQ"
    let command = parts[0].replace('+', '').trim();

    // Get the parameters part, split by commas, and convert to numbers if possible
    // Example: " 25,99" becomes ["25", "99"] then [25, 99]
    let params = parts[1].trim().split(',').map(p => parseFloat(p.trim()) || p.trim());

    // Check if we have a mapping defined for this command
    if (commandToIndexMap.hasOwnProperty(command)) {

      // Get the array of indices where parameters should go
      // Example: for "CSQ", indices = [0, 1]
      let indices = commandToIndexMap[command];

      // Loop through each parameter and place it at its mapped index
      // Example: params[0]=25 goes to parsedValues[indices[0]]=parsedValues[0]
      //          params[1]=99 goes to parsedValues[indices[1]]=parsedValues[1]
      for (let i = 0; i < params.length && i < indices.length; i++) {
        parsedValues[indices[i]] = params[i];
      }
    }
  }

  // Return the complete array (always the same size)
  return parsedValues;
}
