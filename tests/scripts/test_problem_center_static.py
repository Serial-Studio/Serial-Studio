"""
Static consistency checks for the problem center (spec 0033).

Runs without Qt and without a live app: every assertion reads a repository
file, so the suite catches registration drift (a command registered in C++ but
missing from the assistant safety manifest, or a new API scope with no
description) at lint time instead of at runtime.
"""

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

COMMANDS = ("problems.list", "problems.run", "problems.listCheckers")
OTHER_TIERS = ("confirm", "blocked", "deviceGated", "alwaysConfirm")


_SPEC_0070_SPLITS = {
    "core/Ui/AI/ToolDispatcher.cpp": "core/Ui/AI/Tools",
    "core/Ui/AI/Conversation.cpp": "core/Ui/AI/Conversation",
    "core/Storage/Sessions/DatabaseManager.cpp": "core/Storage/Sessions/DatabaseManager",
    "core/Storage/Sessions/Player.cpp": "core/Storage/Sessions/Player",
    "core/Devices/IO/Drivers/BluetoothLE.cpp": "core/Devices/IO/Drivers/BluetoothLE",
    "core/Devices/IO/Drivers/USB.cpp": "core/Devices/IO/Drivers/USB",
    "core/Devices/IO/Drivers/Audio.cpp": "core/Devices/IO/Drivers/Audio",
    "core/Devices/IO/Drivers/OpcUa.cpp": "core/Devices/IO/Drivers/OpcUa",
    "core/Devices/IO/Drivers/Modbus.cpp": "core/Devices/IO/Drivers/Modbus",
    "core/Devices/IO/ConnectionManager.cpp": "core/Devices/IO/ConnectionManager",
    "core/Ui/UI/Widgets/Waterfall.cpp": "core/Ui/UI/Widgets/Waterfall",
    "core/Ui/UI/Widgets/Terminal.cpp": "core/Ui/UI/Widgets/Terminal",
    "core/Ui/UI/Taskbar.cpp": "core/Ui/UI/Taskbar",
    "core/Ui/UI/WindowManager.cpp": "core/Ui/UI/WindowManager",
    "core/Ui/UI/Dashboard.cpp": "core/Ui/UI/Dashboard",
    "core/Ui/UI/Widgets/PainterContext.cpp": "core/Ui/UI/Widgets/Painter",
    "core/Api/API/Server.cpp": "core/Api/API/Server",
    "core/Storage/CSV/Player.cpp": "core/Storage/CSV/Player",
    "core/Pipeline/DataModel/FrameBuilder.cpp": "core/Pipeline/DataModel/FrameBuilder",
    "core/Devices/MQTT/Publisher.cpp": "core/Devices/MQTT",
    "core/Ui/Misc/ExtensionManager.cpp": "core/Ui/Misc/Extensions",
}


def _component_text(path: str) -> str:
    """A component's source: the named file plus the concern TUs extracted from it.

    Spec 0070 moved cohesive concerns out of the god files into sibling TUs. These
    checks assert on a component's behaviour, not on which of its files a given
    function ended up in, so they read the whole component.
    """
    p = ROOT / path
    parts = [p.read_text(encoding="utf-8")] if p.exists() else []
    d = ROOT / _SPEC_0070_SPLITS.get(path, "")
    if path in _SPEC_0070_SPLITS and d.is_dir():
        for f in sorted(d.iterdir()):
            if f.suffix in (".cpp", ".h"):
                parts.append(f.read_text(encoding="utf-8"))
    return "\n".join(parts)


def read_text(path: str) -> str:
    if path.endswith((".cpp", ".h")):
        return _component_text(path)
    return (ROOT / path).read_text(encoding="utf-8")


def safety_manifest() -> dict:
    return json.loads(read_text("app/rcc/ai/command_safety.json"))


def test_problem_commands_are_safe_tier():
    safety = safety_manifest()
    safe = set(safety["safe"])

    for name in COMMANDS:
        assert name in safe, f"{name} missing from the 'safe' tier"


def test_problem_commands_are_in_exactly_one_tier():
    safety = safety_manifest()

    for name in COMMANDS:
        for tier in OTHER_TIERS:
            assert name not in set(safety[tier]), f"{name} also listed in '{tier}'"


def test_problem_commands_are_registered_in_cpp():
    handler = read_text("core/Api/API/Handlers/ProblemsHandler.cpp")

    for name in COMMANDS:
        assert f'QStringLiteral("{name}")' in handler


def test_problems_handler_is_registered_in_the_gpl_block():
    source = read_text("core/Api/API/CommandHandler.cpp")

    assert "API/Handlers/ProblemsHandler.h" in source
    assert "Handlers::ProblemsHandler::registerCommands();" in source

    call = source.index("Handlers::ProblemsHandler::registerCommands();")
    commercial = source.index(
        "#ifdef BUILD_COMMERCIAL", source.index("initializeHandlers")
    )
    assert call < commercial, "the handler must register outside the commercial block"


def test_problems_handler_carries_no_commercial_guard():
    for path in (
        "core/Api/API/Handlers/ProblemsHandler.h",
        "core/Api/API/Handlers/ProblemsHandler.cpp",
    ):
        assert "BUILD_COMMERCIAL" not in read_text(path), f"{path} must stay GPL-clean"


def test_problems_scope_has_a_description():
    dispatcher = read_text("core/Ui/AI/ToolDispatcher.cpp")
    marker = 'QStringLiteral("problems")'

    assert marker in dispatcher
    assert dispatcher.index("scopeDescriptions") < dispatcher.index(marker)


def test_problem_center_command_manifest_entry():
    manifest = json.loads(read_text("app/rcc/commands/app.json"))
    entries = [c for c in manifest["commands"] if c["id"] == "app.problems"]

    assert len(entries) == 1, "app.problems must be declared exactly once"
    entry = entries[0]
    assert entry["kind"] == "action"
    assert entry["category"] == "tools"
    assert entry["icon"] == "notifications/warning"
    assert sorted(entry["contexts"]) == ["app", "dashboard", "editor"]


def test_problem_center_command_is_bound_in_both_contexts():
    for path in (
        "app/qml/Commands/AppCommandBindings.qml",
        "app/qml/Commands/ProjectEditorCommandBindings.qml",
    ):
        source = read_text(path)
        assert (
            '"app.problems": root.cmdAppProblems' in source
        ), f"{path} misses the map entry"
        assert (
            "readonly property QtObject cmdAppProblems" in source
        ), f"{path} misses the binding"
        assert "app.showProblemCenter()" in source, f"{path} misses the panel call"


def test_problem_center_panel_is_hosted_by_main_qml():
    source = read_text("app/qml/main.qml")

    assert "Dialogs/ProblemCenter.qml" in source
    assert "function showProblemCenter()" in source
    assert "function onJumpRequested(kind, uniqueId)" in source


def test_problem_commands_are_not_destructive():
    registry = read_text("core/Api/API/CommandRegistry.cpp")
    start = registry.index("destructiveCommandSet")
    window = registry[start : start + 20000]

    for name in COMMANDS:
        assert name not in window, f"{name} must not be a destructive command"
