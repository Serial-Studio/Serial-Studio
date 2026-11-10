# Device Declarations

Your Arduino prints values with `Serial.println`. A device declaration lets it also tell Serial Studio how to draw them: titles, units, ranges and widgets, sent as one line of text. Serial Studio builds the dashboard from that line on connect, with no project file and without opening the Project Editor.

The SerialStudio device library writes the line for you in Arduino C++, plain C and Python. This sketch declares a GPS map:

```cpp
#include <SerialStudio.h>

SerialStudio studio(Serial, "TinyGPS");

void setup() { Serial.begin(115200); }

void loop() {
  studio.poll();

  studio.group(F("GPS Map"), F("gps"));
  studio.channel(F("Latitude"), 48.8584, 4).units(F("deg")).widget(F("lat")).graph(false);
  studio.channel(F("Longitude"), 2.2945, 4).units(F("deg")).widget(F("lon")).graph(false);
  studio.channel(F("Altitude"), 35.0, 1).units(F("m")).widget(F("alt")).graph(false);
  studio.send();
}
```

The first pass through `loop()` sends the declaration. Every later pass sends a frame with the current values. Install the library first (see [Arduino](#arduino)).

In Serial Studio:

1. In the **Device Setup** panel, set **Operation Mode** to **Quick Plot (Auto-Detected Format)** (see [Operation Modes](Operation-Modes.md)).
2. Select the serial port.
3. Set the baud rate to 115200 to match `Serial.begin`.
4. Press **Connect**.

A GPS Map widget appears. `graph(false)` leaves out the per-channel plots, so only the map shows.

Declarations are free and work in Quick Plot mode. Project File mode offers them without applying them (see [Adopting a Declaration](#adopting-a-declaration)). Declared controls need Pro, because the Output Panel they generate is a [Pro feature](Output-Controls.md). Serial Studio offers the 14-day Pro trial in a Yes/No prompt the first time you use a Pro feature; registering it needs an internet connection. See [Pro vs Free](Pro-vs-Free.md).

The library is MIT licensed, so it can be embedded in closed firmware, and lives in the `firmware/` directory of the Serial Studio repository. Releases up to 4.2.1 do not understand declarations: they rebuild the dashboard on any text line. Use a later release. A device that declares nothing is a plain Quick Plot stream, exactly as before.

## How it works

1. The device sends the declaration at power-up, and again whenever Serial Studio asks.
2. In Quick Plot mode, Serial Studio skips its guess at your data format and builds the dashboard from the declaration.
3. Every later line is a frame whose values follow the declared channel order.

**Ask Devices for Their Dashboard** in **Preferences → Plotting** is off by default. When it is on, Serial Studio sends one request line, `#ss?`, as a Quick Plot connection opens, and the library answers with the declaration. That finds a device that declared at boot before Serial Studio was listening, including after a reset. Console Only and Project File modes never send the request. If the dashboard does not appear, turn the setting on and reconnect; with it off, a declaration is read only when the device sends it, for example at power-up.

Sending the same declaration again changes nothing. A changed declaration rebuilds the dashboard. Each declaration, accepted or refused, is reported in the Console pane.

A declaration never changes the operation mode, never writes a file and never changes a connection setting. The generated dashboard is an ordinary Quick Plot session: use **Save as Project** in the dashboard's start menu to keep it as a project file.

## Wire format

This section is for firmware that does not use the library. A declaration is a single line ending in a newline:

```
#ss1{"title":"TinyGPS","groups":[{"title":"GPS Map","widget":"gps","datasets":[{"title":"Latitude","units":"°","widget":"lat"},{"title":"Longitude","units":"°","widget":"lon"},{"title":"Altitude","units":"m","widget":"alt"}]}]}
```

- `#ss` is the reserved prefix. The digit after it is the wire version, currently `1`. An unknown version is ignored with a console message.
- The rest of the line is a JSON object with a non-empty `title` and a `groups` array of 1 to 64 groups. The declaration needs at least one dataset, so a controls-only declaration is refused.
- A group has a `title`, an optional `widget` (`datagrid`, `multiplot`, `accelerometer`, `gyro`, `gps`, `plot3d`, `barpanel` or `ledpanel`) and a `datasets` array. A group whose datasets do not fit its widget is shown as a data grid, with a console note.
- A dataset takes the same properties the Project Editor shows for a dataset: `title`, `units`, `widget`, `plotMin`, `plotMax`, `widgetMin`, `widgetMax`, `fft`, `alarmBands` and `labels`, among others. Keys that are not dataset properties are dropped and listed in the console.
- The libraries call a dataset a channel.
- Dataset order is frame order. A dataset takes the next frame position, unless it names an `index`, which reuses that position so two groups can show the same channel.
- Frames after a declaration are plain values in declared order, `1.5,2.5,3.5`, or named values, `Latitude:48.8584`, which map by name.

A declaration is data. It never carries a script, a transform, a file path or a connection setting. A malformed, oversized or over-cap declaration is ignored with a console message; the caps are listed under [Limits](#limits).

### Declared controls

Declared controls generate an Output Panel, a [Pro feature](Pro-vs-Free.md); the declaration itself is free. A group with `"groupType":1` holds `outputWidgets` instead of datasets. Each control has a `title` and an `outputType`: `0` button, `1` slider, `2` toggle, `3` text field (numeric input only), `4` knob. Sliders and knobs take `outputMin`, `outputMax` and `outputStep`.

The controls appear on the dashboard as [Output Controls](Output-Controls.md). Operating one makes Serial Studio send a line such as:

```
#ss!Speed=128
```

In an Arduino sketch, the control goes in a control group next to at least one channel, and `onControl()` receives the name and value:

```cpp
int onSpeed(const char* name, double value) { analogWrite(9, (int)value); return 0; }

studio.onControl(onSpeed);
studio.controlGroup(F("Controls"));
studio.control(F("Speed"), SS_CONTROL_SLIDER);
```

The callback returns `0` to accept the command. A control title is 1 to 32 characters from `A-Z`, `a-z`, `0-9`, space, `_`, `.` and `-`, with no leading, trailing or doubled spaces; an invalid title refuses the whole declaration. When Serial Studio tracks replies, it appends `@` and a token to the value and the library strips it. The token is appended to text frames only.

## Limits

| Limit | Value | Counts |
|-------|-------|--------|
| Title or label | 64 bytes | each string |
| Declaration line | 16384 bytes | whole line |
| Groups | 64 | per declaration, control groups included |
| Datasets | 512 | per group, and 512 in total across all groups |
| Controls | 64 | per control group |
| Alarm bands | 16 | per dataset |
| FFT markers | 16 | per dataset |
| Labels | 256 | per dataset |
| Control title | 32 bytes | each control |

Past a cap, Serial Studio ignores the declaration and reports it in the Console pane. The Python library raises `ValueError`; the C and Arduino libraries stop declaring.

## The library

All three variants share one wire format, and each calls a dataset a channel. Call `poll()` once per loop. On the first loop after power-up, and after every `#ss?` request, the library writes the declaration instead of a frame and drops that loop's values. On every other loop it sends a frame.

### Arduino

Install **SerialStudio** from the Arduino Library Manager, or copy `firmware/arduino` to `Arduino/libraries/SerialStudio`. The header allocates nothing at run time and keeps its declaration text in flash.

The GPS sketch in the [introduction](#device-declarations) is a complete example.

| Call | Effect |
|------|--------|
| `group(title, widget)` | Opens a dataset group. The widget is optional. |
| `channel(name, value, places)` | Declares and sends one channel; `places` is the number of decimal places sent. Without a value, the channel is declared only. |
| `units`, `widget`, `range`, `plotMin`, `plotMax`, `widgetMin`, `widgetMax`, `fft`, `index` | Dataset properties, chained after `channel()`. |
| `alarm(min, max, severity, label)` | Adds an alarm band. |
| `label(value, text)`, `labelsDefault(text)` | Names the values of a state channel. |
| `graph(false)` | Plots are on by default. This turns them off. |
| `controlGroup(title)`, `control(title, kind)` | Declares an Output Panel and its button, slider, toggle or knob. |
| `named(true)`, `separator(c)` | Sends `name:value` frames, or another separator. |
| `onControl(fn)`, `onLine(fn)` | Receives `#ss!` commands, and every other input line. |
| `poll()` | Reads input. Call once per loop. |

The Arduino examples are in `firmware/arduino/examples`, and the [MPU6050](../../examples/MPU6050/README.md) and [TinyGPS](../../examples/TinyGPS/README.md) examples each have a declared sketch.

### C

Copy `firmware/c/serialstudio.c`, `serialstudio.h` and `ss_vocabulary.h` into the project. The file is C99. Output goes through a write callback; input arrives through `ss_feed()`.

```c
/* write_to_uart(const uint8_t* data, size_t len) is your own UART send function. */
ss_t ss;
ss_init(&ss, write_to_uart, NULL, "TinyGPS");

ss_group(&ss, "GPS Map", "gps");
ss_channel_double(&ss, "Latitude", 48.8584, 4);
ss_units(&ss, "deg");
ss_widget(&ss, "lat");
ss_send(&ss);
```

`ss_group`, `ss_channel_int`, `ss_channel_double`, `ss_channel_str`, `ss_control_group` and `ss_control` open groups and declare channels. Property macros such as `ss_units`, `ss_widget`, `ss_plot_min`, `ss_range` and `ss_alarm` apply to the channel just opened. `ss_set_handlers()` registers the control and line callbacks.

### Python

Install with `pip install serialstudio`. The module uses the standard library only and writes bytes through a callable, so it works over a serial port, a socket or a pipe. The example below talks to a serial port, so it also needs `pip install pyserial`. The first `send()` writes the declaration; each later one writes a frame.

```python
import time
import serial
import serialstudio

port = serial.Serial("COM3", 115200)
device = serialstudio.Device("TinyGPS", write=port.write)

while True:
    device.poll()
    device.group("GPS Map", "gps")
    device.channel("Latitude", 48.8584, places=4, units="deg", widget="lat", graph=False)
    device.channel("Longitude", 2.2945, places=4, units="deg", widget="lon", graph=False)
    device.channel("Altitude", 35.0, places=1, units="m", widget="alt", graph=False)
    device.send()
    time.sleep(0.1)
```

Dataset properties are keyword arguments in snake case (`plot_max=5`, `fft_sampling_rate=200`). Pass `range=(0, 100)` to set both the widget and plot range. `Device(title, write, separator=",", named=False, on_control=None, on_line=None)` takes the callbacks; `feed(data)` buffers received bytes and `poll()` dispatches them. `serialstudio.tcp_server(device)` serves a device over TCP on localhost, for testing against a [Network](Drivers-Network.md) source.

## Adopting a declaration

If you opened a `.ssproj` in Project File mode and the connected device also sends a declaration, the declaration never rebuilds the loaded project. Serial Studio only compares it with the loaded project. When it differs, one notification reports that the device declares a different dashboard, and two buttons appear in the Setup panel under the project file: **Use Device Dashboard** and **Keep Project**.

**Use Device Dashboard** (Project File mode) disconnects, offers to save unsaved edits, and opens the declared project as a new, unsaved project with the loaded project's connection. The loaded file stays untouched on disk: reopening it reverses the choice.

**Keep Project** (Project File mode) dismisses the offer for this project and this declaration. It comes back only when the device declares something different or another project file is loaded. Disconnecting forgets the declaration, because the next connection may be another device.

Adopting is the only way a declaration reaches Project File mode. To keep a Quick Plot dashboard instead, use **Save as Project**.

## Binary frames

Skip this if your device prints text. Binary declarations work in Quick Plot mode only. A device that sends packed binary frames declares their layout in the same line. The declaration itself is always text; only the frames after it are binary. The libraries write the numeric codes (`frameDetection`, `decoderMethod`), so you never type them. The line gains a `sources` array with one entry that names the framing and the `packed_struct` frame parser template:

```
#ss1{"title":"MPU6050","sources":[{"frameStart":"AA55","hexadecimalDelimiters":true,"frameDetection":3,"checksumAlgorithm":"CRC-16","decoderMethod":3,"frameParserTemplate":"packed_struct","frameParserParams":{"fields":"f32,f32,f32,f32,f32,f32,f32","endianness":"little"}}],"groups":[...]}
```

Each frame is the start delimiter, the fields in declared order and a checksum. `fields` lists `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f32` or `f64` per channel, and `endianness` is `little` or `big`. A frame whose length does not match the layout is dropped. The `connection` of a source is never read from a declaration.

The one-call `binary()` route is the recommended one; the libraries send the frames for you:

```python
device = serialstudio.Device("MPU6050", write=port.write)
device.binary("f32,f32,f32,f32,f32,f32,f32", "little", b"\xaa\x55", "CRC-16")
```

The C library offers `ss_set_binary()` and the Arduino header `binary()`. The libraries write the `CRC-16` checksum only. On an AVR board `double` has 32 bits, so `f64` fields carry single-precision values.

When Serial Studio switches to the declared framing, the first frames after a declaration can be dropped. A device that declares again sends the declaration as a text line inside the binary stream. Serial Studio finds it in the raw bytes of the connection, and a changed declaration clears the binary framing and rebuilds the dashboard from the new one.

## See also

- [Operation Modes](Operation-Modes.md)
- [Getting Started](Getting-Started.md)
- [Output Controls](Output-Controls.md)
- [Project Editor](Project-Editor.md)
- [Pro vs Free](Pro-vs-Free.md)
- [Auto-Generating Projects](Auto-Generating-Projects.md)
