---
spec: 0001-offline-activation
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-07-01
---

# Plan 0001 — App-side offline activation (signed license certificate)

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Design revision (2026-07-01, post-review)

The UX and repo layout changed after the first implementation pass, on maintainer direction:

- **"Activate Offline" is now a guided wizard** (`app/qml/Dialogs/OfflineActivation.qml`), not a
  bare file picker: Step 1 export a device file → Step 2 open the portal → Step 3 import the
  `.sslic`. The end user never touches a CLI.
- **`OfflineLicense` gains two slots:** `exportMachineInfo(path)` writes a `.ssmachine` JSON
  (`type/version/machineId/app/appVersion/generatedAt`) that the user uploads to the portal, and
  `openActivationPortal()` opens `https://cloud.serial-studio.com/offline-activation`.
- **The `tools/ss-license-sign` CLI was removed from this repo.** Signing belongs to the portal
  project (it holds the private key); this repo only *verifies*. The certificate format documented
  below is the contract that portal implements. The headless self-test (T10) carries its own
  embedded test vectors, so it does not depend on any in-repo signer.
- The Ed25519 **public** key stays a plain `constexpr` in `OfflinePublicKey.h` — it is not secret
  (verify-only), so it is not obfuscated; integrity of the key in official builds is covered by the
  existing `SS_LICENSE_GUARD`/HMAC machinery the offline token already flows through.

The sections below are the original plan; where they say "file import only" or reference
`ss-license-sign`, read them through this revision.

## Approach (one paragraph)

