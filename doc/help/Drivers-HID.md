# HID Driver (Pro)

The HID driver lets Serial Studio talk to USB Human Interface Device (HID) class peripherals: keyboards, mice, gamepads, joysticks, drawing tablets, custom HID firmware, and any vendor-specific device that registers itself as a HID class member.

HID is the simplest path to a device that works on every consumer OS without per-vendor driver installs. Every modern OS includes a generic HID driver, so a HID-class device shows up immediately. This makes HID a good choice for custom controllers and low-cost data-acquisition devices.

## What is USB HID?

The HID class was defined by the USB-IF in 1996 to handle "input devices for humans" generically. The original goal was to standardise keyboards, mice, joysticks, and game controllers so the OS would not need a per-vendor driver for each one. Over time, "human interface" stretched to include anything with a small periodic data stream: barcode scanners, RFID readers, fingerprint sensors, and simple instruments.

A HID device is a USB device that:

1. Declares itself as a HID class device in its **Interface Descriptor**.
2. Provides a **HID Report Descriptor** that describes the structure of the data it will send and receive.
3. Communicates through **HID reports** sent over **interrupt endpoints**.

The USB packet structure is the standard interrupt-transfer mechanism (see the [USB driver page](Drivers-USB.md)); the HID class layer adds the report descriptor and a small control-transfer protocol.

### Reports

A **report** is a unit of data exchanged between host and device. There are three types:

- **Input report**: device to host. Periodic. Carries button state, axis position, or sensor readings; the bread and butter of HID.
- **Output report**: host to device. State the host wants the device to act on, such as an LED on/off or a vibration motor strength.
- **Feature report**: bidirectional. Configuration values, calibration, anything outside the streaming exchange.

Reports may be *numbered* (each begins with a 1-byte report ID, used when the device has multiple report formats) or *unnumbered* (a single format, no ID byte).

### Report descriptors

Rather than each HID device family inventing its own protocol, the device describes its data structure to the host through a **report descriptor**. A report descriptor is a sequence of small items declaring:

- The **usage page** and **usage** of each field, defining its semantic meaning.
- The **size** of each field, in bits.
- The **count** of fields of that size.
- The **logical and physical ranges** of values.

A trivial gamepad might declare:

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

That declares a joystick with two 8-bit signed axes (X and Y, range -127 to +127) and 8 single-bit buttons. A report is 3 bytes: X, Y, and a packed byte of button states.

The OS's generic HID driver reads this descriptor, identifies the device, and presents standardised events to user-space.

### Usage pages

A **usage page** is a namespace defining the meaning of fields. Standard usage pages include:

- **Generic Desktop** (page `0x01`): X/Y/Z axes, joysticks, mice, keyboards.
- **Keyboard / Keypad** (page `0x07`): keyboard scan codes.
- **Button** (page `0x09`): generic numbered buttons.
- **LED** (page `0x08`): caps lock, num lock, scroll lock indicators.
- **Sensor** (page `0x20`): accelerometers, gyroscopes, environmental sensors.

Vendors can also define **custom usage pages** in the top half of the 32-bit page space. A custom page lets the device declare arbitrary fields without colliding with anything in the standard space.

### hidapi

