#!/usr/bin/env python3
"""
IEC 60870-5-104 controlled station (substation) simulator for Serial Studio.

Implements the monitor direction of the protocol as a real RTU serves it:

  APCI      68 <len> <4 control octets> [ASDU]        I, S and U format frames
  STARTDT   act in, con out                           data transfer is opened by the client
  TESTFR    act in, con out; act out when idle        keep-alive in both directions
  C_IC_NA_1 act in, ActCon, the point set, ActTerm    station interrogation (QOI 20)
  M_ME_NC_1 short float measurands, COT 20 then 3     three analogue points
  M_SP_NA_1 single points, COT 20 then 3              two status points

Sequence-number accounting is real: every I-frame carries the send count and the
piggybacked receive count, the station acknowledges with S-frames once w frames are
outstanding, and it stops sending once k frames go unacknowledged.

The control direction (commands, set-points, file transfer) is not implemented: Serial
Studio's client only monitors, and the only frame it ever writes is the interrogation.

Usage:
    python3 iec104_server.py
    python3 iec104_server.py --port 2404 --ca 1
    python3 iec104_server.py --interval 0.5

Requirements: python3 standard library only.
"""

import argparse
import math
import random
import socket
import struct
import sys
import time

# ------------------------------------------------------------------------------------
# APCI vocabulary
# ------------------------------------------------------------------------------------

START_BYTE = 0x68
APCI_BYTES = 6
MIN_APDU_LENGTH = 4
MAX_APDU_LENGTH = 253
SEQUENCE_MODULO = 32768

U_STARTDT_ACT = 0x07
U_STARTDT_CON = 0x0B
U_STOPDT_ACT = 0x13
U_STOPDT_CON = 0x23
U_TESTFR_ACT = 0x43
U_TESTFR_CON = 0x83

# ------------------------------------------------------------------------------------
# ASDU vocabulary
# ------------------------------------------------------------------------------------

TYPE_SINGLE_POINT = 1
TYPE_MEASURED_FLOAT = 13
TYPE_INTERROGATION = 100

CAUSE_SPONTANEOUS = 3
CAUSE_ACTIVATION = 6
CAUSE_ACT_CONFIRM = 7
CAUSE_ACT_TERMINATE = 10
CAUSE_INTERROGATED = 20

QOI_STATION = 20
ASDU_HEADER_BYTES = 6
IOA_BYTES = 3

QUALITY_GOOD = 0x00

# ------------------------------------------------------------------------------------
# Point table
# ------------------------------------------------------------------------------------

# Information object addresses, in the order the interrogation reply reports them. That
# order is what Serial Studio assigns its wire slots from, so it is also the dataset
# order of a generated project.
IOA_BUS_VOLTAGE = 1001
IOA_LINE_CURRENT = 1002
IOA_FREQUENCY = 1003
IOA_BREAKER_CLOSED = 2001
IOA_FAULT_ACTIVE = 2002

MEASURANDS = (IOA_BUS_VOLTAGE, IOA_LINE_CURRENT, IOA_FREQUENCY)
STATUS_POINTS = (IOA_BREAKER_CLOSED, IOA_FAULT_ACTIVE)


class Station:
    """The simulated substation: three analogue readings and two status points."""

    def __init__(self):
        self.started = time.time()
        self.breaker_closed = True
        self.fault_active = False

    def measurands(self):
        """Returns the three short-float points, in interrogation order."""
        elapsed = time.time() - self.started
        voltage = 11000.0 + 180.0 * math.sin(elapsed / 23.0) + random.uniform(-8.0, 8.0)
        current = 0.0
        if self.breaker_closed:
            current = (
                145.0 + 25.0 * math.sin(elapsed / 11.0) + random.uniform(-1.5, 1.5)
            )

        frequency = 50.0 + 0.04 * math.sin(elapsed / 7.0) + random.uniform(-0.01, 0.01)
        return (
            (IOA_BUS_VOLTAGE, voltage),
            (IOA_LINE_CURRENT, current),
            (IOA_FREQUENCY, frequency),
        )

    def status_points(self):
        """Returns the two single points, in interrogation order."""
        return (
            (IOA_BREAKER_CLOSED, self.breaker_closed),
            (IOA_FAULT_ACTIVE, self.fault_active),
        )

    def step(self):
        """Advances the discrete state; returns True when a status point changed."""
        if random.random() >= 0.03:
            return False

        if self.fault_active:
            self.fault_active = False
            self.breaker_closed = True
        else:
            self.fault_active = True
            self.breaker_closed = False

        return True


