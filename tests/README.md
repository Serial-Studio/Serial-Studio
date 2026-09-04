# Tests

Integration, security, performance, and script-unit tests for Serial Studio.

Most of these tests connect to a running Serial Studio instance over TCP (port 7777), simulate devices sending telemetry, and check that frames are parsed, exported, and displayed correctly.

## Quick start

**Requirements:** Python 3.8 or later.

- Integration, security, and performance tests also need a running Serial Studio instance with the API server enabled: **Settings → Miscellaneous → Enable API Server** (port 7777).
  The port is configurable since spec 0075: the `API/Port` QSettings key, the port spin box beside
  the enable toggle, and the `--api-port <n>` CLI flag all set it, defaulting to `API_TCP_PORT`
  (7777). The fixtures still assume 7777, so leave it there when running the suites. A
  local-only server now binds **both** loopback families (127.0.0.1 and ::1); an external server
  binds one dual-stack `Any` listener.
- Script tests need **Node.js** (`node` on `PATH`). Serial Studio itself does not need to be running.

```bash
# Install dependencies
pip install -r tests/requirements.txt

# Run all integration tests
pytest tests/integration/ -v

# Run all security tests
pytest tests/security/ -v

# Run all frame-parser script tests (Node.js only, no Serial Studio needed)
pytest tests/scripts/ -v

# Run a single file
pytest tests/integration/test_frame_parsing.py -v

# Run a single test
pytest tests/integration/test_csv_export.py::test_csv_export_basic -v
```

## Test categories

| Category    | Directory             | Requires Serial Studio | Description                                  |
|-------------|-----------------------|------------------------|----------------------------------------------|
| Integration | `tests/integration/`  | Yes                    | Functional tests against a live API          |
| Security    | `tests/security/`     | Yes                    | Resilience and boundary tests            |
| Performance | `tests/performance/`  | Yes                    | Throughput benchmarks                        |
| Scripts     | `tests/scripts/`      | No (Node.js only)      | Unit tests for JS frame-parser scripts, plus source-level C++ regressions |
| Tooling     | `scripts/tests/`      | No                     | Fixture-driven tests for `code-verify.py` and the CI workflows |
| C++ units   | `app/tests/`          | No (ctest, not pytest) | Qt Test suites over selected production TUs  |
| Fuzz        | `app/tests/fuzz/`     | No (ctest or libFuzzer)| Untrusted-bytes entry points + checked-in corpora |

## Integration tests (`tests/integration/`)

Each test connects to Serial Studio over TCP, configures it through the API, streams simulated telemetry, and asserts on the result.

