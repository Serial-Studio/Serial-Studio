#!/usr/bin/env python3
"""UDP frame simulator for the big_db_test load project.

Streams a large, steady set of binary frames over UDP so the big_db_test
Serial Studio project (``big_db_test.ssproj``) can be exercised without
hardware. It feeds many boards x many channels every cycle, which keeps the
parse + transform + dashboard hot path under continuous load -- intended for
manual testing and continuous hot-path load in CI.

Wire format (one frame per UDP datagram)
----------------------------------------
The project source is configured with ``decoder = Binary`` and
``frameDetection = NoDelimiters``, so each datagram is one complete frame with
no start/end delimiter. Each frame is packed exactly the way the project's
``frameParserCode`` expects::

    byte 0   ID high   ((id >> 8) & 0xFF)
    byte 1   ID low    (id & 0xFF)
    byte 2   length    (payload length, informational -- parser uses length)
    byte 3+  payload    d0..d7

11-bit frame ID layout::

    bits [9:8] board type   (1 = TA, 2 = TB, 3 = TC)
    bits [7:3] board id      (1..6)
    bits [2:0] reading id    (0/1 = data, 2 = diagnostics, 5 = aux [TB],
                              6 = info, 7 = boot)

IDs with bit 10 set are command frames (host to board); the parser ignores
them and this simulator never sends them.

Reading payloads (big-endian uint16 values)::

    0  channels 1-4   per-type encoding (see channel_value)
    1  channels 5-8   same encoding
    2  diagnostics    one status byte per channel (0x00 = healthy)
    5  aux            [accepted_mask, stored_flag, epoch_be32]   (TB only)
    6  info           one ASCII descriptor byte per channel (TB/TC only)
    7  boot           [reset_cause, selftest]

Suppression follows the device: the diagnostics frame is emitted only while a
channel is faulted (its absence means healthy), a data frame whose four
channels are all invalid is suppressed, the boot frame is one-shot at startup,
and the info/aux descriptor frames repeat every ~5 s. A ``--faults`` channel
carries the 0xFFFF invalid sentinel in its data frame paired with fault byte
0xA8 in diagnostics, exactly as the device pairs them.

Second stream (``--vib-port``)
------------------------------
The project's second source, "Vibration", is a plain-text UDP source decoded by
the built-in delimited parser: one line of four comma-separated integer counts
per sample, newline terminated. ``--vib-port`` turns the stream on; each cycle
sends the samples that fell due in one datagram, ``--vib-rate`` samples per
second. The four channels carry 50 Hz and 120 Hz tones (the project's spectrum
markers) at different mixes, in milli-g counts.

Usage
-----
    python3 big_db_test.py --host 127.0.0.1 --port 8080
    python3 big_db_test.py --port 8080 --vib-port 8081       # both sources
    python3 big_db_test.py --rate 20 --cycles 200            # bounded CI run
    python3 big_db_test.py --print                           # dump hex, send nothing

Point a Serial Studio Network/UDP source at the same host/port (or run with
``--print`` to inspect the byte stream). Stdlib only; no third-party deps.
"""

from __future__ import annotations

import argparse
import math
import signal
import socket
import sys
import time

# Board types (ID bits [9:8]).
TA = 1
TB = 2
TC = 3
TYPE_NAMES = {TA: "TA", TB: "TB", TC: "TC"}

# Reading ids (ID bits [2:0]).
READING_DATA_LO = 0
READING_DATA_HI = 1
READING_DIAG = 2
READING_AUX = 5
READING_INFO = 6
READING_BOOT = 7

NUM_CHANNELS = 8
BOARDS_PER_TYPE = 6

# Diagnostic status byte values.
DIAG_HEALTHY = 0x00
# A representative fault code (open / dead loop) used for fault injection; the
# device pairs it with the invalid sentinel in the data frame.
DIAG_FAULT = 0xA8
INVALID_VALUE = 0xFFFF

# Tones on the vibration stream, matching the project's spectrum markers.
VIB_TONE_LOW_HZ = 50.0
VIB_TONE_HIGH_HZ = 120.0

# Most sample lines in one datagram: keeps it under a 1500-byte MTU after a stall.
VIB_MAX_LINES = 48

# How often the low-rate descriptor frames (aux / info) are re-sent, in
# seconds, matching the device's ~5 s housekeeping cadence.
DESCRIPTOR_PERIOD_S = 5.0


def frame_id(board_type: int, board_id: int, reading_id: int) -> int:
    """Return the 11-bit frame ID for the given board/reading triplet."""
    return ((board_type & 0x03) << 8) | ((board_id & 0x1F) << 3) | (reading_id & 0x07)


