# Console automation

Your bootloader prints `READY` and waits for `GO`. You could sit and type it every time, or let the console answer. Console automation covers three small tools in the console: a command list that sends pinned commands in order, an auto-reply table that answers matching messages, and color rules that paint words such as `ERROR` so they stand out in a busy log.

## Overview

All three tools are available in the free and Pro editions and need no project. They work in Console Only and Quick Plot mode as well as with a project file. Their settings are stored with the application, not in the project file, so opening someone else's project never changes what your console sends.

| Tool | Where it is | What it does |
|------|-------------|--------------|
| Command list | The command library popover next to the send field | Sends the ticked pins in order, each after its own delay, once or in a loop |
| Auto Reply | **Auto Reply** in the console toolbar | Sends a reply when a received message matches a rule |
| Colors | **Colors** in the console toolbar | Draws chosen words in chosen colors |

If the console toolbar is narrow, **Auto Reply** and **Colors** sit under **Utilities**. Their panels open below the console output, in the same place as the [annotation panel](Console-Annotations.md), one panel at a time.

Nothing here transmits on its own. A command list runs when you press **Run List** or **Loop**, and Auto Reply answers only after you press **Start Replying** in the current session. Both stop when the device disconnects. In a deployment package opened as a sealed session, the command list controls and Auto Reply are hidden; Colors stays available.

## Command list

Pin the commands you send often, then send them as a sequence.

1. Send a command from the console, open the command library popover next to the send field, and pin it with the plus button. A pin remembers the text and the send settings in effect when it was pinned: hexadecimal or text, the line ending, the checksum and the text encoding.
2. Repeat for each command. Use the arrow buttons to put the pins in the order they should be sent.
3. Tick the pins that belong to the list and type a delay in milliseconds in the field beside each one. The delay is how long the list waits before sending that pin.
4. Press **Run List** to send every ticked pin once, or **Loop** to start over after the last one until you press **Stop**.

A reset-and-query sequence for an instrument looks like this:

| Pin | Delay (ms) | Sent |
|-----|-----------|------|
| `*RST` | 0 | Immediately |
| `*CLS` | 500 | Half a second after `*RST` |
| `*IDN?` | 1000 | One second after `*CLS` |

A highlight bar marks the pin that is sent next while the list runs.

| Setting | Range | Default |
|---------|-------|---------|
| Delay per pin | 0 to 3,600,000 ms | 1000 ms |
| Pins | Up to 100 | none |

Details worth knowing:

- Each pin is sent with its own stored settings, not with whatever the send field shows at that moment. A list can mix text and hexadecimal pins.
- The list is copied when the run starts. Editing, moving or deleting pins during a run does not change what that run sends.
- A delay of 0 still takes about one millisecond, so a loop of zero-delay pins cannot freeze the window.
- While the connection is paused the list holds its position and sends nothing. It continues when you resume.
- A command list and cyclic send cannot run together. Stop one to start the other.
- List sends are not added to the send history.

## Auto Reply

Auto Reply compares each received message with a table of rules and sends the reply of every rule that matches, back to the device the message came from.

### Auto Reply quick start

1. Connect the device and click **Auto Reply** in the console toolbar.
2. Click **Add Rule**. A new rule looks for `OK` and answers `ACK` followed by a newline. It starts unticked.
3. Change what the rule looks for and what it answers. For the bootloader above: **Starts with**, `READY`, reply `GO`.
4. Tick the rule.
5. Click **Start Replying**. The button stays pressed and a counter shows how many replies were sent.

### Rule fields

| Field | What it means |
|-------|---------------|
| Tick box | Only ticked rules are compared. New rules start unticked |
| **When a message** | How the message is compared; see the match modes below |
| **Hex** (first) | Treats the rule as hexadecimal: the text is bytes such as `01 03 00`, and the message is a burst of bytes instead of a line |
| **This text or bytes** | What to look for, up to 256 characters |
| **Send this reply** | The reply payload. A text reply accepts escape sequences such as `\r` |
| **Hex** (second) | Sends the reply as hexadecimal bytes |
| **Line ending** | Appended to the reply: none, new line, carriage return or both |
| **Checksum** | Appended last, computed over the reply and its line ending |

