# Serial Studio device library
# https://serial-studio.com/
#
# Copyright (C) 2020-2026 Alex Spataru
#
# SPDX-License-Identifier: MIT

"""Declare a Serial Studio dashboard from a script and stream its frames.

The same ``channel()`` call that sends a value also declares it: when a
declaration is due (start-up, or the app's ``#ss?`` request) the pass emits the
declaration line instead of a frame, and every other pass emits a frame.
"""

import socket
import struct
import threading

from . import _vocabulary as _v
from ._vocabulary import __version__

__all__ = ["Device", "TcpServer", "tcp_server", "__version__"]

_CONTROL_KINDS = {"button": 0, "slider": 1, "toggle": 2, "text": 3, "knob": 4}
_CONTROL_ALIASES = {
    "min": "outputMin",
    "max": "outputMax",
    "step": "outputStep",
    "units": "outputUnits",
    "decimals": "outputDecimals",
    "quantize": "quantizeToStep",
}
_FIELD_TYPES = {
    "i8": "b", "u8": "B", "i16": "h", "u16": "H", "i32": "i", "u32": "I",
    "i64": "q", "u64": "Q", "f32": "f", "f64": "d",
}
_FRAME_DETECTION_START = 3
_DECODER_BINARY = 3
_REPLY_TITLES = ("Reply Token", "Reply Code", "Reply Message")
_REPLY_BYTES = 32
_FORMATS = {"none": 0, "fixed": 1, "bits": 2, "scale": 3}
_ESCAPE_NEXT = frozenset("0123456789+.-")
_ESCAPE_THIS = frozenset(" ,;:=")
_LINE_LIMIT = 256


def _camel(key):
    """snake_case to the wire's camelCase; a camelCase key passes through."""
    head, *rest = key.split("_")
    return head + "".join(part[:1].upper() + part[1:] for part in rest)


def _string(text):
    """A JSON string with the inertness rule: ``,;:=`` and space before a number char escape."""
    text = str(text)
    if len(text.encode("utf-8")) > _v.MAX_STRING_BYTES:
        raise ValueError("string longer than %d bytes: %r" % (_v.MAX_STRING_BYTES, text))

    out = ['"']
    for i, ch in enumerate(text):
        follower = text[i + 1] if i + 1 < len(text) else ""
        if ch == '"' or ch == "\\":
            out.append("\\" + ch)
        elif ord(ch) < 0x20 or (ch in _ESCAPE_THIS and follower in _ESCAPE_NEXT):
            out.append("\\u%04x" % ord(ch))
        else:
            out.append(ch)

    out.append('"')
    return "".join(out)


def _number(value):
    """A JSON number: integers as written, floats with up to six fraction digits."""
    if isinstance(value, bool):
        return "true" if value else "false"

    if isinstance(value, int):
        return str(value)

    if value != value or value in (float("inf"), float("-inf")):
        raise ValueError("non-finite number in a declaration")

    text = "%.6f" % value
    if "." in text:
        text = text.rstrip("0").rstrip(".")

    return "0" if text in ("-0", "") else text


def _scalar(value):
    if isinstance(value, bool) or isinstance(value, (int, float)):
        return _number(value)

    return _string(value)


def _object(entries, allowed):
    """A sub-entity object (alarm band, label, marker) with keys checked against the vocabulary."""
    parts = []
    for key, value in dict(entries).items():
        key = _camel(key)
        if key not in allowed:
            raise ValueError("%r is not a key of this entry (%s)" % (key, ", ".join(allowed)))

        parts.append(_string(key) + ":" + _scalar(value))

    return "{" + ",".join(parts) + "}"


def _property(key, value):
    """One ``"key":value`` member of a dataset, validated against the generated vocabulary."""
    if key in _v.SUB_ENTITY_KEYS:
        allowed = _v.SUB_ENTITY_KEYS[key]
        if key == "labels" and isinstance(value, dict):
            value = [{"value": k, "text": t} for k, t in value.items()]

        elif key == "labels":
            value = [{"value": k, "text": t} for k, t in value]

        value = list(value)
        if len(value) > _v.SUB_ENTITY_CAPS[key]:
            raise ValueError("%s holds at most %d entries" % (key, _v.SUB_ENTITY_CAPS[key]))

        return _string(key) + ":[" + ",".join(_object(item, allowed) for item in value) + "]"

    if key not in _v.DATASET_KEYS or key == "title":
        raise ValueError("%r is not a dataset property of the declaration" % key)

    if key == "format" and isinstance(value, str):
        value = _FORMATS[value]

    return _string(key) + ":" + _scalar(value)


