# Threading and Timing Guarantees

A short, honest description of what Serial Studio's acquisition pipeline guarantees, what it doesn't, and why the threading model looks the way it does. If you're integrating Serial Studio into something time-sensitive or trying to figure out where to expect jitter, read this once.

## TL;DR

- **Not a hard real-time system.** Serial Studio is built on Qt's event loop. There are no scheduling deadlines, no jitter bounds, and no preemption guarantees. Don't run a flight controller through it.
- **Soft real-time, high throughput.** The acquisition pipeline targets 256 kHz+ frame rates with zero allocations and lock-free queues. In practice, modern desktop hardware sustains that comfortably.
- **Source-owned timestamps.** Frames are time-stamped at the driver boundary, not at display or export. The dashboard you see and the CSV you export carry the same timestamp.
- **Threading is per-driver at the edges, unified in the middle.** Some drivers spawn their own threads; some lean on Qt's async I/O. Either way, frame parsing happens on one dedicated pipeline thread, never the GUI thread, by design. Export and persistence happen off that thread too, on their own worker threads.

## What is and isn't guaranteed

### Guaranteed

- **Order preservation.** Frames are delivered to consumers in the order the driver produced them. The pipeline uses a single-producer/single-consumer ring (`moodycamel::ReaderWriterQueue`) that's FIFO by construction.
- **Source-derived timestamps.** Every parsed frame carries a `steady_clock` timestamp set at acquisition. Dashboard, CSV, MDF4, API, gRPC, MQTT, and the Historian all see the same instant for the same frame. No consumer re-stamps.
- **No frame loss in steady state.** If your CPU can keep up with the producer, the queue stays drained and nothing is dropped. If it can't, you'll see `[FrameReader] Frame queue full -- frame dropped` in the log. That message is the canary; treat it as a real signal, not a warning.
- **Zero allocation in the pipeline.** No `new`, no `make_shared`, no `QByteArray::append` after init. `FrameBuilder` stages each parsed frame's values into a pooled `DataBlock` (pre-sized columns, no per-frame allocation) and flushes it to the dashboard and every active export sink when the display tick advances or the block reaches its sample cap, whichever comes first. The dashboard reads the pooled block directly; sinks get one shared trimmed copy. A block's pool slot is recycled once every reader has dropped it (detected by a `use_count()` probe), and only falls back to a one-shot allocation if every slot is in flight at once, which means a downstream consumer is not draining.
- **Crash isolation across consumers.** A slow MQTT publish or a failing CSV write won't block FrameBuilder or the dashboard. Each consumer has its own worker thread and its own queue.

### Not guaranteed

- **Latency bounds.** There is no upper bound on end-to-end latency from acquisition to dashboard. On an idle machine it's in the low milliseconds; on a loaded machine running a slow Lua transform, it can grow.
- **Jitter bounds.** Frame-to-frame spacing on the dashboard is whatever Qt's event loop schedules that millisecond. The dashboard tick runs at the configurable UI refresh rate (default 60 Hz, adjustable from 1 to 240 Hz); widgets sample the latest frame on their tick.
- **Determinism on Windows.** Windows' `steady_clock` resolution is roughly 15 ms. Two frames produced inside the same tick get the same timestamp at acquisition. Export workers break ties using a monotonic counter (`monotonicFrameNs`), but on the dashboard you'll see them collapse onto one visual sample.
- **Wall-clock accuracy.** All timestamps are `steady_clock`, not `system_clock`. They're great for measuring durations and ordering events; they're not synchronized to NTP and don't help you correlate with external systems by absolute time.
- **Hard deadlines.** Nothing in Serial Studio yields if a frame takes too long. A 50 ms transform on one frame just makes that frame take 50 ms; the next frame starts when this one ends.

## Threading model in practice

Every driver's bytes eventually land on one dedicated processing thread ("FramePipeline",
owned by `IO::PipelineHost`), where `FrameReader`, the frame parser engines, and `FrameBuilder`
all live. What differs per driver is how many thread hops it takes to get there.

### Drivers with their own I/O thread

These drivers are explicit about owning a thread for I/O:

- **HID** (`hidapi`) runs a blocking `hid_read` loop on its own `QThread`.
- **USB** (`libusb`) runs the libusb event loop on one thread and the bulk/isochronous read loop on another.
- **Process I/O** runs the pipe read loop on its own `QThread` so the GUI thread isn't blocked on `read()` from a child process or named pipe.
- **Audio** runs a high-priority worker thread driven by a 10 ms `Qt::PreciseTimer` to pull samples from miniaudio. The audio backend itself also delivers callbacks on internal device threads.

