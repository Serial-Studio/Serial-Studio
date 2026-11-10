# Macros

You want to send the same command to your board at the start of every session, or check the link state without clicking through three panels. The Macros window does that: type one command and read the reply, or save a short JavaScript or Lua script and run it whenever you need it.

## Overview

Every action in Serial Studio is a named command, such as `io.connect` or `project.dataset.add`. The Macros window runs them inside the app; no API server needs to be on.

Macros are available in every edition, GPL builds included. The window opens only in author mode: in a runtime-mode (operator) deployment the Macros command does nothing. The command catalog is whatever the running build registers; a command from a driver the build does not ship is not listed, and calling it returns the unknown-command error.

Press **Ctrl+K** (**Cmd+K** on macOS) to open the [command palette](Command-Palette.md), type `Macros`, and press Enter. The **Macros** entry is in the **Tools** category and appears in the palette from both the main window and the dashboard. The main toolbar also has a **Macros** button, next to the Remote Attach and Problems buttons.

The left pane of the window is the command catalog: a search field, the command list grouped by scope, and a documentation panel for the selected command. The right pane holds two tabs, **Terminal** and **Script**.

A macro acts immediately: it can change your project and write to the connected device with no confirmation prompt. Macros run in the application as the local user, with no remote client in the loop.

## Quick start

1. Press **Ctrl+K**, type `Macros`, and press Enter.
2. Open the **Script** tab. It is pre-filled with a JavaScript starter macro that calls `api.getCommands` and prints the first five command names (abbreviated here to one).
3. Click the **Run macro** button (the play icon). The window switches to the **Terminal** tab and prints output like this; `N` is the number of commands in your build. The starter ends each line with `\n`, so blank lines appear between entries:

```text
> [macro] run (js)
Hello from Serial Studio! N commands available.

  api.getCommands - Returns every command the live server exposes ...

  ...
[macro] finished
```

4. Switch back to **Script**, edit the macro, and run it again. Click **Verify macro** first to catch a syntax error without executing anything.

To talk to the board, connect the device from the main window, then replace the macro with this one. `apiCall(name, params)` runs one command and returns `{ ok: true, result: ... }` or `{ ok: false, error: "..." }`; the three lines check for the second form. `console.send` uses the Console's line ending and encoding settings. With no device connected it writes nothing but still replies `sent: true`, so check `io.getStatus` `isConnected` first if that matters:

```javascript
const reply = apiCall("console.send", { data: "LED ON" })
if (!reply.ok)
  throw new Error(reply.error)
```

## Terminal tab

The Macros window's Terminal tab runs one command per line. The input row at the bottom takes a command name followed by an optional JSON object of parameters:

```text
io.getStatus
project.getStatus
project.group.add { "title": "Sensors", "widgetType": 5 }
```

The first two lines only read state. The third adds a group titled `Sensors` to the open project; `5` is the `NoGroupWidget` value of `widgetType`, a plain container for related datasets.

Whitespace after the name starts the parameters. The parameters must parse as a JSON object; a malformed object or a JSON array is refused with `Invalid parameter JSON` or `Parameters must be a JSON object` before anything is dispatched. Press Enter or click the **Run command** button to run the line. The scrollback echoes the line with a `>` prefix, then prints the reply as indented JSON in the same shape the API server sends over the wire, including the error object for an unknown command.

While the first token is being typed, a completion popup opens above the input row. It lists every catalog name that starts with the token, then every name that contains it. **Up** and **Down** move through the popup, **Tab** or **Enter** accepts the highlighted name, and **Esc** closes the popup. Accepting a completion inserts the name together with a parameter skeleton: one key per parameter, prefilled with `""`, `0`, `false`, `[]`, or `{}` according to the parameter's type. Double-clicking a command in the catalog inserts the same skeleton.

With the popup closed, **Up** and **Down** recall earlier lines. History lives only for the current session. The **Clear output** button empties the scrollback, and the right-click menu offers **Copy**, **Select All**, and **Clear**.

## Command catalog

