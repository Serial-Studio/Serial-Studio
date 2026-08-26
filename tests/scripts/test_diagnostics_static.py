"""
Static consistency checks for the connection diagnostics (spec 0035).

Runs without Qt and without a live app: every assertion reads a repository
file, so the suite catches registration drift (a command registered in C++ but
missing from the assistant safety manifest, a new API scope with no
description, or a checker id the API handler documents but the runner never
registers) at lint time instead of at runtime.
"""

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

COMMANDS = ("diagnostics.run", "diagnostics.status")
OTHER_TIERS = ("confirm", "blocked", "deviceGated", "alwaysConfirm")
BUSES = ("serial", "bluetooth", "network", "broker", "audio")

UI_COMMAND = "app.connectionDiagnostics"
UI_BINDING = "cmdAppConnectionDiagnostics"
ICON_TIERS = (16, 24, 32, 48)

BINDINGS_FILES = (
    "app/qml/Commands/AppCommandBindings.qml",
    "app/qml/Commands/ProjectEditorCommandBindings.qml",
)

BLOCKING_CALLS = ("waitFor", "QEventLoop", "processEvents", "msleep", "sleep(")

DIAGNOSTICS_SOURCES = (
    "app/src/Misc/ConnectionDiagnostics.cpp",
    "app/src/Misc/ConnectionDiagnostics.h",
    "app/src/Misc/Diagnostics/AudioChecks.cpp",
    "app/src/Misc/Diagnostics/BluetoothChecks.cpp",
    "app/src/Misc/Diagnostics/DeviceAccess.cpp",
    "app/src/Misc/Diagnostics/DiagnosticsShared.h",
    "app/src/Misc/Diagnostics/NetworkChecks.cpp",
    "app/src/Misc/Diagnostics/SerialChecks.cpp",
    "app/src/API/Handlers/DiagnosticsHandler.cpp",
)


