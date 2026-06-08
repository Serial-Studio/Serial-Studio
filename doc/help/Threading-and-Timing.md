# Threading and Timing Guarantees

A short, honest description of what Serial Studio's hot path guarantees, what it doesn't, and why the threading model looks the way it does. If you're integrating Serial Studio into something time-sensitive or trying to figure out where to expect jitter, read this once.

## TL;DR

- **Not a hard real-time system.** Serial Studio is built on Qt's event loop. There are no scheduling deadlines, no jitter bounds, and no preemption guarantees. Don't run a flight controller through it.
- **Soft real-time, high throughput.** The hot path targets 256 kHz+ frame rates with zero allocations and lock-free queues. In practice, modern desktop hardware sustains that comfortably.
- **Source-owned timestamps.** Frames are time-stamped at the driver boundary, not at display or export. The dashboard you see and the CSV you export carry the same timestamp.
- **Threading is per-driver and intentional.** Some drivers spawn their own threads; some lean on Qt's async I/O. Frame parsing happens on the main thread by design. Export and persistence happen off the main thread.

## What is and isn't guaranteed

### Guaranteed

- **Order preservation.** Frames are delivered to consumers in the order the driver produced them. The hot path uses a single-producer/single-consumer ring (`moodycamel::ReaderWriterQueue`) that's FIFO by construction.
- **Source-derived timestamps.** Every parsed frame carries a `steady_clock` timestamp set at acquisition. Dashboard, CSV, MDF4, API, gRPC, MQTT, and the session database all see the same instant for the same frame. No consumer re-stamps.
- **No frame loss in steady state.** If your CPU can keep up with the producer, the queue stays drained and nothing is dropped. If it can't, you'll see `[FrameReader] Frame queue full -- frame dropped` in the log. That message is the canary; treat it as a real signal, not a warning.
- **Zero allocation on the hot path.** No `new`, no `make_shared`, no `QByteArray::append` after init. Each `TimestampedFramePtr` comes from a fixed-size slot pool inside `FrameBuilder` and is shared (refcounted) across every consumer; the slot is recycled when the last consumer drops the pointer. The pool falls back to a one-shot `make_shared` and logs a single warning only if every slot is in flight at once, which means a downstream consumer is not draining.
- **Crash isolation across consumers.** A slow MQTT publish or a failing CSV write won't block FrameBuilder or the dashboard. Each consumer has its own worker thread and its own queue.

### Not guaranteed

- **Latency bounds.** There is no upper bound on end-to-end latency from acquisition to dashboard. On an idle machine it's in the low milliseconds; on a loaded machine running a slow Lua transform, it can grow.
- **Jitter bounds.** Frame-to-frame spacing on the dashboard is whatever Qt's event loop schedules that millisecond. The dashboard tick runs at the configurable UI refresh rate (default 60 Hz, adjustable from 1 to 240 Hz); widgets sample the latest frame on their tick.
- **Determinism on Windows.** Windows' `steady_clock` resolution is roughly 15 ms. Two frames produced inside the same tick get the same timestamp at acquisition. Export workers break ties using a monotonic counter (`monotonicFrameNs`), but on the dashboard you'll see them collapse onto one visual sample.
- **Wall-clock accuracy.** All timestamps are `steady_clock`, not `system_clock`. They're great for measuring durations and ordering events; they're not synchronized to NTP and don't help you correlate with external systems by absolute time.
- **Hard deadlines.** Nothing in Serial Studio yields if a frame takes too long. A 50 ms transform on one frame just makes that frame take 50 ms; the next frame starts when this one ends.

## Threading model in practice

The actual thread layout depends on which driver you've selected.

### Drivers with their own threads

These drivers are explicit about owning a thread:

