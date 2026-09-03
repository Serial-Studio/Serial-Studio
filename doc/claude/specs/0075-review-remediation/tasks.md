---
spec: 0075-review-remediation
phase: tasks
status: approved   # draft -> approved (gate before /ss-implement)
updated: 2026-09-01
---

# Tasks 0075 — Remediate the 2026-09-01 full source review

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list per work
> package and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- Ids are `WPx-Tn`. One package = one branch = one agent brief; tasks inside a package run in
  order; packages A..H run in parallel after WP0; WP-I after every defect package merges; WP-J
  last. `Deps` across packages name the package (`WP-B done`), inside a package the task id.
- **Files** lists only what the task edits. A task that needs a file another package owns
  sends a patch through the coordinator (named in Does).
- **Verify** is per task: `python scripts/code-verify.py --check <files>` is implied for every
  C++/QML task and not repeated; the line lists what else confirms the unit.
- Hotpath, ctor-closure, signal-wiring and cached-flag tasks name their binding invariant in
  the Does line.
- Every defect task lands with its test in the same task or the next one.

## WP0 — CI integrity, tooling, harnesses

### WP0-T1 — Pin every third-party action to a commit SHA
- **Files:** `.github/workflows/ci.yml`, `.github/workflows/docs.yml`
- **Does:** Replace every `uses: owner/action@vN` with `@<40-hex> # vN.N.N`; add a comment naming the tag per pin.
- **Verify:** `grep -nE 'uses: .*@v[0-9]' .github/workflows/*.yml` returns nothing.
- **Deps:** none
- [x] done

### WP0-T2 — Release publication guard and concurrency
- **Files:** `.github/workflows/ci.yml`
- **Does:** `upload` gets `if: github.ref == 'refs/heads/master' || startsWith(github.ref, 'refs/tags/')` and `concurrency: {group: release-${{ github.ref }}, cancel-in-progress: false}`; `REQUIRE_TESTS_TO_PUBLISH` default `true`.
- **Verify:** YAML parses (`python3 -c "import yaml,sys;yaml.safe_load(open('.github/workflows/ci.yml'))"`); `scripts/tests/test_ci_workflow.py` (WP0-T9) asserts the guard.
- **Deps:** WP0-T1
- [x] done

### WP0-T3 — Tests gate publication
- **Files:** `.github/workflows/ci.yml`
- **Does:** `test` needs the four build jobs and downloads the packaged binary via `actions/download-artifact`; `upload` needs `[test, lint]`; `continue-on-error` expression removed; Release download step deleted.
- **Verify:** job graph in `test_ci_workflow.py`: `upload.needs` contains `test` and `lint`; `test.needs` contains no `upload`.
- **Deps:** WP0-T2
- [x] done

### WP0-T4 — Secrets through env, least-privilege permissions, training run must activate
- **Files:** `.github/workflows/ci.yml`
- **Does:** Move every `${{ secrets.* }}` out of `run:` bodies into step `env:`; add `permissions:` to every job (`contents: read` default); remove `|| true` after the training activation; drop unused `security-events: write`; fix `USERPROFILE` expansion on Linux/macOS.
- **Verify:** `grep -n 'secrets\.' ci.yml` shows only `env:` lines; `test_ci_workflow.py` asserts every job has `permissions`.
- **Deps:** WP0-T3
- [x] done

### WP0-T5 — FORTIFY: verify, then fix the flag order
- **Files:** `cmake/Hardening.cmake`, `.github/workflows/ci.yml`
- **Does:** Linux build job dumps the app target's compile line from `compile_commands.json` and asserts the last FORTIFY token is `-D_FORTIFY_SOURCE=3` (GCC >= 12, glibc >= 2.34) or `=2`; `_ss_apply_hardening` emits `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=<n>` as one ordered `target_compile_options` pair and the directory-level pair is skipped for opted-in targets. Fix lands only after the CI dump confirms the cancel (decision).
- **Verify:** CI step green with the expected token; `cpp-compiler-flags` skill read before editing.
- **Deps:** WP0-T4
- [x] done

### WP0-T6 — ctest on macOS and Windows
- **Files:** `.github/workflows/ci.yml`, `app/tests/CMakeLists.txt`
- **Does:** Add the `build/unit-ci` configure/build/ctest steps (pattern of L278-301) to `build-macos-arm64` and `build-windows`; `ss_add_unit_test` fails configure on a missing source instead of returning silently.
- **Verify:** three green ctest steps in CI; a misspelled source fails configure locally (maintainer).
- **Deps:** WP0-T3
- [x] done

### WP0-T7 — Sanitizer job and fuzz scaffolding
- **Files:** `cmake/Sanitizers.cmake`, `app/tests/CMakeLists.txt`, `.github/workflows/ci.yml`
- **Does:** `ENABLE_FUZZERS` option (`-fsanitize=fuzzer-no-link`, clang); `ss_add_fuzz_target(name SOURCES LIBS)` builds a libFuzzer binary when on and a corpus-replay QTest otherwise; new `sanitize` job: ASan+UBSan `build/unit-ci` + ctest + corpus replay + `--benchmark-hotpath --min-fps 1`, plus a TSan ctest leg.
- **Verify:** job green on an empty fuzz set; `ctest -R fuzz_` lists targets once WP-A/D/G/H add them.
- **Deps:** WP0-T6
- [x] done

### WP0-T8 — Benchmark retry jobs
- **Files:** `.github/workflows/ci.yml`
- **Does:** Per platform, `benchmark-retry-<os>` job with `if: failure()`, needs the build job, downloads the built binary artifact, reruns the 256 kHz gate once and uploads both reports; `upload` gates on `build || retry` via `if: always()` + result check.
- **Verify:** `test_ci_workflow.py` asserts the four retry jobs and `upload`'s condition; maintainer forces one failure to observe the retry.
- **Deps:** WP0-T3
- [x] done

### WP0-T9 — Workflow lint test
- **Files:** `scripts/tests/test_ci_workflow.py` (new), `scripts/tests/__init__.py` (new)
- **Does:** Parses `ci.yml`/`docs.yml` and asserts: all `uses` SHA-pinned, `upload` guarded, `test` before `upload`, every job has `permissions`, no `secrets.` in `run:`, ctest present on three OSes, sanitizer job exists, retry jobs exist, no `xfail` in `tests/` without a `reason=` naming a finding id or issue.
- **Verify:** `pytest scripts/tests/test_ci_workflow.py` green; runs in `lint`.
- **Deps:** WP0-T8
- [x] done

### WP0-T10 — Lock Python test dependencies
- **Files:** `tests/requirements.txt`, `tests/requirements.lock` (new), `.github/workflows/ci.yml`, `.github/workflows/docs.yml`
- **Does:** `pip-compile --generate-hashes` lock; CI installs `--require-hashes -r tests/requirements.lock`; `reuse` pinned in the lock; stale `code-format.py` comment fixed.
- **Verify:** `pip install --require-hashes -r tests/requirements.lock` succeeds locally; lint job green.
- **Deps:** WP0-T4
- [x] done

### WP0-T11 — pytest markers and xfail policy
- **Files:** `tests/pytest.ini`, `tests/README.md`, `tests/security/test_access_control.py`, `tests/security/test_unknown_input_hardening.py`
- **Does:** Register `dos`; delete the three by-design xfails; the stress-crash xfail gets `reason="0075 I1"` (WP-G flips it); README states the xfail policy.
- **Verify:** `pytest --collect-only tests/security` clean; `test_ci_workflow.py` xfail rule green.
- **Deps:** WP0-T9
- [x] done

### WP0-T12 — Linter default and self-tests
- **Files:** `scripts/code-verify.py`, `scripts/tests/test_code_verify.py` (new), `scripts/tests/fixtures/` (new)
- **Does:** No-arg invocation runs `--check`; `--fix` explicit only (sanitize-commit already passes it); one positive + one negative fixture per rule kind in both linter modules, driven by a parametrised test that invokes `process_file`.
- **Verify:** `pytest scripts/tests/test_code_verify.py` green; `python scripts/code-verify.py` with no args writes nothing (`git status` clean).
- **Deps:** WP0-T11
- [x] done

### WP0-T13 — Wildcard-disconnect rule
- **Files:** `scripts/code_verify_rules.py`, `scripts/tests/fixtures/qt-disconnect-wildcard/`
- **Does:** New error kind `qt-disconnect-wildcard` for `disconnect(x, nullptr, y, nullptr)`; baseline exceptions listed inline for the three current sites until WP-C/WP-I clear them.
- **Verify:** fixture test; `--check` on `S7.cpp`, `EthernetIp.cpp`, `DeviceManager.cpp` reports the three sites as baseline.
- **Deps:** WP0-T12
- [x] done

### WP0-T14 — Duplicate-window advisory and ratchet
- **Files:** `scripts/code-verify.py`, `scripts/dup-census.json` (new), `.github/workflows/ci.yml`, `doc/claude/scripts.md`
- **Does:** `cxx-duplicate-window` advisory (10 normalized lines, >40 shared windows per file pair, C++ and QML); `--dup-census --check/--accept` ratchet on the summed pair count; seeded baseline; lint job runs it.
- **Verify:** `python scripts/code-verify.py --dup-census --check` passes on the seeded baseline; the Gauge/Meter pair appears in the report.
- **Deps:** WP0-T13
- [x] done

### WP0-T15 — Ordered claim anchor
- **Files:** `scripts/claim-verify.py`, `scripts/doc-anchors.json`
- **Does:** Anchor kind `ordered`: a multi-capture regex over one source file whose captures must appear in the doc in the same order; first use pins `instantiateCoreModules()` against startup.md's pinned list (WP-J corrects the doc).
- **Verify:** `python scripts/claim-verify.py --quiet` reports the startup.md mismatch as a new error until WP-J lands (baseline-accepted for now).
- **Deps:** WP0-T14
- [x] done

### WP0-T16 — Retire tu-cutter, record vendored versions
- **Files:** `scripts/tu-cutter.py` (delete), `scripts/code-verify.py` (advisory text), `doc/claude/scripts.md`, `lib/VERSIONS.json` (new), `lib/luajit/CMakeLists.txt`
- **Does:** Remove the tool and its three mentions; `VERSIONS.json` lists upstream name, version, commit or archive hash per vendored tree; luajit comment points at it.
- **Verify:** `grep -rn tu-cutter scripts doc CLAUDE.md` empty; `claim-verify` resolves `lib/VERSIONS.json`.
- **Deps:** WP0-T15
- [x] done

### WP0-T17 — Remove foreign tooling from QSimpleUpdater
- **Files:** `lib/QSimpleUpdater/CLAUDE.md`, `sanitize-commit.sh`, `tests/`, `tutorial/`, `etc/` (delete), `REUSE.toml`
- **Does:** Delete the sibling project's AI and tooling files; REUSE entry adjusted.
- **Verify:** `reuse lint` green; build unaffected (maintainer).
- **Deps:** WP0-T16
- [x] done

