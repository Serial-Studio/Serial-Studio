# Protocol Setup Guides

Step-by-step setup instructions for every communication protocol supported by Serial Studio. Each guide walks through the exact UI steps required to establish a connection. For protocol overviews and selection guidance, see [Communication Protocols](Communication-Protocols.md).

---

## Serial/UART Setup

**License:** Free

**Prerequisites:** Device connected via USB or RS-232/RS-485 adapter. Driver installed if required (CH340, FTDI, CP210x).

### Steps

1. Open Serial Studio.
2. In the **Setup Panel**, select **Serial Port** from the I/O Interface dropdown. Alternatively, click the **UART** button in the toolbar if visible.
3. Select the COM port (Windows) or `/dev/ttyUSBx` / `/dev/ttyACMx` (Linux/macOS) from the **Port** dropdown. If the port does not appear, check the Troubleshooting section below.
4. Set the **Baud Rate** to match your device. Common values: 9600, 115200, 921600.
5. Set **Data Bits** (usually 8), **Parity** (usually None), **Stop Bits** (usually 1), and **Flow Control** (usually None).
6. Optionally enable **DTR** if your device uses DTR for reset signaling (common with Arduino).
7. Optionally enable **Auto Reconnect** to automatically reconnect if the device is unplugged and re-plugged.
8. Click **Connect**.

### Troubleshooting

- **Port not listed:** On Linux, add your user to the `dialout` group: `sudo usermod -aG dialout $USER`, then log out and back in. On Windows, install the appropriate USB-serial driver. On all platforms, try a different USB cable or port.
- **No data after connecting:** Verify the baud rate matches the device exactly. Check that your frame delimiters are configured correctly. Try toggling the DTR signal.
- **Garbled data:** Baud rate mismatch is the most common cause. Double-check the device documentation.

---

## TCP Network Setup

**License:** Free

**Prerequisites:** Device and computer on the same network, or device accessible over the internet. Device IP address and port number known.

### Steps

1. In the **Setup Panel**, select **Network Socket** from the I/O Interface dropdown.
2. Set the **Socket Type** to **TCP**.
3. Enter the **Remote Address** (IP address or hostname of the device).
4. Enter the **TCP Port** number the device is listening on.
5. Click **Connect**.

Serial Studio will resolve the hostname (if provided) and attempt a TCP connection. A spinner indicates DNS lookup is in progress.

### Troubleshooting

- **Connection refused:** Verify the device is listening on the specified port. Ping the device to confirm network reachability. Check firewall rules on both ends.
- **Connection times out:** The device may be on a different subnet or behind a NAT. Verify the IP address.

---

## UDP Network Setup

**License:** Free

**Prerequisites:** Device IP address and port number known. For receiving, the device must be sending to your computer's IP.

### Steps

1. In the **Setup Panel**, select **Network Socket** from the I/O Interface dropdown.
2. Set the **Socket Type** to **UDP**.
3. Enter the **Remote Address** (IP address of the device, or multicast group address).
4. Enter the **UDP Remote Port** (port the device sends from or listens on).
5. Set the **UDP Local Port** to the port your computer should bind to for receiving data. Use `0` for automatic port assignment.
6. Optionally enable **UDP Multicast** if the remote address is a multicast group (e.g., `239.x.x.x`).
7. Click **Connect**.

### Troubleshooting

- **No data received.** Check that the device is sending to your computer's IP and the correct local port. Check firewall rules: UDP is often blocked by default. If you're using multicast, make sure both devices are on the same multicast-capable network segment.
- **Data from wrong source:** UDP is connectionless. Any device sending to your bound port will be received. Use firewall rules or application-level filtering if needed.

---

## Bluetooth LE Setup

**License:** Free

**Prerequisites:** Computer with Bluetooth 4.0+ adapter. BLE device powered on and in advertising mode (not connected to another host).

### Steps

