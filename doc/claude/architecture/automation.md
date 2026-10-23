# Architecture — Drivable UI (spec 0101)

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full before
> touching `core/Ui/UI/Automation/`, `core/Ui/ApiHandlers/AutomationHandler.cpp`, an
> `objectName` under `app/qml`, the `qml-objectname-literal` / `qml-control-unnamed` linter
> rules in `scripts/code-verify.py`, or the `DrivesUi` mapping in
> `core/Api/API/Server/ServerAuth.cpp`. The requirements live in
> `doc/claude/specs/0101-drivable-ui-one-surface/spec.md`; Part A (this surface) is in the
> tree, Parts E, C and D follow their own plan addenda.

The live Qt Quick windows are a command surface. Six `ui.automation.*` commands, registered by
`API::Handlers::AutomationHandler` from `UI::ApiHandlers::registerAll`, read or drive them:
`listWindows`, `listElements`, `click`, `key`, `screenshot`, `setReduceMotion`. Four classes
under `core/Ui/UI/Automation/` do the Qt-facing work so a ctest can exercise them without the
application: `WindowInventory` (visible windows, modal detection, focus), `ElementCatalog`
(the bounded item-tree walk, roles, filters, hit testing), `InputSynthesizer` (mouse and key
delivery) and `WindowCapture` (grab, inverse HDR transfer, crop, scale, PNG, size cap).

## Where it runs

Every API command runs on the GUI thread, where the item tree lives, so the handlers read it
without locks. Input goes through the exported `qt_handleMouseEvent` (the symbol QTest uses, which
converts the logical window point to the native pixels the QPA layer expects, so a click lands
where it was asked for on a scaled display) and `QWindowSystemInterface::handleKeyEvent` with
the `SynchronousDelivery` template argument: the application's button and modifier state
stays coherent and every event is dispatched before the call returns. Nothing here opens an event loop, but synchronous delivery runs the target's
own handler on the command's stack: a control whose handler calls `QMessageBox::exec` or a
static file dialog parks the click inside that nested loop, where queued commands keep being
served with `ExecuteDepthGuard::depth()` above zero and therefore without the pre-mutation
checkpoint. The manual says so next to `click`; a checkpoint rule keyed on the outermost
execute of a dispatch chain, rather than global depth, is the follow-up if that ever bites.
`QQuickWindow::grabWindow()` blocks on the render thread's own wait, as any grab does.

Two delivery details are load-bearing. The QPA layer asserts on an explicit
`MouseButtonDblClick`, so a double click is two press-release pairs on adjacent synthetic
timestamps and Qt synthesizes the double-click event itself; a single click first advances
the synthetic clock past `mouseDoubleClickInterval`, so two consecutive click commands never
fuse into one double click. Key sequences press and release the modifier keys around the key
in QTest's order, each event carrying the modifiers already held.

## Identifier convention

A control is addressed by its `objectName` and by nothing else. Names are stable across a
language or theme switch by construction:

| Control kind | Identifier | Example |
|--------------|------------|---------|
| Manifest-driven (ribbon, menus, palette, start menu) | the command id | `io.toggleConnection`, `app.preferences` |
| Hand-built control | lowercase dotted literal `<area>.<control>` | `console.send`, `setup.uart.baudRate`, `settings.general.language` |
| Repeated delegate | `<area>.<kind>.<identity>.<control>` | `dashboard.tile.3.rename`, `taskbar.window.7`, `palette.window.4` |
| Window | the file's camelCase name | `main`, `settings`, `projectEditor`, `external.<widgetIndex>` |

`widgetIndex` is the tile identity because the window manager keys tiles by it and group
tiles have no dataset id. A command id can name more than one control at once (the ribbon
button, a menu item, a start-menu entry): `byId` returns the first in walk order, the ribbon
while no menu is open, and `listElements` with the `id` filter shows every carrier. The sweep is enforced by two linter rules: `qml-objectname-literal`
(blocking everywhere) refuses an `objectName` expression that reads `qsTr(`, a translator or
theme object, or a `.title` / `.text` / `.label` style property; `qml-control-unnamed`
(blocking under `app/qml/Widgets/Command*.qml`, `app/qml/MainWindow/**`,
`app/qml/Dialogs/Settings/**`; its advisory twin `qml-control-unnamed-advisory` elsewhere)
flags a shared widget or raw Qt Quick control instance whose body binds no `objectName`. Both
have fixture pairs under `scripts/tests/fixtures/`. A new control in the swept directories
needs a name before it compiles past CI; a new delegate folds its identity in.

## Roles

`ElementCatalog::roleOf` walks the meta-object chain from the most derived class. At each
level it first matches the QML component name (the class name read up to `_QMLTYPE_`, or
`_QML_` for an inline object) against the repository's Item-based controls, then the C++
class name against the Qt Quick Controls table:

| Role | Qt classes | Repository components |
|------|-----------|-----------------------|
| `button` | `QQuickButton`, `QQuickToolButton`, `QQuickRoundButton`, `QQuickItemDelegate`, `QQuickAbstractButton` (last resort) | `ToolbarButton`, `TaskbarButton`, `BigButton`, `MenuButton` |
| `checkbox` / `radiobutton` / `switch` | `QQuickCheckBox` / `QQuickRadioButton` / `QQuickSwitch` | |
| `pagetab` | `QQuickTabButton` | `SegmentedControl` |
| `menuitem` | `QQuickMenuItem` | |
| `combobox` | `QQuickComboBox` | |
| `editabletext` | `QQuickTextField`, `QQuickTextInput`, `QQuickTextArea`, `QQuickTextEdit` | `SearchField` |
| `spinbox` | `QQuickSpinBox` | |
| `slider` | `QQuickSlider`, `QQuickRangeSlider`, `QQuickDial` | |
| `tile` | | `WidgetDelegate`, `WidgetTitleBar` |
| `item` | a `QQuickItem` of no other kind carrying a dotted identifier | palette cards, start-menu entries |