def make_frame(
    board_type: int, board_id: int, reading_id: int, payload: bytes
) -> bytes:
    """Pack one [ID_hi, ID_lo, length, payload...] frame as the parser expects."""
    fid = frame_id(board_type, board_id, reading_id)
    return bytes(((fid >> 8) & 0xFF, fid & 0xFF, len(payload) & 0xFF)) + payload


def pack_u16(value: int) -> bytes:
    """Return a raw uint16 big-endian."""
    return bytes(((value >> 8) & 0xFF, value & 0xFF))


def healthy_u16(value: float) -> int:
    """Clamp to a healthy in-band uint16 (< 0xFFFD)."""
    return max(0, min(0xFFFC, int(round(value))))


def channel_value(board_type: int, board_id: int, channel: int, t: float) -> float:
    """Return a plausible, slowly varying raw reading for one channel.

    Each board/channel gets a distinct phase so the dashboard shows motion and
    no two tiles are identical. The per-type scaling just keeps the three board
    types in the numeric ranges the project's transforms expect.
    """
    phase = (board_id * 0.7) + (channel * 0.9)
    osc = 0.5 + 0.5 * math.sin((t * 0.5) + phase)  # 0..1
    if board_type == TA:
        return 10000.0 + 38.5 * (150.0 * osc)
    if board_type == TB:
        return 5000.0 + 14000.0 * osc
    return (800.0 * osc + 270.0) * 16.0


def build_cycle_frames(
    board_type: int,
    board_id: int,
    t: float,
    faults: dict,
    send_descriptors: bool,
    send_boot: bool,
    now_epoch: int,
) -> list:
    """Build every frame this board emits for one cycle, in send order.

    Mirrors the device's suppression semantics: diagnostics only while a
    channel is faulted, no all-invalid data frames, one-shot boot.
    """
    frames = []

    fault_channels = faults.get((board_type, board_id), ())  # 1-based channels
    if fault_channels:
        diag = bytearray(DIAG_HEALTHY for _ in range(NUM_CHANNELS))
        for channel in fault_channels:
            diag[channel - 1] = DIAG_FAULT
        frames.append(make_frame(board_type, board_id, READING_DIAG, bytes(diag)))

    for reading_id, first_ch in ((READING_DATA_LO, 1), (READING_DATA_HI, 5)):
        values = [
            (
                INVALID_VALUE
                if ch in fault_channels
                else healthy_u16(channel_value(board_type, board_id, ch, t))
            )
            for ch in range(first_ch, first_ch + 4)
        ]
        if any(v != INVALID_VALUE for v in values):
            payload = b"".join(pack_u16(v) for v in values)
            frames.append(make_frame(board_type, board_id, reading_id, payload))

    if send_boot:
        boot = bytes((0x10, 0x01))  # reset cause = software reset, self-test = pass
        frames.append(make_frame(board_type, board_id, READING_BOOT, boot))

    if send_descriptors:
        # Reading 6 (channel info): one ASCII descriptor byte per channel. Type
        # TA carries no descriptors; TB reports the acquisition mode ('A' =
        # current loop), TC the configured sensor type.
        if board_type != TA:
            code = ord("A") if board_type == TB else ord("K")
            info = bytes(code for _ in range(NUM_CHANNELS))
            frames.append(make_frame(board_type, board_id, READING_INFO, info))

        if board_type == TB:
            aux = bytes((0xFF, 0x01)) + now_epoch.to_bytes(4, "big")
            frames.append(make_frame(board_type, board_id, READING_AUX, aux))

    return frames


def vibration_lines(first_sample: int, count: int, rate: float) -> bytes:
    """Return ``count`` newline-terminated sample lines starting at ``first_sample``.

    Samples are indexed, not clocked, so the tones stay phase-continuous across
    datagrams whatever the send jitter.
    """
    two_pi = 2.0 * math.pi
    lines = []
    for n in range(first_sample, first_sample + count):
        t = n / rate
        low = math.sin(two_pi * VIB_TONE_LOW_HZ * t)
        high = math.sin(two_pi * VIB_TONE_HIGH_HZ * t)
        drift = math.sin(two_pi * 0.2 * t)
        counts = (
            1500.0 * low + 300.0 * high,
            400.0 * low + 1200.0 * high,
            (800.0 + 600.0 * drift) * high,
            250.0 * low * high + 100.0 * drift,
        )
        lines.append(",".join(str(int(round(c))) for c in counts))
    return ("\n".join(lines) + "\n").encode("ascii")