1. In the **Setup Panel**, select **Bluetooth LE** from the I/O Interface dropdown.
2. Ensure Bluetooth is enabled on your computer. Serial Studio will automatically start scanning for BLE devices.
3. Wait for device scanning to complete. Your device should appear in the **Device** dropdown.
4. Select your device from the dropdown.
5. If the device exposes multiple GATT services, select the appropriate **Service** from the dropdown.
6. Select the **Characteristic** that carries your telemetry data (typically a notify characteristic).
7. Click **Connect**.

### Troubleshooting

- **Device not found:** Ensure the device is advertising and not already paired with another host. Move the device closer. Restart Bluetooth on your computer. On macOS, grant Bluetooth permission to Serial Studio in System Settings.
- **Connected but no data:** Verify you selected the correct service and characteristic. The characteristic must support notifications or indications. Check that the device is actively sending data.
- **Adapter not available:** On Linux, ensure `bluetoothd` is running and the user is in the `bluetooth` group.

---

## MQTT Setup (Pro)

**License:** Pro

**Prerequisites:** Access to an MQTT broker. Broker hostname, port, and credentials (if any).

### Steps

1. Click the **MQTT** button in the toolbar to open the MQTT Configuration Dialog.
2. Enter the **Broker Address** (hostname or IP). Default: `127.0.0.1`.
3. Enter the **Port**. Default: `1883` for plaintext, `8883` for TLS.
4. Enter **Username** and **Password** if the broker requires authentication.
5. Select the **MQTT Version**: 3.1, 3.1.1, or 5.0.
6. Select the **Mode**:
   - **Subscriber**: Serial Studio receives messages from the broker and routes them through the data pipeline.
   - **Publisher**: Serial Studio forwards incoming frame data from a connected device to the broker.
7. Enter the **Topic Filter** (subscriber) or **Topic** (publisher). Examples: `sensors/#`, `device/+/telemetry`, `mydevice/data`.
8. Click **Connect**.

### Optional: TLS/SSL Configuration

1. Enable **SSL** in the MQTT dialog.
2. Select the **SSL Protocol** (e.g., TLS 1.2, TLS 1.3).
3. Set the **Peer Verify Mode**: None, Query, Verify, or Auto.
4. Optionally load custom **CA Certificates** by clicking the certificate button and selecting a PEM file or directory.

### Optional: Will Message Configuration

1. Set the **Will Topic** (the topic the broker publishes to if you disconnect unexpectedly).
2. Set the **Will Message** (the payload).
3. Set **Will QoS** (0, 1, or 2) and **Will Retain** flag.

### Optional: Keep Alive and Client ID

- **Keep Alive**: Interval in seconds for PING packets. Enable **Auto Keep Alive** for automatic management.
- **Client ID**: Auto-generated. Click the regenerate button to create a new random ID.
- **Clean Session**: When enabled, the broker discards any previous session state on connect.

### Troubleshooting

- **Connection fails:** Verify broker address and port. Test with a standalone MQTT client (e.g., MQTT Explorer, `mosquitto_sub`) to confirm the broker is reachable.
- **No messages received:** Topic names are case-sensitive. Verify the topic filter matches what the device publishes to. Use `#` as the topic filter to receive all messages for debugging.
- **TLS handshake failure:** Verify the CA certificate matches the broker's certificate chain. Check that the SSL protocol version is supported by the broker.

---

## Modbus RTU Setup (Pro)

**License:** Pro

**Prerequisites:** RS-485-to-USB adapter connected to the computer. Device wired to the RS-485 bus with correct polarity (A-to-A, B-to-B) and 120-ohm termination resistors at each end.

### Steps