Add a new `Licensing::OfflineLicense` singleton alongside `LemonSqueezy` and `Trial`, following
the exact same shape (private ctor, `instance()`, `SimpleCrypt`-encrypted `QSettings`, QML
context property). It imports a `.sslic` file, verifies its Ed25519 signature against a **public
key compiled into the binary** using the already-linked OpenSSL `EVP_DigestVerify` API, checks
that the certificate's bound machine id equals `MachineID::machineId()` and that its `expiresAt`
is still in the future relative to a **monotonic, clock-rewind-resistant clock shared with the
online grace period**, and — only then — builds, `seal()`s, and installs a `CommercialToken`
exactly the way `LemonSqueezy::applyValidatedLicense()` and `installTrialToken()` already do. The
raw verified certificate is persisted encrypted and re-verified + re-installed on every startup
(so expiry is re-enforced each launch, giving R7 for free). Certificates are minted by a
separate, non-shipped `ss-license-sign` CLI target that holds the Ed25519 **private** key from an
environment variable. Everything is `#ifdef BUILD_COMMERCIAL`. No existing online/trial/HMAC/guard
logic changes except one small shared-clock refactor and one precedence guard, both called out
below.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Licensing/OfflineLicense.h` | **New.** Singleton: `activateFromFile(path)`, `machineId()`, `isActivated()`, `daysRemaining()`, `expiresAt()`, `variantName()`, rejection-reason enum + `lastError()`; QML properties. |
| `app/src/Licensing/OfflineLicense.cpp` | **New.** File read → base64 unframe → CBOR parse → Ed25519 verify → machine/expiry checks → build+seal+`setCurrent` token; `readSettings()`/`writeSettings()` persistence; startup re-verify. |
| `app/src/Licensing/OfflineCertificate.h` | **New.** Pure struct + free functions: `parseCertificate(QByteArray) -> {payload, sig}`, `verifyCertificate(bytes, publicKey, machineId, now) -> VerifyResult`. Kept UI/QSettings-free so it is unit-testable headless (AC1–3, AC8). |
| `app/src/Licensing/OfflineCertificate.cpp` | **New.** CBOR field decode, OpenSSL `EVP_DigestVerify` (Ed25519), field/expiry validation, typed reject reasons. |
| `app/src/Licensing/OfflinePublicKey.h` | **New.** The 32-byte Ed25519 **public** key as a `constexpr` byte array (public key only — safe to ship). Generated once, committed. |
| `app/src/Licensing/MonotonicClock.h` / `.cpp` | **New (small).** Extracts the anti-rewind "highest-observed-time" floor (currently `LemonSqueezy::monotonicNow()`) into one shared implementation over the same `licensing/lastSeen` key, so both paths share a single floor. |
| `app/src/Licensing/LemonSqueezy.cpp` | **Edit (minimal).** `monotonicNow()` delegates to `MonotonicClock`; behavior identical. Do **not** touch validation/HMAC logic. |
| `app/src/Licensing/Trial.cpp` | **Edit (1 line).** `trialAvailable()` also returns false when `OfflineLicense::isActivated()` — establishes precedence (online > offline > trial). |
| `app/src/API/Handlers/LicensingHandler.h` / `.cpp` | **Edit.** Register `licensing.activateOffline` (param: `path`) + extend `getStatus` with offline fields; distinct error per reject class (R12). |
| `app/src/Misc/ModuleManager.cpp` | **Edit.** `setContextProperty("Cpp_Licensing_OfflineLicense", &OfflineLicense::instance())` next to lines 620–621; construction here triggers startup re-verify. |
| `app/qml/Dialogs/LicenseManagement.qml` | **Edit.** "Activate offline" button + `FileDialog`, machine-fingerprint display (works pre-activation), offline expiry/days-remaining + approaching-expiry warning. |
| `app/CMakeLists.txt` | **Edit.** Add the four new licensing sources under the existing `BUILD_COMMERCIAL` source group (OpenSSL already linked via `target_link_openssl` at line 1070). |
| `tools/ss-license-sign/` (`main.cpp`, `CMakeLists.txt`) | **New, opt-in target.** Qt Core + OpenSSL console app; mints/signs a `.sslic` from CLI args + `SS_LICENSE_SIGNING_KEY` env. Not built by default, never installed/deployed. |

## Architecture & data flow

**Import (user action).** `LicenseManagement.qml` → `FileDialog` → `OfflineLicense::activateFromFile(path)`
(main thread): read file → base64-decode the framed block → `parseCertificate()` splits CBOR
payload + 64-byte signature → `verifyCertificate(payload, kOfflinePublicKey, MachineID::machineId(),
MonotonicClock::now())`. On success: decode tier from the payload's `variant` string (reuse the
`tierFromVariant` mapping; Lifetime/Enterprise both map to Pro/Enterprise), build a
`CommercialToken` (`setVariantName`, `setInstanceName(machineId)`, `setGraceDaysRemaining(daysToExpiry)`,
`setFeatureTier(...)`, `seal()`), `CommercialToken::setCurrent()`, persist the raw cert via
`SimpleCrypt` under a new `offlineLicense/*` group, emit property-changed signals. On failure:
set `lastError()`, leave activation state untouched (R12 — never partial).

**Startup (persistence, R7).** `ModuleManager` constructs the singleton → `readSettings()` decrypts
the stored cert and re-runs the *full* verify (signature + machine + expiry vs `MonotonicClock::now()`).
Valid → re-install token; expired/invalid → clear stored cert, install nothing. Guarded by
`if (!LemonSqueezy::instance().isActivated())` so a live online license wins (precedence below).

**Token equivalence (R6).** The offline path terminates in the identical
`CommercialToken::setCurrent(sealed token)` used by online/trial. Every `SS_LICENSE_GUARD()` /
`featureTier()` feature gate is unchanged and unaware of the path — this is pure reuse, no gate edits.

**Precedence (online > offline > trial).** `LemonSqueezy` constructs first (its `instance()` is
pulled in by `Trial`'s ctor). `OfflineLicense::readSettings()` installs only when online is not
activated. `Trial::trialAvailable()` gains `&& !OfflineLicense::isActivated()` so an offline license
suppresses the trial, mirroring how it already checks `LemonSqueezy::isActivated()`.

Threading: entirely main-thread, user-action/startup. No `QNetworkAccessManager`, no worker threads.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** Nothing in `FrameReader` / `CircularBuffer` / `FrameBuilder` /
  `Dashboard` draw / span fast lane is read or written. Activation is a startup + user-action path,
  identical in placement to the existing `LemonSqueezy`/`Trial` singletons.
- **New cross-thread signal/slot?** **No.** All main-thread, direct calls; no new connections across
  threads. The existing feature gates already read `CommercialToken::current()` off the hotpath.
- **New input to a cached hotpath flag?** **No.** Does not feed `m_operationMode`, `m_anyAsyncSink`,
  `m_captureLatestFrame`, `m_streamAvailable`, `m_changeDriven`, etc. Feature unlock is read where it
  already is; no cache-refresh wiring needed.
- **Timestamp ownership** — N/A; no frames. The only time source is wall-clock, floored by
  `MonotonicClock` for anti-rewind — deliberately not the frame monotonic clock.

## Data model & persistence

- **No `Frame.h` `Keys::` changes, no project-JSON, no Sessions DB schema.** This is licensing state,
  not telemetry.
- **New `QSettings` group `offlineLicense/`** (encrypted with `SimpleCrypt`, keyed by
  `MachineID::machineSpecificKey()`, `ProtectionHash` — same as `LemonSqueezy`/`Trial`): `cert` (the
  raw verified `.sslic` bytes) and the derived display fields. Reuses the shared `licensing/lastSeen`
  key for the monotonic floor — **no new anti-rewind key**.
- **`CommercialToken` is unchanged.** It already carries exactly what offline needs: `variantName`,
  `instanceName`, `featureTier`, `graceDaysRemaining` (repurposed as days-to-expiry). Expiry is
  enforced at each `readSettings()`/import, not stored as absolute time in the token — matching the
  existing grace-period model.
- **Certificate wire format (the issuer contract, R9):** base64 text of `[ CBOR payload ][ 64-byte
  Ed25519 signature ]`, `.sslic` extension. CBOR payload is a map: `v` (format version, int), `mid`
  (machine id string), `var` (variant/tier string, e.g. `"Lifetime"` / `"Enterprise - Yearly"`),
  `iid` (LemonSqueezy instance id string), `iat` (issued-at, unix seconds), `exp` (expires-at, unix
  seconds). Signature is over the exact CBOR payload bytes. Same key + format = any independent issuer
  (portal or CLI) produces an accepted cert. **The app is source of truth for `mid` format** (it is
  `MachineID::machineId()` verbatim).

## API / SDK surface

- **`licensing.activateOffline`** (params: `{ path: string }`) in `LicensingHandler` — calls
  `OfflineLicense::activateFromFile`, returns success or a `CommandResponse::makeError` whose message
  is the distinct reject reason (bad-signature / machine-mismatch / expired / malformed / wrong-tier),
  satisfying R12/AC7/AC8 headlessly.
- **`licensing.getStatus`** extended with `offlineActivated`, `offlineExpiresAt`,
  `offlineDaysRemaining`, `machineId` so the API reflects offline state (AC7).
- Registered in `LicensingHandler::registerCommands()`; whole handler is already commercial-only.
  Behind `#ifdef BUILD_COMMERCIAL` like the rest of the file.

## QML / UI

- **`LicenseManagement.qml`:** on the activation page (pageIdx 1), add an "Activate offline…" button
  opening a `FileDialog` (`*.sslic`) wired to `Cpp_Licensing_OfflineLicense.activateFromFile(url)`,
  and a read-only, copyable **machine-fingerprint** field bound to `Cpp_Licensing_OfflineLicense.machineId`
  (must render *before* activation — that is why the fingerprint comes from the new object, not from
  `LemonSqueezy.instanceName` which is empty until online-activated).
- On the license page (pageIdx 2), when offline-activated show variant + **days-remaining / expiry**
  and an approaching-expiry warning (R8/AC6), reusing the existing grace-period presentation style. A
  rejected import surfaces `lastError()` via the existing `Misc::Utilities::showMessageBox` pattern.
- No new hotpath, no ComboBox restore-race, standard theme surfaces.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Signature primitive | Ed25519 via OpenSSL EVP · bundle monocypher/libsodium · RSA | **Ed25519 via the already-linked OpenSSL** — zero new dependency (spec constraint), small keys/sigs, modern. |
| Monotonic anti-rewind clock | Extract shared `MonotonicClock` helper · duplicate the logic in `OfflineLicense` · leave it in `LemonSqueezy` and read the key from outside | **Extract a shared helper** both paths call over the same `licensing/lastSeen` key — one implementation of a security-critical primitive; duplication risks divergent floors an attacker could exploit. Costs a minimal, behavior-preserving edit to `LemonSqueezy.cpp`. |
| Activation precedence | online > offline > trial · offline > online · offline as just-another-token | **online > offline > trial** — a live seat-managed license is authoritative; offline is the air-gapped fallback; trial is lowest. Implemented with the same `isActivated()` guard pattern Trial already uses. |
| Public key delivery | `constexpr` byte array in a committed header · `.qrc` resource · fetched | **Committed header constant** — public key is non-secret, immutable, and belongs in the binary; no I/O, no tamper surface via file replacement. |
| Cert payload encoding | CBOR · JSON | **CBOR** (spec-settled) — compact, deterministic byte layout to sign over; `QCborValue` is built in. |
| Verifier location | Pure `OfflineCertificate` free functions · methods on the singleton | **Free functions in a UI/settings-free unit** — makes AC1–3/AC8 testable headlessly and keeps OpenSSL usage in one small surface. |
| Signing tool placement | New non-shipped `tools/ss-license-sign` target · script · fold into app | **Separate opt-in CMake target**, not built by default, never deployed — keeps the private-key-handling code physically out of the shipped binary (hard invariant). |

## Risks & mitigations

- **Private key leakage (highest).** Only the public key is committed; the private key lives solely in
  `SS_LICENSE_SIGNING_KEY` for the CLI. Mitigation: no key material in repo/CI/binary; the signing
  target is excluded from default build and deploy; add a `code-verify`/review check that
  `OfflinePublicKey.h` contains only the 32-byte public key. A leaked *public* key is harmless.
- **Startup token clobbering / ordering.** Two paths can install a token. Mitigation: explicit
  precedence guards (above); `OfflineLicense::readSettings()` runs its install only when online is not
  active, and Trial defers to offline. Covered by AC4 (restart) observation.
- **Clock-rewind to revive an expired cert.** Mitigation: `MonotonicClock` floors "now" at the highest
  time ever seen, shared with the online grace period — the exact mechanism the spec mandates (AC3).
- **GPL build leakage.** All new sources compile only under `BUILD_COMMERCIAL`; the CLI target is
  commercial-only and opt-in. Verify the GPL build still links with the feature absent.
- **Malformed input crashing the parser (AC8).** Mitigation: `QCborValue` parse-error paths and length
  checks before any OpenSSL call; every decode failure maps to a typed reject reason, never a crash or
  partial activation. This is the `common-mistakes.md` "validate at the boundary" class.
- **`instanceName`/HMAC coupling.** The token seal covers `instanceName`; offline sets it to
  `machineId` (same value online uses), so `isValid()` stays consistent — no change to
  `CommercialToken` needed. Confirmed against `CommercialToken.cpp:computeHmac`.

## Test & verification plan

- **Headless C++ verifier self-test (maintainer runs; AC1–AC3, AC8).** Add a hidden
  `--selftest-offline-license` flag (mirroring the existing `--benchmark-hotpath` in-binary convention,
  since the C++ side has no pytest path) that runs `OfflineCertificate` vectors produced by
  `ss-license-sign`: a good cert (accept), a payload/signature bit-flip (reject: bad signature),
  wrong-machine cert (reject: mismatch), past-`exp` cert (reject: expired), rewound clock on an expired
  cert (still expired), and truncated/garbage bytes (reject: malformed). Deterministic, no Qt UI, no
  network.
- **Signing CLI check (maintainer runs; R11).** `ss-license-sign` mints a cert for a given fingerprint;
  feeding it to the self-test and to the app both accept — proves the issuer contract round-trips.
- **Integration / API (maintainer runs against live app with API server on; AC4, AC5, AC7).**
  `pytest tests/integration/` new case: `licensing.getStatus` exposes `machineId`; mint a cert for it;
  `licensing.activateOffline` unlocks Pro (a Pro-gated command/widget becomes available) with **no
  outbound licensing request** (assert via logs / no network); restart the app and confirm activation
  persists; each bad-cert class returns its distinct error.
- **UI observation (maintainer; AC5, AC6).** Fingerprint shows and is copyable pre-activation; a
  near-expiry cert shows days-remaining + warning; an expired cert does not persist activation.
- **Static (before handoff/commit).** `python scripts/code-verify.py --check <new files>`;
  `qt-cpp-review` on the C++ diff; `python scripts/sanitize-commit.py` before any commit. Confirm the
  **GPL build** (`BUILD_GPL3=ON`) still configures/links with all new sources excluded.
