# Serial Studio device library

A sketch declares its dashboard on the wire, and Serial Studio builds it on connect without a
project file. The same `channel()` call that sends a value also declares it: the units, range,
widget and group sit in the line where the sketch already prints the value.

The library comes in three variants with one wire format: an Arduino header, a C99 file and a
Python module. It is MIT licensed, so it can be embedded in closed firmware.

## Requirements

Serial Studio in Quick Plot mode, from the first release that carries Device Declarations. That
release also carries the Quick Plot line formats the declaration line relies on. Releases up to
4.2.1 rebuild the dashboard on any text line and cannot use it.

The app sends one `#ss?` request line when it connects in Quick Plot. The library answers it, so
a device that declared at boot before the app was listening is still discovered. The request can
be turned off in the Setup panel.

## Install

**Arduino.** Install "SerialStudio" from the Library Manager, or copy `firmware/arduino` to
`Arduino/libraries/SerialStudio`. The header allocates nothing, keeps its text in flash and has
no dependency beyond the Arduino core.

**C.** Copy `firmware/c/serialstudio.c`, `serialstudio.h` and `ss_vocabulary.h` into the project.
It is C99 with no RTOS glue. Output goes through a write callback and input arrives through
`ss_feed()`.

**Python.** `pip install serialstudio`. The module uses the standard library only and is
transport-agnostic: it writes bytes through a callable and takes input with `feed()`.

## Use

```cpp
#include <SerialStudio.h>

SerialStudio studio(Serial, F("MPU6050"));

void loop()
{
  studio.poll();

  studio.group(F("Accelerometer"), F("accelerometer"));
  studio.channel(F("Accelerometer X"), ax).units(F("m/s\xc2\xb2")).widget(F("x"));
  studio.channel(F("Accelerometer Y"), ay).units(F("m/s\xc2\xb2")).widget(F("y"));
  studio.channel(F("Accelerometer Z"), az).units(F("m/s\xc2\xb2")).widget(F("z"));
  studio.send();
}
```

On the first pass after power-up, and after every `#ss?` request, `send()` writes the declaration
line instead of a frame and that pass's values are dropped. Every other pass writes one frame.
Call `poll()` once per loop so requests and commands are read.

| Call (Arduino / C / Python) | Effect |
|-----------------------------|--------|
| `group(title, widget)` / `ss_group` / `group` | Opens a dataset group. The widget is optional. |
| `channel(name, value)` / `ss_channel_*` / `channel` | Declares and sends one channel. Without a value, the channel is declared only, which is how a second group shows a channel by `index`. |
| `units`, `widget`, `range`, `plotMin`, `plotMax`, `widgetMin`, `widgetMax`, `fft`, `index`, ... | Dataset properties. Each is a key of the dataset registry. Python takes any of them as a keyword. |
| `alarm(min, max, severity, label)` | Adds an alarm band. |
| `label(value, text)`, `labelsDefault(text)` | Names the values of a state channel. |
| `graph(false)` | Plots are on by default. This turns them off. |
| `controlGroup(title)`, `control(title, kind)` | Declares an Output Panel with a button, slider, toggle or knob. |
| `named(true)`, `separator(c)` | Sends `name:value` frames, or uses another separator. |
| `control(title, text)` with `outputMin`, `outputMax`, `outputUnits`, `outputDecimals` | Declares a numeric text field (Python: `min=`, `max=`, `units=`, `decimals=`). |
| `onControl(fn)`, `onLine(fn)` | Receives `#ss!name=value` commands, and every other input line. The control callback returns the reply code: `0` accepted, anything else refused. A message goes through `replyMessage()` (C: `ss_reply_message()`; Python: return `(code, message)`). |
| `binary(fields, endianness, start)` / `ss_set_binary` | Sends packed binary frames with a start delimiter and CRC-16 instead of text lines. |

Once a tokened command (`#ss!name=value@token`) has been answered, every text frame ends with the token, the code and the message, which is how the app shows a control as acknowledged or refused.

Strings are capped at 64 bytes, groups at 64, datasets at 512 and controls at 64. The Python
module raises `ValueError` past a cap; the C and Arduino variants stop declaring past it.

A declaration is data. It carries no code, no file path and no connection setting.

## Tests

`python3 firmware/tests/check_golden.py` compares each library's output for the reference
sketches with `firmware/tests/golden`, byte for byte. `python3 -m unittest discover
firmware/python/tests` runs the Python cases. CI builds the C and Arduino host drivers, so
`check_golden.py` covers all three variants there.
