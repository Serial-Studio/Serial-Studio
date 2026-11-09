# Icon & Command Registry (spec 0028)

The single source of truth for fixed UI icons and for the commands behind toolbars,
menus, the palette, and shortcuts. Read this before adding a toolbar button, a palette
entry, a menu item, a keyboard shortcut, or a new fixed icon. The goal of the system: a
new command or a whole new toolbar is a data edit plus one small behavior binding, not a
QML surface rewrite.

## Icons

**Tree:** `app/rcc/icons/<category>/<tier>/<name>.svg`. Tiers are `16 24 32 48` (design
px). Categories: `widgets window editor devices panes console database code licensing
notifications commands illustrations system` (`illustrations` holds the empty-state pictures,
drawn at 48 and rendered large; they follow the icon pack like every themed category). The `buttons/` folder is exempt (kept as-is, direct
paths). The `system` category holds `missing.svg` (the placeholder).

**Resolve, never hardcode a path:**
- QML: `Cpp_Misc_IconRegistry.icon("<category>", "<name>", px)` returns a `qrc:/…` URL;
  `Cpp_Misc_IconRegistry.iconById("<category>/<name>", px)` takes the combined id used by
  model roles.
- C++: `Misc::IconRegistry::instance().icon(cat, name, px)` (QML/`qrc:`),
  `.iconPath(cat, name, px)` (C++ `:/…` for QPixmap/QIcon/QSvgRenderer), or
  `.iconById("cat/name", px)`.
- Resolution rule: nearest tier **at or above** `px`, else the largest available. An
  unknown id logs one warning and serves `system/16/missing.svg` (never a crash, never a
  silent blank).

**Adding an icon:** drop `name.svg` into the right `<category>/<tier>/` folder(s), add the
`<file>` line to `app/rcc/rcc.qrc`, done — no C++/enum/QML table. Run
`scripts/registry-verify.py` (checks layout, duplicates, qrc↔disk sync).

**Widget icons:** `SerialStudio::dashboardWidgetIcon(w, large)` maps the widget enum to a
`widgets/<name>` id and resolves it (16/32). The taskbar model publishes a logical `iconId`
role alongside the resolved-URL role so each surface resolves at its own display size
(taskbar 16, palette cells 32) from one model — user-picked workspace icons keep the
`Misc::IconEngine` inline-SVG path and leave `iconId` empty.

**Help-manual icons (`doc/help/*.md`):** never a raw-GitHub artwork URL. Write
`<img src="cmd:<command id>[:checked]" alt="…" width="16" height="16">` for anything that is
a command, `<img src="icon:<category>/<name>" …>` for non-command buttons (widget toolbars).
`app/qml/Widgets/MarkdownIcons.js` rewrites both through `Cpp_UI_CommandRegistry.command()` +
`Cpp_Misc_IconRegistry.iconById()` before either markdown viewer renders; unknown ids serve
the placeholder. On GitHub the unknown scheme degrades to the `alt` text.

**Icon packs (spec 0115):** a pack is a second artwork root, `app/rcc/iconpacks/<pack>/
<category>/<tier>/<name>.svg` plus `pack.json` (`id`, `title`, `translations`, `style`, `mono`),
compiled in through a generated resource list (`basic.qrc`, `art-free.qrc`, or the private tree's
`art.qrc`). The Core registry consults
the active pack before the house tree for the themed categories and never for `window`,
`system` or the exempt `buttons`; a pack is partial by design and falls back per icon. In QML
the `Cpp_Misc_IconRegistry` name is the Ui façade `Misc::IconPacks`, whose `icon`, `iconById`,
`iconOn` and `iconByIdOn` are **function-valued properties with a NOTIFY**: every binding that
calls them re-evaluates on a pack or theme change, so call sites never changed. Icons on the
dark toolbar and taskbar gradients ask for the `"strip"` tone. A mono pack resolves to
`image://iconpack/<pack>/<cat>/<tier>/<name>.svg?tint=RRGGBB` and `Misc::IconPackImageProvider`
recolours the bytes; the tints are the theme's `text` and `toolbar_text`. The selection lives in
the `IconPack` setting (empty = *Follow theme*, which reads the theme's `icon-pack` parameter;
`office` pins the house set).
`scripts/icon-packs.py check` is the gate (fixed categories refused, ids inside the house set,
coverage, mapping and qrc consistency) and `registry-verify.py` runs it on a tree that carries the
packs; the Icons8 mapping is `doc/claude/specs/0115-icon-packs/icons8-map.csv`.

