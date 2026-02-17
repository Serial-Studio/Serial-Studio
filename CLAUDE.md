# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project Overview

Serial Studio is a cross-platform telemetry dashboard built with Qt 6.9.2 and C++20.
It receives data from UART, TCP/UDP, Bluetooth LE, Audio, Modbus TCP/RTU, CAN Bus, and MQTT,
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

**Functions: target 40–80 lines.** Split anything over 100 lines into named helpers.

**Guard clauses over nested error handling:**

❌ `if (ok) { if (valid) { if (!empty) { doWork(); } } }`
✅ `if (!ok || !valid || empty) return; doWork();`

### Comments

**No inline end-of-line comments.**

❌ `doSomething(); // do x y z`

✅ Block comment above the line:
```cpp
// Skip disabled groups to avoid stale dataset reads
if (!group.isEnabled())
  continue;
```

✅ Doxygen for public APIs:
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
4. **No braces on single-statement bodies** (enforced by `RemoveBracesLLVM`).
5. **Validate at system boundaries only** (API input, file I/O, network). Trust internal data.
6. **Maintain SPDX headers.** `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both.
7. **Do not create markdown/doc files** unless explicitly asked.
8. **Update CLAUDE.md** for any significant architectural change.

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
