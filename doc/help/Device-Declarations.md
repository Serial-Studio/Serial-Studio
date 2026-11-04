# Device Declarations

## Overview

A device declaration is one line of text that a device sends to describe its own dashboard: the project title, the groups, and each channel's name, units, range and widget. Serial Studio builds the dashboard from it on connect, with no project file and without opening the Project Editor.

Declarations are free. They work in Quick Plot mode, and Project File mode offers them without applying them (see [Adopting a Declaration](#adopting-a-declaration)). Declared controls are the exception: the generated Output Panel is a [Pro feature](Output-Controls.md), available with a license or the 14-day trial.

The SerialStudio device library writes the declaration for you in Arduino C++, plain C and Python. It is MIT licensed, so it can be embedded in closed firmware. The library lives in the `firmware/` directory of the Serial Studio repository.

A device that declares nothing is a plain Quick Plot stream, exactly as before. A release with the Quick Plot line formats but without declarations shows the line in the console and leaves the dashboard unchanged.

## Requirements

Releases up to 4.2.1 cannot use declarations: they rebuild the dashboard on any text line. Use the first release that includes Device Declarations. That release also carries the Quick Plot line formats the declaration line relies on.

## How it works

1. The device sends the declaration at power-up, and again whenever Serial Studio asks.
2. In Quick Plot mode, the declaration replaces format detection. Serial Studio builds the dashboard from it.
3. Every later line is a frame whose values follow the declared channel order.

With **Ask Devices for Their Dashboard** turned on in **Preferences → Plotting** (off by default), Serial Studio sends one request line, `#ss?`, when a Quick Plot connection opens. The library answers it with the declaration, so a device that declared at boot before Serial Studio was listening is still discovered, including after a reset. Without the request, a device's declaration is read whenever the device sends it, for example at power-up. Console Only and Project File modes never send the request.

Sending the same declaration again changes nothing. A changed declaration rebuilds the dashboard, at most once per second. Each declaration, accepted or refused, is reported in the console.

A declaration never changes the operation mode, never writes a file and never changes a connection setting. The generated dashboard is an ordinary Quick Plot session: use **Save as Project** in the dashboard's start menu to keep it as a project file.

## Wire format

A declaration is a single line ending in a newline:

```
#ss1{"title":"TinyGPS","groups":[{"title":"GPS Map","widget":"gps","datasets":[{"title":"Latitude","units":"°","widget":"lat"},{"title":"Longitude","units":"°","widget":"lon"},{"title":"Altitude","units":"m","widget":"alt"}]}]}
```

- `#ss` is the reserved prefix. The digit after it is the wire version, currently `1`. An unknown version is ignored with a console message.
- The rest of the line is a JSON object with a `title` and a `groups` array.
- A group has a `title`, an optional `widget` (`datagrid`, `multiplot`, `accelerometer`, `gyro`, `gps`, `plot3d` or `barpanel`) and a `datasets` array. A group whose datasets do not fit its widget is shown as a data grid, with a console note.
- A dataset takes the same properties the Project Editor shows for a dataset: `title`, `units`, `widget`, `plotMin`, `plotMax`, `widgetMin`, `widgetMax`, `fft`, `alarmBands`, `labels`, and the rest. A property that is not a dataset property does not exist on the wire; unknown keys are dropped and listed in the console.
- Dataset order is frame order. A dataset takes the next frame position, unless it names an `index`, which reuses that position so two groups can show the same channel.
- Frames after a declaration are plain values in declared order, `1.5,2.5,3.5`, or named values, `Latitude:48.8584`, which map by name.

A declaration is data. It never carries a script, a transform, a file path or a connection setting. Titles and labels are capped at 64 bytes, a line at 16384 bytes, and a declaration at 64 groups and 512 datasets. A malformed, oversized or over-cap declaration is ignored with a console message.

### Declared controls

A group with `"groupType":1` holds `outputWidgets` instead of datasets. Each control has a `title` and an `outputType`: `0` button, `1` slider, `2` toggle, `4` knob. Sliders and knobs take `outputMin`, `outputMax` and `outputStep`.

Serial Studio generates an Output Panel whose controls send:

```
#ss!Speed=128
```

A control title is limited to letters, digits, spaces, `_`, `.` and `-`, at most 32 bytes. The library parses the command and hands the sketch the control's name and value. When Serial Studio tracks replies, it appends `@` and a token to the value, and the library strips it.

The controls appear on the dashboard as [Output Controls](Output-Controls.md), through the Serial Studio library command template.

## The library

All three variants share one wire format. Each pass declares (when a declaration is due) or sends a frame (otherwise): the call that sends a value also declares it. The first pass after power-up, and the first pass after every `#ss?` request, writes the declaration and drops that pass's values.

### Arduino

Install **SerialStudio** from the Arduino Library Manager, or copy `firmware/arduino` to `Arduino/libraries/SerialStudio`. The header allocates nothing at run time and keeps its declaration text in flash.

```cpp
#include <SerialStudio.h>

#if SS_LIBRARY_VERSION_MAJOR != 1
#error This sketch needs version 1 of the SerialStudio library
#endif

SerialStudio studio(Serial, F("TinyGPS"));

void setup() { Serial.begin(115200); }

void loop() {
  studio.poll();

  studio.group(F("GPS Map"), F("gps"));
  studio.channel(F("Latitude"), 48.8584, 4).units(F("\xc2\xb0")).widget(F("lat")).graph(false);
  studio.channel(F("Longitude"), 2.2945, 4).units(F("\xc2\xb0")).widget(F("lon")).graph(false);
  studio.channel(F("Altitude"), 35.0, 1).units(F("m")).widget(F("alt")).graph(false);
  studio.send();
}
```

| Call | Effect |
|------|--------|
| `group(title, widget)` | Opens a dataset group. The widget is optional. |
| `channel(name, value, places)` | Declares and sends one channel. Without a value, the channel is declared only. |
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

Copy `firmware/c/serialstudio.c`, `serialstudio.h` and `ss_vocabulary.h` into the project. The file is C99 and needs no RTOS glue. Output goes through a write callback; input arrives through `ss_feed()`.

```c
ss_t ss;
ss_init(&ss, write_to_uart, NULL, "TinyGPS");

ss_group(&ss, "GPS Map", "gps");
ss_channel_double(&ss, "Latitude", lat, 4);
ss_units(&ss, "\xc2\xb0");
ss_widget(&ss, "lat");
ss_send(&ss);
```

`ss_group`, `ss_channel_int`, `ss_channel_double`, `ss_channel_str`, `ss_control_group` and `ss_control` open groups and declare channels. Property macros such as `ss_units`, `ss_widget`, `ss_plot_min`, `ss_range` and `ss_alarm` apply to the channel just opened. `ss_set_handlers()` registers the control and line callbacks.

### Python

Install with `pip install serialstudio`. The module uses the standard library only and writes bytes through a callable, so it works over a serial port, a socket or a pipe.

```python
import serialstudio

device = serialstudio.Device("TinyGPS", write=port.write)

device.group("GPS Map", "gps")
device.channel("Latitude", 48.8584, places=4, units="°", widget="lat", graph=False)
device.channel("Longitude", 2.2945, places=4, units="°", widget="lon", graph=False)
device.channel("Altitude", 35.0, places=1, units="m", widget="alt", graph=False)
device.send()
```

Dataset properties are keyword arguments in snake case (`plot_max=5`, `fft_sampling_rate=200`). Pass `range=(0, 100)` to set both the widget and plot range. `Device(title, write, separator=",", named=False, on_control=None, on_line=None)` takes the callbacks; `feed(data)` buffers received bytes and `poll()` dispatches them. `serialstudio.tcp_server(device)` serves a device over TCP on localhost, for testing against a Network source.

Python raises `ValueError` past a cap. The C and Arduino variants stop declaring past it.

## Binary frames

A device that sends packed binary frames declares their layout in the same line. The declaration itself is always text; only the frames after it are binary. The line gains a `sources` array with one entry that names the framing and the `packed_struct` frame parser template:

```
#ss1{"title":"MPU6050","sources":[{"frameStart":"AA55","hexadecimalDelimiters":true,"frameDetection":3,"checksumAlgorithm":"CRC-16","decoderMethod":3,"frameParserTemplate":"packed_struct","frameParserParams":{"fields":"f32,f32,f32,f32,f32,f32,f32","endianness":"little"}}],"groups":[...]}
```

Each frame is the start delimiter, the fields in declared order and a checksum. `fields` lists `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f32` or `f64` per channel, and `endianness` is `little` or `big`. A frame whose length does not match the layout is dropped. The `connection` of a source is never read from a declaration.

The libraries send the frames for you:

```python
device = serialstudio.Device("MPU6050", write=port.write)
device.binary("f32,f32,f32,f32,f32,f32,f32", "little", b"\xaa\x55", "CRC-16")
```

The C library offers `ss_set_binary()` and the Arduino header `binary()`. The libraries write the `CRC-16` checksum only. On an AVR board `double` has 32 bits, so `f64` fields carry single-precision values.

When Serial Studio switches to the declared framing, the first frames after a declaration can be dropped. A device that declares again sends the declaration as a text line inside the binary stream. Serial Studio finds it in the raw bytes of the connection, and a changed declaration clears the binary framing and rebuilds the dashboard from the new one. Sending the same declaration again changes nothing.

The acknowledged-reply fields of declared controls are appended to text frames only.

## Adopting a declaration

In Project File mode, a declaration never rebuilds the loaded project. Serial Studio only compares it with the loaded project. When it differs, one notification reports that the device declares a different dashboard, and two buttons appear in the Setup panel under the project file: **Use Device Dashboard** and **Keep Project**.

Use Device Dashboard disconnects, offers to save unsaved edits, and opens the declared project as a new, unsaved project with the loaded project's connection. The loaded file stays untouched on disk: reopening it reverses the choice.

Keep Project dismisses the offer for this project and this declaration. It comes back only when the device declares something different or another project file is loaded. Disconnecting forgets the declaration, because the next connection may be another device.

Adopting is the only way a declaration reaches Project File mode. To keep a Quick Plot dashboard instead, use **Save as Project**.

## Limits

- Declaration replaces format detection only in Quick Plot mode.
- A device that declares nothing needs no library: Quick Plot still detects its format.

## See also

- [Operation Modes](Operation-Modes.md)
- [Getting Started](Getting-Started.md)
- [Output Controls](Output-Controls.md)
- [Project Editor](Project-Editor.md)
- [Auto-Generating Projects](Auto-Generating-Projects.md)
