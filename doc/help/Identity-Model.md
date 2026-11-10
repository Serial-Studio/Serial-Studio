# Dataset Identity Model

Your board prints `25.3,1013.2,42`. Serial Studio splits that line into three values and shows each one as a dataset: the first has `index = 1`, the second `index = 2`, the third `index = 3`. That is all most projects ever need, and the Project Editor sets it for you.

You only need the rest of this page if you write transform scripts or call the API. There, mixing up `datasetId` and `uniqueId` reads the wrong dataset, or nothing. A transform script that reads a dataset by its stable id looks like this:

```js
var t = datasetGetRaw(4137);
```

## The identifiers

A dataset lives inside a group, which lives inside a source. `sourceId`, `groupId` and `datasetId` locate a dataset by position; `index`, `uniqueId` and `alias` do other jobs.

### `index`

`index` is the only identifier you set for ordinary dashboards. It is the 1-based position of the dataset's value in the parsed frame: with `25.3,1013.2,42`, the three datasets have `index = 1`, `2` and `3`. Counting starts at 1 because `0` is not a valid frame position; a dataset whose `index` is 0, negative or larger than the number of fields in the parsed frame is skipped and keeps its last value.

Select the dataset in the Project Editor and edit its index there. Several datasets can share one `index` to style the same raw value in different ways. `index` has no relation to `datasetId`, and the two often differ.

A short frame never clears or errors a dataset; it just stops updating. A widget that stays frozen while the others move has an `index` past the end of the frame.

### `sourceId`

`sourceId` is the zero-based index of the source a dataset reads from. A single-source project has only source `0`. Multi-source projects (Pro) number sources `0`, `1`, `2`… in the order they were added.

### `groupId`

`groupId` is the positional index of the group inside the project: `0` for the first group, `1` for the second, and so on. Adding, deleting, or reordering groups renumbers it, so it is not a stable identity. Groups also have a `uniqueId` that survives reorders (see [`uniqueId`](#uniqueid)). `project.group.get` accepts either one.

### `datasetId`

`datasetId` is the dataset's slot inside its group. A group with three datasets has `datasetId = 0, 1, 2`. Delete the first dataset and the Pressure you cached as `datasetId` 1 now answers to 0. Refresh it from the API after every change.

You pass `datasetId`, together with `groupId`, to the API calls that change the project (update, delete). Connect to the API Server (off by default; enable it under **Preferences > General > API & Plugins**, port 7777) and send the JSON followed by a newline; the [API Reference](API-Reference.md) shows the transport:

```json
{
  "method": "project.dataset.update",
  "params": { "groupId": 5, "datasetId": 0, "title": "Pressure" }
}
```

`project.dataset.update` takes `groupId`, `datasetId` and the fields to change directly in `params`, not inside an `options` object.

### `uniqueId`

`uniqueId` is allocated from a per-project counter when a dataset is created, duplicated, or imported, and saved in the project file. Reordering, renaming, or moving a dataset between sources never changes it. Treat it as opaque: you cannot compute `(sourceId, groupId, datasetId)` from it, and you cannot predict the next value. Projects saved by older versions get their ids filled in once on load.

A duplicated or copy-pasted dataset receives a fresh `uniqueId`, so the original and the copy stay distinguishable.

Transform scripts and [Variables](Variables.md) tables use `uniqueId` because they need one key that is stable across sources. The `dashboard.getData` response lists each dataset with its `uniqueId`. The system table (`__datasets__`) uses it for its `raw:<uniqueId>` variable (the value as parsed) and `final:<uniqueId>` variable (the value after the dataset's [transform](Dataset-Transforms.md)).

### `alias`

An `alias` is an optional name you set in the **Script Alias** field of the Project Editor's dataset form. It is empty by default and must be unique across the project. `datasetGetRaw(4137)` tells you nothing six months later; `datasetGetRaw("ATAM1-CH1")` does. A string argument is always an alias and a number is always a `uniqueId`, so the two never coerce into each other. Transform and frame-parser scripts accept it wherever they accept a `uniqueId`: `datasetGetRaw("ATAM1-CH1")` and `datasetGetFinal("ATAM1-CH1")` look up by alias, and the `__datasets__` table exposes `raw:<alias>` and `final:<alias>` variables next to the numeric ones. `uniqueId` stays the persisted identity that stored cross-references point at.

A duplicate alias is cleared 800 ms after you stop typing, and the editor reports it. An all-digit alias such as `123` gets a warning, because a quoted `"123"` is still looked up as an alias, not as `uniqueId` 123.

The Project Editor tree context menu has a **Seed Aliases from Titles** action that fills every empty alias from its dataset title, deduplicated, and leaves existing aliases untouched.

## Workspace refs: "groupId" there is a group `uniqueId`

Workspace widget refs store the group's `uniqueId` under the JSON key `groupId`. When you read workspace data (`project.workspace.get`) and see a "groupId" far larger than the number of groups, it is a group `uniqueId`, not a positional index. Those values come from the same counter as dataset `uniqueId`s, so a `groupId` larger than the group count is normal. Passing one to a positional-id command (`project.group.update`, `project.dataset.update`, `project.dataset.delete`) hits the wrong group or fails; resolve it with `project.group.get` and `uniqueId`.

Workspace IDs are a separate range: Overview is `1000`, All Data is `1001`, auto per-group workspaces start at `1002`, auto per-folder workspaces start at `3000`, and user workspaces start at `5000`.

## Where each ID is used

| ID          | Used by                                                                       |
|-------------|-------------------------------------------------------------------------------|
| `sourceId`  | `project.source.update`, `project.source.setFrameParserCode` (multi-source: Pro) |
| `groupId`   | `project.group.update`, `project.dataset.update`, `project.dataset.delete`    |
| `datasetId` | `project.dataset.update`, `project.dataset.setOptions`, `project.dataset.delete` |
| `index`     | Maps each slot of the parser's returned array to a dataset                    |
| `uniqueId`  | `dashboard.getData`, `dashboard.tailFrames`, `datasetGetRaw`, `datasetGetFinal`, `raw:<uniqueId>`, `final:<uniqueId>` |
| `alias`     | `datasetGetRaw("name")`, `datasetGetFinal("name")`, `raw:<alias>`, `final:<alias>` |

## Rule of thumb

> **Mutate by `(groupId, datasetId)`. Read by `uniqueId` or alias. Pick the frame value with `index`.**

## See also

- [Project Editor](Project-Editor.md): where datasets, groups, and sources are created and where their IDs come from.
- [Frame Parser Scripting](Frame-Parser.md): the parser that picks values out of the frame and assigns them to datasets by `index`.
- [Dataset Value Transforms](Dataset-Transforms.md): per-dataset scripts that read and write variables by `uniqueId`.
- [API Reference](API-Reference.md): the JSON-RPC surface that exposes mutating CRUD by `(groupId, datasetId)` and live reads by `uniqueId`.