The `item` rule is what makes a hand-built delegate reachable: a dotted `objectName` declares it
interactive. The dot is required on purpose — an authored identifier is always dotted, so a bare
name the author never set (a Qt type name a layout container or control carries in some builds)
is not a declaration and stays off the driving surface; an unnamed plain item is never listed. A
field with a non-normal `echoMode`
(passwords, keys) reports neither its text nor a label taken from it, because the catalog is
readable without consent and sits in the assistant's safe tier. The walk uses an explicit stack with
`kMaxDepth` 64 and `kMaxItems` 16384, skips invisible or fully transparent subtrees, lists
controls in child order, resolves an id to the first match and a point to the last listed
control containing it (the one drawn on top).

## Gating

`click` and `key` carry the `DrivesUi` capability (`core/Api/API/CommandCapabilities.cpp`);
the other four are plain. `ServerAuth::authorizeRemoteCommand` routes `DrivesUi` to a third
`ConsentGate`, `m_uiDrive`, built like the device-write and script-install gates with an
empty settings key: session-only, never written to disk, refused with `CONSENT_REQUIRED` while
its prompt ("Allow API clients to drive the user interface?") is unanswered, denied on the
offscreen platform, and granted in memory by `SERIAL_STUDIO_API_AUTO_CONSENT=1` with the other
two. Consents do not compose: a device-write "yes" never unlocks a click, which
`tst_server_auth` pins without a prompt. `API::SealedPolicy` denies all six names, because a
sealed deployment's windows show the package content. The assistant tiers
(`app/rcc/ai/command_safety.json`) put `listWindows`, `listElements` and `setReduceMotion` in
`safe`, `screenshot` in `confirm`, `click` and `key` in `deviceGated`;
`tests/scripts/test_ai_assistant_static.py` pins each to exactly one tier.

## Headless and the reduce-motion switch

A headless root loads no QML: `listWindows` answers an empty list and every other command an
`EXECUTION_ERROR` naming the missing window, which is also how the GUI pytest tier skips
(AC16). `setReduceMotion` calls the graphics backend's persisted setter, but reaches the
backend through the main window's QML root context property `Cpp_Misc_GraphicsBackend`
(`QQmlEngine::contextForObject` on the window, `QQmlContext::contextProperty`), not through
`Misc::GraphicsBackend::instance()`: the handler adds no edge to the singleton census and the
switch is, in effect, a preference of the interface that exists.

## Qt internal names to re-verify per Qt bump

These are implementation names, not API, beside the `_qt_sg_hdr_format` note in
`dashboard.md`. `tst_ui_automation` pins the first four against an offscreen `QQuickView`.

- `_QMLTYPE_` and `_QML_` in dynamic meta-object class names (`ElementCatalog::componentName`).
- `QQuickOverlay` as the class of the popup overlay child of `contentItem()`, and
  `QQuickPopupItem` for a popup's item, whose QObject parent is its `QQuickPopup`; the
  `modal` flag is read as a property of that parent (`WindowInventory::hasModalPopup`).
- The Qt Quick Controls class names in the role table.
- Synchronous delivery through `qpa/qwindowsysteminterface.h`, a header of the private Gui
  module every layer already links for the RHI, and its assert on an explicit double-click.
- The floating-point grab formats (`QImage::Format_RGBA16FPx4` and relatives) an FP16
  swapchain returns; `WindowCapture::toDisplayReferred` undoes the output transform with the
  constants of `Misc/HdrTransfer.h` through a 4096-entry table, reading the per-window SDR
  white scale from the `Misc::HdrOutput` instance found by class name under the window.

## Tests

- ctest: `tst_ui_automation` (inventory, catalog ids/roles/geometry, filters, component-name
  parsing, modal detection, click and key delivery, grab with crop and scale, the inverse
  transfer on a synthetic FP16 image; software scene graph so every CI leg renders alike),
  `tst_server_auth` (non-composition and the headless grant), the three registry sweeps.
- pytest, `gui` marker (desk only, CI deselects it): `tests/integration/test_ui_automation.py`
  (AC1 to AC4) and `tests/security/test_ui_automation_consent.py` (AC5, two launch modes).
- `tests/integration/test_sealed_session.py` derives its denied list from `SealedPolicy.cpp`,
  so the six names join the sweep without an edit.

## Open after Part A

- The tile title-bar buttons live in `app/qml/Widgets/MiniWindow.qml`, outside the Part A
  lane, with no widget identity; the proposed follow-up is a string property on MiniWindow
  that the delegate sets to its tile id, with the four buttons named from it (`.close` and
  so on).
- Palette tool cells stay unnamed until `PaletteModel.qml` copies the command id into its
  tool nodes (`id: items[i].id`).
- Part E (assistant images), Part C (setup panels dispatch commands) and Part D (Project
  Editor dispatch) each get a plan addendum before their own task lists.
