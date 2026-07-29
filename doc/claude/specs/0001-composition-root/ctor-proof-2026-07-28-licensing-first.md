# Ctor-edge proof re-run — licensing block moved first (spec 0042, 2026-07-28)

Change: in `ModuleManager::instantiateCoreModules()`, the `BUILD_COMMERCIAL` licensing block
(`MachineID`, `LemonSqueezy`, `OfflineLicense`, `Trial`) moved from after `adoptAppState` to
directly after `Misc::Translator::instance()`. Everything else keeps its relative order.

Edge audit (each ctor read in full, 2026-07-28):

| Ctor | Reaches | Later-constructed module reached? |
|------|---------|-----------------------------------|
| `Licensing::MachineID` | platform ID sources, `QSettings` | none |
| `Licensing::LemonSqueezy` | `qApp` (aboutToQuit connect, display name), `MachineID`, `SimpleCrypt`, `QSettings`, `QNetworkAccessManager`, cached-restore chain (`readSettings` -> `readValidationResponse` -> `applyValidatedLicense` / `clearLicenseCache`) | none. Message boxes are gated off in the ctor path (`m_silentValidation == true`); `clearLicenseCache` -> `Trial::reassertTokenIfEntitled()` null-guards `s_trial` (Trial not yet built -- same situation as the pre-move order, where LemonSqueezy also preceded Trial) |
| `Licensing::OfflineLicense` | `MachineID`, `SimpleCrypt`, `QSettings`, connects to `LemonSqueezy` (already built) | none. Interactive boxes live in the import/remove flows, not `readSettings` |
| `Licensing::Trial` | `LemonSqueezy` (already built), `MachineID`, `QNetworkAccessManager`, `qApp`, `readSettings` -> `installTrialToken` (token only) | none. Boxes live in server-reply handlers only |

Preconditions available at the new position: `QApplication` exists (module init runs after
app construction); `Translator` built (licensing `tr()` strings resolve).

`SessionContext::shutdown()` untouched: none of the four licensing singletons are
context-adopted slots, so the pinned release order is unaffected.

Conclusion: no licensing ctor reaches any module constructed after it in the new order;
the reorder is proof-clean. Any future edit inside these ctors re-triggers this check per
the spec-0001 rule.
