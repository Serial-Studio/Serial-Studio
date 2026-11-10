# Operation modes

## Overview

Your board prints `23.5,60,1013` once per second. Quick Plot mode plots those three numbers the moment you connect. Console Only mode shows you the raw bytes instead, and Project File mode lets you design the dashboard yourself.

Those are the three operation modes in Serial Studio. Set the mode with **Operation Mode** in the **Device Setup** panel on the right side of the main window. Every mode works with every driver. UART, Network and Bluetooth LE are free; any other bus needs Pro, in every mode. See [Devices](Devices.md) for the driver list and [Pro vs Free Features](Pro-vs-Free.md) for the rest.

If your board prints numbers, start with Quick Plot. All three modes are available in the free edition; individual features inside a project, such as multi-source, dataset transforms and Variables, need Pro.

## Console Only mode

Console Only is the Serial Studio mode that shows raw bytes in the terminal and builds no dashboard. To use it, set **Operation Mode** to "Console Only (No Parsing)" in the **Device Setup** panel.

Bytes go straight from the device to the terminal. Nothing is split into frames, plotted, or exported to CSV, and no frame parser runs. The terminal shows the stream as ASCII or hex, you can save the transcript to a file, and the input box still sends bytes back to the device.

If the console shows `ÿÿ¿¿` instead of numbers, the baud rate is wrong. Change it in **Device Setup** and reconnect.

Use it to check baud rate, wiring and framing before you build a project, to see what a stream looks like when another mode parses it wrong, or to talk to a device with a text protocol such as AT commands.

## Quick Plot mode

Quick Plot is the Serial Studio mode that plots text lines with no project file and no script. To use it, set **Operation Mode** to "Quick Plot (Auto-Detected Format)" in the **Device Setup** panel, pick your port and baud rate, then connect.

### Example input

An Arduino sketch that prints a temperature and a humidity per line:

```cpp
void setup() {
  Serial.begin(9600);
}

void loop() {
  float temp = 23.5;
  float hum = 60;
  Serial.print(temp);
  Serial.print(",");
  Serial.println(hum);
  delay(1000);
}
```

It prints `23.5,60` once per second. In Quick Plot, set 9600 baud in **Device Setup** to match `Serial.begin(9600)`. Each line is one sample (a frame), and each number becomes one plot (a channel).

To name the plots, print a header once in `setup()` with `Serial.println("Temperature,Humidity");`. The stream then looks like this, and the text line titles the channels:

```
Temperature,Humidity
23.5,60
23.6,59
23.7,59
```

Named values need no header row:

```
temp: 23.5, hum: 60
temp: 23.6, hum: 59
```

When the connection opens, Serial Studio reads each line, detects the format, and builds a dashboard with a Data Grid showing all current values, a Multi-Plot overlaying all channels on one time-series chart, and an individual plot per channel.

### Accepted shapes

Serial Studio detects the format from the first qualifying line, one where at least half the fields are numbers (or name:value pairs with numbers). Each value maps to one channel. A line ends at CR, LF, or CRLF. Semicolon-separated lines such as `23.5;60;1013` work the same way.

| Shape | Example | Result |
|-------|---------|--------|
| Values separated by commas, semicolons, tabs or runs of spaces | `23.5,60,1013` or `23.5 60 1013` | One channel per value |
| Decimal comma under semicolon or space separation | `23,5;60` | Channels `23.5` and `60` |
| Named values, `name:value` or `name=value`, optional spaces after the delimiter | `temp: 23.5, hum: 60` | The names title the channels |
| Unit after a number | `23.5degC`, `1013hPa`, `45%` | The unit becomes the channel's unit and is stripped from the value |
| One-line JSON object | `{"t":23.5,"h":60}` | Nested objects flatten to dotted names (`imu.ax`), arrays to indexed names (`acc[0]`); strings, booleans and nulls are ignored |
| Line wrapped in `/*` and `*/` | `/* 23.5,60 */` | Parsed as its contents |

### Edge cases

- **Text lines.** A line with no numeric field (such as `WiFi connected`) never touches the dashboard. It still appears in the console.
- **Header detection.** For a positional stream, an all-text line before the first data line titles the channels, provided it has the same number of fields as that data line. Boot chatter with a different field count is ignored. Names on data lines take precedence, so a device already streaming `name:value` pairs when you connect keeps its labels.
- **Named streams.** Channels are keyed by name. A name missing from a line leaves that channel without a sample for that frame: plots show a gap and the Data Grid keeps the last value. A new name appends a channel. Positional streams rebuild the dashboard when the field count changes.
- **Format lock.** Once detected, the format stays fixed for the connection. It is re-evaluated only when a line has no numeric field under it but another format fits, such as boot chatter `uptime=1234` followed by `temp=23.5,hum=60`.
- **Field limit.** Quick Plot reads at most 512 fields per line; the rest of a longer line is dropped.
- **Device declarations.** A device that uses the SerialStudio library can send a declaration that builds the dashboard in place of format detection; see [Device Declarations](Device-Declarations.md). With **Preferences → Plotting → Quick Plot → Ask Devices for Their Dashboard** turned on (off by default), Serial Studio also asks for it on connect with one request line, `#ss?`. Console Only and Project File mode never send the request.

