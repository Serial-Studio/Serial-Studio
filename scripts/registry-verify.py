#!/usr/bin/env python3
"""Structural lint for the spec-0028 icon registry tree and its qrc block.

Checks the migrated icon tree (`app/rcc/icons/`) against the registry contract:
every file sits at `<category>/<tier>/<name>.svg` with a known category and a
tier in {16, 24, 32, 48}; no byte-identical duplicates exist outside the exempt
`buttons/` set (spec AC2); rcc.qrc and the disk tree agree in both directions;
and every compat alias points at an existing file. It also lints QML icon
requests: a `Cpp_Misc_IconRegistry.icon(...)` whose px would resolve to a larger
tier than the object's render size (`iconSize`/`icon.width`/`sourceSize`) loads an
oversized SVG and is flagged (spec 0028: request px must match render size). Spec
0038 adds the widget-extension gate: the bundled packages validate against
`widget-manifest.json`, the reserved builtin-id list agrees across the schema, the
C++ catalog and the widget-string mappers, the packages stay in sync with rcc.qrc,
and `WidgetExtensions::hostContextNames()` still mirrors ModuleManager. The
alias report counts live
source references to each old path -- task T21 drops the alias block only when
`--require-no-alias-refs` passes. Exit code 0 = clean, 1 = violations.

Usage:
    python3 scripts/registry-verify.py [--alias-report] [--require-no-alias-refs]
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RCC = ROOT / "app" / "rcc"
ICONS = RCC / "icons"
QRC = RCC / "rcc.qrc"

EXEMPT_FOLDERS = {"buttons"}
CATEGORIES = {
    "widgets",
    "window",
    "editor",
    "devices",
    "panes",
    "console",
    "database",
    "code",
    "licensing",
    "notifications",
    "commands",
    "system",
}
TIERS = {"16", "24", "32", "48"}

SOURCE_ROOTS = (ROOT / "app" / "qml", ROOT / "app" / "src")
SOURCE_EXTS = {".qml", ".cpp", ".h"}

NAME_RE = re.compile(r"^[a-z0-9][a-z0-9-]*\.svg$")


def fail(errors: list[str], message: str) -> None:
    errors.append(message)


def check_tree(errors: list[str]) -> None:
    hashes: dict[str, list[str]] = defaultdict(list)
    for path in sorted(ICONS.rglob("*")):
        if path.is_dir():
            continue
        rel = path.relative_to(ICONS).as_posix()
        top = rel.split("/", 1)[0]
        if top in EXEMPT_FOLDERS or path.name == ".DS_Store":
            continue
        parts = rel.split("/")
        if path.suffix != ".svg":
            fail(errors, f"non-SVG file in icon tree: icons/{rel}")
            continue
        hashes[hashlib.md5(path.read_bytes()).hexdigest()].append(rel)
        if len(parts) != 3:
            fail(errors, f"not <category>/<tier>/<name>.svg: icons/{rel}")
            continue
        category, tier, name = parts
        if category not in CATEGORIES:
            fail(errors, f"unknown category '{category}': icons/{rel}")
        if tier not in TIERS:
            fail(errors, f"tier not in {sorted(TIERS)}: icons/{rel}")
        if not NAME_RE.match(name):
            fail(errors, f"name not kebab-case: icons/{rel}")
    for digest, paths in sorted(hashes.items()):
        if len(paths) > 1:
            fail(errors, f"byte-identical duplicates: {', '.join(paths)}")


def qrc_entries() -> tuple[set[str], dict[str, str]]:
    text = QRC.read_text(encoding="utf-8")
    real = set(re.findall(r"<file>(icons/[^<]+)</file>", text))
    aliases = dict(
        re.findall(r'<file alias="(icons/[^"]+)">(icons/[^<]+)</file>', text)
    )
    return real, aliases


def check_qrc(errors: list[str]) -> dict[str, str]:
    real, aliases = qrc_entries()
    disk = {f"icons/{p.relative_to(ICONS).as_posix()}" for p in ICONS.rglob("*.svg")}
    for entry in sorted(real - disk):
        fail(errors, f"qrc entry has no file on disk: {entry}")
    for entry in sorted(disk - real):
        fail(errors, f"file on disk not registered in qrc: {entry}")
    for alias, target in sorted(aliases.items()):
        if not (RCC / target).exists():
            fail(errors, f"alias points at missing file: {alias} -> {target}")
        if alias in disk:
            fail(errors, f"alias shadows a real file: {alias}")
    return aliases


MANIFESTS = ["app.json", "dashboard.json", "projecteditor.json", "database.json"]
LAYOUTS = [
    "main-toolbar.json",
    "project-toolbar.json",
    "start-menu.json",
    "database-toolbar.json",
]
KNOWN_CONTEXTS = {"app", "dashboard", "editor"}
KNOWN_KINDS = {"action", "toggle"}
KNOWN_WINDOWS = {"main", "editor"}
KNOWN_STANDARD_KEYS = {
    "Open",
    "New",
    "Save",
    "Quit",
    "Back",
    "Forward",
    "Close",
    "Preferences",
    "Undo",
    "Redo",
}
COMMERCIAL_SYMBOLS = ("Cpp_Licensing_", "Cpp_Sessions_", "Cpp_MQTT_")
KNOWN_CATEGORIES = {
    "file",
    "mode",
    "connection",
    "view",
    "export",
    "console",
    "project",
    "license",
    "tools",
    "help",
}


def icon_ref_resolves(ref: str) -> bool:
    category, _, name = ref.partition("/")
    if not category or not name:
        return False
    return any(
        (ICONS / category / str(t) / f"{name}.svg").exists() for t in (16, 24, 32, 48)
    )


def check_manifests(errors: list[str]) -> set[str]:
    ids: set[str] = set()
    shortcuts: dict[tuple[str, str], str] = {}
    for name in MANIFESTS:
        path = RCC / "commands" / name
        if not path.exists():
            fail(errors, f"missing command manifest: {name}")
            continue
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as error:
            fail(errors, f"invalid JSON in {name}: {error}")
            continue
        for command in data.get("commands", []):
            cid = command.get("id", "")
            if not cid or cid in ids:
                fail(errors, f"{name}: missing or duplicate id '{cid}'")
                continue
            ids.add(cid)
            if command.get("kind", "action") not in KNOWN_KINDS:
                fail(errors, f"{name}: {cid} has unknown kind")
            cat = command.get("category")
            if cat is not None and cat not in KNOWN_CATEGORIES:
                fail(errors, f"{name}: {cid} has unknown category '{cat}'")
            for ctx in command.get("contexts", []):
                if ctx not in KNOWN_CONTEXTS:
                    fail(errors, f"{name}: {cid} has unknown context '{ctx}'")
            for key in ("icon", "iconChecked"):
                ref = command.get(key)
                if ref and not icon_ref_resolves(ref):
                    fail(errors, f"{name}: {cid} {key} '{ref}' does not resolve")
            shortcut = command.get("shortcut", "")
            if shortcut.startswith("StandardKey."):
                if shortcut.split(".", 1)[1] not in KNOWN_STANDARD_KEYS:
                    fail(
                        errors,
                        f"{name}: {cid} unknown {shortcut} (extend the C++ table)",
                    )
            for window in command.get("shortcutWindows", []):
                if window not in KNOWN_WINDOWS:
                    fail(errors, f"{name}: {cid} unknown shortcut window '{window}'")
                if shortcut:
                    key = (window, shortcut)
                    if key in shortcuts:
                        fail(
                            errors,
                            f"duplicate shortcut {shortcut} in window {window}: "
                            f"{shortcuts[key]} and {cid}",
                        )
                    shortcuts[key] = cid
    return ids


def check_layout_nodes(
    errors: list[str], name: str, nodes: list, ids: set[str]
) -> None:
    for node in nodes:
        if node.get("type") == "command" and node.get("id") not in ids:
            fail(
                errors, f"{name}: layout references unknown command '{node.get('id')}'"
            )
        for ref in node.get("collapsedCommands", []):
            if ref not in ids:
                fail(errors, f"{name}: collapsedCommands references unknown '{ref}'")
        ref = node.get("icon")
        if ref and not icon_ref_resolves(ref):
            fail(errors, f"{name}: layout icon '{ref}' does not resolve")
        check_layout_nodes(errors, name, node.get("items", []), ids)


def check_layouts(errors: list[str], ids: set[str]) -> None:
    for name in LAYOUTS:
        path = RCC / "commands" / "layouts" / name
        if not path.exists():
            fail(errors, f"missing layout manifest: {name}")
            continue
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as error:
            fail(errors, f"invalid JSON in {name}: {error}")
            continue
        for key in ("sections", "items", "pinnedEnd"):
            check_layout_nodes(errors, name, data.get(key, []), ids)


def check_binding_guards(errors: list[str]) -> None:
    bindings_dir = ROOT / "app" / "qml" / "Commands"
    if not bindings_dir.is_dir():
        return
    for path in sorted(bindings_dir.glob("*.qml")):
        lines = path.read_text(encoding="utf-8").splitlines()
        for number, line in enumerate(lines, 1):
            previous = lines[number - 2] if number >= 2 else ""
            guarded = "Cpp_CommercialBuild" in line or "Cpp_CommercialBuild" in previous
            for symbol in COMMERCIAL_SYMBOLS:
                if symbol in line and not guarded:
                    fail(
                        errors,
                        f"{path.name}:{number}: {symbol} reference without a "
                        f"Cpp_CommercialBuild guard on the same or previous line",
                    )


QML_ROOT = ROOT / "app" / "qml"
RENDER_RES = (
    re.compile(r"\biconSize\s*:\s*(\d+)\b"),
    re.compile(r"\bicon\.width\s*:\s*(\d+)\b"),
    re.compile(r"\bicon\.height\s*:\s*(\d+)\b"),
    re.compile(r"\bsourceSize\.width\s*:\s*(\d+)\b"),
    re.compile(r"\bsourceSize\.height\s*:\s*(\d+)\b"),
)
REQUEST_RE = re.compile(r"IconRegistry\.icon(?:ById)?\([^)]*\b(\d+)\s*\)")
ELEMENT_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_.]*)\s*\{")
SORTED_TIERS = (16, 24, 32, 48)

# Components whose icon slot renders at a fixed size declared in the component file, invisible
# to the same-block scan (e.g. Widgets/Pane.qml draws `icon` at sourceSize 16).
COMPONENT_SLOT_PX = {
    "Pane": 16,
}


def served_tier(px: int) -> int:
    for tier in SORTED_TIERS:
        if px <= tier:
            return tier
    return SORTED_TIERS[-1]


def strip_qml_noise(line: str, in_comment: bool) -> tuple[str, bool]:
    out: list[str] = []
    index = 0
    length = len(line)
    while index < length:
        two = line[index : index + 2]
        if in_comment:
            if two == "*/":
                in_comment = False
                index += 2
                continue
            index += 1
            continue
        if two == "//":
            break
        if two == "/*":
            in_comment = True
            index += 2
            continue
        char = line[index]
        if char in ('"', "'"):
            index += 1
            while index < length and line[index] != char:
                index += 1
            index += 1
            continue
        out.append(char)
        index += 1
    return "".join(out), in_comment


def check_icon_render_sizes(errors: list[str]) -> None:
    for path in sorted(QML_ROOT.rglob("*.qml")):
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        stack: list[dict] = []
        in_comment = False
        rel = path.relative_to(ROOT).as_posix()
        for number, line in enumerate(lines, 1):
            if stack:
                for regex in RENDER_RES:
                    match = regex.search(line)
                    if match and stack[-1]["render"] is None:
                        stack[-1]["render"] = int(match.group(1))
                for match in REQUEST_RE.finditer(line):
                    stack[-1]["requests"].append((int(match.group(1)), number))
            clean, in_comment = strip_qml_noise(line, in_comment)
            names = [m.group(1).rsplit(".", 1)[-1] for m in ELEMENT_RE.finditer(clean)]
            for char in clean:
                if char == "{":
                    name = names.pop(0) if names else None
                    stack.append({"render": None, "requests": [], "name": name})
                elif char == "}" and stack:
                    block = stack.pop()
                    render = block["render"]
                    if render is None:
                        render = COMPONENT_SLOT_PX.get(block["name"])
                    if render is None:
                        continue
                    for px, where in block["requests"]:
                        tier = served_tier(px)
                        if tier > served_tier(render):
                            fail(
                                errors,
                                f"{rel}:{where}: icon requested at {px}px loads the "
                                f"{tier}px tier but is rendered at {render}px (spec "
                                f"0028: request px must match render size)",
                            )


# ---------------------------------------------------------------------------------------------------
# Spec 0037: generated API surfaces
# ---------------------------------------------------------------------------------------------------

SKILLS = RCC / "ai" / "skills"
DATASET_MANIFEST = RCC / "properties" / "dataset.json"
SERIALSTUDIO_H = ROOT / "app" / "src" / "SerialStudio.h"
REGISTRY_GENERATOR = ROOT / "scripts" / "generate-property-registry.py"

# Prose that claims the API rejects a spelling it actually accepts. The dataset manifest
# declares plotMin/widgetMin/xAxis/datasetSourceId as aliases, so a corpus sentence pairing
# one of them with a rejection phrase teaches the assistant a fact the code contradicts.
REJECTION_PHRASES = (
    "accepts only",
    "accepts ONLY",
    "accept only",
    "silently drops",
    "silently drop",
    "silently ignores",
    "silently ignored",
    "silently ignore",
    "writes nothing",
    "does not accept",
    "are NOT the keys",
    "is NOT the key",
)

# Lines whose rejection phrasing is about genuinely unknown fields, not a declared alias.
CORPUS_WHITELIST = (
    # fooBar in the warnings example is a deliberately invalid field name.
    "fooBar",
)


def dataset_manifest() -> dict:
    return json.loads(DATASET_MANIFEST.read_text(encoding="utf-8"))


def dataset_name_space(manifest: dict) -> tuple[set[str], set[str]]:
    """Return (every accepted dataset field spelling, the declared alias spellings)."""
    names: set[str] = set()
    aliases: set[str] = set()
    for prop in manifest["properties"]:
        names.add(prop["jsonKey"])
        api = prop.get("api", {})
        if not api.get("expose"):
            continue
        names.add(api["name"])
        for alias in api.get("aliases", []):
            names.add(alias)
            aliases.add(alias)

    for sub in manifest.get("subEntities", []):
        names.add(sub["apiName"])
        for arg in sub.get("legacy", {}).get("args", []):
            names.add(arg["name"])

    for runtime in manifest.get("runtimeFields", []):
        if runtime.get("jsonKey"):
            names.add(runtime["jsonKey"])

    return names, aliases


def dataset_option_bits() -> dict[str, int]:
    """Parse the DatasetOption bitflags out of SerialStudio.h -- the corpus tables' ground truth."""
    text = SERIALSTUDIO_H.read_text(encoding="utf-8")
    block = re.search(r"enum DatasetOption\s*\{(.*?)\}", text, re.DOTALL)
    if not block:
        return {}

    bits: dict[str, int] = {}
    for name, value in re.findall(r"Dataset(\w+)\s*=\s*(0b[01]+)", block.group(1)):
        if name != "Generic":
            bits[name.lower()] = int(value, 2)
    return bits


