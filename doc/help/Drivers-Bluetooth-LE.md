# Bluetooth Low Energy Driver

Your ESP32 or nRF52 board sends sensor values over Bluetooth Low Energy instead of USB. Serial Studio connects to it as the BLE central, subscribes to one characteristic, and treats every notification as incoming bytes, exactly as if they had arrived over UART. Bluetooth LE is available in both the free and Pro editions. The terms central, service, characteristic and notification are explained in [BLE terms you need](#ble-terms-you-need).

"Classic" Bluetooth devices (the older Serial Port Profile) show up as a virtual COM port; use the [UART driver](Drivers-UART.md) for those.

## Quick start

The firmware needs to advertise a device name and notify one characteristic with lines such as `23.4,51.0`. This ESP32 Arduino sketch does that once per second:

```cpp
#include <BLEDevice.h>
#include <BLE2902.h>

BLECharacteristic* tx;

void setup() {
  BLEDevice::init("SensorNode");
  BLEServer* server = BLEDevice::createServer();
  BLEService* svc = server->createService("FFE0");
  tx = svc->createCharacteristic("FFE1", BLECharacteristic::PROPERTY_NOTIFY);
  tx->addDescriptor(new BLE2902());
  svc->start();
  BLEDevice::getAdvertising()->addServiceUUID("FFE0");
  BLEDevice::startAdvertising();
}

void loop() {
  tx->setValue("23.4,51.0\n");
  tx->notify();
  delay(1000);
}
```

`FFE0` is the service and `FFE1` the characteristic. They are arbitrary UUIDs the sketch chooses, and Serial Studio shows them in its dropdowns. The sketch uses the BLE library bundled with the ESP32 Arduino core. It does not restart advertising after a disconnect, so reset the board between sessions. Keep each notification at 20 bytes or fewer, the payload of the default MTU (see [MTU and throughput](#mtu-and-throughput)).

Then, in Serial Studio:

1. In the **Device Setup** panel, set **Operation Mode** to "Quick Plot (Auto-Detected Format)" (see [Operation Modes](Operation-Modes.md)) and the interface to **Bluetooth LE**. The **Device** dropdown appears once a scan finds a named peripheral; the circular-arrow icon beside it rescans.
2. Pick `SensorNode` in **Device**, then click **Connect** in the main toolbar. Connect opens the link and starts service discovery, so the **Service** dropdown stays hidden until discovery finishes.
3. Pick `FFE0` in **Service**. The **Characteristic** dropdown appears once the service lists characteristics.
4. Pick `FFE1` in **Characteristic**, the one with the Notify property. No data arrives before this step, because Serial Studio enables notifications when you select the characteristic.

Two plots (23.4 and 51.0) start drawing, one point per second. Quick Plot splits the stream on line endings, so each `\n` ends one frame.

## What Serial Studio does

On selecting a characteristic, Serial Studio turns notifications on for it by writing the CCCD (the descriptor that switches notifications on) with the Notify value `0x0001`. You do not enable anything by hand. Indicate-only characteristics are not supported, because the driver never requests indications, and a characteristic without a CCCD delivers nothing. If the characteristic already holds a value when you select it, that value arrives as the first chunk.

Every notification's payload enters the incoming stream unchanged, and the configured frame detection splits it into frames. In a project, set the source's frame detection to **No Delimiters** to map one notification to one frame, and decode packed binary payloads in a [frame parser](Frame-Parser.md).

Each dropdown starts on its own placeholder, **Select Device**, **Select Service** or **Select Characteristic**; choosing it clears the selection. The Service and Characteristic dropdowns appear only when they list at least one entry. Until the first named peripheral is found, the pane shows **Scanning…**; once one is listed, the **Device** dropdown replaces it. With no radio present it shows **No Bluetooth Adapter Detected**, and on operating systems without Qt Bluetooth support it shows **This OS is not Supported Yet.**

### BLE terms you need

- A **peripheral** advertises its name and exposes data. A **central** scans and connects. Serial Studio is the central.
- A **service** groups related data, and a **characteristic** is one value inside it. Standard ones use 16-bit UUIDs (`0x180F` is Battery Service); custom ones use 128-bit UUIDs.
- **Notify** means the peripheral pushes each new value without being asked. This is the telemetry pattern Serial Studio uses.

### Writing to the device

Data sent through the console or an [action](Actions.md) is written to the selected characteristic. Serial Studio uses Write Without Response when the characteristic supports it and Write With Response otherwise. Devices that split RX and TX across two characteristics (the Nordic UART Service pattern) take writes through `io.ble.writeCharacteristic`, which resolves any characteristic in the selected service by UUID.

### Reconnecting and saved settings

A project saves the device (by name or address) and the service and characteristic (by UUID, so a firmware update that reorders the GATT table does not break it). On the next connection Serial Studio reselects them after discovery and reports the source as connected only afterwards. Anything that runs on connect, such as a [Control Loop](Control-Loop.md) or an auto-execute action runs only after the characteristic is selected, so it can write to the characteristic as soon as the source reports connected.

Projects saved by older versions may hold a notify characteristic but no service UUID. Serial Studio then probes the discovered services one by one to find it.

### MTU and throughput

The default MTU (maximum packet size) is 23 bytes, of which 20 are payload. BLE 4.2+ devices can negotiate up to 247 bytes of payload. Serial Studio does not set the MTU or the connection interval; the operating system and the peripheral negotiate both. On paper the ceiling is roughly 260 kbps (one 247-byte packet per 7.5 ms interval); real links are lower.

A 20-byte frame at 100 Hz is 2 kB/s, comfortable for BLE. Several channels at 1 kHz are not; use [TCP or UDP](Drivers-Network.md) for those.

## Scripting and API control

The same flow is scriptable through the `io.ble.*` commands of the [JSON-RPC API](API-Reference.md). The API Server is off by default; when enabled it listens on port 7777.

| Command | Parameter | Selects by |
|---------|-----------|------------|
| `io.ble.selectDevice` | `deviceIndex` | index into `io.ble.listDevices` |
| `io.ble.selectService` | `serviceIndex` | index into `io.ble.listServices` |
| `io.ble.selectServiceByUuid` | `serviceUuid` | UUID |
| `io.ble.setCharacteristicIndex` | `characteristicIndex` | index into `io.ble.listCharacteristics` |
| `io.ble.setNotifyCharacteristic` | `characteristicUuid` | UUID |
| `io.ble.writeCharacteristic` | `characteristicUuid`, base64 `data` | UUID, any characteristic in the selected service |
| `io.ble.startDiscovery` | none | n/a |
| `io.ble.listDevices`, `io.ble.listServices`, `io.ble.listCharacteristics`, `io.ble.getConfig`, `io.ble.getStatus` | none | n/a |

UUID parameters accept the 16-bit short form (`fff1`, `0xFFF1`) or the full 128-bit form, with or without braces. Index selection depends on list order; UUID selection survives GATT-table reordering.

When the in-app AI assistant sends these commands, every one except the list and get commands needs **Allow device control** switched on, and each call still asks for your confirmation. The toggle is in the AI Assistant panel, under the wrench (**Settings**) button in its top bar.

## Common pitfalls

| Symptom | Cause and fix |
|---------|---------------|
| Device does not appear in the scan | Serial Studio lists only peripherals that advertise a name; set one in the firmware. Otherwise power-cycle the peripheral and click the rescan icon. On macOS, grant Serial Studio Bluetooth permission in **System Settings > Privacy & Security > Bluetooth**. |
| Connected, but no characteristic appears | The **Characteristic** dropdown stays hidden until you pick a service and its characteristics are discovered; pick the service first. Some peripherals expose their services only after pairing in the OS Bluetooth settings (Windows Bluetooth settings, macOS System Settings, or `bluetoothctl` on Linux). |
| Connected, but no data arrives | The characteristic must have the Notify property (`PROPERTY_NOTIFY` in the sketch) and a CCCD (`BLE2902` descriptor); Notify-less and Indicate-only characteristics deliver nothing. Serial Studio does not show characteristic properties, so check them in the firmware. |
| Throughput is lower than expected | The connection interval, set by the peripheral and the OS, caps the packet rate. See [MTU and throughput](#mtu-and-throughput); ask the firmware for a shorter interval. |
| Connection drops after a few seconds | Some peripherals that require encryption drop the link until you pair them in the OS Bluetooth settings. Pair first, then reconnect. |
| A second window cannot connect | Most peripherals accept one central at a time. Your phone, still paired to it from yesterday, counts. |
| Linux shows no adapter | Run `bluetoothctl list`; if it prints no controller, BlueZ does not see the adapter. |

## Further reading

- [Bluetooth Low Energy Fundamentals (Nordic Developer Academy)](https://academy.nordicsemi.com/courses/bluetooth-low-energy-fundamentals/)

## See also

- [Protocol Setup Guides](Protocol-Setup-Guides.md): the Device Setup panel options for each protocol.
- [Troubleshooting](Troubleshooting.md): pairing, permission, and adapter-detection diagnostics.
