# Serial Studio Performance Optimization Plan

## Executive Summary

This document outlines identified performance bottlenecks in the Serial Studio codebase and proposed optimizations. The optimizations focus on:
1. Compiler optimization flags for modern hardware (2012+)
2. Hot-path data processing improvements
3. String handling optimizations
4. Memory allocation reductions

---

## 1. CMakeLists.txt Compiler Optimization Improvements

### Current State
- Uses `-O2` across all platforms
- Limited CPU-specific optimizations (only SSE4.1 on x86_64)
- Conservative optimization flags

### Proposed Changes
For production builds targeting modern hardware (2012+), upgrade optimization flags:

**All GCC/Clang platforms:**
- Change `-O2` → `-O3` (aggressive inlining and optimization)
- Add `-march=native` when building for local machine, or `-march=x86-64-v2` for portable x86-64 builds
- Add `-ffast-math` (with proper validation for telemetry use case)
- Add `-flto=auto` (link-time optimization with auto-parallelization)
- Add `-DNDEBUG` to disable assertions in release builds

**macOS specific:**
- Already has `-flto`, upgrade to `-flto=auto`
- Add `-march=native` or `-march=x86-64-v2`

**MSVC specific:**
- Already has `/O2` and `/GL`, add `/Ot` (favor speed over size)
- Consider `/arch:AVX` or `/arch:AVX2` for modern processors

**Platform-specific SIMD:**
- Upgrade SSE4.1 to SSE4.2 or AVX for x86-64 (2012+ support)
- Add ARM NEON flags for ARM platforms

### Compatibility Note
`-march=x86-64-v2` provides:
- SSE4.2, SSSE3, SSE4.1
- Compatible with Intel Nehalem (2008+), AMD Bulldozer (2011+)
- Safe for "2012+ hardware" requirement

---

## 2. Hot-Path String Operations (JSON/FrameBuilder.cpp)

### Bottleneck: parseProjectFrame() - Line 402-456
**Issues:**
- `QString::fromUtf8(data.toHex())` creates two temporary objects
- `QString::fromUtf8(data.toBase64())` creates two temporary objects
- `QString::fromUtf8(data).split(',')` creates temporary QString

**Optimizations:**
1. Pre-allocate QStringList with `channels.reserve(64)` (already done ✓)
2. For Hexadecimal mode: Use `data.toHex()` directly, avoid QString conversion until needed
3. For PlainText mode: Consider using `QByteArray::split()` when possible
4. Cache decoder method to avoid repeated calls to `ProjectModel::instance().decoderMethod()`

### Bottleneck: parseQuickPlotFrame() - Line 471-526
**Issues:**
- `QString::fromUtf8(data)` creates temporary QString
- `str.mid(start, i - start).trimmed()` creates multiple temporary strings
- Manual CSV parsing with multiple allocations

**Optimizations:**
1. Use `QByteArray::split(',')` and convert only when necessary
2. Implement zero-copy CSV tokenizer using QByteArrayView (Qt 6.9+)
3. Cache the string conversion result if it will be reused
4. Reserve capacity based on previous frame's channel count (already attempted ✓)

---

## 3. Loop Optimization - Repeated .size() Calls

### Current State
Found 31 instances where `.size()` is called repeatedly in loop conditions:
```cpp
for (int i = 0; i < container.size(); ++i)
```

### Optimization
Cache `.size()` before the loop:
```cpp
const auto size = container.size();
for (int i = 0; i < size; ++i)
```

**Target files (16 files, sorted by frequency):**
1. JSON/FrameBuilder.cpp (6 occurrences)
2. UI/Dashboard.cpp (4 occurrences)
3. JSON/ProjectModel.cpp (3 occurrences)
4. UI/Widgets/LEDPanel.cpp (3 occurrences)
5. UI/Widgets/GPS.cpp, MultiPlot.cpp, others (2 occurrences each)

---

## 4. CircularBuffer KMP Optimization (IO/CircularBuffer.h)

### Bottleneck: findPatternKMP() - Line 292-336
**Issue:**
- Computes LPS (Longest Prefix Suffix) table on every search
- For common patterns (e.g., `\n`, `\r\n`, `*/`), this is redundant

**Optimization:**
1. Cache LPS tables for common delimiters in static storage
2. Use small-pattern fast path for 1-3 byte patterns
3. Consider using SIMD string search for short patterns (SSE4.2 PCMPESTRI)

---

## 5. FrameReader Optimizations (IO/FrameReader.cpp)

### Bottleneck: readEndDelimitedFrames() - Line 236-299
**Issue:**
- Loops through `m_quickPlotEndSequences` calling `findPatternKMP` for each pattern
- Only needs to find the *earliest* match, not all matches