The left pane of the Macros window is the command catalog. Its search field filters the list by command name and description. The list is grouped by scope, the part of the name before the first dot, and each row shows the verb after the dot with its description underneath. Selecting a row, or typing an exact command name in the Terminal tab's input row, fills the documentation panel below the list with the full name, the description, and every parameter as `name (type)`, with an asterisk after the parameters a command requires. A command without parameters shows `No parameters`.

## Script tab

The Script tab holds the macro editor, a code editor with syntax highlighting for the selected language, and a toolbar. The toolbar buttons are icons; hover one to see the name used below.

| Control | Effect |
|---------|--------|
| **JavaScript** / **Lua** | Selects the macro language and the highlighter. Switching replaces the starter macro with the other language's starter when the editor is empty or still holds an unedited starter; edited text is kept. |
| **Load macro** | Opens a `.js` or `.lua` file into the editor. Its suffix sets the language. |
| **Save macro** | Writes the editor text to a file. Its default suffix follows the selected language. |
| **Verify macro** | Compiles the text without running it and prints the verdict to the Terminal tab. |
| **Run macro** | Runs the text, switching to the Terminal tab for the output. Disabled while a macro is running. |
| **Stop macro** | Interrupts a running JavaScript macro. Disabled for Lua and while nothing is running. |
| **Clear editor** | Empties the editor and its undo history. |

**Load macro** and **Clear editor** ask `Discard changes?` when the editor holds edits that have not been saved. Closing the window keeps the editor text and language for the rest of the session, so the draft is still there when the window is opened again; quitting the application discards it.

### What a run prints

Every run starts with a header line, `> [macro] run (js)` or `> [macro] run (lua)`, in the Terminal scrollback. Output from `console.log()` and `print()` follows as it is produced. When the script ends, the scrollback shows the value of the script's final expression (JavaScript) or of its `return` statement (Lua); a script that produces no value ends with `[macro] finished`.

A failed run prints one line prefixed with `[macro]`:

| Message | Meaning |
|---------|---------|
| `[macro] Line 4: Error: boom` | A JavaScript macro threw or hit a runtime error on line 4. |
| `[macro] [string "macro"]:4: boom` | A Lua macro raised an error on line 4. |
| `[macro] Macro stopped by user` | **Stop macro** interrupted a JavaScript macro. |
| `[macro] Macro exceeded the 5000 ms script budget between API calls and was interrupted` | A JavaScript macro spent more than 5 s in script code without calling the host. |
| `[macro] macro exceeded the 30000 ms safety deadline` | A Lua macro ran for more than 30 s in total. |
| `[macro] Macro is empty` | The editor held only whitespace. |

**Verify macro** prints `[macro] verify: no syntax errors` or `[macro] verify failed: <error>`. Both languages name the line: JavaScript as `Line 3: SyntaxError: Expected token ';'`, Lua as `[string "macro"]:3: '=' expected near 'x'`. Verify compiles the text without running it. A JavaScript source is compiled inside a function wrapper under a 2 s budget, so a top-level `return` passes Verify but fails at run time.

## JavaScript and Lua macros

Use JavaScript unless the macro needs `actionFire()` or a dashboard setter. A Lua macro runs on the main window thread and freezes the window until it returns, so one that loops forever holds the whole application for thirty seconds. A host call is any `apiCall`, `delay` or `deviceWriteAndWait`; it hands control back to Serial Studio. The two languages run under different execution models:

| | JavaScript | Lua |
|---|---|---|
| Runs on | Its own worker thread. Window and dashboard stay responsive. | The main window thread. Everything else waits until the macro returns. |
| **Stop macro** | Interrupts at the next host call, sleep, or device wait, and within 5 s of pure script code. | Not available. |
| Time limit | 5 s of script time between host calls. Each `apiCall`, `delay`, and `deviceWriteAndWait` restarts the budget. | 30 s for the whole run. |
| `delay(ms)` | Sleeps the macro without blocking the interface; capped at 1 hour. | Not available. |
| Output | `console.log()`, `console.info()`, `console.warn()`, `console.error()`, and `print()` each write one line to the scrollback, arguments joined by a space. | `print()` writes one line, arguments joined by a tab. |
| Engine | A fresh engine per run; no variable survives into the next run. | A fresh state per run. The `ffi` and `jit` modules are removed and the JIT compiler is off. |

