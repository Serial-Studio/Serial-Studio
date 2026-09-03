# Handoff — WP0 (CI integrity, tooling, harnesses)

All 21 tasks (WP0-T1 .. WP0-T21) are done and ticked in `tasks.md`. Every gate the `lint` job
runs is green on this branch, plus `reuse lint` and the new `scripts/tests/` suite (207 tests).

## Files changed

### CI workflows
| Path | Change |
|------|--------|
| `.github/workflows/ci.yml` | 49 action pins to 40-hex SHAs (resolved live from the GitHub API, each with a `# vX.Y.Z` comment); `upload` guarded to master/tags with a per-ref concurrency group and gated on `test`+`lint` plus a per-platform benchmark verdict; `test` now needs the four build jobs and downloads the packaged artifact instead of a Release; every secret moved into step `env:`; `security-events: write` dropped from four jobs; `permissions:` added to `lint`; `~/.qt-license` replaces the never-defined `${{env.USERPROFILE}}`; training activation is now fatal on failure; a FORTIFY compile-line assert step; ctest on macOS-arm64 and Windows; a `sanitize` job (ASan+UBSan+fuzzers, TSan leg, corpus replay, instrumented benchmark, GPL QML selftest); four `benchmark-retry-*` jobs; qmllint step + report artifact; QML instantiation selftest; `--require-hashes` installs; `pytest scripts/tests/` and `--dup-census --check` in `lint` |
| `.github/workflows/docs.yml` | Action pins, `permissions:`, hashed-lock install |

### Build
| Path | Change |
|------|--------|
| `cmake/Hardening.cmake` | `_ss_apply_fortify()` emits `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=<n>` as one ordered `target_compile_options` pair (L1: a level passed as a *definition* lands in `<DEFINES>` and is cancelled by the later `-U` in `<FLAGS>`); level 3 gated on a configure-time `check_cxx_source_compiles` probe of `__GLIBC__`/`__GLIBC_MINOR__` plus GCC/Clang >= 12 |
| `cmake/Sanitizers.cmake` | `ENABLE_FUZZERS` option (Clang-only, `-fsanitize=fuzzer-no-link`) |
| `app/CMakeLists.txt` | Five duplicate entries removed; `include(GNUInstallDirs)` made explicit; `ss_qmllint` target; `src/SelfTest/QmlInstantiationSuite.cpp` registered |
| `CMakeLists.txt` | `ar`/`ranlib` located with `find_program` instead of hardcoded `/usr/bin` |
| `app/tests/CMakeLists.txt` | `ss_add_unit_test` fails configure on a missing source; new `ss_add_fuzz_target()`; `tst_test_doubles` suite |
| `lib/CMakeLists.txt`, `cmake/MiMalloc.cmake` | zlib / expat / libusb-cmake / mimalloc `GIT_TAG` pinned to commit SHAs (see the deviations below) |
| `lib/luajit/CMakeLists.txt` | Comment points at `lib/VERSIONS.json` for the commit `.relver` corresponds to |
| `lib/VERSIONS.json` (new) | Upstream project, version, ref and the in-tree file that asserts it, for all ten vendored trees |
| `REUSE.toml` | Blanket annotation extended to `lib/VERSIONS.json` and `app/qml/qmllint-baseline.json` |

### Tooling
| Path | Change |
|------|--------|
| `scripts/code-verify.py` | Bare invocation now runs `--check` (L5); `--dup-census --check/--accept` clone ratchet; `scripts/tests/fixtures` excluded from the walked trees; `tu-cutter.py` dropped from three advisory texts |
| `scripts/code_verify_rules.py` | New error kind `qt-disconnect-wildcard` with a per-file count baseline |
| `scripts/claim-verify.py`, `scripts/doc-anchors.json` | New `"kind": "ordered"` anchor; first use pins `instantiateCoreModules()` against `startup.md` |
| `scripts/claim-baseline.json` | The nine ordered-anchor findings accepted until WP-J corrects the doc |
| `scripts/dup-census.json` (new) | 2444 shared windows over 33 file pairs, Gauge/Meter worst at 162 |
| `scripts/tu-cutter.py` | Deleted |
| `doc/claude/scripts.md` | tu-cutter row removed; `--dup-census` documented; `lib/VERSIONS.json` referenced |
| `lib/QSimpleUpdater/` | `CLAUDE.md`, `sanitize-commit.sh`, `tests/`, `tutorial/` deleted; `CMakeLists.txt` and `README.md` de-referenced |

