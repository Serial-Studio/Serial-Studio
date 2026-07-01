---
spec: 0001-offline-activation
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-01
---

# Tasks 0001 — App-side offline activation (signed license certificate)

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Design revision (2026-07-01, post-review)

On maintainer direction the UX became a guided wizard and the repo layout changed. Net effect on
the tasks below:

- **T1 (signing CLI) is removed from this repo.** `tools/ss-license-sign` was deleted; signing is
  the portal's job. The self-test (T10) uses its own embedded vectors, so it still stands. A local
  Python signer (`mint_sslic.py`, uncommitted) exists only for maintainer testing.
- **New in-app pieces:** `OfflineLicense::exportMachineInfo` + `openActivationPortal`, and a
  `OfflineActivation.qml` wizard (export device file → portal → import), registered in CMake. This
  supersedes the "file import only" wording in T11.
- Everything else (verifier, MonotonicClock, token reuse, precedence, API, self-test) is unchanged.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how *this* unit is confirmed before moving on — `python scripts/code-verify.py
  --check <files>` for C++/QML, plus a test or read-back where one fits. C++ has no pytest path,
  so runtime verification of the verifier is the `--selftest-offline-license` flag (T10); live-app
  and UI checks (AC4–7) are maintainer-run and land at the end.
- **Deps** lists task IDs that must land first.
- **Hard invariant threaded through every task:** the Ed25519 **private** key never enters the
  repo, CI, or the shipped binary — only the 32-byte public key ships. No task weakens existing
  `CommercialToken` / HMAC / `SS_LICENSE_GUARD` logic.

## Tasks

### T1 — Signing CLI target + keypair generation (`ss-license-sign`)

- **Files:** `tools/ss-license-sign/main.cpp`, `tools/ss-license-sign/CMakeLists.txt`
- **Does:** New opt-in console target (Qt Core + `target_link_openssl`), **not** built by default
  and never installed/deployed. Generates an Ed25519 keypair (one-off, documented command);
  reads the **private** key from `SS_LICENSE_SIGNING_KEY` at runtime; mints a `.sslic` from CLI
  args (machine id, variant/tier, instance id, issued-at, expires-at) by CBOR-encoding the payload
  and appending the 64-byte signature, base64-framed. Emits the generated **public** key to stdout
  for T2. No private-key bytes are written to any repo file.
- **Verify:** `python scripts/code-verify.py --check tools/ss-license-sign/main.cpp`; manual: build
  the target opt-in, generate a keypair, mint a cert; confirm `SS_LICENSE_SIGNING_KEY` unset →
  clean error, never a committed key.
- **Deps:** none
- [x] done

### T2 — Embed the public key (`OfflinePublicKey.h`)

- **Files:** `app/src/Licensing/OfflinePublicKey.h`
- **Does:** Commit the 32-byte Ed25519 **public** key from T1 as a `constexpr` byte array. Header
  carries the commercial SPDX license block. Public-key-only — no private material, no I/O.
- **Verify:** `python scripts/code-verify.py --check app/src/Licensing/OfflinePublicKey.h`;
  read-back confirms exactly 32 bytes and no private key.
- **Deps:** T1
- [x] done

### T3 — Certificate parser + verifier (`OfflineCertificate`)

- **Files:** `app/src/Licensing/OfflineCertificate.h`, `app/src/Licensing/OfflineCertificate.cpp`
- **Does:** Pure, UI/settings-free free functions: `parseCertificate(bytes)` (base64 unframe →
  split CBOR payload + 64-byte signature), and `verifyCertificate(payload, publicKey, machineId,
  now)` running Ed25519 `EVP_DigestVerify`, then machine-id match and `exp > now`. Returns a typed
  result carrying a distinct reject reason enum (bad-signature / machine-mismatch / expired /
  malformed / wrong-tier) — never throws, never partially succeeds. Decodes CBOR fields per the
  R9 contract (`v/mid/var/iid/iat/exp`). Length-checks before any OpenSSL call.
- **Verify:** `python scripts/code-verify.py --check` on both files. Runtime coverage lands in T10
  (self-test vectors); this task is confirmed structurally + by read-back of the reject-reason
  taxonomy against R12.
- **Deps:** T2
- [x] done

### T4 — Shared monotonic anti-rewind clock (`MonotonicClock`)

- **Files:** `app/src/Licensing/MonotonicClock.h`, `app/src/Licensing/MonotonicClock.cpp`
- **Does:** Extract the "highest-observed-time" floor currently in `LemonSqueezy::monotonicNow()`
  into one shared helper over the same encrypted `licensing/lastSeen` key (same `SimpleCrypt` key
  derivation). Behavior-preserving.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back confirms identical
  logic (same key, same RFC2822 encode, same floor rule).
