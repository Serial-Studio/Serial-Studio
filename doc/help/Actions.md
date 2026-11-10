# Actions

## Overview

Your sketch reads a line from `Serial` and resets itself when it sees `RST`. In a serial monitor you would type `RST` and press Enter. An action puts that same command on a dashboard button.

```cpp
void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd == "RST") resetDevice();
  }
}
```

To make the button, create an action with these fields in the Project Editor:

| Property             | Value           |
|----------------------|-----------------|
| Action Title         | Reset Device    |
| Transmit Data        | `RST`           |
| End-of-Line Sequence | New Line (`\n`) |

The **Reset Device** button sits in a strip above the dashboard widgets and becomes clickable once you connect. Clicking it sends the four bytes `52 53 54 0A` (the letters R, S, T, then a newline) and saves you typing `RST` and Enter on every reset. The steps are under [Creating an action](#creating-an-action).

Actions are available in the free edition. They belong to a project file, so open or create a project first (see [Project Editor](Project-Editor.md)). For continuous controls such as sliders and knobs, or for custom JavaScript transmit functions, see [Output Controls](Output-Controls.md): a Pro feature, and the first use offers a free trial (internet needed to register).

## Creating an action

1. Open or create a project, then open the Project Editor (toolbar wrench icon).
2. Click **Action** in the Project Editor toolbar (in the Output section).
3. Select the new action in the project tree. Its properties open in the panel on the right. Rename it in **Action Title**.

## Action properties

### General information

**Action Title** is the label on the dashboard button (for example "Reset Device"), and **Action Icon** is picked from the built-in icon set; the icon also appears in the Project Editor flow diagram.

**Target Device** picks which connected device receives the command. The dropdown appears only when the project has more than one source. It lists the sources by title, and the default is the first source (shown as Device A unless you renamed it). The flow diagram draws an arrow from each action to its target.

### Data payload

- **Send as Binary.** When checked, the payload field becomes **Transmit Data (Hex)** and is read as hexadecimal bytes (for example `FF 01 A3`), with or without spaces. Anything else (a `0x` prefix, commas, an odd digit count) empties the payload.
- **Transmit Data.** The command to send, such as `RST` or `GET_TEMP`. In text mode these escapes are resolved before sending: `\n`, `\r`, `\t`, `\a`, `\b`, `\f`, `\v` and `\\`. Other escapes such as `\x41` are sent literally; use **Send as Binary** for arbitrary bytes.
- **Text Encoding.** The character encoding of the payload, default UTF-8. The list also offers UTF-16 LE/BE, Latin-1, System, GBK, GB18030, Big5, Shift-JIS, EUC-JP and EUC-KR. Hidden when **Send as Binary** is on.
- **End-of-Line Sequence.** Characters appended after the payload. If your sketch waits for a line, as with `readStringUntil('\n')`, choose New Line.
- **Checksum.** Default **No Checksum**. Any other choice appends the checksum bytes after the payload and the EOL, and the checksum covers both. It works in text and binary mode.

The bytes go out in this order: payload, EOL, checksum.

| End-of-Line option      | Bytes sent         |
|-------------------------|--------------------|
| None                    | Nothing appended   |
| New Line (`\n`)         | `0x0A`             |
| Carriage Return (`\r`)  | `0x0D`             |
| CRLF (`\r\n`)           | `0x0D 0x0A`        |

In text mode the EOL is encoded with the action's Text Encoding, so the byte values above hold for single-byte encodings such as UTF-8 and Latin-1; a UTF-16 action sends two bytes per character.

The End-of-Line Sequence is greyed out while **Send as Binary** is on, but a sequence you chose earlier is still appended after the hex bytes. Set it to None before you switch to binary, or put the bytes in the hex payload.

### Execution behavior

**Auto-Execute on Connect** sends the action once as soon as the device connects, which suits initialization commands such as a configuration string. On-connect sends go out in project-tree order.

The on-connect send is separate from the timer (see [Timer behavior](#timer-behavior)). If **Timer Mode** is not Off, Auto-Execute also starts the timer, so Start on Trigger and Toggle on Trigger begin repeating without a click, and Repeat N Times starts its series. Auto Start starts the timer without Auto-Execute. Either way the timer's first send comes one Interval after connect, so Repeat N Times with Auto-Execute sends Repeat Count + 1 messages in total.

### Timer behavior

Actions can repeat on a timer, which suits periodic polling or keep-alive commands. In the Project Editor, select an action and set **Timer Mode** (free edition).

| Timer Mode        | Behavior |
|-------------------|----------|
| Off               | Manual trigger only (default). Each click sends the command once. |
| Auto Start        | The timer starts when the device connects and repeats at the interval until the device disconnects. |
| Start on Trigger  | The timer starts on the first click and repeats until the device disconnects or the project's actions are rebuilt by an edit. Later clicks send the command once more without restarting the timer. To get a stop button, use Toggle on Trigger. |
| Toggle on Trigger | Each click starts or stops the repeating timer. Every click, including the one that stops it, also sends the command once. |
| Repeat N Times    | On a click, the first send is immediate, then one per interval until **Repeat Count** sends have gone out. Clicking again restarts the count. |

- **Action Interval (ms).** Milliseconds between repeats. Default 100, which is 10 commands per second and can overflow a small UART buffer. It must be greater than 0, otherwise the action never repeats (the app log shows a warning). It is greyed out while Timer Mode is Off.
- **Repeat Count.** Editable only in Repeat N Times mode. Default 3, minimum 1.

Timers keep ticking while the connection is paused. A tick sends nothing, but a Repeat N Times series still counts it, so it can run out with nothing sent. Timers exist only while a device is connected, and editing any action stops every running timer. Auto Start timers come back on the next connect.

## Dashboard appearance

The actions panel is a horizontal strip of buttons across the top of the dashboard, above the widgets. Each button shows the action icon and title. Buttons are greyed out until a device is connected, and while the connection is paused (nothing is sent then). In Toggle on Trigger mode the button shows as checked while its timer runs; no other mode changes the button.

The panel is hidden when the project has no actions or when **Show Actions Panel** is off in **Settings** -> **Layout**. Actions are also listed in the dashboard's start menu and in the Console's send library, so they stay reachable when the panel is hidden.

## Examples

### Poll a sensor

Request a reading every 500 ms:

| Property             | Value             |
|----------------------|-------------------|
| Action Title         | Poll Temperature  |
| Transmit Data        | `GET_TEMP`        |
| End-of-Line Sequence | CRLF (`\r\n`)     |
| Timer Mode           | Auto Start        |
| Interval (ms)        | 500               |

### Send bytes on connect

Send a binary packet as soon as the device connects. The bytes come from your device's datasheet; `AA 01 FF 00 55` is a placeholder.

| Property                | Value               |
|-------------------------|---------------------|
| Action Title            | Initialize Sensor   |
| Transmit Data (Hex)     | `AA 01 FF 00 55`    |
| Send as Binary          | On                  |
| Auto-Execute on Connect | On                  |

## Common mistakes

### Action button doesn't appear or is greyed out

**Symptom.** The action is configured in the Project Editor but no button shows up, or the button cannot be clicked.

**Fix:**

1. If the button is greyed out, connect the device and un-pause the connection. Buttons are clickable only while a connection is active and not paused.
2. If there is no panel at all, open **Settings**, go to the **Layout** tab and turn on **Show Actions Panel**. When it is off, the panel stays hidden even while connected.

### Command not received by the device

**Symptom.** The button appears and can be clicked, but the device doesn't respond.

**Fix:**

1. Open the Console view and confirm the data is being sent.
2. Make sure the EOL setting matches what the firmware expects. Many embedded parsers need `\n` or `\r\n`.
3. In binary mode, make sure the hex is valid: pairs of hex digits, optionally separated by spaces. An odd digit count or any character other than 0-9, A-F and spaces (so no `0x` prefix, no commas) empties the payload. The EOL and checksum are still sent, and the only warning is in the app log.
4. In multi-source projects, check that **Target Device** is set correctly.
5. Check that the connection is not paused. Nothing is sent while it is.

## Tips

- For a request/response protocol, pair a timed action with a [frame parser](Frame-Parser.md): the action sends the request, the parser decodes the reply.

## See also

- [Output Controls](Output-Controls.md): interactive dashboard controls (button, slider, toggle, text field, knob) with custom JavaScript transmit functions; a Pro feature, and the first use offers a free trial.
- [Control Loop (setup / loop)](Control-Loop.md): automates a connected device with `setup()`/`loop()` scripts; the automated counterpart to a manual Action.
- [Project Editor](Project-Editor.md): full guide to creating and configuring projects.
