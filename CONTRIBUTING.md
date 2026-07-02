# Contributing to Serial Studio

Thanks for taking the time to contribute. Bug reports, code, examples, and documentation
fixes are all welcome.

## Reporting bugs

Open an issue with the bug report template. The report is most useful when it includes:

- Operating system and version
- Serial Studio version and edition (GPL build, Trial, or Pro)
- Connection type (UART, TCP/UDP, BLE, MQTT, Modbus, CAN Bus, USB, HID, Audio, Process)
- Steps to reproduce, what you expected, and what happened instead
- The project file (`.ssproj`) and console output when relevant

## Suggesting features

Open an issue with the feature request template, or start a thread in
[Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions) if the idea is
still taking shape. For larger changes, talk it through in an issue before writing code so
the approach is agreed on first.

## Contributing code

- Work on GPL-licensed code and leave the commercial modules alone. Every source file
  carries an SPDX header that states its license.
- Follow the project's clang-format config (LLVM base style, 100 columns, 2-space indent).
- Run `scripts/code-verify.py --check` before opening a pull request; it enforces the
  structural and style rules that CI checks. `scripts/sanitize-commit.py` runs the full
  pipeline (formatting, verification, documentation checks) in one step.
- Style details live in [doc/claude/code-style.md](doc/claude/code-style.md); repo-wide
  rules live in [CLAUDE.md](CLAUDE.md).
- Add Doxygen comments for new public APIs. Avoid inline end-of-line comments.

### Submitting changes

1. Fork the repository and create a feature branch (`git checkout -b feature/my-change`).
2. Commit with descriptive messages.
3. Push to your fork and open a pull request using the template.
4. Make sure CI passes, including the hotpath benchmark gate for changes near the data
   pipeline.

## Contributing documentation

Help pages live in `doc/help/` and example write-ups in `examples/`. Run
`scripts/documentation-verify.py` before submitting; it lints the Markdown that ships with
the app.

## Tests

The Python test suite lives in `tests/` (see [tests/README.md](tests/README.md)).
Integration tests drive a running instance over the TCP API and need
**Settings > Miscellaneous > Enable API Server** turned on. The JS parser unit tests in
`tests/scripts/` only need Node.js.

```bash
pip install -r tests/requirements.txt
pytest tests/scripts/ -v          # parser unit tests, no app required
pytest tests/integration/ -v      # needs a running instance with the API server enabled
```

## Questions

Use [Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions) for questions
and the [help center](https://serial-studio.com/help) for usage documentation.
