/* 
 * @brief Frame parsing function, you can modify this to suit your needs.
 * 
 * By customizing this code, you can use a single JSON project file to 
 * process different kinds of frames that are sent by the microcontroller 
 * or any data source that is connected to Serial Studio. Frame parsing code
 * is specific to every JSON project that you create.
 *
 * @param frame      string with the latest received frame.
 * @param separator  data separator sequence defined by the JSON project.
 *
 * @note.            only data that is *inside* the data delimiters will
 *                   be processed by the frame parser.
 *
 * @note             you can safely declare global variables outside the
 *                   @c parse() function.
 */
function parse(frame, separator) {
    return frame.split(separator);
}