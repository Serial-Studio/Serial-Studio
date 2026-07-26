"""
Serial Studio Remote Dashboard Mirror Client

Reference implementation of the spec-0040 mirror wire contract, written
against doc/claude/specs/0040-remote-dashboard/wire-protocol.md. It is both
the codec (encode/decode/layout hash) and a client that drives the contract
end to end.

Two sources are supported so the codec is testable without an app:

    MirrorClient.from_file("tests/fixtures/mirror/small.ndjson")   # recorded
    MirrorClient(host="192.168.1.20", port=7777, token="abcd...")  # live

The module is importable standalone (no relative imports, no third-party
dependencies) so tests can add tests/utils to sys.path and import it without
pulling in the heavier tests.utils package.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

from __future__ import annotations

import base64
import json
import math
import socket
import time
import uuid
from dataclasses import dataclass
from typing import Any, Iterable, Optional, Sequence

# ----------------------------------------------------------------------
# Protocol constants (mirror of MirrorProtocol.h once T7 lands)
# ----------------------------------------------------------------------

MIRROR_WIRE_VERSION = 1

PUSH_KEY = "mirror"

KIND_STRUCTURE = "structure"
KIND_STRUCTURE_CHUNK = "structureChunk"
KIND_SNAPSHOT = "snapshot"
KIND_HEARTBEAT = "heartbeat"

STRUCTURE_CHUNK_BYTES = 512 * 1024
MAX_STRUCTURE_PARTS = 64

HZ_MIN = 1
HZ_MAX = 60
HZ_DEFAULT = 20

HEARTBEAT_INTERVAL_S = 1.0

WATCHDOG_MIN_S = 0.5
WATCHDOG_MAX_S = 3.0
WATCHDOG_TICKS = 3

RECONNECT_DELAYS_S = (1.0, 2.0, 4.0, 8.0, 16.0, 30.0)

CMD_GET_INFO = "mirror.getInfo"
CMD_GET_STRUCTURE = "mirror.getStructure"
CMD_SUBSCRIBE = "mirror.subscribe"
CMD_SET_RATE = "mirror.setRate"
CMD_UNSUBSCRIBE = "mirror.unsubscribe"

ERR_VERSION_MISMATCH = "MIRROR_VERSION_MISMATCH"
ERR_RATE_OUT_OF_RANGE = "MIRROR_RATE_OUT_OF_RANGE"
ERR_NOT_SUBSCRIBED = "MIRROR_NOT_SUBSCRIBED"
ERR_EPOCH_MISMATCH = "MIRROR_EPOCH_MISMATCH"
ERR_VIEWER_LIMIT = "MIRROR_VIEWER_LIMIT"
ERR_STRUCTURE_TOO_LARGE = "MIRROR_STRUCTURE_TOO_LARGE"

NON_FINITE_NAN = "nan"
NON_FINITE_POS_INF = "inf"
NON_FINITE_NEG_INF = "-inf"

_FNV64_OFFSET = 0xCBF29CE484222325
_FNV64_PRIME = 0x100000001B3
_FNV64_MASK = 0xFFFFFFFFFFFFFFFF

LAYOUT_HASH_SEED = b"mirror-v1\n"


# ----------------------------------------------------------------------
# Errors
# ----------------------------------------------------------------------


class MirrorError(Exception):
    """Base class for every mirror-client failure."""


class MirrorProtocolError(MirrorError):
    """Raised when a peer violates the wire contract."""


class MirrorVersionMismatch(MirrorProtocolError):
    """Raised when the peer speaks a wire version this client cannot decode."""

    def __init__(self, theirs: Any, ours: int = MIRROR_WIRE_VERSION):
        self.theirs = theirs
        self.ours = ours
        super().__init__(f"peer wire version {theirs!r}, this client speaks {ours}")


class MirrorCommandError(MirrorError):
    """Raised when the server rejects a mirror command."""

    def __init__(self, code: str, message: str):
        self.code = code
        self.message = message
        super().__init__(f"{code}: {message}")


# ----------------------------------------------------------------------
# Codec
# ----------------------------------------------------------------------


def layout_hash(dataset_ids: Sequence[Sequence[int]]) -> str:
    """
    FNV-1a 64 over the ordered (sourceId, uniqueId) list.

    Normative input encoding: the seed bytes, then "<sourceId>:<uniqueId>;"
    in UTF-8 for every entry, in list order. Returned as 16 lowercase hex
    digits. Any reordering, insertion, or removal changes the hash.
    """
    h = _FNV64_OFFSET
    for byte in LAYOUT_HASH_SEED:
        h = ((h ^ byte) * _FNV64_PRIME) & _FNV64_MASK

    for pair in dataset_ids:
        chunk = f"{int(pair[0])}:{int(pair[1])};".encode("utf-8")
        for byte in chunk:
            h = ((h ^ byte) * _FNV64_PRIME) & _FNV64_MASK

    return f"{h:016x}"


def _round_significant(value: float, digits: int) -> float:
    """Round to N significant digits; 0 and non-finite values pass through."""
    if digits <= 0 or value == 0.0 or not math.isfinite(value):
        return value

    exponent = math.floor(math.log10(abs(value)))
    return round(value, digits - 1 - exponent)


def encode_structure(
    *,
    epoch: int,
    dataset_ids: Sequence[Sequence[int]],
    project: dict,
    source_ids: Sequence[int],
    operation_mode: int = 0,
    plot_time_range: float = 10.0,
    frozen: bool = False,
    origin_unix_ms: Optional[int] = None,
) -> dict:
    """Builds a `structure` push object for the given epoch."""
    payload = {
        "kind": KIND_STRUCTURE,
        "wireVersion": MIRROR_WIRE_VERSION,
        "epoch": int(epoch),
        "layoutHash": layout_hash(dataset_ids),
        "sourceIds": [int(s) for s in source_ids],
        "datasets": [[int(p[0]), int(p[1])] for p in dataset_ids],
        "operationMode": int(operation_mode),
        "plotTimeRange": float(plot_time_range),
        "frozen": bool(frozen),
        "clock": {
            "domain": "monotonic-relative",
            "originUnixMs": int(
                origin_unix_ms if origin_unix_ms is not None else time.time() * 1000
            ),
        },
        "project": project,
    }
    return {PUSH_KEY: payload}


def encode_snapshot(
    *,
    epoch: int,
    seq: int,
    t_ns: Sequence[int],
    values: Sequence[Any],
    precision: int = 0,
) -> dict:
    """
    Builds a `snapshot` push object.

    `values` accepts floats, ints, strings, and None. Strings move into the
    sparse `strings` map, non-finite floats into the sparse `nonFinite` map,
    and both leave `null` in the positional array so its length always equals
    the structure's dataset count.
    """
    out: list[Any] = []
    strings: dict[str, str] = {}
    non_finite: dict[str, str] = {}

    for index, value in enumerate(values):
        if value is None:
            out.append(None)
        elif isinstance(value, str):
            strings[str(index)] = value
            out.append(None)
        elif isinstance(value, bool):
            out.append(1 if value else 0)
        elif isinstance(value, float) and not math.isfinite(value):
            if math.isnan(value):
                non_finite[str(index)] = NON_FINITE_NAN
            else:
                non_finite[str(index)] = (
                    NON_FINITE_POS_INF if value > 0 else NON_FINITE_NEG_INF
                )
            out.append(None)
        else:
            out.append(_round_significant(float(value), precision))

    payload: dict[str, Any] = {
        "kind": KIND_SNAPSHOT,
        "epoch": int(epoch),
        "seq": int(seq),
        "tNs": [int(t) for t in t_ns],
        "values": out,
    }

    if strings:
        payload["strings"] = strings
    if non_finite:
        payload["nonFinite"] = non_finite

    return {PUSH_KEY: payload}


def encode_heartbeat(*, epoch: int, seq: int) -> dict:
    """Builds a `heartbeat` push, emitted when no snapshot was produced."""
    return {PUSH_KEY: {"kind": KIND_HEARTBEAT, "epoch": int(epoch), "seq": int(seq)}}


def encode_structure_chunks(
    structure_push: dict, chunk_bytes: int = STRUCTURE_CHUNK_BYTES
) -> list[dict]:
    """
    Splits a structure push into `structureChunk` pushes.

    The payload is base64-encoded before slicing so every boundary is safe no
    matter where a multi-byte UTF-8 sequence falls; the 33% inflation is paid
    once per epoch on a human-paced event.
    """
    payload = structure_push[PUSH_KEY]
    blob = base64.b64encode(
        json.dumps(payload, separators=(",", ":")).encode("utf-8")
    ).decode("ascii")

    slices = [blob[i : i + chunk_bytes] for i in range(0, len(blob), chunk_bytes)] or [
        ""
    ]
    if len(slices) > MAX_STRUCTURE_PARTS:
        raise MirrorProtocolError(
            f"structure needs {len(slices)} parts, cap is {MAX_STRUCTURE_PARTS}"
        )

    return [
        {
            PUSH_KEY: {
                "kind": KIND_STRUCTURE_CHUNK,
                "epoch": int(payload["epoch"]),
                "part": index,
                "parts": len(slices),
                "data": text,
            }
        }
        for index, text in enumerate(slices)
    ]


def encode_line(push: dict) -> bytes:
    """Serializes a push object to one NDJSON line, exactly as the server does."""
    return json.dumps(push, separators=(",", ":")).encode("utf-8") + b"\n"


@dataclass
class MirrorStructure:
    """Decoded `structure` message."""

    epoch: int
    wire_version: int
    layout_hash: str
    dataset_ids: list[tuple[int, int]]
    source_ids: list[int]
    operation_mode: int
    plot_time_range: float
    frozen: bool
    origin_unix_ms: int
    project: dict

    @property
    def dataset_count(self) -> int:
        return len(self.dataset_ids)

    def index_of(self, source_id: int, unique_id: int) -> int:
        """Positional index of a dataset, or -1 when it is not in this epoch."""
        try:
            return self.dataset_ids.index((int(source_id), int(unique_id)))
        except ValueError:
            return -1


@dataclass
class MirrorSnapshot:
    """Decoded `snapshot` message with `values` already resolved."""

    epoch: int
    seq: int
    t_ns: list[int]
    values: list[Any]

    @property
    def value_count(self) -> int:
        return len(self.values)


@dataclass
class MirrorHeartbeat:
    """Decoded `heartbeat` message."""

    epoch: int
    seq: int


@dataclass
class MirrorStructureChunk:
    """One slice of a chunked structure payload."""

    epoch: int
    part: int
    parts: int
    data: str


def decode_push(obj: dict) -> Optional[object]:
    """
    Decodes one server push.

    Returns a MirrorStructure, MirrorSnapshot, or MirrorHeartbeat; returns
    None for anything this client must ignore (a push without the `mirror`
    key, or a `mirror` push whose `kind` is unknown). Unknown *fields* are
    ignored everywhere. Raises MirrorVersionMismatch when a structure
    announces an incompatible wire version, because a half-decoded structure
    is the failure this rule exists to prevent.
    """
    if not isinstance(obj, dict) or PUSH_KEY not in obj:
        return None

    payload = obj.get(PUSH_KEY)
    if not isinstance(payload, dict):
        raise MirrorProtocolError("mirror push payload is not an object")

    kind = payload.get("kind")

    if kind == KIND_STRUCTURE:
        version = payload.get("wireVersion")
        if version != MIRROR_WIRE_VERSION:
            raise MirrorVersionMismatch(version)

        pairs = [(int(p[0]), int(p[1])) for p in payload.get("datasets", [])]
        clock = payload.get("clock") or {}
        return MirrorStructure(
            epoch=int(payload.get("epoch", 0)),
            wire_version=int(version),
            layout_hash=str(payload.get("layoutHash", "")),
            dataset_ids=pairs,
            source_ids=[int(s) for s in payload.get("sourceIds", [])],
            operation_mode=int(payload.get("operationMode", 0)),
            plot_time_range=float(payload.get("plotTimeRange", 0.0)),
            frozen=bool(payload.get("frozen", False)),
            origin_unix_ms=int(clock.get("originUnixMs", 0)),
            project=payload.get("project") or {},
        )

    if kind == KIND_SNAPSHOT:
        raw = payload.get("values")
        if not isinstance(raw, list):
            raise MirrorProtocolError("snapshot `values` is not an array")

        resolved: list[Any] = list(raw)
        for key, text in (payload.get("strings") or {}).items():
            resolved[int(key)] = text
        for key, tag in (payload.get("nonFinite") or {}).items():
            resolved[int(key)] = _decode_non_finite(tag)

        return MirrorSnapshot(
            epoch=int(payload.get("epoch", 0)),
            seq=int(payload.get("seq", 0)),
            t_ns=[int(t) for t in payload.get("tNs", [])],
            values=resolved,
        )

    if kind == KIND_HEARTBEAT:
        return MirrorHeartbeat(
            epoch=int(payload.get("epoch", 0)), seq=int(payload.get("seq", 0))
        )

    if kind == KIND_STRUCTURE_CHUNK:
        return MirrorStructureChunk(
            epoch=int(payload.get("epoch", 0)),
            part=int(payload.get("part", 0)),
            parts=int(payload.get("parts", 1)),
            data=str(payload.get("data", "")),
        )

    return None


def _decode_non_finite(tag: str) -> float:
    if tag == NON_FINITE_NAN:
        return float("nan")
    if tag == NON_FINITE_POS_INF:
        return float("inf")
    if tag == NON_FINITE_NEG_INF:
        return float("-inf")
    raise MirrorProtocolError(f"unknown nonFinite tag {tag!r}")


# ----------------------------------------------------------------------
# Statistics
# ----------------------------------------------------------------------


@dataclass
class MirrorStats:
    """Counters a test or a bandwidth run asserts on."""

    lines: int = 0
    bytes_in: int = 0
    structures: int = 0
    snapshots: int = 0
    heartbeats: int = 0
    ignored_pushes: int = 0
    dropped_epoch: int = 0
    dropped_hash: int = 0
    dropped_length: int = 0
    structure_requests: int = 0
    seq_gaps: int = 0
    reconnects: int = 0
    structure_chunks: int = 0
    chunk_resets: int = 0


# ----------------------------------------------------------------------
# Line sources
# ----------------------------------------------------------------------


class FileMirrorSource:
    """Replays a recorded NDJSON push stream; no socket, no app."""

    def __init__(self, path: str):
        self.path = path
        self._lines: list[bytes] = []
        self._cursor = 0

    def open(self) -> None:
        with open(self.path, "rb") as handle:
            self._lines = [ln for ln in handle.read().split(b"\n") if ln.strip()]
        self._cursor = 0

    def close(self) -> None:
        self._lines = []
        self._cursor = 0

    def read_line(self, timeout: float = 0.0) -> Optional[bytes]:
        if self._cursor >= len(self._lines):
            return None

        line = self._lines[self._cursor]
        self._cursor += 1
        return line

    def send_line(self, data: bytes) -> None:
        raise MirrorError("a file source cannot send requests")


class SocketMirrorSource:
    """NDJSON framing over TCP, matching tests/utils/api_client.py exactly."""

    def __init__(self, host: str, port: int, timeout: float):
        self.host = host
        self.port = port
        self.timeout = timeout
        self._socket: Optional[socket.socket] = None
        self._buffer = b""

    def open(self) -> None:
        if self._socket:
            return

        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.settimeout(self.timeout)
        try:
            self._socket.connect((self.host, self.port))
        except (ConnectionRefusedError, socket.timeout, OSError) as exc:
            self._socket = None
            raise MirrorError(
                f"could not connect to {self.host}:{self.port} "
                f"(is the API server enabled and reachable?)"
            ) from exc

    def close(self) -> None:
        if self._socket:
            try:
                self._socket.close()
            except OSError:
                pass
        self._socket = None
        self._buffer = b""

    def send_line(self, data: bytes) -> None:
        if not self._socket:
            raise MirrorError("not connected")
        self._socket.sendall(data)

    def read_line(self, timeout: float = 1.0) -> Optional[bytes]:
        if not self._socket:
            raise MirrorError("not connected")

        deadline = time.monotonic() + timeout
        while True:
            newline = self._buffer.find(b"\n")
            if newline != -1:
                line = self._buffer[:newline]
                self._buffer = self._buffer[newline + 1 :]
                if line.strip():
                    return line
                continue

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return None

            self._socket.settimeout(min(remaining, 0.1))
            try:
                chunk = self._socket.recv(65536)
            except socket.timeout:
                continue
            if not chunk:
                raise MirrorError("connection closed by server")
            self._buffer += chunk


# ----------------------------------------------------------------------
# Client
# ----------------------------------------------------------------------


class MirrorClient:
    """
    Drives the spec-0040 mirror contract.

    Live use follows the normative order from wire-protocol.md: connect,
    subscribe with `frames:false` as the very first request (so the per-frame
    firehose is off before it can flood the socket), then fetch the structure.
    The auth handshake is sent lazily, only after the server answers
    "Authentication required" -- a loopback peer is pre-authenticated by the
    server and would reject an unsolicited auth line.
    """

    DEFAULT_PORT = 7777
    TIMEOUT = 5.0

    def __init__(
        self,
        host: str = "127.0.0.1",
        port: int = DEFAULT_PORT,
        token: str = "",
        hz: int = HZ_DEFAULT,
        timeout: float = TIMEOUT,
        source: Any = None,
    ):
        self.host = host
        self.port = port
        self.token = token
        self.hz = int(hz)
        self.timeout = timeout

        self.stats = MirrorStats()
        self.structure: Optional[MirrorStructure] = None
        self.values: list[Any] = []
        self.info: dict = {}
        self.last_error: str = ""

        self._source = source or SocketMirrorSource(host, port, timeout)
        self._authenticated = False
        self._subscribed = False
        self._last_seq: Optional[int] = None
        self._structure_pending = False
        self._last_push_at: Optional[float] = None
        self._last_snapshot_at: Optional[float] = None
        self._pending: list[dict] = []
        self._chunk_epoch: Optional[int] = None
        self._chunks: dict[int, str] = {}
        self._chunk_total = 0

    # -- construction ---------------------------------------------------

    @classmethod
    def from_file(cls, path: str, hz: int = HZ_DEFAULT) -> "MirrorClient":
        """Builds a client that reads a recorded NDJSON stream instead of a socket."""
        client = cls(hz=hz, source=FileMirrorSource(path))
        client._source.open()
        return client

    def __enter__(self) -> "MirrorClient":
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        self.close()

    # -- lifecycle ------------------------------------------------------

    def connect(self) -> None:
        """Opens the transport without sending anything."""
        self._source.open()

    def close(self) -> None:
        """Closes the transport; the remote capture is unaffected."""
        self._source.close()
        self._authenticated = False
        self._subscribed = False

    def attach(self, hz: Optional[int] = None) -> dict:
        """Runs the full normative sequence: connect, subscribe, get structure."""
        self.connect()
        info = self.subscribe(hz=hz)
        self.get_structure()
        return info

    def reattach(self) -> dict:
        """Reconnects with exponential backoff and re-fetches the structure."""
        for delay in RECONNECT_DELAYS_S:
            self.close()
            time.sleep(delay)
            try:
                info = self.attach()
            except MirrorError as exc:
                self.last_error = str(exc)
                continue

            self.stats.reconnects += 1
            return info

        raise MirrorError(f"reattach exhausted backoff: {self.last_error}")

    # -- staleness ------------------------------------------------------

    @property
    def watchdog_s(self) -> float:
        """Staleness bound: 3 mirror ticks, clamped to [500 ms, 3 s]."""
        return min(
            WATCHDOG_MAX_S, max(WATCHDOG_MIN_S, WATCHDOG_TICKS / float(max(1, self.hz)))
        )

    @property
    def stale(self) -> bool:
        """True when nothing at all has arrived within the watchdog: link suspect."""
        if self._last_push_at is None:
            return True
        return (time.monotonic() - self._last_push_at) > self.watchdog_s

    @property
    def live(self) -> bool:
        """True when snapshots are arriving: the remote capture is producing data."""
        if self._last_snapshot_at is None:
            return False
        return (time.monotonic() - self._last_snapshot_at) <= self.watchdog_s

    # -- requests -------------------------------------------------------

    def subscribe(
        self, hz: Optional[int] = None, frames: bool = False, precision: int = 0
    ) -> dict:
        """Subscribes to the mirror. MUST be the first request on a live socket."""
        if hz is not None:
            self.hz = int(hz)

        result = self._request(
            CMD_SUBSCRIBE,
            {
                "wireVersion": MIRROR_WIRE_VERSION,
                "hz": self.hz,
                "frames": frames,
                "precision": int(precision),
            },
        )

        peer_version = result.get("wireVersion")
        if peer_version != MIRROR_WIRE_VERSION:
            raise MirrorVersionMismatch(peer_version)

        self.info = result
        self._subscribed = True
        if "effectiveHz" in result:
            self.hz = int(result["effectiveHz"])

        return result

    def set_rate(self, hz: int) -> dict:
        """Renegotiates the mirror cadence on an existing subscription."""
        result = self._request(CMD_SET_RATE, {"hz": int(hz)})
        self.hz = int(result.get("effectiveHz", hz))
        return result

    def unsubscribe(self) -> dict:
        """Stops the mirror. Does not re-enable the per-frame stream."""
        result = self._request(CMD_UNSUBSCRIBE, {})
        self._subscribed = False
        return result

    def get_info(self) -> dict:
        """Stateless pre-flight probe. Never required by the attach sequence."""
        return self._request(CMD_GET_INFO, {})

    def get_structure(self) -> Optional[MirrorStructure]:
        """
        Fetches the current structure and adopts it.

        On a file source this only records the request; the recorded stream
        carries the structure push that satisfies it.
        """
        self.stats.structure_requests += 1

        if isinstance(self._source, FileMirrorSource):
            self._structure_pending = True
            return self.structure

        result = self._request(CMD_GET_STRUCTURE, {})
        structure = decode_push({PUSH_KEY: result})
        if not isinstance(structure, MirrorStructure):
            raise MirrorProtocolError("mirror.getStructure did not return a structure")

        self._adopt(structure)
        return structure

    def _request(self, command: str, params: dict) -> dict:
        """Sends one command, retrying once behind the lazy auth handshake."""
        for attempt in (0, 1):
            response = self._round_trip(command, params)

            if response.get("success"):
                return response.get("result", {})

            error = response.get("error", {})
            message = error.get("message", "")
            if attempt == 0 and not self._authenticated and "uthentication" in message:
                self._send_auth()
                continue

            self.last_error = message
            raise MirrorCommandError(error.get("code", "UNKNOWN"), message)

        raise MirrorCommandError("UNKNOWN", "request failed")

    def _round_trip(self, command: str, params: dict) -> dict:
        request_id = str(uuid.uuid4())
        message: dict[str, Any] = {
            "type": "command",
            "id": request_id,
            "command": command,
        }
        if params:
            message["params"] = params

        self._source.send_line(
            json.dumps(message, separators=(",", ":")).encode("utf-8") + b"\n"
        )

        deadline = time.monotonic() + self.timeout
        while time.monotonic() < deadline:
            line = self._source.read_line(deadline - time.monotonic())
            if line is None:
                break

            obj = json.loads(line.decode("utf-8"))
            if obj.get("type") == "response" and obj.get("id") == request_id:
                return obj

            self._stash(obj)

        raise MirrorError(f"timeout waiting for {command} response")

    def _send_auth(self) -> None:
        """Sends the token handshake line the server asks for on a remote hop."""
        if not self.token:
            raise MirrorCommandError(
                "UNAUTHORIZED", "server requires a token and none was supplied"
            )

        self._source.send_line(
            json.dumps(
                {"type": "auth", "token": self.token}, separators=(",", ":")
            ).encode("utf-8")
            + b"\n"
        )

        line = self._source.read_line(self.timeout)
        if line is None:
            raise MirrorError("timeout waiting for auth response")

        obj = json.loads(line.decode("utf-8"))
        if not obj.get("success"):
            raise MirrorCommandError("UNAUTHORIZED", "authentication refused")

        self._authenticated = True

    def _stash(self, obj: dict) -> None:
        """Holds a push that arrived while a response was outstanding."""
        self._pending.append(obj)

    # -- push pump ------------------------------------------------------

    def pump(self, max_events: int = 0, timeout: float = 1.0) -> list[object]:
        """
        Reads and applies pushes, returning the decoded ones.

        Applies the epoch and layout-hash rules: a snapshot whose epoch is not
        the held one, or whose length disagrees with the held structure, is
        dropped and a structure re-fetch is requested (at most one in flight).
        """
        events: list[object] = []

        while True:
            if self._pending:
                obj: Optional[dict] = self._pending.pop(0)
                line_len = len(json.dumps(obj, separators=(",", ":")).encode("utf-8"))
            else:
                line = self._source.read_line(timeout)
                if line is None:
                    break
                line_len = len(line)
                obj = json.loads(line.decode("utf-8"))

            self.stats.lines += 1
            self.stats.bytes_in += line_len + 1

            decoded = decode_push(obj)
            if decoded is None:
                self.stats.ignored_pushes += 1
            else:
                self._last_push_at = time.monotonic()
                if self._apply(decoded):
                    events.append(decoded)

            if max_events and len(events) >= max_events:
                break

        return events

    def drain(self, timeout: float = 1.0) -> list[object]:
        """Pumps until the source is exhausted (file) or the timeout expires."""
        return self.pump(max_events=0, timeout=timeout)

    def _apply(self, decoded: object) -> bool:
        if isinstance(decoded, MirrorStructure):
            self.stats.structures += 1
            self._adopt(decoded)
            return True

        if isinstance(decoded, MirrorHeartbeat):
            self.stats.heartbeats += 1
            return True

        if isinstance(decoded, MirrorStructureChunk):
            return self._accumulate_chunk(decoded)

        assert isinstance(decoded, MirrorSnapshot)
        self.stats.snapshots += 1

        if self.structure is None or decoded.epoch != self.structure.epoch:
            self.stats.dropped_epoch += 1
            self._request_structure()
            return False

        if decoded.value_count != self.structure.dataset_count:
            self.stats.dropped_length += 1
            self._request_structure()
            return False

        if self._last_seq is not None and decoded.seq != self._last_seq + 1:
            self.stats.seq_gaps += 1

        self._last_seq = decoded.seq
        self.values = decoded.values
        self._last_snapshot_at = time.monotonic()
        return True

    def _accumulate_chunk(self, chunk: MirrorStructureChunk) -> bool:
        """Collects structure parts; adopts the structure once the set is complete."""
        self.stats.structure_chunks += 1

        if chunk.parts > MAX_STRUCTURE_PARTS:
            raise MirrorProtocolError(
                f"structure announced {chunk.parts} parts, cap is {MAX_STRUCTURE_PARTS}"
            )

        if self._chunk_epoch != chunk.epoch:
            if self._chunk_epoch is not None:
                self.stats.chunk_resets += 1
            self._chunk_epoch = chunk.epoch
            self._chunks = {}
            self._chunk_total = chunk.parts

        self._chunks[chunk.part] = chunk.data
        if len(self._chunks) < self._chunk_total:
            return False

        blob = "".join(self._chunks[i] for i in range(self._chunk_total))
        payload = json.loads(base64.b64decode(blob).decode("utf-8"))
        self._chunk_epoch = None
        self._chunks = {}

        structure = decode_push({PUSH_KEY: payload})
        if not isinstance(structure, MirrorStructure):
            raise MirrorProtocolError("reassembled chunks are not a structure")

        self.stats.structures += 1
        self._adopt(structure)
        return True

    def _adopt(self, structure: MirrorStructure) -> None:
        """Adopts a structure only after its layout hash verifies."""
        expected = layout_hash(structure.dataset_ids)
        if structure.layout_hash != expected:
            self.stats.dropped_hash += 1
            raise MirrorProtocolError(
                f"layout hash mismatch: announced {structure.layout_hash}, "
                f"computed {expected}"
            )

        self.structure = structure
        self.values = [None] * structure.dataset_count
        self._last_seq = None
        self._structure_pending = False

    def _request_structure(self) -> None:
        if self._structure_pending:
            return

        self._structure_pending = True
        if isinstance(self._source, FileMirrorSource):
            self.stats.structure_requests += 1
            return

        try:
            self.get_structure()
        except MirrorError as exc:
            self.last_error = str(exc)

    # -- value access ---------------------------------------------------

    def value(self, source_id: int, unique_id: int) -> Any:
        """Current value of one dataset, or None when it is not in this epoch."""
        if self.structure is None:
            return None

        index = self.structure.index_of(source_id, unique_id)
        if index < 0 or index >= len(self.values):
            return None

        return self.values[index]

    def numeric_values(self) -> list[float]:
        """Every finite numeric value in the current snapshot."""
        return [
            v
            for v in self.values
            if isinstance(v, (int, float))
            and not isinstance(v, bool)
            and math.isfinite(v)
        ]

    def summary(self) -> str:
        """One-line description used by the fixture drive-through."""
        count = 0 if self.structure is None else self.structure.dataset_count
        numeric = self.numeric_values()
        span = (
            f"{min(numeric):.4g} .. {max(numeric):.4g}"
            if numeric
            else "no numeric data"
        )
        return (
            f"epoch={0 if self.structure is None else self.structure.epoch} "
            f"datasets={count} snapshots={self.stats.snapshots} "
            f"dropped={self.stats.dropped_epoch + self.stats.dropped_hash} "
            f"values[{span}]"
        )


# ----------------------------------------------------------------------
# Fixture drive-through
# ----------------------------------------------------------------------


def replay_fixture(path: str) -> MirrorClient:
    """Drives a recorded fixture end to end and returns the settled client."""
    client = MirrorClient.from_file(path)
    client.drain()
    return client


def _main(argv: Iterable[str]) -> int:
    paths = list(argv)
    if not paths:
        print("usage: mirror_client.py <fixture.ndjson> [...]")
        return 2

    for path in paths:
        client = replay_fixture(path)
        print(f"{path}: {client.summary()}")

    return 0


if __name__ == "__main__":
    import sys

    raise SystemExit(_main(sys.argv[1:]))