# ------------------------------------------------------------------------------------
# Encoders
# ------------------------------------------------------------------------------------


def write_sequence(value):
    """Encodes one 15-bit sequence number into its two control octets."""
    return bytes([(value << 1) & 0xFE, (value >> 7) & 0xFF])


def read_sequence(apdu, pos):
    """Decodes the 15-bit sequence number held in the control octets at pos."""
    return ((apdu[pos + 1] << 7) | (apdu[pos] >> 1)) & 0x7FFF


def u_frame(function):
    """Builds a U-format APDU carrying one control function."""
    return bytes([START_BYTE, MIN_APDU_LENGTH, function, 0x00, 0x00, 0x00])


def s_frame(recv_seq):
    """Builds an S-format APDU acknowledging everything received so far."""
    return bytes([START_BYTE, MIN_APDU_LENGTH, 0x01, 0x00]) + write_sequence(recv_seq)


def i_frame(send_seq, recv_seq, asdu):
    """Builds an I-format APDU around one ASDU."""
    header = bytes([START_BYTE, MIN_APDU_LENGTH + len(asdu)])
    return header + write_sequence(send_seq) + write_sequence(recv_seq) + asdu


def asdu_header(type_id, count, cause, common_address, sequence=False):
    """Builds the six-octet ASDU header; the common address is two octets, little endian."""
    vsq = (0x80 if sequence else 0x00) | (count & 0x7F)
    return struct.pack("<BBBBH", type_id, vsq, cause, 0, common_address)


def encode_ioa(ioa):
    """Encodes a three-octet information object address, little endian."""
    return bytes([ioa & 0xFF, (ioa >> 8) & 0xFF, (ioa >> 16) & 0xFF])


def measurand_asdu(points, cause, common_address):
    """Builds an M_ME_NC_1 ASDU: address, IEEE-754 short float, quality descriptor."""
    body = bytearray()
    for ioa, value in points:
        body += encode_ioa(ioa)
        body += struct.pack("<f", value)
        body.append(QUALITY_GOOD)

    return asdu_header(TYPE_MEASURED_FLOAT, len(points), cause, common_address) + bytes(
        body
    )


def single_point_asdu(points, cause, common_address):
    """Builds an M_SP_NA_1 ASDU: address, then the SIQ whose low bit carries the state."""
    body = bytearray()
    for ioa, state in points:
        body += encode_ioa(ioa)
        body.append(0x01 if state else 0x00)

    return asdu_header(TYPE_SINGLE_POINT, len(points), cause, common_address) + bytes(
        body
    )


def interrogation_asdu(cause, common_address):
    """Builds a C_IC_NA_1 confirmation or termination: one object at address zero."""
    return (
        asdu_header(TYPE_INTERROGATION, 1, cause, common_address)
        + encode_ioa(0)
        + bytes([QOI_STATION])
    )


# ------------------------------------------------------------------------------------
# Session
# ------------------------------------------------------------------------------------