def table_rows(text: str) -> list[tuple[list[str], list[str]]]:
    """Return every markdown table row paired with its header cells."""
    rows: list[tuple[list[str], list[str]]] = []
    header: list[str] = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped.startswith("|"):
            header = []
            continue
        cells = [c.strip() for c in stripped.strip("|").split("|")]
        if not header:
            header = cells
            continue
        if all(set(c) <= set("-: ") for c in cells):
            continue
        rows.append((header, cells))
    return rows


def check_option_bit_tables(errors: list[str]) -> None:
    """Every widget-option bit stated in the corpus must match the DatasetOption enum."""
    bits = dataset_option_bits()
    if not bits:
        fail(errors, "could not parse DatasetOption from app/src/SerialStudio.h")
        return

    for path in sorted(SKILLS.glob("*.md")):
        text = path.read_text(encoding="utf-8")
        for header, cells in table_rows(text):
            index = next(
                (i for i, c in enumerate(header) if "bit" in c.lower()),
                None,
            )
            if index is None or index >= len(cells):
                continue
            slug = re.search(r"`\"?([a-z0-9-]+)\"?`", cells[0])
            value = re.search(r"`?(\d+)`?", cells[index])
            if not slug or not value or slug.group(1) not in bits:
                continue
            if bits[slug.group(1)] != int(value.group(1)):
                fail(
                    errors,
                    f"{path.name}: option '{slug.group(1)}' is stated as bit "
                    f"{value.group(1)}, SerialStudio.h declares {bits[slug.group(1)]}",
                )

        missing = sorted(b for b in bits if f'`"{b}"`' not in text)
        if len(missing) < len(bits) and missing:
            fail(
                errors,
                f"{path.name}: widget-option table omits {', '.join(missing)}; "
                "state every DatasetOption bit or cross-reference the one table that does",
            )


