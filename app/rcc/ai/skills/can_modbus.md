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

Multiplexed signals: both simple and extended multiplexing import.
The importer emits one dataset per muxed signal, titled `<name> (mux
N)` — or `<name> (mux lo-hi)`, and `<parent>=<values>` joined by `/`
when several switches gate it. The selector itself is titled `<name>
(selector)`. The generated Lua parser walks each message's signals in
decode order, reads every selector before the signals it gates, and
publishes only the signals whose gates match, so a muxed dataset
never decodes noise from another payload sharing the same bits — it
keeps its last valid value. `SG_MUL_VAL_` switch ranges and
`SwitchAndSignal` intermediates (a switch that is itself multiplexed)
are handled, so nested chains import too.

Only two cases are dropped: a switch value that does not fit a signed
64-bit integer, and a circular or dangling `SG_MUL_VAL_` chain. The
post-import dialog reports how many signals that cost. If the user
needs those, they write a custom parser by hand — the generated Lua
is a readable, editable starting point.

### Without a DBC

If the user has a custom protocol on CAN, you'll write a frame parser
that reads the raw `id, dlc, data[]` and extracts your fields. The CAN
driver feeds the parser one message at a time. Frames are published as
binary: standard = `[ID_hi, ID_lo, DLC, payload...]`, extended =
`[0x80|ID28..24, ID23..16, ID15..8, ID7..0, DLC, payload]`, zero-padded
to 11/13 bytes; parse with `decoderMethod: 3` (Binary).

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
4. `io.modbus.setPollInterval{intervalMs}` — default 100ms is sane. Faster
   intervals can saturate slow RTU devices.
5. **Register groups**: a register group is a contiguous range to poll.
   Use `io.modbus.addRegisterGroup{...}` for each, with:
   - `type`: 0 = HoldingRegisters, 1 = InputRegisters, 2 = Coils,
     3 = DiscreteInputs
   - `startAddress` and `count`
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
