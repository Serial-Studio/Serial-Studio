# Output Controls

## Overview

Output controls are interactive dashboard widgets that send data back to a connected device. While standard widgets visualize incoming telemetry, output controls let you transmit commands, setpoints, and parameters — enabling full bidirectional communication from the Serial Studio dashboard.

Each output control uses a user-defined JavaScript `transmit(value)` function that converts widget interactions (button clicks, slider drags, text input) into the exact bytes your device expects. This makes output controls protocol-agnostic: the same slider widget can drive a plain-text serial command, a JSON payload, or a binary packet — just by changing the transmit function.

Output controls require a **Pro license**.

## How Output Controls Work

```mermaid
flowchart LR
    A["User Interaction<br/>(click, drag, type)"] --> B["Widget passes value<br/>to transmit(value)"]
    B --> C["JavaScript returns<br/>formatted command"]
    C --> D["Serial Studio sends<br/>bytes to device"]
```

1. The user interacts with a control on the dashboard (clicks a button, moves a slider, types text, etc.).
2. The widget calls its JavaScript `transmit(value)` function with the interaction value.
3. The function returns a string or byte sequence.
4. Serial Studio transmits the result to the connected device.

Transmission is rate-limited to a minimum of 50 ms between sends, preventing device buffer overflows during continuous interactions like slider drags.

## Output Control Types

### Button

Sends a single command on click.

| Property | Value |
|----------|-------|
| Value passed to `transmit()` | `1` (integer) |
| Interaction | Single click |
| Use cases | Reset, start/stop, trigger measurement |

### Slider

Sends a numeric value from a draggable slider.

| Property | Default | Description |
|----------|---------|-------------|
| Min Value | 0 | Lower bound of the slider range |
| Max Value | 100 | Upper bound of the slider range |
| Step Size | 1 | Increment between discrete positions |
| Initial Value | 0 | Starting position |
| Units | — | Label suffix (e.g., "%", "rpm") |

The value passed to `transmit()` is a number clamped to [Min, Max]. Transmissions occur continuously while dragging, rate-limited to 50 ms intervals.

### Toggle

Binary on/off switch.

| Property | Default | Description |
|----------|---------|-------------|
| ON Label | — | Text shown in the ON state |
| OFF Label | — | Text shown in the OFF state |
| Initial Value | 0 | Starting state (0 = off, 1 = on) |

Passes `1` to `transmit()` when switched on, `0` when switched off.

### Text Field

Accepts arbitrary typed input and sends it as a string.

| Property | Value |
|----------|-------|
| Value passed to `transmit()` | The typed string |
| Interaction | Press Enter or click Send |
| Use cases | AT commands, debug console, custom queries |

### Knob

Rotary dial for continuous setpoint adjustment. Same numeric properties as Slider (Min, Max, Step, Initial Value, Units) but displayed as a circular dial.

### Ramp Generator

Automated value sweep for testing and stress scenarios.

| Property | Default | Description |
|----------|---------|-------------|
| Start Value | 0 | Ramp starting point |
| Target Value | 100 | Ramp ending point |
| Speed | 1 | Units per second |
| Cycles | 0 | Number of forward-backward sweeps (0 = infinite) |

When started, the ramp generator automatically increments the value from start to target at the configured speed, then reverses back. Each step calls `transmit()` with the current value. Use the dashboard controls to start, stop, and configure the ramp.

## Creating Output Controls

1. Open the Project Editor (toolbar wrench icon, or Ctrl+Shift+P / Cmd+Shift+P).
2. Click one of the **Add Output** buttons in the toolbar (Button, Slider, Toggle, Text Field, Knob, or Ramp).
3. An **Output Panel** group is created automatically if one does not exist.
4. Select the new control in the tree view to configure its properties and transmit function.

Output controls live inside **Output Panel** groups. You can also add an Output Panel group first (via the toolbar), then add controls to it. Each Output Panel can hold multiple controls of mixed types, arranged in a configurable column grid.

## The Transmit Function

Every output control has a JavaScript `transmit(value)` function that defines how interactions become device commands. The function is compiled once when the dashboard opens and executed on each interaction.

### Writing a Transmit Function

The function receives a single `value` parameter and must return a string:

```javascript
function transmit(value) {
  // value is:
  //   1           for Button clicks
  //   0 or 1      for Toggle state changes
  //   number       for Slider, Knob, Ramp Generator
  //   "string"    for TextField input

  return "CMD " + value + "\r\n";
}
```