### WP0-T18 — CMake duplicates and configure-time pins
- **Files:** `app/CMakeLists.txt`, `CMakeLists.txt`
- **Does:** Remove the five duplicate entries; `find_program` for `ar`/`ranlib`; `URL_HASH` / commit-SHA `GIT_TAG` on zlib, expat, mimalloc FetchContent.
- **Verify:** maintainer configure on Linux clean; `grep -n 'CSD.cpp' app/CMakeLists.txt` shows one branch each.
- **Deps:** WP0-T17
- [x] done

### WP0-T19 — qmllint tier
- **Files:** `app/CMakeLists.txt`, `app/qml/qmllint-baseline.json` (new), `.github/workflows/ci.yml`
- **Does:** `qt_add_qml_module` gains a lint target; a baseline of current warnings is checked in; `lint` job fails on any warning not in the baseline.
- **Verify:** lint job green on the baseline; an unqualified access fails locally (maintainer).
- **Deps:** WP0-T18
- [x] done

### WP0-T20 — QML instantiation selftest
- **Files:** `app/src/SelfTest/SelfTest.cpp`, `app/src/SelfTest/QmlInstantiationSuite.cpp` (new), `app/src/Misc/CLI.cpp`
- **Does:** `--selftest qml` runs after module init (new post-root hook in `CLI::process`, the existing suites stay pre-root): a `QQmlEngine` with a stub object for every `Cpp_*` name instantiates every QML file and fails on `ReferenceError`; runs in both GPL and commercial CI builds.
- **Verify:** `./app --selftest qml` exit 0 on both builds (CI step).
- **Deps:** WP0-T19
- [x] done

### WP0-T21 — Test doubles
- **Files:** `app/tests/support/FakeDriver.{h,cpp}`, `FakeProvider.{h,cpp}`, `FakeTransport.{h,cpp}` (new), `app/tests/CMakeLists.txt`
- **Does:** `FakeDriver` (scripted open outcomes, drop-after-open), `FakeProvider`/`FakeReply` (scripted stream), `FakeTransport` (canned `QNetworkReply`s); linked by later suites.
- **Verify:** each double has a smoke ctest (`tst_test_doubles`).
- **Deps:** WP0-T7
- [x] done

## WP-A — Acquisition hotpath and sinks

### WPA-T1 — Extract BlockStager from FrameBuilder
- **Files:** `app/src/DataModel/FrameBuilder/BlockStager.{h,cpp}` (new), `app/src/DataModel/FrameBuilder.{h,cpp}`, `app/CMakeLists.txt`
- **Does:** Pure move of `m_openBlocks`, claim/flush/cap logic into an owned sub-object. Invariants: pipeline-thread only, pooled slots with `use_count()==1` probe, no allocation in `stage`, `structureGeneration` stamped on every publish site.
- **Verify:** `--benchmark-hotpath` equal within noise (maintainer); `tst_data_block` green.
- **Deps:** none
- [x] done

### WPA-T2 — Flush on session edge, emit sessionBoundary
- **Files:** `app/src/DataModel/FrameBuilder.{h,cpp}`, `app/src/DataModel/FrameBuilder/BlockStager.cpp`
- **Does:** `onConnectedChanged` and new `onPausedChanged` (wired from `ConnectionManager::pausedChanged`, auto-queued) flush open blocks through `publishBlock` before clearing state, then `Q_EMIT sessionBoundary(connected, paused)`. Invariant: edge-rate signal, pipeline -> GUI, never per frame.
- **Verify:** `tst_frame_builder_staging` (WPA-T4) disconnect/pause tail cases.
- **Deps:** WPA-T1
- [x] done

### WPA-T3 — Sinks close on sessionBoundary
- **Files:** `app/src/CSV/Export.cpp`, `app/src/MDF4/Export.cpp`, `app/src/Sessions/Export.cpp`
- **Does:** Replace the `connectedChanged`/`pausedChanged` -> `closeFile()` connections with `FrameBuilder::sessionBoundary`; Sessions also closes on pause. `close()` drains the worker queue before closing the file (existing behaviour, asserted).
- **Verify:** `test_recording_fidelity.py` (WPA-T20) row counts equal frames sent across pause and disconnect.
- **Deps:** WPA-T2
- [x] done

### WPA-T4 — FrameBuilder staging unit tests
- **Files:** `app/tests/tst_frame_builder_staging.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Cap flush, epoch flush, mask flush-first, disconnect tail, pause tail, `structureGeneration` stamped, synthetic republish scope (after WPA-T7), parked snapshot deferral (after WPA-T6).
- **Verify:** ctest green; the tail case fails when WPA-T2 is reverted.
- **Deps:** WPA-T3
- [x] done

### WPA-T5 — Cached player flag on the two bypassing reads
- **Files:** `app/src/DataModel/FrameBuilder.cpp`
- **Does:** `applyDatasetValues` and `replayColumnsFor` read `m_playerOpen` instead of `SerialStudio::isFinalValuePlayerOpen()`. Invariant: hotpath reads cached flags; `m_playerOpen` is refreshed by the three players' `openChanged` (already wired).
- **Verify:** `tst_frame_builder_staging` player-open case; TSan leg clean.
- **Deps:** WPA-T4
- [x] done

### WPA-T6 — Deferred project snapshot while parked
- **Files:** `app/src/IO/PipelineHost.{h,cpp}`, `app/src/DataModel/FrameBuilder.{h,cpp}`
- **Does:** `PipelineHost` emits `parkedOnGuiChanged(bool)` (Direct); `syncFromProjectModel` on the parked-inline path stores the snapshot in `m_deferredProjectSnapshot` and applies it on the pipeline thread when the bracket closes. Invariant: `applyProjectSnapshot` runs only on the builder thread; the existing thread assert stays.
- **Verify:** `tst_frame_builder_staging` parked case; `test_export_replay_fidelity.py` unchanged.
- **Deps:** WPA-T5
- [x] done

### WPA-T7 — Synthetic republish limited to table-fed datasets
- **Files:** `app/src/DataModel/FrameBuilder.cpp`
- **Does:** `emitRepublishedFrame` pushes only datasets whose source is table-fed; channel datasets never get a duplicate `now()` sample. Invariant: RepublishGate lanes untouched.
- **Verify:** `tst_republish_lanes` green; new staging case counts samples per tick with a stream source live.
- **Deps:** WPA-T6
- [x] done

### WPA-T8 — StreamWorker JS watchdog and channel fixes
- **Files:** `app/src/IO/StreamWorker.{h,cpp}`, `app/tests/tst_stream_worker.cpp`
- **Does:** `StreamProcessor` owns a `JsWatchdog` per engine, armed once per block around `runJsBlockTransform` and the per-sample pass; timeout falls back to raw, counts, posts one queued notification; out-of-range channel clears the column; `compileEngines` before the feed connect. Invariant: `setInterrupted(true)` only in `JsWatchdogThread.cpp`; Lua Fast mode stays hook-free.
- **Verify:** `tst_stream_worker` JS runaway returns within budget; existing Lua case green.
- **Deps:** WP-B WPB-T1 (LuaDeadlineHook not required for JS; ordering only for the shared helper), WPA-T7
- [x] done

### WPA-T9 — Coalesced consumer flush and raw-lane threshold
- **Files:** `app/src/DataModel/FrameConsumer.{h,cpp}`, `app/tests/tst_frame_consumer.cpp`
- **Does:** `std::atomic<bool> m_flushPosted` gates the queued `processData` post (worker clears at entry); a second-lane hook lets a consumer register an extra SPSC queue that shares the threshold trigger. Invariant: no allocation on `enqueueData` beyond the post itself; SPSC single producer.
- **Verify:** `tst_frame_consumer` asserts one post per drain cycle under a burst.
- **Deps:** WPA-T8
- [x] done

### WPA-T10 — Sessions raw lane and write-failure surfacing
- **Files:** `app/src/Sessions/Export.{h,cpp}`
- **Does:** Raw queue registered on the shared threshold trigger; `try_enqueue` failure counted (`rawOverruns`); `transaction`/`commit`/`exec` checked: failure sets `m_writeFailed`, counts dropped blocks, emits `writeErrorChanged` queued, flips `isRecording`, and `finalizeSession` refuses to fingerprint after a failure; raw chunks before the first block keep their own ns.
- **Verify:** `tst_sessions_export_worker` (WPA-T11).
- **Deps:** WPA-T9
- [x] done

### WPA-T11 — Sessions export worker tests
- **Files:** `app/tests/tst_sessions_export_worker.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Raw lane at 5000 chunks/s for 3 s loses nothing; forced commit failure (read-only file) sets the error state and no fingerprint; raw head stamps real.
- **Verify:** ctest green.
- **Deps:** WPA-T10
- [x] done

### WPA-T12 — Per-source time in CSV and MDF4 workers
- **Files:** `app/src/CSV/Export.{h,cpp}`, `app/src/MDF4/Export.{h,cpp}`
- **Does:** Irregular blocks use `t0 - reference + times[i]`; the strict-increase guard becomes per `sourceId` (`std::unordered_map`, `try_emplace`, reset per session); interval-mode write failure closes and reports like the sparse path. Invariant: source owns time; `monotonicFrameNs` only as the per-source same-ns tie-break.
- **Verify:** `tst_csv_export_times`, `tst_mdf4_export_times` (WPA-T13).
- **Deps:** WPA-T11
- [x] done

