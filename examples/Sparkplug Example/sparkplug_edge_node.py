#!/usr/bin/env python3
"""
Sparkplug B Edge Node simulator for Serial Studio.

Publishes a Sparkplug B v1.0 edge node against any MQTT broker:

  spBv1.0/<group>/NBIRTH/<node>   birth certificate, every metric named + aliased
  spBv1.0/<group>/NDATA/<node>    periodic updates, alias only (no names on the wire)
  spBv1.0/<group>/NDEATH/<node>   registered as the connection's last will
  spBv1.0/<group>/NCMD/<node>     subscribed; "Node Control/Rebirth" re-publishes NBIRTH

The Sparkplug B payload is Protocol Buffers, but this script carries its own minimal
encoder and decoder, so the only dependency is paho-mqtt. The field numbers below are
the ones Eclipse Tahu's sparkplug_b.proto assigns, which is what Serial Studio's
decoder reads.

Usage:
    python3 sparkplug_edge_node.py
    python3 sparkplug_edge_node.py --host 192.168.1.20 --group Plant1 --node Line3
    python3 sparkplug_edge_node.py --interval 0.2

Requirements:
    pip install paho-mqtt
"""

import argparse
import math
import random
import struct
import sys
import time

try:
    import paho.mqtt.client as mqtt
except ImportError:
    print("This example needs paho-mqtt: pip install paho-mqtt", file=sys.stderr)
    raise SystemExit(1)

# ------------------------------------------------------------------------------------
# Sparkplug B wire vocabulary (Eclipse Tahu sparkplug_b.proto)
# ------------------------------------------------------------------------------------

PAYLOAD_TIMESTAMP = 1
PAYLOAD_METRICS = 2
PAYLOAD_SEQ = 3

METRIC_NAME = 1
METRIC_ALIAS = 2
METRIC_TIMESTAMP = 3
METRIC_DATATYPE = 4
METRIC_IS_NULL = 7
METRIC_INT_VALUE = 10
METRIC_LONG_VALUE = 11
METRIC_FLOAT_VALUE = 12
METRIC_DOUBLE_VALUE = 13
METRIC_BOOLEAN_VALUE = 14
METRIC_STRING_VALUE = 15

WIRE_VARINT = 0
WIRE_FIXED64 = 1
WIRE_LENGTH_DELIMITED = 2
WIRE_FIXED32 = 5

# Datatype codes carried in Metric field 4
DT_INT8 = 1
DT_INT16 = 2
DT_INT32 = 3
DT_INT64 = 4
DT_UINT8 = 5
DT_UINT16 = 6
DT_UINT32 = 7
DT_UINT64 = 8
DT_FLOAT = 9
DT_DOUBLE = 10
DT_BOOLEAN = 11
DT_STRING = 12

SEQ_MODULUS = 256
NAMESPACE = "spBv1.0"
REBIRTH_METRIC = "Node Control/Rebirth"


# ------------------------------------------------------------------------------------
# Minimal protobuf encoder
# ------------------------------------------------------------------------------------


def encode_varint(value):
    """Encodes an unsigned integer as a base-128 varint."""
    out = bytearray()
    value &= 0xFFFFFFFFFFFFFFFF
    while True:
        byte = value & 0x7F
        value >>= 7
        if value:
            out.append(byte | 0x80)
        else:
            out.append(byte)
            return bytes(out)


def encode_tag(field, wire):
    """Encodes the field number and wire type into one protobuf tag."""
    return encode_varint((field << 3) | wire)


def varint_field(field, value):
    """Encodes one varint-typed field."""
    return encode_tag(field, WIRE_VARINT) + encode_varint(value)


def bytes_field(field, payload):
    """Encodes one length-delimited field."""
    return (
        encode_tag(field, WIRE_LENGTH_DELIMITED) + encode_varint(len(payload)) + payload
    )


def fixed32_field(field, raw):
    """Encodes one 32-bit fixed-width field, little endian."""
    return encode_tag(field, WIRE_FIXED32) + struct.pack("<I", raw & 0xFFFFFFFF)


