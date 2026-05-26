# Backups & Recovery

Serial Studio keeps rolling backups of your project so you can undo edits and recover from a mistake or a crash. Every time you change the project, a snapshot of the whole `.ssproj` is written to disk in the background. If an edit goes wrong, you restore an earlier snapshot from the **Recover Backup** dialog, or ask the AI Assistant to do it for you.

Backups are always on and available in every build, GPL and Pro alike. There is nothing to set up.

> Backups protect the project *definition* (sources, groups, datasets, frame parsers, transforms, widgets, workspaces). They are not a recording of your telemetry. To archive incoming data, see [Session Database](Session-Database.md) or [CSV Export & Playback](CSV-Export-Playback.md).

## When a snapshot is taken

Serial Studio writes a snapshot:

- **On load.** Opening a project writes a baseline snapshot before you touch anything.
- **After an edit.** Any change marks the project modified; about five seconds later a snapshot is written. The delay is a debounce, so a burst of rapid edits collapses into a single snapshot rather than dozens.
- **Before a risky operation.** Deleting a group or dataset, moving items, clearing workspaces, applying a template, opening another project, and similar destructive actions each take a snapshot first.
- **On request.** The AI Assistant can force a labelled checkpoint before a multi-step edit.

Identical content is never written twice: if the project bytes match the previous snapshot, the existing file is kept instead of writing a duplicate.

## Where backups live

Snapshots are plain `.ssproj` files (the same format as your project, stored compactly), grouped by project name:

```
<AppData>/backups/<project-name>/<timestamp>[__label].ssproj
```

A project you have not saved yet is filed under `untitled`. The **Open Folder** button in the Recover Backup dialog reveals this location in your file manager.

Serial Studio keeps the **50 most recent** snapshots per project and deletes the oldest beyond that.

## Restoring a backup

Open the **Project Editor**, then click **Restore** on the toolbar (next to **Lock**). The **Recover Backup** dialog lists recent snapshots, newest first, each showing:

- the time it was taken, in your local timezone, and
- a label describing why it was taken (`Project Loaded`, `Auto-save`, `Before Delete Group`, a checkpoint name, and so on).

Select an entry to see a one-line preview of what restoring would change (groups added or removed, a renamed title). Click **Restore**, or double-click the row, to load that snapshot back into the project.

**Restoring is reversible.** Before loading the chosen snapshot, Serial Studio snapshots your current state first (labelled *Before Restore*), so if you pick the wrong one you can restore your way back. The restored project is also written straight to your project file, so the recovery survives a crash before your next manual save.

If the list is empty, no edits have been made to this project yet. Edit or save the project to start the rolling backup.

## Undoing with the AI Assistant

The [AI Assistant](AI-Assistant.md) (Pro) uses these same snapshots. Ask it to "undo that", "roll back the last change", or "revert to before you deleted that group" and it will:

1. List the recent checkpoints.
2. Show them to you and ask which one to restore.
3. Restore the one you pick, after you approve the confirmation prompt.
4. Report the reverse snapshot it took, so the undo is itself undoable.

Every destructive command the assistant runs also returns the path of the snapshot taken beforehand, so a bad automated edit is always one restore away. The assistant never restores without showing you the list first.
