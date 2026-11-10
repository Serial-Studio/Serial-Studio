# HID Driver (Pro)

Your board shows up as a gamepad, a custom HID device or a USB sensor, not as a COM port? Use this driver. A board that prints with `Serial.println` shows up as a COM port (or `/dev/ttyACM0`): use the [UART driver](Drivers-UART.md) instead.

HID is a Pro feature. The first time you use a Pro feature, Serial Studio offers the 14-day trial in a Yes/No prompt (it needs an internet connection). After the trial ends, the prompt offers activation or purchase instead. See [Pro vs Free](Pro-vs-Free.md).

## Connect a HID device

1. In the **Setup Panel**, choose **HID Device** from the Connection dropdown.
2. Pick your device in the **HID Device** dropdown. Entry 0, **Select Device**, is a placeholder and cannot be opened. While disconnected, Serial Studio rescans every 2 seconds, so a device you plug in appears within a moment. The selection is remembered; if the selected device disappears, it resets to **Select Device**.
3. Click **Connect**.
4. Set the project's frame detection to **Binary (Direct)**, which treats each input report as one frame, and write a frame parser for the report bytes in the Project Editor (see [Frame parsing](#frame-parsing)).

The read-only **Usage Page** and **Usage** fields show the selected device's usage as hex values as soon as you pick it (a joystick shows `0x0001` and `0x0004`). They go empty after Disconnect, and stay empty when the device reports no usage page.

Each entry is listed as `Product Name (VID:PID)` with VID and PID in uppercase hex. If the device reports no product string, the entry shows the manufacturer string, or just `VID:PID`. Interfaces of a composite device that share VID, PID and serial number are merged into one entry.

Input reports: each report arrives as one chunk of raw bytes, so set frame detection to **Binary (Direct)**.

Writing: text sent from the Console goes out through `hid_write` unchanged, so start it with the report ID byte, `0x00` for devices without numbered reports. Serial Studio does not send feature reports.

### Frame parsing

Serial Studio does not read the report descriptor; you decode the bytes yourself. To see the layout, watch the Console in hex view while you move one control and note which byte changes, or read the descriptor in the firmware. Assume a gamepad whose report is 3 bytes: X (signed 8-bit), Y (signed 8-bit), then one byte of button bits. The parser is short:

```javascript
function parse(frame) {
    if (frame.length < 3) return [];
    var x = frame[0] > 127 ? frame[0] - 256 : frame[0];
    var y = frame[1] > 127 ? frame[1] - 256 : frame[1];
    return [x, y, frame[2]];
}
```

Move the stick and the X and Y datasets change. Leave it centred and the console stays quiet, because the device sends nothing.

Devices with numbered reports prepend the report ID byte to every read; the example assumes unnumbered reports. Skip that byte, or branch on it when the device has several report formats. Byte order and field sizes are whatever the firmware sends. See [Frame Parser Scripting](Frame-Parser.md).

### Threading

The HID driver runs a dedicated read thread that calls `hid_read_timeout` (100 ms timeout, 65-byte buffer: one report ID byte plus 64 data bytes), so each read returns at most one input report and anything longer is truncated. The thread stamps each report with `SteadyClock::now()` as it arrives. See [Threading and Timing Guarantees](Threading-and-Timing.md).

## Common pitfalls

- **Device not listed, or listed but will not open.** On Linux the user needs `hidraw` access. Create `/etc/udev/rules.d/99-hidraw.rules` with a rule scoped to your device, putting your own VID and PID in place of the placeholders:

  ```text
  SUBSYSTEM=="hidraw", ATTRS{idVendor}=="1234", ATTRS{idProduct}=="5678", MODE="0666"
  ```

  Then run `sudo udevadm control --reload-rules` and replug the device. The open-failure dialog on Linux also suggests checking that your user is in the `plugdev` group. Windows and macOS expose HID without extra setup.
