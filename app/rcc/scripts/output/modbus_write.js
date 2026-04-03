/**
 * Modbus RTU write register using built-in protocol helpers.
 * Returns binary payload: [addr_hi, addr_lo, value_hi, value_lo].
 *
 * Available helpers:
 *   modbusWriteRegister(address, value)  - 16-bit holding register (4 bytes)
 *   modbusWriteCoil(address, on)         - coil ON=0xFF00 / OFF=0x0000 (4 bytes)
 *   modbusWriteFloat(address, value)     - IEEE-754 float, two registers (6 bytes)
 *
 * Note: These produce raw binary payloads suitable for Modbus RTU.
 * For Modbus ASCII framing (:ADDR_FUNC_REG_VAL_LRC\r\n), frame manually.
 */
var REGISTER = 0x0001;

function transmit(value) {
  // Write a 16-bit integer to a holding register
  return modbusWriteRegister(REGISTER, value);

  // Alternative: write a coil (ideal for toggle widgets)
  // return modbusWriteCoil(REGISTER, value);

  // Alternative: write a float across two registers
  // return modbusWriteFloat(REGISTER, value);
}