def _crc16(data):
    """CRC-16 as the app's "CRC-16" checksum: init 0xFFFF, polynomial 0x1021, sent big-endian."""
    crc = 0xFFFF
    for byte in data:
        x = ((crc >> 8) ^ byte) & 0xFF
        x ^= x >> 4
        crc = ((crc << 8) ^ (x << 12) ^ (x << 5) ^ x) & 0xFFFF

    return crc


def _result(value):
    """A control callback's return value as (code, message)."""
    if value is None or value is True:
        return 0, ""

    if value is False:
        return 1, ""

    if isinstance(value, tuple):
        return int(value[0]), str(value[1])

    if isinstance(value, str):
        return 1, value

    return int(value), ""


def _frame_value(value, places):
    if isinstance(value, bool):
        return "1" if value else "0"

    if isinstance(value, int):
        return str(value)

    if isinstance(value, float):
        return "%.*f" % (places, value)

    return str(value)


class Device:
    """One declared device: group()/channel()/control() each pass, then send().

    ``write`` receives bytes. ``on_control(name, value)`` receives a parsed
    ``#ss!name=value`` command; ``on_line(text)`` receives every other input line.
    """

    def __init__(
        self, title, write=None, *, separator=",", named=False, on_control=None, on_line=None
    ):
        self.title = title
        self.write = write
        self.separator = separator
        self.named = named
        self.on_control = on_control
        self.on_line = on_line
        self.declare_due = True
        self._binary = None
        self._has_controls = False
        self._reply = None
        self._lock = threading.RLock()
        self._inbox = bytearray()
        self._lines = []
        self._begin_state()
        _string(title)

    def _begin_state(self):
        self._in_pass = False
        self._declaring = False
        self._groups = []
        self._parts = []
        self._datasets = 0
        self._controls = 0
        self._payload = bytearray()
        self._payload_fields = []

    def binary(self, fields, endianness="little", start=b"\xaa\x55", checksum="CRC-16"):
        """Send packed binary frames: ``fields`` is a CSV of i8..u64, f32, f64 in channel order."""
        types = [name.strip().lower() for name in fields.split(",")]
        if not types or any(name not in _FIELD_TYPES for name in types):
            raise ValueError("fields must be a CSV of %s" % ", ".join(_FIELD_TYPES))

        if endianness not in ("little", "big"):
            raise ValueError("endianness must be little or big")

        if checksum not in ("CRC-16", None) or not 1 <= len(start) <= 4:
            raise ValueError("binary frames take a 1 to 4 byte start and CRC-16 or no checksum")

        with self._lock:
            self._binary = {
                "types": types,
                "csv": ",".join(types),
                "format": ("<" if endianness == "little" else ">"),
                "endianness": endianness,
                "start": bytes(start),
                "checksum": checksum,
            }

    def _start_pass(self):
        if not self._in_pass:
            self._in_pass = True
            self._declaring = self.declare_due

    def group(self, title, widget=None):
        """Open a dataset group; its channels follow."""
        with self._lock:
            self._start_pass()
            if self._declaring:
                if len(self._groups) >= _v.MAX_GROUPS:
                    raise ValueError("at most %d groups" % _v.MAX_GROUPS)

                head = _string("title") + ":" + _string(title)
                if widget is not None:
                    head += "," + _string("widget") + ":" + _string(widget)

                self._groups.append({"head": head, "items": [], "controls": False})

    def control_group(self, title):
        """Open an Output Panel group; its controls follow."""
        with self._lock:
            self._start_pass()
            if self._declaring:
                if len(self._groups) >= _v.MAX_GROUPS:
                    raise ValueError("at most %d groups" % _v.MAX_GROUPS)

                head = _string("title") + ":" + _string(title)
                self._groups.append({"head": head, "items": [], "controls": True})

    def channel(self, name, value=None, *, places=2, **props):
        """Declare (when due) and send one channel; ``value=None`` declares without sending."""
        with self._lock:
            self._start_pass()
            if self._declaring:
                self._declare_channel(name, props)
            elif value is not None and self._binary is not None:
                self._pack(value)
            elif value is not None:
                text = _frame_value(value, places)
                self._parts.append(("%s:%s" % (name, text)) if self.named else text)

    def _pack(self, value):
        types = self._binary["types"]
        index = len(self._payload_fields)
        if index >= len(types):
            raise ValueError("more values than the %d declared binary fields" % len(types))

        code = _FIELD_TYPES[types[index]]
        number = float(value) if code in "fd" else int(value)
        self._payload += struct.pack(self._binary["format"] + code, number)
        self._payload_fields.append(code)

    def _declare_channel(self, name, props):
        if not self._groups or self._groups[-1]["controls"]:
            raise ValueError("channel() needs an open dataset group()")

        if self._datasets >= _v.MAX_DATASETS:
            raise ValueError("at most %d datasets" % _v.MAX_DATASETS)

        members = [_string("title") + ":" + _string(name)]
        graph = True
        for raw, value in props.items():
            key = _camel(raw)
            if key == "graph":
                graph = bool(value)
            elif key == "range":
                low, high = value
                for k, v in zip(("widgetMin", "widgetMax", "plotMin", "plotMax"), (low, high, low, high)):
                    members.append(_property(k, v))
            else:
                members.append(_property(key, value))

        if graph:
            members.append(_string("graph") + ":true")

        self._groups[-1]["items"].append("{" + ",".join(members) + "}")
        self._datasets += 1

    def control(self, title, kind, **props):
        """Declare one output control (button, slider, toggle or knob) in a control_group()."""
        with self._lock:
            self._start_pass()
            if not self._declaring:
                return

            if not self._groups or not self._groups[-1]["controls"]:
                raise ValueError("control() needs an open control_group()")

            if self._controls >= _v.MAX_CONTROLS:
                raise ValueError("at most %d controls" % _v.MAX_CONTROLS)

            if kind not in _CONTROL_KINDS:
                raise ValueError("control kind must be one of %s" % ", ".join(_CONTROL_KINDS))

            members = [
                _string("title") + ":" + _string(title),
                _string("outputType") + ":" + str(_CONTROL_KINDS[kind]),
            ]
            for raw, value in props.items():
                key = _CONTROL_ALIASES.get(raw, _camel(raw))
                if key not in _v.CONTROL_KEYS or key in ("title", "outputType"):
                    raise ValueError("%r is not a control property of the declaration" % raw)

                members.append(_string(key) + ":" + _scalar(value))

            self._groups[-1]["items"].append("{" + ",".join(members) + "}")
            self._controls += 1
            self._has_controls = True

    def send(self):
        """Close the pass: write the declaration if one is due, otherwise the frame."""
        with self._lock:
            line = self._declaration() if self._declaring else self._frame()
            if self._declaring:
                self.declare_due = False

            self._begin_state()
            if line and self.write is not None:
                self.write(line if isinstance(line, bytes) else line.encode("utf-8"))

    def _declaration(self):
        if not self._groups:
            raise ValueError("a declaration needs at least one group")

        groups = []
        for group in self._groups:
            key = "outputWidgets" if group["controls"] else "datasets"
            kind = "," + _string("groupType") + ":1" if group["controls"] else ""
            groups.append("{%s%s,%s:[%s]}" % (group["head"], kind, _string(key), ",".join(group["items"])))

        line = "%s%d{%s:%s%s,%s:[%s]}\n" % (
            _v.PREFIX,
            _v.WIRE_VERSION,
            _string("title"),
            _string(self.title),
            self._sources(),
            _string("groups"),
            ",".join(groups),
        )
        if len(line) > _v.MAX_LINE_BYTES:
            raise ValueError("declaration longer than %d bytes" % _v.MAX_LINE_BYTES)

        return line

    def _sources(self):
        binary = self._binary
        if binary is None:
            return ""

        members = [
            _string("frameStart") + ":" + _string(binary["start"].hex().upper()),
            _string("hexadecimalDelimiters") + ":true",
            _string("frameDetection") + ":" + str(_FRAME_DETECTION_START),
        ]
        if binary["checksum"]:
            members.append(_string("checksumAlgorithm") + ":" + _string(binary["checksum"]))

        params = "{%s:%s,%s:%s}" % (
            _string("fields"),
            _string(binary["csv"]),
            _string("endianness"),
            _string(binary["endianness"]),
        )
        members += [
            _string("decoderMethod") + ":" + str(_DECODER_BINARY),
            _string("frameParserTemplate") + ":" + _string(_v.BINARY_TEMPLATE),
            _string("frameParserParams") + ":" + params,
        ]
        return "," + _string("sources") + ":[{" + ",".join(members) + "}]"

    def _frame(self):
        if self._binary is not None:
            return self._binary_frame()

        parts = list(self._parts)
        if parts and self._has_controls and self._reply is not None:
            parts += self._reply_fields()

        return self.separator.join(parts) + "\n" if parts else ""

    def _binary_frame(self):
        if not self._payload_fields:
            return b""

        if len(self._payload_fields) != len(self._binary["types"]):
            raise ValueError("a binary frame needs all %d values" % len(self._binary["types"]))

        frame = self._binary["start"] + bytes(self._payload)
        if self._binary["checksum"]:
            frame += struct.pack(">H", _crc16(self._payload))

        return frame

    def _reply_fields(self):
        """Token, code and message of the last tokened command, appended to every frame."""
        token, code, message = self._reply
        values = (str(token), str(code), message)
        if not self.named:
            return list(values)

        return ["%s:%s" % (title, value) for title, value in zip(_REPLY_TITLES, values) if value]

    def feed(self, data):
        """Buffer received bytes (or text); complete lines wait for poll()."""
        if isinstance(data, str):
            data = data.encode("utf-8")

        with self._lock:
            self._inbox += data
            while True:
                end = self._inbox.find(b"\n")
                if end < 0:
                    break

                line = bytes(self._inbox[:end]).rstrip(b"\r")
                del self._inbox[: end + 1]
                self._lines.append(line.decode("utf-8", "replace"))

            if len(self._inbox) > _LINE_LIMIT:
                self._inbox.clear()

    def poll(self):
        """Dispatch the buffered lines: ``#ss?``, ``#ss!name=value[@token]``, anything else."""
        with self._lock:
            lines, self._lines = self._lines, []

        for line in lines:
            self._dispatch(line)

        return len(lines)

    def _dispatch(self, line):
        if line.startswith(_v.REQUEST):
            self.declare_due = True
        elif line.startswith(_v.COMMAND):
            name, sep, rest = line[len(_v.COMMAND) :].partition("=")
            if sep:
                self._command(name, *rest.partition("@")[::2])
        elif self.on_line is not None:
            self.on_line(line)

    def _command(self, name, value, token):
        result = None
        if self.on_control is not None:
            result = self.on_control(name, self._command_value(value))

        if not token.strip().lstrip("-").isdigit():
            return

        code, message = _result(result if self.on_control is not None else 1)
        for ch in (self.separator, ",", ";", "\n", "\r"):
            message = message.replace(ch, " ")

        with self._lock:
            self._reply = (int(token), code, message[:_REPLY_BYTES])

    @staticmethod
    def _command_value(text):
        try:
            number = float(text)
        except ValueError:
            return text

        return int(number) if number.is_integer() else number


