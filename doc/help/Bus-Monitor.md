# Bus monitor (Pro)

Your PLC polls three drives over RS-485 and one of them drops out a few times an hour. The dashboard shows a gap; it does not show why. The bus monitor lists every message on the line with its time, and in Modbus RTU mode it says which request went unanswered, which reply failed its CRC and which slave answered with an exception.

## Overview

The bus monitor is a panel in the console. It has two modes:

| Mode | What it shows |
|------|---------------|
| **Messages** | Every send and every received burst of bytes, with its time, direction and length |
| **Modbus RTU** | Every Modbus RTU frame on the line, labelled master or slave, with request and response paired and faults diagnosed |

The bus monitor is a Serial Studio Pro feature. Its **Bus Monitor** button appears in the console toolbar when a license or the trial is active; without a license, the first use of a Pro feature offers a trial through a Yes/No prompt (it needs an internet connection). See [Pro vs Free](Pro-vs-Free.md).

It needs no project and works in Console Only, Quick Plot and Project File mode, on any connection the console shows. It only reads: nothing it does transmits. With several devices connected it follows the device selected in the console.

The monitor records only while its panel is open. Messages that pass while the panel is closed are not kept, and reopening the panel starts with empty buffers.

## Quick start: watch a Modbus RTU line

1. Wire a USB to RS-485 adapter in parallel with the bus (A to A, B to B, common to common). The adapter only listens.
2. In the **Setup** panel choose **UART**, pick the adapter's port and set the baud rate, parity and stop bits of the bus. Select **Console Only (No Parsing)** and click **Connect**. Do not use the Modbus driver for this: it is a master and would poll the line itself.
3. Click **Bus Monitor** in the console toolbar. If the toolbar is narrow, the button sits under **Utilities**.
4. Set the mode to **Modbus RTU**.

Rows appear as the master polls. A healthy exchange is two rows:

| Time | Role | Slave | Function | Length | Data Summary | CRC | Diagnosis |
|------|------|-------|----------|--------|--------------|-----|-----------|
| 10:06:13.413 | Master | 1 | Read Holding Registers | 8 | Start address 0x0010, Quantity 2 | OK | |
| 10:06:13.431 | Slave | 1 | Read Holding Registers | 9 | U16: [10, 20] | OK | |

Unplug the slave and the master's row turns red: `Slave 1 did not respond`.

## Controls

| Control | Mode | What it does |
|---------|------|--------------|
| Mode | both | Switches between **Messages** and **Modbus RTU**. Rows already listed stay; buffered bytes and request pairing start over |
| **Pause** / **Resume** | both | While paused, incoming data is discarded and not counted. Resuming starts from clean buffers |
| **Clear** | both | Empties the table, the timeline and the counters |
| **Response timeout** | Modbus RTU | How long a request waits for its response: 10 to 10,000 ms, default 500 ms |
| **Quiet time** | Messages | Idle time that ends a received message: 1 to 1000 ms, default 10 ms |
| **Issues only** | Modbus RTU | Keeps only the rows with a warning or an error |
| **U16**, **S16**, **U32**, **S32**, **F32**, **BIN** | Modbus RTU | Formats used to show register values in the Data Summary. Tick several to see them side by side |

The line at the right of the controls counts messages, errors and warnings since the last clear, and shows the average interval between the listed messages. Removing old rows does not lower the counts.

The monitor keeps the most recent 5000 messages. Its settings are saved with the application.

## Table

| Column | Meaning |
|--------|---------|
| Time | When the message completed, to the millisecond |
| Role, or Direction | **Master**, **Slave** or **Unknown** in Modbus RTU mode; **TX** or **RX** in Messages mode |
| Slave | The slave address. `-` for bytes that did not verify as a frame |
| Function | The function name, such as Read Holding Registers. An exception names the function it refuses |
| Length | Bytes in the whole frame, address and CRC included |
| Data Summary | What a request asks for, or the values a response carries |
| CRC | **OK**, or **Failed** for bytes that never verified |
| Diagnosis | Every finding on the row, separated by semicolons |
| Raw Frame | The bytes in hexadecimal |