def check_enum_domains(errors: list[str], manifest: dict) -> None:
    """An integer enum value stated in the corpus must exist in the declared domain."""
    domains: dict[str, set[int]] = {}
    for prop in manifest["properties"]:
        api = prop.get("api", {})
        source = manifest["optionSources"].get(prop.get("options", ""), {})
        if not api.get("expose") or source.get("kind") != "parallelValues":
            continue
        domains[api["name"]] = {int(e["value"]) for e in source["entries"]}

    if not domains:
        return

    anchor = re.compile("|".join(re.escape(n) for n in sorted(domains)))
    for path in sorted(SKILLS.glob("*.md")):
        text = path.read_text(encoding="utf-8")
        anchors = [(m.start(), m.group(0)) for m in anchor.finditer(text)]
        if not anchors:
            continue

        for match in re.finditer(r"\b(\d+)=", text):
            prior = [a for a in anchors if a[0] < match.start()]
            if not prior:
                continue
            start, name = prior[-1]
            if match.start() - start > 400:
                continue
            if int(match.group(1)) not in domains[name]:
                fail(
                    errors,
                    f"{path.name}: {name} value {match.group(1)} is not in the "
                    f"declared domain {sorted(domains[name])}",
                )


def paragraphs(text: str) -> list[tuple[int, str]]:
    """Return (first line number, text) per blank-line block; each table row stands alone.

    A markdown table is one visual block but many independent claims -- scanning it as a
    single paragraph pairs one row's field name with another row's rejection wording.
    """
    out: list[tuple[int, str]] = []
    start = 1
    buffer: list[str] = []

    def flush() -> None:
        nonlocal buffer
        if buffer:
            out.append((start, "\n".join(buffer)))
            buffer = []

    for number, line in enumerate(text.splitlines(), 1):
        if line.lstrip().startswith("|"):
            flush()
            out.append((number, line))
            continue
        if line.strip():
            if not buffer:
                start = number
            buffer.append(line)
            continue
        flush()

    flush()
    return out