- **Deps:** none
- [x] done

### T5 — Delegate `LemonSqueezy::monotonicNow()` to `MonotonicClock`

- **Files:** `app/src/Licensing/LemonSqueezy.cpp`
- **Does:** Replace the body of `monotonicNow()` with a call to the T4 helper. No change to any
  validation / HMAC / grace-period behavior. Minimal, isolated edit to a sensitive file.
- **Verify:** `python scripts/code-verify.py --check app/src/Licensing/LemonSqueezy.cpp`; read-back
  confirms only `monotonicNow()` changed and the `licensing/lastSeen` key/semantics are unchanged.
- **Deps:** T4
- [x] done

### T6 — `OfflineLicense` singleton (import, persistence, startup re-verify)

- **Files:** `app/src/Licensing/OfflineLicense.h`, `app/src/Licensing/OfflineLicense.cpp`
- **Does:** New singleton mirroring `Trial`/`LemonSqueezy` (private ctor, `instance()`,
  `SimpleCrypt` over a new `offlineLicense/` group, QML props). `activateFromFile(path)` →
  `OfflineCertificate` verify (using `MachineID::machineId()` + `MonotonicClock::now()`) → on
  success build a `CommercialToken` (`setVariantName`, `setInstanceName(machineId)`,
  `setGraceDaysRemaining(daysToExpiry)`, `setFeatureTier(tierFromVariant)`, `seal()`,
  `setCurrent()`) and persist the raw cert; on failure set `lastError()`, leave state untouched.
  Exposes `machineId()`, `isActivated()`, `daysRemaining()`, `expiresAt()`, `variantName()`.
  `readSettings()` re-verifies + re-installs on startup, guarded by
  `!LemonSqueezy::instance().isActivated()`. `CommercialToken` itself is **unchanged**.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back confirms token build
  matches `installTrialToken` shape and the online-precedence guard is present.
- **Deps:** T3, T4
- [x] done

### T7 — Activation precedence (Trial defers to offline)

- **Files:** `app/src/Licensing/Trial.cpp`
- **Does:** One-line change: `trialAvailable()` also returns false when
  `OfflineLicense::isActivated()`, establishing online > offline > trial. No other Trial logic
  touched.
- **Verify:** `python scripts/code-verify.py --check app/src/Licensing/Trial.cpp`; read-back
  confirms the guard mirrors the existing `LemonSqueezy::isActivated()` check.
- **Deps:** T6
- [x] done

### T8 — Build wiring (commercial sources + opt-in tool target)

- **Files:** `app/CMakeLists.txt`
- **Does:** Add T2/T3/T4/T6 sources to the existing `BUILD_COMMERCIAL` source group (OpenSSL
  already linked at line 1070). Ensure `tools/ss-license-sign` is a separate, opt-in target not
  pulled into the default app build or deploy. Confirm GPL build excludes all new sources.
- **Verify:** `python scripts/code-verify.py --check app/CMakeLists.txt`; maintainer configures
  both `BUILD_COMMERCIAL=ON` (new sources present) and `BUILD_GPL3=ON` (all new sources excluded,
  still configures).
- **Deps:** T2, T3, T4, T6
- [x] done

### T9 — API: `licensing.activateOffline` + `getStatus` extension

- **Files:** `app/src/API/Handlers/LicensingHandler.h`, `app/src/API/Handlers/LicensingHandler.cpp`
- **Does:** Register `licensing.activateOffline` (param `path`) calling
  `OfflineLicense::activateFromFile`, returning success or a `makeError` whose message is the
  distinct T3 reject reason (R12). Extend `getStatus` with `offlineActivated`, `offlineExpiresAt`,
  `offlineDaysRemaining`, `machineId`. Commercial-only like the rest of the handler.
- **Verify:** `python scripts/code-verify.py --check` on both files; maintainer API check deferred
  to T11 (AC7).
- **Deps:** T6
- [x] done

### T10 — Headless verifier self-test (`--selftest-offline-license`)

- **Files:** app CLI entry (the `--benchmark-hotpath`-style flag site), + a self-test source under
  `app/src/Licensing/` if a dedicated unit is cleaner
- **Does:** Hidden flag that runs `OfflineCertificate` over vectors minted by `ss-license-sign`:
  good (accept), payload/signature bit-flip (reject: bad signature), wrong-machine (reject:
  mismatch), past-`exp` (reject: expired), rewound-clock-on-expired (still expired), truncated/
  garbage (reject: malformed). Deterministic, no Qt UI, no network; non-zero exit on any mismatch.
  Satisfies **AC1, AC2, AC3, AC8** headlessly.
