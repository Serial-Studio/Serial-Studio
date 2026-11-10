# Historian (Pro)

You ran a test bench for a week and now want to look at Tuesday's run again. The Historian keeps every connected session, from connect to disconnect, in one SQLite file (a single-file database most languages can read) per project. A year of test flights is one file, not 400 CSVs.

You can browse, tag, annotate, export to CSV and replay any session through the dashboard. After a run, open **Historian**, pick the session and press **Replay**.

The Historian is a Pro feature. The first Pro feature you use makes Serial Studio offer the trial with a Yes/No prompt (it needs an internet connection); the trial never starts on its own. After a license or trial ends, Serial Studio stops opening the database. The file stays a standard SQLite database that any SQLite tool can read.

## Turning recording on

1. Open or create a project. The project title names the database file.
2. Open the **Setup** pane.
3. Under **Data Export**, switch on **Historian Database** (Pro). The toggle is hidden in the free build and greyed out when the operation mode is Console Only (see [Operation Modes](Operation-Modes.md)).
4. Connect your device. Recording starts on connect, and each connection adds a new session to the project's database.
5. Disconnect. Recording stops and the session's end timestamp is written.

The session then appears in the Historian window. Serial Studio picks the path and creates the schema on first use. A background thread writes in batched transactions, so disk I/O does not block the data path.

### File location

All sessions for a given project title live in a single `.db` file, grouped by project name:

```
<Workspace>/Session Databases/<Project Title>/<Project Title>.db
```

The workspace root is the folder set as the **Workspace Folder** in Settings. Project titles are sanitized so the filename is safe on every OS. Projects with no title fall back to `Untitled`. Sessions are separated inside the file by row, not by file.

## What gets recorded

A line like `25.3,1013` from `Serial.println` becomes two datasets, Temperature and Pressure, and each frame's values are stored with their time. "Final" values are what the dashboard shows, after any [dataset transform](Dataset-Transforms.md); "pre-transform" values are the same numbers before the transform ran. Both are stored, numeric and text.

Each session also stores every raw byte that arrived on the driver exactly as received, and your [Variables](Variables.md), polled once a second with only the changed ones written. It keeps a snapshot of the project JSON as it was when recording started. That is how a session recorded with one version of the project replays faithfully later, even if the live project has changed.

Replay re-renders widgets from the stored final values. The raw byte stream is archived for inspection, CSV, or external analysis, but it is not re-parsed during replay.

## The Historian window

Open the Historian from the **Historian** button on the toolbar. Without an active Pro license or trial, the button shows the trial/activation prompt instead of the window. The window opens one project's database and lists every session recorded in that file, newest first.

In operator mode (see [Operator Deployments](Operator-Deployments.md)) the Historian opens only while Historian Database is on, and it shows only the current project's database.

Select a session, then use the toolbar buttons (**Replay**, **Export CSV**, **Export PDF**, **Restore Project**, **Delete**; **Lock** applies to the whole file) to:

- Replay it through the dashboard.
- Export it to CSV: final (post-transform) values, one column per dataset plus a timestamp column, written to the workspace.
- Generate an HTML or PDF report with a cover page, metadata, summary statistics and interactive plots. See [Session Reports](Session-Reports.md).
- Tag it with freeform labels such as `flight-test` or `anomaly`, and add notes. Tags are shared across every session in the project database, so one label can group sessions from different runs. In operator mode only the session being recorded accepts notes; completed sessions are read-only.
- Delete it, with its blocks, raw bytes, tags and table snapshots, in a single transaction.

**Restore Project** extracts the project JSON embedded in the session into a standalone `.ssproj` file and opens it in the editor. **Lock** sets a password on the database that blocks session deletion until you unlock it with the same password. There is no password recovery, so store it somewhere safe.

## Replay

To replay a Historian session, select it in the Historian window and press **Replay**. Replay (Pro) plays the session back through the dashboard as if the device were connected, and drives MQTT and the API like a live connection. CSV, MDF4 and Historian recording are suppressed while any player (CSV, MDF4 or Historian replay) is open, so replaying a session writes no new export files.

Play/Pause, previous/next frame and a progress slider work as in the [CSV player](CSV-Export-Playback.md).

When replay starts, Serial Studio saves the current operation mode and project path, and the session's embedded project is restored for the run. When replay ends, it restores the saved operation mode and project path. You can replay a session while your live project differs.

Replay is read-only. It does not modify the recorded session.

