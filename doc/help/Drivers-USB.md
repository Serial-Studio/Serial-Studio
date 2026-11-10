# Raw USB Driver (Pro)

If your board prints with `Serial.println` over a USB cable, you do not need this page. Arduino and ESP32 boards appear as a serial port, so use the [UART driver](Drivers-UART.md). Gamepads, joysticks and HID firmware belong to the [HID driver](Drivers-HID.md).

Raw USB is for the rest: a logic analyzer, an oscilloscope or a data-acquisition board whose datasheet says "uses bulk endpoint 0x81" and that never shows up as a COM port. Serial Studio opens such a device through **libusb** and reads the endpoint you pick.

Raw USB is a Pro feature (see [Pro vs Free](Pro-vs-Free.md)). The first time you use a Pro feature, Serial Studio offers the Pro trial with a Yes/No prompt; registering the trial needs an internet connection.

## Raw USB setup (Pro)

Raw USB needs Pro. On first use Serial Studio offers the Pro trial (Yes/No, needs internet); answering No leaves the feature locked, and after the trial ends it offers activation or purchase instead.

In the **Setup Panel**, select **USB Device** from the Connection dropdown. The default path takes four steps:

1. Pick your device from the **USB Device** dropdown.
2. Leave **Transfer Mode** on **Bulk/Interrupt Stream**.
3. Check the **IN Endpoint**. Serial Studio selects the first matching one, such as `EP 0x81 – Bulk IN (IF0, max 64 B)`, where `IF0` is interface 0. Pick another if the datasheet says so.
4. Click **Connect**.

Raw bytes arrive from the endpoint. A [frame parser](Frame-Parser.md) or Quick Plot turns them into dashboard values, the same as on any other driver.

| Setting | Shown in | Values | Default |
|---------|----------|--------|---------|
| **USB Device** | all modes | `VID:PID – Manufacturer Product`; the list updates on libusb hotplug events, with a 2-second rescan where hotplug is unavailable | **Select Device** |
| **Transfer Mode** | all modes | **Bulk/Interrupt Stream**, **Advanced (Bulk + Control)**, **Isochronous** | **Bulk/Interrupt Stream** |
| **IN Endpoint** | all modes | the device's IN endpoints that fit the Transfer Mode | first match |
| **OUT Endpoint** | all modes | the device's OUT endpoints that fit the Transfer Mode | **None (Read-only)** |
| **Max Packet Size** | Isochronous | 1 to 49152 bytes | 1024, then pre-filled from the endpoint |

The endpoint dropdowns and **Max Packet Size** lock while connected. Endpoint labels use `Bulk`, `Int` or `Iso` for the type, and add `ALT2` (for example `IF1 ALT2`) for a non-zero alternate setting.

The endpoint lists follow the Transfer Mode. **Isochronous** lists only isochronous endpoints; the other two modes list only bulk and interrupt endpoints. Changing the mode clears the selection and picks the first match. A device with no endpoint of the right type shows **No Usable IN Endpoints**, and **Connect** tells you which mode to switch to. Writes through the OUT endpoint time out after 1 second.

### Transfer modes

| Mode | Transfer types | Use it for |
|------|----------------|------------|
| **Bulk/Interrupt Stream** | Bulk and interrupt IN/OUT, picked by the selected endpoint's type | Most devices: streaming data or small periodic reports |
| **Advanced (Bulk + Control)** | Bulk, plus vendor control transfers | Devices that need a vendor command to start streaming |
| **Isochronous** | Asynchronous isochronous IN | Fixed-rate streams where a few dropped packets are acceptable |

In the USB setup pane the mode is labeled **Advanced (Bulk + Control)**; the driver's generic property list labels it **Advanced Control**.

Choosing **Advanced (Bulk + Control)** opens a Yes/No confirmation (default No), because a vendor control write can do anything the firmware allows, including bricking the device. The dialog appears once and Serial Studio remembers a Yes. Until you accept it in the setup pane, a project load or `io.usb.setTransferMode` with mode 1 is refused with an "Advanced USB mode refused" warning. The API call still returns success when refused, so read `transferMode` back.

