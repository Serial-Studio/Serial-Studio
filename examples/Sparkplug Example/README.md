# Sparkplug Edge Node

## Overview

This project exercises Serial Studio's Sparkplug support with a simulated edge node, whose payloads
follow the Sparkplug B v1.0 specification. The
companion script publishes a birth certificate (`NBIRTH`) that declares four process metrics with
both their names and their aliases, then streams alias-only `NDATA` updates, registers an `NDEATH`
last will with the broker, and re-publishes its birth certificate when Serial Studio asks for a
rebirth over `NCMD`.

Serial Studio speaks Sparkplug on top of its normal MQTT driver: turn the mode on, name the group,
and the driver decodes the Protocol Buffers payloads, resolves aliases against the birth
certificates, and hands the dashboard one channel per metric.

> The MQTT driver and Sparkplug mode need a Serial Studio Pro license. See
> [serial-studio.com](https://serial-studio.com/) for details.

## Requirements

- An MQTT broker. [Mosquitto](https://mosquitto.org/) is the usual choice:

```bash
# macOS
brew install mosquitto && brew services start mosquitto

# Debian / Ubuntu
sudo apt install mosquitto && sudo systemctl start mosquitto
```

A default Mosquitto install listens on `127.0.0.1:1883` and, since 2.0, only accepts local
connections without authentication. That is exactly what this example needs. If your broker
refuses anonymous clients, pass `--username` and `--password` to the script and fill the same
credentials into the MQTT pane.

- The Python edge node needs `paho-mqtt`:

```bash
pip install paho-mqtt
```

The Sparkplug payload is Protocol Buffers, but the script carries its own small encoder and
decoder, so no `protobuf` package is involved.

## Running the edge node

The bundled `Sparkplug Edge Node.ssproj` starts the edge node for you. Its control script launches
`sparkplug_edge_node.py` against `127.0.0.1:1883` the moment you press **Connect**, and Serial
Studio stops the helper again when you disconnect, change project, or quit.

The one piece that is not auto-started is the broker. Sparkplug is MQTT underneath, so a broker has
to be listening on `127.0.0.1:1883` before you connect (see [Requirements](#requirements) for
starting Mosquitto). The control script launches the publisher, not the broker; with no broker up
the edge node has nothing to connect to.

To run the edge node by hand instead:

```bash
python3 sparkplug_edge_node.py
```

Flags:

| Flag | Effect |
|------|--------|
| `--host H` | Broker host (default `127.0.0.1`). |
| `--port N` | Broker port (default `1883`). |
| `--group ID` | Sparkplug group ID (default `SerialStudioDemo`). |
| `--node ID` | Sparkplug edge node ID (default `EdgeNode1`). |
| `--client-id ID` | MQTT client ID (default `sparkplug-edge-node`). |
| `--username NAME --password PW` | Broker credentials, for a broker that requires them. |
| `--interval S` | `NDATA` period in seconds (default `0.5`). |

A group ID cannot contain `/`, `+` or `#`: it is interpolated straight into the subscription
topic, so a wildcard would silently subscribe to the wrong namespace. Serial Studio refuses those
characters too.

## Topics and metrics

The node publishes under the `spBv1.0` namespace:

| Topic | When |
|-------|------|
| `spBv1.0/SerialStudioDemo/NBIRTH/EdgeNode1` | On connect, and on every rebirth request |
| `spBv1.0/SerialStudioDemo/NDATA/EdgeNode1` | Every `--interval` seconds |
| `spBv1.0/SerialStudioDemo/NDEATH/EdgeNode1` | Registered as the last will; also sent on a clean stop |
| `spBv1.0/SerialStudioDemo/NCMD/EdgeNode1` | Subscribed, not published |

The birth certificate declares six metrics:

| Metric | Alias | Sparkplug type | Notes |
|--------|-------|----------------|-------|
| `bdSeq` | (none) | Int64 | Birth/death sequence, per the specification carries no alias |
| `Node Control/Rebirth` | 1 | Boolean | The command Serial Studio writes to ask for a new birth |
| `Temperature` | 2 | Float | Slow sine plus noise |
| `Pressure` | 3 | Double | Slow sine plus noise |
| `Motor/Running` | 4 | Boolean | Trips occasionally |
| `Motor/RPM` | 5 | Int32 | Ramps toward 1480 or 0 |

`NDATA` carries the aliases only, with no metric names on the wire. That is the point of the
birth certificate: the host learns `alias 2 means Temperature` once, and every later message is
a handful of bytes. Serial Studio also synthesises one extra channel per edge node, `Online`,
from the birth and death certificates, so the dashboard shows whether the node is alive without
the node having to publish a heartbeat metric.

## Enabling Sparkplug in Serial Studio

1. Pick **MQTT Subscriber** as the I/O interface in the Setup panel.
2. Set **Hostname** to `127.0.0.1` and **Port** to `1883`.
3. Tick **Sparkplug**.
4. Set **Group ID** to `SerialStudioDemo`.

The group ID replaces the topic filter while Sparkplug is on: the driver subscribes to
`spBv1.0/<group>/#`, or to `spBv1.0/#` when the group is left empty. Anything on those topics that
is not Sparkplug traffic for the configured group is still published raw, so a mixed broker keeps
working.

## Create Project from Births

The metric list is discovered, not declared, so the project is generated after the first birth
certificate arrives:

1. Start the broker and the edge node.
2. Connect Serial Studio with Sparkplug enabled.
3. Wait for the pane to report the discovered metrics (a second at most).
4. Press **Create Project from Births**.
5. The Project Editor opens with one group per edge node and one dataset per metric: LEDs for
   booleans, plots for numerics. Save it and close the editor to reach the dashboard.

Or skip all of that and open the bundled `Sparkplug Edge Node.ssproj`, which is what the generator
produces for the default group and node IDs.

Slot indices are assigned in the order the births declare the metrics and are never reused, so a
rebirth that renames aliases does not repoint the datasets of a project you already generated. A
node that publishes *new* metrics after generation does need the project regenerating.

## Rebirth handling

If Serial Studio receives an alias it cannot resolve, or sees a gap in the per-node sequence
number, it publishes a `Node Control/Rebirth` command to `spBv1.0/<group>/NCMD/<node>` (at most one
every five seconds per node) and buffers the traffic until the new certificate arrives. The script
answers by re-publishing `NBIRTH` with a bumped `bdSeq`; you will see the request logged in its
console.

To watch this happen, stop Serial Studio, leave the script running for a while, then reconnect:
the host has no alias table, the first `NDATA` is unresolvable, and a rebirth is requested
immediately.

## Files

- `sparkplug_edge_node.py`: the edge node simulator, with its own protobuf codec.
- `Sparkplug Edge Node.ssproj`: ready-made project for the default group and node IDs.
- `README.md`: this file.

## Notes

- The `NDEATH` will only fires when the connection *drops*. A graceful disconnect sends no will,
  which is why the script publishes its own death certificate on Ctrl+C.
- Sparkplug payloads are binary; the Console view shows them as noise. Switch it to
  **Hexadecimal** if you want to look at the raw bytes.
- Two edge nodes may not share an MQTT client ID. If you run several copies of the script, give
  each one its own `--client-id` and `--node`.
