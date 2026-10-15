# Macros

An in-process command terminal and script editor. The Macros window runs single API commands and multi-line JavaScript or Lua scripts against the running application, with the same command surface that the API server exposes over TCP, but without a socket, a client program, or the API server being enabled.

## Overview

Serial Studio registers every operation it can perform as a named command (`io.connect`, `project.dataset.add`, `dashboard.tick`, and so on). Remote programs reach those commands through the [API server](API-Reference.md); frame parsers, transforms, and the Control Loop reach them through the [SDK](SerialStudio-SDK.md). Macros are the third way in: type a command by hand, or write a script that calls several, and watch the replies in a scrollback.

Macros are available in every edition, GPL builds included. Commands that belong to Pro drivers exist only in a build that ships them, so the catalog in the window is exactly what the running build registers.

Open the window from the [command palette](Command-Palette.md): press **Ctrl+K**, type `Macros`, and press Enter. Its entry sits in the **Tools** category and is listed from the main window and the dashboard. There is no toolbar button and no Start menu entry.

Two panes make up the window. The left pane is the command catalog: a search field, the command list grouped by scope, and a documentation panel for the selected command. The right pane holds two tabs, **Terminal** and **Script**.

Commands run as the local user, so device writes issued from a macro go straight through; the consent prompt that remote API clients receive before a device write does not apply here.

## Quick start

1. Press **Ctrl+K**, type `Macros`, and press Enter.
2. Open the **Script** tab. It is pre-filled with a starter macro that calls `api.getCommands` and prints the first five command names.
3. Click the **Run macro** button (the play icon). The window switches to the **Terminal** tab and prints:

```
> [macro] run (js)
Hello from Serial Studio! ... commands available.
  api.getCommands - Returns every command the live server exposes ...
  ...
[macro] finished
```

4. Switch back to **Script**, edit the macro, and run it again. Click **Verify macro** first to catch a syntax error without executing anything.

## Terminal tab

The Terminal tab runs one command per line. The input row at the bottom takes a command name followed by an optional JSON object of parameters:

```
io.getStatus
project.group.add { "title": "Sensors", "widgetType": 5 }
```

Whitespace after the name starts the parameters. The parameters must parse as a JSON object; a malformed object or a JSON array is refused with `Invalid parameter JSON` or `Parameters must be a JSON object` before anything is dispatched. Press Enter or click the **Run command** button to run the line. The scrollback echoes the line with a `>` prefix, then prints the reply as indented JSON in the same shape the API server sends over the wire, including the error object for an unknown command.

While the first token is being typed, a completion popup opens above the input row. It lists every catalog name that starts with the token, then every name that contains it. **Up** and **Down** move through the popup, **Tab** or **Enter** accepts the highlighted name, and **Esc** closes the popup. Accepting a completion inserts the name together with a parameter skeleton: one key per parameter, prefilled with `""`, `0`, `false`, `[]`, or `{}` according to the parameter's type. Double-clicking a command in the catalog inserts the same skeleton.

With the popup closed, **Up** and **Down** recall earlier lines. History lives only for the current session. The **Clear output** button empties the scrollback, and the right-click menu offers **Copy**, **Select All**, and **Clear**.

## Command catalog

The search field filters the list by command name and description. The list is grouped by scope, the part of the name before the first dot, and each row shows the verb after the dot with its description underneath. Selecting a row, or typing an exact command name in the input row, fills the documentation panel below the list with the full name, the description, and every parameter as `name (type)`, with an asterisk after the parameters a command requires. A command without parameters shows `No parameters`.

## Script tab

The Script tab holds the macro editor, a code editor with syntax highlighting for the selected language, and a toolbar:

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
| `[macro] macro:4: boom` | A Lua macro raised an error on line 4. |
| `[macro] Macro stopped by user` | **Stop macro** interrupted a JavaScript macro. |
| `[macro] Macro exceeded the 5000 ms script budget between API calls and was interrupted` | A JavaScript macro spent more than 5 s in script code without calling the host. |
| `[macro] macro exceeded the 30000 ms safety deadline` | A Lua macro ran for more than 30 s in total. |
| `[macro] Macro is empty` | The editor held only whitespace. |

**Verify macro** prints `[macro] verify: no syntax errors` or `[macro] verify failed: <error>`. Both languages name the line: JavaScript as `Line 3: SyntaxError: Expected token ';'`, Lua as `macro:3: '=' expected near 'x'`. Verifying never executes the script; a JavaScript source is compiled inside a function wrapper under a 2 s budget.

## JavaScript and Lua macros

The two languages run under different execution models, and the difference matters for anything longer than a few calls:

| | JavaScript | Lua |
|---|---|---|
| Runs on | Its own worker thread. Window and dashboard stay responsive. | The interface thread. Everything else waits until the macro returns. |
| **Stop macro** | Interrupts at the next host call, sleep, or device wait, and within 5 s of pure script code. | Not available. |
| Time limit | 5 s of script time between host calls. Each `apiCall`, `delay`, and `deviceWriteAndWait` restarts the budget. | 30 s for the whole run. |
| `delay(ms)` | Sleeps the macro without blocking the interface; capped at 1 hour. | Not available. |
| Output | `console.log()`, `console.info()`, `console.warn()`, `console.error()`, and `print()` all write one line to the scrollback. | `print()` writes one line, arguments joined by tabs. |
| Engine | A fresh engine per run; no variable survives into the next run. | A fresh state per run. The `ffi` and `jit` modules are removed and the JIT compiler is off. |

A JavaScript macro that loops forever with no host call is interrupted by the 5 s budget on its own. A Lua macro in the same loop freezes the interface until the 30 s deadline fires, so pace long Lua work through host calls or keep it in JavaScript.

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

In JavaScript, `console.send(...)` takes the place of `deviceWrite()`. Firing an action and toggling the dashboard utilities have no command equivalents, so a macro that needs `actionFire()` or the dashboard setters runs in Lua. Calling an absent helper raises a `ReferenceError`.

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

Macros are plain text files with a `.js` or `.lua` suffix. **Load macro** and **Save macro** open in the `Macros` folder under the application data directory (`<AppData>/Macros`, created on first use), and any other folder can be chosen from the dialog. Loading picks the language from the suffix: `.lua` selects Lua, anything else selects JavaScript. Saving writes the file atomically and prints `[macro] saved <name>`; loading prints `[macro] loaded <name>`.

## Limits

| Limit | Value |
|-------|-------|
| JavaScript script budget between host calls | 5000 ms |
| Lua run deadline | 30000 ms |
| `delay()` cap | 3600000 ms (1 hour) |
| Verify budget (JavaScript) | 2000 ms |
| Command history | Current session only |
| Macro draft | Kept while the window is closed, discarded on quit |

A macro can do exactly what a local API client can do: every registered command, including the mutating `project.*` commands. Commands that offer a `dryRun` parameter honor it from a macro as well.

## See also

- [SDK Reference](SerialStudio-SDK.md): the `apiCall` envelope, the generated wrappers, and the helper groups.
- [API Reference](API-Reference.md): every command, its parameters, and the reply shapes the Terminal tab prints.
- [Control Loop](Control-Loop.md): the project-scoped script that runs for the life of a connection, the counterpart to an ad-hoc macro.
- [Command Palette](Command-Palette.md): the **Ctrl+K** palette that opens the Macros window.
