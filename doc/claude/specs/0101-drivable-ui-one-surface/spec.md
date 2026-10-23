---
spec: 0101-drivable-ui-one-surface
title: Drivable UI and One Command Surface
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-10-16
author: Alex Spataru
---

# Spec 0101 — Drivable UI and One Command Surface

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Nothing outside the process can observe or operate the real GUI. The API server exposes more
than 400 commands, the pytest suite drives the application through them, and the in-app assistant
dispatches them, but none of those clients can tell whether a button is visible, press it, type
into a field, or see what the window shows. GUI regressions ship because nothing exercises the
GUI: the Project Editor is the recurring regression hotspot, and a recent fix on master
(a push button whose opacity ignored its enabled state) is exactly the class of defect a
screenshot or an element query would have caught. Counted on 2026-10-19, the QML tree has 250
files, two `objectName`s and no accessibility hints, so even a test that could reach the window
would have nothing stable to address. The assistant is blind in the same way: none of its five
providers encodes an image, tool results are text under a small byte budget, and the chat
composer accepts no attachments, so a user cannot show it a wiring photo or a datasheet page
and it cannot look at the dashboard it is editing.

The GUI also bypasses the command catalog that every other client uses. The same count found
1728 direct QML-to-C++ method calls, about 850 of them actions or mutations, plus 321 property
writes of which 167 are two-way driver-configuration writes in the connection setup panels. The
UI manifests hold 185 commands whose behavior bindings call C++ directly, so a user action and
its API counterpart are two code paths that can drift in validation, licensing, undo and
error handling. The cadcraft comparison that prompted this spec routes every surface (menus,
toolbar, command line, CLI, control channel, MCP) through one catalog and exposes the window to
agents through inspect, elements, click, key and screenshot calls. Serial Studio already has the
catalog, the MCP server and the capability and sealed-session classifications. It lacks the
window surface and the discipline that the UI's own actions go through the catalog. Spec 0102
makes that catalog reachable by MCP hosts without a bridge script; the two specs are
independent and either may land first.

The two halves belong together. The driving surface is what makes the catalog migration safe
to verify: once a test can press the real button and read back the result, every migrated
action can be checked for parity against its command.

## Goals

- An API client, an MCP client or a pytest can list the windows, enumerate the visible controls
  with stable identifiers, press a control, type into one, and capture a screenshot, and the
  result of each action is observable in the next query.
- The assistant can look at the window it operates and the user can show it an image: a
  screenshot the assistant takes or a picture the user pastes reaches the model, every image
  that leaves the machine is visible in the transcript, and a model that cannot see images says
  so instead of failing.
- A GUI click is gated at least as strictly as a device write, because it can press Connect,
  Delete or Save. A remote caller never gets it without the existing per-project consent, and
  the assistant only gets it behind the device-control toggle with a confirmation card.
- A sealed deployment session exposes nothing visual or structural to any client.
- A user action in the connection setup panels and in the Project Editor runs the same command
  an API client would run, with the same validation, licensing gate and error, and records the
  same undo step.
- A user action never acquires the API path's disk side effects: no pre-mutation checkpoint, no
  autosave. Saving means what it means today.
- A migrated action is provably equivalent to its command: the same inputs via the GUI and via
  the API produce the same project state.

## Non-Goals

- Rewriting the UI in another toolkit or language. The QML layer stays, and its reactive read
  bindings (theme, fonts, model state) stay bindings. Only actions move.
- Merging the UI command manifests into the API registry. The manifests keep owning
  presentation metadata (titles, icons, shortcuts, layouts); this spec changes what their
  bindings call, not where metadata lives.
- Migrating view helpers, alarm sound playback, window creation and removal, editor selection
  and tree expansion, widget-setting saves that fire during drag and resize, or licensing
  prompts. These stay direct on purpose: they are latency-sensitive or view state, not document
  or device state.
- A record-and-replay tool, a video stream of the window, or a visual diff service. One
  screenshot per request is the whole surface.
- Screen-reader accessibility. Naming controls makes them addressable; a full accessibility
  pass is separate work.
- Image generation, OCR, or any processing of an image beyond handing it to the model. No
  video and no live screen feed to the model: one capture per tool call.