### Built-in Templates

The code editor includes 8 ready-to-use templates. Select one from the template dropdown and customize it for your device.

#### Simple Command

Sends plain text with a line terminator. Adapts to the widget type automatically.

```javascript
function transmit(value) {
  if (typeof value === "string")
    return value + "\r\n";

  if (value === 1)
    return "ON\r\n";

  if (value === 0)
    return "OFF\r\n";

  return "SET " + value + "\r\n";
}
```

#### JSON Command

Sends structured JSON objects. Useful for firmware that parses JSON input.

```javascript
function transmit(value) {
  var obj = {
    cmd: "set",
    value: value
  };
  return JSON.stringify(obj) + "\n";
}
```

#### Binary Packet

Sends framed binary data with STX/ETX delimiters.

```javascript
function transmit(value) {
  var STX = String.fromCharCode(0x02);
  var ETX = String.fromCharCode(0x03);
  var cmd = String.fromCharCode(0x01);
  var val = String.fromCharCode(Math.round(value) & 0xFF);
  return STX + cmd + val + ETX;
}
```

#### PWM Control

Sends a duty cycle value (0-255) for motor speed, LED brightness, or heater control.

```javascript
function transmit(value) {
  var duty = Math.round(Math.max(0, Math.min(255, value)));
  return "PWM " + duty + "\r\n";
}
```

#### PID Setpoint

Sends a floating-point setpoint with 2 decimal places for PID controllers.

```javascript
function transmit(value) {
  return "SP " + Number(value).toFixed(2) + "\r\n";
}
```

#### Relay Toggle

Sends distinct ON/OFF commands for relay or digital output control.

```javascript
function transmit(value) {
  return value ? "RELAY ON\r\n" : "RELAY OFF\r\n";
}
```

#### AT Command

Sends AT-style commands for modems, Bluetooth modules, and WiFi modules.

```javascript
function transmit(value) {
  if (typeof value === "string" && value.length > 0)
    return "AT+" + value + "\r\n";

  return "AT\r\n";
}
```

#### Modbus Register Write

Writes a slider value directly to a Modbus holding register using the built-in helper function.

```javascript
function transmit(value) {
  return modbusWriteRegister(0x0001, value);
}
```

#### CAN Bus Frame

Sends a numeric value as a CAN frame using the built-in helper function.

```javascript
function transmit(value) {
  return canSendValue(0x100, value, 2);
}
```

### Importing from File

Click the import button in the code editor toolbar to load a `.js` file from disk. This is useful for sharing transmit functions across projects or version-controlling them separately.

## Protocol Helper Functions

Every output widget's JavaScript engine includes built-in helper functions for Modbus and CAN Bus protocols. These handle binary byte-packing so you don't have to construct raw bytes manually.

### Modbus Helpers

#### `modbusWriteRegister(address, value)`

Writes a 16-bit integer to a single holding register.

| Parameter | Type | Description |
|-----------|------|-------------|
| `address` | Number | Register address (0x0000–0xFFFF) |
| `value` | Number | Value to write (rounded to integer, 0–65535) |

```javascript
function transmit(value) {
  return modbusWriteRegister(0x0001, value);
}
```

#### `modbusWriteCoil(address, on)`

Writes a coil value (ON = 0xFF00, OFF = 0x0000).

| Parameter | Type | Description |
|-----------|------|-------------|
| `address` | Number | Coil address (0x0000–0xFFFF) |
| `on` | Boolean/Number | Truthy = ON, falsy = OFF |

```javascript
// Toggle widget controlling a relay coil
function transmit(value) {
  return modbusWriteCoil(0x0000, value);
}
```

#### `modbusWriteFloat(address, value)`

Writes an IEEE-754 32-bit float across two consecutive holding registers (big-endian).

| Parameter | Type | Description |
|-----------|------|-------------|
| `address` | Number | Starting register address |
| `value` | Number | Floating-point value |

```javascript
// Slider writing a temperature setpoint as a 32-bit float
function transmit(value) {
  return modbusWriteFloat(0x0010, value);
}
```

### CAN Bus Helpers

#### `canSendFrame(id, payload)`

