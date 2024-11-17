/**
 * Splits a data frame into an array of elements using a comma separator.
 *
 * Use this function to break a string (like "value1,value2,value3") into
 * individual pieces, which can then be displayed or processed in your project.
 *
 * @param[in]  frame   A string containing the data frame.
 *                     Example: "value1,value2,value3"
 * @return     An array of strings with the split elements.
 *             Example: ["value1", "value2", "value3"]
 *
 * @note You can declare global variables outside this function if needed
 *       for storing settings or keeping state between calls.
 */
function parse(frame) {
    return frame.split(',');
}
