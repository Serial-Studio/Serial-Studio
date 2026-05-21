import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def read_text(path: str) -> str:
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
