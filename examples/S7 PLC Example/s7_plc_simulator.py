#!/usr/bin/env python3
"""
Siemens S7 (ISO-on-TCP / S7comm) PLC simulator for Serial Studio.

Implements just enough of the protocol stack a real S7-300/400/1200/1500 CPU speaks for
Serial Studio's S7 driver to open a session and poll variables:

  RFC 1006 TPKT framing            03 00 <length be16> <TPDU>
  ISO 8073 class-0 connect         CR (0xE0) in, CC (0xD0) out
  S7comm setup communication       job 0xF0 in, ack 0xF0 out with the PDU length
  S7comm read variable             job 0x04 in, ack 0x04 out with one result per item

The write service, block up/download, PLC control and the cyclic services are not
implemented: the driver never sends them.

The address space is one data block, DB1, holding four values that drift over time:

  DB1.DBD0   REAL   Tank_Level      slow sine, 0-100 %
  DB1.DBW4   INT    Motor_Speed     ramps toward 1450 or 0 RPM
  DB1.DBX6.0 BOOL   Pump_Running    trips occasionally
  DB1.DBB7   BYTE   Cycle_Counter   wraps every 256 cycles

Usage:
    sudo python3 s7_plc_simulator.py
    sudo python3 s7_plc_simulator.py --bind 0.0.0.0
    python3 s7_plc_simulator.py --port 1102          # unprivileged, for protocol tests

Port 102 is privileged on macOS and Linux, so the simulator needs sudo there. Serial
Studio's S7 driver always dials port 102 (the ISO-TSAP port is fixed by the protocol),
so --port is only useful when driving the simulator from another script.

Requirements: python3 standard library only.
"""

import argparse
import math
import random
import socket
import socketserver
import struct
import sys
import threading
import time

# ------------------------------------------------------------------------------------
# ISO-on-TCP / COTP vocabulary (RFC 1006, ISO 8073 class 0)
# ------------------------------------------------------------------------------------

TPKT_VERSION = 0x03
TPKT_HEADER_BYTES = 4

TPDU_CONNECT_REQUEST = 0xE0
TPDU_CONNECT_CONFIRM = 0xD0
TPDU_DATA = 0xF0
END_OF_TRANSMISSION = 0x80

PARAM_TPDU_SIZE = 0xC0
PARAM_CALLING_TSAP = 0xC1
PARAM_CALLED_TSAP = 0xC2

TPDU_SIZE_CODE = 0x0A  # 1 << 10 = 1024 bytes
CONNECT_TPDU_BYTES = 18
DATA_TPDU_BYTES = 3
CONNECT_HEADER_BYTES = 7

# ------------------------------------------------------------------------------------
# S7comm vocabulary
# ------------------------------------------------------------------------------------

PROTOCOL_ID = 0x32
ROSCTR_JOB = 0x01
ROSCTR_ACK_DATA = 0x03

FUNCTION_SETUP = 0xF0
FUNCTION_READ_VAR = 0x04

SPECIFICATION_ANY = 0x12
ANY_LENGTH_BYTES = 0x0A
SYNTAX_ANY = 0x10
TRANSPORT_BIT = 0x01
TRANSPORT_BYTE = 0x02

RESULT_BIT = 0x03
RESULT_BYTES = 0x04
RETURN_SUCCESS = 0xFF
RETURN_OUT_OF_RANGE = 0x05
RETURN_NO_OBJECT = 0x0A

JOB_HEADER_BYTES = 10
ACK_HEADER_BYTES = 12
REQUEST_ITEM_BYTES = 12

AREA_INPUT = 0x81
AREA_OUTPUT = 0x82
AREA_MEMORY = 0x83
AREA_DATA_BLOCK = 0x84

# The length the simulator settles the setup negotiation on; the client clamps whatever
# it is told to 240..8192 and plans its read chunks against it.
NEGOTIATED_PDU_BYTES = 480
MAX_AMQ = 1

DB_NUMBER = 1
DB_SIZE_BYTES = 8


# ------------------------------------------------------------------------------------
# Simulated data block
# ------------------------------------------------------------------------------------


class DataBlock:
    """DB1 as a flat byte buffer, updated by a background thread and read under a lock."""

    def __init__(self):
        self.lock = threading.Lock()
        self.bytes = bytearray(DB_SIZE_BYTES)
        self.started = time.time()
        self.speed = 0
        self.running = False
        self.cycle = 0

    def step(self):
        """Advances the process one tick and re-encodes the block, big endian throughout."""
        elapsed = time.time() - self.started
        level = 50.0 + 35.0 * math.sin(elapsed / 12.0) + random.uniform(-0.4, 0.4)

        if random.random() < 0.01:
            self.running = not self.running

        target = 1450 if self.running else 0
        self.speed += int((target - self.speed) * 0.2)
        self.cycle = (self.cycle + 1) % 256

        with self.lock:
            struct.pack_into(">f", self.bytes, 0, level)
            struct.pack_into(">h", self.bytes, 4, self.speed)
            self.bytes[6] = 0x01 if self.running else 0x00
            self.bytes[7] = self.cycle

    def read(self, offset, count):
        """Returns count bytes at the given offset, or None when the range leaves the block."""
        if offset < 0 or count <= 0 or offset + count > DB_SIZE_BYTES:
            return None

        with self.lock:
            return bytes(self.bytes[offset : offset + count])

    def read_bit(self, offset, bit):
        """Returns a bit as a single octet whose low bit carries it, or None when out of range."""
        if offset < 0 or offset >= DB_SIZE_BYTES or bit < 0 or bit > 7:
            return None

        with self.lock:
            return bytes([(self.bytes[offset] >> bit) & 0x01])