_SPEC_0070_SPLITS = {
    "app/src/AI/ToolDispatcher.cpp": "app/src/AI/ToolDispatcher",
    "app/src/API/Handlers/ProjectHandlerEntities.cpp": "app/src/API/Handlers/Entities",
    "app/src/Sessions/DatabaseManager.cpp": "app/src/Sessions/DatabaseManager",
    "app/src/IO/Drivers/BluetoothLE.cpp": "app/src/IO/Drivers/BluetoothLE",
    "app/src/UI/Widgets/Waterfall.cpp": "app/src/UI/Widgets/Waterfall",
    "app/src/UI/Taskbar.cpp": "app/src/UI/Taskbar",
    "app/src/UI/Widgets/PainterContext.cpp": "app/src/UI/Widgets/Painter",
    "app/src/API/Server.cpp": "app/src/API/Server",
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


def test_diagnostics_commands_are_safe_tier():
    safe = set(safety_manifest()["safe"])

    for name in COMMANDS:
        assert name in safe, f"{name} missing from the 'safe' tier"


def test_diagnostics_commands_are_in_exactly_one_tier():
    safety = safety_manifest()

    for name in COMMANDS:
        for tier in OTHER_TIERS:
            assert name not in set(safety[tier]), f"{name} also listed in '{tier}'"


def test_diagnostics_commands_are_registered_in_cpp():
    handler = read_text("app/src/API/Handlers/DiagnosticsHandler.cpp")

    for name in COMMANDS:
        assert f'QStringLiteral("{name}")' in handler


def test_diagnostics_handler_is_registered_in_the_gpl_block():
    source = read_text("app/src/API/CommandHandler.cpp")

    assert "API/Handlers/DiagnosticsHandler.h" in source
    assert "Handlers::DiagnosticsHandler::registerCommands();" in source

    call = source.index("Handlers::DiagnosticsHandler::registerCommands();")
    commercial = source.index(
        "#ifdef BUILD_COMMERCIAL", source.index("initializeHandlers")
    )
    assert call < commercial, "the handler must register outside the commercial block"


def test_diagnostics_handler_carries_no_commercial_guard():
    for path in (
        "app/src/API/Handlers/DiagnosticsHandler.h",
        "app/src/API/Handlers/DiagnosticsHandler.cpp",
    ):
        assert "BUILD_COMMERCIAL" not in read_text(path), f"{path} must stay GPL-clean"


def test_diagnostics_scope_has_a_description():
    dispatcher = read_text("app/src/AI/ToolDispatcher.cpp")
    marker = 'QStringLiteral("diagnostics")'

    assert marker in dispatcher
    assert dispatcher.index("scopeDescriptions") < dispatcher.index(marker)


def test_bus_slugs_match_the_declared_bus_count():
    shared = read_text("app/src/Misc/Diagnostics/DiagnosticsShared.h")
    slugs = re.findall(r'return QStringLiteral\("(\w+)"\);', shared)

    for bus in BUSES:
        assert bus in slugs, f"bus slug '{bus}' is not returned by busSlug()"

    assert "inline constexpr int kBusCount = %d;" % len(BUSES) in shared


def test_checker_ids_are_derived_from_the_bus_slugs():
    shared = read_text("app/src/Misc/Diagnostics/DiagnosticsShared.h")
    runner = read_text("app/src/Misc/ConnectionDiagnostics.cpp")

    assert 'QStringLiteral("diagnostics.") + busSlug(bus)' in shared
    assert re.search(
        r"center\.registerChecker\(\s*Diagnostics::checkerId\(bus\),", runner
    ), "runner must register checkers under Diagnostics::checkerId(bus)"
    assert "Misc::ProblemCenter::OnDemand," in runner


def test_handler_documents_every_checker_id():
    handler = read_text("app/src/API/Handlers/DiagnosticsHandler.cpp")

    for bus in BUSES:
        assert (
            f"diagnostics.{bus}" in handler
        ), f"the handler must name diagnostics.{bus}"


def test_diagnostics_never_block_the_event_loop():
    for path in DIAGNOSTICS_SOURCES:
        source = read_text(path)
        for call in BLOCKING_CALLS:
            assert call not in source, f"{path} introduces a blocking call: {call}"


def test_remedy_commands_are_not_inside_translated_strings():
    serial = read_text("app/src/Misc/Diagnostics/SerialChecks.cpp")

    assert 'QStringLiteral("sudo usermod -aG %1 %2")' in serial
    assert "sudo usermod" not in serial.replace(
        'QStringLiteral("sudo usermod -aG %1 %2")', ""
    ).replace('QStringLiteral("sudo usermod -aG %1 $USER")', "")


def test_diagnostics_command_manifest_entry():
    manifest = json.loads(read_text("app/rcc/commands/app.json"))
    entries = [c for c in manifest["commands"] if c["id"] == UI_COMMAND]

    assert len(entries) == 1, f"{UI_COMMAND} must be declared exactly once"
    entry = entries[0]
    assert entry["kind"] == "action"
    assert entry["category"] == "tools"
    assert entry["icon"] == "commands/diagnostics"
    assert sorted(entry["contexts"]) == ["app", "dashboard", "editor"]


def test_diagnostics_command_icon_resolves_in_the_icon_tree():
    manifest = json.loads(read_text("app/rcc/commands/app.json"))
    entry = next(c for c in manifest["commands"] if c["id"] == UI_COMMAND)
    category, name = entry["icon"].split("/", 1)

    tree = ROOT / "app" / "rcc" / "icons" / category
    tiers = [t for t in ICON_TIERS if (tree / str(t) / f"{name}.svg").is_file()]
    assert tiers, f"icon '{entry['icon']}' resolves in no tier of the icon tree"


def test_diagnostics_command_is_bound_in_both_contexts():
    for path in BINDINGS_FILES:
        source = read_text(path)
        assert f'"{UI_COMMAND}": root.{UI_BINDING}' in source, f"{path}: no map entry"
        assert f"property QtObject {UI_BINDING}" in source, f"{path}: no binding"
        assert "app.runConnectionDiagnostics()" in source, f"{path}: no run call"


def test_diagnostics_entry_points_are_hosted_by_the_ui():
    main_qml = read_text("app/qml/main.qml")
    setup_qml = read_text("app/qml/MainWindow/Panes/Setup.qml")
    dialog_qml = read_text("app/qml/Dialogs/ProblemCenter.qml")

    assert "function runConnectionDiagnostics()" in main_qml
    assert "Cpp_Misc_ConnectionDiagnostics.runAll()" in main_qml
    assert "Cpp_Misc_ConnectionDiagnostics.runAll()" in dialog_qml
    assert "Cpp_Misc_ConnectionDiagnostics.running" in dialog_qml
    assert (
        "Cpp_Misc_ConnectionDiagnostics" not in setup_qml
    ), "diagnostics moved to the Problem Center dialog; the Setup pane hosts no entry point"


def test_diagnostics_context_property_is_registered():
    module_manager = read_text("app/src/Misc/ModuleManager.cpp")

    wiring = "Misc::ConnectionDiagnostics::instance().setupExternalConnections();"

    assert '"Cpp_Misc_ConnectionDiagnostics"' in module_manager
    assert wiring in module_manager


def test_diagnostics_commands_are_not_destructive():
    registry = read_text("app/src/API/CommandRegistry.cpp")
    start = registry.index("destructiveCommandSet")
    window = registry[start : start + 20000]

    for name in COMMANDS:
        assert name not in window, f"{name} must not be a destructive command"