class TcpServer:
    """Serve one device over TCP: the first client receives its bytes and feeds its input."""

    def __init__(self, device, host="127.0.0.1", port=0):
        self.device = device
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._socket.bind((host, port))
        self._socket.listen(1)
        self.port = self._socket.getsockname()[1]
        self._client = None
        self._running = True
        self._thread = threading.Thread(target=self._serve, daemon=True)
        self._thread.start()

    def _serve(self):
        while self._running:
            try:
                client, _ = self._socket.accept()
            except OSError:
                return

            self._client = client
            self.device.write = self._send
            self._read(client)

    def _send(self, data):
        client = self._client
        if client is not None:
            try:
                client.sendall(data)
            except OSError:
                self._client = None

    def _read(self, client):
        while self._running:
            try:
                data = client.recv(512)
            except OSError:
                break

            if not data:
                break

            self.device.feed(data)
            self.device.poll()

        self._client = None
        client.close()

    def close(self):
        """Stop serving and release the port.

        shutdown() comes first: on Linux, close() alone leaves a socket listening while another
        thread is blocked in its accept(), which keeps the port taken for the life of the process.
        macOS and Windows refuse shutdown() on a listening socket, so its error is ignored.
        """
        self._running = False
        for sock in (self._client, self._socket):
            if sock is None:
                continue

            try:
                sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass

            try:
                sock.close()
            except OSError:
                pass


def tcp_server(device, host="127.0.0.1", port=0):
    """Start a TcpServer for ``device``; its ``port`` attribute holds the bound port."""
    return TcpServer(device, host, port)