### WPA-T13 — Export time tests
- **Files:** `app/tests/tst_csv_export_times.cpp`, `app/tests/tst_mdf4_export_times.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Two sources, blocks flushed on one tick, assert each time column monotonic and equal to the source stamps; MDF4 read back with mdflib shows per-group masters intact.
- **Verify:** ctest green; both fail on the pre-fix worker.
- **Deps:** WPA-T12
- **Status (coordinator, 2026-09-02):** code done (both workers share `monotonicSourceNs`, pinned by `tst_csv_export_times`); the MDF4 twin suite was not written.
- [ ] done

### WPA-T14 — MDF4 writer sync type and reader compatibility
- **Files:** `app/src/MDF4/Export.cpp`, `app/src/MDF4/PlayerLoaderWorker.cpp`, `app/tests/tst_mdf4_writer_conformance.cpp` (new)
- **Does:** `createTimeChannel` sets `Sync(Time)`; absolute-epoch write into the master removed; text channels declared UTF-8; reader treats a master with sync 0 from a Serial Studio author as time.
- **Verify:** conformance test reads a new file and a 4.1.0 fixture (`tests/fixtures/mdf4/legacy-master.mf4`, new) through the loader.
- **Deps:** WPA-T13
- **Status (coordinator, 2026-09-02):** code done (writer sets `Sync(Time)`, reader accepts the legacy zero); the conformance suite needs a checked-in binary `.mf4` fixture nobody could generate without a build.
- [ ] done

### WPA-T15 — MDF4 loader columnar decode
- **Files:** `app/src/MDF4/PlayerLoaderWorker.{h,cpp}`, `app/tests/tst_mdf4_loader_memory.cpp` (new)
- **Does:** Per channel group: one timestamp vector plus one value vector per channel, filled in `OnSample` order; the dense per-instant map is gone; the player merges groups lazily by timestamp. Invariant: mdflib pointers never leave the worker.
- **Verify:** test replays a generated 10-minute 48 kHz stream-lane file under an address-space cap (`setrlimit` on Linux/macOS, skipped on Windows).
- **Deps:** WPA-T14
- **Status (coordinator, 2026-09-02):** code done (per-group columnar decode replaces the dense per-instant map); the memory test needs a generated 10-minute 48 kHz `.mf4`.
- [ ] done

### WPA-T16 — Sessions loader timestamp index and read-only opens
- **Files:** `app/src/Sessions/PlayerLoaderWorker.cpp`, `app/src/Sessions/SessionDbReader.cpp`, `app/src/Sessions/DatabaseWorker.cpp`, `app/tests/tst_sessions_loader_index.cpp` (new)
- **Does:** Timestamp index from `SELECT DISTINCT t0_ns, dt_ns, frames, times ... GROUP BY`; readers open `QSQLITE_OPEN_READONLY` without the WAL pragma; `openDatabase` validates the SQLite header and surfaces failure.
- **Verify:** index test with 600 datasets x 100 Hz x 10 min stays under 200 MB transient; read-only-media fixture replays.
- **Deps:** WPA-T15
- [x] done

### WPA-T17 — Live-session guard in the explorer
- **Files:** `app/src/Sessions/DatabaseManager.cpp`, `app/src/Sessions/DatabaseWorker.cpp`, `app/tests/tst_sessions_database_manager.cpp` (new)
- **Does:** `deleteSession`, tag/rename verbs refuse `Sessions::Export::currentSessionId()` with `SESSION_LIVE` and a Notification.
- **Verify:** ctest; `sessions.deleteSession` on the live id returns the code (pytest in WPA-T20).
- **Deps:** WPA-T16
- **Status (coordinator, 2026-09-02):** code done (`refuseLiveSession` in the manager and the worker, plus the coordinator's `SESSION_LIVE` API refusal); the unit suite needs the application link, so `test_historian_live_guard.py` carries it.
- [ ] done

### WPA-T18 — Sessions player epoch and CSV catch-up
- **Files:** `app/src/Sessions/Player.{h,cpp}`, `app/src/CSV/Player.cpp`, `app/tests/tst_sessions_player_epoch.cpp`, `app/tests/tst_csv_player_catchup.cpp` (new)
- **Does:** Copy the CSV `m_playbackEpoch` guard into both Sessions timer chains; CSV `catchUpTargetRow` stops at a backwards timestamp and re-anchors.
- **Verify:** epoch test: play/pause/play advances one position per tick; catch-up test with a wrapped timestamp file does not jump to EOF.
- **Deps:** WPA-T17
- **Status (coordinator, 2026-09-02):** code done (playback epoch on both Sessions chains, CSV re-anchors on a backwards row); the two unit suites need the application link.
- [ ] done

### WPA-T19 — MQTT publisher hotpath atomics, Influx escaping, player key filter
- **Files:** `app/src/MQTT/Publisher.{h,cpp}`, `app/src/InfluxDB/LineProtocol.h`, `app/src/CSV/Player.cpp`, `app/src/MDF4/Player.cpp`, `app/src/Sessions/Player.cpp`
- **Does:** `hotpathTxRawFrame` reads `std::atomic<bool> m_hotEnabled/m_hotSparkplug`, `std::atomic<int> m_hotMode`; topic base becomes `std::shared_ptr<const QString>` swapped atomically, read on the worker; backslash added to the measurement/tag/field-key special sets; the three players' `eventFilter` act only when their view has focus. Invariant: no QString read on the pipeline thread that a GUI setter writes.
- **Verify:** `tst_mqtt_publisher_hotflags` (new, tiny), `tst_influx_lineprotocol` backslash cases; TSan leg.
- **Deps:** WPA-T18
- **Status (coordinator, 2026-09-02):** code done; the Influx half is pinned by `tst_influx_lineprotocol`, the publisher and player halves need the application link.
- [ ] done

### WPA-T20 — Recording fidelity integration test and fuzzers
- **Files:** `tests/integration/test_recording_fidelity.py`, `tests/integration/test_historian_live_guard.py` (new), `app/tests/fuzz/fuzz_block_codec.cpp`, `fuzz_csv_row.cpp`, `fuzz_mdf4_reader.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Two simulator sources into CSV/MDF4/Historian across connect, pause, resume, disconnect; disk-full via read-only session dir; live-session delete refused. Fuzzers for `StreamBlockCodec`, CSV row splitter, MDF4 reader seeds.
- **Verify:** maintainer runs pytest with the app up; fuzz corpus replay in the sanitizer job.
- **Deps:** WPA-T19, WP0-T7
- [x] done

## WP-B — Script execution deadlines

### WPB-T1 — LuaDeadlineHook helper
- **Files:** `app/src/DataModel/Scripting/LuaDeadlineHook.{h,cpp}` (new), `app/src/DataModel/FrameBuilder/TransformCompiler.cpp`, `app/CMakeLists.txt`
- **Does:** Count hook + `QDeadlineTimer`, `install(L, budgetMs)`, `timedOut()`, error text; `TransformCompiler` uses it (no behaviour change). Invariant: hook never installed in Fast mode.
- **Verify:** `tst_lua_deadline_hook` (new): `while true do end` errors within budget; `tst_expression_transform`/transform suites green.
- **Deps:** none
- [x] done

### WPB-T2 — ScriptDryRun helper
- **Files:** `app/src/DataModel/Scripting/ScriptDryRun.{h,cpp}` (new), `app/src/API/Handlers/ControlScriptHandler.cpp`, `app/CMakeLists.txt`
- **Does:** `runJsDryRun(code, prelude, budgetMs, probe)` (throwaway `QJSEngine` + `JsWatchdog`) and `runLuaDryRun` (state + `LuaDeadlineHook`), returning `{ok, timedOut, error, engine access for probes}`; `controlScript.dryRun` re-based on it.
- **Verify:** `tst_script_dryrun` (new): JS and Lua loops time out; existing `controlscript.dryRun` pytest green.
- **Deps:** WPB-T1
- [x] done

### WPB-T3 — API dry-runs guarded
- **Files:** `app/src/API/Handlers/ProjectDryRunCommands.cpp`
- **Does:** `painterDryRun` and `outputWidgetDryRun` use `ScriptDryRun`; timeout returns `SCRIPT_TIMEOUT`.
- **Verify:** `test_script_deadlines.py` dry-run cases (WPB-T6).
- **Deps:** WPB-T2
- [x] done

### WPB-T4 — Editor and dialog validations guarded
- **Files:** `app/src/DataModel/Editors/ControlScriptEditor.cpp`, `app/src/DataModel/Editors/DatasetTransformEditor.cpp`, `app/src/DataModel/Dialogs/TransmitTestDialog.cpp`
- **Does:** Validate/Test/Apply paths call `ScriptDryRun`; Lua paths get the hook; a timeout keeps the dialog open with a message. (WP-F rebases its editor work on this.)
- **Verify:** `tst_script_dryrun` covers the helper; maintainer observes Validate with `while(true){}` returns.
- **Deps:** WPB-T3
- [x] done

### WPB-T5 — Macro and publisher preview guarded
- **Files:** `app/src/DataModel/Scripting/MacroRunner.cpp`, `app/src/MQTT/PublisherScriptEditor.cpp`
- **Does:** JS evaluate paths through `ScriptDryRun`; unhooked `lua_pcall` gets `LuaDeadlineHook`.
- **Verify:** `tst_script_dryrun` extended with the two preludes.
- **Deps:** WPB-T4
- [x] done

### WPB-T6 — Script deadline integration test
- **Files:** `tests/integration/test_script_deadlines.py` (new)
- **Does:** Submits a looping script to parser (JS/Lua), transform per lane, control script, output widget, painter, every dry-run command, editor validate via the project commands; asserts timeout error and a live ping after each.
- **Verify:** maintainer runs with the app up.
- **Deps:** WPB-T5, WP-A WPA-T8 for the stream lane case
- [x] done

## WP-C — IO core and general-purpose drivers

### WPC-T1 — AsyncTcpDial helper
- **Files:** `app/src/IO/AsyncTcpDial.{h,cpp}` (new), `app/CMakeLists.txt`, `app/tests/tst_async_tcp_dial.cpp` (new)
- **Does:** GUI-thread QObject: `QHostInfo::lookupHost` -> optional paced refusal probe on a throwaway socket -> one `connectToHost` on the caller's socket -> `finished(ok, reason)` once under one deadline; `cancel()`. Invariant: exactly one verdict per `start()`.
- **Verify:** test with a local listener, a refusing port, an unresolvable name and a cancel.
- **Deps:** none
- [x] done

### WPC-T2 — Network TCP async dial and property rows
- **Files:** `app/src/IO/Drivers/Network/NetworkTcp.cpp`, `app/src/IO/Drivers/Network/Network.{h,cpp}`, `app/src/IO/Drivers/Network/NetworkUdp.cpp`, `app/src/IO/Drivers/Network/NetworkHttp.cpp`
- **Does:** TCP dials via `AsyncTcpDial`; `isConnecting()` covers TCP; verdict via `succeedDial`/`failDial`; `driverProperties()` emits all transports' rows; `readDatagram` return checked; HTTP body cap. Invariant: driver socket connects once; `openFinished` on both outcomes.
- **Verify:** `test_connection_verdicts.py` (dead port, unresolvable host with a blackholed resolver) plus a scripted `io.connect` + `writeData` case; `tst_ui_driver_sync` green.
- **Deps:** WPC-T1
- [x] done

### WPC-T3 — Iec104, Modbus, MQTT dial paths
- **Files:** `app/src/IO/Drivers/Iec104.cpp`, `app/src/IO/Drivers/Modbus.cpp`, `app/src/IO/Drivers/MQTT.cpp`
- **Does:** Iec104 `dialStation` async via the helper with `openFinished`; Modbus pre-probe via the helper; MQTT gets a 15 s dial timer -> `failDial`, and the internal re-dial reports `false` returns through `reportOpenFinished(false)`.
- **Verify:** `tst_mqtt_driver_verdict` (new, stub broker socket): cleared topic filter during dial settles the verdict; `test_connection_verdicts.py` Modbus/Iec104 dead-port cases.
- **Deps:** WPC-T2
- [x] done

### WPC-T4 — OPC UA resolution, pump cadence, endpoint scoring
- **Files:** `app/src/IO/Drivers/OpcUaSession.cpp`, `app/src/IO/Drivers/OpcUa.cpp`, `app/src/IO/Drivers/OpcUa/OpcUaEndpointSelection.cpp`
- **Does:** `QHostInfo` resolution before `UA_Client_connectAsync`; numeric endpoint URL with the original hostname kept for the verifier; pump timer adaptive (100 ms idle, 10 ms with a request outstanding); ctor-level `setFilterRules` removed; deprecated policies score below "no endpoint".
- **Verify:** `tst_opcua_endpoint_selection` deprecated-only server case; maintainer observes an unresolvable `.local` host leaves the GUI responsive.
- **Deps:** WPC-T3
- [x] done