Sends an arbitrary CAN frame with the given identifier and payload.

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | Number | CAN identifier (11-bit standard: 0x000–0x7FF) |
| `payload` | Array or String | Payload bytes as an array of numbers (0–255), or a raw string |

```javascript
// Button sending a fixed command frame
function transmit(value) {
  return canSendFrame(0x200, [0x01, 0x00, 0xFF]);
}
```

```javascript
// Slider packing its value into a 3-byte payload
function transmit(value) {
  var v = Math.round(value);
  return canSendFrame(0x100, [0x01, (v >> 8) & 0xFF, v & 0xFF]);
}
```

#### `canSendValue(id, value, bytes)`

Sends a numeric value packed big-endian into a CAN frame.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `id` | Number | — | CAN identifier |
| `value` | Number | — | Numeric value (rounded to integer) |
| `bytes` | Number | 2 | Number of payload bytes (1–8) |

```javascript
// Slider sending a 16-bit value on CAN ID 0x100
function transmit(value) {
  return canSendValue(0x100, value, 2);
}
```

```javascript
// Knob sending a 32-bit value on CAN ID 0x300
function transmit(value) {
  return canSendValue(0x300, value, 4);
}
```

### Combining Helpers with Custom Logic

The helpers return strings that can be concatenated or conditionally selected:

```javascript
// Write different registers based on a toggle state
function transmit(value) {
  if (value)
    return modbusWriteRegister(0x0010, 1);  // Enable
  else
    return modbusWriteRegister(0x0010, 0);  // Disable
}
```

```javascript
// Send a CAN frame with a header byte and the widget value
function transmit(value) {
  return canSendFrame(0x150, [0xAA, Math.round(value) & 0xFF]);
}
```

## Output Panel Layout

Output controls are displayed in an Output Panel widget on the dashboard. The panel uses an adaptive layout engine:

- Controls are arranged in a configurable number of columns (set in the Output Panel group properties).
- Small controls (Button, Slider, Toggle, TextField) stack vertically within columns.
- Tall controls (Knob, Ramp Generator) span the full column height.
- If controls overflow the visible area, the panel scrolls vertically.

## Multi-Source Projects

In projects with multiple data sources (devices), each output control has a **Target Device** property. This determines which connected device receives the transmitted data. The tree view shows the target device tag next to each control.

## Output Controls vs. Actions

Both output controls and [Actions](Actions.md) send data to connected devices, but they serve different purposes:

| Feature | Output Controls | Actions |
|---------|----------------|---------|
| Widget types | 6 (button, slider, toggle, text, knob, ramp) | Button only |
| Data formatting | JavaScript `transmit()` function | Fixed TX Data + EOL |
| Continuous values | Yes (slider, knob, ramp) | No |
| Timer/auto-repeat | Ramp generator only | Yes (4 timer modes) |
| Auto-execute on connect | No | Yes |
| License | Pro | Free |

**Use Actions** for simple fire-and-forget commands, periodic polling, and auto-execute-on-connect sequences. **Use Output Controls** when you need interactive controls with continuous values, custom data formatting, or a mix of widget types.

## Examples

### Motor Speed Controller

Control motor speed with a slider and an emergency stop button.

| Control | Type | Properties |
|---------|------|------------|
| Speed | Slider | Min: 0, Max: 100, Units: "%" |
| Emergency Stop | Button | — |

Speed transmit function:
```javascript
function transmit(value) {
  return "SPD " + Math.round(value) + "\r\n";
}
```

Emergency Stop transmit function:
```javascript
function transmit(value) {
  return "ESTOP\r\n";
}
```

### Relay Control Panel

Toggle 3 relays independently.

| Control | Type | Properties |
|---------|------|------------|
| Relay 1 | Toggle | ON: "Closed", OFF: "Open" |
| Relay 2 | Toggle | ON: "Closed", OFF: "Open" |
| Relay 3 | Toggle | ON: "Closed", OFF: "Open" |

Each relay uses a customized transmit function with its relay number:
```javascript
// Relay 1
function transmit(value) {
  return value ? "R1 ON\r\n" : "R1 OFF\r\n";
}
```

### Sensor Calibration Interface

Combine a text field for commands with a knob for fine adjustment.

| Control | Type | Properties |
|---------|------|------------|
| Command | TextField | — |
| Offset | Knob | Min: -10, Max: 10, Step: 0.1, Units: "mV" |