class Session:
    """One controlling-station connection: sequence state, the interrogation, then updates."""

    def __init__(self, connection, address, args, station):
        self.connection = connection
        self.address = address
        self.args = args
        self.station = station

        self.send_seq = 0
        self.recv_seq = 0
        self.acked_seq = 0
        self.unacked_rx = 0

        self.started = False
        self.interrogated = False
        self.buffer = bytearray()
        self.last_update = 0.0
        self.first_unacked_rx = None
        self.last_traffic = time.monotonic()
        self.test_pending = False

    # ------------------------------------------------------------------------------
    # Framing
    # ------------------------------------------------------------------------------

    def outstanding(self):
        """Returns how many sent I-frames the controlling station has not acknowledged."""
        return (self.send_seq - self.acked_seq + SEQUENCE_MODULO) % SEQUENCE_MODULO

    def send(self, apdu):
        """Writes one encoded APDU and refreshes the idle clock."""
        self.connection.sendall(apdu)
        self.last_traffic = time.monotonic()

    def send_asdu(self, asdu):
        """Sends one ASDU as an I-frame, unless the k window is closed."""
        if self.outstanding() >= self.args.k:
            return False

        self.send(i_frame(self.send_seq, self.recv_seq, asdu))
        self.send_seq = (self.send_seq + 1) % SEQUENCE_MODULO

        # An I-frame carries the receive count, so it clears the acknowledgement debt
        # the same way an S-frame would.
        self.unacked_rx = 0
        self.first_unacked_rx = None
        return True

    def take_apdu(self):
        """Removes one complete APDU from the receive buffer, or returns None."""
        if len(self.buffer) < 2:
            return None

        if self.buffer[0] != START_BYTE:
            raise OSError("bad APCI start byte")

        length = self.buffer[1]
        if length < MIN_APDU_LENGTH or length > MAX_APDU_LENGTH:
            raise OSError("bad APCI length")

        total = length + 2
        if len(self.buffer) < total:
            return None

        apdu = bytes(self.buffer[:total])
        del self.buffer[:total]
        return apdu

    # ------------------------------------------------------------------------------
    # Inbound handling
    # ------------------------------------------------------------------------------

    def dispatch(self, apdu):
        """Routes one decoded APDU by its control field."""
        self.last_traffic = time.monotonic()
        control = apdu[2]

        if not control & 0x01:
            self.handle_information(apdu)
            return

        if control & 0x03 == 0x01:
            self.acked_seq = read_sequence(apdu, 4)
            return

        self.handle_unnumbered(control)

    def handle_unnumbered(self, function):
        """Answers the U-format functions a controlled station owes an answer to."""
        if function == U_STARTDT_ACT:
            print("STARTDT act -> con")
            self.send(u_frame(U_STARTDT_CON))
            self.started = True
            return

        if function == U_STOPDT_ACT:
            print("STOPDT act -> con")
            self.send(u_frame(U_STOPDT_CON))
            self.started = False
            self.interrogated = False
            return

        if function == U_TESTFR_ACT:
            self.send(u_frame(U_TESTFR_CON))
            return

        if function == U_TESTFR_CON:
            self.test_pending = False

    def handle_information(self, apdu):
        """Accepts one I-frame, advances the receive count and acts on the ASDU it carries."""
        send_seq = read_sequence(apdu, 2)
        if send_seq != self.recv_seq:
            raise OSError(
                "out-of-order I-frame (expected %d, got %d)" % (self.recv_seq, send_seq)
            )

        self.acked_seq = read_sequence(apdu, 4)
        self.recv_seq = (self.recv_seq + 1) % SEQUENCE_MODULO
        if self.unacked_rx == 0:
            self.first_unacked_rx = time.monotonic()

        self.unacked_rx += 1

        asdu = apdu[APCI_BYTES:]
        if len(asdu) < ASDU_HEADER_BYTES:
            return

        type_id = asdu[0]
        cause = asdu[2] & 0x3F
        common_address = struct.unpack_from("<H", asdu, 4)[0]
        if common_address != self.args.ca:
            print("ignoring ASDU for common address %d" % common_address)
            return

        if type_id == TYPE_INTERROGATION and cause == CAUSE_ACTIVATION:
            self.run_interrogation()

    # ------------------------------------------------------------------------------
    # Outbound flows
    # ------------------------------------------------------------------------------

    def run_interrogation(self):
        """Answers a station interrogation: confirmation, the whole point set, termination."""
        print("C_IC_NA_1 station interrogation")
        self.send_asdu(interrogation_asdu(CAUSE_ACT_CONFIRM, self.args.ca))
        self.send_asdu(
            measurand_asdu(self.station.measurands(), CAUSE_INTERROGATED, self.args.ca)
        )
        self.send_asdu(
            single_point_asdu(
                self.station.status_points(), CAUSE_INTERROGATED, self.args.ca
            )
        )
        self.send_asdu(interrogation_asdu(CAUSE_ACT_TERMINATE, self.args.ca))

        # Spontaneous traffic waits for the interrogation to finish, so the point set a
        # client discovers is the interrogation reply and not whatever happened to change
        # between STARTDT and the request.
        self.interrogated = True
        self.last_update = time.monotonic()

    def send_updates(self):
        """Publishes the spontaneous traffic a station sends between interrogations."""
        changed = self.station.step()
        self.send_asdu(
            measurand_asdu(self.station.measurands(), CAUSE_SPONTANEOUS, self.args.ca)
        )
        if changed:
            self.send_asdu(
                single_point_asdu(
                    self.station.status_points(), CAUSE_SPONTANEOUS, self.args.ca
                )
            )

    def service_timers(self):
        """Honours the w/t2 acknowledgement obligation and the t3 idle test."""
        now = time.monotonic()

        overdue = (
            self.first_unacked_rx is not None
            and now - self.first_unacked_rx >= self.args.t2 / 1000.0
        )
        if self.unacked_rx >= self.args.w or (self.unacked_rx > 0 and overdue):
            self.send(s_frame(self.recv_seq))
            self.unacked_rx = 0
            self.first_unacked_rx = None

        if not self.test_pending and now - self.last_traffic >= self.args.t3 / 1000.0:
            self.send(u_frame(U_TESTFR_ACT))
            self.test_pending = True

        if self.interrogated and now - self.last_update >= self.args.interval:
            self.last_update = now
            self.send_updates()

    # ------------------------------------------------------------------------------
    # Loop
    # ------------------------------------------------------------------------------

    def run(self):
        """Serves the connection until the peer disconnects or breaks the protocol."""
        print("connection from %s:%d" % self.address[:2])
        self.connection.settimeout(0.1)
        try:
            while True:
                try:
                    chunk = self.connection.recv(4096)
                    if not chunk:
                        break

                    self.buffer += chunk
                    while True:
                        apdu = self.take_apdu()
                        if apdu is None:
                            break

                        self.dispatch(apdu)

                except socket.timeout:
                    pass

                self.service_timers()

        except (ConnectionError, OSError) as error:
            print("connection closed: %s" % error)

        finally:
            self.connection.close()
            print("connection from %s:%d ended" % self.address[:2])