A JavaScript macro that loops forever with no host call is interrupted by the 5 s budget on its own. A Lua macro in the same loop freezes the interface until the 30 s deadline ends the run, and host calls do not extend it. Split long work into several runs, or write it in JavaScript.

### SDK helpers inside a macro

Both languages preload `apiCall(method, params)`, `apiCallList()`, and the generated wrappers (`api.getCommands()`, `io.getStatus()`, `project.dataset.add(...)`, and the rest of the [SDK call surface](SerialStudio-SDK.md)). `apiCall` returns the same envelope in both: `{ ok: true, result: ... }` or `{ ok: false, error: "..." }`.

The direct host helpers are not installed uniformly:

| Helper group | JavaScript macro | Lua macro |
|---|---|---|
| `apiCall`, `apiCallList`, generated wrappers | yes | yes |
| `delay`, `deviceWriteAndWait` | yes | no |
| `newFrame`, `refreshDashboard`, `dashboardTick`, `ensureDashboard` | yes | no |
| `tableGet` / `tableSet` and the handle variants | yes, routed through `apiCall` | yes |
| `notify*` (Pro) | yes, routed through `apiCall` | yes |
| `deviceWrite`, `actionFire`, dashboard setters (`clearPlots`, `setActiveWorkspace`, ...) | no | yes |

In JavaScript, `apiCall("console.send", { data: "..." })` takes the place of `deviceWrite()`. A macro that needs `actionFire()` or the dashboard setters runs in Lua. Calling an absent helper raises a `ReferenceError`.

### Examples

Poll the link state once a second for five seconds (JavaScript):

```javascript
for (let i = 0; i < 5; ++i) {
  const reply = apiCall("io.getStatus")
  if (!reply.ok)
    throw new Error(reply.error)

  console.log(reply.result.isConnected ? "link up" : "link down")
  delay(1000)
}
```

Count the commands in each scope (Lua):

```lua
local reply = apiCall("api.getCommands")
if not reply.ok then
  error(reply.error)
end

local scopes = {}
for _, cmd in ipairs(reply.result.commands) do
  local scope = cmd.name:match("^[^.]+")
  scopes[scope] = (scopes[scope] or 0) + 1
end

for scope, count in pairs(scopes) do
  print(scope, count)
end
```

## Macro files

A macro is a plain text file with a `.js` or `.lua` suffix. The Macros window's **Load macro** and **Save macro** dialogs open in the `Macros` folder of the application data directory (created on first use); any other folder can be chosen from the dialog. Loading picks the language from the suffix: `.lua` selects Lua, anything else selects JavaScript. Saving writes the file atomically and prints `[macro] saved <name>`; loading prints `[macro] loaded <name>`.

## Limits

| Limit | Value |
|-------|-------|
| JavaScript script budget between host calls | 5000 ms |
| Lua run deadline | 30000 ms |
| `delay()` cap | 3600000 ms (1 hour) |
| Verify budget (JavaScript) | 2000 ms |

A macro can call any command the catalog lists, including the mutating `project.*` commands. Commands that offer a `dryRun` parameter honor it from a macro as well.

## See also

- [API server](API-Reference.md) and [SDK](SerialStudio-SDK.md): remote programs reach the same commands through the API server; frame parsers, transforms and the Control Loop reach them through the SDK.
- [SDK Reference](SerialStudio-SDK.md): the `apiCall` envelope, the generated wrappers, and the helper groups.
- [API Reference](API-Reference.md): every command, its parameters, and the reply shapes the Terminal tab prints.
- [Control Loop](Control-Loop.md): the project-scoped script that runs for the life of a connection, the counterpart to an ad-hoc macro.
