# Protocol Setup Guides

An Arduino running `Serial.println("1,2,3")` at 115200 baud is the shortest path: select **UART/COM**, pick the port, set **Baud Rate** to 115200 and click **Connect** in the toolbar. The console then shows the raw text, and [Getting Started](Getting-Started.md#your-first-connection-quick-plot-mode) turns it into plots. Quick Plot mode needs no project: it expects comma-separated values ending in a newline, which is what `Serial.println("1,2,3")` sends.

The **Setup Panel** is the **Device Setup** panel on the right edge of the window, where you pick a driver and set its connection fields. Project-level settings, such as the MQTT Publisher, live in the Project Editor.

UART, Network and Bluetooth LE are free; the rest need Pro. OPC UA, EtherNet/IP, S7 and IEC 104 have their own pages: [OPC UA](Drivers-OPC-UA.md), [EtherNet/IP](Drivers-EtherNet-IP.md), [S7](Drivers-S7.md), [IEC 104](Drivers-IEC-104.md). For protocol selection, see [Communication Protocols](Communication-Protocols.md); for the driver model, see [Devices](Devices.md).

Sections marked Pro need a Pro license. The first time you use a Pro feature, Serial Studio offers a trial with a Yes/No prompt (internet connection required); see [Pro vs Free](Pro-vs-Free.md).

## Serial/UART Setup

**License:** Free

**Prerequisites:** Device connected via USB or RS-232/RS-485 adapter. Driver installed if required (CH340, FTDI, CP210x).

### Steps

1. In the **Setup Panel**, select **UART/COM** from the Connection dropdown.
2. Select the COM port (Windows), `/dev/ttyUSBx` / `/dev/ttyACMx` (Linux), or `/dev/cu.usbserial-*` (macOS) from the **COM Port** dropdown.
3. Set the **Baud Rate** to match your device. Default: 9600. Common values: 9600, 115200, 921600. The field is editable, so rates not in the list can be typed directly.
4. **Data Bits** defaults to 8, **Parity** to None, **Stop Bits** to 1, and **Flow Control** to None; change any of these to match your device.
5. Leave **Send DTR Signal** enabled (the default) if your device uses DTR, the line Arduino boards use to reset on connect, for reset signaling; disable it if a DTR pulse resets your device unexpectedly.
6. Optionally enable **Auto Reconnect** (off by default) to reconnect when the device is unplugged and re-plugged.
7. Click **Connect**.

### Troubleshooting

- **Port not listed:** On Linux, see [Serial port permissions](Linux-Installation.md#serial-port-permissions). On Windows, install the appropriate USB-serial driver. On all platforms, try a different USB cable or port.
- **No data after connecting:** Confirm the baud rate matches `Serial.begin()` on the device. If text shows in the console but nothing plots, Serial Studio did not see a newline-terminated line of comma-separated values; see [Frame Parser](Frame-Parser.md). Try toggling **Send DTR Signal**.
- **Garbled data:** Garbled characters are the sound of two devices speaking at different speeds. Double-check the baud rate in the device documentation.

Full driver reference: [UART](Drivers-UART.md).

## TCP Network Setup

**License:** Free

**Prerequisites:** Device and computer on the same network, or device accessible over the internet. Device IP address and port number known.

### Steps

1. In the **Setup Panel**, select **Network** from the Connection dropdown.
2. Set the **Socket Type** to **TCP**.
3. Enter the **Remote Address** (IP address or hostname of the device). Default: `127.0.0.1`.
4. Enter the **Remote Port** the device is listening on. Default: 23, a placeholder; set the port your device listens on.
5. Click **Connect**.

Serial Studio resolves the hostname when it dials. An unresolvable host shows up as a connection error, not a disabled **Connect** button.

Serial Studio is a TCP client only: it dials out to a listening server and never accepts inbound connections. If the device expects to push data to a listener, run a small TCP relay in front of Serial Studio, or switch to UDP.

### Troubleshooting

- **Connection refused:** Verify the device is listening on the specified port. Ping the device to confirm network reachability. Check firewall rules on both ends.
- **Connection times out:** The device may be on a different subnet or behind a NAT. Verify the IP address.

Full driver reference: [Network](Drivers-Network.md).

## UDP Network Setup

**License:** Free

**Prerequisites:** Device IP address and port number known. For receiving, the device must be sending to your computer's IP.

### Steps

1. In the **Setup Panel**, select **Network** from the Connection dropdown.
2. Set the **Socket Type** to **UDP**.
3. Enter the **Remote Address** (IP address of the device, or multicast group address). Default: `127.0.0.1`.
4. Enter the **Remote Port** (the destination port when Serial Studio writes data back). Default: 53, a placeholder; set the port your device receives on. This field is hidden while **Multicast** is checked.
5. Set the **Local Port** to the port your computer should bind to for receiving data. Use `0` (the default) for automatic port assignment; bind a fixed port when the device sends to a known destination.
6. Optionally enable **Multicast** if the remote address is a multicast group (e.g., `239.x.x.x`). Serial Studio joins the group on connect.
7. Click **Connect**.

### Troubleshooting

- **No data received:** Check that the device is sending to your computer's IP and the correct local port. Check firewall rules: UDP is often blocked by default. If you're using multicast, verify the device and this computer are on the same multicast-capable network segment.
- **Data from wrong source:** UDP is connectionless. Any device sending to your bound port will be received. Use firewall rules or application-level filtering if needed.

Full driver reference: [Network](Drivers-Network.md).

## Bluetooth LE Setup

**License:** Free

**Prerequisites:** Computer with Bluetooth 4.0+ adapter. BLE device powered on and in advertising mode (not connected to another host).

### Steps

1. In the **Setup Panel**, select **Bluetooth LE** from the Connection dropdown.
2. Ensure Bluetooth is enabled on your computer. Serial Studio starts scanning for BLE devices as soon as the driver is selected.
3. Your device appears in the **Device** dropdown within a few seconds; if it does not, see Troubleshooting below.
4. Select your device from the dropdown.
5. If the device exposes multiple GATT services, select the appropriate **Service** from the dropdown.
6. Select the **Characteristic** that carries your telemetry data (typically a notify characteristic, one that pushes new values to Serial Studio). Your firmware's documentation or a BLE scanner app lists which characteristic notifies; the one that sends your readings is the one to pick.
7. Click **Connect**.

### Troubleshooting

- **Device not found:** Ensure the device is advertising and not already paired with another host. Move the device closer. Restart Bluetooth on your computer. On macOS, grant Bluetooth permission to Serial Studio in System Settings.
- **Connected but no data:** Verify you selected the correct service and characteristic. The characteristic must support notifications or indications. Check that the device is actively sending data.
- **Adapter not available:** On Linux, ensure `bluetoothd` is running and the user is in the `bluetooth` group.

Full driver reference: [Bluetooth LE](Drivers-Bluetooth-LE.md).

## MQTT Setup (Pro)

**License:** Pro

**Prerequisites:** Access to an MQTT broker. Broker hostname, port, and credentials (if any). Topic filter (subscriber) or base topic (publisher).

Your sensors already publish to a broker, or you want Serial Studio to publish to one. MQTT has two surfaces in Serial Studio; both can run in the same project.

- **Subscriber (per-source driver).** Treats a broker topic as if it were a serial or network device. Configured under **Devices** in the Project Editor, exactly like UART or Network. Use this when the data already lives on a broker.
- **Publisher (per-project).** Pushes parsed frames, raw bytes, or notifications out to a broker. Configured under the **MQTT Publisher** node in the Project Editor (directly below Devices). Use this when Serial Studio is the producer and downstream tools subscribe.

### Subscriber: add an MQTT source

For a single-device setup, select **MQTT Subscriber** from the Connection dropdown in the **Setup Panel** and fill in the same fields there. For multi-device projects:

1. Open the Project Editor.
2. Add a new source under **Devices** (or pick an existing one) and set its **Bus Type** to **MQTT Subscriber**.
3. Under **Connection Settings**, enter:
   - **Hostname** and **Port** (`1883` plaintext, `8883` TLS; default `1883`).
   - **Username** / **Password** if the broker requires authentication.
   - **Client ID** (auto-generated; click **Regenerate** to pick a new one).
   - **Topic Filter**. Examples: `sensors/#`, `device/+/telemetry`, `mydevice/data`. Connecting is blocked while this is empty, unless Sparkplug is enabled.
   - **Version** (MQTT 3.1, 3.1.1, or 5.0).
   - **Clean Session** on (default) for interactive use.
4. To use TLS, enable **Use SSL/TLS** and pick an **SSL Protocol** and **Peer Verify** mode. For a private CA, set **CA Certificates** to **Load From Folder…** and pick the directory holding the PEM files.
5. Configure frame detection on the source. Each MQTT message preserves payload boundaries, so set the source's frame detection method to **No Delimiters**: each MQTT message becomes one frame.
6. Save the project and click **Connect** in the toolbar.

Repeat with another source set to **MQTT Subscriber** to add a second broker connection (or a second topic on the same broker). Each MQTT source has its own broker session, credentials, and TLS configuration.

See [MQTT Subscriber](Drivers-MQTT.md) for the full protocol primer.

### Publisher: configure the project-level publisher

1. Open the Project Editor and select **MQTT Publisher** in the left tree.
2. Under **Publishing**, turn on **Enable Publishing**.
3. Pick **Payload**:
   - **Dashboard Data (JSON)** publishes each parsed frame as compact JSON.
   - **Dashboard Data (CSV)** publishes each parsed frame as a CSV row.
   - **Raw RX Data** republishes the bytes that arrived on any active driver, unmodified.
   - **Custom Script** publishes the value returned by the `mqtt(frame)` script hook.
4. Set **Publish Rate (Hz)**, how often the publisher emits (1-30 Hz, default 10).
5. Set **Topic Base**. The Publisher silently produces no traffic when this is empty.
6. Turn on **Publish Notifications** to mirror dashboard events to MQTT; set **Notification Topic** if you want them on a separate topic from the frame stream (it defaults to the Topic Base when empty).
7. Under **Broker**, fill in hostname, port, credentials, protocol version, and keep-alive.
8. For TLS, enable **Use SSL/TLS** and pick a protocol, peer-verify mode, and verify depth. Use **Load CA Certs** in the header bar to add PEM certificates for a private CA.
9. Click **Test Connection** in the header bar to probe the broker without disturbing the live session. The result appears in a message box.
10. Click **Connect** in the header bar to open the publishing session.

See [MQTT Publisher](MQTT-Publisher.md) for the full reference, including the `mqtt(frame)` script hook for frame parsers.

### Troubleshooting

- **Connection fails:** Verify broker address and port. Test with a standalone MQTT client (e.g., MQTT Explorer, `mosquitto_sub`) to confirm the broker is reachable. On the Publisher form, **Test Connection** does the same probe inside Serial Studio.
- **Subscribed but no messages:** Topic names are case-sensitive. Verify the topic filter matches what the device publishes to. Use `#` as the topic filter to receive all messages for debugging.
- **TLS handshake failure:** Verify the CA certificate matches the broker's certificate chain. After loading a new CA, disconnect and reconnect (or run **Test Connection**) to apply it.
- **Publisher and a subscriber loop on the same topic:** Publishing to a topic the project also subscribes to creates an echo. Use different base topics, or disable one side.

## Modbus RTU Setup (Pro)

**License:** Pro

**Prerequisites:** RS-485-to-USB adapter connected to the computer. Device wired to the RS-485 bus with correct polarity (A-to-A, B-to-B) and 120-ohm termination resistors at each end.

### Steps

1. In the **Setup Panel**, select **Modbus** from the Connection dropdown.
2. Set the **Protocol** to **Modbus RTU**.
3. Select the **Serial Port** corresponding to your RS-485 adapter.
4. Set the **Baud Rate** to match the device (common: 9600, 19200, 115200).
5. Set **Parity** (commonly Even for Modbus), **Data Bits** (usually 8), and **Stop Bits** (usually 1).
6. Set the **Slave Address** (device address on the bus, 1-247; default 1).
7. Click **Configure Register Groups…** and add one or more groups, each specifying:
   - **Register Type**: Holding Registers (0x03, 16-bit read/write values at numbered addresses), Input Registers (0x04, 16-bit read-only values), Coils (0x01, single read/write bits), or Discrete Inputs (0x02, single read-only bits).
   - **Start Address**: The first register to read (0-65535).
   - **Register Count**: How many consecutive registers to read (1-125).
   - **Slave Address**: optional per-group unit (1-247). Leave it empty to use the Slave Address from the Setup Panel, which is how you poll several units on one bus.
8. Set the **Poll Interval (ms)**. Default: 100 ms, which polls at 10 Hz; the range is 50-60000 ms and values outside it are clamped.
9. Click **Connect**.

### Generate Project from Register Groups

Once you have configured your register groups, you can auto-generate a Serial Studio project file instead of building one by hand:

1. In the Setup Panel with **Modbus** selected, click **Configure Register Groups…** and add your groups.
2. Click **Generate Project** in the **Modbus Register Groups** dialog.
3. Serial Studio asks you to save the `.ssproj` file.
4. The Project Editor opens for further customization.

The generated project includes one group per register block, one dataset per register/coil, and a generated Lua frame parser. See [How Multi-Group Polling Works](#how-multi-group-polling-works) for details on the frame parser.

### Troubleshooting

- **No response:** Verify the slave address matches the device. Check RS-485 wiring polarity. Confirm the baud rate and parity match the device settings. Ensure termination resistors are in place.
- **Timeout errors:** Increase the poll interval. Reduce the number of registers per group. Check the physical bus for noise or excessive cable length.
- **CRC errors:** Almost always a wiring issue: swapped A/B lines, missing termination, or ground loop.

Full driver reference: [Modbus](Drivers-Modbus.md).

## Modbus TCP Setup (Pro)

**License:** Pro

**Prerequisites:** Device connected to the network with a known IP address and Modbus TCP port.

### Steps

1. In the **Setup Panel**, select **Modbus** from the Connection dropdown.
2. Set the **Protocol** to **Modbus TCP**.
3. Enter the **Host** (device IP address or hostname).
4. Enter the **Port** (Serial Studio prefills 5020; standard Modbus TCP devices listen on 502).
5. Set the **Slave Address** (usually 1 for TCP gateways, but may vary).
6. Click **Configure Register Groups…** and add one or more groups: **Register Type** (Holding, Input, Coils or Discrete Inputs), **Start Address** (0-65535), **Register Count** (1-125) and an optional per-group **Slave Address**.
7. Set the **Poll Interval (ms)**. Default: 100 ms; the range is 50-60000 ms and values outside it are clamped.
8. Click **Connect**.

### Troubleshooting

- **Connection refused:** Verify the device is listening on the specified port. Check firewall rules. Confirm the IP address is correct.
- **Connection times out:** The dial gives up after 5 seconds; check the IP address and routing.
- **No data:** Verify the slave address, register type, start address, and count match the device's register map documentation.

## Modbus Register Map Import (Pro)

**License:** Pro

A register map file describes every register of a device. Serial Studio reads it and generates a project with register groups, datasets, a Lua frame parser and dashboard widgets.

### Supported Formats

Serial Studio auto-detects the format from the file extension and accepts **CSV**, **XML**, and **JSON** files.

### How to Import

1. In the **Setup Panel**, select **Modbus** from the Connection dropdown.
2. Click **Import Register Map…**.
3. Select a CSV, XML, or JSON register map file.
4. Review the registers in the **Modbus Register Map Preview** dialog.
5. Click **Create Project**. Serial Studio asks you to save the `.ssproj` file.
6. The register groups are also loaded into the Modbus driver automatically.

### CSV Format

CSV is the most common format for Modbus register maps. Most PLC vendors, SCADA tools, and device datasheets can export or be converted to this format.

**Columns** (header row required, order is flexible; only `address` is required, and `name` is the first one to add):

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
| `slave`       | `unit_id`, `unitid`, `device`                     | No       | connection's unit |
| `bit`         | `bit_index`                                       | No       | whole word |
| `rw`          | `access`, `writable`                              | No       | read-only  |
| `order`       | `word_order`, `byte_order`                        | No       | abcd       |

The last four columns describe how the device is wired rather than what a register holds:

- **`slave`**: the unit the register lives on. Registers from different units become separate polled blocks, each read from its own unit, and the generated parser tells the replies apart by the unit byte they carry. Leave it empty for a single-device bus. (`unit` alone still means the units column.)
- **`bit`**: a bit index inside a holding or input register (0 = least significant). The row becomes a light in the block's LED Panel group, following that bit; the rest of the word stays untouched.
- **`rw`**: `w`, `rw`, `write`, `yes` or `true` marks the register writable. A writable row also produces an output control in a generated Controls panel that writes the register (a toggle for coils and bit rows, a slider for numeric registers) and reads its state back from the block's table. A single write covers at most 123 registers.
- **`order`**: the word/byte order of a 32-bit or 64-bit value: `abcd` (big-endian, the default), `cdab` (word swap), `badc` (byte swap) or `dcba` (little-endian). `big`/`little`/`swap` are accepted as aliases.

**Register type values:** `holding` (or `0x03`, `3`, `hr`), `input` (or `0x04`, `4`, `ir`), `coil` (or `0x01`, `1`), `discrete` (or `0x02`, `2`, `di`).

**Data type values:** `uint16`, `int16`, `uint32`, `int32`, `uint64`, `int64`, `float32`, `float64`, `bool`. Unknown types fall back to `uint16`; `bool` defaults its range to 0-1.

The same four optional fields are accepted as XML attributes and JSON keys under the same names (`slave`, `bit`, `rw`, `order`).

Lines starting with `#` are treated as comments and skipped.

A complete map is in [Example Files](#example-files).

### Converting Vendor Data to CSV

Most Modbus device documentation provides register maps as tables in PDF or Excel. To convert:

1. **Identify the columns:** Look for register address, name/description, data type, and units.
2. **Map register types:** Holding registers (function code 03), input registers (04), coils (01), discrete inputs (02).
3. **Note multi-register values.** A 32-bit value (`float32`, `uint32`, `int32`) occupies two consecutive 16-bit registers; a 64-bit value occupies four. Set the address to the first register and Serial Studio reads the rest as one value.
4. **Add scaling if needed:** If the device stores temperature as raw value × 0.1, add `scale,0.1` to the CSV.
5. **Save as `.csv`** with UTF-8 encoding.

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
- **Project groups.** One Data Grid group per contiguous block of same-type registers, plus one workspace per group and an Overview workspace.
- **Variables tables.** One table per block, with one variable per entry; the parser publishes values into the tables and each dataset reads its value back through a small Lua transform, so datasets never depend on parser channel order.
- **Datasets.** One per register entry, with name, units, min/max, a plot toggle, and widget type (LED with an alarm band for booleans, gauge for temperatures/pressures, bar for percentages). Plots and gauges open on demand from the Data Grid rows instead of crowding the dashboard.
- **Frame parser.** A commented, editable Lua parser with one declarative spec line per register. It extracts values from Modbus protocol frames, handles data types (uint16, int16, float32, ...), and applies scaling/offset. See [How Multi-Group Polling Works](#how-multi-group-polling-works) for details on how the parser handles multiple register groups.

### Example Files

A hydraulic test stand register map in CSV:

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

The same map in JSON uses the flat layout (`"registers": [...]`) with one object per row, for example `{"address": 4, "type": "holding", "name": "Flow Rate", "dataType": "float32", "units": "GPM", "min": 0, "max": 50}`. In XML it is the nested layout with one `<register .../>` element per row under `<holding-registers>`, `<input-registers>`, `<coils>` and `<discrete-inputs>`.

## How Multi-Group Polling Works

Modbus (Pro) polls register groups in order, and the generated Lua frame parser decodes each reply.

### Driver Side

Each poll tick, the Modbus driver polls group 0, then 1, then 2, and so on in list order, and emits one frame per response.

### Parser Side

The Modbus frame parser matches each reply to a register block by its function code (the second byte of the response) and keeps the other blocks' last values in their Variables tables. For two groups, holding registers carry function code `0x03` and coils `0x01`, so each reply updates only its own block's datasets. If a response times out, the parser keeps that block's previous values and resynchronizes on the next matching reply.

### Practical Considerations

- **Customization:** The generated Modbus parser is editable Lua. Each register is one line in the `BLOCKS` spec (`name`, `offset`, `type`, optional `scale`/`shift`); to add a register, add a line there plus a matching table variable and a dataset that reads it back with `tableGet()`. For larger changes, adjust the register groups and regenerate the project.

## CAN Bus Setup (Pro)

**License:** Pro

**Prerequisites:** CAN-to-USB adapter (PEAK PCAN, Kvaser, CANable, SocketCAN-compatible) connected and drivers installed.

### Steps

1. In the **Setup Panel**, select **CAN Bus** from the Connection dropdown.
2. Select the **CAN Driver** (backend). The Qt plugins are `socketcan` (Linux), `peakcan`, `vectorcan`, `systeccan`, `tinycan`, and `virtualcan`; Serial Studio adds its own `canable_gsusb` (CANable USB), `slcan` (Serial CAN), and `seeed_usbcan` (Seeed / Waveshare) backends.
3. Select the **Interface** from the dropdown (e.g., `can0`, `PCAN_USBBUS1`).
4. Set the **Bitrate** to match the CAN network exactly. Default: 500000. Common values: 125000, 250000, 500000, 1000000 bps.
5. Optionally enable **Flexible Data-Rate** if the network uses CAN FD frames. **Listen-Only** (passive monitoring) and **Loopback** (self-test) are off by default.
6. Click **Connect**.

To skip writing a project by hand, click **Import DBC File…** under **DBC Database**: the importer generates a complete project from a CAN signal database (see [Auto-Generating Projects](Auto-Generating-Projects.md)). Without a DBC, frames reach the [frame parser](Frame-Parser.md) as binary records (ID, DLC, payload); decode them with the **Binary (Direct)** decoder and your own parse function.

### Linux SocketCAN Preparation

Before connecting in Serial Studio, bring up the CAN interface from a terminal:

```bash
sudo ip link set can0 type can bitrate 500000
sudo ip link set up can0
```

For CAN FD, add the data bitrate:

```bash
sudo ip link set can0 type can bitrate 500000 dbitrate 2000000 fd on
sudo ip link set up can0
```

### Troubleshooting

- **No messages received:** Verify the bitrate matches the network exactly. Even a slight mismatch causes all frames to be rejected. Check CAN-H/CAN-L wiring and termination (120 ohm at each bus endpoint; measure approximately 60 ohm between CAN-H and CAN-L).
- **Interface not listed:** Ensure the adapter driver is installed. On Linux, verify the interface is up with `ip link show can0`. On Windows, check Device Manager for the CAN adapter.
- **Error frames only:** Bitrate mismatch, missing termination, or bus contention. Check all physical connections.

Full driver reference: [CAN Bus](Drivers-CAN-Bus.md).

## Audio Input Setup (Pro)

**License:** Pro

**Prerequisites:** Audio input device (microphone, line-in, audio interface) connected and recognized by the OS.

### Steps

1. In the **Setup Panel**, select **Audio** from the Connection dropdown.
2. Select the **Input Device** from the dropdown (lists all detected audio capture devices).
3. Select the **Sample Rate**. Common: 44100 Hz, 48000 Hz. Available rates depend on the device.
4. Leave **Normalization** ticked (the default) to get samples as floats in the -1.0 to 1.0 range; the driver then picks the sample format itself and the **Sample Format** selector is hidden.
5. If you unticked **Normalization**, select the **Sample Format**. Options, as supported by the device: Unsigned 8-bit, Signed 16-bit, Signed 24-bit, Signed 32-bit, Float 32-bit.
6. Select the **Channels** layout (Mono, Stereo, or a multichannel layout, depending on the device).
7. Click **Connect**.

Audio data flows into the pipeline as PCM samples, which can be visualized with Plot, FFT Plot, and Waterfall widgets.

### Troubleshooting

- **No audio detected:** Verify the input device is selected correctly in the dropdown. Check the OS audio settings to confirm the input is not muted and the level is adequate. On macOS, grant Microphone permission.
- **Distorted signal:** Reduce the input gain in OS audio settings. Use a line-in input instead of a microphone input when connecting sensors.
- **Wrong sample rate options:** The available sample rates are reported by the hardware. If the rate you need is not listed, try a different audio interface.

Full driver reference: [Audio](Drivers-Audio.md).

## Raw USB Setup (Pro)

**License:** Pro

**Prerequisites:** USB device with accessible bulk, control, or isochronous endpoints. On Linux, appropriate `udev` rules. On Windows, a WinUSB-compatible driver (installable via Zadig).

### Steps

1. In the **Setup Panel**, select **USB Device** from the Connection dropdown.
2. Pick the device from the **USB Device** dropdown. Devices are listed as `VID:PID – Manufacturer Product`.
3. Select the **Transfer Mode**:
   - **Bulk/Interrupt Stream** (default): Standard synchronous bulk or interrupt IN/OUT. Works for most devices.
   - **Advanced (Bulk + Control)**: Bulk transfers plus vendor-specific control transfers. A confirmation dialog appears before enabling, since incorrect control requests can crash or damage hardware.
   - **Isochronous**: Asynchronous isochronous transfers for fixed-rate streaming.
4. Select the **IN Endpoint** (for reading data from the device).
5. Select the **OUT Endpoint** (for writing data to the device), if applicable.
6. For Isochronous mode, set the **Max Packet Size** to the maximum transfer size reported by the selected endpoint (prefilled from the endpoint descriptor).
7. Click **Connect**.

### Troubleshooting

- **Device not listed:** On Linux, add a `udev` rule granting your user access to the device's VID/PID, then run `sudo udevadm control --reload-rules && sudo udevadm trigger`. On Windows, use Zadig to install a WinUSB driver for the device. On macOS, the device should appear if no kernel driver has claimed it.
- **Permission denied:** On Linux, verify the `udev` rule is correct and the user is in the appropriate group. Running as root is a quick test but not recommended for production.
- **No data:** Verify the correct IN endpoint is selected. Check that the device firmware is sending data on that endpoint.

Full driver reference: [USB](Drivers-USB.md).

## HID Setup (Pro)

**License:** Pro

**Prerequisites:** USB HID-class device (gamepad, joystick, custom HID firmware, sensor) connected.

### Steps

1. In the **Setup Panel**, select **HID Device** from the Connection dropdown.
2. Wait for device enumeration. HID devices are re-enumerated every 2 seconds automatically.
3. Pick the device from the **HID Device** dropdown. Devices are listed as `Product Name (VID:PID)`.
4. Click **Connect**.
5. Once connected, the **Usage Page** and **Usage** fields show which HID collection is active, which helps when a device exposes several interfaces.

### Troubleshooting

- **Device not listed:** On Linux, add a `udev` rule for `hidraw` access: create a file in `/etc/udev/rules.d/` with `KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666"` (or a more restrictive rule targeting your VID/PID). On Windows and macOS, HID devices are generally accessible without additional configuration.
- **No data:** Verify the device is actively sending HID reports. Some HID devices only send reports when there is a state change (e.g., button press on a gamepad).

Full driver reference: [HID](Drivers-HID.md).

## Process I/O Launch Mode Setup (Pro)

**License:** Pro

**Prerequisites:** A script or executable that writes data to stdout in a format Serial Studio can parse.

### Steps

1. In the **Setup Panel**, select **Process** from the Connection dropdown.
2. Set the **Mode** to **Launch Process**.
3. Enter the **Executable** path, or click **Browse** to select it. Bare command names are resolved through the PATH.
4. Enter **Arguments** (command-line arguments for the process), if any.
5. Optionally set the **Working Dir** for the process.
6. Click **Connect**.

Serial Studio spawns the process, reads its merged stdout and stderr, and feeds the output into the data pipeline. You can write to the process's stdin via the console.

### Troubleshooting

- **Process not found:** Verify the executable path is correct. On Linux/macOS, ensure the script has execute permission (`chmod +x script.py`). For Python scripts, you may need to specify `python3` as the executable and the script path as an argument.
- **No data:** Ensure the script flushes its stdout (Python: use `flush=True` in `print()` or run with `python3 -u`). Buffered output will not appear until the buffer fills.
- **Process crashes immediately:** Check the working directory and arguments. Review the console output for error messages from the process.

Full driver reference: [Process I/O](Drivers-Process-IO.md).

## Process I/O Named Pipe Setup (Pro)

**License:** Pro

**Prerequisites:** An external process writing data to a named pipe (FIFO on Linux/macOS, `\\.\pipe\Name` on Windows).

### Steps

1. In the **Setup Panel**, select **Process** from the Connection dropdown.
2. Set the **Mode** to **Named Pipe**.
3. Enter the **Pipe Path**, or click **Browse** to select it.
   - Linux/macOS: `/tmp/myfifo`. Serial Studio creates the FIFO automatically if it doesn't already exist.
   - Windows: `\\.\pipe\MyPipeName`. The external process must create the named pipe before you connect.
4. Optionally click **Pick Running Process…** to choose a running process from a list.
5. Click **Connect**.

Serial Studio opens the named pipe for reading and streams data into the pipeline. The external process must open the pipe for writing.

### Troubleshooting

- **Cannot open pipe:** On Windows, the pipe must be created by the writing process before Serial Studio connects. On Linux/macOS, Serial Studio creates the FIFO itself if it's missing; if opening still fails, check that the path is writable and not already claimed by another process.
- **No data:** Ensure the external process is actively writing to the pipe. A pipe delivers nothing until a writer has it open.

Full driver reference: [Process I/O](Drivers-Process-IO.md).

## See Also

- [Communication Protocols](Communication-Protocols.md): protocol overview and comparison.
- [Devices](Devices.md): the driver model, multi-device projects, and per-driver summaries.
- [Auto-Generating Projects](Auto-Generating-Projects.md): DBC, Modbus map, and Protobuf importers.
- [Frame Parser Scripting](Frame-Parser.md): writing a `parse(frame)` function for custom data formats.
- [MQTT Topics & Semantics](MQTT-Topics.md): protocol vocabulary used by both MQTT surfaces.
- [MQTT Subscriber](Drivers-MQTT.md): in-depth subscriber-driver documentation.
- [MQTT Publisher](MQTT-Publisher.md): project-level publisher documentation.
- [Getting Started](Getting-Started.md): first-time setup tutorial.
- [Troubleshooting](Troubleshooting.md): general troubleshooting guide.
