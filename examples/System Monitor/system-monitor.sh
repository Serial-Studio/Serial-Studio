#!/usr/bin/env sh
#
# system-monitor.sh
#
# Launch wrapper for the System Monitor example.
# Serial Studio (Process driver) can point to this script directly so users
# do not need to type the Python command manually.
#
# Usage (from terminal, optional):
#   chmod +x system-monitor.sh
#   ./system-monitor.sh [--interval SECONDS]
#
# Copyright (C) 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Prefer python3; fall back to python if python3 is not found.
if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    echo "ERROR: Python not found. Install Python 3 and try again." >&2
    exit 1
fi

# Ensure psutil is available; install it quietly if missing.
if ! "$PYTHON" -c "import psutil" >/dev/null 2>&1; then
    echo "Installing psutil..." >&2
    "$PYTHON" -m pip install --quiet psutil >&2
fi

exec "$PYTHON" "$SCRIPT_DIR/system-monitor.py" "$@"