1. In the **Setup Panel**, select **Modbus** from the I/O Interface dropdown.
2. Set the **Protocol** to **Modbus RTU**.
3. Select the **Serial Port** corresponding to your RS-485 adapter.
4. Set the **Baud Rate** to match the device (common: 9600, 19200, 115200).
5. Set **Parity** (commonly Even for Modbus), **Data Bits** (usually 8), and **Stop Bits** (usually 1).
6. Set the **Slave Address** (device address on the bus, 1--247).
7. Add one or more **Register Groups**, each specifying:
   - **Register Type**: Coil, Discrete Input, Input Register, or Holding Register.
   - **Start Address**: The first register to read.
   - **Count**: How many consecutive registers to read.
8. Set the **Poll Interval** in milliseconds (e.g., 100 ms for 10 Hz polling).
9. Click **Connect**.

### Generate Project from Register Groups

Once you have configured your register groups, you can auto-generate a Serial Studio project file instead of building one by hand:

1. Configure all register groups as described above.
2. Click **Generate Project** in the register groups dialog.
3. You will be prompted to save the `.ssproj` file.
4. The project editor opens for further customization.

The generated project includes one group per register block, one dataset per register/coil, and a Lua frame parser. See [How Multi-Group Polling Works](#how-multi-group-polling-works) below for details on the frame parser.

### Troubleshooting

- **No response:** Verify the slave address matches the device. Check RS-485 wiring polarity. Confirm the baud rate and parity match the device settings. Ensure termination resistors are in place.
- **Timeout errors:** Increase the poll interval. Reduce the number of registers per group. Check the physical bus for noise or excessive cable length.
- **CRC errors:** Almost always a wiring issue: swapped A/B lines, missing termination, or ground loop.

---

## Modbus TCP Setup (Pro)

**License:** Pro

**Prerequisites:** Device connected to the network with a known IP address and Modbus TCP port.

### Steps

1. In the **Setup Panel**, select **Modbus** from the I/O Interface dropdown.
2. Set the **Protocol** to **Modbus TCP**.
3. Enter the **Host** (device IP address or hostname).
4. Enter the **Port** (default: 502).
5. Set the **Slave Address** (usually 1 for TCP gateways, but may vary).
6. Add one or more **Register Groups** as described in the Modbus RTU section above.
7. Set the **Poll Interval**.
8. Click **Connect**.

### Troubleshooting

- **Connection refused:** Verify the device is listening on the specified port. Check firewall rules. Confirm the IP address is correct.
- **No data:** Verify the slave address, register type, start address, and count match the device's register map documentation.

---

## Modbus Register Map Import (Pro)

**License:** Pro

Instead of manually adding register groups one by one, you can import a register map file that describes all registers for your device. Serial Studio will auto-generate a complete project with register groups, datasets, a Lua frame parser, and appropriate dashboard widgets.

### Supported Formats

Serial Studio auto-detects the format from the file extension and accepts **CSV**, **XML**, and **JSON** files.

### How to Import

1. In the **Setup Panel**, select **Modbus** from the I/O Interface dropdown.
2. Click **Import Register Map...**.
3. Select a CSV, XML, or JSON register map file.
4. Review the registers in the preview dialog.
5. Click **Create Project**. You will be prompted to save the `.ssproj` file.
6. The register groups are also loaded into the Modbus driver automatically.

### CSV Format

CSV is the most common format for Modbus register maps. Most PLC vendors, SCADA tools, and device datasheets can export or be converted to this format.

**Columns** (header row required, order is flexible):

| Column        | Aliases                                           | Required | Default    |
|---------------|---------------------------------------------------|----------|------------|
| `address`     | `addr`, `register`, `reg`                         | Yes      | None       |
| `name`        | `label`, `tag`                                    | No       | Register N |
| `type`        | `register_type`, `function`, `fc`                 | No       | holding    |
| `dataType`    | `data_type`, `format`                             | No       | uint16     |
| `units`       | `unit`, `eng_units`                               | No       | None       |
| `min`         | `minimum`, `range_min`                            | No       | 0          |
| `max`         | `maximum`, `range_max`                            | No       | 65535      |
| `scale`       | `factor`, `multiplier`                            | No       | 1.0        |
| `offset`      |                                                   | No       | 0.0        |
| `description` | `desc`, `comment` (used as name if no name column)| No       | None       |

**Register type values:** `holding` (or `0x03`, `3`, `hr`), `input` (or `0x04`, `4`, `ir`), `coil` (or `0x01`, `1`), `discrete` (or `0x02`, `2`, `di`).

**Data type values:** `uint16`, `int16`, `uint32`, `int32`, `float32`, `float64`, `bool`.

Lines starting with `#` are treated as comments and skipped.

**Example:**

```csv
address,name,type,dataType,units,min,max,scale,offset
0,Temperature,holding,uint16,°C,0,150,0.1,0
1,Pressure,holding,uint16,PSI,0,5000,1,0
2,RPM,holding,uint16,RPM,0,3000,1,0
4,Flow Rate,holding,float32,GPM,0,50,1,0
0,E-Stop,coil,bool,,0,1,,
1,Motor Run,coil,bool,,0,1,,
```

### Converting Vendor Data to CSV

Most Modbus device documentation provides register maps as tables in PDF or Excel. To convert:

1. **Identify the columns:** Look for register address, name/description, data type, and units.
2. **Map register types:** Holding registers (function code 03), input registers (04), coils (01), discrete inputs (02).
3. **Note multi-register values.** A `float32` value occupies two consecutive 16-bit registers. Set the address to the first register. Serial Studio handles the rest.
4. **Add scaling if needed:** If the device stores temperature as raw value × 0.1, add `scale,0.1` to the CSV.
5. **Save as `.csv`** with UTF-8 encoding.

**Tip:** You only need the `address` and `name` columns to get started. Everything else has sensible defaults.

### XML Format

Two layouts are supported:

**Flat (type as attribute):**

```xml
<modbus>
  <register address="0" type="holding" name="Temperature" dataType="uint16" units="°C" min="0" max="150" scale="0.1"/>
  <register address="0" type="coil" name="E-Stop"/>
</modbus>
```

**Nested (type from parent element):**

```xml
<modbus>
  <holding-registers>
    <register address="0" name="Temperature" dataType="uint16" units="°C"/>
  </holding-registers>
  <coils>
    <register address="0" name="E-Stop"/>
  </coils>
</modbus>
```

Supported parent tags: `holding-registers`, `input-registers`, `coils`, `discrete-inputs` (with or without hyphens).

### JSON Format

Two layouts are supported:

**Flat (type per register):**

```json
{
  "registers": [
    {"address": 0, "type": "holding", "name": "Temperature", "dataType": "uint16", "units": "°C", "min": 0, "max": 150, "scale": 0.1},
    {"address": 0, "type": "coil", "name": "E-Stop", "dataType": "bool"}
  ]
}
```

**Grouped (type from key):**

```json
{
  "holdingRegisters": [
    {"address": 0, "name": "Temperature", "dataType": "uint16", "units": "°C"}
  ],
  "coils": [
    {"address": 0, "name": "E-Stop"}
  ]
}
```

Supported group keys: `holdingRegisters`, `holding_registers`, `holding`, `inputRegisters`, `input_registers`, `input`, `coils`, `discreteInputs`, `discrete_inputs`, `discrete`.

### What Gets Generated

The importer produces:

- **Register groups** loaded into the Modbus driver (ready to connect immediately).
- **Project groups.** One per contiguous block of same-type registers.
- **Datasets.** One per register entry, with name, units, min/max, and widget type (LED for booleans, gauge for temperatures/pressures, bar for percentages).
- **Frame parser.** JavaScript code that extracts values from Modbus protocol frames, handles data types (uint16, int16, float32), and applies scaling/offset. See [How Multi-Group Polling Works](#how-multi-group-polling-works) for details on how the parser handles multiple register groups.

### Example Files

Below is the same hydraulic test stand register map in all three supported formats.

**CSV:**

```csv
address,name,type,dataType,units,min,max,scale,offset
0,Temperature,holding,uint16,°C,0,150,0.1,0
1,Pressure,holding,uint16,PSI,0,5000,1,0
2,RPM,holding,uint16,RPM,0,3000,1,0
3,Valve Position,holding,uint16,%,0,100,0.1,0
4,Flow Rate,holding,float32,GPM,0,50,1,0
6,Motor Load,holding,uint16,%,0,100,0.1,0
7,Vibration,holding,uint16,mm/s,0,50,0.1,0
0,Pump Inlet Temp,input,int16,°C,-40,150,0.1,0
1,Pump Outlet Temp,input,int16,°C,-40,150,0.1,0
2,Ambient Temp,input,int16,°C,-40,80,0.1,0
0,E-Stop,coil,bool,,0,1,,
1,Motor Run,coil,bool,,0,1,,
2,Valve Open,coil,bool,,0,1,,
3,Alarm Active,coil,bool,,0,1,,
0,Door Sensor,discrete,bool,,0,1,,
1,Level Switch,discrete,bool,,0,1,,
```

**JSON (flat format):**

```json
{
  "registers": [
    {"address": 0, "type": "holding", "name": "Temperature", "dataType": "uint16", "units": "°C", "min": 0, "max": 150, "scale": 0.1},
    {"address": 1, "type": "holding", "name": "Pressure", "dataType": "uint16", "units": "PSI", "min": 0, "max": 5000},
    {"address": 2, "type": "holding", "name": "RPM", "dataType": "uint16", "units": "RPM", "min": 0, "max": 3000},
    {"address": 3, "type": "holding", "name": "Valve Position", "dataType": "uint16", "units": "%", "min": 0, "max": 100, "scale": 0.1},
    {"address": 4, "type": "holding", "name": "Flow Rate", "dataType": "float32", "units": "GPM", "min": 0, "max": 50},
    {"address": 6, "type": "holding", "name": "Motor Load", "dataType": "uint16", "units": "%", "min": 0, "max": 100, "scale": 0.1},
    {"address": 7, "type": "holding", "name": "Vibration", "dataType": "uint16", "units": "mm/s", "min": 0, "max": 50, "scale": 0.1},
    {"address": 0, "type": "input", "name": "Pump Inlet Temp", "dataType": "int16", "units": "°C", "min": -40, "max": 150, "scale": 0.1},
    {"address": 1, "type": "input", "name": "Pump Outlet Temp", "dataType": "int16", "units": "°C", "min": -40, "max": 150, "scale": 0.1},
    {"address": 2, "type": "input", "name": "Ambient Temp", "dataType": "int16", "units": "°C", "min": -40, "max": 80, "scale": 0.1},
    {"address": 0, "type": "coil", "name": "E-Stop", "dataType": "bool"},
    {"address": 1, "type": "coil", "name": "Motor Run", "dataType": "bool"},
    {"address": 2, "type": "coil", "name": "Valve Open", "dataType": "bool"},
    {"address": 3, "type": "coil", "name": "Alarm Active", "dataType": "bool"},
    {"address": 0, "type": "discrete", "name": "Door Sensor", "dataType": "bool"},
    {"address": 1, "type": "discrete", "name": "Level Switch", "dataType": "bool"}
  ]
}
```

**XML:**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<modbus>
  <holding-registers>
    <register address="0" name="Temperature" dataType="uint16" units="°C" min="0" max="150" scale="0.1"/>
    <register address="1" name="Pressure" dataType="uint16" units="PSI" min="0" max="5000"/>
    <register address="2" name="RPM" dataType="uint16" units="RPM" min="0" max="3000"/>
    <register address="3" name="Valve Position" dataType="uint16" units="%" min="0" max="100" scale="0.1"/>
    <register address="4" name="Flow Rate" dataType="float32" units="GPM" min="0" max="50"/>
    <register address="6" name="Motor Load" dataType="uint16" units="%" min="0" max="100" scale="0.1"/>
    <register address="7" name="Vibration" dataType="uint16" units="mm/s" min="0" max="50" scale="0.1"/>
  </holding-registers>
  <input-registers>
    <register address="0" name="Pump Inlet Temp" dataType="int16" units="°C" min="-40" max="150" scale="0.1"/>
    <register address="1" name="Pump Outlet Temp" dataType="int16" units="°C" min="-40" max="150" scale="0.1"/>
    <register address="2" name="Ambient Temp" dataType="int16" units="°C" min="-40" max="80" scale="0.1"/>
  </input-registers>
  <coils>
    <register address="0" name="E-Stop"/>
    <register address="1" name="Motor Run"/>
    <register address="2" name="Valve Open"/>
    <register address="3" name="Alarm Active"/>
  </coils>
  <discrete-inputs>
    <register address="0" name="Door Sensor"/>
    <register address="1" name="Level Switch"/>
  </discrete-inputs>
</modbus>
```

---

## How Multi-Group Polling Works

When a Modbus connection has multiple register groups, the driver and the auto-generated frame parser coordinate through a simple sequential protocol. Understanding this mechanism helps when debugging or customizing the generated parser.

### Driver Side

Each poll cycle, the Modbus driver iterates through the configured register groups in order:

1. Poll group 0 → wait for response → emit frame
2. Poll group 1 → wait for response → emit frame
3. Poll group 2 → wait for response → emit frame
4. (next poll timer tick) → start again at group 0

The groups are always polled in the same fixed order (the order they appear in the register groups list). Each response is emitted as a separate frame to the data pipeline.

### Parser Side

The auto-generated frame parser maintains a `currentGroup` counter that starts at 0. Each time `parse(frame)` is called, the parser uses the counter to determine which register group the frame belongs to, extracts the data accordingly, then advances the counter:

```js
currentGroup = (currentGroup + 1) % numberOfGroups;
```

For a single register group, the counter is always 0: the `switch` block has one `case` and the modulo is `% 1`.

For two groups (e.g., holding registers at address 0 and coils at address 100), the generated parser looks like:

```js
var values = new Array(totalDatasets).fill(0);
var currentGroup = 0;

function parse(frame) {
  if (frame.length < 3)
    return values;

  var data = frame.slice(3);

  switch (currentGroup) {
    case 0: // Holding Registers @ 0
      values[0] = (data[0] << 8) | data[1]; // Temperature
      values[1] = (data[2] << 8) | data[3]; // Pressure
      break;
    case 1: // Coils @ 100
      values[2] = (data[0] >> 0) & 1; // E-Stop
      values[3] = (data[0] >> 1) & 1; // Motor Run
      break;
  }

  currentGroup = (currentGroup + 1) % 2;
  return values;
}
```

Frame 1 arrives → parser is in `case 0` → extracts holding register values → advances to 1.
Frame 2 arrives → parser is in `case 1` → extracts coil values → advances back to 0.

### Practical Considerations

- **Typical group count:** Most Modbus devices use 1–3 register groups. A single contiguous block of holding registers is the most common configuration.
- **Synchronization.** The parser relies on the driver always delivering frames in the same fixed order. This works reliably because each poll cycle is sequential: group N+1 isn't polled until group N's response arrives.
- **Frame loss:** If a Modbus response times out or is dropped (rare over TCP, uncommon over RTU), the parser's counter may temporarily misalign for one poll cycle. It self-corrects on the next complete cycle. At typical poll rates (100ms+), this causes at most a brief glitch.
- **Customization:** If you modify the generated parser, keep the `currentGroup` counter logic intact. Adding or removing register groups requires regenerating the parser (or manually updating the `switch` cases and modulo value).

---

## CAN Bus Setup (Pro)

**License:** Pro

**Prerequisites:** CAN-to-USB adapter (PEAK PCAN, Kvaser, CANable, SocketCAN-compatible) connected and drivers installed.

### Steps

1. In the **Setup Panel**, select **CAN Bus** from the I/O Interface dropdown.
2. Select the **Plugin** (CAN backend):
   - Linux: `socketcan`
   - Windows: `peakcan`, `vectorcan`, or `systeccan`
3. Select the **Interface** from the dropdown (e.g., `can0`, `PCAN_USBBUS1`).
4. Set the **Bitrate** to match the CAN network exactly. Common values: 125000, 250000, 500000, 1000000 bps.
5. Optionally enable **CAN FD** if the network uses Flexible Data-rate frames.
6. Click **Connect**.

### Linux SocketCAN Preparation

Before connecting in Serial Studio, bring up the CAN interface from a terminal:

```
sudo ip link set can0 type can bitrate 500000
sudo ip link set up can0
```

For CAN FD, add the data bitrate:

```
sudo ip link set can0 type can bitrate 500000 dbitrate 2000000 fd on
sudo ip link set up can0
```

### Troubleshooting

- **No messages received:** Verify the bitrate matches the network exactly. Even a slight mismatch causes all frames to be rejected. Check CAN-H/CAN-L wiring and termination (120 ohm at each bus endpoint; measure approximately 60 ohm between CAN-H and CAN-L).
- **Interface not listed:** Ensure the adapter driver is installed. On Linux, verify the interface is up with `ip link show can0`. On Windows, check Device Manager for the CAN adapter.
- **Error frames only:** Bitrate mismatch, missing termination, or bus contention. Check all physical connections.

---

## Audio Input Setup (Pro)

**License:** Pro

**Prerequisites:** Audio input device (microphone, line-in, audio interface) connected and recognized by the OS.

### Steps

1. In the **Setup Panel**, select **Audio Input** from the I/O Interface dropdown.
2. Select the **Input Device** from the dropdown (lists all detected audio capture devices).
3. Select the **Sample Rate**. Common: 44100 Hz, 48000 Hz. Available rates depend on the device.
4. Select the **Sample Format** (bit depth). Common: 16-bit integer, 32-bit float.
5. Select the **Channel Configuration** (Mono or Stereo).
6. Click **Connect**.

Audio data flows into the pipeline as PCM samples, which can be visualized with Plot and FFT Plot widgets.

### Troubleshooting

- **No audio detected:** Verify the input device is selected correctly in the dropdown. Check the OS audio settings to confirm the input is not muted and the level is adequate. On macOS, grant Microphone permission.
- **Distorted signal:** Reduce the input gain in OS audio settings. Use a line-in input instead of a microphone input when connecting sensors.
- **Wrong sample rate options:** The available sample rates are reported by the hardware. If the rate you need is not listed, try a different audio interface.

---

## Raw USB Setup (Pro)

**License:** Pro

**Prerequisites:** USB device with accessible bulk, control, or isochronous endpoints. On Linux, appropriate `udev` rules. On Windows, a WinUSB-compatible driver (installable via Zadig).

### Steps

1. In the **Setup Panel**, select **Raw USB** from the I/O Interface dropdown.
2. Pick the device from the dropdown. Devices are listed as `VID:PID, Product Name`.
3. Select the **Transfer Mode**:
   - **Bulk Stream** (default): Standard synchronous bulk IN/OUT. Works for most devices.
   - **Advanced Control**: Bulk transfers plus vendor-specific control transfers. A confirmation dialog appears before enabling.
   - **Isochronous**: Asynchronous isochronous transfers for fixed-rate streaming.
4. Select the **IN Endpoint** (for reading data from the device).
5. Select the **OUT Endpoint** (for writing data to the device), if applicable.
6. For Isochronous mode, set the **ISO Packet Size**.
7. Click **Connect**.

### Troubleshooting

- **Device not listed:** On Linux, add a `udev` rule granting your user access to the device's VID/PID, then run `sudo udevadm control --reload-rules && sudo udevadm trigger`. On Windows, use Zadig to install a WinUSB driver for the device. On macOS, the device should appear if no kernel driver has claimed it.
- **Permission denied:** On Linux, verify the `udev` rule is correct and the user is in the appropriate group. Running as root is a quick test but not recommended for production.
- **No data:** Verify the correct IN endpoint is selected. Check that the device firmware is sending data on that endpoint.

---

## HID Setup (Pro)

**License:** Pro

**Prerequisites:** USB HID-class device (gamepad, joystick, custom HID firmware, sensor) connected.

### Steps

1. In the **Setup Panel**, select **HID** from the I/O Interface dropdown.
2. Wait for device enumeration. HID devices are re-enumerated every 2 seconds automatically.
3. Pick the device from the dropdown. Devices are listed as `VID:PID, Product Name`.
4. Review the **Usage Page** and **Usage** fields to confirm the correct device function is selected.
5. Click **Connect**.

### Troubleshooting

- **Device not listed:** On Linux, add a `udev` rule for `hidraw` access: create a file in `/etc/udev/rules.d/` with `KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666"` (or a more restrictive rule targeting your VID/PID). On Windows and macOS, HID devices are generally accessible without additional configuration.
- **No data:** Verify the device is actively sending HID reports. Some HID devices only send reports when there is a state change (e.g., button press on a gamepad).

---

## Process I/O setup: Launch mode (Pro)

**License:** Pro

**Prerequisites:** A script or executable that writes data to stdout in a format Serial Studio can parse.

### Steps

1. In the **Setup Panel**, select **Process I/O** from the I/O Interface dropdown.
2. Set the **Mode** to **Launch**.
3. Enter the **Executable** path, or click the browse button to select it. Serial Studio searches standard PATH locations.
4. Enter **Arguments** (command-line arguments for the process), if any.
5. Optionally set the **Working Directory** for the process.
6. Click **Connect**.

Serial Studio spawns the process, reads its merged stdout and stderr, and feeds the output into the data pipeline. You can write to the process's stdin via the console.

### Troubleshooting

- **Process not found:** Verify the executable path is correct. On Linux/macOS, ensure the script has execute permission (`chmod +x script.py`). For Python scripts, you may need to specify `python3` as the executable and the script path as an argument.
- **No data:** Ensure the script flushes its stdout (Python: use `flush=True` in `print()` or run with `python3 -u`). Buffered output will not appear until the buffer fills.
- **Process crashes immediately:** Check the working directory and arguments. Review the console output for error messages from the process.

---

## Process I/O setup: Named Pipe mode (Pro)

**License:** Pro

**Prerequisites:** An external process writing data to a named pipe (FIFO on Linux/macOS, `\\.\pipe\Name` on Windows).

### Steps

1. In the **Setup Panel**, select **Process I/O** from the I/O Interface dropdown.
2. Set the **Mode** to **Named Pipe**.
3. Enter the **Pipe Path**, or click the browse button to select it.
   - Linux/macOS: `/tmp/myfifo` (create with `mkfifo /tmp/myfifo` beforehand).
   - Windows: `\\.\pipe\MyPipeName`.
4. Click **Connect**.

Serial Studio opens the named pipe for reading and streams data into the pipeline. The external process must open the pipe for writing.

### Troubleshooting

- **Cannot open pipe:** Verify the pipe exists before connecting. On Linux/macOS, create it with `mkfifo /path/to/pipe`. On Windows, the pipe must be created by the writing process before Serial Studio connects.
- **No data:** Ensure the external process is actively writing to the pipe. Named pipes block until both reader and writer are connected.

---

## See Also

- [Communication Protocols](Communication-Protocols.md): protocol overview and comparison.
- [MQTT Integration](MQTT-Integration.md): in-depth MQTT documentation.
- [Getting Started](Getting-Started.md): first-time setup tutorial.
- [Troubleshooting](Troubleshooting.md): general troubleshooting guide.