For these drivers, `HAL_Driver::dataReceived` is emitted from the I/O worker thread and Qt's `AutoConnection` promotes the hop into the pipeline thread to a `Qt::QueuedConnection` automatically. The frame data crosses one thread boundary (I/O thread to pipeline thread) on the way to `FrameReader`.

### Drivers riding Qt's event loop

These drivers don't spawn an I/O thread. They use Qt's async I/O facilities, which run on whatever thread the driver lives on (the GUI thread, in practice):

- **UART** (`QSerialPort`)
- **Network** (`QTcpSocket`, `QUdpSocket`)
- **Bluetooth LE** (`QLowEnergyController`, `QLowEnergyService`)
- **CAN Bus** (`QCanBusDevice`)
- **Modbus** (`QModbusDevice`)
- **MQTT** (`QMqttClient`)

For these drivers, `dataReceived` fires on the GUI thread. `FrameReader` still lives on the
pipeline thread, not the GUI thread, so this hop is a queued cross-thread dispatch too, same as
for drivers with their own I/O thread.

Either way, the queued hop happens once per received chunk, not once per parsed frame — a chunk
from a single `read()` or socket notification can carry many frames, and splitting it into
frames is `FrameReader`'s job. Keeping that split off the GUI thread amortizes the cross-thread
cost across however many frames the chunk contains, instead of paying it once per frame.

### FrameReader and FrameBuilder run on a dedicated pipeline thread

This is the part that most often surprises people, so the rationale is worth stating outright.

Since spec 0051 M3, `FrameReader`, the frame parser engines, and `FrameBuilder` all live on
`IO::PipelineHost`'s own processing thread, never the GUI thread. Everything downstream of the
chunk queue — delimiter scanning, decoding, `parse()`, transforms, staging values into a pooled
`DataBlock` — runs as ordinary same-thread function calls on that one thread, so none of it pays
a per-frame cross-thread cost. Only two rates ever cross a thread boundary on the frame path:
chunk rate coming in from the driver, and display-tick rate going out to the GUI, where the
dashboard drains finished `DataBlock`s from a lock-free ring on `Dashboard::onDisplayTick`.
Running the split and the parse on a thread of their own keeps a slow parser or transform from
also stalling paint and input handling; running everything inside that thread as plain calls,
never queued signals, keeps the per-frame cost down. The design has been validated against UART,
audio at 48 kHz+, network, CAN, and Modbus through experience, bug reports, and a fair amount of
blood.

`FrameReader::processData` and `PipelineHost::routeFrames` (which hands a completed frame to
`FrameBuilder`) are both plain same-thread calls on the pipeline thread for the same reason: a
queued same-thread hop is pure overhead.

### Consumers run on worker threads

Everything that consumes a parsed frame except the dashboard runs off the pipeline thread, on a `FrameConsumer` worker:

- **CSV export.**
- **MDF4 export.**
- **Historian** (Pro). The SQLite writer batches inserts in WAL mode; raw bytes go through a second lock-free queue.
- **MQTT publisher.**
- **API server.** All TCP client sockets are serviced on the server's own worker thread, off the main thread, so a slow client can't stall the pipeline.
- **gRPC server** (when enabled).

The pipeline thread flushes a shared, trimmed copy of the current `DataBlock` into each consumer's queue and moves on — one copy per flush, made only while at least one such consumer is active, not one per frame. The worker drains its queue on its own clock. A blocked or slow consumer can only fill its own queue; it can't back-pressure the producer.

### Dashboard runs on the GUI thread

The dashboard drains finished `DataBlock`s from a lock-free ring fed by the pipeline thread, on the GUI thread, on the UI tick (default 60 Hz, configurable from 1 to 240 Hz). It renders whatever has accumulated in the ring since the last tick; it doesn't render every frame. At 256 kHz input and a 60 Hz tick, the dashboard is rendering roughly one flush out of every 4,300 frames' worth of data. Everything else has already been logged or exported by the consumer threads.

This is the right tradeoff for a UI: a 250 kHz refresh would melt the GPU and the user can't see it anyway.

## Timestamp ownership

A frame's timestamp is set once, at the driver boundary, and never overwritten.

