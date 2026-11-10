# Backups & Recovery

You deleted a dataset, saved, and only then noticed you needed it. Serial Studio has already written a snapshot of the project from before the change. Open the **Project Editor**, click the **Restore Backup** toolbar button, pick the row, and click **Restore** in the dialog. The steps are under [Restoring a backup](#restoring-a-backup).

A snapshot is a copy of your whole `.ssproj` (the file your project is saved in), written in the background while you work. Backups are on in every edition, GPL and Pro alike, and there is no setting to turn them off. The one exception is a [sealed session](Operator-Deployments.md#sealed-sessions) (a Pro session opened from a deployment package), which never writes snapshots so that its project never reaches disk in readable form.

> Backups protect the project *definition* (sources, groups, datasets, frame parsers, transforms, widgets, workspaces). They are not a recording of your telemetry. To archive incoming data, see [Historian](Historian.md) or [CSV Export & Playback](CSV-Export-Playback.md).

For step-by-step reversal of single edits, use **Undo** and **Redo** on the Project Editor toolbar. Backups restore a whole earlier version of the project.

## When a snapshot is taken

Serial Studio writes a snapshot:

- On load. Opening a project writes a baseline snapshot (`Project Loaded`) before you touch anything.
- After an edit. Each edit restarts a 5-second timer, so a snapshot is written 5 seconds after you stop editing and a burst of quick edits produces one snapshot. An edit made in the last 5 seconds before a crash is not captured.
- Before a destructive command. Deleting something by hand in the Project Editor takes no pre-delete snapshot; Undo and the 5-second automatic snapshot cover it. Scripts and the AI Assistant that go through the API Server's `project.*` commands or the AI Assistant's tools take a labelled snapshot first when they create, open or load a project, apply a template, delete or move datasets and groups, delete actions, output widgets or workspaces, clear or reorder workspaces, remove a widget, or run a batch. A dry run takes none. Labels such as `Before Delete Group` come only from these paths.
- On request. The AI Assistant can force a labelled checkpoint before a multi-step edit.

A snapshot whose content matches the newest one is skipped. An older matching snapshot does not count, so going from A to B and back to A writes a third file.

Snapshots cover edits made inside Serial Studio. For a program that changes the `.ssproj` on disk, see [Project Editor → If the project file changes on disk](Project-Editor.md#if-the-project-file-changes-on-disk).

## Where backups live

To find your backups, open the **Recover Backup** dialog and click **Open Folder**. It reveals the folder in your file manager.

For reference, the layout is:

```
<app data folder>/backups/<project-basename>-<hash>/<timestamp>[__label].ssproj
```

- The subfolder is the project's base name plus a short hash of its full path, so two projects with the same file name in different folders keep separate histories. A project you have not saved yet is filed under `untitled`.
- File names use a UTC timestamp, for example `2026-11-10T14-03-22-418__auto.ssproj`; the dialog shows local time. Labels keep letters, digits, `-` and `_` (spaces and dots become `-`, other characters are dropped) and are cut to 32 characters.
- Each file is your project, stored compactly, plus a small `_backupMeta` block that restore strips. A snapshot written by a newer Serial Studio is refused on restore.
- Serial Studio keeps the **50 most recent** snapshots per project and deletes the oldest beyond that. Automatic, load and before-a-command snapshots share that pool, so a long editing session can push out the `Project Loaded` baseline.

## Restoring a backup

1. Open the [Project Editor](Project-Editor.md).
2. Click the **Restore Backup** toolbar button, to the right of **Undo** and **Redo** (tooltip: "Restore a recent automatic snapshot of the current project"; also in the command palette). The **Recover Backup** dialog opens.
3. Select a snapshot. A one-line preview says what restoring would change: groups added or removed, a changed title, or a different frame parser.
4. Click **Restore** in the dialog, or double-click the row.

The list shows up to 50 recent snapshots, newest first, each with the time it was taken and a label saying why: `Project Loaded`, `Auto-save`, `Before Delete Group`, `Before Apply Template`, `Before Batch Operation`, `Before Restore`, or a checkpoint name. A label the app does not recognise is shown title-cased, so a checkpoint named `pre-flash-test` appears as `Pre Flash Test`.

Picked the wrong snapshot? Restore again. Before loading your pick, Serial Studio saves the current state as `Before Restore`. If that state already matches the newest snapshot, no extra file is written and the newest row is the one to go back to.

The dialog's **Restore** loads the snapshot into the project and marks it modified. Nothing is written to your `.ssproj` until you save.

If the dialog says "No backups for this project yet. Edit or save the project to start the rolling backup.", no snapshot exists for it: the project is new and has not been edited yet, or the session is sealed.

## Rolling back with the AI Assistant

The [AI Assistant](AI-Assistant.md) (Pro, see [Pro vs Free](Pro-vs-Free.md)) restores from the same snapshots through the `assistant.checkpoint` and `assistant.restore` tools. Ask it to "undo that", "roll back the last change", or "revert to before you deleted that group" and it will:

1. List the recent checkpoints.
2. Show you the list; you pick one and approve the confirmation prompt, and it restores that snapshot.
3. Report the `Before Restore` snapshot it took, so the rollback is itself undoable.

If the project has been saved to a file, the assistant's restore writes it back to that file immediately. For an unsaved project it stays modified in memory. Every command that takes a snapshot first (see [When a snapshot is taken](#when-a-snapshot-is-taken)) returns the path of the snapshot taken beforehand, so a bad automated edit is one restore away.