- Dashboard widget internals (plot curves, gauge needles). Widget tiles are addressable by their
  chrome; what is drawn inside them is verified by other means.
- Native platform dialogs (file pickers, OS message boxes). They are not in the window's item
  tree and cannot be inventoried or pressed; a test reaches the same outcome through the
  path-taking command the dialog would have called.
- A control to revoke an API consent during a session. None of the three consents has one
  today; adding one is separate work and would cover all three at once.

## Requirements

### A. Observing and driving the GUI

1. **R1 — Window inventory.** A client can list every application window (the main window and
   any detached or mini window) with its title, geometry, display scale, whether it is active,
   whether a modal dialog or popup is open in it, and which element holds keyboard focus.
2. **R2 — Element catalog.** For a given window, a client can list the visible interactive
   controls: buttons, toggles, check boxes, radios, combo boxes, text fields, spin boxes,
   segmented controls, menu and toolbar entries, tabs, and dashboard widget tile chrome. Each
   entry reports a stable identifier when the control has one, otherwise its role and label,
   plus its geometry in window coordinates, its enabled and checked state, and its text for
   editable fields. The list can be filtered by identifier or label substring.
3. **R3 — Stable identifiers.** Every shared widget component used across the application
   carries an identifier, and controls generated from the command manifests (ribbon, start menu,
   palette, context menus) use the command id as their identifier. Identifiers do not change
   with the UI language, theme, window size or build variant. A control without an identifier
   still appears in the catalog with its role and label. A control inside a repeated delegate
   (a dashboard tile, a project tree row, a table row) carries an identifier that includes the
   identity of the item it represents, so two tiles never share one. Identifiers are unique
   within a window, and every call that addresses an element names its window, the main window
   being the default.
4. **R4 — Click.** A client can press a control by identifier or by window coordinates, with a
   choice of button, modifier keys and single or double click. A click by identifier lands at the
   control's center, and the reply states whether a control was hit, which one, and the
   coordinates used. Pressing a disabled or invisible control is an error, not a silent no-op.
5. **R5 — Key input.** A client can send text or a key sequence to the focused element, or to a
   named element that takes focus first. Key sequences use the same notation the command
   manifests use for shortcuts.
6. **R6 — Screenshot.** A client can capture a window as a PNG, optionally cropped to one
   element and scaled. The capture reflects the last presented frame and works with HDR output
   enabled.
7. **R7 — Observable completion.** Click and key replies arrive after the input has been
   delivered and processed, so an immediately following element query or screenshot reflects
   it. A click that opens a modal dialog replies once the dialog is visible, not when it closes.
   Replies wait for visibility and enabled state, never for an entrance animation to finish. A
   client can switch the application's reduced-motion preference on and off, so a test that
   compares pixels or times a popup never depends on chrome animation; that switch is a safe
   preference command, not one of the five driving commands.
8. **R8 — Gating.** Window inventory, element catalog and screenshot are read-only commands.
   Click and key carry their own capability class, and a remote caller is refused until a
   consent of its own is granted: a third decision beside the existing device-write and
   script-install consents, with the same lifetime (the session; none of the three is written
   to disk), the same prompt mechanism, the same refusal while the prompt is unanswered, and the
   same headless-consent environment variable granting it for CI. Consents do not compose: a user who allowed device writes for a
   logging script has not allowed remote clicks, and the GUI-driving consent grants no device
   write or script install. For the assistant, click and key sit in the device-gated set, so they are
   blocked unless "Allow device control" is on and always need a confirmation card even with
   auto-approve enabled. Inventory and catalog are safe for the assistant. Screenshot is
   confirm-tier for the assistant: an approval card unless auto-approve is on, because the
   pixels leave the machine.
9. **R9 — Sealed sessions.** In a sealed deployment session all five commands refuse with the
   sealed-session error. Nothing visual or structural leaves a sealed process.
10. **R10 — Discoverability.** The five commands appear in the command catalog, the MCP tool
    list and the API reference like every other command, with their parameters and tiers
    documented. When spec 0102 lands, they join its curated MCP roster so a host finds them
    without discovery.

