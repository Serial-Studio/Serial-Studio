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

Reading payloads (big-endian uint16 values)::

    0  channels 1-4   per-type encoding (see channel_value)
    1  channels 5-8   same encoding
    2  diagnostics    one status byte per channel (0x00 = healthy)
    5  aux            [accepted_mask, stored_flag, epoch_be32]   (TB only)
    6  info           one ASCII descriptor byte per channel (TB/TC only)
    7  boot           [reset_cause, selftest]

The diagnostics frame is sent every cycle on purpose: the parser gates each
data value behind a known-healthy ``_diag`` entry, and continuously re-sending
all readings keeps the parser hot path under steady load for CI.

Usage
-----
    python3 big_db_test.py --host 127.0.0.1 --port 8080
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

# Diagnostic status byte values the parser accepts as "healthy enough".
DIAG_HEALTHY = 0x00
# A representative fault code used for fault injection.
DIAG_FAULT = 0xA8

# How often the low-rate descriptor frames (aux / info / boot) are re-sent, in
# seconds.
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


def u16_be(value: float) -> bytes:
    """Clamp to a healthy uint16 (< 0xFFFD) and return it big-endian."""
    v = max(0, min(0xFFFC, int(round(value))))
    return bytes(((v >> 8) & 0xFF, v & 0xFF))


def channel_value(board_type: int, board_id: int, channel: int, t: float) -> float:
    """Return a plausible, slowly varying raw reading for one channel.

    Each board/channel gets a distinct phase so the dashboard shows motion and
    no two tiles are identical. The per-type scaling just keeps the three board
    types in different numeric ranges; the exact factors are arbitrary.
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
    now_epoch: int,
) -> list:
    """Build every frame this board emits for one cycle, in send order.

    Diagnostics is emitted first so the parser's value gate is open before the
    data frames in the same cycle land.
    """
    frames = []

    diag = bytearray(DIAG_HEALTHY for _ in range(NUM_CHANNELS))
    for channel in faults.get((board_type, board_id), ()):  # 1-based channels
        diag[channel - 1] = DIAG_FAULT
    frames.append(make_frame(board_type, board_id, READING_DIAG, bytes(diag)))

    lo = b"".join(
        u16_be(channel_value(board_type, board_id, ch, t)) for ch in range(1, 5)
    )
    hi = b"".join(
        u16_be(channel_value(board_type, board_id, ch, t)) for ch in range(5, 9)
    )
    frames.append(make_frame(board_type, board_id, READING_DATA_LO, lo))
    frames.append(make_frame(board_type, board_id, READING_DATA_HI, hi))

    if send_descriptors:
        # Reading 6 (channel info): one ASCII descriptor byte per channel. Type
        # TA carries no descriptors; TB and TC report a per-type descriptor code.
        if board_type != TA:
            code = ord("A") if board_type == TB else ord("K")
            info = bytes(code for _ in range(NUM_CHANNELS))
            frames.append(make_frame(board_type, board_id, READING_INFO, info))

        boot = bytes((0x10, 0x00))  # reset cause = software reset, self-test = pass
        frames.append(make_frame(board_type, board_id, READING_BOOT, boot))

        if board_type == TB:
            aux = bytes((0xFF, 0x01)) + now_epoch.to_bytes(4, "big")
            frames.append(make_frame(board_type, board_id, READING_AUX, aux))

    return frames


def parse_fault_spec(spec: str) -> dict:
    """Parse ``--faults TA:3:2,TB:1:5`` into {(type, board): {channel,...}}."""
    name_to_type = {v: k for k, v in TYPE_NAMES.items()}
    faults: dict = {}
    for item in (s.strip() for s in spec.split(",") if s.strip()):
        try:
            type_name, board_s, channel_s = item.split(":")
            board_type = name_to_type[type_name.upper()]
            key = (board_type, int(board_s))
        except (ValueError, KeyError):
            raise argparse.ArgumentTypeError(
                f"bad fault spec: '{item}' (want TYPE:board:channel)"
            )
        faults.setdefault(key, set()).add(int(channel_s))
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
    return args


def main(argv=None) -> int:
    """Run the simulator until the cycle budget is spent or interrupted."""
    args = parse_args(sys.argv[1:] if argv is None else argv)

    sock = None
    if not args.dump:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = (args.host, args.port)

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
                    board_type, board_id, t, args.faults, send_descriptors, now_epoch
                ):
                    if args.dump:
                        fid = (frame[0] << 8) | frame[1]
                        print(f"0x{fid:03X} {frame.hex(' ')}")
                    else:
                        sock.sendto(frame, dest)
                    sent += 1

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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
