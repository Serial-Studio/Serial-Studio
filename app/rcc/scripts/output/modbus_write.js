/**
 * Modbus ASCII write holding register command with LRC checksum.
 * For VFDs, power meters, temperature controllers (Modbus ASCII mode).
 */
var SLAVE_ADDR = 1;
var REGISTER   = 40001;

function transmit(value) {
  var addr = ("0" + SLAVE_ADDR.toString(16)).slice(-2).toUpperCase();
  var reg  = ("000" + REGISTER.toString(16)).slice(-4).toUpperCase();
  var val  = ("000" + Math.round(value).toString(16)).slice(-4).toUpperCase();

  // Modbus ASCII frame: :ADDR_FUNC_REG_VALUE_LRC\r\n
  var msg = addr + "06" + reg + val;

  // LRC checksum
  var lrc = 0;
  for (var i = 0; i < msg.length; i += 2)
    lrc = (lrc + parseInt(msg.substr(i, 2), 16)) & 0xFF;
  lrc = (((lrc ^ 0xFF) + 1) & 0xFF);

  return ":" + msg + ("0" + lrc.toString(16)).slice(-2).toUpperCase() + "\r\n";
}