- **HID** (`hidapi`) runs a blocking `hid_read` loop on its own `QThread`.
- **USB** (`libusb`) runs the libusb event loop on one thread and the bulk/isochronous read loop on another.
- **Process I/O** runs the pipe read loop on its own `QThread` so the main thread isn't blocked on `read()` from a child process or named pipe.
- **Audio** runs a high-priority worker thread driven by a 10 ms `Qt::PreciseTimer` to pull samples from miniaudio. The audio backend itself also delivers callbacks on internal device threads.

For these drivers, `HAL_Driver::dataReceived` is emitted from the worker thread and Qt's `AutoConnection` promotes the hop to `Qt::QueuedConnection` automatically. The frame data crosses one thread boundary on the way to `FrameReader`.

### Drivers riding Qt's event loop

These drivers don't spawn threads. They use Qt's async I/O facilities, which run on whatever thread the driver lives on (the main thread, in practice):

- **UART** (`QSerialPort`)
- **Network** (`QTcpSocket`, `QUdpSocket`)
- **Bluetooth LE** (`QLowEnergyController`, `QLowEnergyService`)
- **CAN Bus** (`QCanBusDevice`)
- **Modbus** (`QModbusDevice`)
- **MQTT** (`QMqttClient`)

For these drivers, `dataReceived` fires on the main thread and the connection to `FrameReader` is a same-thread dispatch. No copy, no event-queue insertion.

This is not laziness; it's a measured choice. The Qt classes for these protocols are already non-blocking and deliver bytes via signals. Wrapping them in a worker thread would only add a queued cross-thread emit per frame, which at high rates is the most expensive thing you can do in Qt.

### FrameReader and FrameBuilder run on the main thread

This is the part that most often surprises people, so the rationale is worth stating outright.

> **Why FrameReader is on the main thread:** moving it to a worker thread means every emit of `HAL_Driver::dataReceived` becomes a `QMetaCallEvent` allocation, an event-queue insertion, and a deferred dispatch on the receiving thread. At 256 kHz that's hundreds of thousands of allocations per second, and it dominates the profile. Serial Studio tried this. There were no gains. The CPU and memory cost of the cross-thread hop wiped out anything won by parallelizing the parser. The current design has been validated against UART, audio at 48 kHz+, network, CAN, and Modbus through experience, bug reports, and a fair amount of blood. If it keeps up with audio's FFT pipeline, it keeps up with almost everything else.

The connection from `DeviceManager::frameReady` to `ConnectionManager::onFrameReady` is `Qt::DirectConnection` for the same reason: a queued main-thread-to-main-thread hop is pure overhead.

### Consumers run on worker threads

Everything that consumes a parsed frame except the dashboard runs off the main thread, on a `FrameConsumer` worker:

- **CSV export.**
- **MDF4 export.**
- **Session database** (Pro). The SQLite writer batches inserts in WAL mode; raw bytes go through a second lock-free queue.
- **MQTT publisher.**
- **API server.** All TCP client sockets are serviced on the server's own worker thread, off the main thread, so a slow client can't stall the pipeline.
- **gRPC server** (when enabled).

The main thread enqueues a shared `TimestampedFramePtr` into each consumer's queue and moves on. The worker drains its queue on its own clock. A blocked or slow consumer can only fill its own queue; it can't back-pressure the producer.

### Dashboard runs on the main thread

The dashboard reads from the same `TimestampedFramePtr` everyone else reads from, on the main thread, on the UI tick (default 60 Hz, configurable from 1 to 240 Hz). It samples the latest frame; it doesn't process every frame. At 256 kHz input and a 60 Hz tick, the dashboard is rendering one out of roughly 4,000 frames. Everything else has already been logged or exported by the consumer threads.

This is the right tradeoff for a UI: a 250 kHz refresh would melt the GPU and the user can't see it anyway.

## Timestamp ownership

A frame's timestamp is set once, at the driver boundary, and never overwritten.

```text
Driver
  │ HAL_Driver::publishReceivedData(data, timestamp)
  ▼ (CapturedDataPtr carries timestamp + frameStep)
FrameReader (main thread)
  │ splits chunk into N logical frames, propagates per-frame timestamps
  ▼
FrameBuilder (main thread)
  │ when one captured chunk expands into N parsed frames,
  │ publishes them at timestamp + step·i
  ▼
TimestampedFramePtr → Dashboard, CSV, MDF4, API, gRPC, Sessions, MQTT
```

