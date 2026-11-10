# Project Lock

You built a dashboard for a machine on the shop floor, and you would rather the person running it did not rewire the frame parser. Project Lock puts a password in front of the Project Editor. The dashboard keeps running.

Project Lock is available in the free and Pro editions. It covers the Project Editor only; the [Historian](Historian.md) session database has its own, separate password.

## Lock a project

1. Open the project in the Project Editor.
2. Click **Lock** in the Project Editor toolbar. It sits in its own section, right of Undo, Redo and Restore. The command palette entry is **Lock Project Editor**.
3. Enter a password in the **Lock Project** prompt.
4. Type the same password again in the confirmation prompt. The two entries must match exactly.

The editor body is then replaced by a blurred screen titled "This project is password protected". Locking clears the Undo and Redo history and saves the project at once if it passes validation. If validation fails, the lock is active in memory only; fix the validation error and save to write the hash. Any non-empty password is accepted; there is no length or complexity rule. The stored format is described in [What lives in the project file](#what-lives-in-the-project-file).

Leaving the first field empty, or clicking Cancel on it, cancels silently. An empty or mismatched confirmation, or Cancel on the confirmation prompt, shows a "Passwords do not match" warning and leaves the project unlocked.

## What the dashboard sees

Nothing changes. The lock gates the Project Editor body and nothing else: connecting, switching workspaces, widget layouts on the dashboard, frame parsing, exports, reports and the API Server still runs.

## Unlock a project

In Project File mode, a locked project shows the Project Editor's blurred screen with three buttons:

- **Unlock** asks for the password.
- **Open Other Project** opens a different project file.
- **Create New Project** starts a blank one.

Only **Unlock** opens this project's editor. A correct password clears the hash, restores the editor and saves the project again (validation permitting), and clears the Undo and Redo history. A wrong password shows an "Incorrect password" warning and the lock stays.

Unlocking is also how you remove the lock for good or change the password. Unlock, then either leave the project unlocked or click **Lock** again with a new password.

Loading a locked project never asks for the password. The dashboard runs, and the editor stays hidden until someone clicks **Unlock**.

## What the lock does not do

The lock stops accidental edits and curious clicking. It does not stop anyone who opens the `.ssproj` in a text editor and deletes the `passwordHash` line. That is also the recovery path if you forget the password: delete the `passwordHash` line in a text editor and save the file. The next time Serial Studio loads it, the project is unlocked. Keep the JSON valid, including commas around the removed line.

If the project file itself must be protected, use operating-system file permissions, an admin-only network share or a read-only deployment. The lock is a UI gate, not a vault.

There is no command-line way to set a lock. Lock a project once in the editor, then distribute the saved file; the lock travels with it, because the hash is part of the project JSON.

## Why the Project Editor is blurred

The Project Editor body is blurred when the project is locked, or when the app is not in Project File mode (a device is connected or a playback is open). The heading and buttons tell the cases apart:

| Situation                                       | Heading                                       | Buttons |
|-------------------------------------------------|-----------------------------------------------|---------|
| Project locked, app in Project File mode        | "This project is password protected"          | **Unlock**, **Open Other Project**, **Create New Project** |
| Connected, not in Project File mode             | "Disconnect to edit the project"              | **Disconnect and Edit**, **Close** |
| Playback open, not in Project File mode         | "Close the playback to edit the project"      | **Close** |

The password prompt appears only in Project File mode. A locked project that is connected or playing back shows the Disconnect or Close message first; end the session, and the lock screen follows. The session keeps running until you end it. See [Operation Modes](Operation-Modes.md) for how the modes relate.

## What lives in the project file

A locked project carries one extra key under the JSON root:

```jsonc
{
  "title": "Production Cell A",
  "passwordHash": "pbkdf2-sha256$600000$<saltB64>$<hashB64>",
  "groups": [ /* ... */ ]
}
```

`passwordHash` has the form `pbkdf2-sha256$<iterations>$<salt>$<hash>`. Empty or absent means no lock, and there is no separate `locked` flag: a non-empty hash is the lock. New locks use 600000 iterations; a stored hash with more than 10,000,000 iterations, or a malformed one, never verifies. Bare 32-character MD5 hashes written by Serial Studio 3.2.x still load.

## See also

- [Project Editor](Project-Editor.md): the editor that the lock gates.
- [Operator Deployments](Operator-Deployments.md): pair the lock with a runtime-mode deployment for a kiosk-style operator dashboard.
- [Operation Modes](Operation-Modes.md): Project File, Quick Plot and Console Only modes.
- [Pro vs Free Features](Pro-vs-Free.md): the feature comparison.