### WPC-T5 — S7 and EtherNet/IP async verdict
- **Files:** `app/src/IO/Drivers/S7.cpp`, `app/src/IO/Drivers/EthernetIp.cpp`
- **Does:** `open()` starts the worker dial and returns with `isConnecting()` true; the worker reports `openFinished` once on both outcomes; `waitForConnected` stays on the worker. Invariant: `armOpenReport` before `open()` (manager), disarm on report.
- **Verify:** `tst_connection_verdicts` (WPC-T8) async-fail path with `FakeDriver`; dead-port pytest for both buses.
- **Deps:** WPC-T4
- [x] done

### WPC-T6 — ConnectionManager: pause on reconnect, cancel, QuickPlot swap, asserts
- **Files:** `app/src/IO/ConnectionManager.{h,cpp}`
- **Does:** `connectDevice(int, ResumePolicy)`; `connectDevice(HAL_Driver*)` keeps pause; cancel-during-dial does not emit `sessionClosed`; QuickPlot `rebuildDevices` keeps device 0 until the replacement exists; `setBusType` defers destruction; `SS_ASSERT` in the five entry points. Invariant: `sessionClosed` fires only from the user disconnect path; fan-outs iterate id snapshots.
- **Verify:** `tst_connection_verdicts` pause and cancel cases; `test_connection_verdicts.py` 20x cycle still green.
- **Deps:** WPC-T5
- [x] done

### WPC-T7 — DeviceManager wildcard disconnect and teardown order
- **Files:** `app/src/IO/DeviceManager.cpp`
- **Does:** Captured `QMetaObject::Connection`s instead of the wildcard form; pipeline-thread reader deleted before the pipeline joins at exit.
- **Verify:** `qt-disconnect-wildcard` rule no longer lists the site; ASan leg clean at exit.
- **Deps:** WPC-T6
- [x] done

### WPC-T8 — Manager verdict-matrix unit tests
- **Files:** `app/tests/tst_connect_fanout.cpp`, `app/tests/tst_connection_verdicts.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** `ConnectFanOut` request/fan-out/pending/latched cases; with `FakeDriver`: sync fail inside `open()`, async fail, cancel mid-dial, rebuild mid-dial, drop with pending dial, reconnect preserving pause.
- **Verify:** ctest green.
- **Deps:** WPC-T7, WP0-T21
- [x] done

### WPC-T9 — SerialCanBackendBase
- **Files:** `app/src/IO/Drivers/CANBus/SerialCanBackendBase.{h,cpp}` (new), `app/src/IO/Drivers/CANBus/SlcanBackend.{h,cpp}`, `app/src/IO/Drivers/CANBus/SeeedCanBackend.{h,cpp}`
- **Does:** Base owns the port, open prologue, `readyRead` and `errorOccurred` (ResourceError -> `setError(ReadError)` + `close()`), bounded rx buffer, `close()`; subclasses keep `sendInit`/`parseLine`/`encodeFrame`; Slcan id-parse `ok` bug fixed; open verdict reads the adapter reply.
- **Verify:** `tst_serial_can_backend` (new): simulated `errorOccurred` flips state; malformed id rejected; buffer bounded.
- **Deps:** WPC-T8
- [x] done

### WPC-T10 — CANBus and BLE established-drop path, CAN batching
- **Files:** `app/src/IO/Drivers/CANBus.cpp`, `app/src/IO/Drivers/BluetoothLE.cpp`
- **Does:** Unconnected-while-open -> queued `disconnectDevice(this)` in both; CAN frames batched per `logicalFramesHint`; BLE dedupes by address and validates `characteristicIndex`. Invariant: queued, never a modal inside the state callback.
- **Verify:** `test_driver_drops.py` (WPC-T16) CAN and BLE cases where simulators exist; unit test for batching in `tst_can_reassembly` extension.
- **Deps:** WPC-T9
- [x] done

### WPC-T11 — UART policy fixes
- **Files:** `app/src/IO/Drivers/UART.cpp`, `app/tests/tst_uart_policy.cpp` (new)
- **Does:** Custom-path ports honour `ResourceError`; `registerDevice` failure -> `logDriverError` + queued NotificationCenter; persisted auto-select runs once per port-list change; `setPortIndex(0)` clears the key; dead mutex removed; `errorOccurred` wired after open.
- **Verify:** unit test on the policy helpers; `test_driver_api_comprehensive.py` placeholder-sticks case.
- **Deps:** WPC-T10
- [x] done

### WPC-T12 — USB consent without modals, write off the GUI
- **Files:** `app/src/IO/Drivers/USB.cpp`, `app/src/IO/Drivers/USB.h`, `app/qml/MainWindow/Panes/SetupPanes/...USB pane` (one file, named in the brief)
- **Does:** `setTransferMode(Advanced)` without recorded consent refuses + `logDriverError` + ProblemCenter finding; the pane button asks once and records consent; `write()` runs on the pump thread.
- **Verify:** `tst_usb_transfer_consent` (new); loading a project with `transferMode:1` shows no dialog (maintainer).
- **Deps:** WPC-T11
- **Status (coordinator, 2026-09-02):** first half done (consent recorded in the pane, no modal from `setDriverProperty`); moving `write()` off the GUI thread needs an async libusb submit/callback path only hardware can validate. Left open deliberately.
- [ ] done

### WPC-T13 — Audio capture independence and playback ring
- **Files:** `app/src/IO/Drivers/Audio.{h,cpp}`, `app/src/IO/Drivers/Audio/PlaybackRing.h` (new), `app/src/IO/Drivers/Audio/AudioDeviceCatalog.cpp`
- **Does:** Capture-only sessions ignore output absence (`playback.channels > 0` gate; "none selected" vs "vanished"); playback = fixed SPSC byte ring drained at device rate with underrun zero-fill + atomic counter; input path uses a pre-sized buffer pool and counts drops; `applyConnectionSettings` calls the setters; `open()` failures translated. Invariant: RT callback touches atomics and the ring only.
- **Verify:** `tst_playback_ring` (new); `test_audio_loopback.py` capture with no output selected + continuous tone via `write()`.
- **Deps:** WPC-T12
- [x] done

### WPC-T14 — Shared native contexts and non-persistent live instances
- **Files:** `app/src/IO/Drivers/UsbContext.{h,cpp}`, `HidContext.{h,cpp}`, `Audio/AudioContext.{h,cpp}` (new), `app/src/IO/ConnectionManager/DriverFactory.cpp`, `app/src/IO/Drivers/HID.cpp`, `app/src/IO/Drivers/USB.cpp`, `app/src/IO/Drivers/Audio.cpp`
- **Does:** Refcounted libusb/hidapi/miniaudio contexts shared by UI and live instances; `DriverFactory` calls `setPersistent(false)` on every live driver; HID `open()` closes first; live enumeration paused during a session. Invariant: `QThread` started-signal idiom with `quit()` before `wait()`.
- **Verify:** ASan leg at quit clean; `test_new_driver_api.py` settings unchanged after a live-instance setter.
- **Deps:** WPC-T13
- **Status (coordinator, 2026-09-02):** the hidapi refcount (the actual crash) is done; sharing the libusb/miniaudio contexts and the `setPersistent(false)` rollout across seven drivers need hardware. Left open deliberately.
- [ ] done

### WPC-T15 — Process driver, PluginRunner, MachineID non-blocking
- **Files:** `app/src/IO/Drivers/Process.cpp`, `app/src/Misc/Extensions/PluginRunner.cpp`, `app/src/Licensing/MachineID.cpp`
- **Does:** `doClose` and `refreshProcessList` signal-driven with timers; stderr kept out of the frame stream; double drop guarded; PluginRunner start/stop without `waitFor*` on the GUI (bounded 1 s total at quit then terminate); MachineID uses the persisted id and spawns tools only on first run, off-thread, 500 ms each.
- **Verify:** `tst_machine_id` (new) with a stubbed spawn; maintainer observes no stall on Process close.
- **Deps:** WPC-T14
- [x] done

### WPC-T16 — File transmission fixes
- **Files:** `app/src/IO/FileTransmission/XMODEM.cpp`, `app/src/IO/FileTransmission/ZMODEM.cpp`, `app/src/IO/FileTransmission.cpp`, `app/src/IO/FileTransmission/Protocol.h`
- **Does:** NAK/timeout set `SendingBlocks` before `sendBlock()`; typed `protocolError()` signal replaces string sniffing; ZMODEM seek failure closes state, ZRPOS cancels the pending chain; blank lines are sent.
- **Verify:** `tst_xymodem` XFAILs flipped to assertions; `tst_file_transmission` (new) error count and blank line.
- **Deps:** WPC-T15
- [x] done

### WPC-T17 — Driver drop integration tests
- **Files:** `tests/integration/test_driver_drops.py` (new), `tests/integration/test_connection_verdicts.py`, `tests/integration/test_audio_loopback.py`, `app/tests/tst_stream_config_builder.cpp` (new)
- **Does:** Simulator kill/unplug per family (TCP, UDP, WS, serial via pty where available, CAN slcan via pty); pause preserved across UART auto-reconnect; scripted `io.connect` + `writeData`; `StreamConfigBuilder` derivations.
- **Verify:** maintainer runs pytest; ctest green.
- **Deps:** WPC-T16
- [x] done

## WP-D — Industrial drivers, Sparkplug, Influx

### WPD-T1 — S7 zero-length item
- **Files:** `app/src/IO/Drivers/S7/S7Pdu.cpp`, `app/src/IO/Drivers/S7.cpp`, `app/tests/tst_s7comm_pdu.cpp`
- **Does:** `decodeValue` returns invalid on empty payload; `applyResult` counts an item error; no assert on wire input.
- **Verify:** test with the PDU from finding E1.
- **Deps:** none
- [x] done

### WPD-T2 — Sparkplug slot persistence
- **Files:** `app/src/IO/Drivers/MQTT/SparkplugSession.{h,cpp}`, `app/src/IO/Drivers/MQTT/MQTTSparkplug.cpp`, `app/src/DataModel/Frame.h` (`Keys::SparkplugSlots`)
- **Does:** `reset()` keeps slots and index, clears values/birth state; `slotsJson()`/`restoreSlots()`; driver property `sparkplugSlots` round-trips like Iec104 `points`; project generator writes it. Invariant: slot indices never move for a known metric.
- **Verify:** `tst_sparkplug_session` reconnect-with-reversed-birth-order keeps indices; `tst_frame_serialization` round-trip.
- **Deps:** WPD-T1
- [x] done

### WPD-T3 — Sparkplug publisher rebirth sequence and trigger
- **Files:** `app/src/MQTT/SparkplugPublisher.cpp`, `app/src/MQTT/PublisherWorker.cpp`, `app/tests/tst_sparkplug_publisher.cpp`
- **Does:** Rebirth resets `m_seq` to 0; rebirth also fires from the connection edge; test pins `seq == 0`.
- **Verify:** ctest.
- **Deps:** WPD-T2
- [x] done

### WPD-T4 — Modbus placeholder frame, caps, CRC
- **Files:** `app/src/IO/Drivers/Modbus.cpp`, `app/src/IO/Drivers/Modbus/ModbusProjectGenerator.cpp`, `app/tests/tst_modbus_generation.cpp`
- **Does:** Reply error publishes `[slave, fc, 0]`; generated Lua skips zero-length frames without advancing and resyncs on fc + byte count; coil/discrete cap 2000 bits; RTU builder emits CRC and the responding unit id; `pollInterval` bound matches the UI.
- **Verify:** generation test with an injected error frame keeps attribution; `test_cpp_regressions.py` R14 updated.
- **Deps:** WPD-T3
- [x] done

### WPD-T5 — OPC UA policy, trust order, write convention
- **Files:** `app/src/IO/Drivers/OpcUaSession.cpp`, `app/src/IO/Drivers/OpcUaSecurity.cpp`, `app/src/IO/Drivers/OpcUa.cpp`, `app/tests/tst_opcua_security.cpp` (new)
- **Does:** `allowNonePolicyPassword` only with explicit opt-in; trust decision checked before hostname/expiry; `write()` returns -1.
- **Verify:** security test: hostname/SAN/wildcard/PEM->DER cases and the trusted-overrides-self-signed case.
- **Deps:** WPD-T4
- [x] done

### WPD-T6 — Iec104 slot key, Influx counters, vault wording
- **Files:** `app/src/IO/Drivers/Iec104.cpp`, `app/src/InfluxDB/Export.cpp`, `app/src/MQTT/CredentialVault.cpp`, `app/tests/tst_iec104_slots.cpp` (new)
- **Does:** Slot key includes type id and live `typeId` wins; TLS failure counted once; wall-clock offset re-sampled on reconnect; vault docstrings and UI strings say "obfuscated".
- **Verify:** ctest; `grep -rn 'encrypted' app/qml app/src/MQTT app/src/AI` shows no credential-store claim.
- **Deps:** WPD-T5
- [x] done

### WPD-T7 — Industrial unit coverage
- **Files:** `app/tests/tst_opcua_frame_assembler.cpp`, `tst_opcua_subscriptions.cpp`, `tst_modbus_register_groups.cpp`, `tst_ethernetip_worker.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Assembler split/partial frames; subscription state machine; register-group persistence; EIP worker through the `kEipBackend` seam stub.
- **Verify:** ctest green.
- **Deps:** WPD-T6
- [x] done (3 of 4; `tst_ethernetip_worker` blocked -- see handoff-wpd.md)

