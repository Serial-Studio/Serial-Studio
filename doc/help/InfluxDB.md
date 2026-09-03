# InfluxDB export (Pro)

The InfluxDB sink writes the datasets Serial Studio is already parsing into an InfluxDB 2.x bucket, one point per sample, over the database's HTTP write API. Where [CSV Export](CSV-Export-Playback.md), [MDF4](MDF4.md) and the [Historian](Session-Database.md) record to a local file or database, this sink pushes the same samples to a time-series server that Grafana, Chronograf or a downstream alerting rule can query while the capture is still running.

The sink is a per-project singleton: one Serial Studio instance has exactly one InfluxDB sink, and what it writes depends on the project's dataset layout rather than on which bus the data arrived on. Every active source is written into the same measurement and separated by a `source` tag.

HTTP lives entirely on a dedicated worker thread, the same `FrameConsumer` pattern the CSV, MDF4 and Historian sinks use, so a slow or unreachable server never blocks frame parsing, the dashboard, or the UI.

Use this when:

- A dashboard has to outlive the Serial Studio session, or be visible to people who are not at the bench.
- Several benches feed one shared history, each identified by its bucket or measurement.
- Retention, downsampling and alerting belong to the database rather than to the capture tool.

## Edition and licensing

InfluxDB export is a Pro feature. It is compiled only into the commercial build and, at runtime, every enable is re-checked against the licence, so a project file saved with the sink switched on cannot turn it on in a build that is not licensed.

An active trial is a valid licence for this purpose: the trial installs a commercial token, and the sink accepts it exactly as it accepts a purchased one. A licence that arrives after the project was loaded, which is the normal order when a trial is activated or a machine is activated mid-session, re-applies the project's request, so the sink switches on without a project reload.

## Where to configure it

The sink has its own node in the Project Editor's left tree, a top-level item alongside **Control Loop**, **MQTT Publisher**, each action and each data source.

```
Project
├─ Control Loop
├─ MQTT Publisher
├─ InfluxDB Sink           <-- here
├─ Action 1
├─ Main Device (UART)
└─ Dashboard Widgets
    └─ ...groups...
```

Selecting it opens a form in the same table layout as a device source. The header bar above the form carries the live state and the counters:

- A green LED and **Writing to InfluxDB** once the endpoint has accepted a write, a red LED and **Not writing** otherwise. When the last write failed, the failure message replaces **Not writing**.
- A counter line on the right, reading `<n> written, <n> dropped, <n> errors`.

## Form fields

| Field | Effect |
|-------|--------|
| **Enabled** | Master toggle. Re-checked against the licence on every change, so it stays off in an unlicensed build. |
| **Server URL** | Base URL of the InfluxDB server, for example `http://localhost:8086`. The sink appends the write path itself. |
| **Organization** | Organization that owns the bucket. |
| **Bucket** | Destination bucket. |
| **Measurement** | Measurement every point is written under. Defaults to `serial_studio` when the project names none. |
| **API Token** | InfluxDB API token with write permission on the bucket. Write-only: typing a value stores it and clears the field, and no part of the application reads it back. The placeholder reads `Stored; type a new token to replace it` once a token is held. |

`Server URL`, `Organization`, `Bucket` and `Measurement` are saved in the project file. The token is not: it goes to this machine's credential vault, keyed by the endpoint's host and port, stored obfuscated in the machine's settings. Moving a project file to another machine therefore carries the endpoint but not the credential.

### The URL rule

A bearer token must not ride a cleartext connection to a remote host, so the sink accepts:

- `https` to any host.
- `http` only to `127.0.0.1`, `localhost` or `::1`.

Anything else is refused when the field is committed, and the header bar reports `Refused an insecure InfluxDB URL: use https, or http only for a loopback host`. The typed value is not kept. A local InfluxDB on `http://localhost:8086` is the common development case and is allowed; the same server reached by LAN address is not, and needs TLS.

## What gets written

Each sample of each published block becomes one line of InfluxDB line protocol:

```
serial_studio,source=0 Temperature=21.5,Humidity=43i,Valve=true 1757001600123456789
```

