# BLE Battery Level Monitor

This [Serial Studio](https://github.com/Serial-Studio/Serial-Studio) project visualizes the battery level from BLE devices (like iPhones) over [Bluetooth Low Energy](https://en.wikipedia.org/wiki/Bluetooth_Low_Energy).

![Screenshot](doc/screenshot.png)

## What it does

- Connects to BLE devices using Serial Studio's native BLE support.
- Reads the Battery Level characteristic (0x2A19).
- Shows the battery percentage (0 to 100) on a live gauge and a plot.
- Parses raw binary data with no delimiters or framing.

The project includes a control script that selects the service and characteristic for you on connect.

## BLE service

Bluetooth Low Energy exposes battery level through the Battery Level characteristic, `0x2A19` (`uint8`, 0 to 100):

- **Service UUID:** `0x180A`. This project is wired to the Device Information Service, where the test device (an iPhone) advertises its battery characteristic. Devices that follow the standard expose it under the Battery Service, `0x180F` instead; change the service UUID in the control script to match your hardware.
- **Characteristic UUID:** `0x2A19`. Battery Level (`uint8`, 0 to 100).

The bundled control script picks the configured service, selects its first characteristic, subscribes to notifications (falling back to polling), and decodes the raw value with a 1-byte binary read.

## Project configuration

| Setting           | Value           |
|-------------------|-----------------|
| Data Conversion   | Binary (Direct) |
| Frame Detection   | No Delimiters   |
| Checksum          | None            |
| Dataset Index     | 1               |
| Value Range       | 0 to 100        |
| Widget            | Gauge, Plot     |
| Units             | %               |
| Title             | Battery Level   |

**Parser:**

```js
/**
 * @brief Converts a byte-like iterable into an array of numbers.
 *
 * @param frame An iterable containing byte values (0–255).
 * @return Array of numeric byte values.
 */
function parse(frame) {
    return Array.from(frame);
}
```

### Setup

1. Open Serial Studio and load `BLE Battery Level.ssproj`.
2. Select Bluetooth LE as the input source.
3. Pick your BLE device (for example your iPhone) and click Connect.
4. The control script selects the service (`0x180A` as shipped) and its battery characteristic (`0x2A19`) automatically.
5. Watch the gauge update in real time.
