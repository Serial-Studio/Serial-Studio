---
spec: 0075-review-remediation
package: WP-B (script execution deadlines)
status: complete
updated: 2026-09-01
---

# Handoff WP-B — Script execution deadlines

All six tasks (WPB-T1..T6) are done and ticked in `tasks.md`. `python3 scripts/code-verify.py
--check` is clean (0 errors, 0 advisories) on every changed and new file; `clang-format` ran on
every C++ file. Nothing was built, launched or committed.

## Files changed

| File | Change |
|------|--------|
| `app/src/DataModel/Scripting/LuaDeadlineHook.{h,cpp}` (new) | The one Lua deadline: a `LUA_MASKCOUNT` count hook (10000 instructions) over a `QDeadlineTimer`, raising `"<label> timed out after <n> ms"`. Owning form (`install`/`arm`/`disarm`/`timedOut`) plus a static `bind(L, QDeadlineTimer*, budget, label)` / `enable(L)` pair for owners whose timer lives elsewhere. |
| `app/src/DataModel/Scripting/ScriptDryRun.{h,cpp}` (new) | Throwaway deadline-guarded script session: JS (`QJSEngine` + `JsWatchdog`) or Lua (sandboxed `lua_State` + `LuaDeadlineHook`). `evaluate` / `call` / `runLuaChunk` / `callLua` / `timedOut` / engine+state accessors for probes, plus a one-shot `runJsDryRun(code, prelude, budget)` returning `ScriptDryRunResult{ok, timedOut, line, error}`. `kScriptDryRunBudgetMs = 2000`. |
| `app/src/DataModel/FrameBuilder/TransformCompiler.cpp` | Adopts the helper: the inline hook function, its instruction constant and the `__ss_transform__` registry entry are gone; `bind()` runs inside the existing protected bootstrap, `enable()` only in Safe mode. Budget, deadline ownership (`TransformEngine::luaDeadline`, still armed by `FrameBuilder`) and error text are unchanged. |
| `app/src/API/Handlers/ControlScriptHandler.cpp` | `controlScript.dryRun` re-based on `ScriptDryRun`; the bridge stub and SDK prelude are now guarded too. Response fields and the timeout message are byte-identical. |
| `app/src/API/Handlers/ProjectDryRunCommands.cpp` | `painterDryRun` and `outputWidgetDryRun` run in a session; a top-level timeout returns `SCRIPT_TIMEOUT`, and `runOutputWidgetSample` calls `transmit()` through the watchdog, reporting `sampleRun.timedOut`. |
| `app/src/DataModel/Editors/ControlScriptEditor.cpp` | `evaluate()` (Validate) guarded; a timeout shows a message and keeps the editor alive. |
| `app/src/DataModel/Editors/DatasetTransformEditor.cpp` | `validateTransform` and `testTransform` guarded in both languages; the two duplicated safe-lib tables and all the `lua_close` bookkeeping are gone (the session owns the state). |
| `app/src/DataModel/Dialogs/TransmitTestDialog.cpp` | Compile and `transmit()` call guarded; timeouts render as dialog output. |
| `app/src/DataModel/Scripting/MacroRunner.cpp` | `verifyJs` guarded (a macro source that closes the wrapper early does execute). Its Lua paths were already hooked and are untouched. |
| `app/src/MQTT/PublisherScriptEditor.cpp` | `definesMqttFunction` and `runScript` guarded in both languages; the previously unhooked `lua_pcall` at the preview path now runs under the deadline. |
| `app/CMakeLists.txt`, `app/tests/CMakeLists.txt` | Two new sources + two headers appended contiguously; two new ctest suites appended contiguously at the end of the suite list. |

## Tests added

- `app/tests/tst_lua_deadline_hook.cpp` (`tst_lua_deadline_hook`, links `LuaDeadlineHook.cpp` +
  `SSAssert.cpp`, `Qt6::Core` + `luajit`): bounded chunk untouched; `while true do end` becomes
  `LUA_ERRRUN` at/after the budget; message names label and budget; state reusable afterwards;
  never-armed state is never cut off; the external-deadline (`bind`) form, i.e. the transform
  lane's shape.