- **Composite device opens the wrong interface.** Serial Studio lists one entry per VID, PID and serial number and opens the first interface hidapi reports for it. The Usage Page and Usage fields come from the first interface with a nonzero usage page, so they can differ from the interface that opened. If the wrong one opened, expose the data interface with a distinct serial number or its own VID:PID in firmware.
- Connected but no data arrives? Some HID devices only send reports on state change. An untouched gamepad is a very quiet device; move it to confirm.
- **Writes fail on macOS.** Serial Studio sends output only through `hid_write`. A device that accepts output only as feature reports cannot be written to.
- Device unplugged while connected: Serial Studio disconnects and shows a **HID Device Error** dialog. The device list does not rescan while a session is open; it refreshes once on disconnect.

## Remote API commands

The TCP API and the in-app AI assistant configure this driver through the `io.hid.*` scope. They need the API Server, which is off by default (port 7777).

| Command | Parameters | Returns |
|---------|------------|---------|
| `io.hid.listDevices` | none | `devices` array and `selectedIndex` (index 0 is the "Select Device" placeholder) |
| `io.hid.setDeviceIndex` | `deviceIndex` (integer) | `deviceIndex` and `deviceName`; an out-of-range index returns `Invalid deviceIndex: N. Valid range: 0-M` |
| `io.hid.getConfig` | none | `deviceIndex`, `usagePage`, `usage` (`0xNNNN` strings, empty when the device reports none) |

Transport details are in the [API Reference](API-Reference.md); command safety tiers are in [AI Assistant](AI-Assistant.md#the-safety-tiers).

## USB HID basics

A HID device is a USB device that declares itself as HID class, describes its data in a **report descriptor**, and exchanges **reports** over interrupt endpoints (see the [USB driver page](Drivers-USB.md)). The OS's generic HID driver reads the descriptor, so the device works without a per-vendor driver.

A **report** is a unit of data exchanged between host and device. There are three types:

- **Input report**: device to host. Carries button state, axis position or sensor readings.
- **Output report**: host to device, such as an LED on/off or a vibration motor strength.
- **Feature report**: bidirectional configuration and calibration values.

Reports are *numbered* (each starts with a 1-byte report ID, used when the device has several formats) or *unnumbered* (one format, no ID byte).

The descriptor declares each field's **usage page** and **usage** (its meaning), size in bits, count and value range. A trivial gamepad might declare two signed 8-bit axes (X and Y, -127 to +127) and 8 single-bit buttons:

```
Usage Page (Generic Desktop)
Usage (Joystick)
Collection (Application)
  Usage (X)
  Usage (Y)
  Logical Minimum (-127)
  Logical Maximum (127)
  Report Size (8)
  Report Count (2)
  Input (Data, Var, Abs)

  Usage Page (Button)
  Usage Minimum (Button 1)
  Usage Maximum (Button 8)
  Logical Minimum (0)
  Logical Maximum (1)
  Report Size (1)
  Report Count (8)
  Input (Data, Var, Abs)
End Collection
```

A report from that device is 3 bytes: X, Y and a packed byte of button states.

The driver is built on [hidapi](https://github.com/libusb/hidapi), a small cross-platform C library over the OS HID stacks (HIDClass on Windows, IOHIDManager on macOS). The OS keeps its own HID driver bound to the device, so no driver unbinding is needed.

## Further reading

- [Human Interface Devices (HID) Specifications and Tools (USB-IF)](https://www.usb.org/hid)
- [Introduction to HID report descriptors (Linux Kernel Documentation)](https://docs.kernel.org/hid/hidintro.html)
- [HID Report Descriptors (Adafruit Learning System)](https://learn.adafruit.com/custom-hid-devices-in-circuitpython/report-descriptors)
- [hidapi on GitHub](https://github.com/libusb/hidapi)

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step HID setup.
- [Devices](Devices.md): driver capability summary across all transports.
- [Use Cases](Use-Cases.md): gamepads, custom HID firmware, and HID-class sensors.
- [Drivers: USB](Drivers-USB.md): for vendor-specific (non-HID) USB devices.
- [Drivers: UART](Drivers-UART.md): for USB-CDC virtual serial ports.
