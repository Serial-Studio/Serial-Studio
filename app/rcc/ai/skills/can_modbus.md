# CAN Bus and Modbus (Pro)

Both are bus-layer protocols with their own framing semantics. The
dashboard's frame parser still runs on top of whatever the bus driver
extracts, but the bus drivers do enough work that you usually don't need
a custom parser.

## CAN Bus

CAN is message-oriented: each message has an ID and 0–8 data bytes. The
driver expects you to decode IDs into named signals.

### Configuration order

1. `io.canbus.listPlugins{}` returns Qt's CAN plugin list (peakcan,
   socketcan, vectorcan, ixxatcan, tinycan, virtualcan...). Most
   embedded/development setups use socketcan on Linux, peakcan on
   Windows.
2. `io.canbus.setPluginIndex{index}` from the list above.
3. `io.canbus.listInterfaces{}` returns physical channels for the
   selected plugin (e.g. `can0`, `vcan0`).
4. `io.canbus.setInterfaceIndex{index}`.
5. `io.canbus.setBitrate{bitrate}` — common: 250000, 500000, 1000000.
6. `io.canbus.setCanFd{enabled}` if the bus uses CAN-FD.
7. `io.connect{}`.

### DBC import

For real CAN networks, import a DBC file via the Project Editor's DBC
importer (no API endpoint yet — surface this to the user). The importer
generates groups + datasets for every signal in every message.

Multiplexed signals: simple multiplexing is supported. The importer
recognises the message's `MultiplexorSwitch` selector and emits one
dataset per muxed signal titled `<name> (mux N)`. The configured
Built-In signal map extracts the selector first and only updates a
muxed signal when the selector's raw value equals its mux value, so
muxed datasets don't decode noise from other payloads sharing the same
bits — they keep their last valid value when the selector doesn't
match. Extended multiplexing (`SG_MUL_VAL_`, `SwitchAndSignal`
intermediates, value ranges) is not supported; those signals are
skipped and the post-import dialog reports how many were dropped. Tell
the user to switch the source to Lua or JavaScript and write a custom
parser if extended mux is required.

### Without a DBC

If the user has a custom protocol on CAN, you'll write a frame parser
that reads the raw `id, dlc, data[]` and extracts your fields. The CAN
driver feeds the parser one message at a time. Frame format:
`<id_hex>;<data_hex>` by default.

## Modbus

Modbus is request/response: the dashboard polls registers from one or
more slave devices on a fixed interval.

### Configuration order

1. `io.modbus.setProtocolIndex{index}` — `io.modbus.listProtocols{}`
   returns the choices: 0 = Modbus RTU (over serial), 1 = Modbus TCP.
2. **For RTU**:
   `io.modbus.setSerialPortIndex{index}` (`listSerialPorts` first),
   `io.modbus.setBaudRate{baudRate}` (`listBaudRates`),
   `io.modbus.setDataBitsIndex` / `setParityIndex` / `setStopBitsIndex`
   (each has a list*).
   `io.modbus.setSlaveAddress{address}` — the slave id.
3. **For TCP**:
   `io.modbus.setHost{host}`, `io.modbus.setPort{port}` (default 502).
4. `io.modbus.setPollInterval{ms}` — default 100ms is sane. Faster
   intervals can saturate slow RTU devices.
5. **Register groups**: a register group is a contiguous range to poll.
   Use `io.modbus.addRegisterGroup{...}` for each, with:
   - `registerType`: 0 = Coils, 1 = DiscreteInputs, 2 = HoldingRegisters,
     3 = InputRegisters
   - `startAddress` and `count`
   - `dataType`: int16, uint16, int32, uint32, float32 (each int32/float
     spans 2 holding registers)
6. `io.connect{}`.

### Modbus map import

For complex slave devices, the Project Editor has a Modbus Map Importer
that takes CSV/XML/JSON register descriptions and generates groups +
datasets. Surface this to the user when they have a vendor map file.

### Without a map

Each register group becomes one dataset entry. The frame parser sees the
formatted register values; the default parser is fine for most setups.

## Common gotchas

- **CAN bitrate mismatch**: silent failure. The bus driver doesn't error;
  you just see no frames. Verify the wire bitrate first.
- **Modbus RTU framing**: the slave address must match exactly; multiple
  slaves on one bus are not handled by Serial Studio's driver — it polls
  one slave per active source.
- **Modbus poll interval too aggressive**: cheap PLCs respond at
  ~50–100ms; faster intervals queue up, time out, and the dashboard
  reports stale data. Default 100ms is right for almost everything.
- **CAN-FD mismatch**: enabling CAN-FD on a 2.0-only bus causes the
  controller to error-frame the bus. Confirm with the user.