def fixed64_field(field, raw):
    """Encodes one 64-bit fixed-width field, little endian."""
    return encode_tag(field, WIRE_FIXED64) + struct.pack("<Q", raw & 0xFFFFFFFFFFFFFFFF)


def encode_value(datatype, value):
    """Encodes the value member of the Metric oneof that the datatype selects."""
    if datatype == DT_BOOLEAN:
        return varint_field(METRIC_BOOLEAN_VALUE, 1 if value else 0)

    if datatype == DT_STRING:
        return bytes_field(METRIC_STRING_VALUE, str(value).encode("utf-8"))

    if datatype == DT_FLOAT:
        return fixed32_field(
            METRIC_FLOAT_VALUE, struct.unpack("<I", struct.pack("<f", value))[0]
        )

    if datatype == DT_DOUBLE:
        return fixed64_field(
            METRIC_DOUBLE_VALUE, struct.unpack("<Q", struct.pack("<d", value))[0]
        )

    # The signed integer codes travel as two's-complement varints truncated to their
    # declared width; the receiver restores the sign from the datatype.
    widths = {DT_INT8: 8, DT_INT16: 16, DT_INT32: 32, DT_INT64: 64}
    if datatype in widths:
        raw = int(value) & ((1 << widths[datatype]) - 1)
        field = METRIC_LONG_VALUE if datatype == DT_INT64 else METRIC_INT_VALUE
        return varint_field(field, raw)

    if datatype in (DT_UINT8, DT_UINT16, DT_UINT32):
        return varint_field(METRIC_INT_VALUE, int(value))

    if datatype == DT_UINT64:
        return varint_field(METRIC_LONG_VALUE, int(value))

    raise ValueError("unsupported Sparkplug datatype %d" % datatype)


def encode_metric(name, alias, datatype, value, timestamp_ms):
    """Encodes one Metric sub-message: identity, headers, then the value."""
    out = bytearray()
    if name is not None:
        out += bytes_field(METRIC_NAME, name.encode("utf-8"))

    if alias is not None:
        out += varint_field(METRIC_ALIAS, alias)

    if timestamp_ms:
        out += varint_field(METRIC_TIMESTAMP, timestamp_ms)

    out += varint_field(METRIC_DATATYPE, datatype)
    out += encode_value(datatype, value)
    return bytes(out)


def encode_payload(metrics, seq, timestamp_ms):
    """Encodes one Payload: timestamp, every metric, then the sequence number."""
    out = bytearray()
    out += varint_field(PAYLOAD_TIMESTAMP, timestamp_ms)
    for metric in metrics:
        out += bytes_field(PAYLOAD_METRICS, metric)

    if seq is not None:
        out += varint_field(PAYLOAD_SEQ, seq)

    return bytes(out)


# ------------------------------------------------------------------------------------
# Minimal protobuf decoder (NCMD only)
# ------------------------------------------------------------------------------------


def decode_varint(data, pos):
    """Reads one base-128 varint, returning the value and the new cursor."""
    result = 0
    shift = 0
    for _ in range(10):
        if pos >= len(data):
            raise ValueError("truncated varint")

        byte = data[pos]
        pos += 1
        result |= (byte & 0x7F) << shift
        if not byte & 0x80:
            return result, pos

        shift += 7

    raise ValueError("varint too long")


def skip_field(data, pos, wire):
    """Advances the cursor past a field this decoder does not model."""
    if wire == WIRE_VARINT:
        return decode_varint(data, pos)[1]

    if wire == WIRE_FIXED64:
        return pos + 8

    if wire == WIRE_FIXED32:
        return pos + 4

    if wire == WIRE_LENGTH_DELIMITED:
        length, pos = decode_varint(data, pos)
        return pos + length

    raise ValueError("unsupported wire type %d" % wire)


