# SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
# SPDX-License-Identifier: GPL-3.0-or-later
"""Tests for the two spec-0077 census modes of code-verify.py: the per-edge singleton census
(AC3) and the bus census (AC4)."""

from __future__ import annotations

import importlib.util
import sys
from pathlib import Path

import pytest

SCRIPTS = Path(__file__).resolve().parents[1]
REPO = SCRIPTS.parent


def _load():
    sys.path.insert(0, str(SCRIPTS))
    spec = importlib.util.spec_from_file_location(
        "code_verify_census", SCRIPTS / "code-verify.py"
    )
    module = importlib.util.module_from_spec(spec)
    sys.modules["code_verify_census"] = module
    spec.loader.exec_module(module)
    return module


CV = _load()

MESSAGES = """
namespace Core::Bus {
/**
 * @brief One entry of the catalog below.
 */
struct CatalogEntry final {
  QString id;
};

/**
 * @brief The catalog.
 */
struct CatalogChanged final {
  QVector<CatalogEntry> entries;
};

struct LinkChanged final {
  bool connected;
};

struct Orphan final {
  int reserved;
};
}  // namespace Core::Bus
"""


def _tree(tmp_path: Path, sources: dict[str, str]) -> Path:
    (tmp_path / "core" / "Core" / "Bus").mkdir(parents=True)
    (tmp_path / "core" / "Core" / "Bus" / "Messages.h").write_text(
        MESSAGES, encoding="utf-8"
    )
    for rel, text in sources.items():
        path = tmp_path / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
    (tmp_path / "app" / "src").mkdir(parents=True, exist_ok=True)
    return tmp_path


def test_bus_topics_separate_value_types_from_topics():
    topics, value_types = CV._bus_topics(MESSAGES)
    assert topics == ["CatalogChanged", "LinkChanged", "Orphan"]
    assert value_types == {"CatalogEntry"}


def test_bus_census_counts_publishers_and_subscribers(tmp_path):
    root = _tree(
        tmp_path,
        {
            "core/Pipeline/A.cpp": "m_bus.publishState<Core::Bus::CatalogChanged>(x);\n"
            "m_bus.publish<Core::Bus::LinkChanged>(true);\n",
            "core/Ui/B.cpp": "bus.subscribe<CatalogChanged>(this, fn);\n"
            "const auto link = bus.latest<Core::Bus::LinkChanged>();\n",
        },
    )
    census = CV._collect_bus_census(root)
    assert census["topics"]["CatalogChanged"] == {"publishers": 1, "subscribers": 1}
    assert census["topics"]["LinkChanged"] == {"publishers": 1, "subscribers": 1}
    assert any("Orphan has no publisher" in e for e in census["errors"])
    assert any("Orphan has no subscriber" in e for e in census["errors"])


def test_bus_census_flags_an_unknown_topic_and_a_hotpath_token(tmp_path):
    root = _tree(
        tmp_path,
        {
            "core/Pipeline/DataModel/FrameBuilder.cpp": "m_bus.publish<Core::Bus::LinkChanged>(true);\n",
            "core/Ui/C.cpp": "bus.subscribe<Core::Bus::Ghost>(this, fn);\n",
        },
    )
    errors = CV._collect_bus_census(root)["errors"]
    assert any("hotpath TU" in e for e in errors)
    assert any("Ghost> names no topic" in e for e in errors)


def test_the_repository_bus_census_has_no_defects():
    census = CV._collect_bus_census(REPO)
    assert census["errors"] == [], census["errors"]
    assert census["topics"], "Messages.h declares no topics"


