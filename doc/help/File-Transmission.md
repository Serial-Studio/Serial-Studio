# File transmission

Serial Studio can send files to a connected device using plain text, raw binary, or industry-standard transfer protocols (XMODEM, XMODEM-1K, YMODEM, ZMODEM). It's handy for uploading firmware, configuration files, scripts, or any other data to an embedded target over an active connection.

> **Availability.** File transmission is currently in the nightly build. It ships in the public release starting with version 3.2.8.

## Protocol overview

```mermaid
flowchart LR
    File["Local File"] --> SS["Serial Studio"]
    SS -->|Plain Text / Raw Binary| Dev["Connected Device"]
    SS -->|XMODEM / YMODEM / ZMODEM| Dev
    Dev -->|ACK / NAK / ZRPOS| SS
```

| Mode        | Error detection  | Block size  | Pause/resume     | Best for |
|-------------|------------------|-------------|------------------|----------|
| Plain Text  | None             | Line-based  | Yes              | Human-readable config files, AT commands |
| Raw Binary  | None             | 64-8192 B   | Yes              | Simple binary uploads without protocol overhead |
| XMODEM      | CRC-16           | 128 B       | No               | Legacy devices, small files |
| XMODEM-1K   | CRC-16           | 1024 B      | No               | Legacy devices, larger files |
| YMODEM      | CRC-16           | 1024 B      | No               | Files where the receiver needs the filename and size |
| ZMODEM      | CRC-32           | 64-8192 B   | Crash recovery   | Large files, unreliable links, modern firmware loaders |

---

## Opening the File Transmission dialog

Click the **File Transmission** button in the toolbar while a device connection is active. The dialog closes automatically if the connection drops.

---

## Transfer modes

### Plain text

Sends the file line by line as text. Each line is terminated with a newline. A good fit for sending scripts, AT command sequences, or configuration files to devices that process text input.

**Configuration:**

- **Transmission interval.** Delay between consecutive lines (0 to 10,000 ms, default 100 ms). Raise this if the device needs time to process each line before accepting the next.

**Behavior:**

- Lines are read sequentially from the file.
- A newline is appended automatically if the line doesn't already end with one.
- You can pause and resume transmission. It continues from where it left off.

---

### Raw binary

Sends the file in fixed-size binary blocks with no framing or error checking. Use this when the receiver expects raw bytes and handles its own integrity checks.

**Configuration:**

- **Block size.** Bytes per block (64 to 8,192, default 1,024).
- **Transmission interval.** Delay between consecutive blocks (0 to 10,000 ms, default 100 ms).

**Behavior:**

- The file is read in sequential chunks of the configured block size.
- The last block may be smaller than the configured size.
- You can pause and resume transmission.

---

### XMODEM

A classic byte-oriented protocol that sends data in 128-byte blocks with CRC-16 error detection. The receiver initiates the transfer by sending a `C` character to request CRC mode.

**Configuration:**

- **Timeout.** How long to wait for receiver responses (1,000 to 60,000 ms, default 10,000 ms).
- **Max retries.** Retry attempts per block on NAK or timeout (1 to 100, default 10).

**Protocol flow:**

1. Serial Studio waits for the receiver to send `C` (CRC mode request).
2. Each 128-byte block is sent with a sequence number and CRC-16 checksum.
3. The receiver responds with ACK (success) or NAK (retransmit).
4. After all blocks are sent, Serial Studio sends EOT to signal completion.

**Notes:**

- Files smaller than 128 bytes are padded to fill the block.
- Only CRC-16 mode is supported (not the legacy checksum mode).

---

### XMODEM-1K

Identical to XMODEM but uses 1,024-byte blocks instead of 128-byte, which cuts protocol overhead for larger files.

**Configuration:**

- Same as XMODEM: **Timeout** and **Max retries**.

---

### YMODEM

Extends XMODEM-1K with a metadata block that carries the filename and file size, so the receiver knows what it's getting before the data arrives.

**Configuration:**

- Same as XMODEM: **Timeout** and **Max retries**.

**Protocol flow:**

1. Serial Studio sends block 0 with the filename and file size.
2. Data is transmitted in 1,024-byte blocks with CRC-16.
3. After the data, Serial Studio sends EOT (twice, per YMODEM convention).
4. An empty block 0 signals end-of-batch.

**Notes:**

- The receiver can use the file size from block 0 to strip padding from the last block.

---

### ZMODEM

A streaming protocol that doesn't wait for per-block acknowledgment, which makes it much faster than XMODEM/YMODEM on high-latency or high-throughput links. It uses CRC-32 for stronger error detection and supports crash recovery.

**Configuration:**

- **Block size.** Bytes per data subpacket (64 to 8,192, default 1,024).
- **Timeout.** How long to wait for receiver responses (1,000 to 60,000 ms, default 10,000 ms).
- **Max retries.** Retry attempts on error (1 to 100, default 10).

**Key features:**

- **Streaming.** Data subpackets flow continuously without waiting for ACK after each one, maximizing throughput.
- **Crash recovery.** If a transfer is interrupted and restarted, the receiver can request retransmission from a specific file offset (ZRPOS), so you don't resend data that already arrived.
- **File metadata.** The ZFILE header carries the filename, size, and modification timestamp.
- **ZDLE escaping.** Control characters are transparently escaped, so the data stream doesn't interfere with terminal or modem control sequences.

---

## Progress and status

During an active transfer, the dialog shows:

- **Progress bar.** Percent of the file transmitted.
- **Transfer speed.** Current throughput in B/s, KB/s, or MB/s.
- **Bytes sent / total.** Absolute byte counters.
- **Error count.** Number of protocol-level errors (NAKs, retries, timeouts). Only incremented during protocol-based transfers.
- **Status text.** Current protocol state or last event.

---

## Activity log

The bottom section of the dialog has a scrollable activity log with timestamped events: block transmissions, acknowledgments, errors, retries, and completion status. It keeps the most recent 200 entries. Click **Clear** to reset it.

---

## Settings reference

All settings are saved automatically and restored between sessions.

| Setting                | Applies to                              | Range             | Default    |
|------------------------|-----------------------------------------|-------------------|------------|
| Transmission interval  | Plain Text, Raw Binary                  | 0-10,000 ms       | 100 ms     |
| Block size             | Raw Binary, ZMODEM                      | 64-8,192 bytes    | 1,024 bytes|
| Timeout                | XMODEM, XMODEM-1K, YMODEM, ZMODEM       | 1,000-60,000 ms   | 10,000 ms  |
| Max retries            | XMODEM, XMODEM-1K, YMODEM, ZMODEM       | 1-100             | 10         |

---

## Tips

- **Picking a protocol.** Use ZMODEM when you can. It's the fastest and most robust. Fall back to XMODEM/YMODEM only if the receiver doesn't support ZMODEM.
- **Slow receivers.** Raise the transmission interval (Plain Text / Raw Binary) or the timeout (protocol modes) if the device can't keep up.
- **Noisy links.** Use smaller block sizes to cut the cost of retransmissions. For ZMODEM, 256 or 512 bytes is a good starting point on unreliable connections.
- **Large files.** XMODEM-1K, YMODEM, or ZMODEM all reduce per-block overhead compared to standard XMODEM.
- **Firmware uploads.** Many bootloaders support XMODEM or YMODEM natively. Check your device docs for the expected protocol.