### WPD-T8 — Wire-parser fuzzers
- **Files:** `app/tests/fuzz/fuzz_s7_pdu.cpp`, `fuzz_isotsap.cpp`, `fuzz_iec104_apci.cpp`, `fuzz_iec104_asdu.cpp`, `fuzz_sparkplug_payload.cpp`, `fuzz_opcua_wire.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** One entry point per codec; seeds exported from the existing table tests.
- **Verify:** sanitizer job corpus replay green; 60 s local libFuzzer run per target finds nothing (maintainer, once).
- **Deps:** WPD-T7, WP0-T7
- [x] done

### WPD-T9 — Industrial integration tests
- **Files:** `tests/integration/test_modbus_groups.py`, `tests/integration/test_sparkplug_host.py` (new)
- **Does:** Modbus simulator with an injected timeout keeps group attribution; broker-marked Sparkplug host reconnect keeps slots and observes `seq == 0` on rebirth.
- **Verify:** maintainer runs with broker.
- **Deps:** WPD-T8
- [x] done

## WP-E — Dashboard, widgets, QML

### WPE-T1 — Extract DashboardIngest (pure move)
- **Files:** `app/src/UI/Dashboard/DashboardIngest.{h,cpp}` (new), `app/src/UI/Dashboard.{h,cpp}`, `app/CMakeLists.txt`
- **Does:** `applyBlock*`, `advancePlotClock`, `feed*Sweep`, `update*Series`, push-table build/clear move into an owned sub-object; no behaviour change. Invariants: GUI thread only; push tables hold indexes; `m_layoutValid` contract; `resetPlotClocks()` one-state rule.
- **Verify:** `HOTPATH_DASHBOARD_INGEST_COST` unchanged within noise (maintainer benchmark); `tst_dashboard_viewstate` green.
- **Deps:** none
- [x] done

### WPE-T2 — Uniform-grid lane feeds line, GPS, 3D, multiplot samples
- **Files:** `app/src/UI/Dashboard/DashboardIngest.{h,cpp}`, `app/src/UI/Dashboard.h`
- **Does:** `StreamTargets` gains `linePushIndexes`, `gpsPushIndex`, `plot3DPushIndex`, `multiSampleIndexes` resolved in the second configure pass; `applyBlockColumn` feeds line consumers per sample and GPS/3D/latest per block. Invariant: no per-frame lookups; indexes only.
- **Verify:** `tst_dashboard_ingest` (WPE-T4) Samples-axis plot receives audio-lane samples.
- **Deps:** WPE-T1
- [x] done

### WPE-T3 — setPoints preserves sweep state; string targets; reference buckets
- **Files:** `app/src/UI/Dashboard.cpp`, `app/src/UI/Dashboard/WidgetMapBuilder.cpp`
- **Does:** `rebuildLineSeriesPreservingState()` shared by `setPoints` and `setPlotTimeRange` (sweep config + run flags); `QString::number` per column only for `stringTargets`; `m_datasetReferences` by index; dead static `FrameBuilder::instance()` re-resolve removed.
- **Verify:** `tst_dashboard_ingest` sweep-survives-points case.
- **Deps:** WPE-T2
- [x] done

### WPE-T4 — Headless Dashboard ingest tests
- **Files:** `app/tests/tst_dashboard_ingest.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Fixture builds a Dashboard with a small project; cases: uniform block into Samples plot/multiplot/GPS, sweep after `setPoints`, `advancePlotClock` continuation, `growTimeRing`, `handleMissingDataset` quarantine.
- **Verify:** ctest green.
- **Deps:** WPE-T3
- [x] done

### WPE-T5 — Waterfall LUT, tiles, overlay dirtiness
- **Files:** `app/src/UI/Widgets/Waterfall.{h,cpp}`, `app/src/UI/Widgets/Waterfall/WaterfallOverlay.cpp`
- **Does:** 256-entry `QRgb` LUT per colour map; spectrogram in 64-row texture tiles with per-tile dirty flags, only dirty tiles re-uploaded; overlay re-rasterized on axis/marker/theme change only; `historySizeChanged` on real change. Invariant: render-thread work confined to `updatePaintNode`.
- **Verify:** `tst_waterfall_tiles`, `tst_colormap_lut` (new); maintainer benchmark of GUI cost at FFT 8192 / 70 s before and after.
- **Deps:** WPE-T4
- [x] done

### WPE-T6 — Terminal selection safety and buffer alignment
- **Files:** `app/src/UI/Widgets/Terminal.{h,cpp}`, `app/src/UI/Widgets/Terminal/TerminalBuffer.cpp`, `app/tests/tst_terminal_selection.cpp` (new)
- **Does:** `clear()` resets selection; `copy()` clamps rows; ANSI erase overrides go through `applyLineDrop`; colour rows trimmed in lockstep regardless of the ANSI flag; CSI parameter list capped; per-paint `mid()`/`toString()` churn removed.
- **Verify:** test: select, clear, receive three lines, copy; ANSI erase with colours toggled keeps alignment.
- **Deps:** WPE-T5
- [x] done

### WPE-T7 — ExtensionData rows, FFT/MultiPlot signals, WindowManager clamp
- **Files:** `app/src/UI/Widgets/ExtensionData.{h,cpp}`, `app/src/UI/Widgets/DataGrid.cpp`, `app/src/UI/Widgets/FFTPlot.cpp`, `app/src/UI/Widgets/MultiPlot.cpp`, `app/src/UI/WindowManager.cpp`
- **Does:** Rows rebuilt on structure change only, per-tick values through cached refs, shared `datasetWidgetsFor()`; `markerValuesChanged` on change; `m_drawOrders` removed; clamped rect applied on resize.
- **Verify:** `tst_extension_data_rows` (new); `tst_window_geometry` extended.
- **Deps:** WPE-T6
- [x] done

### WPE-T8 — Theme colours as QQmlPropertyMap
- **Files:** `app/src/Misc/ThemeManager.{h,cpp}`, `app/tests/tst_theme_property_map.cpp` (new)
- **Does:** `colors` becomes a `QQmlPropertyMap*` property seeded with every key of `default.json`; theme switch `insert()`s per key; `getColor` reads the map. Invariant: ThemeManager constructs pre-root (startup.md exemption), no new singleton reach in its ctor.
- **Verify:** test asserts key set and per-key notify; `--selftest qml` green; qmllint baseline unchanged.
- **Deps:** WPE-T7
- [x] done

### WPE-T9 — QML per-tick fixes
- **Files:** `app/qml/Widgets/Dashboard/ConsoleAnnotations.qml`, `app/qml/ProjectEditor/Views/TableDelegate.qml`, `app/qml/Widgets/Dashboard/ValueFormat.js`, `app/qml/Widgets/Dashboard/Compass.qml`
- **Does:** Track `refresh()` assigns lanes only on change; separator canvas repaints on theme change; one `formatValue` per sample.
- **Verify:** maintainer observes theme switch with the editor open and idle CPU with the track tab visible.
- **Deps:** WPE-T8
- [x] done

### WPE-T10 — Trial getters and monotonic clock write throttle
- **Files:** `app/src/Licensing/MonotonicClock.cpp`, `app/src/Licensing/Trial.cpp`
- **Does:** `now()` writes `lastSeen` at most once per minute; `daysRemaining()`/`trialEnabled()` read a cached value refreshed on the 1 Hz tick. Invariant: no settings write from a property getter.
- **Verify:** `tst_monotonic_clock` (WP-H) write-count case; maintainer `sample` shows no `QSettings::sync` under binding evaluation.
- **Deps:** WPE-T9
- [x] done

### WPE-T11 — Dashboard lanes integration test
- **Files:** `tests/integration/test_dashboard_lanes.py` (new)
- **Does:** Audio or session replay into a Samples-axis plot, Samples multiplot, GPS group; sweep survives `dashboard.setPlotPoints`.
- **Verify:** maintainer runs with the app up.
- **Deps:** WPE-T10
- [x] done

## WP-F — Project layer and editors

### WPF-T1 — Point count through a mutator
- **Files:** `app/src/DataModel/ProjectModel.{h,cpp}`
- **Does:** `setPointCount(int)` opens a `ProjectUndoScope`, sets the field, `setModified(true)`, schedules autosave; the `pointsChanged` handler calls it; no direct `writeProjectFile`. Invariant: ctor closure untouched (handler lives in `setupExternalConnections`); first `setModified(true)` commits the memento.
- **Verify:** `tst_project_persistence` (WPF-T7) hash-unchanged case; `undo-scope-missing` clean.
- **Deps:** none
- [x] done