def decode_metric_names(payload):
    """Returns the names of every metric a payload carries whose boolean value is true."""
    names = []
    pos = 0
    while pos < len(payload):
        tag, pos = decode_varint(payload, pos)
        field, wire = tag >> 3, tag & 0x07
        if field != PAYLOAD_METRICS or wire != WIRE_LENGTH_DELIMITED:
            pos = skip_field(payload, pos, wire)
            continue

        length, pos = decode_varint(payload, pos)
        block = payload[pos : pos + length]
        pos += length

        name = None
        asserted = False
        inner = 0
        while inner < len(block):
            tag, inner = decode_varint(block, inner)
            field, wire = tag >> 3, tag & 0x07
            if field == METRIC_NAME and wire == WIRE_LENGTH_DELIMITED:
                size, inner = decode_varint(block, inner)
                name = block[inner : inner + size].decode("utf-8", "replace")
                inner += size
            elif field == METRIC_BOOLEAN_VALUE and wire == WIRE_VARINT:
                value, inner = decode_varint(block, inner)
                asserted = value != 0
            else:
                inner = skip_field(block, inner, wire)

        if name is not None and asserted:
            names.append(name)

    return names


# ------------------------------------------------------------------------------------
# Simulated process
# ------------------------------------------------------------------------------------


class Process:
    """A small drifting process: two analogue readings and a motor that trips now and then."""

    def __init__(self):
        self.started = time.time()
        self.rpm = 1480
        self.running = True

    def sample(self):
        """Advances the simulation and returns the current metric values."""
        elapsed = time.time() - self.started
        temperature = 62.0 + 8.0 * math.sin(elapsed / 17.0) + random.uniform(-0.3, 0.3)
        pressure = 4.25 + 0.45 * math.sin(elapsed / 9.5) + random.uniform(-0.02, 0.02)

        if random.random() < 0.004:
            self.running = not self.running

        target = 1480 if self.running else 0
        self.rpm += int((target - self.rpm) * 0.25)
        return temperature, pressure, self.running, self.rpm


# ------------------------------------------------------------------------------------
# Edge node
# ------------------------------------------------------------------------------------


