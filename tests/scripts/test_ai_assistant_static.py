import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


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


def test_assistant_safe_rails_are_registered():
    safety = json.loads(read_text("app/rcc/ai/command_safety.json"))
    safe = set(safety["safe"])

    assert "assistant.snapshot" in safe
    assert "assistant.dataset.resolve" in safe
    assert "assistant.workspace.resolve" in safe
    assert "assistant.workspace.plan" in safe
    assert "assistant.script.dryRun" in safe
    assert "project.frameParser.dryCompile" in safe
    assert "project.dryRun.endToEnd" in safe


def test_system_prompt_prefers_assistant_rails():
    prompt = read_text("core/Ui/AI/ContextBuilder.cpp")

    assert "assistant.workspace.addTile" in prompt
    assert "assistant.script.apply" in prompt
    assert "assistant.project.bulkApply" in prompt
    assert "Do NOT call project.save after edits" in prompt


def test_skills_reference_high_level_ai_workflows():
    dashboard = read_text("app/rcc/ai/skills/dashboard_layout.md")
    parsers = read_text("app/rcc/ai/skills/frame_parsers.md")
    transforms = read_text("app/rcc/ai/skills/transforms.md")
    painter = read_text("app/rcc/ai/skills/painter.md")
    discovery = read_text("app/rcc/ai/skills/tool_discovery.md")

    assert "assistant.workspace.addTile" in dashboard
    assert 'assistant.script.dryRun{kind:"frame_parser"' in parsers
    assert 'assistant.script.apply{kind:"transform"' in transforms
    assert 'assistant.script.apply{kind:"painter"' in painter
    assert "assistant.project.bulkApply" in discovery


def test_search_index_contains_current_assistant_terms():
    index = json.loads(read_text("app/rcc/ai/search_index.json"))
    body = json.dumps(index)

    assert "assistant.workspace.addTile" in body
    assert "assistant.script.apply" in body
    assert "assistant.project.bulkApply" in body


def test_discovery_commands_are_safe_tier():
    safety = json.loads(read_text("app/rcc/ai/command_safety.json"))
    safe = set(safety["safe"])

    assert "project.search" in safe
    assert "project.group.get" in safe
    tiers = ["confirm", "blocked", "deviceGated", "alwaysConfirm"]
    for tier in tiers:
        assert "project.search" not in set(safety[tier])
        assert "project.group.get" not in set(safety[tier])


def test_discovery_results_are_never_elided():
    conversation = read_text("core/Ui/AI/Conversation.cpp")

    for name in ("project.search", "project.group.get", "meta.search"):
        assert f'QStringLiteral("{name}")' in conversation


def test_meta_search_is_fully_wired():
    conversation = read_text("core/Ui/AI/Conversation.cpp")
    dispatcher = read_text("core/Ui/AI/ToolDispatcher.cpp")

    assert conversation.count('QStringLiteral("meta.search")') >= 2
    assert 'QStringLiteral("meta.search")' in dispatcher
    assert "searchCommands" in dispatcher


def test_widget_display_commands_are_tiered_and_documented():
    safety = json.loads(read_text("app/rcc/ai/command_safety.json"))
    safe = set(safety["safe"])
    confirm = set(safety["confirm"])

    assert "project.dashboard.getWidgetTitles" in safe
    assert "project.dashboard.setWidgetTitle" in confirm
    assert "project.dashboard.setWidgetFreezeTitle" in confirm

    handler = read_text("core/Api/API/Handlers/DashboardHandler.cpp")
    docs = read_text("doc/help/API-Reference.md")
    for name in (
        "project.dashboard.setWidgetTitle",
        "project.dashboard.getWidgetTitles",
        "project.dashboard.setWidgetFreezeTitle",
    ):
        assert f'QStringLiteral("{name}")' in handler
        assert name in docs


def test_incomplete_result_notices_are_distinct():
    conversation = read_text("core/Ui/AI/Conversation.cpp")

    assert "TOO LARGE" in conversation
    assert "not the transcript-aging stub" in conversation
    assert "SUCCEEDED when it ran" in conversation
    assert "Not a size limit" in conversation
