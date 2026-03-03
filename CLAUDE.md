# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project Overview

Serial Studio is a cross-platform telemetry dashboard built with Qt 6.9.2 and C++20.
It receives data from UART, TCP/UDP, Bluetooth LE, Audio, Modbus TCP/RTU, CAN Bus, MQTT,
Raw USB (libusb bulk/isochronous), HID devices (hidapi), and Process I/O (stdout or named pipe),
then visualizes it in real-time with 15+ widget types. Target data rate: 256 KHz+.

## Build

```bash
# Release
cmake -B build -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug + sanitizers
cmake -B build -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

Key CMake flags: `ENABLE_HARDENING`, `ENABLE_PGO` (3-stage: GENERATE → run → USE),
`USE_SYSTEM_ZLIB`, `USE_SYSTEM_EXPAT` (required for Flathub).

## Directory Structure

```
app/src/
├── IO/          HAL drivers + CircularBuffer + FrameReader
├── DataModel/   FrameBuilder, Frame, FrameConsumer, ProjectModel
├── UI/          Dashboard + 15 widget types
├── API/         TCP server port 7777 (MCP + legacy JSON-RPC)
├── CSV/ MDF4/   File playback & export
└── Misc/        JsonValidator, ThemeManager, utilities
app/qml/         Declarative UI
lib/             KissFFT, QCodeEditor, mdflib, OpenSSL
```

## Architecture

### Critical Files

| File | Purpose |
|------|---------|
| `app/src/SerialStudio.h` | Central enums (BusType, OperationMode, FrameDetection) |
| `app/src/Concepts.h` | C++20 concepts (Numeric, ByteLike, Driver, Frameable…) |
| `app/src/IO/CircularBuffer.h` | Lock-free SPSC ring buffer with KMP delimiter search |
| `app/src/IO/FrameReader.h` | Immutability-based thread safety (NO mutexes) |
| `app/src/IO/HAL_Driver.h` | Abstract driver interface; `ByteArrayPtr = shared_ptr<const QByteArray>` |
| `app/src/DataModel/FrameBuilder.cpp:817` | Hotpath: zero-copy to Dashboard, single alloc to exports |
| `app/src/DataModel/FrameConsumer.h` | Lock-free worker template (dual-trigger flush) |
| `app/src/DataModel/ProjectModel.cpp` | `m_widgetSettings` is the single store for all UI persistence (layout, active group, widget properties) |

### Data Flow

```
Driver thread  →  CircularBuffer (SPSC, KMP)  →  main thread
Main thread    →  FrameReader → FrameBuilder → Frame
               →  Dashboard::hotpathRxFrame(const Frame&)   ← 0 allocs
               →  [exports only] make_shared<TimestampedFrame>  ← 1 alloc
               →  CSV / MDF4 / API::Server worker threads
```

### Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | Config set **once** in constructor. On settings change, call `IO::Manager::resetFrameReader()` to destroy and recreate. **Never add mutexes.** |
| `CircularBuffer` | **SPSC only.** One producer thread, one consumer thread. Never use as MPMC. |
| `Dashboard` | Updated on **main thread only**. Receives `const Frame&` — no copy, no allocation. |
| Export workers | Lock-free enqueue from main thread, batch-processed on worker thread. |

### Operation Modes & Delimiters

| Mode | Value | Frame delimiters | CSV delimiter | JS parser |
|------|-------|-----------------|---------------|-----------|
| ProjectFile | 0 | Configurable (`frameStart`/`frameEnd`/`frameDetection`) | Via JS | ✅ |
| DeviceSendsJSON | 1 | Fixed `/*` `*/` | N/A (JSON) | ❌ |
| QuickPlot | 2 | None (line-based CR/LF/CRLF) | Comma only | ❌ |

Delimiter stack: `frameEnd` → optional `lineTerminator` → `parseCsvValues()` (comma, hardcoded).
Custom delimiters require ProjectFile mode + JavaScript `frameParser`.

### Known Architectural Debt

- **36 singletons** — high coupling, hard to test
- **ProjectModel.cpp** — 4,062 lines, refactoring target
- **Large widgets** — Terminal (1,708), GPS (1,417), Plot3D (1,521) lines
- **No automated tests** — no GTest/Catch2 yet

## Code Style

### Formatting (enforced by clang-format)

- 100-column limit, 2-space indent, Allman braces, pointer binds to name
- Run `clang-format` before committing

### Naming

| Kind | Convention |
|------|------------|
| Classes | `CamelCase` |
| Functions | `camelCase` |
| Variables / parameters | `lower_case` |
| Private members | `lower_case_` (trailing underscore) |
| Constants / constexpr | `kCamelCase` |
| Macros | `UPPER_CASE` |

### Control Flow — The Most Important Rules

**Maximum nesting depth: 3 levels.** Use early returns and early continues instead.

❌ Bad:
```cpp
void process(const Frame &frame)
{
  if (frame.isValid())
    for (const auto &g : frame.groups())
      if (g.isEnabled())
        for (const auto &d : g.datasets())
          if (d.hasValue())            // level 5 — unacceptable
            doWork(d);
}
```

✅ Good:
```cpp
void process(const Frame &frame)
{
  if (!frame.isValid())
    return;

  for (const auto &g : frame.groups())
  {
    if (!g.isEnabled())
      continue;

    processGroup(g);
  }
}