- `app/tests/tst_script_dryrun.cpp` (`tst_script_dryrun`, links `ScriptDryRun.cpp`,
  `LuaDeadlineHook.cpp`, `JsWatchdog.cpp`, `JsWatchdogThread.cpp`, `SSAssert.cpp`; `Qt6::Core`,
  `Qt6::Qml`, `luajit`): JS evaluate + call timeouts with engine reuse, syntax error is not a
  timeout, Lua chunk + call timeouts, sandbox has no `io`/`os`, and the three `runJsDryRun`
  outcomes.
- `tests/integration/test_script_deadlines.py` (9 tests, `@pytest.mark.project`): control-script
  dry run, painter dry run, output-widget compile and sample run, transform dry run (JS+Lua),
  frame-parser dry run (JS+Lua), and a frame-lane transform timeout end to end. Every case
  asserts a live command afterwards. **I did not run it**: the app on `127.0.0.1:7777` is the
  pre-fix master build, where `project.painter.dryRun` with `while(true){}` is exactly the
  permanent freeze being fixed. I did validate every command name, parameter name and response
  shape the file asserts against the live app using benign scripts (`valid`, `sampleRun.ok`,
  `outputs`, `frameCount`, `controlScript.getStatus`), and `--collect-only` is clean.

## Patches for the coordinator (files outside my list)

**1. `app/src/API/CommandProtocol.h`** — add the shared error-code constant the plan lists
(`SCRIPT_TIMEOUT`). WP-G also adds codes to this namespace, so it is one merge:

```diff
 namespace ErrorCode {
 constexpr const char* InvalidJson        = "INVALID_JSON";
 constexpr const char* InvalidParam       = "INVALID_PARAM";
 constexpr const char* MissingParam       = "MISSING_PARAM";
 constexpr const char* UnknownCommand     = "UNKNOWN_COMMAND";
 constexpr const char* ExecutionError     = "EXECUTION_ERROR";
 constexpr const char* OperationFailed    = "OPERATION_FAILED";
 constexpr const char* InvalidMessageType = "INVALID_MESSAGE_TYPE";
+constexpr const char* ScriptTimeout      = "SCRIPT_TIMEOUT";
 
 }  // namespace ErrorCode
```

Once it lands, in `app/src/API/Handlers/ProjectDryRunCommands.cpp` delete the file-local
`kScriptTimeoutCode` and replace both `QString::fromLatin1(kScriptTimeoutCode)` uses with
`ErrorCode::ScriptTimeout`. Until then the wire value is identical, so no test changes.

**2. `doc/claude/architecture/scripting.md` (WP-J)** — the "JS interruption is cross-thread"
paragraph should gain the Lua counterpart and the new seam: *one* Lua mechanism
(`DataModel::LuaDeadlineHook`, count hook + `QDeadlineTimer`, never installed in Fast mode) and
*one* GUI-thread entry point for throwaway evaluations (`DataModel::ScriptDryRun`, 2 s budget),
used by every editor validate/test path, the transmit and MQTT previews, the macro verifier and
the API dry runs.

**3. `tests/README.md` (WP-J)** — add `tst_lua_deadline_hook`, `tst_script_dryrun` and
`tests/integration/test_script_deadlines.py` to the catalog.

**4. WP-A (`StreamWorker`, WPA-T8)** — no dependency on `ScriptDryRun`; the stream lane owns
long-lived engines, so it wants `JsWatchdog` directly (arm once per block, `lastCallTimedOut()`)
exactly as `TransformCompiler` does. If WP-A wants to drop `StreamWorker`'s inline Lua hook, the
drop-in is: keep the `QDeadlineTimer` member, call
`LuaDeadlineHook::bind(L, &m_luaDeadline, kWatchdogMs, "transform")` + `LuaDeadlineHook::enable(L)`
in place of `lua_sethook` (Safe mode only), and delete the file-local hook function. Behaviour is
identical except the message becomes `"transform timed out after <n> ms"`.

## Invariants found that the plan did not state

- **`TransformEngine::luaDeadline` is armed by `FrameBuilder.cpp`, not by the compiler**, and
  `TransformCompiler.h` is not in WP-B's file list — so the helper had to support a
  caller-owned timer (`bind(lua_State*, QDeadlineTimer*, ...)`), not just an owned one. That is
  why the class has both forms.