### Product code
| Path | Change |
|------|--------|
| `app/src/SelfTest/SelfTest.h/.cpp` | Second, post-root suite registry; the run loop factored so both registries share it |
| `app/src/SelfTest/QmlInstantiationSuite.cpp` (new) | Instantiates every compiled `.qml` against stubs for every `Cpp_*` name scraped from the QML sources; fails only on `ReferenceError` |

### Tests
| Path | Change |
|------|--------|
| `scripts/tests/test_ci_workflow.py` (new) | 36 assertions over both workflows plus the xfail policy |
| `scripts/tests/test_code_verify.py` (new) | Fixture-driven linter tests + three ratchets |
| `scripts/tests/fixtures/<kind>/{good,bad}.*` (new) | 82 hand-authored fixture pairs, one per rule kind |
| `app/tests/support/Fake{Driver,Provider,Transport}.{h,cpp}` (new) | The shared doubles, exactly the interfaces the brief specified |
| `app/tests/tst_test_doubles.cpp` (new) | Seven smoke tests over the three doubles |
| `app/tests/fuzz/{README.md,CorpusReplayMain.cpp,corpus/README.md}` (new) | Fuzz-target convention and the corpus-replay QTest harness |
| `tests/requirements.txt`, `tests/requirements.lock` (new) | `PyYAML` + `reuse` added; hash-pinned universal lock for Python 3.11 |
| `tests/pytest.ini`, `tests/README.md` | `dos` marker registered; xfail policy and the fuzz tier documented |
| `tests/security/test_access_control.py` | Three "by design, not a finding" xfails deleted with their tests |
| `tests/security/test_unknown_input_hardening.py` | The stress-crash xfail now names spec 0075 I1 |

## Verification run on this branch

```
code-verify --check                 0 errors, 10 advisory (unchanged baseline)
code-verify --singleton-census      pass      code-verify --tu-census   pass
code-verify --dup-census --check    pass (2444, seeded)
claim-verify --quiet                pass (9 ordered-anchor findings baselined)
documentation-verify --quiet        pass      registry-verify           pass
generate-{command-strings,property-registry,sdk} --check   pass
reuse lint                          compliant
pytest scripts/tests/               207 passed
pytest --collect-only tests/{security,scripts}   530 collected, clean
yaml.safe_load on both workflows    ok (14 jobs / 1 job)
```

Not runnable here (no Qt, no compiler, per the brief): ctest, the qmllint gate, `--selftest qml`,
and the sanitize job. The FORTIFY assert step and the qmllint comparison were exercised against
synthetic inputs and behave correctly in both the pass and fail directions.

## Patches for the coordinator

### 1. `app/src/Misc/CLI.h` + `app/src/Misc/CLI.cpp` (owned by WP-H)

The post-root selftest hook. `CLI::process()` runs *before* the composition root, so the hook
cannot literally live inside it; the entry points go on `CLI` and `main()` calls them after
`bootstrapModuleManager()`. `SelfTest::Runner::postRootSuiteNames()` /
`runPostRootAndReport()` already exist on this branch.

`CLI.h`, in the public accessor block:

```cpp
  [[nodiscard]] bool postRootSelfTestRequested() const;
```

`CLI.h`, beside `runSelfTests()` in the private method block:

```cpp
  ProcessResult runPostRootSelfTests();
```

`CLI.cpp`, replacing the pre-root dispatch:

```cpp
#ifdef SS_INAPP_TESTS
  if ((m_parser.isSet(m_opts.selftestOpt) || m_parser.isSet(m_opts.selftestSuiteOpt))
      && !postRootSelfTestRequested())
    return runSelfTests();
#endif
```

`CLI.cpp`, next to `runSelfTests()`:

```cpp
/**
 * @brief True when --selftest-suite names a suite that needs the composition root. main() runs
 *        those after ModuleManager has built the modules; CLI::process() is too early.
 */
bool CLI::postRootSelfTestRequested() const
{
#ifdef SS_INAPP_TESTS
  const QString suite = m_parser.value(m_opts.selftestSuiteOpt).trimmed();
  return !suite.isEmpty() && SelfTest::Runner::postRootSuiteNames().contains(suite);
#else
  return false;
#endif
}

#ifdef SS_INAPP_TESTS
/**
 * @brief Runs the post-root suite named by --selftest-suite.
 */
CLI::ProcessResult CLI::runPostRootSelfTests()
{
  const QString suite = m_parser.value(m_opts.selftestSuiteOpt).trimmed();
  const int rc        = SelfTest::Runner::runPostRootAndReport(suite);
  return rc == EXIT_SUCCESS ? ProcessResult::ExitSuccess : ProcessResult::ExitFailure;
}
#endif
```

`postRootSelfTestRequested()` must sit OUTSIDE the `#ifdef SS_INAPP_TESTS` block so `main()`
needs no `#ifdef`.

