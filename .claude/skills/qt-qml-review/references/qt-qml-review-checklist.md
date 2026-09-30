<!-- Adapted for Serial Studio from The Qt Company qt-qml-review skill (v1.0, BSD-3-Clause,
https://github.com/TheQtCompanyRnD/agent-skills). Kept verbatim as the universal Qt6 QML
rule text; this repo ships NO bundled linter — rules marked (lint) are checked by the
Phase 2 agents (plus qmllint where it overlaps). Where a rule conflicts with this
repository, serial-studio-qml-rules.md supersedes it (notably § 14 context properties and
§ 1 style-specific imports). -->
<!-- universal Qt API text, deliberately not repo symbols -->
<!-- claim-verify off -->

# Qt6 QML Review Checklist

Comprehensive review rules for Qt6 QML code. Used by the Python
linter (`qt_qml_lint.py`) for mechanically-checkable rules and by
the six deep-analysis agents for semantic/cross-file checks.

Rules marked **(lint)** are enforced by the linter. Rules marked
**(agent)** require semantic analysis beyond regex capability.

---

## 1. Imports

### IMP-1 (lint): Redundant QtQuick.Window
`import QtQuick.Window` is unnecessary when `import QtQuick` is
present. In Qt 6, Window types were folded into the QtQuick module.

### IMP-2 (lint): Versioned imports
Qt 6 dropped the requirement for version numbers on all imports.
Versioned imports (`import QtQuick 2.15`) cap the API surface and
cause "missing type" confusion.

### IMP-3 (lint): Plain Controls import with customization
When customizing `contentItem`, `background`, `indicator`, or
`handle`, import a specific style (`QtQuick.Controls.Basic`) rather
than plain `QtQuick.Controls`. The default style abstraction layer
can produce unexpected rendering.

### IMP-4 (lint): Import ordering
Order imports: Qt modules first, then third-party, then local C++,
then QML folder imports. Consistent ordering aids readability and
matches `qmlformat --sort-imports`.
If this causes shadowing issues, because types with the same name
are defined in various modules, use a namespaced import:
`import mymodule as MM`

### IMP-5 (lint): Qt.include() deprecated
`Qt.include()` was deprecated in Qt 5.14 and removed from Qt 6
documentation. Use ES module imports or explicit QML imports.

### IMP-6 (lint): Duplicate imports
The same module imported more than once. Remove the duplicate.

---

## 2. Attribute Ordering

### ORD-1 (lint): QML attribute ordering convention
Within each QML object block, attributes must appear in this order:

1. `id`
2. Property assignments (`width: 100`, `color: "red"`)
3. Property declarations (`property type name`, `required property`)
4. Signal declarations (`signal name()`)
5. Attached properties (`Layout.fillWidth`, `Drag.active`)
6. `states`
7. `transitions`
8. Signal handlers (`onClicked`, `Component.onCompleted`)
9. Child objects (visual first, then non-visual)
10. JavaScript functions

This ordering ensures the most intrinsic properties are visible
first. Signal handlers should be grouped by semantic relation,
and ordered shortest-first as a fallback.

The linter reports only the first ordering violation per block.
Blocks with special internal structure (Connections, Behavior,
animation types, State, Transition, PropertyChanges) are exempt.

---

## 3. Bindings & Properties

### BND-1 (lint): property var
Use typed properties (`int`, `string`, `color`, etc.) instead of
`property var`. Typed properties enable `qmlsc` compilation to C++,
eliminate meta-object overhead, and allow `qmllint` type checking.
Matches qmllint's `prefer-non-var-properties` warning.

### BND-2 (lint): Imperative = destroys binding
Any `property = value` in JavaScript permanently replaces the
declarative binding with a static value. Use `Qt.binding(() => expr)`
to restore reactivity if needed. The `qt.qml.binding.removal`
logging category (Qt 5.10+) is the only runtime diagnostic. qmllint
does NOT detect this.

### BND-3 (lint): Qt.binding with old-style function
Use arrow syntax: `Qt.binding(() => expr)` not
`Qt.binding(function() { return expr })`. Arrow functions avoid
`this` context issues inside `Qt.binding()`.

### BND-5 (lint): list<> property type
QML `list` properties have no granular change signals for add, move,
or remove. Only whole-list replacement triggers notification. Binding
expensive operations to list properties causes subtle update bugs.
Consider a `ListModel` or emit change signals manually.

### (agent): Binding loops
The runtime detects single-cycle loops
(`"QML: Binding loop detected"`) but cannot detect multi-cycle loops
(A changes B via signal handler, B's binding updates A). These silent
loops cause performance degradation. Common source: `implicitWidth` /
`implicitHeight` in layouts.

### (agent): Property alias chains
Aliases to aliases are fragile. Each link must resolve; if any
intermediate component hasn't finished initialization, the value is
`undefined`. Aliases are not activated until the component is fully
initialized -- referencing them during construction can fail.

### (agent): Qualified lookup
Bare property names (`someProperty` instead of `root.someProperty`)
resolve via QML's dynamic scope chain, which is fragile and blocks
`qmlsc` compilation. qmllint warns via the `unqualified` category.

### (agent): pragma ComponentBehavior: Bound
Adding `pragma ComponentBehavior: Bound` to files with delegates
restricts inline components to their creation-context IDs, enabling
`qmlsc` to resolve bindings statically. Data must be passed via
`required property` instead of outer-scope id access. Qt plans to
change the default to `Bound` in a future version.

---

## 4. Layout & Anchoring

### LAY-1 (lint): anchors + Layout on same item
Anchors and `Layout.*` properties conflict. An item managed by a
Layout must use only `Layout.*` for sizing and positioning.

### LAY-2 (lint): Bare width/height inside Layout child
Setting `width` or `height` directly on a Layout-managed item
silently breaks the layout's size negotiation. Use
`Layout.preferredWidth`, `Layout.fillWidth`, etc.

### LAY-3 (lint): Four anchor edges instead of fill
Setting `anchors.left`, `anchors.right`, `anchors.top`, and
`anchors.bottom` separately to the `parent`'s values is verbose.
Use `anchors.fill: parent`.


### LAY-4 (lint): Cross-branch anchoring
Anchors must only point to the `parent` or `silbling` items.


### LAY-6 (lint): Bare x/y inside Layout child
Layouts manage positioning. Setting `x:` or `y:` on a layout child
is ignored by the layout engine and creates confusion.

---

## 5. Loader & Dynamic Creation

### LDR-1 (lint): Loader.item without status guard
With `asynchronous: true`, `Loader.item` is `null` until
`status === Loader.Ready`. Binding to `Loader.item.someProp` without
a guard causes `TypeError`. Use optional chaining (`?.`) or gate on
`Loader.status`.

### LDR-2 (lint): Qt.createComponent with string URL
String-based `Qt.createComponent()` is not type-safe and is not supported
by QML tooling. Prefer `Component {}` definitions, or the module URI +
type name overload of `Qt.createComponent`.

### LDR-3 (lint): Qt.createQmlObject
Parses a QML string at runtime on every call. No component caching.
Slow and error-prone. Use `Loader` or `Component.createObject()`.

### LDR-4 (agent): createObject without lifecycle management
Objects created via `Component.createObject()` must be referenced
via JS variables or must have a parent set; otherwise they can be
collected by the garbage collector at unexpected times.

### LDR-5 (lint): Loader with both source and sourceComponent
These are mutually exclusive. Setting both is confusing, as the
behavior is unspecified.

---

## 6. ListView & Delegates

### DEL-1 (lint): model.roleName without required property
Modern Qt 6 best practice is to declare `required property` for each
model role. Once any required property is declared, the implicit
`model` context object is no longer injected (but can also be explicitly
requested as a `required property` ). Required properties
enable compilation, eliminate `unqualified` warnings and have generally
better tooling support (e.g. in qmlls, qmllint).

### DEL-2 (lint): state in delegate with reuseItems
With `reuseItems: true`, a delegate is pooled and rebound rather
than recreated. Bindings to model roles re-evaluate automatically;
all other state persists across reuse and bleeds between items.
Reset non-model-derived state in `ListView.onReused`
(`Component.onCompleted` runs only once, at first creation).

### DEL-3 (lint): connect() in Component.onCompleted without receiver
Direct `connect()` without a receiver creates signal connections that
outlive delegate destruction, causing `TypeError` when the signal fires
on a destroyed delegate. Use `Connections {}` objects instead, or pass
an explicit receiver.

### DEL-4 (agent): Missing required property int index
When using `required property` in delegates, built-in roles like
`index` and `modelData` must also be declared explicitly -- they will
not auto-inject when any required property exists. Requires
understanding the delegate context from the ListView's `delegate:`
assignment.

### (agent): Delegate complexity
Delegates multiply cost by item count. Complex delegate trees with
nested Repeaters, multiple Loaders, or heavy bindings degrade
scrolling performance. Keep delegates minimal.

### (agent): currentIndex reliability
`currentIndex` defaults to 0 (not -1) when a model is set. Known
bugs: QTBUG-48633 (model change resets to 0), QTBUG-93293 (initial
binding ignored). Workaround: re-apply in `onModelChanged`.

---

## 7. States & Transitions

### STA-1 (lint): PropertyChanges target: syntax (Qt 6)
Qt 6 prefers `PropertyChanges { myId.property: value }` syntax. The old
`target: myId; property: value` form still works but should only be used
with ui.qml files used with Qt Design Studio.

### STA-2 (lint): Transition without from/to
A `Transition {}` without explicit `from`/`to` fires on every state
change, including unintended ones. Use explicit `from`/`to` pairs.
If there are multiple rules, a catch-all transition will only be picked
if no other rule matches either on `from` or `to`.

### STA-3 (lint): Top-level states in reusable component
`states` is a `QQmlListProperty` -- assigning from outside a
component *adds* to the existing list rather than replacing it,
causing conflicts. Wrap internal states in a `StateGroup`. Only
flagged when the file has `required property` declarations
(indicating it is a reusable component).

### STA-4 (lint): PropertyChanges instead of imperative assignments
Drive state-dependent property values through declarative PropertyChanges
blocks rather than assigning properties imperatively (`obj.prop = ...`)
from signal handlers or scripts. Only the declarative form participates in
`restoreEntryValues`, so original values/bindings are restored when the
state is left.

### (agent): restoreEntryValues surprises
`PropertyChanges.restoreEntryValues` defaults to `true`. Properties
revert on state exit, which surprises developers who set properties
imperatively while in a state.

### (agent): Binding.restoreMode (Qt 5 to Qt 6 migration)
Default changed from `RestoreNone` (Qt 5) to
`RestoreBindingOrValue` (Qt 6). Qt 5 code relying on Binding to
"stick" its value after deactivation silently reverts in Qt 6.
If a `Binding` gets disabled, set the value explicitly.

---

## 8. Images

### IMG-1 (lint): Image without sourceSize
Without `sourceSize`, Qt decodes the full-resolution image into GPU
memory. A 4000x3000 photo displayed at 100x75 still allocates ~48MB
of texture memory. Set `sourceSize` to display dimensions for large
images.

### IMG-2 (lint): Network Image without asynchronous: true
Image decoding blocks the UI thread by default. For network sources,
the entire download+decode is synchronous without `asynchronous: true`.

### IMG-3 (agent): Image without status check
Dynamic/network sources can fail. Check `Image.status` for error
handling rather than assuming successful load.

---

## 9. Performance & Rendering

### PRF-1 (lint): Transparent Rectangle
`Rectangle { color: "transparent" }` creates a scene graph geometry
node even when transparent. Use `Item` for grouping -- it generates
no geometry node. The cost compounds in delegates.

### PRF-2 (lint): opacity: 0 without animation
`opacity: 0` still incurs rendering overhead and retains keyboard
focus. `visible: false` skips rendering entirely and removes from
input handling. Use `opacity: 0` only during fade animations.
Suppressed when the file contains opacity animation declarations.

### PRF-3 (lint): clip: true
Qt docs: "Clipping is a visual effect, NOT an optimization." Forces a
separate scene graph batch (scissor/stencil). Acceptable on ListView
(many children) but costly on small items.

### PRF-4 (lint): font.pixelSize animation
Every `font.pixelSize` change triggers full text relayout (glyph
shaping, line breaking). Use a `scale` transform on the `Text`
element for size animations instead.

### PRF-5 (lint): Text.RichText
RichText invokes a full HTML/CSS parser, significantly more expensive
than PlainText or StyledText. Use `textFormat: Text.PlainText` unless
rich formatting is needed.

### PRF-6 (lint): layer.enabled
Renders the subtree to an offscreen FBO, then composites as texture.
The layered item cannot be batched with siblings. Multisampling on
layers is especially expensive. Enable only during effects/animations.

### (agent): font.preferShaping: false
Set `font.preferShaping: false` when complex text shaping features
(ligatures, kerning, Arabic/Indic scripts) are not needed. Reduces
text layout cost, especially in delegates and frequently updated
Text elements.

### PRF-7 (agent): Expensive expressions in bindings
Function calls in hot bindings re-execute on every dependency change.
Cache expensive computations in a `readonly property`. The agent
should identify bindings that call functions which could be cached.

### (agent): QRegularExpression in loops
Constructing `QRegularExpression` inside a loop recompiles on every
iteration. Compile once before the loop.

### (agent): Non-const range-for triggering COW detach
Non-const iteration over QML list/model containers can trigger
copy-on-write deep copies.

---

## 10. Style & Conventions

### STY-1 (lint): Top-level component missing id: root
The QML convention is `id: root` on the top-level component. This
enables qualified lookup (`root.someProperty`).

### STY-3 (lint): Multiple dot-notation for same group
When setting 3+ sub-properties of the same group (e.g.,
`sourceSize.width`, `sourceSize.height`, `sourceSize...`), use group
notation instead: `sourceSize { width: 32; height: 32 }`. Attached
property namespaces (Layout, Component, etc.) are exempt.

### STY-6 (lint): id not camelCase
QML convention is `lowerCamelCase` for ids. Underscore or UPPER ids
break convention.

### (agent): Unnecessary id assignments
Only assign `id` if the object is actually referenced elsewhere.
Unnecessary IDs add cognitive overhead and risk duplicate-ID errors.
Use `objectName` or comments for labeling.

### (agent): Reusable component sizing
Reusable components should never set explicit `width`/`height`
internally. Instead, provide `implicitWidth` and `implicitHeight`
calculated from content (text metrics, icon size, padding, child
layout). This lets consumers freely resize or omit size to get a
sensible default.

### (agent): `parent` resolution pitfalls
`parent` in QML refers to the visual parent, which differs by
context:
- **Delegates**: `parent` is the delegate's internal container, NOT
  the ListView. Use `ListView.view` or an explicit `id`.
- **Loader items**: `parent` is the Loader itself. Accessing
  grandparent via `parent.parent` is fragile.
- **Popups**: `parent` is the overlay, not the logical parent.
- In all contexts, `parent` can be `null` during creation and
  destruction -- always null-check.

---

## 11. Signals & Connections

### SIG-1 (lint): Connections without explicit target
Default target is `parent`, which causes unintended signal handling
if the parent type changes. Always set `target` explicitly. Set
`target: null` if the real target is assigned later at runtime.

### SIG-2 (lint): Deprecated onFoo: handler syntax
The `onFoo:` syntax in `Connections` blocks is deprecated since
Qt 5.15. Use `function onFoo() {}` instead.

### SIG-3 (lint): Mixed handler syntax in Connections
Mixing old `onFoo:` handlers with new `function onFoo()` handlers in
the same `Connections` block silently ignores the function-based
handlers. Use one style consistently.

### (agent): Signals communicate up, functions communicate down
Signals should notify parent/owner of internal state changes. Signal
handlers should react, not mutate the emitting object. Functions
communicate downward (parent tells child to do something). Never
emit C++ signals from QML -- use function calls or property
assignments.

---

## 12. Error Handling & Security

### ERR-1 (lint): Hardcoded HTTP URL
Unencrypted `http://` URLs expose data in plaintext. Use `https://`
for any network endpoint. Localhost and test URLs are excluded.

### ERR-2 (lint): Hardcoded Unix paths
`/tmp/` and other Unix-specific paths do not exist on Windows. Qt
provides `QStandardPaths::writableLocation(QStandardPaths::TempLocation)`
for cross-platform temporary file access.

---

## 13. JavaScript Quality


### JS-1 (lint): var instead of let/const
`var` has function scope and hoisting, causing subtle bugs. `let` and
`const` have block scope. Qt coding instructions mandate `let`/`const`.

### JS-2 (lint): Loose equality
Loose equality (`==`/`!=`) performs type coercion, which is almost
never desired in QML property comparisons. Use strict equality
(`===`/`!==`). Matches qmllint's `equality-type-coercion` warning.

### JS-3 (lint): Dynamic code execution
Dynamic JS code execution (such as the `eval` function) blocks JIT
compilation in QV4 and is a security risk. qmllint flags it. There
is never a valid use case in QML.

### (agent): Minimize JavaScript
Prefer C++ for logic and QML bindings for UI state. Heavy JS blocks
force interpreter fallback and prevent `qmlsc` compilation.

---

## 14. C++ Integration (agent-only)

### (agent): No context properties
`rootContext()->setContextProperty()` is expensive (re-evaluated on
every access), globally scoped, invisible to tooling, and prevents
compilation. Use QML_ELEMENT or set properties, expose global state
via singletons, and set state on components via
the `createWithInitialProperties` API family.

### (agent): Object ownership across QML/C++ boundary
The QML engine takes ownership of QObjects (and instances of derived 
types) when they are returned from a `Q_INVOKABLE` method. It does not
take ownership of objects retrieved from properties. You can retain
ownership explicitly by either parenting the object or using 
`QJSEngine::setObjectOwnership` with `QJSEngine::CppOwnership`.
You can also explicitly forfeit ownership using
`QJSEngine::setObjectOwnership` with `QJSEngine::JavaScriptOwnership`.

---

## 15. Migration (Qt 5 to Qt 6) (agent-only)

### (agent): Connections handler syntax migration
Old: `Connections { onClicked: ... }` --
New: `Connections { function onClicked() { ... } }`.
Mixing both in one block silently breaks the function-based handlers.

### (agent): PropertyChanges target syntax migration
Old: `PropertyChanges { target: id; prop: val }` --
New: `PropertyChanges { id.prop: val }`.

### (agent): GraphicalEffects to MultiEffect
`QtGraphicalEffects` (Qt 5) -> `Qt5Compat.GraphicalEffects` (bridge)
-> `MultiEffect` (Qt 6.5+). `MultiEffect` combines blur, shadow,
colorization in a single pass.

### (agent): Binding.restoreMode default change
Qt 5 default: `RestoreNone`. Qt 6 default: `RestoreBindingOrValue`.
Code relying on "set and forget" behavior silently reverts in Qt 6.

### (agent): Pointer handlers replace MouseArea
`TapHandler`, `DragHandler`, `HoverHandler` are non-visual,
composable, and support multi-touch. `MouseArea` steals touch events
with exclusive grabs; mixing both causes conflicts.

---

Copyright (C) 2026 The Qt Company.

<!-- claim-verify on -->