## Performance notes

Sessions are written in Write-Ahead-Logging (WAL) mode with `synchronous=NORMAL`. WAL lets the Historian and external tools read the database while a recording is running.

| Setting                          | Value |
|----------------------------------|-------|
| Block queue                      | 8,192 entries |
| Flush threshold / interval       | 1,024 queued blocks or once per second |
| Raw-byte chunks per transaction  | up to 10,000 |
| Table-snapshot queue             | 1,024 entries |
| Table-snapshot rows per transaction | up to 1,000 |

When the disk falls behind and the block queue fills, the dropped blocks are counted in `sessions.frames_dropped`.

Databases grow linearly with data rate and session length. As a floor, each stored numeric value takes 8 bytes (float64), twice that with its pre-transform twin, plus the raw bytes. No retention policy exists: delete sessions in the Historian window or with the `sessions.*` API commands.

## Choosing between CSV, MDF4, Historian and reports

| Goal                                                              | Best option               |
|-------------------------------------------------------------------|---------------------------|
| Hand a single file to a collaborator who uses Excel or pandas     | CSV export (free)         |
| Long recordings, high data rates, automotive toolchain            | [MDF4 export](MDF4.md) (Pro) |
| Archive every run of a project, then search, tag, or replay later | Historian (Pro)           |
| A shareable, printable summary with charts for a customer or lab notebook | Session report (Pro) |

Session reports are rendered on demand from the database.

## Database format (for developers)

The Historian file is a standard SQLite 3 database: no custom container, no compression, no encryption. Any SQLite client can open it directly, so recorded sessions are usable from your own software without going through Serial Studio at all. The schema is created with `PRAGMA user_version = 4` and evolves additively (new columns and tables are added, existing ones are never renamed or reordered), so queries that name their columns keep working across application upgrades.