### B. UI-origin dispatch

11. **R11 — UI semantics.** When a user action dispatches through the command registry it keeps
    today's UI semantics: it records one undo step (keystroke bursts still coalesce), sets the
    modified flag, takes no pre-mutation checkpoint, schedules no autosave, and returns no backup
    path. Rolling backups and the assistant's checkpoints keep working as they do now.
12. **R12 — Origin is local.** A UI-origin dispatch is never treated as a remote request: the
    remote consent gate does not apply, licensing does not see a remote origin, and the
    assistant's safety tiers do not apply. The sealed-session policy applies identically to every
    origin, which matches what the UI refuses today.
13. **R13 — Errors stay visible.** A command refused or failed from the UI produces the same
    user-visible feedback the direct path produced (dialog, notice or inline error). Never a
    silent no-op.
14. **R14 — Parity.** For every migrated action, performing it in the GUI and invoking the same
    command over the API from the same starting state yields the same project state and the
    same number of undo steps.

### C. Stage 1: connection setup panels

15. **R15 — Driver configuration through commands.** Every setting a user changes in a driver's
    setup panel (all fourteen sources) dispatches through the corresponding connection command.
    Readbacks stay bindings. Where no command exists, one is added with the full registration
    ritual (capability class, sealed policy, assistant tier, reference entry).
16. **R16 — No behavior change.** When a setting takes effect does not change: settings that
    apply live keep applying live, settings that apply on the next connect keep doing so, and
    the panels show no added latency.

### D. Stage 2: Project Editor

17. **R17 — Document mutations through commands.** Every document-mutating action in the Project
    Editor (adding, removing, moving and renaming groups, datasets, output controls, tables,
    folders and workspaces; option and widget-type changes; importer and generator flows)
    dispatches through the corresponding project command. Selection, expansion, hover and other
    view state stay direct. Where no command exists, one is added with the full ritual. Importer
    and generator flows keep their own dialogs and end by dispatching the existing load command.
18. **R18 — Dialogs stay in the UI.** An action that confirms with the user (delete
    confirmation, folder prompts, widget-change confirmation) shows its dialog in the UI first
    and dispatches the command only after confirmation. The command itself never opens a dialog.
19. **R19 — Undo granularity unchanged.** Multi-selection fan-outs are one undo step, and
    per-keystroke edits (titles, code) coalesce into one step per burst with no per-keystroke
    document serialization.

### E. Assistant image support

20. **R20 — Images are first-class content.** The assistant's conversation can carry an image
    in a user message and in a tool result. When the assistant takes a screenshot, the model
    receives the image itself, never a text encoding of it. The screenshot tool joins the
    assistant's curated default roster; click and key are reachable only through its discovery
    tools, behind the tiers of R8.
21. **R21 — Provider coverage.** Anthropic, OpenAI and Gemini accept images natively. The local
    provider and OpenAI-compatible endpoints accept them when the selected model advertises
    vision. Where an endpoint reports a model's capabilities, that report decides; where it is
    silent, a per-model toggle in the assistant settings does, so a silent server never blocks
    a user. When the selected model cannot see images, the image is replaced by a short text
    note saying so and the turn continues; the turn never fails because of an image.
22. **R22 — User attachments.** The user can paste or drop a PNG or JPEG into the chat
    composer, sees it there before sending, and can remove it. Attaching is refused with a
    visible note when the selected model cannot see images. Attachments have a size cap and a
    per-message count cap.
23. **R23 — The transcript shows every image the model received.** A capture appears inline in
    its tool card and an attachment inline in its user message, downscaled for display, stored
    with the chat and restored when the chat is reopened.
24. **R24 — Approval shows the capture.** When the screenshot tool needs approval, the card
    shows the captured image before the user approves, so what is approved is what leaves the
    machine. With auto-approve on, the capture is sent and still shown in the card.
25. **R25 — Bounded context.** A capture is downscaled to a bounded size before it is sent.
    Only the most recent few images stay in the model's context verbatim; older ones are
    replaced by a text placeholder, so a long look-then-act session cannot exhaust the window.
    The bounds are the plan's numbers.