# ------------------------------------------------------------------------------------
# Listener
# ------------------------------------------------------------------------------------


def serve(args):
    """Accepts one controlling station at a time, as a strict 104 station does."""
    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.bind, args.port))
    listener.listen(1)
    print(
        "IEC 60870-5-104 station listening on %s:%d (common address %d)"
        % (args.bind, args.port, args.ca)
    )

    station = Station()
    try:
        while True:
            connection, address = listener.accept()
            connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            Session(connection, address, args, station).run()

    except KeyboardInterrupt:
        print("\nstopping")

    finally:
        listener.close()


def parse_arguments():
    """Parses the command line."""
    parser = argparse.ArgumentParser(description="IEC 60870-5-104 station simulator")
    parser.add_argument(
        "--bind", "--host", dest="bind", default="127.0.0.1", help="listen address"
    )
    parser.add_argument(
        "--port", type=int, default=2404, help="listen port (default 2404)"
    )
    parser.add_argument(
        "--ca", type=int, default=1, help="common address of ASDU (default 1)"
    )
    parser.add_argument("--k", type=int, default=12, help="send window k (default 12)")
    parser.add_argument("--w", type=int, default=8, help="ack window w (default 8)")
    parser.add_argument(
        "--t2", type=int, default=10000, help="ack timeout in ms (default 10000)"
    )
    parser.add_argument("--t3", type=int, default=20000, help="idle test timeout in ms")
    parser.add_argument(
        "--interval",
        type=float,
        default=1.0,
        help="spontaneous update period in seconds",
    )
    return parser.parse_args()


def main():
    """Entry point."""
    args = parse_arguments()
    if args.w > args.k:
        print("The ack window w cannot exceed the send window k", file=sys.stderr)
        return 1

    serve(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
