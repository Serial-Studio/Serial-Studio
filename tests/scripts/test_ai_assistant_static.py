import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


_SPEC_0070_SPLITS = {
    "app/src/AI/ToolDispatcher.cpp": "app/src/AI/Tools",
    "app/src/AI/Conversation.cpp": "app/src/AI/Conversation",
    "app/src/Sessions/DatabaseManager.cpp": "app/src/Sessions/DatabaseManager",
    "app/src/Sessions/Player.cpp": "app/src/Sessions/Player",
    "app/src/IO/Drivers/BluetoothLE.cpp": "app/src/IO/Drivers/BluetoothLE",
    "app/src/IO/Drivers/USB.cpp": "app/src/IO/Drivers/USB",
    "app/src/IO/Drivers/Audio.cpp": "app/src/IO/Drivers/Audio",
    "app/src/IO/Drivers/OpcUa.cpp": "app/src/IO/Drivers/OpcUa",
    "app/src/IO/Drivers/Modbus.cpp": "app/src/IO/Drivers/Modbus",
    "app/src/IO/ConnectionManager.cpp": "app/src/IO/ConnectionManager",
    "app/src/UI/Widgets/Waterfall.cpp": "app/src/UI/Widgets/Waterfall",
    "app/src/UI/Widgets/Terminal.cpp": "app/src/UI/Widgets/Terminal",
    "app/src/UI/Taskbar.cpp": "app/src/UI/Taskbar",
    "app/src/UI/WindowManager.cpp": "app/src/UI/WindowManager",
    "app/src/UI/Dashboard.cpp": "app/src/UI/Dashboard",
    "app/src/UI/Widgets/PainterContext.cpp": "app/src/UI/Widgets/Painter",
    "app/src/API/Server.cpp": "app/src/API/Server",
    "app/src/CSV/Player.cpp": "app/src/CSV/Player",
    "app/src/DataModel/FrameBuilder.cpp": "app/src/DataModel/FrameBuilder",
    "app/src/MQTT/Publisher.cpp": "app/src/MQTT",
    "app/src/Misc/ExtensionManager.cpp": "app/src/Misc/Extensions",
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
    prompt = read_text("app/src/AI/ContextBuilder.cpp")

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
    conversation = read_text("app/src/AI/Conversation.cpp")

    for name in ("project.search", "project.group.get", "meta.search"):
        assert f'QStringLiteral("{name}")' in conversation


def test_meta_search_is_fully_wired():
    conversation = read_text("app/src/AI/Conversation.cpp")
    dispatcher = read_text("app/src/AI/ToolDispatcher.cpp")

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

    handler = read_text("app/src/API/Handlers/DashboardHandler.cpp")
    docs = read_text("doc/help/API-Reference.md")
    for name in (
        "project.dashboard.setWidgetTitle",
        "project.dashboard.getWidgetTitles",
        "project.dashboard.setWidgetFreezeTitle",
    ):
        assert f'QStringLiteral("{name}")' in handler
        assert name in docs


def test_incomplete_result_notices_are_distinct():
    conversation = read_text("app/src/AI/Conversation.cpp")

    assert "TOO LARGE" in conversation
    assert "not the transcript-aging stub" in conversation
    assert "SUCCEEDED when it ran" in conversation
    assert "Not a size limit" in conversation