**Two art trees and the Basic pack (spec 0116):** Basic is the colorful icon theme drawn for this
project: `art/drawn/` holds its SVG sources and `scripts/basic-art.py` renders `@1x`/`@2x` PNG pairs
to `app/rcc/iconpacks/basic/`, listed in `app/rcc/basic.qrc`, which every build compiles. Beside it,
exactly one art tree is compiled. Official builds take the licensed Office set from the private
`Serial-Studio/Icons` repository through `SS_ICON_ART_DIR` (a sibling `../Icons` checkout is picked
up automatically), and `SS_REQUIRE_ICON_ART` makes a missing checkout a configure error. Without it,
`app/rcc/art-free.qrc` supplies the Tabler window controls, hidden `fine-line` pack, images and the
action icons. `IconRegistry::locateHouse` serves the house tree, then `fallbackPackId()`, so Basic is
the default where Office is absent and fills any gap where it is present. `app/rcc/rcc.qrc` lists no
art except the shared Tabler buttons. Licensed art never enters this repository: `art-verify.py`
fails on any art file the manifest does not list. Basic must stay original: a drawing is made from
the idea it stands for, never from another theme's icon.

**Widget toolbar icons (spec 0115 R23, spec 0116):** the dashboard widget toolbars draw at 18 px
inside every widget and take their art from their own setting (`WidgetIconPack`, Preferences >
General > Widget Toolbar Icons), independent of the active pack. Empty follows the theme's
`widget-icon-pack` parameter, and a theme that names none uses the house set. `DashboardToolButton` takes
`glyph: "<category>/<name>"` and resolves it through `Cpp_Misc_IconRegistry.monoIconById(id, px)`:
the chosen pack, then the Fine Line pack (`IconRegistry::monoPackId()`), then the
house file. Only a mono pack is tinted; `registry-verify.py` checks every `glyph` id resolves. A collapsed ribbon
section tints its icon with the section's own text colour through `retint`, so it follows the
other icons of its toolbar on the strip and on secondary toolbars alike. The dashboard taskbar,
the Terminal ribbon and content marks (notification severities, empty-state illustrations) keep
the pack's colour artwork.

**Legacy paths:** old `qrc:/icons/…` URLs persisted in user project files are remapped by
`Misc::legacyIconPath()` (generated table, consulted in `IconEngine::resolveActionIconSource`).
Regenerate with `scripts/generate-legacy-icons.py` only if the migration manifest changes.

## Commands

A command is declared once (metadata) and bound once per context (behavior). Nothing
about a command lives inside a toolbar/menu QML file anymore.

### Manifests (metadata + layout) — `app/rcc/commands/`

- **Command manifests** `app.json`, `dashboard.json`, `projecteditor.json`, `database.json`.
  Each command: `id` (`scope.verb`), `title`, `tooltip?`, `icon` (`"category/name"`),
  `titleChecked?`/`iconChecked?` (toggles), `shortcut?`, `shortcutWindows?`,
  `kind` (`action`|`toggle`), `pro?` (dropped from the catalog on GPL builds),
  `contexts` (which palettes list it — see below), `order?` (palette sort),
  `category` (palette grouping key, one of file/mode/connection/view/export/console/
  project/license/tools/help — validated by `registry-verify`; display names + section
  order live in `PaletteModel.categoryLabel`/`categoryOrder`).
- **Layout manifests** `layouts/{main-toolbar,project-toolbar,start-menu,database-toolbar}.json`.
  Node types: `section` (with `collapsible`/`collapsiblePro`/`collapsePriority`/
  `showSeparator`/`collapsedCommands`), `command` (`id` + optional `title`/`icon`/`tooltip`
  overrides + `role`), `grid` (`rows`, `itemHints`, `role: "drivers"` for the build-variant
  driver grid), `separator`, `slot` (a named hole the host fills, e.g. Start-menu
  Workspaces/Actions/Plugins), `submenu`.

