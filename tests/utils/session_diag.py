"""
Diagnostics for session-recording failures.

A missing session .db says nothing about *why* it is missing: the exporter may
never have been enabled, the device may never have delivered a frame, or the
write may have failed. CI has produced whole-platform runs where every session
test failed with "DB was never created" and the app log stayed silent, so the
assertions carry this snapshot with them.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""


def session_diagnostics(api_client) -> dict:
    """Best-effort snapshot of the state a session recording depends on."""
    snapshot = {}
    probes = {
        "sessions": ("sessions.getStatus", None),
        "dashboard": ("dashboard.getStatus", None),
        "connection": ("io.getStatus", None),
    }

    for key, (command, params) in probes.items():
        try:
            snapshot[key] = api_client.command(command, params or {})
        except (
            Exception
        ) as exc:  # noqa: BLE001 - diagnostics must never mask the real failure
            snapshot[key] = f"<{type(exc).__name__}: {exc}>"

    return snapshot