def check_field_name_refs(errors: list[str], manifest: dict) -> None:
    """Field names stated in the corpus's write-form mapping tables must be declared."""
    names, aliases = dataset_name_space(manifest)

    for path in sorted(SKILLS.glob("*.md")):
        text = path.read_text(encoding="utf-8")
        for header, cells in table_rows(text):
            joined = " ".join(header).lower()
            if "dataset.update" not in joined and "write-form" not in joined:
                continue
            for cell in cells:
                for token in re.findall(r"`([A-Za-z][A-Za-z0-9]*)`", cell):
                    if token not in names:
                        fail(
                            errors,
                            f"{path.name}: field '{token}' is stated in a dataset "
                            "write-form table but is not declared in "
                            "app/rcc/properties/dataset.json",
                        )

        for number, block in paragraphs(text):
            if any(token in block for token in CORPUS_WHITELIST):
                continue
            named = sorted(a for a in aliases if a in block)
            if not named:
                continue
            for phrase in REJECTION_PHRASES:
                if phrase in block:
                    fail(
                        errors,
                        f"{path.name}:{number}: says the API rejects "
                        f"{', '.join(named)}, but the manifest declares "
                        f"{'it' if len(named) == 1 else 'them'} as accepted "
                        f"alias(es) of project.dataset.update",
                    )
                    break


def check_corpus_field_refs(errors: list[str]) -> None:
    """Reference lint over the bundled assistant corpus; never rewrites a file."""
    if not SKILLS.is_dir() or not DATASET_MANIFEST.exists():
        fail(
            errors, "corpus lint skipped: missing skills directory or dataset manifest"
        )
        return

    manifest = dataset_manifest()
    check_option_bit_tables(errors)
    check_enum_domains(errors, manifest)
    check_field_name_refs(errors, manifest)


def check_api_snapshot(errors: list[str]) -> None:
    """Fold the buildless api-schema.json projection check into this run.

    The projection warns locally (only a build can refresh the snapshot) and fails in CI,
    where the generator sees CI in the environment.
    """
    if not REGISTRY_GENERATOR.is_file():
        fail(
            errors,
            "api snapshot check skipped: generate-property-registry.py is missing",
        )
        return

    result = subprocess.run(
        [sys.executable, str(REGISTRY_GENERATOR), "--check-snapshot"],
        capture_output=True,
        text=True,
    )
    sys.stdout.write(result.stdout)
    if result.returncode != 0:
        fail(errors, "api-schema.json does not match the dataset property manifest")


# ---------------------------------------------------------------------------------------------------
# Spec 0036: dataset property manifest
# ---------------------------------------------------------------------------------------------------

PROPERTY_SCHEMA = RCC / "properties" / "schema.json"
FRAME_H = ROOT / "app" / "src" / "DataModel" / "Frame.h"
HOOKS_H = ROOT / "app" / "src" / "DataModel" / "Project" / "PropertyHooks.h"
EDITOR_H = ROOT / "app" / "src" / "DataModel" / "ProjectEditor.h"
MODEL_H = ROOT / "app" / "src" / "DataModel" / "ProjectModel.h"
GENERATED_REGISTRY = (
    ROOT / "app" / "src" / "DataModel" / "Generated" / "DatasetRegistry.h"
)

# Hooks the generated code never calls: the caller owns the dialog and the deferred form
# snap-back, so no PropertyHooks symbol backs the name.
CALLER_OWNED_HOOKS = {"onAliasRejected"}

# Manifest fields whose value names a hook, paired with the hook kind it must declare.
HOOK_REFERENCES = {
    "visibleWhen": "predicate",
    "enabledWhen": "predicate",
    "validate": "validator",
    "placeholderHook": "placeholder",
    "onCommit": "commit",
}

SCHEMA_KEYWORDS = {
    "$ref",
    "$id",
    "$schema",
    "additionalProperties",
    "definitions",
    "description",
    "enum",
    "items",
    "minItems",
    "properties",
    "required",
    "title",
    "type",
}

SCHEMA_TYPES = {"object": dict, "array": list, "string": str, "boolean": bool}


def type_matches(value, kind: str) -> bool:
    if kind == "integer":
        return isinstance(value, int) and not isinstance(value, bool)
    if kind == "number":
        return isinstance(value, (int, float)) and not isinstance(value, bool)
    return isinstance(value, SCHEMA_TYPES[kind])


def validate_object(
    value: dict, schema: dict, where: str, root: dict, errors: list[str]
):
    props = schema.get("properties", {})
    extra = schema.get("additionalProperties", True)
    for name in schema.get("required", []):
        if name not in value:
            fail(errors, f"dataset.json: {where} is missing required '{name}'")

    for name, entry in value.items():
        if name in props:
            validate_node(entry, props[name], f"{where}.{name}", root, errors)
        elif extra is False:
            fail(errors, f"dataset.json: {where}.{name} is not declared in schema.json")
        elif isinstance(extra, dict):
            validate_node(entry, extra, f"{where}.{name}", root, errors)