`UI::CommandRegistry` (`Cpp_UI_CommandRegistry`) loads and validates all of them, filters
`pro` on GPL, translates titles/tooltips at query time (see Translations), and re-emits
`commandsChanged` on language switch. Add a new manifest/layout by adding a `loadManifest`/
`loadLayout` line in its ctor + the `<file>` in `rcc.qrc`.

### Behavior — `app/qml/Commands/*CommandBindings.qml`

One `QtObject` per context with a `readonly property var map: ({ "<id>": root.cmdX, … })`
and one `readonly property QtObject cmdX` per id holding only `run()` plus optional
`enabled`/`visible`/`checked`/`tooltip`. Metadata (title/icon/shortcut) is NOT here.
`AppCommandBindings`, `DashboardCommandBindings`, `ProjectEditorCommandBindings` live in
`Commands/`; a commercial-only surface keeps its bindings beside its own QML instead (e.g.
`DatabaseExplorer/DatabaseCommandBindings.qml`) so the `registry-verify` commercial-guard
scan of `Commands/*.qml` does not false-positive on `Cpp_Sessions_`/`Cpp_MQTT_` refs.

**Commercial guard:** in `Commands/*.qml`, any line referencing `Cpp_Licensing_`/
`Cpp_Sessions_`/`Cpp_MQTT_` must carry `Cpp_CommercialBuild` on the same or previous line
(short-circuit patterns) — enforced by `registry-verify.py` so GPL builds never evaluate
Pro symbols.

### Join + render

- `Commands/CommandModel.qml` joins registry metadata with a context's binding set(s):
  `CommandModel { context: "app"; bindingSets: [appBindings, dashBindings] }`. `items(query)`
  returns provider-shaped entries (`name/icon/iconId/run/checked/shortcut/visible/enabled`);
  `binding(id)`/`entryFor(id)` are the per-id lookups. A command with no binding in the set
  is silently dropped, so **every id you tag for a context needs a binding reachable by that
  context's model**.
- `Widgets/CommandToolbar.qml` renders a ribbon from a layout surface:
  `CommandToolbar { model: aCommandModel; surface: "main-toolbar" }`. It builds
  `RibbonSection`s/`ToolbarButton`s/grids from the layout tree with live behavior; hosts keep
  only their chrome and any right-pinned buttons (e.g. Connect/Activate). Start menu and the
  palette consume the same models via their own delegates.

### Contexts and the two-binding-set pattern

`contexts` on a command decides which palettes list it: `"app"` (main window, not
connected), `"dashboard"` (dashboard/workspace-switcher palette), `[]` (toolbar/shortcut
only, never in a palette). A command can be in both.

Because `AppCommandBindings` and `DashboardCommandBindings` both define `dashboard.autoLayout`/
`dashboard.freeze` with different bodies (main-window delegates to the visible pane; dashboard
acts directly), **binding-set order matters** — `CommandModel.binding` returns the first set
that has the id:
- Main-window palette + toolbar: `[app, dashboard]` (app first).
- Dashboard palette + toolbar: `[dashboard, app]` (dashboard first).

This lets both palettes show the union of commands while each keeps the correct
implementation of the shared ids.

### Shortcuts

Sequences live in the command manifest (`shortcut` + `shortcutWindows`). Each host window
runs one `Instantiator` over `Cpp_UI_CommandRegistry.shortcutCommands("<window>")`,
gating on the binding's `enabled` (never `visible`). A few chords that can't be one command
(Tab / Ctrl+1..9 families) stay as hand-written `Shortcut` blocks — see
`doc/claude/specs/0028-icon-registry/shortcut-checklist.md`.

### Translations