def test_class_index_resolves_a_reach_to_its_library(tmp_path):
    root = _tree(
        tmp_path,
        {
            "core/Pipeline/DataModel/ProjectModel.h": "namespace DataModel {\nclass ProjectModel {};\n}\n",
            "core/Storage/CSV/Export.h": "namespace CSV {\nclass Export {};\n}\n",
            "core/Ui/Console/Export.h": "namespace Console {\nclass Export {};\n}\n",
        },
    )
    index = CV._class_layer_index(root)
    assert CV._resolve_reach_layer("DataModel::ProjectModel", "Ui", index) == "Pipeline"
    assert CV._resolve_reach_layer("CSV::Export", "Api", index) == "Storage"
    assert CV._resolve_reach_layer("Console::Export", "Api", index) == "Ui"
    assert CV._resolve_reach_layer("Export", "Storage", index) == "Storage"
    assert CV._resolve_reach_layer("QCoreApplication", "Ui", index) == "Qt"
    assert CV._resolve_reach_layer("Nobody", "Ui", index) == "Unknown"


def test_the_repository_has_no_cross_library_reach():
    rules = CV._SEMANTIC_RULES
    if rules is None or not getattr(rules, "HAS_TREE_SITTER", False):
        pytest.skip("tree-sitter is needed to classify the reaches")
    census = CV._collect_singleton_census(REPO)
    assert census["cross_library_total"] == 0, census["cross_library"][:20]


def _clock_tree(tmp_path: Path, counts: dict[str, int]) -> Path:
    for rel, count in counts.items():
        path = tmp_path / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(
            "".join(
                "auto t%d = std::chrono::steady_clock::now();\n" % i
                for i in range(count)
            ),
            encoding="utf-8",
        )
    return tmp_path


def _clock_baseline(monkeypatch, tmp_path: Path, root: Path) -> None:
    monkeypatch.setattr(CV, "_CLOCK_CENSUS_BASELINE", tmp_path / "clock-census.json")
    assert CV._run_clock_census(root, check=False, accept=True) == 0


def test_clock_census_counts_sinks_and_skips_replay(tmp_path):
    root = _clock_tree(
        tmp_path,
        {
            "core/Storage/CSV/Export.cpp": 2,
            "core/Storage/Sessions/Player.cpp": 5,
            "core/Storage/Sessions/ReplayClock.cpp": 5,
            "core/Ui/Other.cpp": 5,
        },
    )
    census = CV._collect_clock_census(root)
    assert census["per_file"] == {"core/Storage/CSV/Export.cpp": 2}
    assert census["total"] == 2


def test_clock_census_growth_fails_and_names_the_grower(tmp_path, monkeypatch, capsys):
    root = _clock_tree(tmp_path, {"core/Storage/CSV/Export.cpp": 1})
    _clock_baseline(monkeypatch, tmp_path, root)
    _clock_tree(tmp_path, {"core/Storage/CSV/Export.cpp": 2})
    assert CV._run_clock_census(root, check=True, accept=False) == 1
    err = capsys.readouterr().err
    assert "Export.cpp: 1 -> 2" in err
    assert "monotonicFrameNs" in err


def test_clock_census_shrink_passes_with_a_rebaseline_hint(
    tmp_path, monkeypatch, capsys
):
    root = _clock_tree(tmp_path, {"core/Storage/CSV/Export.cpp": 2})
    _clock_baseline(monkeypatch, tmp_path, root)
    _clock_tree(tmp_path, {"core/Storage/CSV/Export.cpp": 1})
    assert CV._run_clock_census(root, check=True, accept=False) == 0
    assert "shrank" in capsys.readouterr().out


def test_clock_census_accept_reseeds_and_missing_baseline_exits_two(
    tmp_path, monkeypatch
):
    root = _clock_tree(tmp_path, {"core/Storage/CSV/Export.cpp": 1})
    monkeypatch.setattr(CV, "_CLOCK_CENSUS_BASELINE", tmp_path / "clock-census.json")
    assert CV._run_clock_census(root, check=True, accept=False) == 2
    assert CV._run_clock_census(root, check=False, accept=True) == 0
    assert CV._run_clock_census(root, check=True, accept=False) == 0
