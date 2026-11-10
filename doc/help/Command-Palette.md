# Command palette

You have a device connected, the dashboard is showing, and you want a CSV log of it. Press **Ctrl+K** (**Cmd+K** on macOS), type `csv`, and press Enter on **CSV File**. Logging starts. No menu was opened. **CSV File** is a toggle, so pressing Enter on it again stops the log.

The command palette is one search box over your dashboard layouts (workspaces), the widgets on them, and every menu action in Serial Studio. It is in the free and Pro editions. Commands for Sessions, File Transmission, the AI Assistant and Notifications appear only in the Pro edition.

## Opening and closing

Press **Ctrl+K** (**Cmd+K** on macOS) to open the palette, and again to close it. **Esc**, **Ctrl+W** (**Cmd+W**) and a click outside the dialog close it too. The keys work in the main window, the Project Editor, and pop-out dashboard windows. In the Project Editor, **Ctrl+K** does nothing while the project is locked.

Two buttons open it as well: the workspace button in the dashboard taskbar, and the **Command Palette** button on the main toolbar. The palette is modal: the window behind it ignores input until you close it.

## What the palette lists

- **On the dashboard** it lists your [workspaces](Toolbar-Reference.md#a-note-on-workspaces) and the dashboard commands, such as **Pause** and the export toggles. A workspace is a saved layout of dashboard widgets. Typing also searches workspace folders, dataset groups, and open widgets.
- **In Console view, or while the dashboard is not showing,** it lists only the application commands: file actions, operation-mode switches, connection, preferences, and help. There are no workspace, group, or widget entries. Commands that need the dashboard, such as the View entries, are listed dimmed.
- **In the [Project Editor](Project-Editor.md)** it lists the project commands: new, open, save, and the add-item actions from the editor toolbar.

In runtime mode (an operator deployment) the authoring commands, such as the Project Editor and Open Project, are hidden.

## Browsing versus searching

The palette switches mode on its own, depending on whether the search box is empty.

### Browse (empty search box)

An empty box shows a grid of large cells. On the dashboard the first row is **Workspaces**, ending in an **Add Workspace** cell; commands follow in labeled sections, one per category. A cell marked **Folder** opens that workspace folder. Use **Back** or the breadcrumb bar at the top to go up a level. Inside a folder the grid shows only that folder's workspaces and an **Add Workspace** cell, which creates the workspace in that folder; the command sections appear only at the top level.

### Search (any text typed)

The grid becomes a dense list of results. Each row shows the item's icon, its name, a dimmed path or category, and its keyboard shortcut where one exists. Typing `csv` shows a row named **CSV File** under **Data Export**, with its shortcut at the right.

Search ignores case. Each space-separated word must appear somewhere in the item's name, in any order, so `export csv` and `csv export` find the same entries. Words match as substrings. The characters `-`, `_`, `.` and `/` count as spaces, and a word also matches the name with its spaces removed, so `csvfile` finds **CSV File**. Workspaces, folders and commands match on their own name only, not the dimmed category or path. Groups and widgets also match on the name of the group they belong to, so typing a group's name lists its widgets.

The same search reaches widgets: type part of a widget's name and Enter jumps to it. Typing `pause` finds **Pause**; while the dashboard is paused the same entry reads **Resume**.

On the dashboard, results fall into these sections, each hidden when empty:

| Section | Contains |
|---------|----------|
| **Folders** | Workspace folders whose name matches. |
| **Workspaces** | Workspaces whose name matches. |
| **Groups** | Dataset groups of the loaded project whose name matches. |
| **Widgets** | Open widgets whose name matches. |
| **File**, **Operation Mode**, **Connection**, **View**, **Data Export**, **Console**, **Project**, **License**, **Tools**, **Help** | Matching commands, one section per category, in that order. |

**Groups** and **Widgets** together show at most 30 results; add a word to narrow the query if the entry you want is missing.

## Keyboard and Enter

The search box keeps focus the whole time, so your hands never leave the keyboard. In search results, **Up** and **Down** (or **Left** and **Right**) move one result. In the browse grid, **Up** and **Down** move a row of cells, and **Left** and **Right** move one cell. Movement stops at the first and last entry instead of wrapping, and the first arrow press highlights the first entry.

**Enter** acts on the highlighted entry, or on the first one if nothing is highlighted:

- A workspace: the palette switches to it and closes.
- A widget: the palette shows it in the workspace that contains it. If no workspace contains it, the widget opens in its own window. The palette closes.
- A command: the palette runs it and closes. Dimmed commands are unavailable right now and ignore **Enter**.
- A **Folder** cell drills into the folder, and **Add Workspace** prompts for a new workspace. The palette stays open for both.

**Esc** always closes the palette; it never steps up a folder level.

## See also

- [Toolbar & Button Reference](Toolbar-Reference.md): the buttons these commands mirror.
- [Macros](Macros.md): the command terminal and script editor.
- [Problem Center](Problem-Center.md): the window behind the **Problem Center** and **Connection Diagnostics** entries.
- [Project Editor](Project-Editor.md): where the project commands live.
