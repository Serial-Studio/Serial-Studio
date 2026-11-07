# Legacy project fixtures (spec 0075, WP-F)

Hand-written `.ssproj` documents in the shapes Serial Studio still has to load (the two
`led-flag-*` example copies excepted). One file per migration in
`core/Pipeline/DataModel/Project/ProjectLoader.cpp`. None of them is produced by the
current writer, which is the point: the migration paths cannot be exercised by round-tripping a
file the build itself saved.

| Fixture | Migration under test | Expected after load |
|---------|----------------------|---------------------|
| `separator.ssproj` | `migrateLegacySeparator()` | The `parse(frame, separator)` body is rewritten to `frame.split(",")`; the document is re-saved once. |
| `xaxis-index.ssproj` | `migrateLegacyXAxisIds()` | `Signal.xAxis` stops being the frame index `1` and becomes `Time Base`'s uniqueId (an unresolvable index would become `kXAxisTime = -2`). |
| `layout-keys.ssproj` | `migrateLegacyLayoutKeys()` | `__layout__:0__` becomes `layout:0`, keeping only its `data` member. |
| `schema-v0.ssproj` | schema stamping + `migrateLegacyDashboardLayout()` | Every group and dataset gets a uniqueId, `schemaVersion`/`nextUniqueId` are stamped, and `dashboardLayout` + `activeGroupId` move into `widgetSettings` as `layout:1`. |
| `uid-dedup.ssproj` | uniqueId de-duplication + `migrateLegacyWorkspaceRefs()` | The three datasets sharing uniqueId 7 and the two groups sharing uniqueId 3 end up with distinct ids, and the workspace ref's `groupId: 1` (a positional id) is rebound to group 1's uniqueId. |
| `led-flag-all.ssproj` | `migrateLegacyLedFlags()` (spec 0114) | The pre-0114 CAN Bus example: `BCM_LIGHTS` and `BCM_DOORS` (every dataset flagged `led`) are each followed by a `<title> LEDs` group with widget `ledpanel` holding copies of all five datasets; no dataset carries `led` afterwards. |
| `led-flag-mixed.ssproj` | `migrateLegacyLedFlags()` (spec 0114) | The pre-0114 Modbus PLC Simulator: `Holding Registers` keeps its nine datasets and gains a two-light `Holding Registers LEDs` group right after it; the workspace's `widgetType: 8` ref moves from the source group's uniqueId to the panel's. |
| `led-flag-hidden.ssproj` | `migrateLegacyLedFlags()` (spec 0114) | Hand-written: `Front` (flagged) lights, `Rear` (flagged but `hideOnDashboard`) gets no copy, `Speed` is left alone; the panel has one dataset and the LED workspace ref is retargeted to it. |

The `test_legacy_*` tests in `tests/integration/test_project_integrity.py` load each of these
through the API against a running app and assert the row above. They are kept tiny: every field
not needed by its migration is omitted, so a change to the defaults cannot silently invalidate
the fixture.

Never re-save these from the app: an app-written file is a 4.x document and stops being a
legacy fixture.