def validate_node(
    value, schema: dict, where: str, root: dict, errors: list[str]
) -> None:
    """Validate one manifest node against the draft-07 subset schema.json actually uses.

    Self-contained rather than jsonschema-backed so the gate runs identically in
    sanitize-commit.py and in CI; an unrecognized keyword fails loudly instead of being
    skipped, so the schema can never outgrow the validator silently.
    """
    if "$ref" in schema:
        target = schema["$ref"]
        name = target.rsplit("/", 1)[-1]
        if not target.startswith("#/definitions/") or name not in root.get(
            "definitions", {}
        ):
            fail(errors, f"schema.json: unresolved $ref '{target}'")
            return
        validate_node(value, root["definitions"][name], where, root, errors)
        return

    for keyword in schema:
        if keyword not in SCHEMA_KEYWORDS:
            fail(
                errors,
                f"schema.json: unsupported keyword '{keyword}' at {where}; teach "
                "registry-verify's validator about it before using it",
            )
            return

    kind = schema.get("type")
    if kind and not type_matches(value, kind):
        fail(errors, f"dataset.json: {where} must be a {kind}")
        return

    if "enum" in schema and value not in schema["enum"]:
        fail(
            errors, f"dataset.json: {where} = {value!r} is not one of {schema['enum']}"
        )

    if isinstance(value, dict):
        validate_object(value, schema, where, root, errors)
    elif isinstance(value, list):
        if len(value) < schema.get("minItems", 0):
            fail(errors, f"dataset.json: {where} needs {schema['minItems']} item(s)")
        for index, entry in enumerate(value):
            if "items" in schema:
                validate_node(entry, schema["items"], f"{where}[{index}]", root, errors)


def struct_fields(text: str, name: str) -> list[str]:
    """Return the member names of a struct, in declaration order."""
    block = re.search(
        rf"struct (?:alignas\(\d+\) )?{name} \{{(.*?)\n\}};", text, re.DOTALL
    )
    if not block:
        return []

    fields: list[str] = []
    for line in block.group(1).splitlines():
        stripped = line.split("///")[0].strip()
        match = re.match(r"^[A-Za-z_][\w:<>, ]*?\s+(\w+)\s*(=|;)", stripped)
        if match:
            fields.append(match.group(1))
    return fields


def enum_values(text: str, pattern: str) -> list[str]:
    block = re.search(pattern, text, re.DOTALL)
    if not block:
        return []
    body = re.sub(r"//[^\n]*", "", block.group(1))
    return [v.strip().split("=")[0].strip() for v in body.split(",") if v.strip()]


def declared_symbols(text: str) -> set[str]:
    """Return every function name declared in a header."""
    return set(re.findall(r"\b(\w+)\s*\(", text))


def manifest_field_owners(manifest: dict) -> dict[str, list[str]]:
    """Map every struct field the manifest accounts for to the entries that claim it."""
    owners: dict[str, list[str]] = defaultdict(list)
    for prop in manifest["properties"]:
        owners[prop["field"]].append(f"property {prop['id']}")
    for runtime in manifest["runtimeFields"]:
        owners[runtime["field"]].append("runtimeFields")
    for sub in manifest["subEntities"]:
        owners[sub["field"]].append("subEntities")
    return owners


def check_manifest_keys(errors: list[str], manifest: dict, keys: set[str]) -> None:
    """Every jsonKey the manifest names must be a real Keys:: constant in Frame.h."""
    referenced: list[tuple[str, str]] = []
    for prop in manifest["properties"]:
        if prop.get("jsonKey"):
            referenced.append((prop["id"], prop["jsonKey"]))
        for legacy in prop.get("legacyKeys", []):
            referenced.append((prop["id"], legacy))
    for runtime in manifest["runtimeFields"]:
        if runtime.get("jsonKey"):
            referenced.append((runtime["field"], runtime["jsonKey"]))
    for sub in manifest["subEntities"]:
        if sub.get("jsonKey"):
            referenced.append((sub["field"], sub["jsonKey"]))

    for owner, constant in referenced:
        if constant not in keys:
            fail(
                errors,
                f"dataset.json: {owner} names jsonKey '{constant}', which is not a "
                "Keys:: constant in app/src/DataModel/Frame.h",
            )


def check_manifest_hooks(errors: list[str], manifest: dict) -> None:
    """Every referenced hook is declared, of the right kind, and backed by C++."""
    hooks = manifest["hooks"]
    hooks_text = HOOKS_H.read_text(encoding="utf-8") if HOOKS_H.exists() else ""
    frame_text = FRAME_H.read_text(encoding="utf-8") if FRAME_H.exists() else ""
    hook_symbols = declared_symbols(hooks_text)
    frame_symbols = declared_symbols(frame_text)

    referenced: list[tuple[str, str, str]] = []
    for prop in manifest["properties"]:
        for attribute, kind in HOOK_REFERENCES.items():
            if prop.get(attribute):
                referenced.append((f"property {prop['id']}", prop[attribute], kind))
    for sub in manifest["subEntities"]:
        referenced.append((sub["field"], sub["readHook"], "subEntity"))
        referenced.append((sub["field"], sub["writeHook"], "subEntity"))
    for source in manifest["optionSources"].values():
        if source.get("hook"):
            referenced.append(("optionSources", source["hook"], "optionProvider"))

    for owner, name, kind in referenced:
        declared = hooks.get(name)
        if declared is None:
            fail(errors, f"dataset.json: {owner} references undeclared hook '{name}'")
            continue
        if declared["kind"] != kind:
            fail(
                errors,
                f"dataset.json: {owner} uses hook '{name}' as a {kind}, but it is "
                f"declared as a {declared['kind']}",
            )

    for name, declared in hooks.items():
        pool = frame_symbols if declared["kind"] == "subEntity" else hook_symbols
        home = (
            "Frame.h" if declared["kind"] == "subEntity" else "Project/PropertyHooks.h"
        )
        symbol = re.search(r"(\w+)\(", declared["signature"])
        if symbol and symbol.group(1) in pool:
            continue
        if name in pool:
            continue
        if name in CALLER_OWNED_HOOKS and "caller-owned" in declared["signature"]:
            continue
        fail(
            errors,
            f"dataset.json: hook '{name}' is not declared in app/src/DataModel/{home} "
            "and is not marked caller-owned",
        )


