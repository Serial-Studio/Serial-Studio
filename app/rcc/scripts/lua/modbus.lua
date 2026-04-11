--
-- Enhanced Modbus Frame Parser for Serial Studio
--
-- A robust, configurable Modbus parser with:
--   - Automatic scaling/division for fixed-point values
--   - Signed integer support
--   - 32-bit register pair support
--   - Comprehensive exception handling
--   - Debug logging option
--
-- Compatible with Serial Studio's Modbus TCP/RTU driver.
-- Requires Binary (Direct) decoder mode in the Project Editor.
--

--------------------------------------------------------------------------------
-- Configuration — Edit this section to match your device
--------------------------------------------------------------------------------

local CONFIG = {
  -- Number of output values (datasets in Serial Studio)
  numItems = 9,

  -- Register address offset (0 for most devices, 40001 for legacy Modicon)
  registerOffset = 0,

  -- Enable console debug logging
  debug = false,

  -- Register definitions — customize for your application.
  -- Properties:
  --   name:   Human-readable name (for debugging)
  --   scale:  Divisor for fixed-point (1 = no scaling)
  --   signed: true for signed 16-bit (-32768 to 32767)
  --   units:  Display units (for documentation)
  registers = {
    [0] = { name = "Emergency Stop", scale = 1,  signed = false, units = "bool" },
    [1] = { name = "Start LED",      scale = 1,  signed = false, units = "bool" },
    [2] = { name = "Temperature",    scale = 1,  signed = false, units = "\xC2\xB0F" },
    [3] = { name = "Pressure",       scale = 1,  signed = false, units = "PSI"  },
    [4] = { name = "Motor RPM",      scale = 1,  signed = false, units = "RPM"  },
    [5] = { name = "Valve Position", scale = 1,  signed = false, units = "%"    },
    [6] = { name = "Flow Rate",      scale = 10, signed = false, units = "GPM"  },
    [7] = { name = "Motor Load",     scale = 1,  signed = false, units = "%"    },
    [8] = { name = "Vibration",      scale = 10, signed = false, units = "mm/s" }
  }
}

--------------------------------------------------------------------------------
-- Modbus Standard Exception Codes
--------------------------------------------------------------------------------

local MODBUS_EXCEPTIONS = {
  [0x01] = { name = "ILLEGAL_FUNCTION",      desc = "Function code not supported"    },
  [0x02] = { name = "ILLEGAL_DATA_ADDRESS",  desc = "Register address out of range"  },
  [0x03] = { name = "ILLEGAL_DATA_VALUE",    desc = "Value out of range for register"},
  [0x04] = { name = "SERVER_DEVICE_FAILURE", desc = "Unrecoverable device error"     },
  [0x05] = { name = "ACKNOWLEDGE",           desc = "Request accepted, processing"   },
  [0x06] = { name = "SERVER_DEVICE_BUSY",    desc = "Device busy, retry later"       },
  [0x07] = { name = "NEGATIVE_ACKNOWLEDGE",  desc = "Cannot perform request"         },
  [0x08] = { name = "MEMORY_PARITY_ERROR",   desc = "Memory parity check failed"     },
  [0x0A] = { name = "GATEWAY_PATH_UNAVAIL",  desc = "Gateway path not available"     },
  [0x0B] = { name = "GATEWAY_TARGET_FAILED", desc = "Gateway target not responding"  }
}

--------------------------------------------------------------------------------
-- Modbus Standard Function Codes
--------------------------------------------------------------------------------

local MODBUS_FUNCTIONS = {
  [0x01] = { name = "Read Coils",               type = "read",  dataType = "bit"      },
  [0x02] = { name = "Read Discrete Inputs",     type = "read",  dataType = "bit"      },
  [0x03] = { name = "Read Holding Registers",   type = "read",  dataType = "register" },
  [0x04] = { name = "Read Input Registers",     type = "read",  dataType = "register" },
  [0x05] = { name = "Write Single Coil",        type = "write", dataType = "bit"      },
  [0x06] = { name = "Write Single Register",    type = "write", dataType = "register" },
  [0x0F] = { name = "Write Multiple Coils",     type = "write", dataType = "bit"      },
  [0x10] = { name = "Write Multiple Registers", type = "write", dataType = "register" },
  [0x17] = { name = "Read/Write Multiple",      type = "both",  dataType = "register" }
}

--------------------------------------------------------------------------------
-- State — Persistent storage for parsed values
--------------------------------------------------------------------------------

local parsedValues = {}
for i = 1, CONFIG.numItems do parsedValues[i] = 0 end

local lastException = nil
local frameCount = 0

--------------------------------------------------------------------------------
-- Helper Functions
--------------------------------------------------------------------------------

-- Converts an unsigned 16-bit value to signed.
local function toSigned16(value)
  if value > 0x7FFF then return value - 0x10000 end
  return value
end

-- Emits a debug log message when CONFIG.debug is enabled.
local function debugLog(message)
  if CONFIG.debug then
    print("[Modbus] " .. message)
  end
end

-- Returns the register config for the given zero-based index, with defaults.
local function getRegisterConfig(index)
  return CONFIG.registers[index]
      or { name = "Register " .. index, scale = 1, signed = false }
end

-- Applies scaling and sign conversion to a raw 16-bit register value.
local function processValue(index, rawValue)
  local config = getRegisterConfig(index)
  local value = rawValue
  if config.signed then value = toSigned16(value) end
  return value / (config.scale or 1)
end

--------------------------------------------------------------------------------
-- Main Parser Function
--------------------------------------------------------------------------------