### Save as Project

To keep the stream after the session, use **Save as Project** on the dashboard. The saved project's Built-In parser uses the Delimited text template for plain comma streams and the Line values template otherwise.

Known difference: in a saved project a missing named field repeats the previous value, where the live session leaves a gap.

### Limitations

Quick Plot has no manual format override (use Save as Project for anything the detector cannot settle) and no binary or hex input. Widgets, alarms, parser scripts and multi-source are Project File features; see the [Feature comparison](#feature-comparison).

## Project File mode

Project File is the Serial Studio mode where a `.ssproj` file on the host defines the dashboard. To use it, set **Operation Mode** to "Parse via Project File" in the **Device Setup** panel, then load an existing project or build one in the Project Editor (the **Project Editor** button in the toolbar).

Say your device sends `23.5,60` followed by a newline. In the Project Editor:

1. Add two datasets, Temperature and Humidity (a dataset is one value on the dashboard). The first number lands in the first dataset, the second in the second.
2. Set the source's frame detection to **End Delimiter Only** and type `\n`.
3. The decoder already defaults to **Plain Text (UTF-8)**; connect.

Where each setting lives in the editor is covered in [Project Editor](Project-Editor.md).

Beyond that, the `.ssproj` JSON file also holds the groups, widgets, alarms, FFT settings and actions. A Built-In template, or an optional `parse(frame)` function in Lua or JavaScript, turns arbitrary protocols into the array of values Serial Studio expects. A project can define several devices; see [Multi-source support](#multi-source-support) (Pro).

This mode gives access to every widget type and configuration option in Serial Studio, including alarms and FFT analysis.

### Frame detection methods

Serial Studio needs to know where one reading ends and the next begins; a delimiter is the byte or bytes that mark that. A source is one connected device, and frame detection is configured per source in the Project Editor.

If your device ends each line with a newline, choose **End Delimiter Only** and type `\n`; you can skip the rest of this section.

| Method                      | API value | Behavior |
|-----------------------------|-----------|----------|
| **End Delimiter Only**      | 0         | A frame ends when the end delimiter is seen. The common choice for line-terminated CSV. |
| **Start and End Delimiter** | 1         | A frame begins at the start delimiter and ends at the end delimiter, for example `$DATA...;\n`. |
| **No Delimiters**           | 2         | All incoming data goes to the frame parser without splitting. For length-prefixed or fixed-size binary protocols where the parser finds the boundaries. |
| **Start Delimiter Only**    | 3         | A frame runs from one start delimiter to the next. Released only when the next start marker arrives (see Start Delimiter Only below). |
| **Fixed Length**            | 4         | Every frame is exactly **Frame Length** bytes. Details under Fixed Length below. |

Delimiters can be plain text or hexadecimal byte sequences (toggle "Hexadecimal Delimiters" in the Project Editor). Fixed Length uses no delimiters, so the Project Editor hides the delimiter fields for it.

**Start Delimiter Only.** In Project File mode, the second occurrence of the delimiter becomes the start of the next frame, so a frame is released only once that next start marker arrives. The most recent frame sits buffered until then; for a slow or paused sender (a 1 Hz source, for example), the dashboard runs one frame behind.

**Fixed Length.** In Project File mode, every frame is **Frame Length** bytes, checksum included, from 1 to 524288. A source with a length outside that range falls back to No Delimiters.

- The **Header Rule** (a mask and a value) selects the first byte of a frame: a byte starts a frame when `(byte & mask) == value`, and bytes before it are discarded.
- The **Body Rule** applies the same test to every other byte of the frame. A frame that fails it is dropped, and the search for the next header resumes at the offending byte.
- A mask of `00` turns a rule off. With no Header Rule the stream is assumed to be aligned and is cut every N bytes.
- If a checksum is set and fails, the reader drops one byte and searches for the next header (with a Header Rule), or drops the whole frame (without one).

For example, a 5-byte frame whose first byte always has bit 7 set and whose other bytes never do uses Frame Length `5`, Header Rule mask `80` value `80`, and Body Rule mask `80` value `00`. If a byte is lost on the wire, the reader skips forward to the next byte with bit 7 set instead of staying misaligned.

### Decoder methods

The decoder determines how raw bytes are converted before being passed to the frame parser (or split as CSV).

| Decoder                 | API value | Description |
|-------------------------|-----------|-------------|
| **Plain Text (UTF-8)**  | 0         | Bytes are decoded as UTF-8 text. The most common choice for ASCII/CSV protocols. |
| **Hexadecimal**         | 1         | Each byte is converted to a two-character hex string. For example, bytes `0x03 0xFF 0x02` become `"03ff02"`. |
| **Base64**              | 2         | Bytes are converted to a Base64 string. |
| **Binary (Direct)**     | 3         | Raw bytes arrive as a Lua table or JavaScript array of integers 0 to 255. |

### Frame parser script

Most newcomers need no script. Every new source defaults to a Built-In (Native) template: a no-code parser configured through a parameter form, covering CSV, JSON, NMEA and many more formats. See [Frame Parser Reference](Frame-Parser.md) for the full template catalog.

When no Built-In template fits, write a `parse()` function in Lua or JavaScript that turns each frame into the array of values Serial Studio expects. The examples below show both; the full Lua API is in the [Frame Parser Reference](Frame-Parser.md).

The signature, in JavaScript:

```javascript
function parse(frame) {
    return [value1, value2, value3];
}
```

**Key behaviors:**

- The returned array is mapped to datasets by index. Datasets are numbered from 1 in the Project Editor, so element 0 goes to dataset 1, element 1 goes to dataset 2, and so on.
- `frame` is a string with the Plain Text, Hexadecimal and Base64 decoders, and an array of integers with Binary (Direct).
- Advanced returns: an array of arrays emits several frames from one call (`[[row1_val1, row1_val2], [row2_val1, row2_val2]]`), and `[scalar, [vec1, vec2, vec3]]` expands the inner array into separate dataset values.
- `parse()` takes exactly one argument; the two-argument `parse(frame, separator)` form from v2 projects is rejected when the script loads.

**Example: parsing a semicolon-delimited protocol.** For the input `23.5;60;1013` this returns three values for three datasets.

```javascript
function parse(frame) {
    return frame.split(";");
}
```

The same parser in Lua:

```lua
function parse(frame)
  return frame:split(";")
end
```

`split` is a Serial Studio helper, not standard Lua; see [Frame Parser Reference](Frame-Parser.md).

**Example: parsing a fixed-size binary packet.** Set the decoder to **Binary (Direct)** and frame detection to **Fixed Length** with Frame Length `4`. Bytes 0-1 are a big-endian uint16 temperature in tenths of a degree; bytes 2-3 are a big-endian uint16 pressure.

```javascript
function parse(frame) {
  var temp = ((frame[0] << 8) | frame[1]) * 0.1;
  var pres = (frame[2] << 8) | frame[3];
  return [temp, pres];
}
```

### Multi-source support

Multi-source is a Pro feature of Project File mode; the free edition allows one source per project. A multi-source project defines several devices, and each source has its own ID and title, bus type (UART, Network, BLE and so on), connection settings, frame detection, decoder and frame parser.

All sources feed one dashboard at the same time. For example, a weather station project might define one UART source for a ground sensor array and one TCP source for a remote wind station, both feeding one dashboard.

## Picking the right mode

If the console shows gibberish, fix the baud rate before you blame the firmware. Start in Console Only to confirm the bytes make sense, move to Quick Plot once you see clean numbers one line per sample, and move to Project File when you need more control.

| Scenario                                                     | Recommended mode |
|--------------------------------------------------------------|------------------|
| Garbled stream, unknown baud rate, AT commands, a text CLI  | Console Only |
| Arduino or ESP32 printing numbers, one line per sample       | Quick Plot |
| Need gauges, bars, compass, GPS map, FFT, or alarms          | Project File |
| Custom binary protocol with length-prefixed packets          | Project File + Binary decoder + Lua/JS parser |
| Multiple sensors on different ports in one dashboard         | Project File (multi-source, Pro) |
| Production telemetry system with saved configuration         | Project File |

### Feature comparison

| Feature                     | Console Only        | Quick Plot       | Project File                  |
|-----------------------------|---------------------|------------------|-------------------------------|
| Setup effort                | None                | None             | Create project in editor      |
| Dashboard                   | No                  | Auto             | Custom                        |
| Frame detection             | None (raw stream)   | Line-based (auto)| Configurable per source       |
| Line format                 | N/A                 | Detected (see Accepted shapes) | Any (Built-In template or parser script) |
| Frame parser (Built-In/Lua/JS) | No               | No               | Yes                           |
| Dataset value transforms    | No                  | No               | Yes (Pro)                     |
| Variables (registers)       | No                  | No               | Yes (Pro)                     |
| Custom widgets              | No                  | No (plots only)  | Yes (project-defined)         |
| Alarms and LED indicators   | No                  | No               | Yes                           |
| FFT analysis                | No                  | No               | Yes                           |
| Multi-source                | No                  | No               | Yes (Pro)                     |
| CSV export                  | No                  | Yes              | Yes                           |
| MDF4 export                 | No                  | Yes (Pro)        | Yes (Pro)                     |
| Saved configuration         | No                  | No               | Yes (.ssproj file)            |

## See also

- [Project Editor](Project-Editor.md): building and configuring a project file, including picking and configuring a Built-In template.
- [Frame Parser Reference](Frame-Parser.md): the Built-In template catalog and the Lua/JavaScript `parse()` API in detail.
- [Dataset Value Transforms](Dataset-Transforms.md): per-dataset value transforms applied after parsing.
- [Devices](Devices.md): the driver types available to any operation mode.
- [Pro vs Free Features](Pro-vs-Free.md): which drivers and dashboard features require a Pro license.