### 2. `app/src/main.cpp` (owned by WP-H)

Inside the `ModuleManager` scope, replacing the `applyProjectAndAutoConnect` .. `app.exec()` run:

```cpp
    if (cli.postRootSelfTestRequested()) {
      status = cli.runPostRootSelfTests() == Misc::CLI::ProcessResult::ExitSuccess
                 ? EXIT_SUCCESS
                 : EXIT_FAILURE;
    } else {
      cli.applyProjectAndAutoConnect(app);
      // ... the existing applyOperatorRuntimeSettings / applyExportToggles /
      //     applyVisualizationOptions / applyBusConfiguration calls, unchanged ...
      status = app.exec();
      teardownTrace("event-loop-exited");
    }
```

### 3. `CLAUDE.md` line 316 (owned by WP-J)

WP0-T16's verify (`grep -rn tu-cutter scripts doc CLAUDE.md` empty) needs this last mention gone.
Replace the hazard cell's final sentence:

- before: ``Never split one class across TUs; `tu-cutter.py` is retired for class work.``
- after: `Never split one class across TUs: decompose into member sub-objects instead.`

## Deviations from tasks.md, and why

1. **WP0-T8 — retry design.** `tasks.md` says the retry jobs `needs` the build job and rerun the
   gate; `plan.md` says a two-attempt runner. Neither works verbatim: a hard-failing benchmark
   step aborts the build job before packaging, so a retry job has nothing to download and
   `upload` could not publish anyway. Implemented as: the in-job gate is `continue-on-error`
   with `id: benchmark`, the job exposes `outputs.benchmark`, and `benchmark-retry-<os>` runs
   `if: needs.<build>.outputs.benchmark == 'failure'` against the **packaged** artifact (no Qt
   install, no rebuild). `upload` uses `always()` and requires, per platform, that the in-job
   attempt OR its retry passed. This is stricter than before, not softer.
2. **WP0-T3 vs `REQUIRE_TESTS_TO_PUBLISH`.** The `env` context is not available in a job-level
   `if:`, so the flag cannot gate `upload` there. It is enforced by `upload`'s first step
   instead, which is a real consumer; the default is now `true`.
3. **WP0-T13 — three baseline sites, actually nineteen.** The 4-arg wildcard
   `disconnect(a, nullptr, b, nullptr)` appears at 19 sites across 16 files, not 3. Making it an
   error with only S7/EthernetIp/DeviceManager exempted would have reddened CI on 16 unrelated
   sites. Baselined **by count per file** (survives edits above the call, unlike line numbers);
   a new wildcard anywhere, including a second one in a listed file, is an error. Verified both
   directions. The `qt-disconnect-nullptr` comment claiming the 4-arg form is "idiomatic Qt and
   explicitly NOT what the rule cares about" was corrected — it contradicted CLAUDE.md and C9/E8.
4. **WP0-T14 — no C++ pair crosses the threshold.** The census normalizes comments and
   whitespace but deliberately not identifiers, so it has zero false positives. S7/EthernetIp
   share 24 windows (below the 40 threshold) because the clones differ by identifier;
   Gauge/Meter share 174. Catching the S7/EIP family would need identifier-insensitive matching,
   which trades the zero-false-positive property away. WP-D/WP-I own E8 directly anyway.
