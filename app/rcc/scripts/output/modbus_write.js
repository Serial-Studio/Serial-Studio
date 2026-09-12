/**
 * Modbus RTU write register using built-in protocol helpers.
 * Returns binary payload: [addr_hi, addr_lo, value_hi, value_lo].
 *
 * Available helpers (every one takes an optional trailing unit, 1..247, to
 * write to that unit instead of the connection's own):
 *   modbusWriteRegister(address, value, unit)   - 16-bit holding register (4 bytes)
 *   modbusWriteRegisters(address, values, unit) - consecutive registers
 *   modbusWriteCoil(address, on, unit)          - coil ON=0xFF00 / OFF=0x0000 (4 bytes)
 *   modbusWriteFloat(address, value, unit)      - IEEE-754 float, two registers (6 bytes)
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