def check_manifest_form(errors: list[str], manifest: dict) -> None:
    """Widgets, form ids, option sources and cross-references resolve."""
    props = {prop["id"]: prop for prop in manifest["properties"]}
    editor_text = EDITOR_H.read_text(encoding="utf-8") if EDITOR_H.exists() else ""
    model_text = MODEL_H.read_text(encoding="utf-8") if MODEL_H.exists() else ""
    widgets = set(enum_values(editor_text, r"enum EditorWidget \{(.*?)\};"))
    accessors = declared_symbols(model_text)

    for prop in manifest["properties"]:
        widget = prop.get("widget")
        if widget and widget not in widgets:
            fail(
                errors,
                f"dataset.json: property {prop['id']} declares widget '{widget}', "
                "which is not a ProjectEditor::EditorWidget enumerator",
            )
        for attribute in ("pairWith", "persistWith"):
            partner = prop.get(attribute)
            if partner and partner not in props:
                fail(
                    errors,
                    f"dataset.json: property {prop['id']}.{attribute} names undeclared "
                    f"property '{partner}'",
                )
        source = prop.get("options")
        if source and source not in manifest["optionSources"]:
            fail(
                errors,
                f"dataset.json: property {prop['id']} names undeclared option source "
                f"'{source}'",
            )

    for name, source in manifest["optionSources"].items():
        for field in source.get("fields", []):
            if field not in props:
                fail(
                    errors,
                    f"dataset.json: option source {name} drives undeclared property "
                    f"'{field}'",
                )
        for attribute in ("labelProvider", "valueProvider"):
            provider = source.get(attribute)
            if provider and provider not in accessors:
                fail(
                    errors,
                    f"dataset.json: option source {name}.{attribute} names "
                    f"{provider}(), which ProjectModel.h does not declare",
                )

    for builder in manifest["form"]["builders"]:
        for row in builder["rows"]:
            if row not in props:
                fail(
                    errors,
                    f"dataset.json: form builder {builder['function']} emits undeclared "
                    f"property '{row}'",
                )


def check_form_id_order(errors: list[str], manifest: dict) -> None:
    """The generated enum must carry the manifest's frozen enumerator order.

    Renumbering DatasetItem silently repoints every persisted form-row id, so the
    manifest order and the generated header are compared enumerator for enumerator.
    """
    props = {prop["id"]: prop for prop in manifest["properties"]}
    order = manifest["formIdOrder"]
    for pid in order:
        if pid not in props:
            fail(errors, f"dataset.json: formIdOrder names undeclared property '{pid}'")
            return
        if not props[pid].get("formId"):
            fail(
                errors, f"dataset.json: formIdOrder names '{pid}', which has no formId"
            )
            return

    for prop in manifest["properties"]:
        if prop.get("formId") and prop["id"] not in order:
            fail(
                errors,
                f"dataset.json: property {prop['id']} declares a formId but is absent "
                "from formIdOrder",
            )

    if not GENERATED_REGISTRY.exists():
        fail(errors, "dataset.json: DatasetRegistry.h is missing; run the generator")
        return

    text = GENERATED_REGISTRY.read_text(encoding="utf-8")
    emitted = enum_values(text, r"typedef enum \{(.*?)\} DatasetItem;")
    expected = [props[pid]["formId"] for pid in order]
    if emitted != expected:
        fail(
            errors,
            "DatasetRegistry.h: the DatasetItem enumerator order does not match "
            "dataset.json's formIdOrder; regenerate with "
            "python3 scripts/generate-property-registry.py",
        )


