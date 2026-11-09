# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""scripts/art-verify.py: one fixture per way art can slip into the public tree (spec 0116 AC1)."""

from __future__ import annotations

import hashlib
import importlib.util
import json
import struct
from pathlib import Path

import pytest

SCRIPT = Path(__file__).resolve().parent.parent / "art-verify.py"
SPEC = importlib.util.spec_from_file_location("art_verify", SCRIPT)
AV = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(AV)

MIT = "MIT"
CLEAN_SVG = b'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M0 0h1"/></svg>'


def png(px: int) -> bytes:
    """A PNG signature and IHDR, which is all the gate reads."""
    return (
        b"\x89PNG\r\n\x1a\n"
        + struct.pack(">I", 13)
        + b"IHDR"
        + struct.pack(">II", px, px)
    )


class Tree:
    """A throwaway rcc folder plus the manifest describing it."""

    def __init__(self, root: Path) -> None:
        self.root = root
        self.manifest: dict = {"files": {}, "targets": [], "standins": []}

    def add(
        self, rel: str, data: bytes, license_id: str = MIT, listed: bool = True
    ) -> None:
        path = self.root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        if listed:
            digest = hashlib.sha256(data).hexdigest()
            self.manifest["files"][rel] = {"sha256": digest, "license": license_id}

    def pair(self, rel: str, px: int, listed: bool = True) -> None:
        self.add(f"{rel}.png", png(px), listed=listed)
        self.add(f"{rel}@2x.png", png(px * 2), listed=listed)

    def errors(self) -> list[str]:
        (self.root / AV.MANIFEST_NAME).write_text(
            json.dumps(self.manifest), encoding="utf-8"
        )
        return AV.check(self.root)[0]


@pytest.fixture
def tree(tmp_path: Path) -> Tree:
    return Tree(tmp_path)


def test_clean_tree_passes(tree: Tree) -> None:
    tree.pair("icons/commands/16/save", 16)
    tree.pair("actions/Resistor", 48)
    tree.add("icons/buttons/close.svg", CLEAN_SVG)
    tree.manifest["targets"] = ["icons/commands/16/save", "actions/Resistor"]
    assert tree.errors() == []


def test_unlisted_art_fails(tree: Tree) -> None:
    tree.add("icons/commands/16/stray.svg", CLEAN_SVG, listed=False)
    assert any("not in the manifest" in error for error in tree.errors())


def test_listed_but_missing_fails(tree: Tree) -> None:
    tree.manifest["files"]["icons/commands/16/gone.svg"] = {
        "sha256": "0",
        "license": MIT,
    }
    assert any("listed in the manifest but missing" in error for error in tree.errors())


def test_changed_file_fails(tree: Tree) -> None:
    tree.add("icons/buttons/close.svg", CLEAN_SVG)
    tree.manifest["files"]["icons/buttons/close.svg"]["sha256"] = "0" * 64
    assert any("hash differs" in error for error in tree.errors())


@pytest.mark.parametrize(
    "license_id", ["LicenseRef-Icons8", "LGPL-3.0-or-later", "CC-BY-SA-3.0"]
)
def test_unsafe_license_fails(tree: Tree, license_id: str) -> None:
    tree.add("icons/buttons/close.svg", CLEAN_SVG, license_id=license_id)
    assert any("license not allowed" in error for error in tree.errors())


@pytest.mark.parametrize(
    "svg",
    [
        b'<svg xmlns="http://www.w3.org/2000/svg"><g id="surface51347"/></svg>',
        b'<svg xmlns="http://www.w3.org/2000/svg"  viewBox="0 0 48 48" width="48px" height="48px"/>',
    ],
)
def test_icons8_signature_fails_even_when_listed(tree: Tree, svg: bytes) -> None:
    tree.add("icons/buttons/close.svg", svg)
    assert any("Icons8 export signature" in error for error in tree.errors())


def test_missing_2x_partner_fails(tree: Tree) -> None:
    tree.add("icons/commands/16/save.png", png(16))
    assert any("partner" in error for error in tree.errors())


def test_wrong_pixel_size_fails(tree: Tree) -> None:
    tree.add("icons/commands/16/save.png", png(24))
    tree.add("icons/commands/16/save@2x.png", png(32))
    assert any("expected 16x16" in error for error in tree.errors())


def test_unresolved_target_fails(tree: Tree) -> None:
    tree.manifest["targets"] = ["actions/Resistor"]
    assert any("does not resolve" in error for error in tree.errors())


def test_missing_manifest_fails(tmp_path: Path) -> None:
    assert AV.check(tmp_path)[0] == [f"missing {AV.MANIFEST_NAME}"]


def test_standins_are_reported_separately(tree: Tree) -> None:
    tree.manifest["standins"] = ["oscilloscope"]
    assert tree.errors() == []
    assert AV.check(tree.root)[1] == ["oscilloscope"]