- **Measurement** is the **Measurement** field, escaped once.
- **`source`** is the only tag, carrying the numeric source id, so several devices in one project stay separable inside a single measurement.
- **Field keys** are dataset titles. A title is escaped once when the source's structure is published rather than per sample. An empty title, or a title shared by two datasets, is qualified with the dataset's unique id (`Temperature_7`), so two datasets can never collapse onto one field key and overwrite each other.
- **Field types** are latched per dataset on first use. InfluxDB fixes a field's type on first write, so a later sample of the opposite kind is skipped rather than sent to be rejected. Values with no line-protocol spelling, the not-a-number and infinity floats, are skipped the same way and counted in `fieldsSkipped`.
- **Timestamps** are the block's own, in nanoseconds. The sink asks the server for `precision=ns`, so the acquisition timebase survives the round trip instead of being replaced by server arrival time.

Points accumulate into a batch and are POSTed to `<Server URL>/api/v2/write` with the organization, bucket and precision as query parameters and the token in an `Authorization: Token …` header. A batch is flushed when it reaches roughly 512 KiB.

One request is outstanding at a time. If a batch is ready while a request is still in flight, that batch is dropped and counted, because acquisition must never wait on a slow server. A request that hangs is abandoned after 30 seconds so it cannot pin the slot.

## Statistics

The header counters, and the same four values through the API, are pulled on the one-second tick rather than pushed per point:

| Counter | Meaning |
|---------|---------|
| **written** | Points accepted by the server. |
| **dropped** | Points that never reached the bucket: a batch rendered while a request was still in flight, the points of a write the server rejected, and anything still queued when the sink closes. A steadily climbing value means the endpoint cannot keep up with the capture rate. |
| **errors** | HTTP failures. The most recent failure message, truncated, is shown in the header bar. |
| **fieldsSkipped** | Fields omitted from a point: a non-finite float, or a value contradicting the field's latched type. Available through the API rather than the header bar. |

## Scripting it

Three [API](API-Reference.md) commands cover the sink:

- `influx.setConfig` patches the endpoint. Every field is optional, so a call changes only what it names. `token` is accepted and goes to the credential vault; no verb reads it back. A field of the wrong type, or an insecure URL, is rejected wholesale rather than coerced.
- `influx.setEnabled` toggles the sink. An unlicensed build reports `enabled=false`.
- `influx.getStatus` snapshots enabled and open state, the endpoint fields, whether a token is stored, the four counters, and the last write error.

## Common pitfalls

- **The URL was refused.** The scheme rule is the usual cause: `http` is accepted only for a loopback host. Use `https`, or an SSH tunnel to loopback.
- **Enabled will not stay on.** The build is not licensed, or the licence went invalid. In a trial, check that the trial is still active; the counters and the LED stay at rest when the sink is refused.
- **A dataset is missing from the points.** Either its title collided and it is carrying a `_<id>` suffix under a name you did not expect, or its values are contradicting the field type latched on the first write. A bucket whose field was first written as a float will not accept strings for that field.
- **`dropped` climbs steadily while `errors` stays flat.** The server is slower than the capture. Reduce the sample rate, move the server closer, or accept the loss; the sink drops rather than applying backpressure to acquisition.
- **A new machine writes nothing.** Tokens do not travel with the project file. Re-enter the token on each machine.
- **Points arrive with the wrong times.** The sink sends the acquisition timestamps at nanosecond precision. Times that look like ingest time usually mean something else is writing to the same measurement.

## See also

- [MQTT Publisher](MQTT-Publisher.md): the other live outbound sink, including a line-protocol template for reaching InfluxDB through Telegraf instead of directly.
- [CSV Export & Playback](CSV-Export-Playback.md), [MDF4 Export & Playback](MDF4.md), [Historian](Session-Database.md): the local recording sinks.
- [API Reference](API-Reference.md): the `influx.*` commands.
- [Project Editor](Project-Editor.md): the tree the sink node lives in.
- [Pro vs Free Features](Pro-vs-Free.md): InfluxDB export is a Pro feature.
- [Troubleshooting](Troubleshooting.md): general troubleshooting guide.
