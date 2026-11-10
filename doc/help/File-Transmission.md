# File transmission (Pro)

Your board has a bootloader that waits for a firmware image on the serial port, or a sketch that reads configuration lines. File Transmission sends that file from Serial Studio over the connection that is already open. Typical jobs: flashing an image through a bootloader, loading a script onto a controller, replaying a list of AT commands.

The device has to be listening for the file. Serial Studio cannot make a board accept one: a sketch that only calls `Serial.println` ignores an XMODEM transfer.

The lowest-risk first transfer needs no handshake. Flash this sketch, which echoes each line it receives:

```cpp
void setup() { Serial.begin(115200); }

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('
');
    Serial.print("got: ");
    Serial.println(line);
  }
}
```

Connect to the board, open **File Transmission**, choose **Plain Text**, select a `.txt` file and click **Begin Transmission**. Each line of the file comes back in the console.

> **Pro feature.** File Transmission needs the commercial build with a Pro license; GPL builds compiled from source do not include it. Without a license, opening it offers the Pro trial (a Yes/No prompt; registering needs an internet connection). See [Pro vs Free](Pro-vs-Free.md).

## Send a file

1. Connect to the device.
2. Open **File Transmission** (see [Opening the File Transmission dialog](#opening-the-file-transmission-dialog)).
3. Pick the mode in **Transfer Protocol**.
4. Click **Select File...** and choose the file.
5. Adjust the settings the chosen mode shows (see the [Settings reference](#settings-reference)).
6. For XMODEM, XMODEM-1K, YMODEM and ZMODEM, make your device (the receiver) start waiting for the file. That is a bootloader or firmware action, not a Serial Studio setting. Then click **Begin Transmission**.

After you click **Begin Transmission**, the same button does the rest. In Plain Text and Raw Binary it reads **Pause Transmission** mid-transfer and **Resume Transmission** while paused. In the protocol modes it reads **Stop Transmission** and cancels the transfer. Closing the dialog clears the selected file.

## Choosing a mode

A CRC is a checksum the receiver uses to detect corrupted blocks. EOT is the end-of-transmission byte that closes an XMODEM-family transfer.

If your bootloader names a protocol, use that one. If you do not know, look for "XMODEM", "YMODEM" or "ZMODEM" in the bootloader's manual; if none is mentioned, use Plain Text for lines of text or Raw Binary for bytes the device parses itself.

| Mode        | Error detection  | Block size                | Pause/resume     | Best for |
|-------------|------------------|---------------------------|------------------|----------|
| Plain Text  | None             | One line                  | Yes              | Human-readable config files, AT commands |
| Raw Binary  | None             | 1,024 B (64-8,192 B)      | Yes              | Simple binary uploads without protocol overhead |
| XMODEM      | CRC-16           | 128 B                     | No               | Legacy devices, small files |
| XMODEM-1K   | CRC-16           | 1,024 B                   | No               | Legacy devices, larger files |
| YMODEM      | CRC-16           | 1,024 B                   | No               | Files where the receiver needs the filename and size |
| ZMODEM      | CRC-32           | 1,024 B (64-8,192 B)      | No               | Large files, unreliable links, modern firmware loaders (resumes from an offset after a crash) |

## Opening the File Transmission dialog

Connect to a device first. Then open the **Start** menu on the dashboard taskbar and choose **Tools** > **File Transmission**. If you pinned it under **Preferences** > **Taskbar** > **Pinned Buttons**, the taskbar button opens it too. In the console, the attach button in the send row (tooltip "Send a file to the connected device") opens the same dialog.

In a runtime-mode deployment the dialog opens only when the build was packaged with `--file-transmission` and Pro is active. The dialog closes when the connection drops.

## Transfer modes

### Plain text

In **Plain Text** mode (File Transmission, Pro), Serial Studio sends the file line by line as text, one line every **Transmission Interval**. A line that does not end with a newline gets `\n` appended; a line that already ends in `\r\n` keeps it. A device that needs a bare `\r` is not served by this mode.

Raise **Transmission Interval** if the device needs time to process each line before accepting the next (default 100 ms, range 0-10,000 ms). Pause and resume continue from where the transfer left off.

### Raw binary

In **Raw Binary** mode (File Transmission, Pro), Serial Studio sends the file in fixed-size blocks with no framing or error checking, for receivers that expect raw bytes and check integrity themselves.

**Block Size** (default 1,024 bytes, range 64-8,192) and **Transmission Interval** (default 100 ms, range 0-10,000 ms) set the pace. Throughput is block size divided by interval, so the defaults give about 10 KB/s; lower the interval for large files. The last block may be smaller than the configured size. Pause and resume work as in Plain Text.

### XMODEM

In **XMODEM** mode (File Transmission, Pro), Serial Studio uses a byte-oriented protocol older than most of the devices it talks to. It sends 128-byte blocks with CRC-16 error detection. The receiver starts the transfer by sending a `C` character to request CRC mode, so start the receiver first.

**Timeout** (default 10,000 ms, range 1,000-60,000) bounds each wait for the receiver, including the wait for that first `C`. **Max Retries** (default 10, range 1-100) is how many timeouts or NAKs a transfer tolerates before it gives up.

1. Serial Studio waits for the receiver to send `C`.
2. Each 128-byte block is sent with a sequence number and CRC-16 checksum.
3. The receiver answers ACK (success) or NAK (retransmit).
4. After the last block, Serial Studio sends EOT.

The last block is padded with `0x1A` (Ctrl-Z) up to the block size. XMODEM carries no file size, so the receiver cannot strip the padding; use YMODEM or ZMODEM when the exact size matters. Only CRC-16 mode is supported, not the legacy checksum mode.

### XMODEM-1K

In **XMODEM-1K** mode (File Transmission, Pro), Serial Studio uses XMODEM with 1,024-byte blocks, which cuts protocol overhead for larger files. It uses **Timeout** (default 10,000 ms) and **Max Retries** (default 10), waits for the receiver's `C` and pads the last block like XMODEM.

### YMODEM

In **YMODEM** mode (File Transmission, Pro), Serial Studio uses XMODEM-1K plus a metadata block that carries the filename and file size, so the receiver knows what it is getting before the data arrives. It uses **Timeout** (default 10,000 ms) and **Max Retries** (default 10).

1. Serial Studio sends block 0 with the filename and file size.
2. Data goes out in 1,024-byte blocks with CRC-16.
3. After the data, Serial Studio sends EOT twice, per YMODEM convention.
4. An empty block 0 signals end-of-batch.

The receiver can use the file size from block 0 to strip the padding from the last block. Filenames longer than 104 bytes are truncated in block 0.

### ZMODEM

In **ZMODEM** mode (File Transmission, Pro), Serial Studio uses a streaming protocol: it does not wait for an acknowledgment after each block, so link latency does not cap throughput. It uses CRC-32 and supports crash recovery.

Start the receiver first (for example, the bootloader's ZMODEM receive command), then click **Begin Transmission**.

**Block Size** sets the data subpacket size (default 1,024 bytes, range 64-8,192). **Timeout** (default 10,000 ms) and **Max Retries** (default 10) apply as in XMODEM.

- Data subpackets stream continuously without per-packet ACKs.
- A receiver restarting after an interrupted transfer can ask the sender to resume from a given byte offset (ZRPOS) instead of starting over.
- The file header carries the filename, size and modification time.

## If nothing happens

The usual cause is a receiver that is not waiting yet. Start the receiver on the device first, then click **Begin Transmission**. In the four protocol modes, Serial Studio waits up to **Timeout** for the receiver to start the transfer (a `C` in the XMODEM family), retries up to **Max Retries** times, then the status text shows "Transfer failed: Timeout: no response from receiver". Plain Text and Raw Binary have no handshake, so a device that is not listening misses the data. Clicking **Begin Transmission** at a sketch that only calls `Serial.println` is like handing a book to someone who is not in the room.

## Progress and log

During an active transfer, the dialog shows:

- **Progress: N%** with a progress bar.
- **Transfer speed.** Current throughput in B/s, KB/s, or MB/s.
- **Bytes sent / total.** Absolute byte counters.
- **Errors: N.** Number of protocol-level errors (NAKs, retries, timeouts). It appears after the first error and only counts in the protocol modes.
- **Status text.** Current protocol state or last event.

The **Activity Log** at the bottom of the dialog lists timestamped events: block transmissions, acknowledgments, errors, retries, and completion status. It keeps the most recent 200 entries. Click **Clear** to reset it.

## Settings reference

The dialog does not persist these settings; they return to the defaults below each time Serial Studio starts, and the dialog disables the controls while a transfer is active.

| Setting                | Applies to                              | Range             | Default    |
|------------------------|-----------------------------------------|-------------------|------------|
| Transmission Interval  | Plain Text, Raw Binary                  | 0-10,000 ms       | 100 ms     |
| Block Size             | Raw Binary, ZMODEM                      | 64-8,192 bytes    | 1,024 bytes|
| Timeout                | XMODEM, XMODEM-1K, YMODEM, ZMODEM       | 1,000-60,000 ms   | 10,000 ms  |
| Max Retries            | XMODEM, XMODEM-1K, YMODEM, ZMODEM       | 1-100             | 10         |

The ranges are the dialog's spin box limits. Block Size steps by 64 and Timeout by 1,000 ms.

## Tips

- If a slow receiver drops data, raise **Transmission Interval** (Plain Text, Raw Binary) or **Timeout** (protocol modes).
- On a noisy link, lower **Block Size** in ZMODEM so a failed subpacket resends less data.

## See also

- [Operator Deployments](Operator-Deployments.md): the `--file-transmission` flag exposes this dialog in runtime-mode builds.
- [Toolbar Reference](Toolbar-Reference.md): the Start menu and pinned taskbar entries for File Transmission.
- [Pro vs Free](Pro-vs-Free.md): edition gating for this feature.
- [Communication Protocols](Communication-Protocols.md): the connections File Transmission sends data over.