void processGroup(const Group &g)
{
  for (const auto &d : g.datasets())
  {
    if (d.hasValue())
      doWork(d);
  }
}
```

**Single-statement bodies: no braces.**

❌ `if (x) { return true; }`
✅ `if (x) return true;`

**Single-statement bodies on their own line: always follow with a blank line.**

When the body of an `if`, `for`, `while`, or `else` is on a separate line (brace-free),
add a blank line after it before the next sibling statement. This is mandatory — it
makes the boundary of the braces-free body unambiguous at a glance.

❌ Bad:
```cpp
if (!frame.isValid())
  return;
doWork(frame);
```

✅ Good:
```cpp
if (!frame.isValid())
  return;

doWork(frame);
```

❌ Bad (multiple consecutive brace-free ifs):
```cpp
if (s["interpolate"] !== undefined)
  root.interpolate = s["interpolate"]
if (s["showArea"] !== undefined)
  root.showArea = s["showArea"]
```

✅ Good:
```cpp
if (s["interpolate"] !== undefined)
  root.interpolate = s["interpolate"]

if (s["showArea"] !== undefined)
  root.showArea = s["showArea"]
```

The one exception is guard-clause chains at the top of a function where all branches
return/continue and the intent is visually obvious from context.

**Functions: target 40–80 lines.** Split anything over 100 lines into named helpers.

**Guard clauses over nested error handling:**

❌ `if (ok) { if (valid) { if (!empty) { doWork(); } } }`
✅ `if (!ok || !valid || empty) return; doWork();`

### QML Bindings

**`Q_INVOKABLE` functions with arguments are not reactive in QML bindings.** When a binding
calls a `Q_INVOKABLE` with arguments, QML's binding engine does not track it. The correct
fix is to expose the result as a `Q_PROPERTY` with `NOTIFY` on the C++ side so the binding
is naturally reactive — not to add workarounds in QML.

The existing `widgetFontRevision` + comma pattern in this codebase is a known wart from
before this rule existed. Do not introduce new instances of it and do not invent new
workarounds (named hacks like `widgetFont083`, block expressions, etc.) — fix the C++ API.

### Comments

**Prefer self-documenting code over comments.**

Ideally, functions contain no comments at all — the code explains itself through
good naming. When a comment is necessary, one line is the limit except in very
unusual cases.

**No inline end-of-line comments.**

❌ `doSomething(); // do x y z`

✅ Single-line block comment above the line (only when the intent is not obvious):
```cpp
// KMP search requires the needle to be re-initialised after a partial match
m_kmpState = 0;
```

✅ Doxygen for public APIs — keep it brief:
```cpp
/**
 * @brief Validates CRC16-CCITT checksum over frame payload.
 * @param frame  Raw bytes including checksum suffix
 * @return true if checksum matches, false otherwise
 */
bool validateChecksum(const QByteArray &frame);
```

✅ TODO/FIXME for known issues:
```cpp
// TODO: exponential backoff on reconnection
// FIXME: possible race if FrameReader destroyed mid-processData
```

### Performance

- **Hotpaths**: zero-copy const refs, static-cached singleton refs, `[[likely]]`/`[[unlikely]]`
- **Allocations**: 0 on the dashboard path; 1 `make_shared<TimestampedFrame>` when exports active
- **Algorithms**: KMP for delimiters (O(n+m)), `constexpr` CRC tables
- **Never allocate in hotpaths.** Never copy a Frame on the dashboard path.
- Profile first (perf/Valgrind/Instruments) — don't optimize without data.

## Development Rules

1. **No mutexes in FrameReader or CircularBuffer.** Immutability is the design.
2. **No inline end-of-line comments.** Block comments or self-documenting names only.
3. **Max 3 nesting levels.** Use early returns, early continues, extract functions.
4. **No braces on single-statement bodies** (enforced by `RemoveBracesLLVM`). Always follow a brace-free body on its own line with a blank line before the next sibling statement.
5. **Validate at system boundaries only** (API input, file I/O, network). Trust internal data.
6. **Maintain SPDX headers.** `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both.
7. **Do not create markdown/doc files** unless explicitly asked.
8. **Update CLAUDE.md** for any significant architectural change.
10. **Prefer self-documenting code over comments.** Aim for zero comments inside function bodies. One line max when a comment is truly necessary.
9. **Make QML bindings reactive via `Q_PROPERTY`, not workarounds.** If a `Q_INVOKABLE` result needs to be reactive, expose it as a `Q_PROPERTY` with `NOTIFY` on the C++ side. Never introduce new comma-expression hacks or named variants to work around missing signals.

## File Creation Policy

Only create files when the user explicitly asks, or when implementing actual source/config files.

Do **not** create: SUMMARY.md, IMPROVEMENTS.md, TODO.md, STATUS.md, design docs, example files.

Share information conversationally instead.

## MCP / API Testing

API server runs on TCP port 7777. Supports MCP (JSON-RPC 2.0) and legacy JSON-RPC simultaneously.

```bash
# Start client
cd "examples/MCP Client" && python3 client.py
```

182 tools (all API commands), 2 resources (`serialstudio://frame/current`, `serialstudio://frame/history`).

See `examples/MCP Client/README.md` for full documentation.