Command titles/tooltips are translated through `QCoreApplication::translate("Commands", …)`.
`scripts/generate-command-strings.py` emits `core/Ui/UI/CommandStrings.cpp`
(a `QT_TRANSLATE_NOOP` stub) so lupdate sees them; it runs inside `sanitize-commit.py` and
`--check` gates drift. Dynamic per-state strings (a toggle's changing tooltip) use `qsTr`
in the binding.

### Context menus (spec 0063)

The Project Editor's context menus are a third render surface for the same catalog, alongside
the ribbon and the palette.

- **One manifest**, `app/rcc/commands/layouts/editor-menus.json`, holds every menu as a named
  entry under `menus`. `CommandRegistry::loadMenus()` stores each one as its own layout surface
  keyed `editor-menu/<name>`, so `layout()` and all node enrichment are shared with the toolbars.
- **Node types:** `command`, `separator`, `submenu`, plus two menu-only ones. `include` splices
  another menu's items — bare (`{"type": "include", "include": "add-group"}`) or as the body of a
  container (`{"type": "submenu", "title": "Add Group", "include": "add-group"}`); resolution is
  depth-capped and cycle-guarded, unknown targets warn and expand to nothing. `dynamic` names a
  `role` the host fills at open time (the folder cascade behind "Move to Folder").
- **Fragments** `add-group`, `add-dataset`, `add-output`, `add-output-controls`,
  `dataset-visuals`, `move-to-folder` and `item-order` are authored once and included everywhere,
  so a new widget template is one manifest entry plus one binding — visible on the tree, the flow
  diagram, and the toolbars' `GroupTemplateMenu`.
- **Behavior + target:** `Commands/ProjectEditorMenuBindings.qml` is both the binding set and the
  target holder — the right-clicked row's kind/ids/path live there as properties, so
  `enabled`/`visible`/`checked`/`title` stay ordinary bindings. `setTarget()` before opening,
  `clearTarget()` on close. Each host owns its own instance (tree, diagram, template menu); the
  diagram sets `suppressViewChange`, the tree sets `treeSelection` for the bulk entries.
- **Renderer:** `Widgets/CommandMenu.qml`, the menu twin of `CommandToolbar`. `openSurface(name)`
  builds and pops up; items are created on open and destroyed on close, an entry with no binding
  or `visible === false` is never created, an all-invisible submenu is dropped, and separators
  survive only between two visible neighbours (no hand-written `visible:` on dividers). A
  `dynamic` handler returns the objects it created so they are destroyed with the rest.
- **Row kinds:** the tree stamps `TreeItemKind` on every menu-bearing row, including the six that
  used to carry none (project root, frame parser, and the Dashboard Widgets / Variables /
  Dataset Values / Workspaces headers). The blank spacer row stays unkinded and inert.
- **Pro entries** are gated with `enabled: Cpp_CommercialBuild` in the binding, or `proGated` on a
  container node — *not* with the manifest `pro` flag, which deletes the command from the catalog
  on GPL builds instead of showing it disabled.
- **Verify:** `registry-verify.py` walks the menu manifest (known command ids, resolvable icons,
  resolvable `include` targets) and fails when a menu references an id with no binding in
  `ProjectEditorMenuBindings.qml`. `generate-command-strings.py` walks `menus` too, so submenu
  titles reach the translations.

## Recipes

- **New toolbar button:** add the command to the context manifest, add a `command` node to
  the layout manifest, add a `cmd…` binding entry. It appears with icon/tooltip/shortcut/
  translation wired.
- **New context-menu entry:** add the command to `projecteditor.json` with `contexts: []`, add a
  `command` node to the right menu in `editor-menus.json` (or to a fragment, to get it on every
  surface at once), add a `cmd…` entry to `ProjectEditorMenuBindings.qml`. No menu QML changes.
- **New palette command:** add the command with the right `contexts`, ensure a binding
  exists in every context's model (`registry-verify` + the binding-coverage check catch a
  miss).
- **New toolbar surface (like the Session DB):** new command manifest + layout manifest +
  bindings file, a `loadManifest`/`loadLayout` pair, qrc entries, and swap the host's
  hand-built ribbon for `CommandToolbar { model; surface }`.
- **Verify:** `scripts/registry-verify.py` (tree + manifests + guard scan),
  `scripts/generate-command-strings.py --check`, `scripts/code-verify.py --check`.