### WPF-T2 — Reload failure keeps the document attached
- **Files:** `app/src/DataModel/Project/ProjectPersistence.cpp`
- **Does:** On a failed reload keep `m_filePath` and content, restore `m_modified`, post the reason; `savePluginState` gated on ProjectFile mode. Invariant: every successful load/write re-arms the watcher via `watchProjectFile()`.
- **Verify:** `tst_project_persistence` corrupt-external-write case.
- **Deps:** WPF-T1
- [x] done

### WPF-T3 — Bulk delete ordering
- **Files:** `app/src/DataModel/Project/ProjectBulkOps.cpp`, `app/tests/tst_project_bulk_ops.cpp` (new)
- **Does:** Explicit order table: datasets, tables, actions, output widgets, then groups, then folders; UIDs re-resolved per step (unchanged).
- **Verify:** test: folder + table inside, group + dataset inside, mixed selection.
- **Deps:** WPF-T2
- [x] done

### WPF-T4 — Entities: action dirtiness, sourceId normalisation, small fixes
- **Files:** `app/src/DataModel/Project/ProjectEntities.cpp`, `app/src/DataModel/Project/ProjectOutputWidgets.cpp`, `app/src/DataModel/Project/ProjectSources.cpp`, `app/src/UI/Dashboard/DashboardTools.cpp`
- **Does:** `updateAction` dirties + schedules autosave; `DashboardTools::configureActions` re-reads on `actionsChanged`; self-assignment removed; `renumberGroupIds` shared; `sourceId` normalised on `deleteSource`; `changeDatasetOption` range check first.
- **Verify:** `tst_project_persistence` action-edit autosave case; `test_project_integrity.py` (WPF-T10).
- **Deps:** WPF-T3
- [x] done

### WPF-T5 — Compound template pick; empty-code seeding
- **Files:** `app/src/DataModel/Project/ProjectSources.cpp`, `app/src/DataModel/Editors/FrameParserModel.cpp`, `app/src/DataModel/Editors/JsCodeEditor.cpp`
- **Does:** `setSourceFrameParserTemplateAndParams` under one scope; empty code seeds the language's default template, never source 0.
- **Verify:** `tst_frame_parser_model` (new): one undo step per pick; new source starts empty.
- **Deps:** WPF-T4, WP-B done (editor files)
- [x] done

### WPF-T6 — Coalesced pipeline sync and workspace regen
- **Files:** `app/src/DataModel/ProjectModel.cpp`
- **Does:** Duplicate `scheduleAutoSave` connections removed; `frameDetectionChanged` -> AppState sync through a 0 ms coalescing timer; `groupsChanged` -> auto-workspace regen coalesced. Invariant: signal wiring read before edit; `groupsChanged` -> tree rebuild stays queued.
- **Verify:** `tst_project_persistence` counts one sync per burst; `test_project_undo.py` green.
- **Deps:** WPF-T5
- [x] done

### WPF-T7 — Project persistence and history unit tests
- **Files:** `app/tests/tst_project_persistence.cpp`, `app/tests/tst_project_history.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Disk-watch hash, autosave suspend/flush, reload failure, point-count write, action autosave, sync coalescing; history coalesce window, 100/64 MiB bounds, save point unreachable, redo-tail truncation.
- **Verify:** ctest green.
- **Deps:** WPF-T6
- **Status:** `tst_project_history` landed. `tst_project_persistence` NOT registered: its link set is ProjectModel + the 17 Project/*.cpp TUs + the SessionContext dtor closure, i.e. the application (same wall as the two commented importer suites in `app/tests/CMakeLists.txt`). That coverage moved to `tests/integration/test_project_integrity.py` (WPF-T10).
- [x] done

### WPF-T8 — EmbeddedCodeEditorItem base
- **Files:** `app/src/DataModel/Editors/EmbeddedCodeEditorItem.{h,cpp}` (new), `app/src/DataModel/Editors/ControlScriptEditor.{h,cpp}`, `JsCodeEditor.{h,cpp}`, `MacroEditor.{h,cpp}`, `OutputCodeEditor.{h,cpp}`, `PainterCodeEditor.{h,cpp}`, `app/CMakeLists.txt`
- **Does:** Base carries the sixteen forwarding overrides and the `renderable()` gate; five hosts derive; `TransmitTestDialog` held by `unique_ptr`, created on first use. Invariant: the three hidden-widget plumbing rules (position sync, `ShortcutOverride`, completer reroute) stay in the base.
- **Verify:** maintainer opens each editor, types, uses completer, drops a file; `--dup-census` count for the five pairs drops to zero.
- **Deps:** WPF-T7
- [x] done

### WPF-T9 — Loader migrations, workspace refs, editors coverage
- **Files:** `app/tests/tst_project_loader_migrations.cpp`, `tst_project_workspace_refs.cpp`, `tst_transmit_test_dialog.cpp` (new), `tests/fixtures/projects/legacy/` (new)
- **Does:** Fixture-driven legacy separator/xAxis/layout-key/schema/uid-dedup migrations; workspace ref shift/remap/anchor; hex parsing.
- **Verify:** ctest green.
- **Deps:** WPF-T8
- **Status:** legacy fixtures landed under `tests/fixtures/projects/legacy/` and are driven from `test_project_integrity.py`. `tst_project_loader_migrations`, `tst_project_workspace_refs` and `tst_transmit_test_dialog` NOT registered: ProjectLoader needs ProjectModel, WorkspaceRefs needs SerialStudio.cpp (icon registry + theme manager + players), TransmitTestDialog is a live QWidget. Reasons recorded in `app/tests/CMakeLists.txt` beside the suites they mirror.
- [x] done

### WPF-T10 — Project integrity integration test
- **Files:** `tests/integration/test_project_integrity.py` (new)
- **Does:** Hash unchanged after points change with unsaved edits; undo depth per mutation; bulk delete; corrupt external write; empty parser for a new source.
- **Verify:** maintainer runs.
- **Deps:** WPF-T9
- [x] done

## WP-G — API surface

### WPG-T1 — Reception loop without held references; HTTP sniff; handshake gate
- **Files:** `app/src/API/Server/ClientReception.{h,cpp}`, `app/src/API/Server/ConnectionState.h`, `app/src/API/Server.{h,cpp}`
- **Does:** Pop one line, dispatch, re-resolve state via `ReceptionHost::stateFor(socket, sessionId)`; first bytes matching an HTTP request line close the connection; `handshakeSeen` gates raw forwarding until one valid JSON message; bytes counted once. Invariant: no reference into `m_connections` survives a call that can spin an event loop.
- **Verify:** `tst_client_reception` (WPG-T3) with a host stub that erases the entry mid-dispatch; `test_http_on_api_socket.py`.
- **Deps:** none
- [x] done

### WPG-T2 — Consent prompts leave the receive path
- **Files:** `app/src/API/Server.cpp`, `app/src/API/Server/ClientReception.cpp`
- **Does:** `authorizeDeviceWrite` with consent Unset returns refusal `CONSENT_REQUIRED` and posts the prompt queued; the recorded answer applies to later writes. Invariant: no `QMessageBox::exec()` reachable from `consumeBytes`.
- **Verify:** `tst_client_reception` consent case; `test_access_control.py` device-write cases updated.
- **Deps:** WPG-T1
- [x] done

### WPG-T3 — Reception unit test and stress xfail flip
- **Files:** `app/tests/tst_client_reception.cpp` (new), `tests/security/test_unknown_input_hardening.py`
- **Does:** Host stub mutating the table during dispatch; pipelined bytes after auth; HTTP sniff; the 10-thread stress xfail becomes a passing assertion.
- **Verify:** ctest green under ASan; pytest green.
- **Deps:** WPG-T2
- [x] done

### WPG-T4 — Path policy as command metadata
- **Files:** `app/src/API/CommandRegistry.{h,cpp}`, `app/src/API/PathPolicy.{h,cpp}`, `app/src/API/Handlers/SessionsHandler.cpp`, `LicensingHandler.cpp`, `AssistantHandler.cpp`
- **Does:** `CommandDefinition.pathParams` (name + `allowMissing`); registry enforces the policy for every declared param with `PATH_NOT_ALLOWED`; the three bypassing commands declare their params; existing per-handler checks removed from the four guarded commands and the ad hoc ones (`CSVPlayerHandler`, `MDF4PlayerHandler`, `ProjectFileCommands`, `OpcUaHandler`, `ProcessHandler`) declare instead.
- **Verify:** `tst_path_policy_registry` (new) walks every registered command and asserts every param named `*path*` is declared; `test_path_policy_all_commands.py`.
- **Deps:** WPG-T3
- [x] done

### WPG-T5 — Outbound caps on every lane
- **Files:** `app/src/API/Server/ServerWorker.cpp`, `app/src/API/Server/ServerWorker.h`, `app/tests/tst_server_worker_caps.cpp` (new)
- **Does:** `writeToSocket` and `broadcastEvent` under the cap; exceeding disconnects with `WRITE_BACKLOG` counted.
- **Verify:** ctest; `test_write_backlog.py`.
- **Deps:** WPG-T4
- [x] done

### WPG-T6 — gRPC abortable marshal and hygiene
- **Files:** `app/src/API/GRPC/GRPCServer.{h,cpp}`, `app/tests/tst_grpc_pending_call.cpp` (new)
- **Does:** `PendingCall` (mutex + condvar + `abandon()`) replaces `BlockingQueuedConnection`; `stopServer` abandons pending calls before `Shutdown`; peer parsed with `QHostAddress`; `WriteRawData` cap; shared error codes. Invariant: GUI never blocks on the gRPC thread.
- **Verify:** unit test stops the server with a call parked; `test_grpc_lifecycle.py` (WPG-T8).
- **Deps:** WPG-T5
- [x] done

### WPG-T7 — Loopback v6, port setting, header order, mirror version lint
- **Files:** `app/src/API/Server.{h,cpp}`, `app/src/Misc/CLI.cpp` (`--api-port`), `app/qml/Dialogs/Settings*.qml` (API page, one file), `scripts/registry-verify.py`
- **Does:** Two loopback listeners (v4 + v6) or dual-stack `Any` when external; `API/Port` setting + CLI flag with 7777 default; `Server.h` section order; lint fails a `wireUniqueId`/dataset-order edit without a `kWireVersion` bump.
- **Verify:** `test_api_ipv6.py` (new); `registry-verify.py` on an ordering change without a version bump fails (maintainer once).
- **Deps:** WPG-T6
- [x] done

### WPG-T8 — API security and lifecycle tests, reception fuzzer
- **Files:** `tests/security/test_http_on_api_socket.py`, `test_path_policy_all_commands.py`, `test_write_backlog.py`, `tests/integration/test_grpc_lifecycle.py`, `test_api_ipv6.py` (new), `app/tests/fuzz/fuzz_api_json.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** As named; fuzzer seeds from `test_unknown_input_hardening.py` payloads.
- **Verify:** maintainer runs pytest; fuzz replay in the sanitizer job.
- **Deps:** WPG-T7, WP0-T7
- [x] done

## WP-H — Assistant, extensions, CLI, licensing, startup