26. **R26 — Storage stays inside the chat store.** Captures and attachments are stored with the
    chat, under the assistant's existing storage roots, compressed, and the chat store's growth
    stays bounded. Deleting a chat deletes its images.

### F. Ordering

27. **R27 — Part A, then E, then C, then D.** The driving surface lands first and is used to
    verify every migration. Assistant image support lands second, since its screenshot path is
    Part A's. Stage 1 lands before Stage 2. Each part lands as its own squashed commit.

## Acceptance Criteria

- [ ] **AC1** — pytest (integration): the window inventory lists the main window; after a mini
  window is opened through the UI, it lists two, and the focused element id matches the
  control last clicked. (R1, R7)
- [ ] **AC2** — pytest (integration): the element catalog of the main window contains the
  Connect control and the main toolbar commands by their command ids, and the same ids are
  returned after the UI language is switched through the Preferences controls driven by the
  surface itself. Structural gate: a linter rule fails any identifier expression that is not a
  literal, a command id or an item identity, so a translated or theme-derived identifier cannot
  be written; the rule is blocking. (R2, R3)
- [ ] **AC3** — pytest (integration): with a network source configured, clicking the Connect
  element by id makes the connection state read back true within the usual poll; typing into
  the console send field via key input is reflected in the next catalog query; clicking a
  disabled control returns an error. (R4, R5, R7)
- [ ] **AC4** — pytest (integration): a window screenshot decodes as PNG with dimensions equal
  to the reported geometry times scale, and an element crop matches the element's rect. Plus a
  maintainer observation on an HDR display with HDR output on: the capture is not clamped or
  dim. (R6)
- [ ] **AC5** — pytest (security tier): without consent, a remote click is refused with the
  consent-required error; granting the device-write consent alone does not unlock it; with the
  headless-consent variable set it runs. ctest: the capability-registry and
  sealed-policy-registry tests pass with the five names and the motion switch classified; the
  assistant tier ctest and the static assistant-corpus test show click and key resolve to
  always-confirm with device control on and blocked with it off, inventory and catalog resolve
  to safe, and screenshot resolves to confirm. (R8, R10)
- [ ] **AC6** — pytest (the existing sealed-session integration test): in a sealed session all
  five commands answer the sealed-session error. (R9)
- [ ] **AC7** — ctest: a mutating command dispatched with UI origin records exactly one undo
  step, leaves the project modified, creates no checkpoint file, triggers no autosave write, and
  the remote-origin check reads false while it runs. (R11, R12)
- [ ] **AC8** — pytest (integration), Stage 1: for every driver, each setup-panel control
  changed through click and key input reads back the same driver state as the equivalent
  connection command. Structural gate: a linter count of remaining direct driver property writes
  in setup panels is zero, and it stays a blocking rule. (R15, R16)
- [ ] **AC9** — pytest (integration), Stage 2: a scripted editor session driven through the
  GUI (add group, add dataset, change an option, rename, delete with confirmation) produces
  project JSON identical to the same sequence sent as project commands, and the undo step count
  matches; typing a ten-character title records one undo step. (R14, R17, R18, R19)
- [ ] **AC10** — Regression: the non-destructive integration suite stays green, the JS parser
  units stay green, `--benchmark-hotpath` is untouched (no hotpath code is in scope), and a
  maintainer walk of the Project Editor in the running app finds no changed behavior. (all)
- [ ] **AC11** — Docs and ritual: the API reference documents the five commands and every new
  connection or project command; registry-verify, term-verify and claim-verify pass; the
  command-safety tiers list every new name explicitly. (R10, R15, R17)
- [ ] **AC12** — ctest (provider encoding, beside the existing provider-JSON test): a tool
  result and a user message carrying an image are encoded in each provider's shape, and for a
  model without vision the image becomes the text note and the request still validates. (R20,
  R21)
- [ ] **AC13** — Maintainer observation with a vision model: asking the assistant what the
  dashboard shows produces a screenshot tool call whose approval card shows the capture; after
  approval the reply describes the visible widgets. With auto-approve on, no card appears and
  the capture still shows in the tool card. (R20, R23, R24)
