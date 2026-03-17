# Data Flow in Serial Studio

## Overview

Understanding how data moves through Serial Studio helps you configure it correctly and troubleshoot issues. This page traces the journey of a single byte from your device to a rendered widget on screen.

## The Pipeline

The following diagram shows the complete data flow from hardware device to rendered dashboard widgets, including the optional export path for CSV, MDF4, and API output.

```svg
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 720 920" font-family="monospace" font-size="13">
  <!-- Background -->
  <rect width="720" height="920" fill="#f8f9fa" rx="8"/>

  <!-- Device -->
  <rect x="220" y="20" width="280" height="44" rx="6" fill="#2d3748" stroke="#1a202c" stroke-width="1.5"/>
  <text x="360" y="47" text-anchor="middle" fill="#fff" font-weight="bold">Device (Hardware)</text>

  <!-- Arrow 1 -->
  <line x1="360" y1="64" x2="360" y2="100" stroke="#718096" stroke-width="2" marker-end="url(#arrow)"/>
  <text x="375" y="87" fill="#718096" font-size="11">raw bytes</text>

  <!-- Driver -->
  <rect x="220" y="100" width="280" height="44" rx="6" fill="#2b6cb0" stroke="#2c5282" stroke-width="1.5"/>
  <text x="360" y="127" text-anchor="middle" fill="#fff" font-weight="bold">Driver (I/O Thread)</text>

  <!-- Arrow 2 -->
  <line x1="360" y1="144" x2="360" y2="180" stroke="#718096" stroke-width="2" marker-end="url(#arrow)"/>
  <text x="375" y="167" fill="#718096" font-size="11">raw bytes</text>

  <!-- Circular Buffer -->
  <rect x="220" y="180" width="280" height="44" rx="6" fill="#2f855a" stroke="#276749" stroke-width="1.5"/>
  <text x="360" y="207" text-anchor="middle" fill="#fff" font-weight="bold">Circular Buffer (10 MB, SPSC)</text>

  <!-- Arrow 3 -->
  <line x1="360" y1="224" x2="360" y2="260" stroke="#718096" stroke-width="2" marker-end="url(#arrow)"/>
  <text x="375" y="247" fill="#718096" font-size="11">buffered bytes</text>

  <!-- Frame Reader -->
  <rect x="220" y="260" width="280" height="44" rx="6" fill="#9b2c2c" stroke="#822727" stroke-width="1.5"/>
  <text x="360" y="287" text-anchor="middle" fill="#fff" font-weight="bold">Frame Reader (KMP Detection)</text>

  <!-- Arrow 4 -->
  <line x1="360" y1="304" x2="360" y2="340" stroke="#718096" stroke-width="2" marker-end="url(#arrow)"/>
  <text x="375" y="327" fill="#718096" font-size="11">complete frames</text>

  <!-- Frame Builder -->
  <rect x="140" y="340" width="440" height="140" rx="6" fill="#fff" stroke="#cbd5e0" stroke-width="1.5"/>
  <text x="360" y="362" text-anchor="middle" fill="#2d3748" font-weight="bold">Frame Builder (Main Thread)</text>
  <line x1="160" y1="372" x2="560" y2="372" stroke="#e2e8f0" stroke-width="1"/>
  <text x="180" y="395" fill="#2b6cb0" font-size="12">Quick Plot: CSV split on comma</text>
  <text x="180" y="415" fill="#2b6cb0" font-size="12">JSON Mode: parse JSON directly</text>
  <text x="180" y="435" fill="#2b6cb0" font-size="12">Project File: Decoder -> JS parse() -> values</text>
  <text x="180" y="460" fill="#718096" font-size="11">Output: Frame (groups + datasets with values)</text>

  <!-- Arrow 5 main path -->
  <line x1="360" y1="480" x2="360" y2="530" stroke="#718096" stroke-width="2" marker-end="url(#arrow)"/>
  <text x="375" y="510" fill="#718096" font-size="11">const Frame&</text>

  <!-- Dashboard -->
  <rect x="220" y="530" width="280" height="44" rx="6" fill="#6b46c1" stroke="#553c9a" stroke-width="1.5"/>
  <text x="360" y="557" text-anchor="middle" fill="#fff" font-weight="bold">Dashboard (Zero-Copy, 20 Hz)</text>

  <!-- Arrow 6 -->
  <line x1="360" y1="574" x2="360" y2="610" stroke="#718096" stroke-width="2" marker-end="url(#arrow)"/>

  <!-- Widgets -->
  <rect x="220" y="610" width="280" height="44" rx="6" fill="#d69e2e" stroke="#b7791f" stroke-width="1.5"/>
  <text x="360" y="637" text-anchor="middle" fill="#fff" font-weight="bold">Widgets (Plot, Gauge, Map, ...)</text>

  <!-- Export branch from Frame Builder -->
  <line x1="580" y1="420" x2="640" y2="420" stroke="#718096" stroke-width="1.5"/>
  <line x1="640" y1="420" x2="640" y2="700" stroke="#718096" stroke-width="1.5"/>
  <line x1="640" y1="700" x2="600" y2="700" stroke="#718096" stroke-width="1.5" marker-end="url(#arrow)"/>

  <!-- Export box -->
  <rect x="440" y="680" width="160" height="130" rx="6" fill="#fff" stroke="#ed8936" stroke-width="1.5" stroke-dasharray="6,3"/>
  <text x="520" y="702" text-anchor="middle" fill="#c05621" font-weight="bold" font-size="11">Export Path</text>
  <text x="520" y="722" text-anchor="middle" fill="#718096" font-size="11">(1 heap alloc)</text>
  <line x1="455" y1="732" x2="585" y2="732" stroke="#e2e8f0" stroke-width="1"/>
  <text x="460" y="752" fill="#2d3748" font-size="11">CSV Export</text>
  <text x="460" y="772" fill="#2d3748" font-size="11">MDF4 Export</text>
  <text x="460" y="792" fill="#2d3748" font-size="11">API Server :7777</text>

  <!-- Legend -->
  <rect x="40" y="840" width="640" height="60" rx="6" fill="#edf2f7" stroke="#cbd5e0" stroke-width="1"/>
  <text x="60" y="862" fill="#2d3748" font-size="11" font-weight="bold">Threading:</text>
  <rect x="140" y="850" width="12" height="12" fill="#2b6cb0"/>
  <text x="158" y="862" fill="#4a5568" font-size="11">I/O Thread</text>
  <rect x="260" y="850" width="12" height="12" fill="#9b2c2c"/>
  <text x="278" y="862" fill="#4a5568" font-size="11">I/O Thread</text>
  <rect x="380" y="850" width="12" height="12" fill="#6b46c1"/>
  <text x="398" y="862" fill="#4a5568" font-size="11">Main Thread</text>
  <rect x="510" y="850" width="12" height="12" fill="#fff" stroke="#ed8936" stroke-width="1.5"/>
  <text x="528" y="862" fill="#4a5568" font-size="11">Worker Threads</text>
  <text x="60" y="886" fill="#718096" font-size="11">SPSC = Single-Producer Single-Consumer | KMP = Knuth-Morris-Pratt string search</text>

  <!-- Arrow marker -->
  <defs>
    <marker id="arrow" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
      <polygon points="0 0, 10 3.5, 0 7" fill="#718096"/>
    </marker>
  </defs>
</svg>
```