**Optimization:**
1. Implement multi-pattern search (find earliest match in one pass)
2. For QuickPlot mode, common sequences are `\n`, `\r`, `\r\n` - use optimized single-byte search
3. Cache the delimiter that matched last time (likely to match again)

---

## 6. Dashboard Action Model Building (UI/Dashboard.cpp)

### Bottleneck: actions() - Line 438-460
**Issue:**
- Builds QVariantList from scratch on every call
- Creates QVariantMap with string keys for each action

**Optimization:**
1. Cache the QVariantList and only rebuild when actions change
2. Use a change flag to track when action state changes (e.g., timer toggles)
3. Consider returning const reference to cached list when unchanged

---

## 7. Frame Structure Optimizations

### Observation
Frame structures use `alignas(8)` which is good, but:
- `std::vector` members can cause cache misses
- Frequent copying of Dataset objects

**Optimizations:**
1. Use `std::vector::reserve()` more aggressively to avoid reallocations
2. Consider small-vector optimization for datasets (most groups have <16 datasets)
3. Pass Frame by const reference wherever possible (avoid copies)
4. Use move semantics more aggressively in parseProjectFrame/parseQuickPlotFrame

---

## 8. String Concatenation in Dashboard

### Minor Bottleneck: formatValue() and label generation
**Issue:**
- String concatenation with `+` operator creates temporaries
- Used in widgets for axis labels

**Optimization:**
1. Use `QString::arg()` which is optimized for single-pass formatting
2. Pre-allocate string capacity when building labels
3. Cache formatted labels when values haven't changed

---

## Implementation Priority

### High Priority (Significant Impact):
1. **CMakeLists.txt optimization flags** - Easy win, affects entire codebase
2. **parseProjectFrame() string operations** - In critical data path
3. **parseQuickPlotFrame() CSV parsing** - In critical data path
4. **Loop .size() caching** - Low effort, measurable improvement

### Medium Priority:
5. **CircularBuffer KMP caching** - Moderate complexity, good payoff
6. **FrameReader multi-pattern search** - Moderate complexity
7. **Dashboard actions caching** - UI responsiveness improvement

### Low Priority (Nice to have):
8. **Frame structure optimizations** - More invasive, incremental gains
9. **String concatenation** - Micro-optimization

---

## Testing Strategy

1. **Benchmark before/after** using the existing tests in `/tests`
2. **Performance metrics to track:**
   - Frame processing latency (µs per frame)
   - Frames per second throughput
   - CPU usage at high data rates (>1000 frames/sec)
   - Memory allocation rate (via profiler)

3. **Test scenarios:**
   - High-frequency serial data (115200 baud+)
   - Quick Plot mode with 50+ channels
   - Project mode with complex frame parser
   - Bluetooth LE with frequent reconnections

4. **Validation:**
   - Ensure all existing tests pass
   - Visual inspection of dashboard at high data rates
   - Verify no data loss or corruption

---

## Compatibility Considerations

### Compiler Flags:
- `-march=x86-64-v2`: Requires 2008+ Intel or 2011+ AMD processors ✓
- `-ffast-math`: Verify telemetry calculations remain IEEE-compliant
- `-flto`: Increases build time, but worth it for release builds

### Code Changes:
- All optimizations maintain C++20 standard compliance
- Qt 6.9.2 features used appropriately
- No breaking changes to public APIs

---

## Expected Performance Gains

Conservative estimates based on profiling similar applications:

| Optimization | Expected Speedup |
|--------------|------------------|
| Compiler flags (-O3, -march, -flto) | 15-25% |
| String operation optimizations | 10-20% in hot paths |
| Loop caching | 2-5% overall |
| CircularBuffer KMP caching | 5-10% in frame parsing |
| Dashboard action caching | UI lag reduction |

**Combined estimated improvement: 25-40% reduction in frame processing latency**

---

## Risks and Mitigations

1. **Risk:** `-ffast-math` may affect precision
   **Mitigation:** Benchmark with real telemetry data, disable if issues arise

2. **Risk:** `-march=x86-64-v2` may not run on older hardware
   **Mitigation:** Document requirement clearly, provide fallback build option

3. **Risk:** LTO may cause hard-to-debug issues
   **Mitigation:** Keep debug builds without LTO, only use in release

4. **Risk:** String optimization bugs causing data corruption
   **Mitigation:** Extensive testing with automated test suite

---

## Future Considerations

Beyond this plan:
- SIMD vectorization for data processing (AVX2)
- Parallel frame parsing for multi-core systems
- GPU acceleration for FFT and plotting
- Zero-copy data paths where possible