A row with an error is tinted red and a row whose worst finding is a warning is tinted yellow. The table follows the newest row until you scroll up, and follows again when you scroll back to the end. In Messages mode the Modbus columns are hidden.

In the Data Summary, 32-bit formats read two registers per value with the high word first, so an odd last register is not shown. With no format ticked the registers are shown in hexadecimal. Coils are listed as bits, lowest bit of each byte first.

## Diagnoses

Errors, shown in red:

| Diagnosis | Meaning |
|-----------|---------|
| CRC check failed | Bytes that match no frame with a valid CRC. They are listed as one row |
| Incomplete frame | Bytes still unframed after the line stayed idle for 100 ms |
| Slave N did not respond | No response within the timeout, or the master sent its next request first |
| Response does not match the request | The response came from another slave or answers another function |
| Write response differs from the request | A write was echoed with another address, value or quantity |
| Byte count does not match the quantity | A read returned more or fewer bytes than were asked for |
| Undefined exception code | An exception reply with a code the protocol does not define |
| Request parameters are out of range | An address above 247, a read sent to the broadcast address, or a quantity, value or byte count outside the protocol's limits |

Warnings, shown in yellow:

| Diagnosis | Meaning |
|-----------|---------|
| Late response | A response that arrived after its request had timed out |
| Response without a request | A response with no request before it. Common for the first row after the panel opens |
| Slave reported an exception | An exception reply with a standard code, named in the Data Summary |
| Unknown or custom function code | A function the monitor has no length rule for |

Information, not tinted: **Broadcast write, no response expected** for a write to address 0.

The no-response diagnosis is added to the request's own row, so a silent slave does not add rows. It appears when the timeout passes even if nothing else is sent.

## Timeline

The **Timeline** tab draws the same messages as bars on a time axis. Requests and sends are in the upper lane, responses and received messages in the lower one. Bars take the color of their severity.

- The window is 5 seconds by default. **Zoom In** and **Zoom Out** halve and double it, between 200 ms and 10 minutes. The mouse wheel does the same.
- The right edge follows the newest message.
- Hover a bar to see its time, role, length, bytes and diagnosis.

A slave that answers late shows as a wider gap between a bar in the upper lane and the one below it; a slave that stops answering leaves red bars in the upper lane with nothing under them.

## How frames are found

On the wire, Modbus RTU frames are separated by silence. A serial adapter delivers bytes in chunks that do not preserve those silences, so the monitor does not rely on them. It reads the function code, works out the lengths a request or a response with that function can have, and accepts the one whose CRC verifies. After bytes that verify as nothing, it moves forward one byte at a time until frames verify again, and lists the skipped bytes as one failed row.

Whether a frame is a request or a response follows from its function and length. Where the two look alike (a write echo, or a coil response of the same size as its request), a request that is waiting for an answer from the same slave decides.

## Limits

- Modbus RTU only. Modbus TCP and Modbus ASCII are not decoded.
- Times are those of the chunk a frame completed in. Two frames delivered in one chunk share a time, so interval readings are as fine as the adapter's buffering, usually 1 to 16 ms. Lower the adapter's latency timer for finer readings.
- A custom function code is listed as a frame only when it arrives as a chunk of its own.
- A master that repeats a write before the slave answers can be read as the echo of the first one.
- After a damaged frame, up to a few hundred bytes can be listed a moment late while the monitor finds the next valid frame.
- Sniffed registers are not turned into dashboard values. To plot them, read them with the [Modbus driver](Drivers-Modbus.md).

## See also

- [Modbus Driver](Drivers-Modbus.md): poll a device as a Modbus master and plot its registers.
- [Console Annotations](Console-Annotations.md): label byte ranges of any protocol with a decoder script.
- [Console Automation](Console-Automation.md): command lists, auto-reply rules and color rules.
- [UART / Serial Driver](Drivers-UART.md): serial port settings and the control-line indicators.
- [Toolbar & Button Reference](Toolbar-Reference.md): every console toolbar button.