In Isochronous mode, **Max Packet Size** is pre-filled from the selected endpoint, including the high-bandwidth multiplier (up to 3 x 1024 B on high speed). Change it only when the datasheet says otherwise.

### Control transfers

With **Advanced (Bulk + Control)** selected and the device connected, a **Control Transfer** composer appears in the USB setup pane. A logic analyzer that stays silent until it gets a vendor command is waiting to be asked, and this is where you ask.

| Field | Format |
|-------|--------|
| **Request Type** (`bmRequestType`) | Hex. Bit 7 sets the direction: `0x80` and above is IN, below is OUT |
| **Request** (`bRequest`), **wValue**, **wIndex** | Hex |
| **Data** | OUT: hex bytes, such as `EF BE 00 00` |
| **Read Length** | IN: bytes to read |

The data length is 0 to 4096 bytes, and one control transfer runs at a time. A vendor "start streaming" write could be Request Type `40`, Request `01`, wValue `0001`, wIndex `0000` and no data. Click **Send Control Transfer**; the result appears in the composer.

## Permissions and OS specifics

USB access is permission-controlled differently on each OS.

- **Linux.** By default only `root` can open arbitrary USB devices. Find your device's IDs with `lsusb` (the `ID` field) or in the **USB Device** dropdown. Create `/etc/udev/rules.d/99-myusbdevice.rules` containing the rule below, with your own VID and PID. `MODE="0666"` lets every local user open the device.

  ```
  SUBSYSTEM=="usb", ATTR{idVendor}=="1234", ATTR{idProduct}=="5678", MODE="0666"
  ```

  Then reload the rules with `sudo udevadm control --reload-rules && sudo udevadm trigger`, and unplug and replug the device.

  Kernel drivers: Serial Studio enables libusb's auto-detach on Linux, so a kernel class driver bound to the interface (for example `cdc_acm`) is detached when the interface is claimed and re-attached on release.
