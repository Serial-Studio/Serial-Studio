# InfluxDB export (Pro)

You want your sensor readings on a Grafana panel that keeps updating while the capture runs, and keeps existing after you close Serial Studio. The InfluxDB sink sends every parsed sample to an InfluxDB 2.x server as it arrives. It does not work with InfluxDB 1.x. If the server is slow or down, the sink drops points; parsing and the dashboard carry on.

Typical uses: a Grafana dashboard for people away from the bench, or one shared history fed by several benches. To keep samples in a local file instead, see [CSV Export](CSV-Export-Playback.md), [MDF4](MDF4.md) or the [Historian](Historian.md).

## Quick setup

The sink writes the datasets of your project, so build the project first (see [Project Editor](Project-Editor.md)). A device printing `21.5,43,OK` with datasets titled Temperature, Humidity and Status produces the example line under [What gets written](#what-gets-written).

1. Install InfluxDB 2.x; it serves a web UI at `http://localhost:8086`. The first-start form asks for an organization name and a first bucket name. A bucket is where the points are stored.
2. Under API Tokens in the InfluxDB UI, create a token with write access to that bucket. Copy it then; InfluxDB shows it once.
3. In Serial Studio, open the Project Editor and select **Data Export** > **InfluxDB Sink** in the left tree.
4. Fill in **Server URL**, **Organization** and **Bucket**, then paste the token into **API Token**.
5. Switch **Enabled** on, then connect your device. The sink writes only while a device is connected.
6. The header bar above the form reads **Writing to InfluxDB** and its counter line reads `<n> written, ...` with `<n>` climbing. In the InfluxDB Data Explorer, look for the `serial_studio` measurement.

## Edition and licensing

InfluxDB export is a Pro feature (see [Pro vs Free Features](Pro-vs-Free.md)). Without a license or an active trial, **Enabled** stays off, even if the project file was saved with it on.

The first time you try to switch on **Enabled** (or any Pro feature), Serial Studio asks Yes or No whether to start the 14-day trial. The trial never starts on its own, and registering it needs an internet connection. After the trial ends, the prompt offers activation or purchase instead. An active trial unlocks the sink like a purchased license. A license that activates after the project has loaded switches the sink on without a reload.

## Where to configure it

**InfluxDB Sink** is a child of the **Data Export** node (Pro only) in the Project Editor's left tree, next to **MQTT Publisher**.

```
Project
├─ ...
├─ Data Export
│   ├─ MQTT Publisher
│   └─ InfluxDB Sink       <-- here
└─ ...
```

Selecting it opens a form with a **Publishing** section (**Enabled**) and a **Server** section (the other fields). The header bar above the form carries the live state and the counters:

- A green LED and **Writing to InfluxDB** once the endpoint has accepted a write, a red LED and **Not writing** otherwise. When the last write failed, the failure message replaces **Not writing**. A failure reads `Server answered HTTP <status>: <response body, first 1 KiB>` or `TLS verification failed: <reason>`. Redirects are not followed, so point the URL at the final `https` address.
- A counter line on the right, reading `<n> written, <n> dropped, <n> errors`.

## Form fields

| Field | Effect |
|-------|--------|
| **Enabled** | Master toggle. Writes while a device is connected. Stays off without a license or trial. |
| **Server URL** | Base URL of the InfluxDB server, for example `http://localhost:8086`. The sink appends the write path itself. |
| **Organization** | Name of the organization that owns the bucket, as InfluxDB shows it. |
| **Bucket** | Destination bucket. It must already exist; Serial Studio does not create it. |
| **Measurement** | The name your points are stored under. Defaults to `serial_studio`; leave it unless several projects share a bucket. An empty name, or one starting with `#`, falls back to the default, and surrounding whitespace is trimmed. |
| **API Token** | InfluxDB API token with write permission on the bucket. Write-only: after you paste a token the field clears, which is normal, and the placeholder reads `Stored; type a new token to replace it`. |

`Server URL`, `Organization`, `Bucket` and `Measurement` are saved in the project file. The token is not: Serial Studio keeps it, obfuscated, in this computer's settings, keyed by the server's host and port. Moving a project file to another machine carries the endpoint but not the token, and changing the host or port of the URL makes the stored token go missing, so re-enter it in both cases.

### The URL rule

A bearer token must not ride a cleartext connection to a remote host, so the sink accepts:

- `https` to any host.
- `http` only to `127.0.0.1`, `localhost` or `::1`.

Anything else is refused when the field is committed, and the header bar reports `Refused an insecure InfluxDB URL: use https, or http only for a loopback host`. The typed value is not kept. A LAN address on plain `http` is refused, even if it is your own desk; put TLS in front of it.

## What gets written

Each sample becomes one point, written as one line of InfluxDB line protocol:

```
serial_studio,source=0 Temperature=21.5,Humidity=43,Status="OK" 1757001600123456789
```

`Temperature`, `Humidity` and `Status` are dataset titles from the project, and the trailing number is the timestamp in nanoseconds.

- The measurement is the **Measurement** field, and `source` is the only tag. It carries the numeric source id, so several devices in one project stay separable inside a single measurement.
- **Field keys** are dataset titles. An empty title, or a title shared by two datasets, is qualified with the dataset's unique id (`Temperature_7`), so two datasets can never collapse onto one field key and overwrite each other.
- Field types: a number is written as a float, never as an integer or boolean, and text as a string. A dataset keeps the type of its first sample until the sink reopens. A string dataset is omitted from points that carry no text for it. NaN and infinity cannot be written, so they are skipped and counted in `fieldsSkipped`.
- **Timestamps** are the driver's capture times converted to wall-clock nanoseconds and sent with `precision=ns`. The conversion offset is sampled when the sink opens or the endpoint changes, so a system clock step mid-capture shifts later points until then.

Points accumulate into a batch and are POSTed to `<Server URL>/api/v2/write` with the organization, bucket and precision as query parameters and the token in an `Authorization: Token …` header. The sink POSTs at most once per drained set of blocks, and sooner when a batch reaches 512 KiB.

One request is outstanding at a time. If a batch is ready while a request is still in flight, that batch is dropped and counted. The sink drops rather than waits: a capture that stalls for a database is worse than a gap in a graph. A request that hangs is abandoned after 30 seconds so it cannot pin the slot.

## Statistics

The header counters and the API values are pulled on the one-second tick rather than pushed per point:

| Counter (API key) | Meaning |
|-------------------|---------|
| **written** (`pointsWritten`) | Points accepted by the server. |
| **dropped** (`pointsDropped`) | Points that never reached the bucket: a batch rendered while a request was still in flight, the points of a write the server rejected, and anything still queued when the sink closes. |
| **errors** (`httpErrors`) | HTTP failures. The most recent failure message, with the response body cut to 1 KiB, is shown in the header bar. |
| `fieldsSkipped` (API only) | Fields omitted from a point because the value was NaN or infinity. Not shown in the header bar. |

## API commands

Three [API](API-Reference.md) commands cover the sink. They need the API Server turned on (it is off by default, port 7777).

- `influx.setConfig` patches the endpoint with `url`, `org`, `bucket`, `measurement` and `token`. Every field is optional, so a call changes only what it names. The token goes to the same per-machine store as the form field; no command reads it back. A field of the wrong type, or an insecure URL, is rejected wholesale rather than coerced.
- `influx.setEnabled` takes `enabled` (boolean) and toggles the sink. Without a license or trial it reports `enabled=false`.
- `influx.getStatus` snapshots enabled and open state, the endpoint fields, whether a token is stored, the four counters (`pointsWritten`, `pointsDropped`, `fieldsSkipped`, `httpErrors`), and the last write error.

## Common pitfalls

- The URL was refused. `http` is accepted only for a loopback host. Use `https`, or an SSH tunnel to loopback.
- **Enabled will not stay on.** There is no valid license or active trial; see [Edition and licensing](#edition-and-licensing). The header shows **Not writing** and the counters do not move.
- A field is missing or has a `_<id>` suffix. An empty title, or one shared with another dataset, is written under a `_<id>` suffix (`Temperature_7`). Otherwise the values are NaN or infinity, counted in `fieldsSkipped`.
- Points fail with HTTP 422. The bucket already holds that field with a different type, so the points count under `dropped` and `errors`.
- **`dropped` climbs steadily while `errors` stays flat.** The server is slower than the capture. Reduce the sample rate, check the network path, or use a faster server.
- Nothing is written after moving the project or editing the URL. The token is stored per machine, keyed by host and port. Re-enter it.

## See also

- [MQTT Publisher](MQTT-Publisher.md): the other live outbound sink, including a line-protocol template for reaching InfluxDB through Telegraf instead of directly.
- [CSV Export & Playback](CSV-Export-Playback.md), [MDF4 Export & Playback](MDF4.md), [Historian](Historian.md): the local recording sinks.
- [API Reference](API-Reference.md): the `influx.*` commands.
- [Troubleshooting](Troubleshooting.md): general connection and data problems.