## Stage 1: Device and Driver

Your device sends raw bytes over one of nine supported transports: UART, TCP/UDP, Bluetooth LE, Audio Input, Modbus RTU/TCP, CAN Bus, USB (libusb), HID (hidapi), or Process I/O.

The selected driver receives bytes on a dedicated I/O thread managed by `DeviceManager`. No parsing happens here — the driver's only job is raw byte transport. Each driver type handles its own protocol: serial framing, TCP streams, BLE characteristic notifications, audio sample buffers, and so on.

Drivers are not singletons. `ConnectionManager` holds one UI-configuration instance per driver type (used by the QML interface), while `DeviceManager` creates a fresh live instance for each active connection.

## Stage 2: Circular Buffer

A 10 MB lock-free single-producer single-consumer (SPSC) ring buffer sits between the driver and the frame reader.

- The driver thread writes bytes into the buffer.
- The frame reader reads bytes out of it.
- No locks, no contention, no blocking.

The SPSC design is a hard architectural constraint. There is exactly one producer (the driver) and one consumer (the frame reader). Adding a second producer or consumer would violate the lock-free guarantee and corrupt data.

The buffer handles burst data without dropping bytes. If data arrives faster than it can be consumed, an overflow counter increments so you can detect the condition.

## Stage 3: Frame Reader

The frame reader runs on the I/O thread alongside the driver. Its job is to scan the circular buffer for frame boundaries and extract complete frames.

Delimiter detection uses the KMP (Knuth-Morris-Pratt) string matching algorithm for O(n + m) performance, where n is the buffer size and m is the delimiter length. Four detection modes are available:

- **End Delimiter Only**: finds the end marker and extracts everything before it.
- **Start and End Delimiter**: finds the start marker, then the end marker, and extracts between them.
- **Start Delimiter Only**: frame boundaries fall between consecutive start markers.
- **No Delimiters**: passes all data through. Use this with a JavaScript parser for length-prefixed or self-delimiting protocols.

After extraction, the frame reader optionally validates a checksum (CRC-8, CRC-16, or CRC-32). Valid frames are placed into a lock-free queue with a capacity of 4096 entries. The main thread is signaled when frames are ready.

The frame reader is configured once before being moved to its thread. If configuration changes (different delimiters, different checksum), the entire frame reader is destroyed and recreated via `resetFrameReader()`. Mutexes are never used.

## Stage 4: Frame Builder

The frame builder runs on the main thread. It dequeues frames from the lock-free queue and processes them according to the current operation mode.

### Quick Plot Mode