- **Windows.** The standard class drivers (HID, CDC and so on) hold the device open and stop libusb from claiming it, so the device needs a WinUSB driver. In [Zadig](https://zadig.akeo.ie), choose the device, select **WinUSB** and click **Replace Driver**. The device then stops appearing as a normal COM port or HID device until you restore the original driver.
- **macOS.** A device that no OS class driver has claimed opens directly. When macOS has bound a kernel driver, opening or claiming the interface fails, and Serial Studio does not detach system drivers.

For step-by-step setup, see the [Protocol Setup Guides, Raw USB section](Protocol-Setup-Guides.md).

## Common pitfalls

- **Device not listed.** If `lsusb` does not show it either, the cable, hub or power is at fault. `lsusb -v -d VID:PID` prints the descriptors.
- **Device is listed but will not open.** Another driver or application has claimed it, or you lack permission. Follow [Permissions and OS specifics](#permissions-and-os-specifics) for your OS; on Linux, `dmesg | grep usb` shows kernel hints.
- **No data on the IN endpoint.** Pick the endpoint number the vendor documentation names. Some devices send nothing until they receive a vendor control write; use the [control-transfer composer](#control-transfers).
- **Bulk reads time out.** A timeout is not an error: the read loop retries every 100 ms. A connection that stays open with nothing arriving means the device is not sending on the selected IN endpoint. Any other read error (unplug, stall, overflow) closes the connection and shows "The USB device was disconnected or encountered a fatal read error."
- **Isochronous mode drops samples.** **Max Packet Size** is pre-filled from the endpoint, so dropped samples with the default usually mean a manual override, or a host that cannot grant the bandwidth because other isochronous devices share the hub.

## Reference

### Threading

The driver runs two dedicated threads:

- **Event thread.** Pumps libusb's event loop, which libusb's async API and hotplug callbacks require.
- **Read thread.** Issues synchronous bulk or interrupt reads (64 KB buffer, 100 ms timeout) in Bulk/Interrupt Stream and Advanced (Bulk + Control) modes. In Isochronous mode, data arrives through a pool of eight async transfers (eight packets each) whose completion callbacks run on the event thread.

Bulk and interrupt blocks are stamped the moment the transfer returns and published from the read thread. Isochronous blocks are stamped in the completion callback and published after a hop to the main thread. Either way the frame pipeline picks them up from there. See [Threading and Timing Guarantees](Threading-and-Timing.md).

### Remote API commands

With the API Server enabled (off by default, port 7777), the in-app AI assistant and external clients configure this driver through the `io.usb.*` scope:

| Command | Parameters | Notes |
|---------|------------|-------|
| `io.usb.listDevices` | none | Returns `devices` array and `selectedIndex` (index 0 is the "Select Device" placeholder) |
| `io.usb.setDeviceIndex` | `deviceIndex` (0 to the device count; 0 = **Select Device**) | Out-of-range values return an `InvalidParam` error |
| `io.usb.setTransferMode` | `mode` (0 = Bulk/Interrupt Stream, 1 = Advanced (Bulk + Control), 2 = Isochronous) | Mode 1 is refused until the warning was accepted once in the setup pane (see [Transfer modes](#transfer-modes)); verify with `io.usb.getConfig` |
| `io.usb.setInEndpointIndex` | `endpointIndex` (0 = **Select IN Endpoint** placeholder, 1.. = endpoints) | Endpoint lists populate once a device is selected; index 0 cannot connect; out-of-range returns `InvalidParam` |
| `io.usb.setOutEndpointIndex` | `endpointIndex` (0 = **None (Read-only)**, 1.. = endpoints) | Out-of-range returns `InvalidParam` |
| `io.usb.setIsoPacketSize` | `size` (1 to 49152 bytes) | |
| `io.usb.getConfig` | none | Returns mode, indices, packet size, and both endpoint lists |

Transport details are in the [API Reference](API-Reference.md); command safety tiers are in [AI Assistant](AI-Assistant.md#the-safety-tiers).

## USB background

The host always asks; a USB device never speaks first. Each device has **endpoints**: unidirectional buffers identified by a number and a direction (IN = device to host, OUT = host to device). Endpoint 0 is the control endpoint, and every device has it. Serial Studio reads the rest from the device's descriptors, so you only pick from the list.

Each endpoint has one of four **transfer types**:

- **Control** carries configuration and vendor commands over endpoint 0.
- **Bulk** carries large data with error checking and guaranteed delivery, but no bandwidth guarantee. Most raw data-acquisition devices use it.
- **Interrupt** carries small periodic data with a guaranteed maximum latency. There are no hardware interrupts; the host polls.
- **Isochronous** carries time-sensitive streams with guaranteed bandwidth and no retransmission. A corrupted packet is lost.

A device is identified by a 16-bit **Vendor ID (VID)** and **Product ID (PID)**, listed together as `VID:PID`, for example `1234:5678 – Acme Logic Analyzer`. The USB-IF assigns VIDs; the vendor chooses PIDs.

Standard-class devices (CDC, HID, mass storage) get a generic OS driver, which is why a USB-CDC microcontroller shows up as a COM port with no install. Vendor-specific devices skip the classes, and libusb lets an application talk to their endpoints directly.

## Further reading

- [Beyond Logic: USB in a NutShell](https://www.beyondlogic.org/usbnutshell/usb1.shtml)
- [USB Endpoint Types (Beyond Logic Chapter 4)](https://www.beyondlogic.org/usbnutshell/usb4.shtml)
- [libusb-1.0 API Reference](https://libusb.sourceforge.io/api-1.0/)

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): step-by-step Raw USB setup.
- [Troubleshooting](Troubleshooting.md): WinUSB, udev, and permission diagnostics.
- [Drivers: UART](Drivers-UART.md): for USB-CDC virtual serial ports.
- [Drivers: HID](Drivers-HID.md): for HID-class devices.
- [Devices](Devices.md): driver capability summary across all transports.
- [Use Cases](Use-Cases.md): logic analyzers, scientific instruments, and bulk-endpoint devices.
