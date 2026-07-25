# Spec 0029 — Swarm-audit fixes + licensing simplification

Status: approved in chat (2026-07-24). Source: 60-agent Opus audit of the last 12 commits
(specs 0027/0028 + licensing commit 1aa3c5ec); 30 confirmed findings deduped to 22 issues.
Full findings JSON: session scratchpad `confirmed.json`.

## Decisions (user, 2026-07-24)

- **D1** Trial and Pro get *exactly* the same feature set until the trial expires. Remove
  feature-tier distinctions ("keep it simple and stupid"): drop the dead `Hobbyist` tier and
  every `featureTier() >= Trial` / `< Trial` comparison at feature gates. `isValid()` +
  `SS_LICENSE_GUARD()` remain untouched at every site (validation is NOT weakened; only the
  tautological tier compares go). Welcome-text `> Trial` pick in Translator stays (messaging,
  not a feature).
- **D2** Demo (RocketLaunch) is disabled on GPL builds entirely: QML entry points hidden,
  `DemoLauncher::startDemo()` and `system.startDemo` refuse outside `BUILD_COMMERCIAL`.
  This retires the GPL "Pro features detected" nag-on-demo finding.
- **D3** `kAssumedMaxRateHz` stays at 1.024 MHz (matches native hotpath tier). No rate-aware
  clamp. No change.
- **D4** Fix all remaining confirmed findings (waves below).

## Waves

1. **Licensing correctness**: Trial ctor always `readSettings()` (fixes inert
   `reassertTokenIfEntitled()` on cached-restore startup + wrong `firstRun()`);
   `Trial::onServerReply` only clears the shared token when it owns it (variant "Trial");
   D1 tier removal; D2 demo gating; datasetless-grid drop must not shift saved-workspace
   `relativeIndex` (Dashboard.cpp:2065 region).
2. **Operator/lock gating**: restore `!runtimeMode` on Export submenu; shortcut delegates
   honor `visible` as well as `enabled`; Project Editor palette/shortcuts blocked by
   lock/wrong-mode overlay; `app.fileTransmission` hidden when handler refuses;
   `editor.save`/`saveAs` enabled-gated (visible always).
3. **Command palette**: keyboard nav scrolls highlight into view; Cmd/Ctrl+W closes palette;
   window shortcuts blocked while palette open; modal catcher not hit-testable during
   close fade; palette closes when its model swaps; editor palette section titles restored;
   StartMenuToggle checkmark follows source of truth when run() refuses.
4. **Icons + translation**: fix 24 stale legacy icon remap targets; tier-mismatch requests
   (GroupsView headers, FlowDiagram nodes, buildAutoWorkspaces persisting 16px path);
   `workspaces.list/get` remap legacy paths; `generate-command-strings.py` harvests
   Database Explorer manifests; `collapsedTitle` translated; ThemeManager theme-name
   localization for ar/he/vi.

Refuted findings (14) intentionally not fixed; list in audit output.