5. **WP0-T18 — `CSD.cpp` is not a duplicate.** L9 reads `app/CMakeLists.txt:1569` and `:1610` as
   a duplicate; they are the mutually exclusive `if(WIN32)` and `elseif(UNIX)` branches. Left
   alone (the task's own verify line, "one branch each", already describes the current state).
   The five real duplicates removed were: `FrameConfig.h` in SOURCES *and* HEADERS,
   `ProjectApiSupport.h` twice in HEADERS, the redundant `WIN32_LEAN_AND_MEAN` block under
   `ENABLE_GRPC`, `set(CMAKE_CXX_STANDARD 20)` re-set from the root, and the UNIX-branch
   `install(TARGETS)`.
6. **WP0-T19 — the qmllint gate lives in `build-linux`, not `lint`.** qmllint needs Qt and a
   configured tree; the `lint` job has neither. The baseline ships **unseeded**
   (`"seeded": false`), so the step reports its findings and passes. Seeding is one paste plus a
   flag flip and the instructions are in the file; leaving it unseeded avoids reddening CI over
   warnings nobody has read. This is the one gate that is not yet armed.
7. **WP0-T20 — the hook is in `main()`, not `CLI::process()`.** See patch 1.
8. **`id-placement` is a dead rule (new finding).** `_check_shallow_id()` in `code-verify.py`
   breaks out of its walk at the first content line, so `shallow_id_idx` can only ever equal
   `first_content_idx` and the `id-placement` branch is unreachable — no input trips it,
   confirmed empirically against several QML shapes. It is listed in `UNFIXTURED` with that
   reason rather than quietly given a fixture. Worth a WP-I task.

## Files edited outside my task's Files list

Each was required to make a listed task actually work; none is claimed by another package.

- `app/src/SelfTest/SelfTest.h` — the second registry has to be declared somewhere (T20).
- `app/tests/tst_test_doubles.cpp` — T21's own Verify line names it.
- `lib/CMakeLists.txt` — deleting `QSimpleUpdater/tests` and `tutorial` (T17) leaves the two
  `set(QSIMPLE_UPDATER_BUILD_* OFF ...)` forces pointing at options that no longer exist; also
  carries the zlib/expat/libusb `GIT_TAG` pins, which T18/`plan.md` assumed were in the root
  `CMakeLists.txt`.
- `lib/QSimpleUpdater/CMakeLists.txt` and `README.md` — same deletion; leaving them would have
  made the configure fail on missing `add_subdirectory(tutorial)` and `tests/*.cpp`.
- `cmake/MiMalloc.cmake` — the mimalloc `GIT_TAG` pin T18 asks for.
- `REUSE.toml` — T17 lists it; used for `lib/VERSIONS.json` and `app/qml/qmllint-baseline.json`.

**Kept, against the task text:** `lib/QSimpleUpdater/etc/` is NOT foreign tooling — the build
reads `etc/resources/qsimpleupdater.qrc` and `etc/resources/version.rc.in`. Deleting it would
break the build. Only the vendored project's `CLAUDE.md`, `sanitize-commit.sh`, `tests/` and
`tutorial/` are gone.

## Invariants found that the plan did not state

- **`code_verify_rules._has_attribute()` scans 200 bytes *before* the node.** A `[[nodiscard]]`
  on the preceding declaration masks the rule for the next one, so `qt-missing-nodiscard` has a
  false-negative window of one declaration. The fixture is written around it; the rule is worth
  a look.
- **`_HOTPATH_METHODS` is keyed on the function NAME, not the file path.** Any function called
  `processData`, `updateData`, `pushSample`, ... anywhere in `app/src` gets the hotpath rules.
- **`_strip_strings_and_line_comments()` runs before every regex rule**, so a rule whose pattern
  needs a string literal (`QString("...")`) can never fire. `perf-string-alloc-hotpath` is only
  reachable through the conversion half of its pattern (`.toUtf8()` and friends).
- **`CMAKE_INSTALL_BINDIR` reached `app/CMakeLists.txt` only by accident**: nothing there
  included `GNUInstallDirs`; the variable existed because `lib/mbedtls`, `lib/KissFFT`,
  `lib/hidapi` and `lib/QSimpleUpdater` include it and `lib/` is processed first. Now explicit.
- **`.gitattributes` normalizes the whole tree to LF**, so a CRLF fixture cannot survive
  checkout. The `line-endings` rule is covered by a test that synthesizes the bytes instead.
- **`qmllint` is not reachable from the `lint` job** and `--selftest` suites cannot see the QML
  engine, which is what forced both gates into build jobs.
- **`--benchmark-hotpath` is NOT gated on `SS_INAPP_TESTS`** (only `--selftest` is), which is
  why the retry jobs can run it against a packaged binary.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "Never touch, revert, or restore files
outside your own edits", via the six out-of-list files above and the `lib/QSimpleUpdater`
deletions — this package deletes more files than any other in the spec.

**Evidence it does not:** every deletion is named in WP0-T16/T17's own Files lists
(`scripts/tu-cutter.py`, `QSimpleUpdater/{CLAUDE.md,sanitize-commit.sh,tests,tutorial}`), and
each of the six out-of-list edits is the minimum needed to keep a listed change from breaking
the configure — enumerated above with its reason, and none is in another package's Files list
(checked against `plan.md`'s per-package tables). I deleted `lib/QSimpleUpdater/.clang-format`
and `etc/screenshots/` mid-pass, then **restored both from `HEAD`** once I confirmed they were
outside T17's scope (`lib/` is never clang-formatted, and the README links the screenshots), so
they do not appear in the diff. Nothing in the working tree that I did not author was reverted:
`git status` lists exactly the paths in the tables above. The second-nearest risk is the
`--fix` → `--check` default flip silently changing a workflow somebody relies on:
`sanitize-commit.py` passes `--fix` explicitly (line 231, unchanged), and
`test_explicit_fix_still_writes` pins that the writing path still writes.
