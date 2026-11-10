# Threading and Timing Guarantees

Your Arduino prints 1000 lines a second with `Serial.println`. The dashboard redraws about 60 times a second, and every one of those 1000 lines still reaches the CSV file when export is on (see [CSV Export and Playback](CSV-Export-Playback.md)), with the time it arrived. A frame is one line or packet your device sends.

Serial Studio is soft real-time: it has no latency or jitter bounds, so keep closed-loop control off it.

## Practical advice

### When timing matters and Serial Studio is fine

Logging telemetry from a test stand or a vehicle, where a few milliseconds of uncertainty vanish against the length of the run. Audio-rate capture (48 kHz) with the Audio driver (Pro). CAN Bus (Pro) and Modbus (Pro) monitoring, which run well below the pipeline's headroom. Recording sessions with the [Historian](Historian.md) (Pro) and loading the original timestamps back into Python or MATLAB.

### When you should look elsewhere

- **Closed-loop control.** Anything that needs a deterministic response of a millisecond or less from sensor read to actuator write. Use a real-time OS or a microcontroller.
- **Hard-deadline data acquisition.** If a single missed frame is unacceptable, you need a kernel-bypass DAQ stack (DAQmx, RT-Linux), not a Qt application. Nothing in Serial Studio yields when a frame takes too long: a 50 ms transform on one frame makes that frame take 50 ms.
- **Sub-millisecond cross-source synchronization.** All sources share one steady clock, but each chunk is stamped when its driver receives it. Skew between two sources is the difference in their OS and bus latency plus any event-loop delay, and Serial Studio neither measures nor corrects it.

### Behavior that looks like a timing problem

| Symptom | Cause | What to do |
|---------|-------|------------|
| CSV timestamps come in clumps | Frames extracted from one read share the chunk's timestamp, within nanoseconds (see [Timestamp ownership](#timestamp-ownership)) | Nothing. The clump is the arrival time of the read, which is all the driver measured. |
| The dashboard looks laggy at 100 kHz | The dashboard redraws at the UI refresh rate on purpose, not at the input rate | The rate is set under **Plotting > UI Refresh Rate (Hz)** in [Preferences](Preferences.md) (default 60, range 1 to 240). Open the CSV or the Historian (Pro) for full-rate data. |
| A widget skips frames | Widgets sample the latest data on each redraw, not per frame | Nothing. Export and the Historian see every frame. |
| The dashboard updates late or in jumps while a [dataset transform](Dataset-Transforms.md) is active | Transforms run on the pipeline thread, so a heavy one (regex, JSON parsing, tight Lua loops) delays every block flush behind it. The window stays responsive; only the data is late. | Profile it. For expensive per-frame math, work offline against the Historian database. |

## What is and isn't guaranteed

### Guaranteed

- **Order.** Frames reach every consumer in the order the driver produced them.
- **No frame loss while the CPU keeps up.** If it can't, frames are lost in one of two ways. [FrameReader](Data-Hotpath.md) drops frames, writes `[FrameReader] Frame queue full -- frame dropped (total N)` to the application log, and posts a **Frames dropped** notification, throttled to once every 5 seconds. Separately, if every block in the pool is still held by a consumer, the pipeline does not allocate; it drops blocks, writes `[FrameBuilder] Block pool exhausted ... dropping blocks` to the log and posts a **Block pool exhausted** notification, once. Data is then missing from the display and from recordings. The usual cause is a consumer that is not draining: a slow API client, a stalled recording or a heavy widget. To get back under the limit, lower the baud rate or data rate, simplify the parser or transform, or close widgets and recordings you don't need.
- **No per-frame allocation in the pipeline.** Parsed values are written into reused buffers, so memory use stays flat at any data rate. Details: [The Acquisition Pipeline](Data-Hotpath.md).
- **Consumers don't block each other.** A slow MQTT publish or a failing CSV write doesn't block the pipeline or the dashboard. Each consumer has its own worker thread and its own queue.

### Not guaranteed

There is no upper bound on the time from acquisition to dashboard. The dashboard adds up to one refresh period (16.7 ms at the default 60 Hz), and a slow Lua transform on a loaded machine adds more. Frame-to-frame spacing on the dashboard is whatever Qt's event loop schedules, since widgets sample the latest data on each tick of the UI refresh rate (**Plotting > UI Refresh Rate (Hz)** in [Preferences](Preferences.md)).

