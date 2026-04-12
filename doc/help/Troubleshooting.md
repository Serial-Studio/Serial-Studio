Quick solutions to common Serial Studio issues. If you can't find your problem here, check the [Getting More Help](#getting-more-help) section at the bottom.

## Table of Contents

- [Installation Issues](#installation-issues)
- [Connection Issues](#connection-issues)
- [Dashboard & Visualization Issues](#dashboard--visualization-issues)
- [Frame Parsing Issues](#frame-parsing-issues)
- [CSV Player Issues](#csv-player-issues)
- [Pro Feature Issues](#pro-feature-issues)
- [Performance Issues](#performance-issues)
- [Getting More Help](#getting-more-help)

---

## Installation Issues

### Windows: "Missing VCRUNTIME140.dll" or similar error

**Problem:** Application won't launch, showing error about missing Visual C++ runtime DLLs.

**Solution:**
1. Download and install the **Microsoft Visual C++ Redistributable**
2. Get it from: [Microsoft's official download page](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)
3. Install both x64 and x86 versions if unsure
4. Restart Serial Studio

---

### macOS: "App is damaged and can't be opened" or "Unidentified developer"

**Problem:** macOS prevents Serial Studio from opening due to security settings.

**Solution:**
1. **For "damaged" error:**
   ```bash
   xattr -cr /Applications/Serial\ Studio.app
   ```
   Run this in Terminal, then try opening again

2. **For "unidentified developer":**
   - Go to **System Preferences > Security & Privacy**
   - Click "Open Anyway" button next to Serial Studio message
   - Or right-click Serial Studio and select "Open"

---

### Linux: AppImage won't run ("Permission denied")

**Problem:** Downloaded AppImage file won't execute.

**Solution:**
1. Make the AppImage executable:
   ```bash
   chmod +x Serial-Studio-*.AppImage
   ```

2. Run it:
   ```bash
   ./Serial-Studio-*.AppImage
   ```

---

### Linux: AppImage error "fuse: failed to exec fusermount"

**Problem:** AppImage requires FUSE to run.

**Solution:**
Install libfuse2:

**Ubuntu/Debian:**
```bash
sudo apt install libfuse2
```

**Fedora:**
```bash
sudo dnf install fuse-libs
```

**Arch Linux:**
```bash
sudo pacman -S fuse2
```

---

### Linux: Serial port permission denied

**Problem:** Serial Studio can't access serial ports (permission denied error).

**Solution:**
1. Add your user to the `dialout` group:
   ```bash
   sudo usermod -a -G dialout $USER
   ```

2. **Important:** Log out and log back in for changes to take effect

3. Verify you're in the group:
   ```bash
   groups $USER
   ```
   You should see `dialout` in the list

**Alternative (temporary, not recommended):**
```bash
sudo chmod 666 /dev/ttyUSB0  # Replace with your port
```

---

## Connection Issues

### Serial Port

#### Port not visible in dropdown

**Possible causes:**
- Device not connected
- Driver not installed (Windows)
- Permission issues (Linux/macOS)
- Port already in use

**Solutions:**

1. **Verify device is connected:**
   - Check USB cable is plugged in
   - Try a different USB port
   - Check device has power (LED indicators)

2. **Check device is recognized by OS:**

   **Windows:**
   - Open Device Manager (Win+X → Device Manager)
   - Look under "Ports (COM & LPT)"
   - If you see a yellow warning icon, driver is missing

   **Linux:**
   ```bash
   ls /dev/tty* | grep -E 'ttyUSB|ttyACM'
   ```

   **macOS:**
   ```bash
   ls /dev/tty.*
   ```

3. **Install driver if needed** (Windows):
   - **CH340/CH341**: [CH340 driver](http://www.wch-ic.com/downloads/CH341SER_EXE.html)
   - **FTDI**: [FTDI driver](https://ftdichip.com/drivers/vcp-drivers/)
   - **CP210x**: [Silicon Labs driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

4. **Fix permissions** (Linux): See [Linux serial port permission](#linux-serial-port-permission-denied) above

5. **Close other applications** using the port:
   - Arduino IDE
   - PuTTY / Screen / Minicom
   - Other Serial Studio instances

---

#### "Failed to open port" error

**Possible causes:**
- Port already in use by another application
- Incorrect permissions
- Device disconnected after selection

**Solutions:**

1. **Close competing applications:**
   - Arduino IDE Serial Monitor
   - PuTTY, Screen, Minicom, Tera Term
   - Other terminal programs

2. **Try reconnecting the device:**
   - Unplug USB cable
   - Wait 5 seconds
   - Plug back in
   - Refresh port list in Serial Studio

3. **Check permissions** (Linux): See permission fix above

4. **Try different USB port**

5. **Restart Serial Studio**

---

#### Connected but no data appearing

**Possible causes:**
- Baud rate mismatch
- Device not transmitting
- Wrong COM port selected
- DTR/RTS signal issues

**Solutions:**

1. **Verify baud rate matches device:**
   - Common rates: 9600, 115200
   - Check your device's code or documentation
   - Try common baud rates: 9600, 19200, 38400, 57600, 115200

2. **Verify device is actually transmitting:**
   - Use Arduino IDE Serial Monitor or another terminal to confirm data output
   - Check device code has `Serial.println()` or equivalent
   - Verify device is powered and running (LED blink, etc.)

3. **Double-check selected COM port:**
   - Disconnect device and see which port disappears
   - Reconnect and select that port

4. **Try toggling DTR signal:**
   - Setup Panel → Disable "DTR Signal"
   - Some devices reset when DTR is asserted

5. **Check frame delimiters:**
   - Verify delimiters match what device sends
   - Try "No Delimiters" mode temporarily

---

### Network Socket (TCP/UDP)

#### Cannot connect to TCP socket

**Possible causes:**
- Wrong IP address or port
- Device not listening
- Firewall blocking connection
- Network connectivity issues

**Solutions:**

1. **Verify IP address and port:**
   - Double-check device's IP address (try `ping <ip-address>`)
   - Verify port number matches device configuration
   - Common mistake: Mixing up TCP and UDP

2. **Verify device is listening:**
   - Use telnet to test:
     ```bash
     telnet <ip-address> <port>
     ```
   - If "Connection refused", device isn't listening
   - If it hangs, firewall might be blocking

3. **Check firewall:**
   - Windows: Windows Defender Firewall → Allow an app
   - macOS: System Preferences → Security & Privacy → Firewall
   - Linux: `sudo ufw allow <port>`

4. **Verify network connectivity:**
   - Make sure computer and device are on same network
   - Try `ping <device-ip>`

---

#### UDP not receiving data

**Possible causes:**
- Device sending to wrong IP/port
- Firewall blocking UDP
- Network issues

**Solutions:**

1. **Verify device is sending to correct IP:**
   - Check device configuration
   - Computer's IP address might have changed (DHCP)
   - Find your IP:
     - Windows: `ipconfig`
     - Linux/macOS: `ifconfig` or `ip addr`

2. **Check firewall allows UDP:**
   - UDP is more likely to be blocked than TCP
   - Allow Serial Studio in firewall

3. **Try listening on all interfaces:**
   - Use 0.0.0.0 as listening address (if available)

4. **Use network sniffer to verify packets:**
   - Wireshark can show if UDP packets are arriving

---

### Bluetooth LE

#### BLE device not found during scan

**Possible causes:**
- Device not advertising
- Device already connected to another app
- Bluetooth disabled
- Out of range

**Solutions:**

1. **Verify Bluetooth is enabled:**
   - Check OS Bluetooth settings
   - Ensure Bluetooth adapter is present

2. **Ensure device is advertising:**
   - Reset/power cycle BLE device
   - Check device isn't already connected (phone app, etc.)
   - Many BLE devices stop advertising once connected

3. **Check range:**
   - Move device closer to computer (within 1-2 meters for testing)

4. **Restart Bluetooth:**
   - Turn Bluetooth off and on
   - Or restart computer

---

#### Connected to BLE but no data

**Possible causes:**
- Wrong BLE service/characteristic
- Device not sending notifications
- Need to enable notifications

**Solutions:**

1. **Verify correct service and characteristic:**
   - Check device documentation for UUIDs
   - Use BLE scanner app to verify service/characteristic

2. **Ensure notifications are enabled:**
   - Serial Studio should automatically enable notifications
   - Try disconnecting and reconnecting

---

## Dashboard & Visualization Issues

### Data shows as "0" or "NaN"

**Possible causes:**
- Dataset index mismatch
- Parser function error
- Data format incorrect

**Solutions:**

1. **Check dataset indices:**
   - Dataset index 1 = array position 0
   - Dataset index 2 = array position 1, etc.
   - Make sure indices match your parser's return array

2. **Check console for JavaScript errors:**
   - Look for red error messages
   - Fix any syntax errors in parser function

3. **Add debugging to parser:**
   ```javascript
   function parse(frame) {
       console.log("Input frame:", frame);
       let result = frame.split(',');
       console.log("Parsed result:", result);
       return result;
   }
   ```

4. **Verify frame format:**
   - Use console output to see raw frames
   - Ensure format matches expectations

---

### Widgets not displaying or showing errors

**Possible causes:**
- Wrong widget type for data
- Missing required datasets
- OpenGL issues (3D widgets)
- Project file corruption

**Solutions:**

1. **Verify widget requirements:**
   - Accelerometer widget needs exactly 3 datasets (X, Y, Z)
   - GPS widget needs latitude and longitude datasets
   - See [Widget Reference](Widget-Reference.md) for requirements

2. **Check widget type matches data:**
   - Don't use gauge for text data
   - Don't use bar for unbounded values

3. **For 3D widgets (Accelerometer, Gyroscope, 3D Plot):**
   - Ensure OpenGL is supported by your graphics card
   - Update graphics drivers
   - Some virtual machines don't support OpenGL

4. **Try recreating widget:**
   - Delete widget from project
   - Add it again with fresh configuration

---

### Dashboard layout broken after update

**Possible causes:**
- Project file format changes between versions
- Widget API changes

**Solutions:**

1. **Check release notes** for breaking changes

2. **Recreate project file:**
   - Export CSV of working session
   - Create new project from scratch
   - Test with CSV Player

3. **Report issue** if project file should be compatible

---

## Frame Parsing Issues

### Frames not being detected

**Problem:** Console shows no frames being received, or frames not split correctly.

**Debugging steps:**

1. **Verify delimiters match exactly:**
   - Check delimiters are case-sensitive
   - No extra spaces
   - Use hex view in console to see hidden characters

2. **Check raw console output:**
   - Look at actual bytes being received
   - Verify delimiters are present in stream

3. **Try "No Delimiters" mode:**
   - Use custom parser to extract frames
   - Useful for binary protocols

4. **Check for line ending issues:**
   - Different systems use different line endings:
     - `\n` (LF) - Linux/macOS
     - `\r\n` (CRLF) - Windows
     - `\r` (CR) - Old Mac
   - Try different delimiter combinations

---

### Parser function not working

**Problem:** JavaScript parser returns errors or wrong data.

**Debugging steps:**

1. **Check console for errors:**
   - Look for "SyntaxError", "TypeError", etc.
   - Fix JavaScript syntax errors

2. **Add logging:**
   ```javascript
   function parse(frame) {
       console.log("=== PARSER START ===");
       console.log("Type:", typeof frame);
       console.log("Content:", frame);
       console.log("Length:", frame.length);

       // Your parsing logic
       let result = frame.split(',');

       console.log("Result:", result);
       console.log("=== PARSER END ===");
       return result;
   }
   ```

3. **Verify return value is array:**
   ```javascript
   // CORRECT
   return [1, 2, 3];

   // WRONG
   return "1,2,3";  // Must be array!
   ```

4. **Test with simpler parser first:**
   ```javascript
   function parse(frame) {
       return frame.split(',');
   }
   ```

5. **Check decoder method:**
   - Plain Text: `frame` is string
   - Hexadecimal: `frame` is hex string
   - Base64: `frame` is base64 string
   - Binary (Direct) (Pro): `frame` is byte array

---

### Dataset transform not working

**Problem:** You applied a `transform(value)` function but the dashboard still shows raw values.

**Debugging steps:**

1. **Verify the function is named `transform`** — it must be exactly `function transform(value)` (Lua) or `function transform(value) {` (JavaScript). The function name is case-sensitive.

2. **Click Apply in the transform editor** — changes are only saved when you click Apply. Closing the dialog without Apply discards changes.

3. **Test with the built-in Test area** — enter a raw value in the Input field and click Test. If you see an error, fix the function before applying.

4. **Check for non-numeric datasets** — transforms only run on numeric values. If the dataset value is a string (e.g., "N/A"), the transform is skipped.

5. **Check the console for errors** — compilation errors are logged at connect time. Look for `[FrameBuilder] Transform compile error` messages.

6. **Save and reload the project** — if the transform was applied during a live session, saving ensures it persists. After reload, transforms are recompiled automatically.

See [Dataset Value Transforms](Dataset-Transforms.md) for the complete reference.

---

### Parsing slow/laggy

**Possible causes:**
- Complex parser function
- Very high data rate
- Large frames

**Solutions:**

1. **Optimize parser:**
   - Avoid unnecessary operations
   - Use string.split() instead of regex where possible
   - Don't create objects, return simple arrays

2. **Reduce data rate:**
   - Slow down device transmission
   - Increase frame interval

3. **Profile parser performance:**
   ```javascript
   function parse(frame) {
       let start = Date.now();

       // Your parsing logic
       let result = frame.split(',');

       let elapsed = Date.now() - start;
       if (elapsed > 1) {
           console.log("Parse took", elapsed, "ms");
       }
       return result;
   }
   ```

---

## CSV Player Issues

### Cannot open CSV file

**Possible causes:**
- File format incorrect
- File corrupted
- Wrong encoding

**Solutions:**

1. **Verify CSV format:**
   - First row should be headers
   - Comma-separated values
   - Timestamps in first column

2. **Check file encoding:**
   - Should be UTF-8
   - Try opening in text editor to verify

3. **Try export from Serial Studio:**
   - Record a short session
   - Export to CSV
   - Compare format with your file

---

### Playback is choppy

**Possible causes:**
- Very large file
- High data rate
- System performance

**Solutions:**

1. **Reduce playback speed:**
   - Use 0.5x or 0.25x speed

2. **Close other applications:**
   - Free up system resources

3. **For very large files:**
   - Consider splitting into smaller segments
   - Export only needed columns

---

## Pro Feature Issues

### MQTT connection fails

**See:** [MQTT Integration - Troubleshooting](MQTT-Integration.md)

Common issues:
- Broker address/port incorrect
- Authentication credentials wrong
- Firewall blocking connection
- TLS/SSL certificate issues

---

### Modbus connection fails

**See:** [Protocol-Specific Setup Guides - Modbus](Protocol-Setup-Guides.md)

Common issues:
- Wrong slave ID
- Incorrect function code
- Baud rate mismatch (RTU)
- IP/port wrong (TCP)

---

### CAN Bus issues

**See:** [Protocol-Specific Setup Guides - CAN Bus](Protocol-Setup-Guides.md)

Common issues:
- Bitrate mismatch
- Missing termination resistors
- Adapter not recognized
- DBC file format errors

---

### Pro features show as locked

**Problem:** Pro features are grayed out or show "Requires Pro License".

**Possible causes:**
- Trial expired
- License key not activated
- Using GPL build instead of official binary

**Solutions:**

1. **Check license status:**
   - Help → About Serial Studio
   - Check if "Pro License" or "Trial" is shown

2. **Activate license key:**
   - Help → Activate License
   - Enter purchase email and license key
   - Internet connection required for activation

3. **Download official binary:**
   - If you compiled from source with GPL-only modules, download the official binary from [serial-studio.com](https://serial-studio.com)
   - The official binary includes Pro features (locked behind license)

4. **Check trial status:**
   - Trial is 14 days, one-time only
   - Based on hardware ID (can't reset by reinstalling)

---

## Performance Issues

### High CPU usage

**Possible causes:**
- High data rate
- Complex parser function
- Too many widgets
- Large plot history

**Solutions:**

1. **Reduce data rate:**
   - Slow down device transmission
   - Increase frame interval on device

2. **Optimize parser:**
   - Avoid complex calculations
   - Use efficient string operations

3. **Reduce widgets:**
   - Use fewer plots
   - Reduce plot history length

4. **Close unused applications**

---

### Memory usage growing over time

**Possible causes:**
- Memory leak in parser (global variables)
- Very long session with plot history
- Large CSV logging

**Solutions:**

1. **Check global variables in parser:**
   ```javascript
   // CAREFUL: This array grows forever
   let history = [];
   function parse(frame) {
       history.push(frame);  // Memory leak!
       return frame.split(',');
   }

   // BETTER: Limit size
   let history = [];
   function parse(frame) {
       history.push(frame);
       if (history.length > 1000) {
           history.shift();  // Remove old entries
       }
       return frame.split(',');
   }
   ```

2. **Restart Serial Studio periodically** for very long sessions

3. **Reduce plot history:**
   - Shorter time windows
   - Lower sample rate

---

### Slow frame rate / laggy dashboard

**Possible causes:**
- System performance
- Too many widgets
- High data rate
- 3D widgets on weak GPU

**Solutions:**

1. **Reduce widget count:**
   - Serial Studio aims for 60 FPS
   - More widgets = more rendering

2. **Use fewer 3D widgets:**
   - 3D Accelerometer, Gyroscope, and 3D Plot are GPU-intensive
   - Consider 2D alternatives

3. **Reduce data rate:**
   - Dashboard updates at data rate
   - 60 Hz is plenty for visualization

4. **Update graphics drivers**

5. **Close background applications**

---

## Getting More Help

If you can't find a solution here, try these resources:

### 1. DeepWiki
AI-powered Q&A for Serial Studio:
- [DeepWiki for Serial Studio](https://deepwiki.com/Serial-Studio/Serial-Studio)
- Ask natural language questions
- Get instant AI-generated answers based on documentation

### 2. Search GitHub Issues
Someone may have already solved your problem:
- [Serial Studio Issues](https://github.com/Serial-Studio/Serial-Studio/issues)
- Use search bar to find similar issues
- Check both open and closed issues

### 3. GitHub Discussions
Ask the community:
- [Serial Studio Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions)
- Post questions, share solutions
- Get help from other users

### 4. Report a Bug
Think you found a bug? Report it:
- [Create new issue](https://github.com/Serial-Studio/Serial-Studio/issues/new)
- See [Reporting Bugs Effectively](#reporting-bugs-effectively) below

### 5. Contact Support
For Pro license holders:
- Email: alex@serial-studio.com
- Include license key and detailed problem description

---

## Reporting Bugs Effectively

When reporting an issue, please include:

**System Information:**
- [ ] Serial Studio version (Help → About)
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

```
## Problem
Dashboard widgets show "NaN" instead of values

## System
- Serial Studio v3.1.0
- Windows 11 Pro
- Official binary installer

## Steps to Reproduce
1. Connect Arduino Uno via USB (COM3)
2. Configure: Baud 115200, delimiter: \n
3. Load attached project file
4. Open dashboard

## Expected
Gauges should show temperature values (20-30°C)

## Actual
All gauges show "NaN"

## Additional Info
- Console shows: "TypeError: Cannot read property 'split' of undefined"
- Attached: project.json, sample_data.csv
- Screenshot: screenshot.png
```

---

## See Also

- [Getting Started](Getting-Started.md) - First-time setup guide
- [Data Sources](Data-Sources.md) - Connection configuration
- [Data Flow](Data-Flow.md) - Understanding how parsing works
- [Frame Parser Scripting](JavaScript-API.md) - Lua and JavaScript parser reference
- [Widget Reference](Widget-Reference.md) - Widget requirements and usage
- [FAQ](FAQ.md) - Frequently asked questions

---

**Still stuck?** Don't hesitate to ask for help on [GitHub Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions). The community is here to help!