[**hidapi**](https://github.com/libusb/hidapi) is a small cross-platform C library that provides a uniform API for talking to HID devices on Windows, macOS, and Linux. It abstracts over the OS-specific HID stacks (HIDClass on Windows, IOHIDManager on macOS, hidraw or libusb on Linux). Serial Studio's HID driver is built on hidapi.

The advantage over raw libusb is that HID devices are typically already claimed by an OS driver, and libusb cannot open them without first unbinding that driver. hidapi cooperates with the OS HID stack, so reading a HID device alongside the OS driver works without extra setup.

## How Serial Studio uses it

The HID driver wraps hidapi. The setup flow is:

1. Wait for **device enumeration**. Serial Studio re-enumerates HID devices every 2 seconds for hotplug detection.
2. Pick a device from the **HID Device** dropdown. Entries are listed as `Product Name (VID:PID)` with VID and PID in uppercase hex. Interfaces of a composite device that share VID, PID, and serial number are merged into a single entry.
3. Connect. The **Usage Page** and **Usage** fields appear below the device selector once connected; they are read-only hex values (for example `0x0001` and `0x0004`) reporting the opened device's usage.

From that point, every input report arrives as a chunk of raw bytes in the standard FrameReader pipeline; frame detection and parsing work the same as for any other driver. Writes from the console go out through `hid_write`, where the first payload byte is the report ID (`0x00` for devices without numbered reports).

### Threading

The HID driver runs a dedicated read thread that issues blocking `hid_read_timeout` calls (100 ms timeout, 65-byte buffer: one report ID byte plus a 64-byte report). Each call returns at most one input report. When data arrives, the thread captures a `SteadyClock::now()` timestamp and posts the data to the main thread via `Qt::AutoConnection`, which resolves to `Qt::QueuedConnection` for the cross-thread hop. See [Threading and Timing Guarantees](Threading-and-Timing.md).

### Frame parsing

HID data is just bytes. If the device declares a custom report format (for example 8-byte reports with two `int16` axes and a `uint8` packed button byte), write a frame parser to decode it as you would any other binary protocol. The HID report descriptor itself is advisory at the protocol level; the host does not require you to honour it.

For step-by-step setup, see the [Protocol Setup Guides, HID section](Protocol-Setup-Guides.md).

### Remote API commands

The TCP API and the in-app AI assistant configure this driver through the `io.hid.*` scope:

| Command | Parameters | Returns |
|---------|------------|---------|
| `io.hid.listDevices` | none | `devices` array and `selectedIndex` (index 0 is the "Select Device" placeholder) |
| `io.hid.setDeviceIndex` | `deviceIndex` (integer) | Selected index and device name |
| `io.hid.getConfig` | none | `deviceIndex`, `usagePage`, `usage` |

Transport details and command safety tiers are in the [API Reference](API-Reference.md).

## Common pitfalls

- **Device not listed.** On Linux, the user needs `hidraw` access. Add a udev rule:
  ```
  /etc/udev/rules.d/99-hidraw.rules:
  KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666"
  ```
  Prefer scoping the rule to a specific VID/PID rather than opening every hidraw node. The same rule fixes devices that appear in the list but fail to open. Windows and macOS expose HID without extra setup.
- **Composite device opens the wrong interface.** A composite device can register several HID interfaces (for example one for the keyboard interface and one for vendor data). Serial Studio merges interfaces that share VID, PID, and serial number into one list entry and opens the first interface hidapi reports. Check the read-only Usage Page / Usage fields after connecting; if the wrong interface opened, the firmware-side fix is to expose the data interface with a distinct serial number or its own VID:PID.
- **Connected but no data arrives.** Some HID devices only send reports on state change. A gamepad reports when a button is pressed or an axis moves; if it is idle, nothing arrives. Move the device to confirm.
- **Reports look corrupted.** Check whether the device uses *numbered* reports. If so, the first byte of each read is the report ID, not data. The frame parser must skip it, or split by ID when multiple report formats exist.
- **macOS reads work but writes do not.** On macOS, output reports sometimes require `hid_send_feature_report` instead of `hid_write`, depending on how the device declares its output endpoints. This is a hidapi-level quirk; the workaround usually has to be made on the firmware side.
- **Re-enumeration interval feels slow.** The default is 2 seconds. To speed up hotplug detection, edit `kEnumIntervalMs` in `app/src/IO/Drivers/HID.cpp` and rebuild.

## Further reading

- [Human Interface Devices (HID) Specifications and Tools (USB-IF)](https://www.usb.org/hid)
- [Introduction to HID report descriptors (Linux Kernel Documentation)](https://docs.kernel.org/hid/hidintro.html)
- [HID Report Descriptors (Adafruit Learning System)](https://learn.adafruit.com/custom-hid-devices-in-circuitpython/report-descriptors)
- [AN249: Human Interface Device Tutorial (Silicon Labs, PDF)](https://www.silabs.com/documents/public/application-notes/AN249.pdf)
- [hidapi on GitHub](https://github.com/libusb/hidapi)
- [USB Component: HID (Keil/MDK)](https://www.keil.com/pack/doc/mw/USB/html/group__usbd__hid_functions.html)

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step HID setup.
- [Data Sources](Data-Sources.md): driver capability summary across all transports.
- [Communication Protocols](Communication-Protocols.md): overview of all supported transports.
- [Use Cases](Use-Cases.md): gamepads, custom HID firmware, and HID-class sensors.
- [Frame Parser Scripting](JavaScript-API.md): decoding custom HID report layouts into datasets.
- [Drivers: USB](Drivers-USB.md): for vendor-specific (non-HID) USB devices.
- [Drivers: UART](Drivers-UART.md): for USB-CDC virtual serial ports.
