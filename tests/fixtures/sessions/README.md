# Session verification fixtures (spec 0044)

The verification suite (`tests/integration/test_session_verification.py`) does not check in
binary session databases. Every archive it needs -- including the pre-0044 "legacy" one for
AC5 -- is derived at test time from a session the running app just recorded:

- Tamper fixtures (AC2/AC3) are copies of a fresh recording with one `readings` row or the
  stored `project_json` edited through `sqlite3`.
- The legacy fixture (AC5) is a copy with the spec-0044 columns dropped
  (`ALTER TABLE sessions DROP COLUMN ...`, requires SQLite >= 3.35), the `verifications`
  table removed, and `PRAGMA user_version` reset to 0 -- byte-for-byte what a pre-0044 build
  would have written for the same capture.

Deriving from a live recording keeps the fixture data genuinely reproducible on the build
under test; a checked-in binary would go stale the first time the capture format moves. If a
frozen cross-version fixture ever becomes necessary (e.g. verifying against an archive from
an actual older release), record it with that release, place the `.db` here, and document
its provenance (app version, project, capture steps) next to it.

## Frozen legacy archives (spec 0055)

Spec 0055 unified `readings` and `stream_blocks` into one `blocks` table (schema
`user_version` 3). R8 promises a pre-0055 archive still opens, replays and verifies, and that
promise cannot be checked against a fixture the build under test just produced -- the build
under test no longer writes the old layout at all. These two are therefore the frozen
cross-version fixtures the section above anticipates.

| file | `user_version` | recorded by | contents |
|------|----------------|-------------|----------|
| `legacy_v1_readings.db` | 1 | Serial Studio 4.0.3, project "Regress Identity" | 1 session, 20 `readings`, 20 `raw_bytes`, 1 column; no control script, no transforms |
| `legacy_v2_stream.db` | 2 | Serial Studio 4.0.3, QuickPlot | 1 session, 15 `stream_blocks` (7200 samples), no `readings` |

Provenance: both were copied unmodified from a developer's own
`~/Documents/Serial Studio/Session Databases/` on 2026-08-16, captured by the shipped 4.0.3
build (`sessions.app_version = '4.0.3'`, `capture_format = 1`). The only edit was to
`legacy_v2_stream.db`, which had four sessions: the three others were deleted whole and the
file `VACUUM`ed. Deleting whole sessions is safe for the fingerprints because `raw_sha256` /
`readings_sha256` / `stream_sha256` are stored per session row -- trimming *rows* inside a
kept session would invalidate them, which is why neither file's retained session was reduced.

Expected verdicts on the pre-0055 build, for `tst_sessions_legacy_archive` and the maintainer's
`--verify-session` run to compare against:

- `legacy_v1_readings.db` -> `reproduced` (integrity `verified` for both raw bytes and readings,
  20 recorded rows vs 20 regenerated, 0 mismatches).
- `legacy_v2_stream.db` -> `error` / `diff-failed`. **This is the pre-existing 4.0.3 behaviour,
  not a spec-0055 regression**: the session is pure-stream, so the readings diff has nothing to
  compare. It is kept as a *replay and decode* fixture, not a verification one.