1. Split the frame string on commas.
2. If the first row contains all non-numeric values, treat them as column headers.
3. Auto-generate a Data Grid group and a MultiPlot group.
4. Assign values to auto-created datasets.

No project file is needed. This mode is designed for rapid prototyping with CSV-formatted serial output.

### Device Sends JSON Mode

1. Parse the JSON object directly (delimiters are fixed to `/*` and `*/`).
2. Build the Frame structure from the groups and datasets defined in the JSON.
3. No JavaScript parser is involved.

### Project File Mode

1. Apply the configured decoder (Plain Text, Hexadecimal, Base64, or Binary Direct) to convert raw bytes into a parse-ready format.
2. Call the JavaScript `parse(frame)` function via QJSEngine.
3. The JavaScript function returns an array of values (or a 2D array for multi-frame output).
4. Map returned values to datasets by their Frame Index.
5. Build the Frame object with populated dataset values.

### Multi-Source Routing

In multi-device projects, each device (source) has its own frame reader. The frame builder routes data by source ID through `hotpathRxSourceFrame(sourceId, data)`. Each source maintains its own per-source Frame and its own JavaScript engine instance. Source frames are published independently to the dashboard.

## Stage 5: Dashboard

The dashboard receives a `const Frame&` — a read-only reference. No copying. No heap allocation. This is the critical performance constraint that enables Serial Studio to sustain data rates of 256 KHz and above.

The dashboard updates all active widgets with new values. The UI refresh rate is capped at 20 Hz for optimal performance. Time-series data (plots, FFT, GPS trajectory) is appended to fixed-capacity circular buffers that automatically discard the oldest samples.

All dashboard operations run on the main thread. Widget rendering is handled by Qt Quick's scene graph.

## Stage 6: Export (Optional Parallel Path)

The export path is the only place where a heap allocation occurs per frame. When CSV export, MDF4 export, or the API server is active:

1. A single `TimestampedFrame` is created with a nanosecond timestamp (one `make_shared` call).
2. The shared pointer is enqueued into a lock-free queue for each active export worker.
3. Worker threads dequeue and process frames in batches.

Export worker configuration (CSV example):
- Queue capacity: 8192 frames
- Flush threshold: 1024 frames
- Timer interval: 1000 ms (flush at least once per second)

The API server on port 7777 serializes frames to JSON and broadcasts to connected clients using MCP (JSON-RPC 2.0) or the legacy protocol.

## Performance Characteristics

| Operation | Complexity |
|-----------|-----------|
| Circular buffer append | O(1) amortized |
| KMP delimiter search | O(n + m), n = buffer size, m = delimiter length |
| JavaScript parse | O(n) interpreter time per frame |
| Dashboard update | O(d), d = total datasets (zero-copy) |
| Export enqueue | O(1) lock-free |

Total hotpath memory allocations: zero on the dashboard path. One `shared_ptr` per frame on the export path only when export is active.

## Threading Summary

| Component | Thread | Constraint |
|-----------|--------|-----------|
| Driver | I/O thread (QThread) | One driver per DeviceManager |
| Circular Buffer | Shared (write: I/O, read: I/O) | SPSC only, never MPMC |
| Frame Reader | I/O thread | Configured once, then immutable. Recreate on config change. |
| Frame Builder | Main thread | Dequeues from lock-free queue |
| Dashboard | Main thread | Zero-copy `const Frame&` only |
| CSV/MDF4/API Export | Worker threads | Lock-free enqueue from main, batch dequeue on worker |

## Troubleshooting Data Flow

**No data in console**: Check driver configuration — correct port, baud rate, IP address, or BLE characteristic.

**Data in console but no dashboard**: Verify the operation mode. Check that frame delimiters match what your device actually sends. In Project File mode, confirm the JavaScript parser returns valid arrays.

**Garbled data**: Wrong baud rate, wrong decoder method, or mismatched delimiters. Compare raw console output against your expected format.

**Partial frames**: Delimiter mismatch. Your device may be sending `\r\n` while you configured only `\n`, or vice versa. Inspect the raw hex in the console.

**Dashboard not updating**: Check that dataset Frame Index values in the project file match the positions in your parsed data array. Index 1 maps to the first element returned by `parse()`.

**High CPU with no dashboard**: The frame reader may be finding too many false frames. Tighten your delimiters or add checksum validation.

**Export files empty**: Export workers only write when a device is connected. Check that the export was started before disconnecting.

---

## See Also

- [Getting Started](Getting-Started.md) — First-time setup and Quick Plot tutorial
- [Operation Modes](Operation-Modes.md) — Quick Plot, Project File, and Device Sends JSON
- [Project Editor](Project-Editor.md) — Configure frame parsing and dashboard layout
- [JavaScript API](JavaScript-API.md) — Complete parser function reference
- [Widget Reference](Widget-Reference.md) — All 15+ widget types and their data requirements
- [Communication Protocols](Communication-Protocols.md) — Protocol comparison and setup
- [Troubleshooting](Troubleshooting.md) — Solutions to common problems