- [ ] **AC14** — Maintainer observation plus ctest: pasting a PNG into the composer shows it
  before sending, the reply references it, and the image is still shown after the chat is
  closed and reopened; the chat-store round trip of an image is a unit test. Attaching under a
  non-vision local model is refused with the note. (R21, R22, R23, R26)
- [ ] **AC15** — ctest (conversation budgeting, beside the existing turn test): with more images
  in the history than the retention bound, only the most recent ones are sent verbatim and the
  rest as placeholders; a capture larger than the size bound is downscaled before encoding.
  (R25)
- [ ] **AC16** — GUI-driving tests carry their own pytest marker and run in the integration
  tier; the first task of the plan is a spike proving click, key and screenshot on the
  offscreen platform, and the marker lets a machine without a presented frame skip them. (R6,
  R7)

## Constraints & Invariants

- **A GUI click is the most powerful command in the system.** It inherits the strictest gates
  that exist and never a weaker one. The API server's consent and auth files are a
  decision-visible surface: the gating change is named in chat with its failure modes before it
  lands and never rides inside a bulk package.
- **Consents do not compose.** The GUI-driving consent is a third decision, never a reuse of the
  device-write or script-install one in either direction. The remote gate answers each
  capability with its own consent, as it does today.
- **UI-origin dispatch is local.** Licensing decides by whether a remote origin is active, so a
  UI-origin scope must never read as remote. The assistant's tiers must not apply to it either.
- **No disk side effects from a click.** No pre-mutation checkpoint, no autosave, no backup path
  in the reply. Saving remains explicit.
- **Undo history contract (spec 0031) holds.** Every document mutation records a step through
  the existing two-phase mechanism; keystroke coalescing stays; the presentation setters that
  are deliberately outside history stay outside.
- **Nothing on the hotpath.** The element catalog is built on demand per request, never
  maintained per display tick, and no per-frame work is added anywhere. The 256 kHz gate is not
  in scope and must not move.
- **Nested event loops are the hazard.** A click that opens a modal dialog must reply when the
  dialog is visible; a reply that waits for the dialog to close wedges the client and the
  display tick. No new blocking call from the server thread into the GUI beyond what command
  execution already does.
- **Sealed sessions leak nothing.** All five surface commands are classified as refused under
  seal, and no migrated action may create a new exit for project content.
- **Operation-mode switches stay implicit (spec 0100).** A migrated load or generation path
  still goes through the deferred switch, never the explicit setter.
- **License gates do not move.** Migrating an action never changes which license, trial or
  entitlement gate it answers to, and Pro-only commands stay out of the GPL catalog.
- **Identifiers are language- and theme-independent.** A test written against an id keeps
  passing under every translation and theme.
- **Read bindings are untouched.** The thousands of property reads that make the UI reactive
  are not in scope; only actions move.
- **No new third-party dependency, and no test-only toolkit module linked into the shipped
  binary.**
- **Every new command completes the full registration ritual** (capability class, sealed
  policy, assistant tier, API reference entry). The two ctests that fail on an unclassified name
  are the backstop, not the process.
- **The redactor cannot scrub pixels.** Key and credential fields stay masked on screen, so a
  capture of them shows masks, and the approval card shows the capture before anything is sent.
  A sealed session starts no assistant turn, so no capture leaves one.
- **Images never break a turn.** A provider or model without vision degrades to the text note;
  an oversized or unreadable attachment is refused at attach time, never mid-turn.
- **The assistant's trust boundaries hold.** Images live under the assistant's existing
  storage roots and the key vault is untouched; no image is written outside the sandbox.
- **Landing shape.** Four squashed commits, one per part in the order of R27, each leaving the
  suite green and the spec folder shared; the spec moves to done after Stage 2.

## Open Questions

- **Role vocabulary.** The catalog reports a role for every control. Recommendation: use the
  accessibility role names, so a later screen-reader pass reuses the same identifiers and
  roles instead of adding a second naming.
- **Offscreen behavior.** Whether a presented frame exists on the offscreen platform for the
  screenshot command is unverified. The plan's first task is the spike; if the platform cannot
  present, the GUI marker skips there and CI runs the GUI tier on one platform with a real
  windowing system.