def parse_fault_spec(spec: str) -> dict:
    """Parse ``--faults TA:3:2,TB:1:5`` into {(type, board): {channel,...}}."""
    name_to_type = {v: k for k, v in TYPE_NAMES.items()}
    faults: dict = {}
    for item in (s.strip() for s in spec.split(",") if s.strip()):
        try:
            type_name, board_s, channel_s = item.split(":")
            board_type = name_to_type[type_name.upper()]
            board_id = int(board_s)
            channel = int(channel_s)
            if not (1 <= board_id <= BOARDS_PER_TYPE and 1 <= channel <= NUM_CHANNELS):
                raise ValueError(item)
        except (ValueError, KeyError):
            raise argparse.ArgumentTypeError(
                f"bad fault spec: '{item}' (want TYPE:board:channel, board 1-6, channel 1-8)"
            )
        faults.setdefault((board_type, board_id), set()).add(channel)
    return faults


def parse_args(argv) -> argparse.Namespace:
    """Parse command-line options."""
    p = argparse.ArgumentParser(
        description="Feed the big_db_test project with simulated boards over UDP."
    )
    p.add_argument(
        "--host", default="127.0.0.1", help="destination host (default: 127.0.0.1)"
    )
    p.add_argument(
        "--port", type=int, default=8080, help="destination UDP port (default: 8080)"
    )
    p.add_argument(
        "--rate",
        type=float,
        default=20.0,
        help="per-board cycle rate in Hz (default: 20)",
    )
    p.add_argument(
        "--cycles", type=int, default=0, help="stop after N cycles (0 = run forever)"
    )
    p.add_argument(
        "--boards",
        type=int,
        default=BOARDS_PER_TYPE,
        help="boards per type, 1..6 (default: 6)",
    )
    p.add_argument(
        "--faults",
        type=parse_fault_spec,
        default={},
        help="inject faults, e.g. 'TB:1:3,TA:2:5' (TYPE:board:channel)",
    )
    p.add_argument(
        "--vib-port",
        type=int,
        default=0,
        help="UDP port of the vibration stream (default: 0 = stream off)",
    )
    p.add_argument(
        "--vib-rate",
        type=float,
        default=1000.0,
        help="vibration samples per second (default: 1000)",
    )
    p.add_argument(
        "--print",
        dest="dump",
        action="store_true",
        help="print frames as hex, send nothing",
    )
    args = p.parse_args(argv)
    if not 1 <= args.boards <= BOARDS_PER_TYPE:
        p.error("--boards must be between 1 and 6")
    if args.rate <= 0:
        p.error("--rate must be positive")
    if args.vib_rate <= 0:
        p.error("--vib-rate must be positive")
    return args


def main(argv=None) -> int:
    """Run the simulator until the cycle budget is spent or interrupted."""
    args = parse_args(sys.argv[1:] if argv is None else argv)

    sock = None
    if not args.dump:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = (args.host, args.port)
    vib_dest = (args.host, args.vib_port)
    vib_sent = 0

    running = {"on": True}
    signal.signal(signal.SIGINT, lambda *_: running.__setitem__("on", False))

    period = 1.0 / args.rate
    start = time.monotonic()
    last_descriptor = -DESCRIPTOR_PERIOD_S  # force descriptors on the first cycle
    cycle = 0
    sent = 0

    while running["on"]:
        now = time.monotonic()
        t = now - start
        send_descriptors = (t - last_descriptor) >= DESCRIPTOR_PERIOD_S
        if send_descriptors:
            last_descriptor = t
        now_epoch = int(time.time())

        for board_type in (TA, TB, TC):
            for board_id in range(1, args.boards + 1):
                for frame in build_cycle_frames(
                    board_type,
                    board_id,
                    t,
                    args.faults,
                    send_descriptors,
                    cycle == 0,
                    now_epoch,
                ):
                    if args.dump:
                        fid = (frame[0] << 8) | frame[1]
                        print(f"0x{fid:03X} {frame.hex(' ')}")
                    else:
                        sock.sendto(frame, dest)
                    sent += 1

        if args.vib_port:
            vib_due = int(t * args.vib_rate) - vib_sent
            if vib_due > 0:
                chunk = vibration_lines(
                    vib_sent, min(vib_due, VIB_MAX_LINES), args.vib_rate
                )
                if args.dump:
                    print(chunk.decode("ascii"), end="")
                else:
                    sock.sendto(chunk, vib_dest)
                vib_sent += vib_due

        cycle += 1
        if args.cycles and cycle >= args.cycles:
            break

        target = start + cycle * period
        sleep = target - time.monotonic()
        if sleep > 0:
            time.sleep(sleep)

    if sock is not None:
        sock.close()
    where = "printed" if args.dump else f"sent to {args.host}:{args.port}"
    print(f"\n{cycle} cycles, {sent} frames {where}.", file=sys.stderr)
    if args.vib_port:
        print(f"{vib_sent} vibration samples on port {args.vib_port}.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
