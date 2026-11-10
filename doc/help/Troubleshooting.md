# Troubleshooting

The Console shows text and the dashboard stays empty, or the numbers are wrong. Start with the [Problem Center](Problem-Center.md): press **Ctrl+K**, then type `Problem Center`. It lists the project mistakes, link conditions, and script errors Serial Studio detected, each with a remedy. **Connection Diagnostics** (also in **Ctrl+K**) checks port permissions, Bluetooth state, and host reachability for you.

## Arduino prints values but the dashboard is empty

Take the smallest case: a sketch that ends each reading with a newline.

```cpp
void setup() { Serial.begin(115200); }
void loop() { Serial.println("1.5,2.5,3.5"); delay(100); }
```

Work down this list and stop at the first step that fails.

1. In the **Setup Panel**, select **UART/COM** as the connection, pick the board under **Serial Port** (**COM Port** on Windows), and set **Baud Rate** to the number in `Serial.begin()`. The default is 9600.
2. Set **Operation Mode** to **Quick Plot (Auto-Detected Format)**. The sketch's comma-separated line then plots with no project.
3. Click **Connect** and look at the Console. If the text appears there but the dashboard stays empty, the link works and the problem is framing or parsing: continue with [Frames not being detected](#frames-not-being-detected) and [Data shows as "0" or "NaN"](#data-shows-as-0-or-nan).
4. If the Console stays empty, the bytes never arrive: continue with [Connected but no data appearing](#connected-but-no-data-appearing).

A line ending in `\n` is a frame, and the comma-separated values in it are the dataset values. The [Getting Started](Getting-Started.md) guide shows this end to end in Quick Plot mode, and [Data Flow](Data-Flow.md) defines frames, decoders, and parsers.

## Installation Issues

### Windows: "Missing VCRUNTIME140.dll" or similar error

Windows reports that the Visual C++ runtime is missing. Install the **Microsoft Visual C++ Redistributable** from [Microsoft's download page](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) and start Serial Studio again.

### macOS: "App is damaged and can't be opened" or "Unidentified developer"

macOS blocks the app because of its quarantine flag. For the "damaged" error, clear the flag in Terminal and open the app again. Substitute the bundle name shown in /Applications (for example `Serial-Studio-GPL3.app` for the GPL build).

```bash
xattr -cr "/Applications/Serial-Studio-Pro.app"
```

For "unidentified developer", open **System Settings > Privacy & Security** and click **Open Anyway** next to the Serial Studio message, or right-click the app and select **Open**.

### Linux: AppImage won't run ("Permission denied")

The downloaded file is not marked executable. Mark it and run it:

```bash
chmod +x Serial-Studio-*.AppImage
./Serial-Studio-*.AppImage
```

### Linux: AppImage error "fuse: failed to exec fusermount"

The AppImage mounts itself through FUSE and the host has no working FUSE setup. Extract it and run the contents directly:

```bash
./Serial-Studio-*.AppImage --appimage-extract
./squashfs-root/AppRun
```

### Linux: Serial port permission denied

Opening the port fails with "permission denied". Your user must belong to the group that owns serial devices: `dialout` on most distributions, `uucp` on Arch-based systems.

```bash
sudo usermod -a -G dialout $USER
```

Log out and back in, then run `groups` to confirm. [Serial port permissions](Linux-Installation.md#serial-port-permissions) has the details.

## Connection Issues

Run **Connection Diagnostics** first (**Ctrl+K**, then `Connection Diagnostics`). It reports denied serial port permissions with the exact command to run, a switched-off or unpermitted Bluetooth adapter, and a network host that does not resolve or refuses the port. See [Problem Center](Problem-Center.md#connection-diagnostics).

### Serial Port

#### Port not visible in dropdown

In the **Setup Panel**, select **UART/COM** as the connection first; the port list belongs to that driver. If the board is still missing, the operating system does not see it either:

- **Windows:** open Device Manager (Win+X, then Device Manager) and look under "Ports (COM & LPT)". A yellow warning icon means the driver is missing. Common ones: [CH340/CH341](https://www.wch-ic.com/downloads/CH341SER_EXE.html), [FTDI](https://ftdichip.com/drivers/vcp-drivers/), and [CP210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).
- **Linux:** run `ls /dev/tty* | grep -E 'ttyUSB|ttyACM'`. If the port is listed but cannot be opened, see [Serial port permission denied](#linux-serial-port-permission-denied).
- **macOS:** run `ls /dev/tty.*` and look for `usbmodem` or `usbserial`.

Also try another cable and another USB port. A charge-only cable powers the board and shows nothing else.

#### "Failed to open port" error

Another program holds the port until you close it. Close the Arduino IDE Serial Monitor, PuTTY, Screen, Minicom, Tera Term, and any second Serial Studio instance. If nothing else is running, unplug the device, wait a few seconds, plug it back in, and reselect the port. On Linux, check [the permission fix](#linux-serial-port-permission-denied).

#### Connected but no data appearing

Check these in order:

1. **Baud Rate** matches the device. The default is 9600; a sketch with `Serial.begin(115200)` needs 115200. If you do not know the rate, try 9600, 19200, 38400, 57600, and 115200.
2. The selected port is the right one. Unplug the device and see which entry disappears.
3. The device is transmitting. Open the same port in another terminal, or check the sketch calls `Serial.println()`.
4. In the UART setup, **Send DTR Signal** is on by default. Clear it if opening the port resets your board; many Arduino boards reset when DTR changes. If a native-USB board stays silent, leave it on.
5. The Console shows text but the dashboard stays empty: the frame delimiters do not match. See [Frames not being detected](#frames-not-being-detected).

#### Garbled or unreadable console output

Data arrives, but the Console shows nonsense instead of readable text. Random accented characters, or a scatter of `?` and diamond symbols, mean the UART clock and the device disagree on bit timing: match **Baud Rate** to the device.

If the text is readable except for the same few characters, wrong in the same places every time (a degree symbol that always shows as two odd characters), the bytes arrive intact and the device sends non-ASCII characters. Changing the baud rate does not help. Run **Hex Display** from **Ctrl+K** to see the raw bytes.

### Network Socket (TCP/UDP)

See [Network Sockets](Drivers-Network.md) for every field.

#### Cannot connect to TCP socket

1. Confirm **Remote Address** and **Remote Port** match the device, and that **Socket Type** is TCP, not UDP. `ping <ip-address>` checks the address.
2. Test the listener from a terminal: `telnet <ip-address> <port>`. "Connection refused" means the device is not listening; a hang usually means a firewall is dropping the connection.
3. Allow Serial Studio through the firewall: Windows Defender Firewall (Allow an app), macOS System Settings > Privacy & Security > Firewall, or `sudo ufw allow <port>` on Linux.
4. Check that the computer and the device are on the same network.

#### UDP not receiving data

**Local Port** is the usual cause: it defaults to `0`, and then the OS picks a random port and nothing reaches Serial Studio. Set it to the port the device sends to.

If that is already right:

- Check the device sends to this computer's current IP. DHCP may have changed it (`ipconfig` on Windows, `ip addr` on Linux and macOS).
- Allow Serial Studio through the firewall; UDP is blocked more often than TCP.
- Run Wireshark to see whether the datagrams arrive at all.

### Bluetooth LE

See [Bluetooth LE](Drivers-Bluetooth-LE.md) for every field. Bluetooth LE is available in both editions.

#### BLE device not found during scan

Many BLE peripherals stop advertising once a phone or another app connects to them. Disconnect the device elsewhere, power-cycle it, and keep it within a metre or two of the computer. Then check that the computer's Bluetooth is on, and run **Connection Diagnostics** (**Ctrl+K**), which reports a switched-off or unpermitted adapter.

#### Connected to BLE but no data

Select the **Service**, then the **Characteristic** that carries the telemetry, the one with the Notify property. Serial Studio enables notifications on a characteristic when you select it (it writes the CCCD descriptor), so there is nothing to switch on by hand. A characteristic that is Indicate-only, or has no CCCD, delivers nothing. Serial Studio does not show characteristic properties, so check them in the firmware.

## Dashboard & Visualization Issues

The [Problem Center](Problem-Center.md) lists the project mistakes behind most empty or wrong widgets: a dataset with no frame position, two datasets reading the same index, a plot axis pointing at a deleted dataset, or an inverted range. Each finding has a **Go To** button that opens the Project Editor at the item to fix.

### Data shows as "0" or "NaN"

A dataset shows 0 or NaN when its frame index points past the parser's output or at the wrong value. The first value your parser returns feeds the dataset with frame index 1, the second feeds index 2, and so on. [Parser returns the wrong number of values](#parser-returns-the-wrong-number-of-values) shows how to find the mismatch.

If the indices are right, the parser itself is failing: errors appear in the Console, and [Parser function not working](#parser-function-not-working) shows how to log what it receives. Also check that the raw frames in the Console match the format the parser expects.

### Widgets not displaying or showing errors

- **Wrong data for the widget:** the Accelerometer widget needs exactly 3 datasets (X, Y, Z) and the GPS widget needs latitude and longitude. A gauge cannot show text and a bar is a poor fit for unbounded values. [Widget Reference](Widget-Reference.md) lists the requirements per widget.
- **Broken widget:** delete the widget from the project and add it again with a fresh configuration.

### Dashboard layout broken after update

Loading a project saved by an older Serial Studio version resets a customized workspace layout to the automatic one, on purpose: an empty custom layout is worse than an automatic one. Rearrange the workspaces and save the project. If datasets or widgets are missing rather than moved, report it as a bug.

## Frame Parsing Issues

While connected, the [Problem Center](Problem-Center.md) tells the two failure stages apart: `bytes-without-frames` means the delimiters do not match the stream, `frames-without-values` means frames arrive but the parser returns nothing, and `script.parser` findings show the parser's own errors or that the watchdog switched it off for overrunning its time budget.

### Frames not being detected

The Console shows bytes arriving, but the dashboard receives no frames. The delimiters in the project do not match what the device sends.

1. Open the Console and look at the bytes. Delimiters are case-sensitive and a stray space counts; the Console's **Hex Display** command (**Ctrl+K**) shows hidden characters.
2. A plain `Serial.println` ends each line with `\r\n`, and the project's default end delimiter `\n` covers it. A device that ends lines with a bare `\r` needs its end delimiter changed in the Project Editor.
3. Frame delimiters, decoder and parser are set per source in the Project Editor (click **Project Editor** in the Setup Panel): the **Frame Detection Method**, **Frame Start Delimiter** and **Frame End Delimiter** fields. **Test With Sample Data** lets you paste a frame and adjust detection until it splits correctly.
4. For binary protocols, **No Delimiters** treats each captured chunk as one frame, which suits length-prefixed protocols; the parser must find the boundaries inside it.

### Parser returns the wrong number of values

**Problem:** some widgets stop updating while others keep moving, or a widget never shows a value, although frames arrive.

**Cause:** each dataset reads one frame index, the 1-based position of its value in the parser's output. When the parser returns fewer values than a dataset's index, that dataset receives nothing and keeps its last value, so it looks frozen rather than empty. The usual reasons are a device that sends fewer fields than the project expects, a separator that does not match, or frame indices that are off by one (the first value is index 1, not 0).

**How to find it:**

1. Open the [Problem Center](Problem-Center.md). A `short-frames` warning names each affected device, the number of values its frames carry, the highest frame index the project maps, and the datasets that receive nothing. **Go To** opens the first of them.
2. In the Project Editor, open the device's frame parser page and click **Test With Sample Data**. Paste one real frame. Expand a row: each value is labelled with its frame index and the dataset that reads it, and datasets whose index the frame does not reach are listed as not in this frame.

**Fix:** make the device send every expected value, correct the parser or its separator, or set the affected datasets' frame index to the position of their value in the frame.

### Parser function not working

The Console shows `SyntaxError`, `TypeError`, or Lua errors, or the parser returns the wrong data. Edit the parser in the Project Editor (click **Project Editor** in the Setup Panel), on the device's frame parser page, and read its output in the Console.

1. **Log what the parser receives.** `console.log()` works in JavaScript and Lua; the output appears in the Console.

   ```javascript
   function parse(frame) {
       console.log("Type:", typeof frame, "Length:", frame.length, "Content:", frame);
       let result = frame.split(',');
       console.log("Result:", result);
       return result;
   }
   ```

2. **Return an array** (a table in Lua), not a string: `return [1, 2, 3];` works, `return "1,2,3";` does not.
3. **Match the decoder.** The source's **Decoder** (Project Editor, source settings; default Plain Text) sets the type of `frame`. Plain Text, Hexadecimal and Base64 give a string; Binary (Direct) gives a byte array, 0-indexed in JavaScript and 1-indexed in Lua. The table is in [Frame Parser Scripting](Frame-Parser.md). A string function on a byte array produces the error in [Parse error: "string expected, got table"](#parse-error-string-expected-got-table--framesplit-is-not-a-function).

### Parse error: "string expected, got table" / "frame.split is not a function"

The Console repeats one of these on every frame:

- Lua: `bad argument #1 to 'byte' (string expected, got table)`
- Lua: `attempt to index a table value (local 'frame')`
- JavaScript: `TypeError: frame.split is not a function` (or `frame.match`, `frame.charCodeAt`)

The source uses the **Binary (Direct)** decoder, which hands `parse()` the frame as raw byte values (a 1-indexed table in Lua, an array in JavaScript), but the parser code was written for a string input.

1. Index the bytes directly instead of using string functions:

   ```lua
   -- Lua: frame[1] is the first byte (1-indexed)
   function parse(frame)
     if #frame < 4 then return {} end
     return {
       bit.bor(bit.lshift(frame[1], 8), frame[2]),
       bit.bor(bit.lshift(frame[3], 8), frame[4])
     }
   end
   ```

   ```javascript
   // JavaScript: frame[0] is the first byte (0-indexed)
   function parse(frame) {
       if (frame.length < 4) return [];
       return [(frame[0] << 8) | frame[1], (frame[2] << 8) | frame[3]];
   }
   ```

2. For multi-byte fields, decode from the byte table with the `bit` library and arithmetic, because LuaJIT has no `string.pack` or `string.unpack`:

   ```lua
   function parse(frame)
     if #frame < 4 then return {} end
     local value = bit.lshift(frame[1], 8) + frame[2]  -- big-endian uint16
     return { value }
   end
   ```

3. Or switch the decoder to Plain Text or Hexadecimal in the Project Editor if the device sends text. The decoder and the parser must agree on the frame type; the decoder table is in [Frame Parser Scripting](Frame-Parser.md).

### Dataset transform not working

**Problem:** You applied a `transform(value)` function but the dashboard still shows raw values.

**Debugging steps:**

1. **Check the function is named `transform`.** It has to be exactly `function transform(value)` (Lua) or `function transform(value) {` (JavaScript). The name is case-sensitive.

2. **Click Apply in the transform editor.** Changes are only saved when you click Apply. Closing the dialog without Apply discards changes.

3. **Test with the built-in Test area.** Enter a raw value in the Input field and click Test. If you see an error, fix the function before applying.

4. **Handle non-numeric datasets.** The transform receives the raw value as a number when it parses as numeric, otherwise as a string. A non-numeric value like "N/A" is passed to `transform()` as a string, not skipped, so make sure your function handles the string case.

5. **Check the Problem Center.** A transform that produces errors raises a `script.transform` warning there ([Problem Center](Problem-Center.md)). The compiler's own message goes to the application log, so it is visible in a terminal only if you launch Serial Studio from one.

6. **Save and reload the project.** If the transform was applied during a live session, saving makes sure it persists. After reload, transforms are recompiled automatically.

See [Dataset Value Transforms](Dataset-Transforms.md) for the complete reference.

### Parsing slow/laggy

A parser that overruns its time budget is switched off by the watchdog (a `script.parser` finding in the Problem Center). To keep it fast, use `split(",")` instead of a regular expression, return a flat array, and avoid per-frame object creation. To measure it, time the call:

```javascript
function parse(frame) {
    let start = Date.now();
    let result = frame.split(',');
    let elapsed = Date.now() - start;
    if (elapsed > 1)
        console.log("Parse took", elapsed, "ms");
    return result;
}
```

For a Lua parser, the Fast mode described in [Frame Parser Scripting](Frame-Parser.md) runs much faster but disables the watchdog.

## CSV Player Issues

### Cannot open CSV file

Serial Studio's own exports always open, so record a short session and compare your file with it. The player expects a header row and a first column that holds elapsed time or a timestamp it can recognise: `Elapsed (s)` in Serial Studio's exports, a column with the unit in the name such as `time(ms)`, or date/time strings. Exports are UTF-8 with a byte-order mark. A multi-source file also needs the project that produced it loaded first. [CSV Export & Playback](CSV-Export-Playback.md#opening-a-csv-file) has the accepted formats.

### Playback is choppy

The CSV Player replays at the original capture speed; there is no playback-speed control. For very large or high-rate files, split the file into smaller segments or keep only the columns you need.

## Pro Feature Issues

### MQTT connection fails

MQTT is a Pro feature. See [MQTT Subscriber](Drivers-MQTT.md) and [MQTT Publisher](MQTT-Publisher.md). Run **Test Connection** on the Publisher form to probe broker reachability without disturbing live publishing.

Two instances or sources sharing one client ID kick each other off. If the IDs are distinct, check the broker address and port, the credentials, and the TLS certificates.

### Modbus connection fails

Modbus is a Pro feature. Check, in order:

- **Slave Address** matches the device (1-247, default 1).
- **Register Type**, **Start Address**, and **Register Count** match the device manual.
- RTU: baud rate, parity, and RS-485 wiring polarity. TCP: IP address and port.

See [Modbus RTU Setup](Protocol-Setup-Guides.md#modbus-rtu-setup-pro) and [Modbus TCP Setup](Protocol-Setup-Guides.md#modbus-tcp-setup-pro).

### CAN Bus issues

CAN Bus is a Pro feature. Check the bitrate against the bus, termination resistors at both ends, that the adapter is recognized, and that the DBC file loads without format errors. See [CAN Bus Setup](Protocol-Setup-Guides.md#can-bus-setup-pro).

### A Pro feature shows a trial or activation prompt

This applies to Serial Studio Pro in the official binary. The first time you use a Pro feature, Serial Studio asks **Start your free 14-day Serial Studio Pro trial?** with **Yes** and **No**. Registering the trial needs an internet connection. Answer **No** and the feature stays unavailable until you use it again and say Yes.

**Trial ended:** the prompt offers **Activate License** or **Get Serial Studio Pro** instead. The trial is not offered a second time on the same machine; the free features keep working.

**Transforms and Variables:** a project containing a dataset transform or a Variables table needs Pro to connect, and the prompt lists how many. Right-click the dataset and choose **Remove Transform**, or delete the table, to run the project for free.

**Check status:** click **About** in the toolbar, then **Manage License**, to open the Licensing dialog. While a trial runs, the Console welcome text shows the days remaining. After it ends, it shows `Pro trial expired. Free features remain fully functional.`

**Activate a key:** paste it in the Licensing dialog and click **Activate**. Activation needs an internet connection.

**Trial rules:** one 14-day trial per version and per machine; reinstalling does not reset it, and a new Serial Studio release starts a fresh one ([Pro vs Free](Pro-vs-Free.md)).

**Compiled from source:** a build with only the GPL modules has no Pro features and cannot activate. Install the official binary from [serial-studio.com](https://serial-studio.com).

## Performance Issues

### High CPU usage

The usual causes are a high data rate, a heavy parser, many widgets, and 3D widgets on a slow CPU. Use fewer plots, replace 3D widgets where a flat one does the job, and keep the parser to simple operations such as `split(",")`. A lower point count on the plots also helps; it is a setting on the Settings dialog's plotting page.

### Memory usage growing over time

A parser that keeps a global array grows without bound unless it caps it:

```javascript
let history = [];
function parse(frame) {
    history.push(frame);
    if (history.length > 1000)
        history.shift();
    return frame.split(',');
}
```

Without the `shift()` cap, `history` grows forever. Long sessions with a long plot history also hold more points.

### Slow frame rate / laggy dashboard

3D Plot is the only widget that renders a real 3D scene; swap it for a Plot or a Gauge where a full 3D view is not needed.

The dashboard redraws at 60 Hz by default. **UI Refresh Rate (Hz)** on the Settings dialog's plotting page accepts 1 to 240. A very high data rate still adds CPU load parsing and updating datasets between redraws.

## Getting More Help

Ask questions on [GitHub Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions). For a bug, search the [issues](https://github.com/Serial-Studio/Serial-Studio/issues) first, closed ones included, then [open a new one](https://github.com/Serial-Studio/Serial-Studio/issues/new) with the details listed under [Reporting Bugs Effectively](#reporting-bugs-effectively). [DeepWiki](https://deepwiki.com/Serial-Studio/Serial-Studio) answers questions from the repository, though its answers may differ from this manual. Pro license holders can email alex@serial-studio.com with their order email.

## Reporting Bugs Effectively

Include these in an issue:

**System Information:**
- [ ] Serial Studio version and build. The toolbar's **About** button shows `Version 4.0.3 (abcdef1)`; click that line to copy the full build identity, commit included. A build compiled locally reads `local build` instead of a commit.
- [ ] Operating system and version
- [ ] How you installed (official binary, compiled from source, AppImage, etc.)

**Problem Description:**
- [ ] Clear description of the problem
- [ ] Steps to reproduce (numbered list)
- [ ] Expected behavior
- [ ] Actual behavior

**Additional Information:**
- [ ] Screenshots or screen recording (if applicable)
- [ ] Console output (copy from console panel)
- [ ] Sample project file (if project-specific)
- [ ] Sample data (a few frames or CSV file)

**Example Good Bug Report:**

```text
## Problem
Dashboard widgets show "NaN" instead of values

## System
- Serial Studio v4.0.3 (a1b2c3d)
- Windows 11 Pro
- Official binary installer

## Steps to Reproduce
1. Connect Arduino Uno via USB (COM3)
2. Configure: Baud 115200, delimiter: \n
3. Load attached project file
4. Open dashboard

## Expected
Gauges should show temperature values (20-30 C)

## Actual
All gauges show "NaN"

## Additional Info
- Console shows the parser error text
- Attached: project.json, sample_data.csv
- Screenshot: screenshot.png
```

## See Also

- [Getting Started](Getting-Started.md): first-time setup guide
- [Devices](Devices.md): connection configuration
- [Data Flow](Data-Flow.md): how parsing works
- [Frame Parser Reference](Frame-Parser.md): Built-In, Lua and JavaScript parser reference
- [Widget Reference](Widget-Reference.md): widget requirements and usage
- [FAQ](FAQ.md): frequently asked questions