class EdgeNode:
    """One Sparkplug B edge node: births, periodic data, a death will, and rebirth handling."""

    # Metric name, alias, Sparkplug datatype. The aliases are what NDATA carries instead
    # of the names, so they must match the ones the birth certificate declared.
    METRICS = (
        ("Temperature", 2, DT_FLOAT),
        ("Pressure", 3, DT_DOUBLE),
        ("Motor/Running", 4, DT_BOOLEAN),
        ("Motor/RPM", 5, DT_INT32),
    )

    def __init__(self, args):
        self.args = args
        self.process = Process()
        self.seq = 0
        self.bd_seq = 0
        self.base = "%s/%s" % (NAMESPACE, args.group)
        self.client = self.make_client(args.client_id)
        if args.username:
            self.client.username_pw_set(args.username, args.password)

        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

    @staticmethod
    def make_client(client_id):
        """Builds a paho client; 2.x wants the callback API version, 1.x has no such argument."""
        if hasattr(mqtt, "CallbackAPIVersion"):
            return mqtt.Client(
                mqtt.CallbackAPIVersion.VERSION1,
                client_id=client_id,
                clean_session=True,
            )

        return mqtt.Client(client_id=client_id, clean_session=True)

    def topic(self, verb):
        """Builds the node-scoped topic for one Sparkplug verb."""
        return "%s/%s/%s" % (self.base, verb, self.args.node)

    def next_seq(self):
        """Returns the next per-node sequence number; it wraps at 256."""
        value = self.seq
        self.seq = (self.seq + 1) % SEQ_MODULUS
        return value

    def publish_birth(self):
        """Publishes NBIRTH: every metric with BOTH its name and its alias, plus seq 0."""
        now = int(time.time() * 1000)
        temperature, pressure, running, rpm = self.process.sample()
        values = {
            "Temperature": temperature,
            "Pressure": pressure,
            "Motor/Running": running,
            "Motor/RPM": rpm,
        }

        self.seq = 0
        metrics = [
            encode_metric("bdSeq", None, DT_INT64, self.bd_seq, now),
            encode_metric(REBIRTH_METRIC, 1, DT_BOOLEAN, False, now),
        ]
        for name, alias, datatype in self.METRICS:
            metrics.append(encode_metric(name, alias, datatype, values[name], now))

        payload = encode_payload(metrics, self.next_seq(), now)
        self.client.publish(self.topic("NBIRTH"), payload, qos=0, retain=False)
        print("NBIRTH -> %s (%d metrics)" % (self.topic("NBIRTH"), len(metrics)))

    def publish_data(self):
        """Publishes NDATA carrying aliases only, which is what a real edge node sends."""
        now = int(time.time() * 1000)
        temperature, pressure, running, rpm = self.process.sample()
        values = {
            "Temperature": temperature,
            "Pressure": pressure,
            "Motor/Running": running,
            "Motor/RPM": rpm,
        }

        metrics = [
            encode_metric(None, alias, datatype, values[name], now)
            for name, alias, datatype in self.METRICS
        ]
        payload = encode_payload(metrics, self.next_seq(), now)
        self.client.publish(self.topic("NDATA"), payload, qos=0, retain=False)

    def death_payload(self):
        """Builds the NDEATH certificate the broker publishes when this node drops off."""
        now = int(time.time() * 1000)
        metric = encode_metric("bdSeq", None, DT_INT64, self.bd_seq, now)
        return encode_payload([metric], None, now)

    def on_connect(self, client, userdata, flags, rc):
        """Subscribes to NCMD and publishes the birth certificate."""
        del userdata, flags
        if rc != 0:
            print("Broker refused the connection (rc=%d)" % rc, file=sys.stderr)
            return

        client.subscribe(self.topic("NCMD"), qos=0)
        print("connected to %s:%d" % (self.args.host, self.args.port))
        self.publish_birth()

    def on_message(self, client, userdata, message):
        """Answers a "Node Control/Rebirth" NCMD by re-publishing the birth certificate."""
        del client, userdata
        try:
            names = decode_metric_names(message.payload)
        except ValueError as error:
            print("NCMD decode failed: %s" % error, file=sys.stderr)
            return

        if REBIRTH_METRIC not in names:
            return

        print("NCMD rebirth requested")
        self.bd_seq = (self.bd_seq + 1) % SEQ_MODULUS
        self.publish_birth()

    def run(self):
        """Connects, then publishes NDATA on the configured interval until interrupted."""
        self.client.will_set(
            self.topic("NDEATH"), self.death_payload(), qos=0, retain=False
        )
        self.client.connect(self.args.host, self.args.port, keepalive=60)
        self.client.loop_start()

        try:
            while True:
                time.sleep(self.args.interval)
                if self.client.is_connected():
                    self.publish_data()

        except KeyboardInterrupt:
            print("\nstopping")

        finally:
            # A clean stop still owes the host an NDEATH: the will only fires on a
            # dropped connection, not on a graceful DISCONNECT.
            self.client.publish(
                self.topic("NDEATH"), self.death_payload(), qos=0, retain=False
            )
            time.sleep(0.2)
            self.client.loop_stop()
            self.client.disconnect()


def parse_arguments():
    """Parses the command line."""
    parser = argparse.ArgumentParser(description="Sparkplug B edge node simulator")
    parser.add_argument(
        "--host", default="127.0.0.1", help="broker host (default: 127.0.0.1)"
    )
    parser.add_argument(
        "--port", type=int, default=1883, help="broker port (default: 1883)"
    )
    parser.add_argument(
        "--group", default="SerialStudioDemo", help="Sparkplug group ID"
    )
    parser.add_argument("--node", default="EdgeNode1", help="Sparkplug edge node ID")
    parser.add_argument(
        "--client-id", default="sparkplug-edge-node", help="MQTT client ID"
    )
    parser.add_argument("--username", default="", help="broker username")
    parser.add_argument("--password", default="", help="broker password")
    parser.add_argument(
        "--interval", type=float, default=0.5, help="NDATA period in seconds"
    )
    return parser.parse_args()


def main():
    """Entry point."""
    args = parse_arguments()
    if "/" in args.group or "+" in args.group or "#" in args.group:
        print("A Sparkplug group ID cannot contain '/', '+' or '#'", file=sys.stderr)
        return 1

    EdgeNode(args).run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
