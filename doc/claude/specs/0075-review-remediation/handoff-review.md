# Handoff — coordinator review pass and final integration (2026-09-02)

Every package (WP0, WP-A..I, WP-K, WP-J) is merged into the repository working tree. This file
records the `qt-cpp-review` pass over the integrated C++ diff and the fixes applied to it.

## What ran

`code-verify.py --check` over the 322 changed C++ files as Phase 1 (0 errors, 9 TU-length
advisories), then six parallel read-only analysis agents. Three of the six (thread-safety,
error-handling, performance) died on provider rate limits partway through; their partial output
is folded in below and the areas they did not reach are listed as unreviewed.

## Fixed in this pass

| id | file | what |
|----|------|------|
| D-402 | `UI/Widgets/Waterfall/WaterfallRingTexture.cpp` | The full-image upload wrapped `constBits()` in a non-owning `QByteArray::fromRawData` and then cleared the staging image in the same call, so the deferred `QRhiResourceUpdateBatch` read a buffer whose only owner was the GUI thread's live `QImage`. Now built from the `QImage` overload, which the batch holds by refcount. |
| D-401 | `app/tests/fuzz/fuzz_api_json.cpp` | `fuzzApplication()` constructed a second `QCoreApplication` inside the corpus-replay binary, whose `QTEST_GUILESS_MAIN` had already made one; Qt asserts on that. It now reuses `QCoreApplication::instance()` when one exists. |
| (perf agent) | `API/Server/ServerAuth.cpp` | The consent prompt cleared its re-entrancy guard before opening the modal, so a second API write arriving during the nested event loop stacked a second prompt. The guard now drops only once the answer is recorded. |
| (coordinator) | `API/Handlers/ProjectFileCommands.cpp` | Leftover fragment in the `project.setTitle` description from the integration edit. |
| D-403 | `API/GRPC/GRPCServer.h` | `marshalToGui()` was declared in a fresh `public:` block after `private slots:`; moved into the existing `public:` block. |
| D-404 | `DeviceWriteVerdict.h`, `ConnectionManager.h`, `ScriptDryRun.h` | Trailing comma on the last enumerator. |
| D-405 | `ScriptDryRun.h` (+22 call sites) | `Language` is now an `enum class`; every call site rewritten mechanically. |
| D-407 | `Misc/CLI.h` | `[[nodiscard]]` on `runPostRootSelfTests()`. |
| D-408 | `Misc/ContextRegistry.h` | `QPair` (a bare `std::pair` alias since Qt 6) replaced by `std::pair`. |
| D-101 | `Sessions/ReportOptionsModel.cpp` | `roleNames()` rebuilt a twelve-entry hash per call; now a cached `static const`, matching `ExtensionRowsModel`. |
| (perf agent) | `AI/Assistant.cpp` | `redactedKey()` decrypted the stored key only to hand it to a function that ignores it; it now tests `hasKey()` and never materialises the secret. |
| (perf agent) | `API/Handlers/ModbusHandler.cpp` | Named the register-type ids and the two request ceilings. |
| (spec R9.4) | `app/qml/AI/KeyManagerDialog.qml` | The local-model context window shipped as a property with no user control, so the setting the requirement asks for was unreachable. Added the field beside the local-server URL row. |

## Reported and deliberately not changed

- **D-406** `SerialCanBackendBase::isFatalSerialError` uses a `default:` arm over
  `QSerialPort::SerialPortError`. Enumerating the remaining values is the stricter shape, but the
  fallback here is "not fatal", which is the safe direction for a value Qt adds later.
- **D-405 (partial)** `WaterfallColorMap::Map` and `Test::ReplyEvent::Kind` stay unscoped: the
  first travels through `sample(int, double)` and mirrors `Widgets::Waterfall::ColorMap`, the
  second is a test double's aggregate initialiser. Both conversions are wider than the nit.
- **I-401** `MonotonicClock::now()` still calls `currentDateTime()` rather than
  `currentDateTimeUtc()`; correctness is unchanged (comparisons are instant-based) and the
  consumer audit was not finished.
- **gRPC `ExecuteBatch`** appends a blank `CommandResponse` when an inner command is abandoned
  (confidence 70, agent died before tracing the consumer). Worth a look before release.
- `Assistant::localContextWindow()` was dead surface; the QML control above is the fix.

## Not reviewed (agents lost to rate limits)

Thread-safety and hotpath analysis covered the Waterfall render path and reached
DeviceManager/DeviceIoRouter before dying; **it did not finish FrameBuilder's session boundary,
BlockStager/BlockPublisher, StreamWorker, FrameConsumer, the export workers, PlaybackRing,
PolledPlcWorkerBase or PendingCall**. Error-handling analysis covered the AI and API groups only;
the IO drivers, sessions/export and test groups were not reached. A second pass over those areas
is the highest-value follow-up, and `--benchmark-hotpath` has not been run at all.
