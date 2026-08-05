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
