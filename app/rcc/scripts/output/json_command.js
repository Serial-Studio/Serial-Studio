/**
 * JSON-encoded command payload.
 * For REST-like firmware APIs, ESP32 WebSocket endpoints, Node-RED flows.
 */
function transmit(value) {
  var obj = {
    cmd: "set",
    value: value,
    ts: Date.now()
  };
  return JSON.stringify(obj) + "\n";
}