-- Parses a Modbus response frame into an array of values.
--
-- Frame Structure:
--   Byte 1:   Slave/Unit address (1-247)
--   Byte 2:   Function code (1-127) or Exception (128-255)
--   Byte 3+:  Data payload (varies by function)
--
-- @param frame Binary Modbus ADU (byte table, no CRC)
-- @return Array of parsed values
function parse(frame)
  frameCount = frameCount + 1

  -- Reject frames too short to contain an address + function code + data
  if not frame or #frame < 3 then
    debugLog("Frame too short: " .. (frame and #frame or 0) .. " bytes")
    return parsedValues
  end

  -- Extract ADU header fields
  local slaveAddress = frame[1]
  local functionCode = frame[2]

  debugLog(string.format("Frame #%d: Slave=%d FC=0x%x Len=%d",
                          frameCount, slaveAddress, functionCode, #frame))

  ------------------------------------------------------------------------------
  -- Exception Response (FC >= 0x80)
  ------------------------------------------------------------------------------
  if functionCode >= 0x80 then
    local originalFC    = functionCode & 0x7F
    local exceptionCode = frame[3]
    local exception     = MODBUS_EXCEPTIONS[exceptionCode]
                       or { name = "UNKNOWN", desc = "Unknown exception" }

    lastException = {
      functionCode  = originalFC,
      exceptionCode = exceptionCode,
      name          = exception.name,
      description   = exception.desc
    }

    print(string.format("[Modbus Exception] FC=0x%x Error=0x%x (%s: %s)",
                         originalFC, exceptionCode, exception.name, exception.desc))

    return parsedValues
  end

  ------------------------------------------------------------------------------
  -- Read Coils (0x01) / Read Discrete Inputs (0x02)
  ------------------------------------------------------------------------------
  if functionCode == 0x01 or functionCode == 0x02 then
    local byteCount = frame[3]
    local bitIndex = 0

    debugLog("Reading " .. (byteCount * 8) .. " bits")

    for byteIdx = 0, byteCount - 1 do
      if bitIndex >= CONFIG.numItems then break end
      local dataByte = frame[4 + byteIdx]
      for bit = 0, 7 do
        if bitIndex >= CONFIG.numItems then break end
        parsedValues[bitIndex + 1] = (dataByte >> bit) & 0x01
        bitIndex = bitIndex + 1
      end
    end

  ------------------------------------------------------------------------------
  -- Read Holding Registers (0x03) / Read Input Registers (0x04)
  ------------------------------------------------------------------------------
  elseif functionCode == 0x03 or functionCode == 0x04 then
    local byteCount     = frame[3]
    local registerCount = byteCount // 2

    debugLog("Reading " .. registerCount .. " registers (" .. byteCount .. " bytes)")

    for i = 0, registerCount - 1 do
      if i >= CONFIG.numItems then break end
      local offset = 4 + (i * 2)
      if offset + 1 <= #frame then
        local rawValue = (frame[offset] << 8) | frame[offset + 1]
        parsedValues[i + 1] = processValue(i, rawValue)

        if CONFIG.debug then
          local config = getRegisterConfig(i)
          debugLog(string.format("  [%d] %s = %s %s",
                                  i, config.name,
                                  tostring(parsedValues[i + 1]),
                                  config.units or ""))
        end
      end
    end

  ------------------------------------------------------------------------------
  -- Write Single Coil (0x05) — Echo Response
  ------------------------------------------------------------------------------
  elseif functionCode == 0x05 then
    if #frame >= 6 then
      local coilAddress = (frame[3] << 8) | frame[4]
      local coilValue   = (frame[5] << 8) | frame[6]
      local index       = coilAddress - CONFIG.registerOffset

      if index >= 0 and index < CONFIG.numItems then
        parsedValues[index + 1] = (coilValue == 0xFF00) and 1 or 0
        debugLog("Write Coil [" .. index .. "] = " .. parsedValues[index + 1])
      end
    end

  ------------------------------------------------------------------------------
  -- Write Single Register (0x06) — Echo Response
  ------------------------------------------------------------------------------
  elseif functionCode == 0x06 then
    if #frame >= 6 then
      local registerAddress = (frame[3] << 8) | frame[4]
      local rawValue        = (frame[5] << 8) | frame[6]
      local index           = registerAddress - CONFIG.registerOffset

      if index >= 0 and index < CONFIG.numItems then
        parsedValues[index + 1] = processValue(index, rawValue)
        debugLog("Write Register [" .. index .. "] = " .. parsedValues[index + 1])
      end
    end

  ------------------------------------------------------------------------------
  -- Write Multiple Coils (0x0F) — Confirmation Response
  ------------------------------------------------------------------------------
  elseif functionCode == 0x0F then
    if #frame >= 6 then
      local startAddress = (frame[3] << 8) | frame[4]
      local quantity     = (frame[5] << 8) | frame[6]
      debugLog("Write Multiple Coils: addr=" .. startAddress .. " qty=" .. quantity)
    end

  ------------------------------------------------------------------------------
  -- Write Multiple Registers (0x10) — Confirmation Response
  ------------------------------------------------------------------------------
  elseif functionCode == 0x10 then
    if #frame >= 6 then
      local startAddress = (frame[3] << 8) | frame[4]
      local quantity     = (frame[5] << 8) | frame[6]
      debugLog("Write Multiple Registers: addr=" .. startAddress .. " qty=" .. quantity)
    end

  ------------------------------------------------------------------------------
  -- Unsupported Function Code
  ------------------------------------------------------------------------------
  else
    local fcInfo = MODBUS_FUNCTIONS[functionCode]
    if fcInfo then
      debugLog(string.format("Unsupported function: %s (0x%x)", fcInfo.name, functionCode))
    else
      debugLog(string.format("Unknown function code: 0x%x", functionCode))
    end
  end

  return parsedValues
end