Command transmit function:
```javascript
function transmit(value) {
  return "CAL " + value + "\r\n";
}
```

Offset transmit function:
```javascript
function transmit(value) {
  return "OFFSET " + Number(value).toFixed(1) + "\r\n";
}
```

### Modbus PID Controller

Control a PID loop over Modbus by writing setpoint, Kp, and enable/disable to holding registers.

| Control | Type | Properties |
|---------|------|------------|
| Setpoint | Slider | Min: 0, Max: 500, Step: 0.5, Units: "°C" |
| Kp Gain | Knob | Min: 0, Max: 10, Step: 0.01 |
| Enable | Toggle | ON: "Running", OFF: "Stopped" |

Setpoint transmit function (32-bit float to registers 0x0010–0x0011):
```javascript
function transmit(value) {
  return modbusWriteFloat(0x0010, value);
}
```

Kp Gain transmit function (32-bit float to registers 0x0012–0x0013):
```javascript
function transmit(value) {
  return modbusWriteFloat(0x0012, value);
}
```

Enable transmit function (coil at address 0x0000):
```javascript
function transmit(value) {
  return modbusWriteCoil(0x0000, value);
}
```

### CAN Bus Motor Controller

Control a motor over CAN Bus with speed setpoint and emergency stop.

| Control | Type | Properties |
|---------|------|------------|
| Speed | Slider | Min: 0, Max: 10000, Units: "RPM" |
| Direction | Toggle | ON: "Forward", OFF: "Reverse" |
| E-Stop | Button | — |

Speed transmit function (16-bit value on CAN ID 0x100):
```javascript
function transmit(value) {
  return canSendValue(0x100, value, 2);
}
```

Direction transmit function (single byte on CAN ID 0x101):
```javascript
function transmit(value) {
  return canSendFrame(0x101, [value ? 0x01 : 0x00]);
}
```

E-Stop transmit function (fixed command frame on CAN ID 0x1FF):
```javascript
function transmit(value) {
  return canSendFrame(0x1FF, [0xFF, 0x00]);
}
```

## Common Mistakes

### Controls Do Not Appear on Dashboard

**Symptom:** Output controls are configured in the Project Editor but do not appear on the dashboard.

**Fix:** Ensure the device is connected. Output panels only appear on the dashboard while a connection is active. Also verify that the controls are inside an Output Panel group (group type must be "Output").

### Commands Not Received by Device

**Symptom:** The control is visible and interactive, but the device does not respond.

**Fix:**
1. Check the **Console** view to confirm data is being sent.
2. Verify the transmit function returns a properly terminated string (most devices expect `\r\n`).
3. Confirm the correct **Target Device** is selected in multi-source projects.
4. Check that your Pro license is active — transmission is disabled without it.

### Slider Sends Too Many Commands

**Symptom:** The device is overwhelmed or the serial buffer overflows while dragging a slider.

**Fix:** The built-in 50 ms rate limit prevents most flooding, but if your device needs more time between commands, increase the step size to reduce the number of discrete values, or add debouncing logic in your transmit function.

### Transmit Function Error

**Symptom:** A red error indicator appears on the control.

**Fix:** Open the Project Editor and check the transmit function for syntax errors. The function must be a valid JavaScript function named `transmit` that accepts one parameter and returns a string.

## Tips

- Start with a built-in template and modify it — this avoids common syntax mistakes.
- Test with the Console view open to see exactly what bytes are being transmitted.
- Use the Ramp Generator to stress-test your device's command handling at sustained rates.
- Combine output controls with input widgets in the same dashboard for full closed-loop monitoring (e.g., a slider to set a target temperature alongside a gauge showing the actual temperature).
- Use the built-in protocol helpers (`modbusWriteRegister`, `canSendFrame`, etc.) instead of manually packing binary bytes — see [Protocol Helper Functions](#protocol-helper-functions) above.
- For complex protocols beyond the built-in helpers, write custom helper functions inside the transmit function scope — variables declared outside `transmit()` persist across calls.

## See Also

- [Actions](Actions.md) — simple command buttons with timer support.
- [Project Editor](Project-Editor.md) — complete guide to creating and configuring projects.
- [Widget Reference](Widget-Reference.md) — all input/visualization widget types.
- [Frame Parser Scripting](JavaScript-API.md) — Lua and JavaScript parser reference.
- [Data Sources](Data-Sources.md) — configuring device connections.