def check_property_manifests(errors: list[str]) -> None:
    """Spec-0036 manifest lint: shape, key/hook/widget resolution, struct coverage."""
    if not DATASET_MANIFEST.exists() or not PROPERTY_SCHEMA.exists():
        fail(
            errors,
            "property manifest lint skipped: dataset.json or schema.json is missing",
        )
        return

    try:
        manifest = dataset_manifest()
        schema = json.loads(PROPERTY_SCHEMA.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        fail(errors, f"invalid JSON in app/rcc/properties: {error}")
        return

    validate_node(manifest, schema, "manifest", schema, errors)

    seen: set[str] = set()
    for prop in manifest["properties"]:
        if prop["id"] in seen:
            fail(errors, f"dataset.json: duplicate property id '{prop['id']}'")
        seen.add(prop["id"])

    frame_text = FRAME_H.read_text(encoding="utf-8") if FRAME_H.exists() else ""
    keys = set(re.findall(r"inline constexpr KeyView (\w+)\(", frame_text))
    fields = struct_fields(frame_text, manifest["entity"])
    if not fields:
        fail(errors, f"could not parse struct {manifest['entity']} from Frame.h")
        return

    owners = manifest_field_owners(manifest)
    for field in fields:
        if field not in owners:
            fail(
                errors,
                f"dataset.json: Dataset::{field} is neither declared as a property nor "
                "listed in runtimeFields/subEntities",
            )
    for field, claims in sorted(owners.items()):
        if field not in fields:
            fail(
                errors,
                f"dataset.json: {claims[0]} names Dataset::{field}, which does not exist",
            )
        elif len(claims) > 1:
            fail(
                errors,
                f"dataset.json: Dataset::{field} is declared twice ({', '.join(claims)})",
            )

    check_manifest_keys(errors, manifest, keys)
    check_manifest_hooks(errors, manifest)
    check_manifest_form(errors, manifest)
    check_form_id_order(errors, manifest)


# ---------------------------------------------------------------------------------------------------
# Spec 0038: widget extension packages
# ---------------------------------------------------------------------------------------------------

EXTENSIONS = RCC / "extensions"
WIDGET_SCHEMA = EXTENSIONS / "schema" / "widget-manifest.json"
BUNDLED_WIDGETS = EXTENSIONS / "widget"
WIDGET_CATALOG_CPP = ROOT / "app" / "src" / "UI" / "WidgetExtensions.cpp"
SERIALSTUDIO_CPP = ROOT / "app" / "src" / "SerialStudio.cpp"
MODULE_MANAGER_CPP = ROOT / "app" / "src" / "Misc" / "ModuleManager.cpp"

# Context properties that carry a build constant or a plain value rather than a host object.
# Shadowing them in an extension's context would narrow nothing, so hostContextNames() omits them
# on purpose and the drift lint must not demand them.
VALUE_CONTEXT_PROPERTIES = {
    "Cpp_AppName",
    "Cpp_AppOrganization",
    "Cpp_AppOrganizationDomain",
    "Cpp_AppUpdaterUrl",
    "Cpp_AppVersion",
    "Cpp_BuildDate",
    "Cpp_BuildTime",
    "Cpp_CommercialBuild",
    "Cpp_GrpcAvailable",
    "Cpp_HasWebEngine",
    "Cpp_PrimaryScreen",
    "Cpp_ScreenList",
    "Cpp_UpdaterEnabled",
}


def widget_schema() -> dict | None:
    try:
        return json.loads(WIDGET_SCHEMA.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None


def schema_reserved_ids(schema: dict) -> list[str]:
    return list(schema.get("definitions", {}).get("reservedId", {}).get("enum", []))


def catalog_reserved_ids() -> list[str]:
    """Return the reserved builtin widget strings WidgetExtensions::reservedIds() ships."""
    text = WIDGET_CATALOG_CPP.read_text(encoding="utf-8")
    block = re.search(r"reservedIds\(\)\s*\{(.*?)\n\}", text, re.DOTALL)
    if not block:
        return []
    return re.findall(r'QStringLiteral\("([^"]+)"\)', block.group(1))


def builtin_widget_strings() -> set[str]:
    """Return every widget string SerialStudio.cpp maps to a builtin dashboard widget."""
    text = SERIALSTUDIO_CPP.read_text(encoding="utf-8")
    block = re.search(r"SerialStudio::getDashboardWidget\(.*?\n\}", text, re.DOTALL)
    found = set(re.findall(r'widget == "([^"]+)"', block.group(0) if block else ""))
    table = re.search(r"kDatasetWidgetMap = \{(.*?)\};", text, re.DOTALL)
    if table:
        found |= set(re.findall(r'QStringLiteral\("([^"]+)"\)', table.group(1)))
    return found


def jsonschema_validator(schema: dict):
    """Return a callable validating one manifest, or None when jsonschema is unavailable.

    The dependency is optional on purpose: the rules that carry the security weight (reserved
    ids, bundled-only replaces, path escapes) are checked by hand below, so a machine without
    jsonschema still gates them; the library only adds the full draft-07 reading.
    """
    try:
        import jsonschema  # noqa: PLC0415
    except ImportError:
        return None

    def validate(instance: dict) -> list[str]:
        validator = jsonschema.Draft7Validator(schema)
        return [e.message for e in validator.iter_errors(instance)]

    return validate


def check_widget_schema_rules(errors: list[str], schema: dict) -> None:
    """Seed the schema with a legal and an illegal manifest, so the pairing rule cannot rot.

    The reserved-id enum plus the anyOf pairing is the whole of R10 on the declaration side: a
    package may claim a builtin widget string only by shipping bundled and declaring `replaces`.
    A schema edit that drops the pairing would otherwise pass every other check here.
    """
    validate = jsonschema_validator(schema)
    if validate is None:
        print(
            "registry-verify: jsonschema is not installed -- widget-manifest schema seeds "
            "skipped (the reserved-id and path rules below still run)"
        )
        return

    third_party = {
        "id": "com.acme.thermal-map",
        "type": "widget",
        "title": "Thermal Map",
        "version": "1.2.0",
        "widget": {"scope": "dataset", "qml": "ThermalMap.qml"},
    }
    if validate(third_party):
        fail(
            errors,
            "widget-manifest.json rejects a plain third-party manifest: "
            + "; ".join(validate(third_party)),
        )

    claimed = json.loads(json.dumps(third_party))
    claimed["id"] = "plot3d"
    if not validate(claimed):
        fail(
            errors,
            "widget-manifest.json accepts a package claiming the reserved id 'plot3d' "
            "without declaring replaces (spec 0038 R10)",
        )

    replacing = json.loads(json.dumps(claimed))
    replacing["widget"]["replaces"] = "plot3d"
    if validate(replacing):
        fail(
            errors,
            "widget-manifest.json rejects a reserved id paired with replaces, which the "
            "bundled packages depend on",
        )

    escaping = json.loads(json.dumps(third_party))
    escaping["widget"]["qml"] = "../escape.qml"
    if not validate(escaping):
        fail(errors, "widget-manifest.json accepts a qml entry escaping the package")


def check_widget_reserved_ids(errors: list[str], schema: dict) -> None:
    """The schema, the C++ catalog, and the widget-string mappers must name one reserved set."""
    declared = schema_reserved_ids(schema)
    catalog = catalog_reserved_ids()
    if not catalog:
        fail(
            errors, "could not parse reservedIds() from app/src/UI/WidgetExtensions.cpp"
        )
        return

    for missing in sorted(set(declared) - set(catalog)):
        fail(
            errors,
            f"reserved id '{missing}' is in widget-manifest.json but not in "
            "WidgetExtensions::reservedIds()",
        )
    for missing in sorted(set(catalog) - set(declared)):
        fail(
            errors,
            f"reserved id '{missing}' is in WidgetExtensions::reservedIds() but not in "
            "widget-manifest.json",
        )

    for widget in sorted(builtin_widget_strings() - set(catalog)):
        fail(
            errors,
            f"builtin widget string '{widget}' is resolvable in SerialStudio.cpp but is not "
            "reserved, so an extension package could claim it (spec 0038 R10)",
        )


def widget_package_dirs() -> list[Path]:
    if not BUNDLED_WIDGETS.is_dir():
        return []
    return sorted(p for p in BUNDLED_WIDGETS.iterdir() if (p / "info.json").is_file())


def check_widget_package(
    errors: list[str], path: Path, schema: dict, seen: dict
) -> None:
    """Validate one bundled package: schema, id rules, and file resolution."""
    rel = path.as_posix()
    if path.is_relative_to(ROOT):
        rel = path.relative_to(ROOT).as_posix()

    try:
        manifest = json.loads((path / "info.json").read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        fail(errors, f"{rel}/info.json: invalid JSON: {error}")
        return

    validate = jsonschema_validator(schema)
    for message in validate(manifest) if validate else []:
        fail(errors, f"{rel}/info.json: {message}")

    package = manifest.get("id", "")
    block = manifest.get("widget", {})
    if package != path.name:
        fail(
            errors,
            f"{rel}/info.json: id '{package}' does not match its directory name, which the "
            "bundled qrc paths encode",
        )
    if package in seen:
        fail(
            errors,
            f"{rel}/info.json: duplicate package id '{package}' ({seen[package]})",
        )
    seen[package] = rel

    replaces = block.get("replaces", "")
    if replaces and replaces != package:
        fail(
            errors,
            f"{rel}/info.json: replaces '{replaces}' differs from the package id, which the "
            "host refuses",
        )
    if not replaces and package in schema_reserved_ids(schema):
        fail(
            errors, f"{rel}/info.json: claims reserved id '{package}' without replaces"
        )

    declared = set(manifest.get("files", []))
    on_disk = {p.name for p in path.iterdir() if p.is_file() and p.name != ".DS_Store"}
    for missing in sorted(declared - on_disk):
        fail(errors, f"{rel}/info.json: declared file '{missing}' does not exist")
    for extra in sorted(on_disk - declared):
        fail(errors, f"{rel}: file '{extra}' is not listed in the manifest's files")

    entry = block.get("qml", "")
    if entry and not (path / entry).is_file():
        fail(errors, f"{rel}/info.json: qml entry '{entry}' does not exist")

    icon = block.get("icon", "")
    if icon and "." not in icon and not icon_ref_resolves(icon):
        fail(errors, f"{rel}/info.json: icon id '{icon}' does not resolve")
    elif icon and "." in icon and not (path / icon).is_file():
        fail(errors, f"{rel}/info.json: icon file '{icon}' does not exist")


def check_widget_qrc_sync(errors: list[str]) -> None:
    """Every bundled package file ships in rcc.qrc, and every listed one exists."""
    text = QRC.read_text(encoding="utf-8")
    listed = set(re.findall(r"<file>(extensions/[^<]+)</file>", text))
    disk = {
        p.relative_to(RCC).as_posix()
        for p in EXTENSIONS.rglob("*")
        if p.is_file() and p.name != ".DS_Store"
    }
    for missing in sorted(disk - listed):
        fail(errors, f"bundled extension file not registered in qrc: {missing}")
    for stale in sorted(listed - disk):
        fail(errors, f"qrc entry has no bundled extension file on disk: {stale}")


def check_widget_manifests(errors: list[str]) -> None:
    """Spec-0038 lint: bundled manifests, the reserved-id contract, and qrc sync."""
    schema = widget_schema()
    if schema is None:
        fail(
            errors, "missing or invalid app/rcc/extensions/schema/widget-manifest.json"
        )
        return

    check_widget_schema_rules(errors, schema)
    check_widget_reserved_ids(errors, schema)
    check_widget_qrc_sync(errors)

    seen: dict[str, str] = {}
    packages = widget_package_dirs()
    if not packages:
        fail(
            errors,
            "no bundled widget extension packages found under app/rcc/extensions/widget",
        )
        return

    for path in packages:
        check_widget_package(errors, path, schema, seen)


def check_host_context_names(errors: list[str]) -> None:
    """Every host context property must be shadowed in an extension's QML context.

    Qt cannot enumerate a context's properties, so WidgetExtensions::hostContextNames() is a
    hand-kept mirror of ModuleManager's registrations; a name that drifts out of it stays
    reachable by name from package QML. That is a leaked name, not a breached boundary -- the
    shadowing is a speed bump either way -- but it is the kind of drift a lint closes for free.
    """
    if not MODULE_MANAGER_CPP.is_file() or not WIDGET_CATALOG_CPP.is_file():
        fail(
            errors,
            "context-name drift lint skipped: ModuleManager.cpp or WidgetExtensions.cpp",
        )
        return

    registered = set(
        re.findall(
            r'setContextProperty\(\s*"(Cpp_\w+)"',
            MODULE_MANAGER_CPP.read_text(encoding="utf-8"),
        )
    )
    text = WIDGET_CATALOG_CPP.read_text(encoding="utf-8")
    block = re.search(r"hostContextNames\(\)\s*\{(.*?)\n\}", text, re.DOTALL)
    if not block:
        fail(
            errors,
            "could not parse hostContextNames() from app/src/UI/WidgetExtensions.cpp",
        )
        return

    shadowed = set(re.findall(r"\bCpp_\w+", block.group(1)))
    for missing in sorted(registered - shadowed - VALUE_CONTEXT_PROPERTIES):
        fail(
            errors,
            f"context property '{missing}' is registered in ModuleManager but is not shadowed "
            "by WidgetExtensions::hostContextNames(); add it there (spec 0038 T17)",
        )
    for stale in sorted(shadowed - registered):
        fail(
            errors,
            f"WidgetExtensions::hostContextNames() shadows '{stale}', which ModuleManager no "
            "longer registers; drop it",
        )


def alias_reference_counts(aliases: dict[str, str]) -> dict[str, int]:
    counts = {alias: 0 for alias in aliases}
    for source_root in SOURCE_ROOTS:
        for path in sorted(source_root.rglob("*")):
            if path.suffix not in SOURCE_EXTS:
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            for alias in counts:
                if alias in text:
                    counts[alias] += text.count(alias)
    return counts


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--alias-report", action="store_true")
    parser.add_argument("--require-no-alias-refs", action="store_true")
    args = parser.parse_args()

    if not ICONS.is_dir():
        sys.exit(f"registry-verify: missing icon tree {ICONS}")
    errors: list[str] = []
    check_tree(errors)
    aliases = check_qrc(errors)
    ids = check_manifests(errors)
    check_layouts(errors, ids)
    check_binding_guards(errors)
    check_icon_render_sizes(errors)
    check_property_manifests(errors)
    check_widget_manifests(errors)
    check_host_context_names(errors)
    check_api_snapshot(errors)
    check_corpus_field_refs(errors)

    referenced = 0
    if aliases and (args.alias_report or args.require_no_alias_refs):
        counts = alias_reference_counts(aliases)
        live = {a: n for a, n in counts.items() if n}
        referenced = sum(live.values())
        if args.alias_report:
            for alias, count in sorted(live.items(), key=lambda kv: (-kv[1], kv[0])):
                print(f"  {count:4d}  {alias}")
        print(
            f"aliases: {len(aliases)} ({len(live)} still referenced, "
            f"{referenced} total source refs)"
        )
        if args.require_no_alias_refs and live:
            errors.append(f"{len(live)} aliases still referenced by sources")

    for message in errors:
        print(f"FAIL: {message}")
    print(
        f"registry-verify: {'CLEAN' if not errors else f'{len(errors)} violation(s)'}"
    )
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