```text
Driver
  │ HAL_Driver::publishReceivedData(data, timestamp)
  ▼ (CapturedDataPtr carries timestamp + frameStep)
FrameReader (pipeline thread)
  │ splits chunk into N logical frames, propagates per-frame timestamps
  ▼
FrameBuilder (pipeline thread)
  │ when one captured chunk expands into N parsed frames,
  │ publishes them at timestamp + step·i
  ▼
DataBlock (pooled) → SPSC ring → Dashboard
                    → clone_block_trimmed → CSV, MDF4, API, gRPC, Sessions, MQTT
```

A few specific guarantees fall out of this:

- **The per-frame spread inside one chunk is opt-in, not universal.** `frameStep` defaults to
  1 ns, so on a driver that never sets it — UART, network, and most of the other transports
  above — N frames extracted from one read still carry timestamps within nanoseconds of each
  other; the driver never measured their real spacing, so there's nothing truer to interpolate
  from. Only a driver that knows its own sample cadence fills in a real `frameStep`. Audio is
  the example: it computes `frameStep` from the device sample rate and back-dates the chunk so
  the interpolation across `step·i` reflects when each sample was captured.
- **Audio backdates.** When miniaudio hands over a buffer of N samples at sample rate `sr`, the audio driver back-dates the timestamp to `now - (N-1)/sr`. The first sample in the buffer carries the correct acquisition time, not the time the OS got around to calling our callback.
- **Drivers that post to the main thread capture the timestamp before posting.** Anywhere a driver uses `QMetaObject::invokeMethod` to forward bytes to the main thread, it captures `SteadyClock::now()` in the originating thread first. Default-constructed timestamps would fire on the receiving thread, which is almost always wrong.
- **Export workers don't re-stamp.** They derive strictly-increasing offsets from `monotonicFrameNs(frame->timestamp, baseline)` to break ties when the OS clock is too coarse. That's a safety net, not a clock source.

## Practical advice

### When timing matters and Serial Studio is fine

- Logging telemetry from a test stand or a vehicle, where a few-millisecond uncertainty is invisible against the duration of the run.
- FFT and waterfall analysis at audio rates. The 48 kHz pipeline is a daily-driver use case.
- CAN Bus and Modbus monitoring. These protocols cap at rates well below the pipeline's headroom.
- Recording sessions for offline analysis. The Historian preserves the original timestamps; load them back into Python or MATLAB and you've got the original time series.

### When you should look elsewhere

- **Closed-loop control.** Anything that needs deterministic millisecond-or-less response from sensor read to actuator write. Use a real-time OS or a microcontroller.
- **Hard-deadline data acquisition.** If a single missed frame is unacceptable, you need a kernel-bypass DAQ stack (DAQmx, RT-Linux, etc.), not a Qt application.
- **Sub-millisecond cross-source synchronization.** Each source carries its own clock from its own driver. Two sources won't be aligned to each other better than the OS allows.

### When you might think you have a timing problem and don't

- **"My CSV timestamps look chunky on Windows."** Windows' `steady_clock` ticks at ~15 ms. Same-tick frames did happen at the same time as far as the kernel is concerned. The export worker's `monotonicFrameNs` is what makes them strictly increasing for SQL/CSV ordering, but the visible chunks reflect real clock granularity.
- **"My dashboard is laggy at 100 kHz."** It isn't. The dashboard ticks at the UI refresh rate (60 Hz by default) on purpose, not at the input rate. Open the session report or the CSV after the run; that's the full-rate data.
- **"A widget skips frames."** Widgets sample on their tick, not per frame. They're not supposed to render every frame. The export and Historian paths see every frame; the UI doesn't need to.
- **"My transform makes the dashboard stutter."** Transforms run on the pipeline thread because they read peer-dataset values that are also staged on the pipeline thread. A heavy transform (regex, JSON parsing, tight Lua loops) will block that thread and delay every block flush behind it. Profile it. If you genuinely need expensive math per frame, do it offline against the Historian database.

## See also

- [Data Flow](Data-Flow.md): the high-level user view of how data moves from device to dashboard.
- [The Acquisition Pipeline](Data-Hotpath.md): the technical deep dive into FrameReader, FrameBuilder, and the lock-free queues.
- [Benchmark Dialog](Benchmark.md): measures the pipeline's throughput on your hardware by temporarily relocating it onto the GUI thread; the UI freeze it warns about is that relocation in action.
- [Data Sources](Data-Sources.md): per-driver capability summary, including where each driver sits in the threading model.
- [Drivers — Audio Input](Drivers-Audio.md): the canonical proof-of-concept for the high-throughput pipeline.
