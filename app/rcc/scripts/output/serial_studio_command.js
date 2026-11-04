/**
 * Serial Studio device library command: sends "#ss!<control>=<value>", plus "@<token>" when the
 * app passes a reply token. The library hands the control name and value to the sketch.
 * A device declaration sets CONTROL to the declared control's name.
 */
const CONTROL = "control";

function transmit(value, token) {
  const command = "#ss!" + CONTROL + "=" + value;
  if (token === undefined || token === null)
    return command + "\n";

  return command + "@" + token + "\n";
}