- **Registry binding must happen inside the existing protected bootstrap.** The context is Lua
  userdata, so binding allocates; doing it after the bootstrap `lua_pcall` (where the old
  `lua_pushlightuserdata` sat) would put an allocation outside every protected frame, and
  LuaJIT reaches `lua_atpanic` on allocation failure. `bind()` now runs inside the bootstrap and
  `enable()` (allocation-free) after it.
- **A finished call must never be reported as a timeout.** `QDeadlineTimer::hasExpired()` alone
  says "the budget elapsed", not "the run was cut off", so `callLua` reports `timedOut` only when
  the pcall also failed — the same distinction `JsWatchdog::finishCall` makes with elapsed time.
- **Dry-run Lua now runs with the JIT off.** The editors previously created states with LuaJIT's
  default (JIT on), where a count hook never fires inside a compiled trace; an interruptible
  validation requires interpreter mode. Slower, bounded, and matches Safe-mode runtime semantics.
- **Four Lua hook implementations remain** (`LuaScriptEngine`, `MacroRunner`, `MQTT/PublisherScript`,
  `IO/StreamWorker`), all correct and all outside WP-B's file list. They are the natural WP-I
  "one implementation per concern" follow-up now that `LuaDeadlineHook` exists.
- **Known gap, unchanged by this work:** a Lua hook is per-`lua_State`, and `coroutine.create`
  makes a new state that does not inherit it, so a runaway inside a coroutine escapes every one of
  these deadlines. That was already true of all four existing hooks; worth a spec of its own.

## Behaviour deltas a reviewer should expect

- Lua chunk names in dry-run error messages change from the code snippet form
  (`[string "function transform(v)..."]:2:`) to the short form (`[string "transform"]:2:`) because
  `luaL_dostring` became `luaL_loadbuffer` with an explicit name. Line numbers are unchanged.
- Throwaway Lua states now carry a throwing `lua_atpanic` handler (the editors had none), matching
  `LuaScriptEngine`/`MacroRunner`. A panic during the caller's own installs is still unprotected —
  the same exposure as before, and a candidate for a protected bootstrap in WP-I.
- `project.painter.dryRun` / `project.outputWidget.dryRun` gain one failure mode (`SCRIPT_TIMEOUT`)
  and one response field (`sampleRun.timedOut`). No field was removed or renamed.
- The dry-run sandbox does **not** strip `load`/`dofile`/`string.dump`, matching what the editors
  did before; the runtime transform sandbox does strip them. Aligning the two is a deliberate
  non-goal here (WP-F owns the sandbox-sharing task) and is flagged for it.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "TransformCompiler behaviour is unchanged after
adopting the helper" — the hotpath Lua transform lane runs through code I rewrote, and a silent
change there (a hook that never fires, or one that fires in Fast mode) would either remove the
watchdog or destroy JIT throughput without any test failing.

**Evidence it does not:** the instruction period is the same literal 10000 (now
`kLuaHookInstructionCount`), the budget is still `kTransformWatchdogMs` (100), the watched timer is
still `TransformEngine::luaDeadline` — the same member `FrameBuilder::applyTransformLua` arms and
resets, untouched by this diff — and the raised text is byte-identical
(`"%s timed out after %d ms"` with `"transform"`/100 reproduces `"transform timed out after 100
ms"`, pinned by `tst_lua_deadline_hook::timeoutMessageNamesLabelAndBudget`). The Fast/Safe branch is
unchanged: `enable()` is called only in the `else` arm of `if (projectModel.luaFastMode())`, and
`bind()` (which only registers a pointer, no hook) is what moved into the bootstrap. The removed
`__ss_transform__` registry entry was read by exactly one function, the hook I deleted (verified by
grep over `app/`).

**Second risk:** `setInterrupted(true)` off `JsWatchdogThread.cpp` (lint
`js-interrupt-off-thread`). `ScriptDryRun` only ever *clears* the flag
(`setInterrupted(false)` after a guarded evaluate); every interrupt is still raised by the shared
watchdog thread. `code-verify.py --check` on all 16 files reports no violation.