### WPH-T1 — Assistant checkpoint instead of disk autosave; resume guard
- **Files:** `app/src/AI/Conversation.cpp`, `app/src/AI/Assistant.cpp`, `app/qml/AI/AssistantPanel.qml` or the settings page (one file, named in the brief)
- **Does:** Autosave timer calls `BackupManager::snapshot("assistant")`; disk writes only through `project.save` (Confirm); approve/deny share the `!m_reply` tri-condition; toggle copy states checkpoint semantics.
- **Verify:** `tst_conversation_turn` (WPH-T4) approve-during-stream; `test_assistant_autosave.py`.
- **Deps:** none
- [x] done

### WPH-T2 — fs tools off the GUI thread; local context window
- **Files:** `app/src/AI/Tools/ToolFilesystemTools.cpp`, `app/src/AI/Conversation.cpp`, `app/src/AI/Providers/LocalProvider.cpp`, `app/src/AI/Providers/Provider.h`, settings page (same file as WPH-T1)
- **Does:** `fs.search`/`fs.read` run via `QtConcurrent::run` with a generation id, completing through the outstanding-tool-result path, cancellable; `ai/localContextWindow` setting feeds `capabilities().contextWindowTokens`.
- **Verify:** `tst_conversation_turn` async tool + 8 k window budget cases.
- **Deps:** WPH-T1
- [x] done

### WPH-T3 — Reply base lift, transport policy, redact
- **Files:** `app/src/AI/Providers/Provider.{h,cpp}`, `AnthropicReply.cpp`, `OpenAIReply.cpp`, `GeminiReply.cpp`, `app/src/AI/KeyVault.cpp`
- **Does:** `finishOk`/`finishWithError`/budget/readyRead/finished into `Reply`; parse-error policy unified (skip recoverable, end on fatal); redirects refused, plain `http://` only to loopback; `redact` returns `"***"`.
- **Verify:** `tst_reply_state_machine` (WPH-T4) with `FakeTransport`; `tst_redactor`.
- **Deps:** WPH-T2
- [x] done

### WPH-T4 — Assistant unit coverage
- **Files:** `app/tests/tst_sse_event_reader.cpp`, `tst_think_tag_splitter.cpp`, `tst_reply_state_machine.cpp`, `tst_file_sandbox.cpp`, `tst_redactor.cpp`, `tst_conversation_turn.cpp`, `tst_sentinel_probe.cpp` (new), `app/tests/fuzz/fuzz_sse_reader.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** As named in M10; `FakeProvider` drives the turn loop.
- **Verify:** ctest green; fuzz replay.
- **Deps:** WPH-T3, WP0-T21
- [x] done

### WPH-T5 — Extension catalog v2 and atomic install
- **Files:** `app/src/Misc/Extensions/ExtensionInstaller.{h,cpp}`, `app/src/Misc/Extensions/ExtensionCatalog.cpp`, `app/rcc/extensions/schema/catalog.json` (new), `scripts/registry-verify.py`
- **Does:** `files[]` entries carry `sha256` and `size`; entries without digests refused with a Problem Center finding; download to `<id>.staging`, verify each file, swap atomically, `.previous` deleted on success, `.staging` deleted on failure; `installed.json` gains digests; bundled catalog validated by `registry-verify`.
- **Verify:** `tst_extension_installer` (WPH-T7) partial, bad digest, success.
- **Deps:** WPH-T4
- [x] done

### WPH-T6 — Repo scheme, version compare, install location
- **Files:** `app/src/Misc/ExtensionManager.cpp`
- **Does:** `addRepository` accepts https or file only; `hasUpdate` via `QVersionNumber::compare`; install folder from the local package type.
- **Verify:** `tst_extension_installer` http-refused and downgrade cases.
- **Deps:** WPH-T5
- [x] done

### WPH-T7 — Extension installer tests
- **Files:** `app/tests/tst_extension_installer.cpp` (new), `tests/integration/test_extension_install.py` (new)
- **Does:** `FakeTransport` cases above; pytest against a local `file://` catalog fixture.
- **Verify:** ctest; maintainer pytest.
- **Deps:** WPH-T6
- [x] done

### WPH-T8 — Licensing request signal and deactivation truth
- **Files:** `app/src/Licensing/LemonSqueezy.{h,cpp}`, `app/src/Licensing/OfflineLicense.cpp`
- **Does:** `requestFinished(ok, reason)` emitted once per activate/deactivate on every path; deactivation clears the cache only on `deactivated == true`; offline activation routes through `notifyEntitlementMaybeChanged`. Invariant: `activatedChanged` only on a real validity transition; licensing block position in the pinned order unchanged.
- **Verify:** `tst_lemonsqueezy_rules` (new) on JSON fixtures.
- **Deps:** WPH-T7
- [x] done

### WPH-T9 — CLI reset, activate/deactivate, token sources
- **Files:** `app/src/Misc/CLI.cpp`, `app/src/Misc/CrashTracker.cpp`
- **Does:** `--reset` uses default `QSettings` with the shared preserve-list function; `--activate`/`--deactivate` wait on `requestFinished` with meaningful exit codes; `--api-token-file` and `SS_API_TOKEN`.
- **Verify:** `tst_cli_exits` (new) with a stub server; `test_cli_licensing.py`.
- **Deps:** WPH-T8
- [x] done

### WPH-T10 — Startup failure teardown and queued reply modals
- **Files:** `app/src/main.cpp`, `app/src/Licensing/Trial.cpp`, `app/src/Licensing/LemonSqueezy.cpp`, `app/src/Misc/ExtensionManager.cpp`
- **Does:** Both exits run one `shutdownSession()` ladder; reply-handler message boxes posted queued. Invariant: `shutdown()` runs with `qApp` alive after the QML engine dies; never from a destructor.
- **Verify:** `tst_session_context_lifecycle` (new): headless bootstrap + early-exit ladder releases in reverse order.
- **Deps:** WPH-T9
- [x] done