DB1 = DataBlock()


def simulation_loop(period):
    """Runs the process simulation forever on its own thread."""
    while True:
        DB1.step()
        time.sleep(period)


# ------------------------------------------------------------------------------------
# Encoders
# ------------------------------------------------------------------------------------


def frame_tpkt(tpdu):
    """Prepends the four-octet TPKT header; the length field covers the whole packet."""
    return struct.pack(">BBH", TPKT_VERSION, 0, len(tpdu) + TPKT_HEADER_BYTES) + tpdu


def build_connect_confirm(request):
    """Builds the class-0 connect confirm answering a connect request."""
    source_reference = struct.unpack_from(">H", request, 4)[0]
    called_tsap = request[-2:] if len(request) >= 2 else b"\x03\x02"

    tpdu = bytearray()
    tpdu.append(CONNECT_TPDU_BYTES - 1)
    tpdu.append(TPDU_CONNECT_CONFIRM)
    tpdu += struct.pack(">H", source_reference)
    tpdu += struct.pack(">H", 0x0001)
    tpdu.append(0x00)
    tpdu += bytes([PARAM_TPDU_SIZE, 1, TPDU_SIZE_CODE])
    tpdu += bytes([PARAM_CALLING_TSAP, 2, 0x01, 0x00])
    tpdu += bytes([PARAM_CALLED_TSAP, 2]) + called_tsap
    return frame_tpkt(bytes(tpdu))


def wrap_data(payload):
    """Wraps one S7 protocol data unit in a data TPDU carrying the end-of-transmission flag."""
    tpdu = bytes([DATA_TPDU_BYTES - 1, TPDU_DATA, END_OF_TRANSMISSION]) + payload
    return frame_tpkt(tpdu)


def ack_header(reference, parameter_bytes, data_bytes):
    """Builds the twelve-octet acknowledgement header, error class and code both zero."""
    return struct.pack(
        ">BBHHHHBB",
        PROTOCOL_ID,
        ROSCTR_ACK_DATA,
        0,
        reference,
        parameter_bytes,
        data_bytes,
        0,
        0,
    )


def build_setup_response(reference):
    """Builds the setup-communication acknowledgement naming the negotiated PDU length."""
    parameters = struct.pack(
        ">BBHHH", FUNCTION_SETUP, 0, MAX_AMQ, MAX_AMQ, NEGOTIATED_PDU_BYTES
    )
    return ack_header(reference, len(parameters), 0) + parameters


# ------------------------------------------------------------------------------------
# Read service
# ------------------------------------------------------------------------------------


def serve_item(item):
    """Answers one request item as (return code, transport code, declared length, payload)."""
    if item[0] != SPECIFICATION_ANY or item[2] != SYNTAX_ANY:
        return RETURN_NO_OBJECT, 0, 0, b""

    transport = item[3]
    count = struct.unpack_from(">H", item, 4)[0]
    db_number = struct.unpack_from(">H", item, 6)[0]
    area = item[8]
    start = (item[9] << 16) | (item[10] << 8) | item[11]

    if area != AREA_DATA_BLOCK or db_number != DB_NUMBER:
        return RETURN_NO_OBJECT, 0, 0, b""

    offset, bit = start >> 3, start & 0x07
    if transport == TRANSPORT_BIT:
        payload = DB1.read_bit(offset, bit)
        if payload is None:
            return RETURN_OUT_OF_RANGE, 0, 0, b""

        # A bit answer declares its length in BITS; everything else declares bytes,
        # in bits, under the 0x04 transport code.
        return RETURN_SUCCESS, RESULT_BIT, 1, payload

    payload = DB1.read(offset, count)
    if payload is None:
        return RETURN_OUT_OF_RANGE, 0, 0, b""

    return RETURN_SUCCESS, RESULT_BYTES, len(payload) * 8, payload


def build_read_response(reference, items):
    """Builds the read acknowledgement: one result per requested item, in request order."""
    data = bytearray()
    for index, item in enumerate(items):
        code, transport, declared, payload = serve_item(item)
        data += struct.pack(">BBH", code, transport, declared)
        data += payload

        # An odd payload is padded with one fill octet when another item follows it.
        if index + 1 < len(items) and len(payload) % 2:
            data.append(0x00)

    parameters = bytes([FUNCTION_READ_VAR, len(items)])
    return ack_header(reference, len(parameters), len(data)) + parameters + bytes(data)