Timestamps come from `std::chrono::steady_clock`, not `system_clock`. They measure durations and order events, but they aren't synchronized to NTP, so they don't line up with external systems by absolute time.

## Threading model in practice

You don't need this section to use Serial Studio; it explains why a slow parser can't freeze the window. Every driver's bytes land on one dedicated processing thread, the pipeline thread (owned by `IO::PipelineHost`). `FrameReader`, the frame parser engines and `FrameBuilder` all live there. The drivers differ in where they read bytes before the hop.

| Where the driver reads bytes | Drivers |
|------------------------------|---------|
| Its own I/O thread | HID (Pro), USB (Pro), Process I/O (Pro), Audio (Pro), EtherNet/IP (Pro), S7 (Pro) |
| The GUI thread, through Qt's event loop | UART, Network, Bluetooth LE, CAN Bus (Pro), Modbus (Pro), MQTT (Pro) |

OPC UA (Pro) and IEC-104 (Pro) are not in the table; see [Devices](Devices.md) for where they sit. The built-in gs_usb CAN backend is an exception to its row: it reads on its own thread. In every case the data crosses to the pipeline thread by a queued connection, once per received chunk, not once per frame. One read or socket notification can carry many frames, and splitting it is `FrameReader`'s job on the pipeline thread.
### FrameReader and FrameBuilder run on a dedicated pipeline thread

Delimiter scanning, decoding, `parse()`, transforms and staging values into a block all run as plain function calls on that one thread, so none of it pays a cross-thread cost per frame. Only two rates cross a thread boundary on the frame path: the chunk rate coming in from the driver, and the display-tick rate going out to the GUI. A slow transform stalls the pipeline thread, never paint and input handling.

### Consumers run on worker threads

Everything that consumes a parsed frame, except the dashboard, runs off the pipeline thread on its own worker:

- CSV export (Free).
- MDF4 export (Pro).
- InfluxDB export (Pro).
- Historian (Pro). The SQLite writer batches inserts, and raw bytes go through a second queue.
- MQTT publisher (Pro).
- API Server, off by default, port 7777. Its TCP client sockets are serviced on the server's own worker thread, so a slow client can't stall the pipeline.
- gRPC server, port 8888. It runs only while the API Server is enabled.

The pipeline thread flushes a shared, trimmed copy of the current block into each consumer's queue and moves on, one copy per flush rather than one per frame, and only while at least one such consumer is active. A blocked consumer can fill its own queue, but it can't back-pressure the producer.

### Dashboard runs on the GUI thread

The dashboard drains finished blocks on each UI refresh tick and renders whatever has accumulated since the last tick. At the 256 kHz benchmark rate and the default 60 Hz tick, one redraw covers about 4,300 frames, delivered as several blocks because each flushes at 64 samples (frame mode) or 4096 (stream mode). Consumers drain their own queues independently of the dashboard.

## Timestamp ownership

A frame's timestamp is set once, at the driver boundary, and never overwritten. Each chunk is stamped when the driver receives it, and the dashboard, CSV, MDF4, the Historian, MQTT, the API and gRPC all use that stamp.

- **One chunk, many frames.** The spacing between frames inside one chunk (`frameStep`) defaults to 1 ns. N frames extracted from one UART or network read therefore carry timestamps within nanoseconds of each other; the driver never measured their real spacing. Only Audio (from the device sample rate) and CAN Bus (from the span of each batched burst) set a real `frameStep`.
- **Audio back-dates.** When miniaudio hands over a buffer of N samples at sample rate `sr`, the first sample carries its true acquisition time rather than the time the OS called the audio callback.
- **Tie-breaking.** Export workers don't re-stamp. When two frames share a timestamp they add a minimal offset so exported times strictly increase (`monotonicFrameNs`). This is a tie-breaker, not a clock.

## See also

- [Data Flow](Data-Flow.md): the high-level user view of how data moves from device to dashboard.
- [The Acquisition Pipeline](Data-Hotpath.md): the technical deep dive into FrameReader, FrameBuilder and the lock-free queues.
- [Benchmark Dialog](Benchmark.md): measures pipeline throughput on your hardware; the UI freeze it warns about is expected.
- [Devices](Devices.md): per-driver capability summary, including where each driver sits in the threading model.
- [Drivers: Audio Input](Drivers-Audio.md): the canonical proof-of-concept for the high-throughput pipeline.
- [Pro vs Free Features](Pro-vs-Free.md): which drivers and exporters need Pro.
