---
spec: 0059-frame-annotation-layer
phase: tasks
status: approved     # gate auto-approved, overnight run 2026-08-17
updated: 2026-08-17
---

# Tasks 0059 — Frame annotation layer

- [x] T1 `Console::AnnotationModel` + `AnnotationFilter` (`Console/Annotations.{h,cpp}`)
- [x] T2 `Console::AnnotationDecoder` (JS, watchdog, carry-over, failure latch)
- [x] T3 `Console::Handler` ownership, feed, QML properties; CMake registration
- [x] T4 `ConsoleAnnotations.qml` (track / table + CSV / payload / decoder) + Terminal toggle
- [x] T5 `tst_console_annotations` + CMake registration
- [x] T6 docs (`dashboard.md` tools paragraph)
- [ ] T7 Lua decoders (same model API)
- [ ] T8 Problem Center checker for decoder failures
- [ ] T9 bundled example decoders (delimited frame, length-prefixed + CRC)