| File                                 | What it covers |
|--------------------------------------|----------------|
| `test_frame_parsing.py`              | All 8 checksum types, JSON/CSV parsing, custom delimiters, high-frequency frames |
| `test_csv_export.py`                 | Export enable/disable, timestamps, high-frequency sessions |
| `test_csv_parsing_verified.py`       | Custom CSV delimiters (comma, semicolon, tab, pipe) via JS parsers |
| `test_csv_player.py`                 | CSV player lifecycle: open, play, pause, seek, next/prev frame, progress |
| `test_project_configuration.py`      | Operation modes, JS parsers, project creation, actions |
| `test_project_editor.py`             | Add, delete, and duplicate groups, datasets, and actions via the API |
| `test_project_import_export.py`      | `project.exportJson`, `project.loadFromJSON`, round-trip fidelity |
| `test_project_undo.py`               | Undo/redo history: randomized round-trips, atomic batch/cascade undo, save-point modified flag, multi-field dataset patch as one step |
| `test_property_registry.py`          | Spec 0036 property registry: corpus round-trip against checked-in baselines (capture with `SS_CAPTURE_BASELINES=1` on the pre-change build), the two declared defect fixes, lossless read-then-write-back, every enum domain set by value |
| `test_project_save.py`               | File save and reload, widget settings persistence, layout storage |
| `test_api_surfaces.py`               | Spec 0037 generated surfaces: MCP `tools/list` typed properties + enum domains on `project.dataset.update`, `tools/list` payload size, SDK options-bag field sweep, typed-proto parity (set `SS_EXPORTED_PROTO`; needs a commercial build with `ENABLE_GRPC=ON`) |
| `test_problem_center.py`             | `problems.*` diagnostics: project findings, link counters, failing transforms |
| `test_widget_extensions.py`          | Spec 0038 widget extensions: `extension.widget` findings for uninstalled, malformed, incompatible, dependency-less and reserved-id packages, scope acceptance, no Pro bypass. Two tiers -- the seeded tier writes packages under `<workspace>/Extensions/widget` and skips until Serial Studio is restarted once (set `SS_WORKSPACE` for a non-default workspace, `SS_CLEAN_TEST_PACKAGES=1` to remove them) |
| `test_connection_diagnostics.py`     | `diagnostics.*` connection self-checks: the three reachability verdicts, byte-free probe, ack-and-poll, bus scoping |
| `test_console_configuration.py`      | Echo, timestamps, display and data modes, font, send, line endings |
| `test_source_mirror.py` | Setup-pane → project mirror (single-source): a bus-type switch replaces source 0's connection settings with the new driver's; a driver option edit lands in source 0. |
| `test_console_ansi_vt100.py`         | ANSI SGR colors (standard, bright, 256, RGB), VT100 cursor, edge cases |
| `test_dashboard_configuration.py`    | FPS, data points, operation mode, status and data query fields |
| `test_window_layout.py`              | Active group, window states, auto/manual layout, widget settings persistence |
| `test_batch_api.py`                  | Batch command processing, size limits, ordering, partial failure |
| `test_2d_array_parsing.py`           | Multi-frame expansion from 2D-array JS parsers, BLE use cases |
| `test_driver_api_comprehensive.py`   | Every driver command: UART, Network, BLE, Modbus, CAN Bus, Audio |
| `test_new_driver_api.py`             | HID, raw USB, and Process driver commands; bus-type enumeration |
| `test_api_drivers.py`                | Driver switching, UART/Network/BLE basics, console/export status |
| `test_mcp.py`                        | MCP JSON-RPC 2.0: lifecycle, tools list, read/write calls, resources, prompts |
| `test_licensing.py`                  | License status shape, set/activate/deactivate, concurrent connections |
| `test_image_view.py`                 | ImageView widget: autodetect, manual mode, corrupted data |
| `test_performance_concepts.py`       | Data rate vs. render rate decoupling, FPS range limits |
| `test_workflows.py`                  | End-to-end: configure, connect, receive, export; reconnection, pause/resume |
| `test_fuzzy.py`                      | Malformed JSON, binary garbage, partial frames, oversized frames, chaos |
| `test_recording_fidelity.py`         | Spec 0075: the disconnect and pause tails land in ONE file, two sources keep their own clocks, a read-only historian directory does not report recording |
| `test_historian_live_guard.py`       | Deleting the session that is being recorded leaves it in place and the app answering |
| `test_driver_drops.py`               | TCP peer close, reconnect after a drop, WebSocket server close, serial pty removal (needs `socat` / `websockets` for two of the four) |
| `test_connection_verdicts.py`        | The spec-0050 verdict matrix over the API; TCP is treated as an **async** bus (read the verdict from the status, not from `io.connect`'s response) |
| `test_audio_loopback.py`             | Capture survives with no output device; a written tone is accepted whole by the playback ring |
| `test_modbus_groups.py`              | Stub Modbus TCP server with an injected dropped reply: every published frame is a valid RTU frame, a dropped poll never puts two group-B frames back to back |
| `test_sparkplug_host.py`             | Own mosquitto instance: every wire index survives a broker cycle when nodes re-birth in reverse order |
| `test_script_deadlines.py`           | A looping script on every API-reachable surface times out and the app still answers (parser, transform per lane, control script, output widget, painter, dry runs) |
| `test_project_integrity.py`          | Spec 0075 H-series: a display setting never writes the file, an action payload edit dirties and persists, template+params undo as one step, a new source's parser is not source 0's, the five legacy fixtures migrate, a corrupt external write leaves the document attached |
| `test_dashboard_lanes.py`            | Samples-axis plot fed through `dashboard.tailFrames`, survives a point-count change; audio-gated stream-lane case |
| `test_assistant_autosave.py`         | The assistant takes checkpoints, not saves: the file hash never moves across edits, only `project.save` writes |
| `test_extension_install.py`          | A v1 catalog never installs, a corrupt update keeps the installed version, an http repository is refused |
| `test_cli_licensing.py`              | Fast non-zero exit on a bad key, clean no-op deactivate, `--reset` (needs `SS_BINARY`) |
| `test_api_ipv6.py`                   | `::1` is served, whatever `localhost` resolves to is served, IPv4 still works |
| `test_grpc_lifecycle.py`             | gRPC port follows the API state, garbage is survivable, churn does not wedge the JSON API (skips without a gRPC build) |

```bash
# Run all integration tests
pytest tests/integration/ -v --tb=short

# Run a specific file
pytest tests/integration/test_frame_parsing.py -v

# Run a specific test
pytest tests/integration/test_frame_parsing.py::test_checksum_validation -v -s

# Run only CSV-related tests
pytest tests/integration/ -m "csv" -v

# Run only project-related tests
pytest tests/integration/ -m "project" -v
```

## Security tests (`tests/security/`)

Boundary tests against the TCP API server. They do not reset state between runs, and they use a dedicated `security_client` fixture that tolerates disconnections.

| File                           | What it covers |
|--------------------------------|----------------|
| `test_api_security.py`         | JSON probes, injection probes, buffer abuse, batch probes, connection exhaustion |
| `test_api_weaknesses.py`  | Input validation bypass, command injection, state manipulation, info disclosure |
| `test_resource_exhaustion.py`    | Memory exhaustion, CPU spikes, connection floods, Slowloris, amplification, queue overflow |
| `test_probe_techniques.py`   | Race conditions, integer overflow, parser confusion, timing probes, deserialization |
| `test_unknown_input_boundary.py` | JS sandbox escape, prototype pollution, null-byte injection, terminal escape injection |
| `test_protocol_fuzzing.py`     | Malformed protocol messages, encoding confusion, framing probes |
| `test_access_control.py`       | Auth bypass, privilege escalation, cross-client interference, info disclosure |
| `test_http_on_api_socket.py`   | Every HTTP verb closes the connection with no readable answer, the body never runs as a command, the raw lane stays gated, normal clients unaffected |
| `test_path_policy_all_commands.py` | The 12 guarded command/parameter pairs answer `PATH_NOT_ALLOWED`; `openDatabase` creates no `-wal`/`-shm` sibling; an allowed temp file reaches the handler |
| `test_write_backlog.py`        | The pending-write cap is **16 MiB per socket**: over it, a broadcast is skipped and counted (producer-paced, self-healing) but a command **response** disconnects the client, because skipping an answer it asked for wedges it forever. A healthy peer keeps working |

```bash
# Run all security tests
pytest tests/security/ -v

# Skip tests that may crash or hang the server
pytest tests/security/ -m "not destructive" -v

# Critical weaknesses only
pytest tests/security/ -m "security and critical" -v

# Resource_exhaustion tests only
pytest tests/security/ -m "resource_exhaustion" -v

# Or run the convenience shell script
bash tests/security/run_all_security_tests.sh
```

## Performance tests (`tests/performance/`)

Benchmarks that measure frame throughput and overhead using `pytest-benchmark`.

| File                       | What it covers |
|----------------------------|----------------|
| `benchmark_frame_rate.py`  | Throughput at 10 to 1000 Hz, checksum algorithm overhead, frame-size impact |

```bash
# Run benchmarks
pytest tests/performance/ -v

# With pytest-benchmark output
pytest tests/performance/ --benchmark-only -v

# Compare against a saved baseline
pytest tests/performance/ --benchmark-compare
```

## Script tests (`tests/scripts/`)

Unit tests for the JavaScript frame-parser scripts in `app/rcc/scripts/parser/js/`. Node.js is required. Serial Studio does not need to be running.

Each test calls `run_parser(script_name, frame)`, which spawns a fresh Node.js subprocess, runs the parser against the given frame, and returns the decoded values. Because every call is a new process, there is no shared state between tests.

| File                        | What it covers |
|-----------------------------|----------------|
| `test_frame_parsers.py`     | 28 parser classes: AT commands, Base64, binary TLV, COBS, CSV, fixed-width, hex bytes, INI, JSON, key-value, MAVLink, MessagePack, Modbus, NMEA 0183, NMEA 2000, pipe-delimited, raw bytes, RTCM, semicolon CSV, SiRF binary, SLIP, tab CSV, UBX/u-blox, URL-encoded, XML, YAML, batched sensor data, time-series 2D |
| `test_cpp_regressions.py`   | Logic-only regressions for C++ bugs that don't require a running app (bounds checks, null guards, API schema consistency) |
| `test_diagnostics_static.py`| Spec-0035 registration drift: safety tiers, GPL-block handler, bus slugs and checker ids, command manifest and bindings, no blocking primitive in the diagnostics sources |
| `test_widget_manifests.py`  | Spec-0038 widget extension packages: bundled manifests validate against `widget-manifest.json`, the reserved-id + `replaces` pairing (positive and negative seeds), reserved ids agree across schema/C++ catalog/widget-string mappers, `rcc.qrc` sync, `hostContextNames()` mirrors ModuleManager, and no spec-0038 file describes extensions as contained |
| `test_proto_ledger_static.py`| Spec-0037 gRPC field-number ledger: numbers unique per command, `fields`/`reserved` disjoint, `1` never assigned, `next` monotonic, typed proto agrees with the ledger, simulated insertion/removal stays append-only |

### Tooling tests (`scripts/tests/`)

Runnable with no Qt, no Node.js and no app, like `tests/scripts/`. `test_code_verify.py` drives
`code-verify.py` against one `good`/`bad` fixture pair per rule kind under
`scripts/tests/fixtures/<kind>/`, plus the three census ratchets; `test_ci_workflow.py` asserts
the shape of both GitHub workflows (action pins, job permissions, the upload gating) and enforces
the xfail policy below. The `lint` job and `sanitize-commit.py` both run them.

```bash
pytest scripts/tests/ -q
```

**Adding a lint rule means adding a fixture pair.** A rule with no fixture goes in that file's
`UNFIXTURED` set with a stated reason; that is how the dead `id-placement` rule was caught.

```bash
# Run all script tests (Node.js required)
pytest tests/scripts/ -v

# Run a single parser class
pytest tests/scripts/test_frame_parsers.py::TestNmea0183 -v

# Run C++ regression checks
pytest tests/scripts/test_cpp_regressions.py -v
```

## C++ unit tests (`app/tests/`)

A separate tier from everything above: Qt Test suites compiled into small binaries and run by
`ctest`, not by pytest. No Python, no Node.js, and no running Serial Studio. Each suite links only
the production translation units it exercises, never the application target, so the tier configures
and links in seconds. As of spec 0076, a suite covering a unit that moved into `core/` (e.g.
`tst_circular_buffer`, `tst_checksums`, `tst_dsp_kernels`, `tst_async_engine`) links
`SerialStudio::Core` / `SerialStudio::Protocols` instead of recompiling that `.cpp`, so it
exercises the exact object the application links too.

The targets exist only when the build is configured with `-DSS_BUILD_TESTS=ON`. There are no
CMake presets — configure by hand. Sanitizer builds (`-DDEBUG_SANITIZER=ON` for ASan+UBSan, or
`-DENABLE_TSAN=ON`; mutually exclusive) must also pass `-DSS_USE_MIMALLOC=OFF`. The CI `unit`
job (x86_64 + arm64 for the `DSPSimd` lanes) inlines its lean flags in `ci.yml`: GRPC/WebEngine
off, never builds the app.

```bash
# Configure, build, and run the whole tier
cmake -G Ninja -B build/dev -DCMAKE_BUILD_TYPE=Debug -DSS_BUILD_TESTS=ON
cmake --build build/dev --target ss_unit_tests
ctest --test-dir build/dev --output-on-failure

# One suite
ctest --test-dir build/dev -R dsp_kernels --output-on-failure
```

| Suite | What it covers |
|-------|----------------|
| `tst_circular_buffer` | `roundUpToPowerOfTwo`, append/read/peek/discard accounting, `setCapacity` reconfigure, overflow counting, and the KMP / short-pattern scan lanes in the linear and wrap-straddling cases |
| `tst_checksums`       | All ten `IO::checksum()` algorithms against published `"123456789"` vectors, output byte order, empty and single-byte inputs, registry consistency, unknown names |
| `tst_frame_serialization` | `toJson`/`fromJson` round-trips for `Dataset`, `Group`, `Action`, `Source`, `Frame`, `AlarmBand`, `FrequencyMarker`, `OutputWidget`, `RegisterDef`, `TableDef`, and the workspace/folder structs |
| `tst_dsp_kernels`     | Every `DSPSimd.h` kernel compared bit-for-bit against a scalar build of the same header, over edge lengths, source offsets, and NaN / ±0.0 / ±inf / denormal payloads |
| `tst_real_fft`        | `kiss_fftr` bins 0..N/2 against a complex `kiss_fft` of the same real signal (relative-to-peak tolerance, the one non-bit-exact comparison in the tree) and the odd-size rejection both FFT widgets rely on |
| `tst_frame_delimiters`| `IO::FrameReader` extraction: start / end / start+end delimiters, delimiters split across chunks, multi-byte delimiters at the scan-lane boundary, validation outcomes, dropped-frame and overflow accounting |
| `tst_async_engine`    | The spec-0034 task-tree engine against a virtual clock: sequential and parallel outcomes, timeouts, the retry backoff schedule, cancel mid-step and mid-backoff |
| `tst_ring_wrap`       | `CircularBuffer` wrap-state gaps: `operator[]` across the boundary, `pos`-resumed pattern scans on wrapped content, `setCapacity` while wrapped, offset `peekRangeInto` spans |
| `tst_frame_reader_modes` | `IO::FrameReader` beyond the delimiter suite: checksum x detection-mode combinations, 1/2/4-byte checksum widths, zero-length frames, `NoDelimiters` saturation, mode-switch checksum reset, multi-chunk timestamps, slot-pool heap fallback |
| `tst_frame_json_legacy` | `Frame.cpp` edge paths: `get_tx_bytes`, `read_io_settings`, table folder paths, `commercialCfg` stamping, legacy alarm-band fallback, clamps and malformed-entry skip/abort behavior |
| `tst_frame_support`   | `SerialStudioFrameSupport.cpp` helpers: `hexToBytes`, `resolveEscapeSequences`, `commercialCfg` overload parity and gates, `encodeText`/`decodeText` across all eleven encodings |
| `tst_frame_consumer`  | `FrameConsumerWorkerBase::monotonicFrameNs` monotonic-bump semantics, `resetMonotonicClock`, and the move-only `TimestampedFrame` contract |
| `tst_async_combinators` | The `Async::` free-function combinator layer, the real `autoReconnect()` geometric schedule, the 24-step backoff cap, `RetryTask::setPolicy`, dead-sender `SignalTask`, nested trees |
| `tst_connection_flows` | Spec-0034 connection flows with a fake driver and loopback sockets: `DriverOpenTask`, `SocketConnectTask` (phantom-connect guard, dial generations), `SupervisorTask` drop/recover, the flow factories |
| `tst_hal_driver`      | `HAL_Driver.h` default implementations: sync `beginOpen` seam, `abortOpen`, `applyConnectionSettings` routing, `publishReceivedData`, `makeCapturedData` frame-step clamp |
| `tst_xymodem`         | XMODEM/YMODEM sender state machines through the `writeRequested`/`processInput` seam plus `CRC.h` vectors; block framing, ACK/NAK/EOT flows, padding, cancellation, YMODEM batch headers |
| `tst_zmodem`          | ZMODEM sender against a simulated receiver: header/subpacket framing with independently reimplemented CRCs, ZDLE escaping, reposition, cancel paths, session teardown |
| `tst_cframe_parser`   | The Built-In parser engine: catalog/schema/descriptor contracts and a behavior test for every native template (10 text, 13 binary, 2 multi-frame) plus the span fast lane |
| `tst_lua_compat`      | The Lua 5.1/5.2 compat shim on a raw `lua_State`: math/table/string/bit32 surface, native `string.split`, sandboxed console and restricted `os` |
| `tst_js_watchdog`     | `JsWatchdog`/`JsWatchdogThread`: interrupt of runaway scripts within budget, engine reuse after interrupt, per-engine isolation, exception-vs-timeout distinction, worker shutdown |
| `tst_mirror_protocol` | Spec-0040 wire codec: `layoutHash` FNV-1a pins (including fixture cross-checks), `roundSignificant`, structure/snapshot/heartbeat encoding, chunking and reassembly |
| `tst_json_validator`  | `Misc::JsonValidator` security limits: file-size, depth, and array-size boundaries (exact `>` semantics), custom limits, malformed input |
| `tst_enum_labels`     | `API::EnumLabels` slug/label registries: full GPL enumerator sweeps, round-trip inverters, out-of-range fallbacks, dataset-option bitmask conversions |
| `tst_property_validators` | The spec-0036 `PropertyValidators.cpp` set: color, dataset index, FFT window, and transform-language boundary sweeps |
| `tst_password_hash`   | `Misc::PasswordHash`: PBKDF2-SHA256 round-trip and PHC shape, legacy MD5 verification, malformed-hash rejection matrix, salt randomness |
| `tst_legacy_icons`    | `Misc::legacyIconPath`: identity fallback, sampled table pairs, case sensitivity, target-shape checks against the spec-0028 icon tree |

The table above is a **selection**, not the catalog: `ctest -N --test-dir <build>` lists every
registered suite. The suites spec 0075 added:

| Suite | What it covers |
|-------|----------------|
| `tst_frame_builder_staging` | `DataModel::BlockStager` against a stub `BlockStagerHost`: cap flush, epoch flush, the `flushAll` tail, mask split, `structureGeneration` stamping, per-source block numbers, per-sample offsets, pool exhaustion, `releaseIdleStorage` |
| `tst_csv_export_times` | Per-source export time: two sources keep their own instants, a collision bumps only its own source, a uniform grid never takes the tie-break, a session reset clears every source |
| `tst_sessions_export_worker` | The spec-0044 fingerprint layout the write-failure policy depends on: every field participates, blob/string boundaries cannot shift, NaN folds to 0.0 |
| `tst_sessions_loader_index` | One index entry per instant regardless of dataset count, ascending and de-duplicated, and a read-only archive still loads |
| `tst_export_structure` | `DataModel::ExportStructure`: an empty frame never wipes an adopted template, a published structure only fills an empty slot, an open file keeps its schema, a title cannot escape the workspace |
| `tst_replay_playback_engine` | `DataModel::ReplayPlaybackEngine`: seek window, `points()` floor, an untimed row ends the walk, the epoch retires a superseded chain, replay time is the recording's, the catch-up fill is gated, the label never goes negative. **Not** `tst_replay_seek_engine`, which covers a different class (`UI::ReplaySeekEngine`) |
| `tst_lua_deadline_hook` | `LuaDeadlineHook` in both forms: a bounded chunk is untouched, `while true do end` errors at the budget, the message names label and budget, the state is reusable, a never-armed state is never cut off |
| `tst_script_dryrun` | `ScriptDryRun`: JS evaluate and call timeouts with engine reuse, a syntax error is not a timeout, Lua chunk and call timeouts, the sandbox has no `io`/`os` |
| `tst_stream_worker` | A runaway `transform_block` and a runaway per-sample `transform` both time out, fall back to raw and count; an out-of-range channel clears its column |
| `tst_async_tcp_dial` | `IO::AsyncTcpDial` against a live listener, a refusing port and an unresolvable name; cancel emits nothing; probe-disabled dial |
| `tst_connect_fanout` | `ConnectFanOut`: request lifecycle, pending dials, latched state, wait cursor |
| `tst_connection_verdicts` | The verdict matrix over `Test::FakeDriver`: sync ok/fail, async ok/fail, cancel mid-dial, drop with a pending dial, pause policy |
| `tst_uart_policy` | The two pure UART decisions: a custom path's `ResourceError` is fatal; only `ResourceError` feeds auto-reconnect |
| `tst_serial_can_backend` | LAWICEL and analyzer decoders, bitrate tables, fatal-error classification, the bounded receive buffer |
| `tst_playback_ring` | The audio playback SPSC ring: ordering, wrap, underrun, overflow, reset |
| `tst_ethernetip_worker` | `PolledPlcWorkerBase` via a scripted stub: unchanged values cost no wire entry, dirty marks are consumed by the publish, counters accumulate, link loss reported once, one dial verdict per attempt, the abort latch holds |
| `tst_modbus_register_groups` | Both request caps (125 registers / 2000 bits), duplicate and empty refusal, restore ordering, out-of-range drop, JSON shape, function codes, a golden CRC |
| `tst_opcua_security` | Identity reuse, SAN hostname matching, no wildcard against an IP literal, **trust independent of the hostname**, revoke and return, PEM to DER, the plaintext-password default |
| `tst_opcua_frame_assembler` | Per-array-element slots, delta-only encoding, array fan-out, Bad status keeps the last good value, frame splitting with a rotating cursor, one type-mismatch warning per slot |
| `tst_opcua_subscriptions` | Individual vs all-refused monitored items, notifications reaching the cache, what a reset forgets, interval adoption |
| `tst_iec104_slots` | One address under two type ids is two slots; keys never collide across the 24-bit address range; the type id decides the value class |
| `tst_dashboard_ingest` | `UI::DashboardIngest`: the uniform-grid lane, the ring-capacity bound, GPS per block, plot-clock continuation, stale-generation drop, column mismatch |
| `tst_colormap_lut` | The waterfall LUT is a bake of the continuous map; the ends are the extremes; out-of-range clamps |
| `tst_waterfall_tiles` | The quads tile the plot exactly across the ring seam and the band boundaries (shared by both draw paths) |
| `tst_waterfall_ring_texture` | Staging slots and destinations, slot overflow escalating to a full upload, a full upload superseding staged rows, out-of-range and foreign-image rejection, the idle gate, the single-band seam decomposition |
| `tst_value_widget_hasdata` | The nearest-band clamp still resolves 0.0 to a band, and `Bands::reportedSeverity` overrides it while the widget has no data |
| `tst_plot_curve_geometry` | `GpuStroke::reserveGeometry` reallocates 0 times over 100 steady frames (stationary and wobbling counts), capacity never shrinks, index capacity is whole triangles, the padded tail is degenerate |
| `tst_terminal_selection` | Selection clamp, a front erase reporting dropped rows, colour rows trimmed in lockstep |
| `tst_theme_property_map` | The shipped key set, a no-op republish notifies nothing, per-key notify, a dropped key empties |
| `tst_mmcss_registration` | The **per-thread** MMCSS latch: a worker that registers leaves every other thread unregistered, repeat calls are no-ops. Runs on every platform (the latch is recorded everywhere; only the Windows API behind it is skipped) |
| `tst_project_bulk_ops` | The delete-ordering rank: table before folder, group before folder, dataset and output widget before group, workspace before folder, a full mixed selection, descending ids within a rank, strict-weak-ordering algebra |
| `tst_project_history` | The two-phase capture contract, nested scopes as one step, hint-over-slot precedence, the 1 s coalesce window, the 100-step and 64 MiB bounds, redo-tail truncation, save-point survival, the disabled and applying suppressions |
| `tst_client_reception` | The API receive loop under table churn: an entry erased mid-dispatch, the table rehashed mid-dispatch, single-count byte accounting, the HTTP sniff, the raw-forward handshake gate, the `CONSENT_REQUIRED` refusal |
| `tst_path_policy_registry` | Sweeps every command in `app/rcc/api/api-schema.json`: a path-shaped parameter with no declaration fails (reads the snapshot via `SS_API_SCHEMA_PATH`) |
| `tst_server_worker_caps` | A real loopback pair with no event loop: the cap boundary, a response under cap, broadcast and mirror skipped over cap, the response lane dropping the client |
| `tst_grpc_pending_call` | `PendingCall` dispatch, wait, abandon, timeout, and the four-handlers-parked shutdown shape |
| `tst_sse_event_reader` | Frame splitting, CRLF carry-over, `[DONE]`, multi-line data, recoverable vs fatal parse errors |
| `tst_redactor` | Tool-result scrubbing of key, bearer and PEM shapes; ordinary telemetry untouched |
| `tst_sentinel_probe` | Classification, display strip, the compliance state machine, latch restore |
| `tst_file_sandbox` | Read and write roots, traversal, the dropped-path allowlist, search, **and** the async worker lane (generation echo, queued result) |
| `tst_reply_state_machine` | Three provider backends against `FakeTransport`: one `finished` per reply, 401 vs 429, the unified parse policy, the transport policy, redaction |
| `tst_conversation_turn` | The context-window arithmetic: an 8k window budgets negative uncapped and trims when capped; `FakeProvider` event ordering |
| `tst_extension_installer` | A v1 catalog is refused, a bad digest is refused, a verified install, **a corrupt update leaves the installed version intact**, digests recorded, numeric version compare, repository scheme |
| `tst_simplecrypt` | Round trip, wrong key, tamper, no-key refusal, non-deterministic ciphertext |
| `tst_monotonic_clock` | Floor semantics **and** the once-a-minute persist rate |
| `tst_commercial_token` | Seal integrity, post-seal edits invalidate, current-slot transitions |
| `tst_machine_id` / `tst_machine_id_identity` | A persisted fingerprint short-circuits the platform tool spawn (first) and fingerprint stability, digest shape, non-zero cipher key (second) |
| `tst_test_doubles` | Smoke tests over the three shared doubles in `app/tests/support/` |

**Shared test doubles (`app/tests/support/`).** `FakeDriver` is a `HAL_Driver` with scripted
`open()` outcomes (sync ok/fail, async ok/fail after N ms, drop after open) for the verdict
matrix; `FakeProvider` is an `AI::Provider`/`Reply` replaying a scripted stream (text, tool call,
error, budget breach); `FakeTransport` returns canned `QNetworkReply`s. `ss_add_unit_test` skips
a suite whose sources are absent, so a suite that links a double configures quietly either way.

**Fuzz targets (`app/tests/fuzz/`).** `ss_add_fuzz_target()` registers a libFuzzer entry point that
also runs as an ordinary ctest: with `ENABLE_FUZZERS=OFF` (every default configure) it builds the
same `LLVMFuzzerTestOneInput` behind a `QTest` main that replays every seed in
`app/tests/fuzz/corpus/<target>/`, so `ctest -R '^fuzz_'` is the corpus tier on any toolchain; with
`-DENABLE_FUZZERS=ON` (Clang, composable with `-DDEBUG_SANITIZER=ON`) it builds a real fuzzer.
Conventions and the fuzzing command line: [`app/tests/fuzz/README.md`](../app/tests/fuzz/README.md).

**Linter coverage.** `sanitize-commit.py` clang-formats `app/tests/` like the rest of `app/`, so the
100-column, 2-space, pointer-binds-to-type formatting applies. `code-verify.py`'s structural,
comment-style, and semantic rules do **not**: its first-party check matches `app/src`, `core/`,
and `app/qml` only. Test code is held to the formatting contract, not the structural one — do not add
`// code-verify off` fences there on the assumption that the rules fire.

## Sanitizer tier (CI `sanitize` job)

Not a separate suite: the **same** ctest tier, rebuilt instrumented. Two mutually exclusive
builds, both Clang, both `-DSS_USE_MIMALLOC=OFF` (an allocator override and a sanitizer cannot
both own malloc):

- **ASan + UBSan** (`-DDEBUG_SANITIZER=ON -DENABLE_FUZZERS=ON -DSS_INAPP_TESTS=ON`) runs the whole
  ctest tier, then replays every fuzz corpus (`ctest -R '^fuzz_'`), then drives the real parse
  pipeline instrumented (`--benchmark-hotpath --min-fps 1` — the number is meaningless under
  instrumentation, the code path is the point), then the GPL leg of the QML instantiation suite.
  `ASAN_OPTIONS=detect_leaks=0`, because Qt's plugin and QML machinery leaks by design at exit;
  the memory-error half is what the job is for.
- **TSan** (`-DENABLE_TSAN=ON`) builds and runs the unit tier only. The SPSC and
  DirectConnection invariants it proves live in the suites, not in the GUI.

```bash
cmake -G Ninja -B build/asan -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug \
      -DDEBUG_SANITIZER=ON -DSS_BUILD_TESTS=ON -DSS_USE_MIMALLOC=OFF
cmake --build build/asan && ctest --test-dir build/asan --output-on-failure
```

## qmllint (CI `build-linux` job)

`qmllint` needs Qt and a configured tree, so the gate lives in the Linux build job, not in
`lint`. The `ss_qmllint` target's output is compared against the checked-in
`app/qml/qmllint-baseline.json`: an accepted finding passes, a new one fails, and the full output
is uploaded as an artifact. **The baseline ships unseeded** (`"seeded": false`), so today the step
reports and passes; seeding it is one paste plus a flag flip, and the instructions are in the
file. This is the one gate in the tree that is not yet armed.

## In-app self-tests (`--selftest`)

A separate in-app tier (`app/src/SelfTest/`), exposed only when the build is configured with
`-DSS_INAPP_TESTS=ON`. There are now **two registries**:

- **Pre-root suites** run inside `CLI::process()` **before** the composition root, so such a
  suite must never touch an application singleton.
- **Post-root suites** need the composition root, so `CLI::process()` is too early for them:
  `CLI::postRootSelfTestRequested()` defers them and `main()` runs them after
  `bootstrapModuleManager()`, exiting through the same ladder. `SelfTest::Runner::postRootSuiteNames()`
  is the list.

The one post-root suite today is **`qml`** (`--headless --selftest-suite qml`): it builds a
`QQmlEngine` with stub context objects for every registered `Cpp_*` name and instantiates every
file under `qrc:/serial-studio.com/`, failing on a `ReferenceError`. CI runs it twice — in the
Linux build job and again on the GPL build in the sanitize job, because a Pro-only `Cpp_*` global
reached from a shared `.qml` is a `ReferenceError` there and nowhere else.

Distinct from the licensing self-tests (`--selftest-offline-license`, `--validate-guards`).

## Running across multiple categories

```bash
# Parallel execution (4 workers, integration only)
pytest tests/integration/ -n 4

# All tests, skip destructive ones
pytest tests/ -m "not destructive" -v

# With coverage
pytest tests/ --cov=app/src --cov-report=html

# Single test with printed output
pytest tests/integration/test_frame_parsing.py::test_checksum_validation -v -s
```

## xfail policy

An `xfail` is a defect somebody owns, so `@pytest.mark.xfail` needs a `reason=` naming its
tracking reference: a spec finding id (`"spec 0075 I1: ..."`), a GitHub issue (`#1234`), an
upstream bug (`QTBUG-12345`), or a URL. `scripts/tests/test_ci_workflow.py` enforces it and the
`lint` job runs that test.

"By design, not a finding" is not a reason to xfail — it means the test asserts a guarantee the
product deliberately does not make, so **delete the test**. Three security tests carried exactly
that reason until spec 0075 (they asserted that a localhost-only, unauthenticated-by-design API
authenticates); they are gone. When the fix for a tracked xfail lands, the same commit turns the
marker into a plain assertion.

## Markers

```bash
pytest -m "csv"                      # CSV export and player tests
pytest -m "project"                  # Project configuration tests
pytest -m "network"                  # Network driver tests
pytest -m "not slow"                 # Skip slow tests (>10 s)
pytest -m "security and critical"    # Critical security checks only
pytest -m "destructive"              # Tests that may crash or hang the server
```

| Marker        | Meaning |
|---------------|---------|
| `integration` | Requires a running Serial Studio instance |
| `security`    | Security and robustness tests |
| `performance` | Benchmarks |
| `slow`        | Takes a long time |
| `csv`         | CSV export or player tests |
| `project`     | Project configuration tests |
| `network`     | Network driver tests |
| `uart`        | UART driver tests |
| `ble`         | Bluetooth LE driver tests |
| `fuzzing`     | Fuzzing and chaos tests |
| `dos`         | Denial-of-service probes (CI deselects them: `-m "not dos"`) |
| `resource_exhaustion`         | Resource-exhaustion tests |
| `destructive` | May crash or hang the server |
| `critical`    | Critical severity security checks |
| `high`        | High severity security checks |
| `probe`     | Active probing attempts |

## Directory structure

```
tests/
├── integration/                        # Functional tests against the API
│   ├── conftest.py                     # Shared fixtures (api_client, device_simulator, …)
│   ├── test_frame_parsing.py           # All 8 checksum types, JSON/CSV parsing, delimiters
│   ├── test_csv_export.py              # Export enable/disable, timestamps, high-frequency
│   ├── test_csv_parsing_verified.py    # Custom CSV delimiters (comma, semicolon, tab, pipe)
│   ├── test_csv_player.py              # CSV player commands (open, play, pause, seek, status)
│   ├── test_project_configuration.py   # Operation modes, JS parsers, project creation
│   ├── test_project_editor.py          # Add, delete, and duplicate groups, datasets, actions
│   ├── test_project_import_export.py   # project.exportJson, project.loadFromJSON, roundtrip
│   ├── test_project_save.py            # File save/reload, widget settings, layout persistence
│   ├── test_problem_center.py          # problems.* diagnostics: project, link, script findings
│   ├── test_widget_extensions.py       # Widget extension findings, scope rules, no Pro bypass
│   ├── test_connection_diagnostics.py  # diagnostics.* self-checks: reachability, ack-and-poll
│   ├── test_source_mirror.py           # Setup pane → source 0 mirror: bus switch, option edits
│   ├── test_console_configuration.py   # Console settings: echo, timestamps, modes, font, send
│   ├── test_console_ansi_vt100.py      # ANSI/VT100 color codes, cursor sequences, edge cases
│   ├── test_dashboard_configuration.py # FPS, data points, operation mode, status/data queries
│   ├── test_window_layout.py           # Active group, window states, layout, widget settings
│   ├── test_batch_api.py               # Batch command processing, size limits, partial failure
│   ├── test_2d_array_parsing.py        # Multi-frame expansion from 2D array JS parsers
│   ├── test_driver_api_comprehensive.py# Every driver command (UART, Network, BLE, Modbus …)
│   ├── test_new_driver_api.py          # HID, raw USB, Process driver APIs
│   ├── test_api_drivers.py             # Driver switching and console/export basics
│   ├── test_mcp.py                     # MCP JSON-RPC 2.0 protocol (tools, resources, prompts)
│   ├── test_licensing.py               # License status, set/activate/deactivate
│   ├── test_image_view.py              # ImageView widget modes and corrupted data handling
│   ├── test_performance_concepts.py    # Data rate vs. render rate decoupling
│   ├── test_workflows.py               # End-to-end: configure, connect, receive, export
│   └── test_fuzzy.py                   # Malformed JSON, binary garbage, unicode, chaos
│
├── security/                           # Resilience and boundary tests
│   ├── conftest.py                     # Security fixtures (finding_tracker, check_server_alive)
│   ├── test_api_security.py            # JSON probes, injection probes, buffer abuse
│   ├── test_api_weaknesses.py     # Input validation bypass, parsing probes
│   ├── test_resource_exhaustion.py       # CPU/memory/connection exhaustion
│   ├── test_protocol_fuzzing.py        # Malformed protocol messages, encoding confusion
│   ├── test_probe_techniques.py      # Race conditions, integer overflow, timing probes
│   ├── test_unknown_input_boundary.py    # Sandbox escape, prototype pollution, ReDoS
│   ├── test_access_control.py          # Auth bypass, privilege escalation
│   └── run_all_security_tests.sh       # Run all security tests at once
│
├── performance/                        # Benchmarks
│   └── benchmark_frame_rate.py         # Throughput at 10 to 1000 Hz, checksum overhead
│
├── scripts/                            # Unit tests for JS frame-parser scripts
│   ├── conftest.py                     # run_parser() helper + parse_script fixture
│   ├── test_frame_parsers.py           # 28 parser classes (AT, Base64, NMEA, MAVLink …)
│   ├── test_widget_manifests.py        # Spec-0038 widget packages: schema, reserved ids, qrc
│   ├── test_diagnostics_static.py      # Spec-0035 registration drift
│   ├── test_proto_ledger_static.py     # Spec-0037 gRPC field-number ledger
│   └── test_cpp_regressions.py         # Logic-only C++ regression checks
│
├── fixtures/                           # Checked-in inputs the suites read
│   ├── projects/legacy/*.ssproj        # One hand-written document per loader migration
│   ├── sessions/                       # Frozen 4.0.3 archives of both legacy storages
│   └── mirror/                         # Spec-0040 wire fixtures
│
└── utils/                              # Shared test utilities
    ├── api_client.py                   # SerialStudioClient, a TCP API wrapper
    ├── device_simulator.py             # Simulates TCP/UDP devices sending telemetry
    ├── data_generator.py               # Generates frames with checksums (JSON, CSV, fuzzing)
    └── validators.py                   # Assertions for CSV files and frame structures
```

## How tests work

### Integration, security, and performance tests

Every integration test follows the same pattern:

1. **Connect.** `SerialStudioClient` opens a TCP connection to `localhost:7777`.
2. **Configure.** Set operation mode, delimiters, checksums, and JS parser via the API.
3. **Simulate.** `DeviceSimulator` starts a TCP/UDP server on `localhost:9000`.
4. **Stream.** Tell Serial Studio to connect to the simulator and send telemetry frames.
5. **Assert.** Verify frames were parsed, dashboards updated, or files exported.
6. **Cleanup.** Disconnect the device and restore defaults.

The `conftest.py` fixtures handle most of the boilerplate. A typical test only needs `api_client`, `device_simulator`, and `clean_state`.

### Script tests

Script tests exercise the JavaScript frame-parser scripts in `app/rcc/scripts/parser/js/` with no Qt or Serial Studio dependency. Each test calls `run_parser()`, which:

1. Reads the `.js` source file from `app/rcc/scripts/parser/js/`.
2. Appends a one-liner that calls `parse(frame)` and prints the result as JSON.
3. Runs the combined snippet in a fresh Node.js subprocess, so there's no shared state.
4. Deserializes the JSON output and returns it as a Python list.

Because each call spawns a new process, parser scripts that maintain a `parsedValues` array always start from zeros, and tests stay isolated from each other.

## Key fixtures

| Fixture            | Scope    | What it does |
|--------------------|----------|--------------|
| `api_client`       | function | Connected `SerialStudioClient`, auto-disconnects after the test |
| `clean_state`      | function | Disconnects devices, disables exports, creates a fresh project |
| `device_simulator` | function | TCP server on port 9000 that sends frames |
| `data_generator`   | function | Generates JSON/CSV telemetry with checksums |
| `checksum_types`   | session  | All 8 checksum algorithm types |
| `temp_dir`         | function | Temporary directory, cleaned up after the test |

Security tests have additional fixtures in `security/conftest.py`:

| Fixture              | What it does |
|----------------------|--------------|
| `security_client`    | API client that does not reset state between tests |
| `finding_tracker`       | Logs discovered weaknesses for reporting |
| `check_server_alive` | Verifies the server did not crash after a test |

Script tests expose a single fixture in `scripts/conftest.py`:

| Fixture        | What it does |
|----------------|--------------|
| `parse_script` | Returns the `run_parser(script_name, frame)` callable |

## Operation modes

Tests exercise three modes. Knowing which one you're in matters for frame parsing.

| Mode              | ID  | Frame delimiters                   | Parser             |
|-------------------|-----|------------------------------------|--------------------|
| **ProjectFile**   | `0` | Configurable (`/*` `*/` default)   | JavaScript         |
| **DeviceSendsJSON** | `1` | Fixed `/*` `*/`                   | None (full JSON)   |
| **QuickPlot**     | `2` | None (line-based `\n`)             | None (comma CSV)   |

## Writing a new integration test

Here's a minimal example that creates a project, configures it, and parses frames from a simulated device.

```python
import time
import pytest
from utils import ChecksumType, DataGenerator


@pytest.mark.integration
@pytest.mark.project
def test_my_custom_project(api_client, device_simulator, clean_state):
    # 1. Create a project
    api_client.create_new_project(title="My Test")
    time.sleep(0.3)

    # 2. Add structure
    api_client.command("project.group.add", {"title": "Sensors", "widgetType": 0})
    time.sleep(0.2)
    api_client.command("project.dataset.add", {"options": 0})
    time.sleep(0.1)

    # 3. Set JavaScript frame parser
    api_client.command("project.parser.setCode", {
        "code": "function parse(frame) { return frame.split(','); }"
    })
    time.sleep(0.2)

    # 4. Configure frame parser (delimiters and checksum)
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,         # ProjectFile
        frame_detection=1,        # StartAndEndDelimiter
    )
    time.sleep(0.2)

    # 5. Load project into FrameBuilder
    api_client.set_operation_mode("project")
    result = api_client.command("project.loadIntoFrameBuilder")
    assert result["loaded"]

    # 6. Configure network and connect
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # 7. Send frames and wait for processing
    frames = [
        DataGenerator.wrap_frame(
            f"{i * 10.0}",
            mode="project",
            checksum_type=ChecksumType.NONE,
        )
        for i in range(10)
    ]
    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(2.5)

    # 8. Assert
    assert api_client.is_connected()
    api_client.disconnect_device()
```

### Recommended delays

| Situation                               | Delay           |
|-----------------------------------------|-----------------|
| After a single API call                 | `0.1 s`         |
| After project or config changes         | `0.2 to 0.3 s`  |
| Between major operations                | `0.5 s`         |
| After sending frames, before asserting  | `1.0 to 2.5 s`  |

### Generating frames

```python
from utils import DataGenerator, ChecksumType

# JSON frame (DeviceSendsJSON mode)
payload = json.dumps(DataGenerator.generate_json_frame())
frame = DataGenerator.wrap_frame(payload)                          # /* ... */\n

# CSV frame (ProjectFile mode, no checksum)
frame = DataGenerator.wrap_frame("1.0,2.0,3.0", mode="project", checksum_type=ChecksumType.NONE)

# CSV frame with CRC-16 checksum
frame = DataGenerator.wrap_frame("1.0,2.0", mode="project", checksum_type=ChecksumType.CRC16)

# QuickPlot frame (plain newline-delimited CSV)
frame = b"1.0,2.0,3.0\n"
```

## Troubleshooting

**"Connection refused" on port 7777.** Serial Studio is not running, or the API server is not enabled. Check **Settings → Miscellaneous → Enable API Server**.

**Tests hang for 30 seconds then time out.** Port 9000 may be in use by another process, or Serial Studio stopped responding. Run with `pytest -s` to see live output.

**Frames are not being parsed.** Make sure the operation mode matches what the test expects. ProjectFile mode needs a loaded project; QuickPlot ignores project config.

**"Server disconnected" during a test.** The rate limiter (200 msg/s) closed the connection defensively. Increase delays between commands, or use the `clean_state` fixture, which reconnects automatically.

**Import errors.** Run `pip install -r tests/requirements.txt`. CI installs
`tests/requirements.lock` with `--require-hashes` instead; regenerate the lock when you change
the plain requirements file, or the `lint` and `test` jobs install a different set than you did.

**Script tests fail with `node: command not found`.** Install Node.js and make sure `node` is on your `PATH`. Script tests are the only ones that require it.