A few specific guarantees fall out of this:

- **Audio backdates.** When miniaudio hands over a buffer of N samples at sample rate `sr`, the audio driver back-dates the timestamp to `now - (N-1)/sr`. The first sample in the buffer carries the correct acquisition time, not the time the OS got around to calling our callback.
- **Drivers that post to the main thread capture the timestamp before posting.** Anywhere a driver uses `QMetaObject::invokeMethod` to forward bytes to the main thread, it captures `SteadyClock::now()` in the originating thread first. Default-constructed timestamps would fire on the receiving thread, which is almost always wrong.
- **Export workers don't re-stamp.** They derive strictly-increasing offsets from `monotonicFrameNs(frame->timestamp, baseline)` to break ties when the OS clock is too coarse. That's a safety net, not a clock source.

## Practical advice

### When timing matters and Serial Studio is fine

- Logging telemetry from a test stand or a vehicle, where a few-millisecond uncertainty is invisible against the duration of the run.
- FFT and waterfall analysis at audio rates. The 48 kHz pipeline is a daily-driver use case.
- CAN Bus and Modbus monitoring. These protocols cap at rates well below the hot path's headroom.
- Recording sessions for offline analysis. The session database preserves the original timestamps; load them back into Python or MATLAB and you've got the original time series.

### When you should look elsewhere

- **Closed-loop control.** Anything that needs deterministic millisecond-or-less response from sensor read to actuator write. Use a real-time OS or a microcontroller.
- **Hard-deadline data acquisition.** If a single missed frame is unacceptable, you need a kernel-bypass DAQ stack (DAQmx, RT-Linux, etc.), not a Qt application.
- **Sub-millisecond cross-source synchronization.** Each source carries its own clock from its own driver. Two sources won't be aligned to each other better than the OS allows.

### When you might think you have a timing problem and don't

- **"My CSV timestamps look chunky on Windows."** Windows' `steady_clock` ticks at ~15 ms. Same-tick frames did happen at the same time as far as the kernel is concerned. The export worker's `monotonicFrameNs` is what makes them strictly increasing for SQL/CSV ordering, but the visible chunks reflect real clock granularity.
- **"My dashboard is laggy at 100 kHz."** It isn't. The dashboard ticks at the UI refresh rate (60 Hz by default) on purpose, not at the input rate. Open the session report or the CSV after the run; that's the full-rate data.
- **"A widget skips frames."** Widgets sample on their tick, not per frame. They're not supposed to render every frame. The export and session-database paths see every frame; the UI doesn't need to.
- **"My transform makes the dashboard stutter."** Transforms run on the main thread because they read peer-dataset values that are also on the main thread. A heavy transform (regex, JSON parsing, tight Lua loops) will block. Profile it. If you genuinely need expensive math per frame, do it offline against the session database.

## Summary, in one sentence

Serial Studio is a soft-real-time pipeline that survives 256 kHz audio without missing a beat, optimized for throughput and zero copies, with timestamps owned by the driver and a single-threaded hot path that the project has earned the right to keep through years of profiling. Treat it as a logger, not a controller, and it will not surprise you.

## See also

- [Data Flow](Data-Flow.md): the high-level user view of how data moves from device to dashboard.
- [The Data Hotpath](Data-Hotpath.md): the technical deep dive into FrameReader, FrameBuilder, and the lock-free queues.
- [Benchmark Dialog](Benchmark.md): measures the single-threaded hot path on your hardware; the UI freeze it warns about is this threading model in action.
- [Data Sources](Data-Sources.md): per-driver capability summary, including where each driver sits in the threading model.
- [Drivers — Audio Input](Drivers-Audio.md): the canonical proof-of-concept for the high-throughput hot path.
