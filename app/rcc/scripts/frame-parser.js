/**
 * This function splits a data frame using the project-specified separator,
 * allowing you to customize frame parsing for different project requirements.
 *
 * Global variables can maintain a constant output array, enabling a single
 * Serial Studio project to display information consistently, even with varying
 * frame types.
 *
 * @param[in]  frame      The latest received frame as a string.
 * @param[in]  separator  The data separator defined in the JSON project.
 * @return     Array of strings containing the parsed frame elements.
 *
 * @note Only data within the delimiters is processed.
 * @note Declare global variables outside @c parse() for state/configuration.
 *
 * @example
 * Given frame: "value1,value2,value3", separator: ","
 * Returns: ["value1", "value2", "value3"]
 */
function parse(frame, separator) {
    return frame.split(separator);
}