def split_items(parameters):
    """Splits the request parameters into the twelve-octet items they carry."""
    count = parameters[1]
    items = []
    for i in range(count):
        first = 2 + i * REQUEST_ITEM_BYTES
        if first + REQUEST_ITEM_BYTES > len(parameters):
            return None

        items.append(parameters[first : first + REQUEST_ITEM_BYTES])

    return items


# ------------------------------------------------------------------------------------
# Connection handling
# ------------------------------------------------------------------------------------


class S7Handler(socketserver.BaseRequestHandler):
    """One S7comm session: the ISO handshake, the setup negotiation, then read requests."""

    def handle(self):
        """Reads TPKTs until the peer disconnects or sends something this stack refuses."""
        self.request.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        print("session opened from %s:%d" % self.client_address[:2])

        buffer = bytearray()
        try:
            while True:
                chunk = self.request.recv(4096)
                if not chunk:
                    break

                buffer += chunk
                while True:
                    tpdu = self.take_tpdu(buffer)
                    if tpdu is None:
                        break

                    if not self.dispatch(tpdu):
                        return

        except (ConnectionError, OSError):
            pass

        finally:
            print("session closed from %s:%d" % self.client_address[:2])

    @staticmethod
    def take_tpdu(buffer):
        """Removes one complete TPKT from the buffer and returns the TPDU inside it."""
        if len(buffer) < TPKT_HEADER_BYTES:
            return None

        if buffer[0] != TPKT_VERSION or buffer[1] != 0:
            raise OSError("bad TPKT header")

        total = struct.unpack_from(">H", buffer, 2)[0]
        if total < TPKT_HEADER_BYTES + 2 or len(buffer) < total:
            return None

        tpdu = bytes(buffer[TPKT_HEADER_BYTES:total])
        del buffer[:total]
        return tpdu

    def dispatch(self, tpdu):
        """Routes one TPDU; returns False to close the session."""
        if len(tpdu) < 2:
            return False

        if tpdu[1] == TPDU_CONNECT_REQUEST:
            if len(tpdu) < CONNECT_HEADER_BYTES:
                return False

            rack, slot = divmod(tpdu[-1], 0x20)
            print("ISO connect request for rack %d, slot %d" % (rack, slot))
            self.request.sendall(build_connect_confirm(tpdu))
            return True

        if tpdu[1] != TPDU_DATA:
            return False

        header = tpdu[0] + 1
        return self.handle_pdu(tpdu[header:])

    def handle_pdu(self, pdu):
        """Routes one S7 protocol data unit; returns False to close the session."""
        if len(pdu) < JOB_HEADER_BYTES or pdu[0] != PROTOCOL_ID or pdu[1] != ROSCTR_JOB:
            return False

        reference = struct.unpack_from(">H", pdu, 4)[0]
        parameter_bytes = struct.unpack_from(">H", pdu, 6)[0]
        parameters = pdu[JOB_HEADER_BYTES : JOB_HEADER_BYTES + parameter_bytes]
        if len(parameters) < 1:
            return False

        if parameters[0] == FUNCTION_SETUP:
            print(
                "setup communication, answering with a %d byte PDU"
                % NEGOTIATED_PDU_BYTES
            )
            self.request.sendall(wrap_data(build_setup_response(reference)))
            return True

        if parameters[0] != FUNCTION_READ_VAR or len(parameters) < 2:
            return False

        items = split_items(parameters)
        if items is None:
            return False

        self.request.sendall(wrap_data(build_read_response(reference, items)))
        return True


class S7Server(socketserver.ThreadingTCPServer):
    """Threaded ISO-on-TCP listener; one thread per session, as a real CPU allows several."""

    allow_reuse_address = True
    daemon_threads = True


def parse_arguments():
    """Parses the command line."""
    parser = argparse.ArgumentParser(description="Siemens S7 PLC simulator")
    parser.add_argument(
        "--bind",
        "--host",
        dest="bind",
        default="127.0.0.1",
        help="listen address (default 127.0.0.1)",
    )
    parser.add_argument(
        "--port", type=int, default=102, help="listen port (default 102)"
    )
    parser.add_argument(
        "--rate", type=float, default=10.0, help="simulation rate in Hz"
    )
    return parser.parse_args()


def main():
    """Entry point."""
    args = parse_arguments()

    worker = threading.Thread(target=simulation_loop, args=(1.0 / max(1.0, args.rate),))
    worker.daemon = True
    worker.start()

    try:
        server = S7Server((args.bind, args.port), S7Handler)
    except PermissionError:
        print(
            "Port %d needs root: try sudo python3 s7_plc_simulator.py" % args.port,
            file=sys.stderr,
        )
        return 1

    print(
        "S7 simulator listening on %s:%d (DB%d, %d bytes)"
        % (args.bind, args.port, DB_NUMBER, DB_SIZE_BYTES)
    )
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nstopping")

    finally:
        server.shutdown()
        server.server_close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