- **Verify:** `python scripts/code-verify.py --check` on changed files; maintainer runs
  `<app> --selftest-offline-license` → all vectors pass.
- **Deps:** T3, T1
- [x] done

### T11 — QML: offline import + fingerprint + expiry surfacing

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/qml/Dialogs/LicenseManagement.qml`
- **Does:** Register `Cpp_Licensing_OfflineLicense` context property next to lines 620–621
  (construction triggers startup re-verify). In the dialog: "Activate offline…" button + `*.sslic`
  `FileDialog` wired to `activateFromFile`; a read-only, copyable machine-fingerprint field bound
  to `Cpp_Licensing_OfflineLicense.machineId` that renders **before** activation; days-remaining /
  expiry + approaching-expiry warning on the license page; rejected import surfaces `lastError()`
  via the existing message-box pattern. Satisfies **AC5, AC6** (maintainer UI check).
- **Verify:** `python scripts/code-verify.py --check` on both files; maintainer observes fingerprint
  pre-activation, successful `.sslic` import unlocks Pro, near-expiry warning shows, expired cert
  does not persist.
- **Deps:** T6, T9
- [x] done

### T12 — End-to-end integration test (AC4, AC7)

- **Files:** `tests/integration/test_offline_activation.py` (new)
- **Does:** Maintainer-run pytest against a live commercial build with the API server on: read
  `machineId` via `licensing.getStatus`; mint a cert for it with `ss-license-sign`;
  `licensing.activateOffline` unlocks a Pro-gated capability with **no outbound licensing request**;
  restart persists activation; each bad-cert class returns its distinct error. Satisfies **AC4,
  AC7**.
- **Verify:** Test file lints; maintainer runs `pytest tests/integration/test_offline_activation.py
  -v` with the app up (Settings → Miscellaneous → Enable API Server).
- **Deps:** T9, T10, T11
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` (AC1–AC8) is met and checked off there — AC1–3/AC8 via
      T10, AC4/AC7 via T12, AC5/AC6 via T11. **(Maintainer-run: requires building the app + the
      self-test flag + pytest; not verifiable by the agent.)**
- [x] `python scripts/code-verify.py --check` is clean on all changed files (0 errors, 0 advisories).
- [x] `qt-cpp-review` run on the C++ diff (6 agents); findings addressed (F1 file-size cap, online→offline
      token re-assert, verify-ordering invariant documented) or noted (daysRemaining display staleness — low).
- [x] Hotpath **not touched** — Agent 3 confirmed zero contact with FrameReader/CircularBuffer/FrameBuilder/
      Dashboard/span-lane and no cached-flag edits; startup + user-action path only. No `--benchmark-hotpath`.
- [x] clang-format applied to all changed C++ files (matches repo; untouched files produce zero diff).
- [ ] GPL build (`BUILD_GPL3=ON`) still configures with all new sources excluded; `BUILD_COMMERCIAL=ON`
      includes them; `ss-license-sign` stays opt-in and out of the default build/deploy. **(Maintainer-run.)**
- [x] Private-key invariant holds: only the 32-byte public key is committed (`OfflinePublicKey.h`); the
      private key was handed off to the user out-of-band and never written to the repo/CI/binary.
- [x] `pytest tests/integration/test_offline_activation.py` identified for the maintainer to run (app up,
      API server on; SS_OFFLINE_TEST_CERT set for the happy path).
- [x] Diff is *what was asked, and only that* — no scope creep. **Two working-tree files
      (`doc/help/Dataset-Transforms.md`, `tests/benchmarks/big_db_test/big_db_test.ssproj`) were modified by
      something other than this session and left untouched — flagged to the maintainer, not reverted.**
- [ ] `spec.md` status set to `done` — held at `in-progress` until the maintainer runs the build/self-test/
      pytest gates above (honest status: implemented + statically verified, not yet runtime-verified).

## Post-implementation notes (for the maintainer)

- **Verification you need to run** (I can't build/run): `<app> --selftest-offline-license` (AC1–3, AC8);
  build `tools/ss-license-sign` (`cmake -S tools/ss-license-sign -B build-signtool`), mint a cert for your
  machine id, import it (AC4–7 / T12 pytest); confirm the GPL build still configures.
- **Low-severity, deferred (not fixed):** `OfflineLicense::daysRemaining` only notifies on
  `activatedChanged` and uses the raw system clock for the *displayed* count (the expiry *gate* is
  monotonic-floored and secure). Impact is a transient dialog showing a stale day count across a midnight
  boundary. Fix options (QTimer refresh, or compute in QML from `expiresAt`) were judged heavier than the
  benefit — left for your call.
