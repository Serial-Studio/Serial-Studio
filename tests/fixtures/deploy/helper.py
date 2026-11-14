"""
Helper program for the package-resources fixture (spec 0123).

The fixture project's control script starts this file from the package session's resources
folder. It records where it ran and what the script passed, then waits to be stopped, so the
integration test can read the record while the helper is still a managed process.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import os
import sys
import time
from pathlib import Path

MAX_RUN_SECONDS = 600


def main() -> int:
    here = Path(__file__).resolve().parent
    record = {
        "cwd": os.path.realpath(os.getcwd()),
        "folder": str(here),
        "directory": sys.argv[1] if len(sys.argv) > 1 else "",
        "packageDirectory": sys.argv[2] if len(sys.argv) > 2 else "",
    }
    (here / "helper-ran.json").write_text(json.dumps(record), encoding="utf-8")

    deadline = time.monotonic() + MAX_RUN_SECONDS
    while time.monotonic() < deadline:
        time.sleep(0.5)

    return 0


if __name__ == "__main__":
    sys.exit(main())