### Match modes

| Mode | Matches when | Example |
|------|--------------|---------|
| **Starts with** | The message begins with the text | `OK` matches `OK 12` |
| **Ends with** | The message ends with the text | `READY` matches `boot READY` |
| **Equals** | The message is exactly the text | `PING` matches only `PING` |
| **Contains** | The text appears anywhere | `ERR` matches `x ERR y` |
| **Pattern** | A byte pattern appears anywhere | `5D ?? 7F` matches `5D 01 7F` |
| **Regular expression** | The expression matches anywhere in the line. Text rules only | `ERR[0-9]+` matches `a ERR42 b` |

All modes are case-sensitive. **Pattern** uses the same syntax as the Expect Reply step of a [Connection Sequence](Drivers-UART.md#connection-sequence): hexadecimal bytes where `??` accepts any byte and `5?` any byte whose high half is 5, with `|` between alternatives. A pattern works on text rules too, because it compares bytes.

### What counts as a message

- **Text rules** see one line at a time. A line ends at a line feed; a carriage return before it is dropped. It does not matter how the device splits a line across packets.
- **Hexadecimal rules** see everything received until the line stays quiet for the **Quiet time** at the bottom of the panel (1 to 1000 ms, default 10 ms).

A message longer than 4096 bytes is compared on its first 4096 bytes. For a hexadecimal rule, the rest of that burst is ignored until the line goes quiet again.

### Auto Reply safety rules

- Auto Reply is never on when Serial Studio starts, when you connect, or when a project loads. Rules are saved; the pressed state of **Start Replying** is not.
- Disconnecting stops it. Reconnecting does not start it again.
- If more than 50 replies are sent within any one second, Auto Reply stops and writes one line to the console saying so. This is what protects a device that echoes everything it receives from turning one rule into a flood.
- Replayed data from a recording or the CSV player never triggers a reply.

### Auto Reply limits

- Up to 64 rules. When several rules match one message, each sends its reply, in table order.
- Replies are sent from the user interface, a few milliseconds after the message arrives. That is fine for handshakes and device simulation. It is not a real-time bus responder.
- A regular expression is evaluated on every received line. A slow expression slows the console, so prefer the plain modes when they do the job.
- An edit that would make a rule invalid is refused and the reason appears at the bottom of the panel. The rule keeps its previous contents.

## Colors

A color rule draws one word in a text color and a fill color wherever it appears in the console.

1. Click **Colors** in the console toolbar.
2. Click **Add Rule**. A new rule colors `ERROR` white on red.
3. Type the word to color. Clear **Match case** to color `error` and `Error` as well.
4. Click the **Text** and **Fill** swatches to pick colors. The **Preview** column shows the result.

| Setting | Limit |
|---------|-------|
| Rules | Up to 32 |
| Word length | Up to 64 characters |
| Occurrences colored per line | 64 |

How the rules behave:

- A rule applies to everything on screen, including lines received before the rule existed, in the Console pane and in the dashboard console window alike.
- Only the display changes. Copied text, the Console Log and exported data stay exactly as received.
- Where two rules overlap, the one higher in the table wins. Put `TIMEOUT` above `OUT` if both are rules.
- A rule's colors replace the colors a device sets with ANSI escape codes for the same characters.
- While **Find** is open the color rules are not drawn, so search matches stay readable.
- Rules match plain words. There are no wildcards.
- Color rules are not drawn when the interface uses a right-to-left language.

## See also

- [Toolbar & Button Reference](Toolbar-Reference.md): every console toolbar button and send-row control.
- [Console Annotations](Console-Annotations.md): label ranges of incoming bytes with a decoder.
- [Bus Monitor](Bus-Monitor.md): message timing and Modbus RTU fault diagnosis (Pro).
- [Actions](Actions.md): one-click commands stored in a project, with their own timers.
- [UART / Serial Driver](Drivers-UART.md#connection-sequence): steps that run once on every connect, including Expect Reply patterns.