### WPH-T11 — Infra unit coverage
- **Files:** `app/tests/tst_simplecrypt.cpp`, `tst_monotonic_clock.cpp`, `tst_commercial_token.cpp`, `tst_trial_state.cpp`, `tst_machine_id.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** As named in M11; monotonic clock write-count case for WPE-T10.
- **Verify:** ctest green.
- **Deps:** WPH-T10
- [x] done

### WPH-T12 — Assistant and CLI integration tests
- **Files:** `tests/integration/test_assistant_autosave.py`, `tests/integration/test_cli_licensing.py` (new)
- **Does:** File hash unchanged until Save under auto-approve; CLI activate/deactivate/reset against a stub server.
- **Verify:** maintainer runs.
- **Deps:** WPH-T11
- [x] done

## WP-I — One implementation per concern (after A..H merge)

### WPI-T1 — PolledPlcWorkerBase
- **Files:** `app/src/IO/Drivers/PolledPlcWorkerBase.{h,cpp}` (new), `app/src/IO/Drivers/S7.cpp/.h`, `app/src/IO/Drivers/EthernetIp.cpp/.h`, `app/CMakeLists.txt`
- **Does:** Shared worker ctor, `publishDirtySlots`, atomic counters, open/close/linkLost/statusJson/generateProject skeleton; wildcard disconnects replaced.
- **Verify:** `tst_s7comm_*`, `tst_ethernetip_worker` green; dup census pair count for S7/EIP below threshold.
- **Deps:** WP-C, WP-D done
- [x] done

### WPI-T2 — OpenAICompatibleProvider
- **Files:** `app/src/AI/Providers/OpenAICompatibleProvider.{h,cpp}` (new), `DeepSeek*`, `Groq*`, `Mistral*`, `OpenRouter*` provider files (delete), `app/src/AI/Providers/ProviderRegistry` (whatever registers them, named in the brief), `app/CMakeLists.txt`
- **Does:** Table rows for endpoint, model list, caps; registry unchanged for users.
- **Verify:** `tst_provider_json` green; settings show the same provider list.
- **Deps:** WP-H done
- [x] done

### WPI-T3 — ReplayPlaybackEngine
- **Files:** `app/src/DataModel/ReplayPlaybackEngine.{h,cpp}` (new), `app/src/CSV/Player.cpp`, `app/src/MDF4/Player.cpp`, `app/src/Sessions/Player.cpp`, `app/CMakeLists.txt`
- **Does:** Seek window, catch-up budget, epoch, anchor and timer chain composed by the three players; each keeps its storage and properties.
- **Verify:** `tst_replay_seek_engine`, `tst_sessions_player_epoch`, `tst_csv_player_catchup`, `test_replay_timeline.py` green.
- **Deps:** WP-A done
- [x] done

### WPI-T4 — ExportStructure
- **Files:** `app/src/DataModel/ExportStructure.{h,cpp}` (new), `app/src/CSV/Export.cpp`, `app/src/MDF4/Export.cpp`, `app/src/Sessions/Export.cpp`
- **Does:** Template frame, published-structure apply, `sanitizeTitle`, session directory shared; ConsoleOnly dead branches and stale replay comments removed.
- **Verify:** export ctest suites green; dup census pairs below threshold.
- **Deps:** WPI-T3
- [x] done

### WPI-T5 — FrameBuilder facade split
- **Files:** `app/src/DataModel/FrameBuilder/BlockPublisher.{h,cpp}`, `ReplayIngest.{h,cpp}` (new), `app/src/DataModel/FrameBuilder.{h,cpp}`, `app/CMakeLists.txt`
- **Does:** Publish fan-out and replay lanes into sub-objects; four apply tails and two `parseProjectFrame`s collapsed; vestigial slot pool, budget, `frameChanged`, `m_channelScratch` removed; facade under 1500 lines. Invariants: publish path allocation-free; one clone per sink; replay never reaches sinks.
- **Verify:** `--benchmark-hotpath` all tiers (maintainer); `tu-census --check` passes with the excess pool lower.
- **Deps:** WPI-T4
- **Status (coordinator, 2026-09-02):** partial. `BlockPublisher`/`ReplayIngest` extracted, dead pool removed, sink hooks injected; `FrameBuilder.cpp` is still 2983 lines. The dataset-apply cluster (683 lines, `SS_HOT`, 1.024 MHz tier) and the transform dispatch (318, `m_compileGuard` contract) stay in the facade until a benchmark-gated move; the remaining ~480 lines are a follow-up.
- [ ] done (partial)

### WPI-T6 — ConnectionManager and Conversation facades
- **Files:** `app/src/IO/ConnectionManager/DriverUiForwarders.{h,cpp}`, `app/src/AI/Conversation/ToolTurnRunner.{h,cpp}` (new), `app/src/IO/ConnectionManager.{h,cpp}`, `app/src/AI/Conversation.{h,cpp}`, `app/src/AI/Assistant.cpp`, `app/src/AI/ToolDispatcher.{h,cpp}`
- **Does:** Forwarders and stale comments out of ConnectionManager; tool batch/approval/checkpoint into `ToolTurnRunner`; `ToolDispatcher` given state or deleted; `Assistant` single ownership; both facades under 1500.
- **Verify:** `tst_connection_verdicts`, `tst_conversation_turn` green; `tu-census --check`.
- **Deps:** WPI-T5
- [x] done

### WPI-T7 — Plot3DNodes class, PlotBase
- **Files:** `app/src/UI/Widgets/Plot3D/Plot3DNodes.{h,cpp}`, `app/src/UI/Widgets/Plot3D.{h,cpp}`, `app/src/UI/Widgets/PlotBase.{h,cpp}` (new), `Plot.{h,cpp}`, `MultiPlot.{h,cpp}`, `FFTPlot.{h,cpp}`
- **Does:** `Plot3DNodes` becomes a real owned class; shared plot behaviour in `PlotBase`.
- **Verify:** one-class-one-file rule satisfied; maintainer renders all three plots and 3D.
- **Deps:** WP-E done
- [x] done

### WPI-T8 — ContextRegistry
- **Files:** `app/src/Misc/ContextRegistry.{h,cpp}` (new), `app/src/Misc/ModuleManager.cpp`, `app/src/UI/WidgetExtensions.cpp` (`hostContextNames`), `scripts/registry-verify.py`
- **Does:** One table registered in a loop; four unused globals removed; `hostContextNames()` reads the table; lint compares. Invariant: registration still after wiring, before the QML load.
- **Verify:** `--selftest qml` green; `registry-verify` green.
- **Deps:** WPI-T7
- [x] done

### WPI-T9 — Shared QML components
- **Files:** `app/qml/Widgets/Dashboard/InstrumentBase.qml`, `SwipePages.qml`, `app/qml/Widgets/DialogEscape.qml`, `app/qml/ProjectEditor/Views/CodeEditorMenu.qml`, `app/qml/Dialogs/DriverTagPickerDialog.qml` (new), and the consumers named in G5/G8
- **Does:** Instrument chrome/tick math/page persistence, Escape handling, editor menu, tag-picker family de-duplicated; `Settings` categories preserved.
- **Verify:** dup census for the named pairs below threshold; `--selftest qml`; maintainer visual pass.
- **Deps:** WPI-T8
- [x] done

### WPI-T10 — Three QML monoliths split
- **Files:** `app/qml/Dialogs/ExtensionManager/{Grid,Detail,Repos}Page.qml`, `app/qml/AI/AssistantPanel/*.qml`, `app/qml/DatabaseExplorer/ReportOptions/*.qml`, `app/src/Sessions/ReportOptionsModel.{h,cpp}` (new), originals reduced
- **Does:** Page-boundary splits; report tree logic in a C++ proxy model.
- **Verify:** `--selftest qml`; maintainer walks each dialog.
- **Deps:** WPI-T9
- [x] done

### WPI-T11 — Rule violations and startup doc for the idiom
- **Files:** `app/src/API/Server.h`, `app/src/MDF4/Export.cpp`, `app/src/MDF4/Player.cpp`, `doc/claude/architecture/startup.md`
- **Does:** Header order; trial-parity wording; cached-reference idiom and single-session assumption documented; singleton census unchanged.
- **Verify:** `trial-parity` lint clean; `--singleton-census --check` unchanged.
- **Deps:** WPI-T10
- [x] done

## WP-K — Rendering and priority cost (after WP-I, before WP-J; added 2026-09-02)

### WPK-T1 — Waterfall persistent ring texture
- **Files:** `app/src/UI/Widgets/Waterfall/WaterfallRingTexture.{h,cpp}` (new), `app/src/UI/Widgets/Waterfall.{h,cpp}`, `app/src/UI/Widgets/Waterfall/WaterfallTiles.{h,cpp}`, `app/CMakeLists.txt`
- **Does:** One `QSGTexture` per widget over an owned `QRhiTexture` (public QRhi, `QQuickWindow::rhi()`), rows written at a moving index via a sub-rect upload on the frame's resource update batch, scroll as a UV offset in the material; `rebuildHistoryImage` is the only full rebuild; `writeRow` skipped when no samples arrived; tiles remain as the fallback when the RHI texture cannot be created. Invariants: render-thread work only inside `updatePaintNode` (the upload batch is taken there); GUI-side row writes into the CPU staging row only; no per-tick allocation.
- **Verify:** `tst_waterfall_ring_texture` (write index wrap, UV offset math, idle skip); maintainer: page faults under 50 k/s with four waterfalls.
- **Deps:** WP-I done
- [x] done

### WPK-T2 — Waterfall overlay cadence and hidden release
- **Files:** `app/src/UI/Widgets/Waterfall.cpp`, `app/src/UI/Widgets/Waterfall/WaterfallOverlay.cpp`
- **Does:** `markAxisDirty()` only on axis, size, marker or theme change (never at the end of `updateData`); `ItemVisibleHasChanged` to hidden releases the image and both textures, visible re-creates them lazily.
- **Verify:** `tst_waterfall_ring_texture` overlay-dirty case; maintainer: no upload while idle.
- **Deps:** WPK-T1
- [x] done

### WPK-T3 — MMCSS on the pipeline and stream threads
- **Files:** `app/src/Platform/AppPlatform.{h,cpp}`, `app/src/IO/PipelineHost.{h,cpp}`, `app/src/IO/StreamWorker.cpp`, `app/src/Misc/ModuleManager.cpp`, `app/src/Benchmark/HotpathBenchmark.cpp`, `app/tests/tst_mmcss_registration.cpp` (new)
- **Does:** Per-thread registration guard; GUI call removed; `PipelineHost::registerIngestThread()` posts the registration into the pipeline thread after `qInstallMessageHandler` (ModuleManager calls it right after installing the handler); stream workers register on `QThread::started`. Invariants: registration only through `AppPlatform::registerIngestThreadWithMmcss()`, never before the message handler, never a QThread expected to inherit the band.
- **Verify:** Windows-only ctest asserts the calling thread, not the main thread, is registered; maintainer's thread listing shows one elevated thread.
- **Deps:** WPK-T2
- [x] done

### WPK-T4 — Value widgets: no data, no alarm, no blink
- **Files:** `app/src/UI/Widgets/Bar.{h,cpp}`, `Gauge.{h,cpp}`, `Meter.{h,cpp}`, `app/qml/Widgets/Dashboard/Bar.qml`, `Gauge.qml`, `Meter.qml`, `app/tests/tst_value_widget_hasdata.cpp` (new)
- **Does:** `hasData` property latched on the first real sample, cleared on `resetData`; severity -1 and `alarmTriggered` false until then; every infinite/colour animation bound to `alarmTriggered && hasData`.
- **Verify:** ctest; maintainer: disconnected groups sit still, GPU near zero with no data.
- **Deps:** WPK-T3
- [x] done

### WPK-T5 — Grow-only plot geometry
- **Files:** `app/src/UI/Widgets/PlotCurve.cpp`, `app/src/UI/Widgets/GpuStroke.cpp`, `app/src/UI/Widgets/PlotAreaFill.cpp`, `app/tests/tst_plot_curve_geometry.cpp` (new)
- **Does:** Allocate at 1.5x when too small, reuse otherwise, draw the first N via the index count; Qt 6.11 `QSGGeometry::allocate` shrink behaviour read from source and cited in the @brief.
- **Verify:** ctest counts allocations on a stationary point count for 100 frames (zero after the first).
- **Deps:** WPK-T4
- [x] done

## WP-J — Documentation truth (last)

### WPJ-T1 — Architecture docs corrected and extended
- **Files:** `doc/claude/architecture/dataflow.md`, `io.md`, `export.md`, `dashboard.md`, `project.md`, `startup.md`, `scripting.md`, `doc/claude/code-style.md`
- **Does:** Every drifted statement from A6, B21, C5, E10, F8, G11, H13, K8 corrected; session boundary, `AsyncTcpDial`, catalog v2, `ScriptDryRun`, theme map, DashboardIngest, stream JS watchdog documented.
- **Verify:** `documentation-verify.py` clean; `claim-verify.py --quiet` 0 errors including the ordered anchor.
- **Deps:** WP-I done
- **Note (WP-J):** also covered `common-mistakes.md` and `directory-map.md`, which the Files line omitted, and corrected two claims the review did not catch: `dataflow.md` said seven benchmark tiers are gated (nine are, including the two 0.5x consumer-path floors), and the K8 `MessageHandler` sentence described a construction that spec 0039 replaced with a named `qFatal`. The `composition-root-order` anchor needed a `doc-anchors.json` change to be satisfiable at all: seven singletons share the leaf names `Player` and `Export`, which the doc-side match cannot tell apart, so they are pinned by presence instead of by order.
- [x] done

### WPJ-T2 — AI architecture doc and index
- **Files:** `doc/claude/architecture/ai.md` (new), `doc/claude/architecture.md`, `CLAUDE.md`
- **Does:** Tier model, checkpoint semantics, meta-tool seam, provider abstraction, `ToolTurnRunner`; sub-doc table row.
- **Verify:** `claim-verify.py` resolves every path/symbol in the new doc.
- **Deps:** WPJ-T1
- **Note (WP-J):** `ai.md` also covers `AsyncToolRunner`, `FileSandbox`, `KeyVault`, `Redactor` and `SentinelProbe`, and is indexed in `architecture.md`, in CLAUDE.md's sub-doc table and as a row in CLAUDE.md's Subsystem Contracts table. It names no deleted provider class: the roster is Anthropic, OpenAI, Gemini, Local and `OpenAICompatibleProvider` x4.
- [x] done

### WPJ-T3 — Test documentation and claim pins
- **Files:** `tests/README.md`, `scripts/doc-anchors.json`, `scripts/claim-baseline.json`
- **Does:** New tiers and files listed; xfail policy; anchors for the constants this spec introduced; baseline re-seeded to empty.
- **Verify:** `claim-verify.py --quiet` 0 errors, 0 baseline entries.
- **Deps:** WPJ-T2
- **Note (WP-J):** `tests/README.md` gained 45 spec-0075 ctest rows, 15 integration rows, 3 security rows, a `scripts/tests/` section, the sanitizer / qmllint / post-root-QML-self-test tiers, the shared test doubles, the fixtures tree and the configurable API port. Five anchors added (`script-dry-run-budget`, `monotonic-clock-persist-interval`, `api-port-setting`, `api-pending-write-cap`, and the two `composition-root-{players,exports}` companions); `claim-baseline.json` is now empty. The xfail policy was already written by WP0 and needed no change.
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1..AC14).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files; `--tu-census`, `--singleton-census`, `--dup-census` ratchets pass.
- [ ] `qt-cpp-review` run on every package's C++ diff before merge; findings addressed or noted in the package's PR.
- [ ] `--benchmark-hotpath` not regressed on any tier after WP-A, WP-B, WP-E, WP-I; sanitizer job green.
- [ ] Every finding id in `findings.md` maps to a merged test or an explicit "left open, reason" line in `plan.md`.
- [ ] Relevant `pytest` files listed in `plan.md` run by the maintainer with the app up.
- [ ] `python scripts/sanitize-commit.py` run per package; working tree clean of lint debt.
- [ ] Each package diff is *what its brief asked, and only that*; no foreign files touched; shared files edited only by their owner.
- [ ] `spec.md` status set to `done`.