The database runs in WAL mode (see [Performance notes](#performance-notes)), so reading it while Serial Studio is recording is safe. Open it read-only from external tools (`file:...?mode=ro` in SQLite URI syntax): a second writer makes the recorder's transactions wait on the lock or fail with `SQLITE_BUSY`.

The recording and the Historian can also be driven programmatically. The [API](API-Reference.md) exposes the `sessions.*` commands, such as `sessions.setExportEnabled`, `sessions.list` and `sessions.replay`, and the `--session-export` flag in the [Command-Line Interface](Command-Line-Interface.md) turns recording on at startup.

### Tables

| Table             | Contents |
|-------------------|----------|
| `blocks`          | Per-dataset final and pre-transform values for every published block |
| `raw_bytes`       | Every byte that arrived on the driver, as received |
| `table_snapshots` | Variables, captured on change at a 1 Hz poll; not a per-frame record |
| `sessions`        | Project title, start and end time, embedded project JSON (`project_json`), notes |
| `columns`         | Dataset title, group, units, widget type, virtual flag |

Databases recorded by older versions store per-frame rows in `readings` (one row per dataset per frame, `raw_*` and `final_*` value pairs) and `stream_blocks` (float64 sample blobs encoded like `values_blob`) instead of `blocks`. Current builds still read those tables for replay and CSV export, but no longer write them; `sessions.capture_format` records which format a session uses (`2` for the block format, `1` or NULL for the legacy per-frame format).

### Timestamps

Every `*_ns` column is a 64-bit signed integer counting nanoseconds from the session's first recorded sample, not from the Unix epoch. The wall-clock anchor is `sessions.started_at`, an ISO 8601 local timestamp such as `2026-08-20T14:03:11`; add a `timestamp_ns` value to it to place a sample in calendar time. `sessions.ended_at` uses the same format. Within `raw_bytes`, timestamps are strictly increasing: entries that arrive in the same nanosecond are stepped forward by 1 ns so ordering is unambiguous.

### Sample data: the `blocks` table

One row per dataset per published block, for frame-driven and stream-driven sources alike. Join `blocks` to `columns` on `(session_id, unique_id)` to resolve the dataset's title, group, units, and widget type.

| Column | Meaning |
|--------|---------|
| `session_id`, `unique_id`, `source_id` | Session, dataset, and stream source the row belongs to |
| `block_number` | Publication counter, increasing per session |
| `frames` | Number of samples encoded in the blobs of this row |
| `t0_ns`, `t_end_ns` | Timestamps of the first and last sample in the block |
| `dt_ns` | Sample period. Non-zero: sample *i* sits at `t0_ns + i * dt_ns`. Zero: the block is irregular and per-sample offsets live in `times` |
| `is_numeric` | 1 when every sample in the block is numeric |
| `min_value`, `max_value`, `sum_value`, `finite_count` | Precomputed statistics over the finite final values, NULL when the block has none |

The sample payloads are binary blobs, all little-endian regardless of the recording machine:

| Column | Encoding |
|--------|----------|
| `values_blob` | `frames` IEEE-754 float64 values (8 bytes each): the final, post-transform values |
| `raw_values` | Same encoding, the pre-transform values; NULL when the block carries no pre-transform twin. Frame-driven sources always write it, so without a transform it duplicates `values_blob` |
| `texts` | `frames` entries, each a uint32 byte count followed by that many UTF-8 bytes: the final text values; NULL for numeric-only datasets |
| `raw_texts` | Same encoding as `texts`, the pre-transform text; NULL unless the dataset has both |
| `times` | `frames` int64 values: per-sample nanosecond offsets from `t0_ns`; present only when `dt_ns` is 0 |

A blob whose length does not match `frames` (times 8 for the fixed-width blobs) is truncated or foreign; reject it rather than decoding past its end.

### Metadata tables

- **`sessions`**: one row per recording. It also carries `app_version`, `capture_format`, `repro_class` (the verifier's reproducibility classification), `frames_dropped` and `overflow_bytes` (link-loss counters), `raw_sha256` and `readings_sha256` (integrity fingerprints checked by the built-in verifier), and `view_state` (the dashboard layout at recording time).
- **`columns`**: the per-session dataset catalog: `unique_id`, `title`, `group_title`, `units`, `widget`, `is_virtual`, plus `source_id` and `source_title` for stream sources.
- **`raw_bytes`**: `data` is the received byte stream as a BLOB, `device_id` distinguishes devices on multi-device links.
- **`table_snapshots`**: `table_name` and `register_name` (the column name for a Variables entry) identify the variable; exactly one of `numeric_value` or `string_value` is set.
- **`tags`** and **`session_tags`**: the tag catalog (labels unique, case-insensitive) and the session-to-tag join table.
- **`project_metadata`**: key/value store. `created_at` stamps database creation; `lock_password_hash` appears when the database is locked. The lock blocks deletion inside Serial Studio only; the file is not encrypted.
- **`verifications`**: append-only log of verifier runs with timestamp, application version, verdict, and detail JSON.

### Reading a session from Python

To pull a dataset into your own tooling, decode the blobs directly. This reads the `Temperature` dataset of session 1.

```python
import sqlite3
import struct

db = sqlite3.connect("file:My Project.db?mode=ro", uri=True)

for sid, started in db.execute("SELECT session_id, started_at FROM sessions"):
    print(sid, started)

query = """
  SELECT b.t0_ns, b.dt_ns, b.frames, b.values_blob, b.times
  FROM blocks b
  JOIN columns c ON c.session_id = b.session_id AND c.unique_id = b.unique_id
  WHERE b.session_id = ? AND c.title = 'Temperature'
  ORDER BY b.t0_ns
"""
for t0, dt, frames, blob, times in db.execute(query, (1,)):
    values = struct.unpack(f"<{frames}d", blob)
    if dt:
        stamps = [t0 + i * dt for i in range(frames)]
    else:
        stamps = [t0 + off for off in struct.unpack(f"<{frames}q", times)]
```

Each iteration yields one block: `values` holds the final values and `stamps` their session-relative timestamps in nanoseconds. The same pattern with `raw_values` recovers pre-transform values, and `texts` decodes by walking uint32 length prefixes.

## See also

- [Session Reports](Session-Reports.md): export sessions as self-contained HTML or PDF reports with charts.
- [CSV Export & Playback](CSV-Export-Playback.md): live CSV export and CSV playback.
- [MDF4 Export & Playback](MDF4.md): binary logging with per-channel sample rates and channel metadata (Pro).
- [Data Flow](Data-Flow.md): where session recording sits in the overall pipeline.
- [Dataset Value Transforms](Dataset-Transforms.md): transforms contribute to the recorded final values.
- [Variables](Variables.md): recorded in `table_snapshots` alongside frames.
- [Pro vs Free Features](Pro-vs-Free.md): full list of Pro-only features.

*Formerly called Session Database. The feature, its file layout and its `sessions.*` API are unchanged; only the name shown in the UI and docs changed.*
