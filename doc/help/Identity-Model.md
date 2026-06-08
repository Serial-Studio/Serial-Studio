# Dataset Identity Model

A dataset in Serial Studio carries three integer identifiers: `index`, `datasetId`, and `uniqueId`. They look interchangeable from the outside but mean very different things, and using the wrong one is the most common reason API calls and transform scripts mysteriously stop working. This page explains each of them in one place, with a copy-pastable rule of thumb at the end.

## The three IDs

A dataset lives inside a group, which lives inside a source. Each level of that hierarchy contributes one number.

### `sourceId`

The index of the source the dataset reads from. `0` for the default (and only) source in single-source projects. With multi-source projects (Pro) sources are numbered `0`, `1`, `2`… in the order they were added.

### `groupId`

The **positional** index of the group inside the project: `0` for the first group, `1` for the second, and so on. It is *not* a stable identity, because adding, deleting, or reordering groups renumbers it. For a group identifier that survives reorders, use the group's own `uniqueId` field. Workspace IDs live in a separate reserved range that starts at `1000` (Overview = `1000`, All Data = `1001`, per-group workspaces from `1002`, user workspaces from `5000`), so a workspace ID never collides with a positional `groupId`.

### `datasetId`

The dataset's slot inside its group. A group with three datasets will have `datasetId = 0, 1, 2`. If you reorder, insert, or delete datasets, `datasetId` is reassigned to match the new array index.

This is the ID you pass to **mutating** API calls:

```jsonc
{
  "method": "project.dataset.update",
  "params": { "groupId": 5, "datasetId": 0, "options": { "title": "Pressure" } }
}
```

### `index`

A separate field unrelated to identity. `index` is the **frame offset**, that is, the position of the dataset's value inside the parsed frame. With a CSV like `25.3,1013.2,42` and three datasets, the first dataset has `index = 1`, the second `index = 2`, the third `index = 3`. (Index `0` is reserved.) Multiple datasets can share the same `index` if you want the same raw value styled in different ways. `index` has no relation to `datasetId` and the two often diverge.

### `uniqueId`

A single integer that identifies a dataset *globally* across the whole project. It's **persisted in the project file** (each dataset stores its `uniqueId` directly) and **assigned once** from a project-level counter when the dataset is created, duplicated, or imported. Reordering or renaming a dataset never changes its `uniqueId`.

Treat the integer as opaque: it's allocated, not derived. Two assumptions you'd otherwise make are no longer safe:

- You **cannot** recover `(sourceId, groupId, datasetId)` from a `uniqueId` by arithmetic. The legacy formula `sourceId * 1'000'000 + groupId * 10'000 + datasetId` is only used as a one-shot back-fill when loading a project file from before this scheme existed; once the project is saved again, the values are persisted and no longer follow the formula.
- You **cannot** guess what `uniqueId` a new dataset will receive. It's just the next value from the project's counter (`nextUniqueId`).

`uniqueId` is what transform scripts and Data Tables use, because they need a single key that's stable across sources. It's what you read from the live data API and what the system data table (`__datasets__`) uses for its `raw:<uid>` and `final:<uid>` registers. The Group struct has its own `uniqueId` field with the same semantics, used inside workspace widget refs.

## Where each ID is used

| ID         | Used by                                                                                          |
|------------|--------------------------------------------------------------------------------------------------|
| `sourceId` | Source-scoped API calls (`project.source.update`, `project.source.setFrameParserCode`, etc.)     |
| `groupId`  | Group-scoped API calls (`project.group.update`, dataset CRUD addressing)                         |
| `datasetId`| Dataset CRUD addressing (`project.dataset.update`, `project.dataset.setOptions`, `project.dataset.delete`) |
| `index`    | Frame-parser scripts when assigning values from the parsed array (`group.datasets[0].index = 1`) |
| `uniqueId` | Live-data API (`dashboard.getData`, `dashboard.tailFrames`), transform scripts (`datasetGetRaw(uid)`, `datasetGetFinal(uid)`), Data Tables (`raw:<uid>`, `final:<uid>`) |

## Rule of thumb

The mental shortcut that covers 95% of cases:

> **Mutate by `(groupId, datasetId)`. Read by `uniqueId`. Position by `index`.**

In other words:

- Editing or deleting a dataset? Address it by the group it lives in plus its slot inside that group.
- Reading a live value, looking up a Data Tables register, calling `datasetGetRaw` / `datasetGetFinal` from a transform? Use `uniqueId`.
- Wiring a frame-parser script to pick the right column out of the parsed frame? That's `index`.

## Lifecycle gotchas

- **`datasetId` shifts when you rearrange.** Inserting a dataset at slot 0 renumbers everything that came after. If you cache `datasetId` in your script and then edit the project, refresh it from the API after every mutation.
- **`uniqueId` does NOT follow `datasetId`.** It's persisted with the dataset and stays the same across reorders, renames, retypes, and moves between sources. The one exception is duplicate / copy-paste: a duplicated dataset receives a fresh `uniqueId` from the counter so the original and the copy stay distinguishable.
- **`groupId` shifts when you rearrange.** Adding a new group at the top renumbers everything that came after. For a stable group identifier, use `Group.uniqueId` (returned by `project.group.list` under `uniqueId`); that's what workspace widget refs persist.
- **`index` is yours to set.** Frame parsers control `index` directly. The Project Editor assigns sequential defaults but you can override them.

## Quick reference

```text
Dataset
├─ sourceId   → which source produced the frame
├─ groupId    → positional slot of the parent group (shifts on group reorder)
├─ datasetId  → slot within the group, used for mutations (shifts on dataset reorder)
├─ index      → which column of the parsed frame to read
└─ uniqueId   → opaque, persisted, stable identity (used for reads, refs, transforms)
```

If something is ever ambiguous, default to looking it up fresh from the project snapshot rather than caching it across an edit.

## See also

- [Project Editor](Project-Editor.md): where datasets, groups, and sources are created and where their IDs come from.
- [Frame Parser Scripting](JavaScript-API.md): the parser that picks values out of the frame and assigns them to datasets by `index`.
- [Dataset Value Transforms](Dataset-Transforms.md): per-dataset scripts that read and write registers by `uniqueId`.
- [Data Tables](Data-Tables.md): the system table (`__datasets__`) keyed by `raw:<uniqueId>` and `final:<uniqueId>`.
- [API Reference](API-Reference.md): the JSON-RPC surface that exposes mutating CRUD by `(groupId, datasetId)` and live reads by `uniqueId`.
