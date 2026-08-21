#!/usr/bin/env python3
"""Generate the dataset property-registry surfaces from the property manifest (spec 0036).

Reads app/rcc/properties/dataset.json -- the single declaration of every persisted
or editable dataset property -- and emits four checked-in C++ translation units:

    app/src/DataModel/Generated/DatasetRegistry.h        descriptor table + form-id enum
    app/src/DataModel/Generated/DatasetSerialization.cpp project-JSON write + read
    app/src/DataModel/Generated/DatasetForm.cpp          editor rows + commit dispatcher
    app/src/API/Generated/DatasetApiFields.cpp           API field appliers + typed schema

Output is deterministic (manifest order, LF endings) and each file is fenced with
clang-format off/on so the sanitize pipeline's reformat pass cannot fight --check.

Spec 0037 adds the downstream API surfaces, generated from the committed API snapshot
(app/rcc/api/api-schema.json) plus the same manifest:

    app/rcc/api/proto-fields.json         append-only gRPC field-number ledger
    doc/grpc/serialstudio-typed.proto     client-facing typed proto (codegen input)

and a buildless projection check (--check-snapshot) that compares the dataset verbs'
typed schema in the committed snapshot against the manifest. The snapshot itself can
only be refreshed from a build, so that check warns locally and fails in CI.

Usage:
    python3 scripts/generate-property-registry.py [--check]
    python3 scripts/generate-property-registry.py --check-snapshot [--strict]
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "app" / "rcc" / "properties" / "dataset.json"
API_SCHEMA = ROOT / "app" / "rcc" / "api" / "api-schema.json"
LEDGER = ROOT / "app" / "rcc" / "api" / "proto-fields.json"
TYPED_PROTO = ROOT / "doc" / "grpc" / "serialstudio-typed.proto"

SNAPSHOT_COMMAND = "project.dataset.update"

# Declared by datasetUpdateSchema() in app/src/API/Handlers/ProjectHandler.cpp, not by the
# manifest: the two identity params address the dataset the patch applies to.
IDENTITY_PARAMS = ("groupId", "datasetId")

MAX_REPORTED = 12

OUT_REGISTRY = ROOT / "app" / "src" / "DataModel" / "Generated" / "DatasetRegistry.h"
OUT_SERIAL = (
    ROOT / "app" / "src" / "DataModel" / "Generated" / "DatasetSerialization.cpp"
)
OUT_FORM = ROOT / "app" / "src" / "DataModel" / "Generated" / "DatasetForm.cpp"
OUT_API = ROOT / "app" / "src" / "API" / "Generated" / "DatasetApiFields.cpp"

LICENSE = """/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */
"""

BANNER = (
    "// AUTO-GENERATED from app/rcc/properties/dataset.json; never edit by hand.\n"
    "\n"
    "// Regenerate with: python3 scripts/generate-property-registry.py\n"
)

MAX_COLS = 100
BAR = "//" + "-" * 98


# --------------------------------------------------------------------------------------------------
# Manifest access helpers
# --------------------------------------------------------------------------------------------------


def load_manifest() -> dict:
    return json.loads(MANIFEST.read_text(encoding="utf-8"))


def by_id(manifest: dict) -> dict:
    return {p["id"]: p for p in manifest["properties"]}


def snake(name: str) -> str:
    out = []
    for ch in name:
        if ch.isupper():
            out.append("_")
            out.append(ch.lower())
        else:
            out.append(ch)
    return "".join(out).lstrip("_")


def escaped(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def literal(value) -> str:
    """Render a JSON scalar as the C++ literal the emitters need."""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, str):
        return f'"{escaped(value)}"'
    if isinstance(value, float) and value.is_integer():
        return str(int(value))
    return str(value)


def split_text(text: str, width: int) -> list[str]:
    """Split a string literal into chunks that fit inside width characters."""
    if width < 16:
        width = 16
    words = text.split(" ")
    chunks: list[str] = []
    current = ""
    for word in words:
        candidate = word if not current else current + " " + word
        if len(candidate) > width and current:
            chunks.append(current + " ")
            current = word
        else:
            current = candidate
    chunks.append(current)
    return chunks


def wrapped_call(head: str, text: str, tail: str, translate: bool) -> list[str]:
    """Emit head + a (possibly split) string literal + tail, keeping every line inside 100 cols."""
    opener = "tr(" if translate else ""
    closer = ")" if translate else ""
    align = len(head) + len(opener)
    single = f'{head}{opener}"{escaped(text)}"{closer}{tail}'
    if len(single) <= MAX_COLS:
        return [single]

    chunks = split_text(text, MAX_COLS - align - 4)
    lines = [f'{head}{opener}"{escaped(chunks[0])}"']
    for chunk in chunks[1:]:
        lines.append(" " * align + f'"{escaped(chunk)}"')

    rest = closer + tail
    if len(lines[-1]) + len(rest) <= MAX_COLS:
        lines[-1] += rest
        return lines

    lines[-1] += closer + ","
    lines.append(" " * len(head.rstrip()) + tail.lstrip(", "))
    return lines


def call_break(text: str) -> int:
    """Return the index of the first '(' whose argument list has a top-level comma."""
    for index, char in enumerate(text):
        if char not in "({":
            continue

        depth = 0
        for scan in range(index, len(text)):
            if text[scan] in "([{":
                depth += 1
            elif text[scan] in ")]}":
                depth -= 1
                if depth == 0:
                    break
            elif text[scan] == "," and depth == 1:
                return index

    return -1


def emit_statement(indent: str, text: str) -> list[str]:
    """Emit one statement, breaking it at top-level argument commas when it overflows."""
    if len(indent + text) <= MAX_COLS:
        return [indent + text]

    if " = " in text:
        name, _, rhs = text.partition(" = ")
        if len(indent) + len(name) + 2 <= MAX_COLS:
            tail = emit_statement(indent + "  ", rhs)
            if len(tail) == 1 or len(tail[0]) <= MAX_COLS:
                return [f"{indent}{name} ="] + tail

    open_paren = call_break(text)
    if open_paren < 0:
        return [indent + text]

    head = indent + text[: open_paren + 1]
    rest = text[open_paren + 1 :]
    args: list[str] = []
    depth = 0
    current = ""
    for char in rest:
        if char in "([{":
            depth += 1
        elif char in ")]}":
            depth -= 1
        if char == "," and depth == 0:
            args.append(current)
            current = ""
            continue
        current += char
    args.append(current)

    lines: list[str] = []
    line = head
    for index, arg in enumerate(args):
        piece = arg.strip() + ("," if index + 1 < len(args) else "")
        candidate = line + (" " if line.endswith(",") else "") + piece
        if len(candidate) > MAX_COLS and not line.endswith("("):
            lines.append(line)
            line = " " * len(head) + piece
        else:
            line = candidate

    lines.append(line)
    return lines


def table_entry(parts: list[str]) -> str:
    """Render one brace-initialized table row, wrapped so every line fits inside 100 columns."""
    lines: list[str] = []
    current = "  {"
    for index, part in enumerate(parts):
        piece = part + ("," if index + 1 < len(parts) else "},")
        candidate = current + (" " if current.endswith(",") else "") + piece
        if len(candidate) > MAX_COLS - 2 and not current.endswith("{"):
            lines.append(current)
            current = "   " + piece
        else:
            current = candidate
    lines.append(current)
    return "\n".join(lines) + "\n"


def doxygen(text: str, indent: str = "") -> list[str]:
    lines = [f"{indent}/**"]
    body = f"@brief {text}"
    width = MAX_COLS - len(indent) - 3
    for i, chunk in enumerate(split_text(body, width)):
        lines.append(
            f"{indent} * {chunk.rstrip()}"
            if i == 0
            else f"{indent} *        {chunk.rstrip()}"
        )
    lines.append(f"{indent} */")
    return lines


# --------------------------------------------------------------------------------------------------
# Shared property queries
# --------------------------------------------------------------------------------------------------


def key(prop: dict) -> str:
    return f"Keys::{prop['jsonKey']}"


def key_constants() -> dict[str, str]:
    """Map every Keys:: literal to its constant name so emitted code never hard-codes a key."""
    header = (ROOT / "app" / "src" / "DataModel" / "Frame.h").read_text(
        encoding="utf-8"
    )
    found: dict[str, str] = {}
    for name, value in re.findall(
        r"inline constexpr KeyView (\w+)\(\"([^\"]*)\"\)", header
    ):
        found.setdefault(value, name)

    return found


KEYS = key_constants()


def json_name(name: str) -> str:
    """Render an API/JSON field name, preferring the Keys:: constant when one spells it."""
    constant = KEYS.get(name)
    if constant:
        return f"Keys::{constant}"

    return f'QStringLiteral("{name}")'


def field(prop: dict) -> str:
    return f"d.{prop['field']}"


def is_choice(prop: dict) -> bool:
    return bool(prop.get("options"))


def option_accessor(name: str) -> str:
    if name.endswith("Options"):
        return name

    return f"{name}Options"


def option_type(source: dict) -> str:
    return {
        "staticMap": "PropertyHooks::StaticMapOptions",
        "extensibleMap": "PropertyHooks::ExtensibleMapOptions",
        "parallelValues": "PropertyHooks::ParallelValueOptions",
        "liveProvider": "PropertyHooks::LiveProviderOptions",
        "tuple": "PropertyHooks::TupleOptions",
    }[source["kind"]]


def schema_type(prop: dict) -> str:
    return {
        "int": "integer",
        "double": "number",
        "bool": "boolean",
        "string": "string",
    }[prop["type"]]


def schema_props_for(prop: dict, manifest: dict) -> list[dict]:
    """Return the API schema descriptors one declared property contributes.

    The single definition of "what schema does this property produce": the C++ emitter
    below and the api-schema projector both read this, so a second reading of the manifest
    cannot disagree with the first. An unexposed property contributes nothing.
    """
    api = prop.get("api", {})
    if not api.get("expose"):
        return []

    domain: list = []
    source_name = prop.get("options")
    if source_name:
        source = manifest["optionSources"][source_name]
        if source["kind"] in ("staticMap", "extensibleMap", "parallelValues"):
            domain = [entry["value"] for entry in source["entries"]]

    return [
        {
            "name": api["name"],
            "aliases": list(api.get("aliases", [])),
            "type": schema_type(prop),
            "description": prop.get("description")
            or prop.get("apiDescription")
            or prop["id"],
            "enum": domain,
            "required": False,
        }
    ]


def schema_descriptors(manifest: dict) -> list[dict]:
    """Return every dataset schema descriptor, in manifest order."""
    out: list[dict] = []
    for prop in manifest["properties"]:
        out += schema_props_for(prop, manifest)
    return out


def form_rows(manifest: dict) -> list[dict]:
    props = by_id(manifest)
    rows = []
    for builder in manifest["form"]["builders"]:
        for row in builder["rows"]:
            rows.append(props[row])
    return rows


# --------------------------------------------------------------------------------------------------
# DatasetRegistry.h
# --------------------------------------------------------------------------------------------------


def render_registry(manifest: dict) -> str:
    props = by_id(manifest)
    order = manifest["formIdOrder"]

    out: list[str] = [LICENSE, BANNER, "\n#pragma once\n"]
    out.append("\n#include <QLatin1StringView>\n#include <QVariant>\n")
    out.append('\n#include "DataModel/Frame.h"\n')
    out.append('#include "DataModel/Project/PropertyHooks.h"\n')
    out.append("\n// clang-format off\n")

    out.append(
        "\n" + "\n".join(doxygen("Form-field identifiers for the dataset view.")) + "\n"
    )
    out.append("typedef enum {\n")
    for pid in order:
        out.append(f"  {props[pid]['formId']},\n")
    out.append("} DatasetItem;\n")

    out.append("\nnamespace DataModel::Registry {\n")

    out.append(
        "\n" + "\n".join(doxygen("Value type of a declared dataset property.")) + "\n"
    )
    out.append("enum class PropertyType : quint8 { Int, Double, Bool, String };\n")

    out.append(
        "\n"
        + "\n".join(
            doxygen(
                "Editor row kind of a declared property; mirrors ProjectEditor::EditorWidget."
            )
        )
        + "\n"
    )
    out.append(
        "enum class PropertyWidget : quint8 {\n"
        "  None, TextField, IntField, FloatField, AutoIntField, CheckBox, ComboBox, ColorPicker\n"
        "};\n"
    )

    out.append(
        "\n"
        + "\n".join(doxygen("Project-file write rule of a declared property."))
        + "\n"
    )
    out.append(
        "enum class PersistRule : quint8 {\n"
        "  Always, Never, WhenTrue, WhenFalse, WhenNonEmpty, WhenNonZero, WhenPositive,\n"
        "  WhenNonNegative, WhenNonDefault, WithProperty\n"
        "};\n"
    )

    out.append(
        "\n"
        + "\n".join(
            doxygen(
                "One declared dataset property, joined across every derived surface."
            )
        )
        + "\n"
    )
    out.append(
        "struct DatasetProperty {\n"
        "  const char* id;\n"
        "  const char* field;\n"
        "  const char* apiName;\n"
        "  const char* undoLabel;\n"
        "  const char* coalesceKey;\n"
        "  int formId;\n"
        "  PropertyType type;\n"
        "  PropertyWidget widget;\n"
        "  PersistRule persist;\n"
        "  bool hasFormRow;\n"
        "  bool coalesce;\n"
        "  bool rebuildTree;\n"
        "  bool pro;\n"
        "};\n"
    )

    out.append(
        "\n"
        + "\n".join(doxygen("Every declared dataset property, in manifest order."))
        + "\n"
    )
    out.append("inline constexpr DatasetProperty kDatasetProperties[] = {\n")
    for prop in manifest["properties"]:
        undo = prop.get("undo", {})
        api = prop.get("api", {})
        form_id = prop.get("formId", "")
        has_row = bool(form_id) and not prop.get("deadFormId", False)
        widget = prop.get("widget", "None")
        out.append(
            table_entry(
                [
                    literal(prop["id"]),
                    literal(prop["field"]),
                    literal(api.get("name", "")),
                    literal(undo.get("label", "")),
                    literal(undo.get("coalesceKey", "dataset")),
                    form_id if form_id else "-1",
                    f"PropertyType::{prop['type'].capitalize()}",
                    f"PropertyWidget::{widget}",
                    f"PersistRule::{prop['persist'][0].upper() + prop['persist'][1:]}",
                    literal(has_row),
                    literal(bool(undo.get("coalesce", False))),
                    literal(bool(prop.get("rebuildTree", False))),
                    literal(bool(prop.get("pro", False))),
                ]
            )
        )
    out.append("};\n")

    count = len(manifest["properties"])
    out.append(
        "\n"
        + "\n".join(doxygen("Number of declared dataset properties."))
        + f"\ninline constexpr int kDatasetPropertyCount = {count};\n"
    )

    out.append(
        "\n"
        + "\n".join(
            doxygen(
                "Returns the property bound to a form-field id, or null when none is."
            )
        )
        + "\n"
        "[[nodiscard]] inline const DatasetProperty* datasetPropertyForFormId(int formId)\n"
        "{\n"
        "  for (int i = 0; i < kDatasetPropertyCount; ++i)\n"
        "    if (kDatasetProperties[i].hasFormRow && kDatasetProperties[i].formId == formId)\n"
        "      return &kDatasetProperties[i];\n"
        "\n"
        "  return nullptr;\n"
        "}\n"
    )

    out.append(
        "\n"
        + "\n".join(
            doxygen("Returns the property with a registry id, or null when undeclared.")
        )
        + "\n"
        "[[nodiscard]] inline const DatasetProperty* datasetPropertyById(QLatin1StringView id)\n"
        "{\n"
        "  for (int i = 0; i < kDatasetPropertyCount; ++i)\n"
        "    if (id == QLatin1StringView(kDatasetProperties[i].id))\n"
        "      return &kDatasetProperties[i];\n"
        "\n"
        "  return nullptr;\n"
        "}\n"
    )

    out.append(
        "\n" + BAR + "\n// Choice domains (defined in DatasetForm.cpp)\n" + BAR + "\n"
    )
    for name, source in manifest["optionSources"].items():
        out.append(
            "\n"
            + "\n".join(
                doxygen(f"Returns the shared option source for the {name} domain.")
            )
            + f"\n[[nodiscard]] const {option_type(source)}& {option_accessor(name)}();\n"
        )

    out.append(
        "\n"
        + "\n".join(
            doxygen(
                "Applies one dataset form edit onto d and reports the rebuild the caller must run; "
                "never touches ProjectModel and never rebuilds a form itself."
            )
        )
        + "\n[[nodiscard]] PropertyHooks::RebuildHint applyDatasetFormEdit(int formId,\n"
        "                                                            const QVariant& value,\n"
        "                                                            Dataset& d,\n"
        "                                                            const ProjectModel& pm);\n"
    )

    out.append(
        "\n"
        + "\n".join(
            doxygen(
                "Returns the value a dataset form row carries, or an invalid QVariant when the "
                "row is not built for this dataset."
            )
        )
        + "\n[[nodiscard]] QVariant datasetFormValue(int formId,\n"
        "                                        const Dataset& d,\n"
        "                                        const ProjectModel& pm);\n"
    )

    out.append("\n}  // namespace DataModel::Registry\n")
    out.append("\n// clang-format on\n")
    return "".join(out)


# --------------------------------------------------------------------------------------------------
# DatasetSerialization.cpp
# --------------------------------------------------------------------------------------------------


def write_condition(prop: dict, props: dict) -> str | None:
    rule = prop["persist"]
    name = field(prop)
    if rule == "always":
        return None
    if rule == "whenTrue":
        return f"if ({name})"
    if rule == "whenFalse":
        return f"if (!{name})"
    if rule == "whenNonEmpty":
        return f"if (!{name}.isEmpty())"
    if rule == "whenNonZero":
        return f"if ({name} != 0)"
    if rule == "whenPositive":
        return f"if ({name} > 0)"
    if rule == "whenNonNegative":
        return f"if ({name} >= 0)"
    if rule == "withProperty":
        gate = props[prop["persistWith"]]
        if gate["type"] == "string":
            return f"if (!{field(gate)}.isEmpty())"

        return f"if ({field(gate)})"
    raise SystemExit(f"generate-property-registry: unsupported persist rule '{rule}'")


def write_value(prop: dict, props: dict) -> str:
    if prop.get("invert") or (prop["persist"] == "whenTrue" and prop["type"] == "bool"):
        return "true"

    transform = prop.get("writeTransform")
    if transform == "simplified":
        return f"{field(prop)}.simplified()"
    if transform in ("pairMin", "pairMax"):
        partner = props[prop["pairWith"]]
        low, high = (prop, partner) if transform == "pairMin" else (partner, prop)
        fn = "qMin" if transform == "pairMin" else "qMax"
        return f"{fn}({field(low)}, {field(high)})"

    return field(prop)


def emit_write(prop: dict, props: dict) -> list[str]:
    condition = write_condition(prop, props)
    body = f"obj.insert({key(prop)}, {write_value(prop, props)});"
    if condition is None:
        return [f"  {body}"]

    return [f"  {condition}", f"    {body}", ""]


def read_expression(prop: dict) -> str:
    default = prop.get("default")
    if prop.get("invert"):
        return f"!ss_jsr(obj, {key(prop)}, false).toBool()"

    kind = prop["type"]
    if kind == "bool":
        return f"ss_jsr(obj, {key(prop)}, {literal(bool(default))}).toBool()"
    if kind == "int":
        return f"ss_jsr(obj, {key(prop)}, {literal(default)}).toInt()"
    if kind == "double":
        return f"SerialStudio::toDouble(ss_jsr(obj, {key(prop)}, {literal(default)}))"

    suffix = ".simplified()" if prop.get("readTransform") == "simplified" else ""
    return f"ss_jsr(obj, {key(prop)}, {literal(default or '')}).toString(){suffix}"


def documented_function(signature: str, brief: str, body: list[str]) -> list[str]:
    return [BAR, ""] + doxygen(brief) + [signature, "{"] + body + ["}", ""]


def render_serialization(manifest: dict) -> str:
    props = by_id(manifest)
    runtime = {r["field"]: r for r in manifest["runtimeFields"]}

    doc_props = [p for p in manifest["properties"] if p["persist"] != "never"]
    runtime_written = [
        r for r in manifest["runtimeFields"] if r.get("persist") == "always"
    ]

    def as_property(entry: dict) -> dict:
        return {
            "id": entry["field"],
            "field": entry["field"],
            "jsonKey": entry["jsonKey"],
            "type": entry["type"],
            "default": entry.get("default"),
            "persist": entry["persist"],
            "writeTransform": entry.get("writeTransform"),
            "readTransform": entry.get("readTransform"),
        }

    writable = doc_props + [as_property(r) for r in runtime_written]
    groups = {
        "Flags": [p for p in writable if p["type"] == "bool"],
        "Numbers": [p for p in writable if p["type"] in ("int", "double")],
        "Strings": [p for p in writable if p["type"] == "string"],
    }

    lines: list[str] = [LICENSE.rstrip(), "", BANNER.rstrip(), ""]
    lines += ["#include <QJsonArray>", ""]
    lines += [
        '#include "DataModel/Frame.h"',
        '#include "DataModel/Project/PropertyHooks.h"',
        '#include "SerialStudio.h"',
        "",
    ]
    lines += ["// clang-format off", ""]
    lines += ["namespace DataModel {", ""]

    for name, members in groups.items():
        body: list[str] = []
        for prop in members:
            body += emit_write(prop, props)
        while body and body[-1] == "":
            body.pop()
        lines += documented_function(
            f"static void writeDataset{name}(QJsonObject& obj, const Dataset& d)",
            f"Writes the declared {name.lower()} of a dataset into its project-JSON object.",
            body,
        )

    sub_body: list[str] = []
    for sub in manifest["subEntities"]:
        var = snake(sub["field"])
        sub_body += [
            f"  if (!d.{sub['field']}.empty()) {{",
            f"    QJsonArray {var};",
            f"    for (const auto& entry : d.{sub['field']})",
            f"      {var}.append(serialize(entry));",
            "",
            f"    obj.insert(Keys::{sub['jsonKey']}, {var});",
            "  }",
            "",
        ]
    while sub_body and sub_body[-1] == "":
        sub_body.pop()
    lines += documented_function(
        "static void writeDatasetSubEntities(QJsonObject& obj, const Dataset& d)",
        "Writes the nested alarm-band and frequency-marker collections when they are non-empty.",
        sub_body,
    )

    readable = [p for p in manifest["properties"] if p.get("readBack", True)]
    readable += [
        as_property(r)
        for r in manifest["runtimeFields"]
        if r.get("jsonKey") and r.get("readBack", True)
    ]
    read_groups = {
        "Flags": [p for p in readable if p["type"] == "bool"],
        "Numbers": [p for p in readable if p["type"] in ("int", "double")],
        "Strings": [p for p in readable if p["type"] == "string"],
    }
    for name, members in read_groups.items():
        body = [f"  d.{p['field']} = {read_expression(p)};" for p in members]
        lines += documented_function(
            f"static void readDataset{name}(Dataset& d, const QJsonObject& obj)",
            f"Reads the declared {name.lower()} of a dataset from its project-JSON object.",
            body,
        )

    post_body: list[str] = []
    for step in manifest.get("postRead", []):
        post_body += emit_post_read(step, manifest, props)
    while post_body and post_body[-1] == "":
        post_body.pop()
    lines += documented_function(
        "static void finalizeDatasetRead(Dataset& d, const QJsonObject& obj)",
        "Applies the declared post-read steps: colour validation, derived numeric value, FFT "
        "window clamp, legacy range fallbacks, nested entities and range normalization.",
        post_body,
    )

    lines += ["}  // namespace DataModel", ""]

    lines += documented_function(
        "QJsonObject DataModel::serialize(const Dataset& d)",
        "Serializes a Dataset to a QJsonObject.",
        [
            "  QJsonObject obj;",
            "  writeDatasetFlags(obj, d);",
            "  writeDatasetNumbers(obj, d);",
            "  writeDatasetStrings(obj, d);",
            "  writeDatasetSubEntities(obj, d);",
            "  return obj;",
        ],
    )

    lines += documented_function(
        "bool DataModel::read(Dataset& d, const QJsonObject& obj)",
        "Deserializes a Dataset from a QJsonObject.",
        [
            "  if (obj.isEmpty())",
            "    return false;",
            "",
            "  readDatasetFlags(d, obj);",
            "  readDatasetNumbers(d, obj);",
            "  readDatasetStrings(d, obj);",
            "  finalizeDatasetRead(d, obj);",
            "  return true;",
        ],
    )

    lines += ["// clang-format on"]
    return "\n".join(lines) + "\n"


def emit_post_read(step: str, manifest: dict, props: dict) -> list[str]:
    if step == "validateColor":
        out = []
        for prop in manifest["properties"]:
            if prop.get("validate") != "colorValid":
                continue
            name = field(prop)
            out += [
                f"  if (!PropertyHooks::isValidColor({name}))",
                f"    {name}.clear();",
                "",
            ]
        return out

    if step == "deriveNumericValue":
        return [
            "  if (!d.value.isEmpty())",
            "    d.numericValue = SerialStudio::toDouble(d.value, &d.isNumeric);",
            "",
        ]

    if step == "clampFftWindow":
        prop = props["FftWindow"]
        return [
            f"  if (!PropertyHooks::isValidFftWindow({field(prop)}))",
            f"    {field(prop)} = {literal(prop['default'])};",
            "",
        ]

    if step == "legacyRangePairs":
        out = []
        for prop in manifest["properties"]:
            if not prop.get("legacyPair") or prop.get("writeTransform") != "pairMin":
                continue
            partner = props[prop["pairWith"]]
            low_key, high_key = prop["legacyKeys"]
            out += [
                f"  if (!obj.contains({key(prop)}) || !obj.contains({key(partner)})) {{",
                f"    {field(prop)} = SerialStudio::toDouble(ss_jsr(obj, Keys::{low_key}, 0));",
                f"    {field(partner)} = SerialStudio::toDouble(ss_jsr(obj, Keys::{high_key}, 0));",
                "  }",
                "",
            ]
        return out

    if step in ("readAlarmBands", "readFrequencyMarkers"):
        hook = manifest["hooks"][step]["signature"]
        name = hook.split("DataModel::")[1].split("(")[0]
        return [f"  {name}(d, obj);"]

    if step == "normalizeRanges":
        return ["  normalizeDatasetRanges(d);"]

    raise SystemExit(f"generate-property-registry: unknown postRead step '{step}'")


# --------------------------------------------------------------------------------------------------
# DatasetForm.cpp
# --------------------------------------------------------------------------------------------------


def editable_expression(prop: dict, builder: dict) -> str:
    hook = prop.get("enabledWhen")
    if not hook:
        return "true"
    if builder.get("flagHook") == hook:
        return builder["flagName"]

    return f"PropertyHooks::{hook}(dataset, m_projectModelRef)"


def value_expression(
    prop: dict, manifest: dict, var: str = "dataset", model: str = "m_projectModelRef"
) -> str:
    if not is_choice(prop):
        return f"{var}.{prop['field']}"

    source = manifest["optionSources"][prop["options"]]
    accessor = f"Registry::{option_accessor(prop['options'])}()"
    if source["kind"] == "tuple":
        first, second = source["fields"]
        props = by_id(manifest)
        return (
            f"{accessor}.indexForPair({var}.{props[first]['field']}, "
            f"{var}.{props[second]['field']})"
        )

    return f"{accessor}.indexForValue({model}, {var}.{prop['field']})"


def emit_row(prop: dict, builder: dict, manifest: dict) -> list[str]:
    short = snake(prop["field"]).rstrip("_")
    var = f"item_{short}"
    indent = "    " if prop.get("visibleWhen") else "  "
    lines: list[str] = []

    if prop.get("visibleWhen"):
        lines.append(
            f"  if (PropertyHooks::{prop['visibleWhen']}(dataset, m_projectModelRef)) {{"
        )

    editable = editable_expression(prop, builder)
    if editable not in ("true", builder.get("flagName")):
        lines += emit_statement(indent, f"const bool on_{short} = {editable};")
        editable = f"on_{short}"

    value = value_expression(prop, manifest)
    if is_choice(prop):
        lines += emit_statement(indent, f"const auto val_{short} = {value};")
        value = f"val_{short}"

    lines.append(f"{indent}auto* {var} = new QStandardItem();")
    lines.append(f"{indent}{var}->setEditable({editable});")
    lines.append(f"{indent}{var}->setData({editable}, Active);")
    lines.append(f"{indent}{var}->setData({prop['widget']}, WidgetType);")
    lines.append(f"{indent}{var}->setData({value}, EditableValue);")
    lines.append(f"{indent}{var}->setData({prop['formId']}, ParameterType);")

    if is_choice(prop):
        accessor = f"Registry::{option_accessor(prop['options'])}()"
        lines += emit_statement(
            indent,
            f"{var}->setData({accessor}.labels(m_projectModelRef), ComboBoxData);",
        )

    if prop.get("placeholderHook"):
        hook = prop["placeholderHook"]
        lines += emit_statement(
            indent,
            f"const auto ph_{short} = PropertyHooks::{hook}(dataset, m_projectModelRef);",
        )
        lines.append(f"{indent}{var}->setData(ph_{short}, PlaceholderValue);")
    elif "placeholder" in prop:
        placeholder = prop["placeholder"]
        if isinstance(placeholder, str):
            lines += wrapped_call(
                f"{indent}{var}->setData(", placeholder, ", PlaceholderValue);", True
            )
        else:
            lines.append(
                f"{indent}{var}->setData({literal(placeholder)}, PlaceholderValue);"
            )

    if "minValue" in prop:
        lines.append(f"{indent}{var}->setData({literal(prop['minValue'])}, MinValue);")
    if "maxValue" in prop:
        lines.append(f"{indent}{var}->setData({literal(prop['maxValue'])}, MaxValue);")

    lines += wrapped_call(
        f"{indent}{var}->setData(", prop["label"], ", ParameterName);", True
    )
    lines += wrapped_call(
        f"{indent}{var}->setData(",
        prop["description"],
        ", ParameterDescription);",
        True,
    )
    lines.append(f"{indent}model->appendRow({var});")

    if prop.get("visibleWhen"):
        lines.append("  }")

    lines.append("")
    return lines


def emit_section_header(section: dict) -> list[str]:
    lines = [
        "  static auto& registry = Misc::IconRegistry::instance();",
        "  auto* header = new QStandardItem();",
        "  header->setData(SectionHeader, WidgetType);",
    ]
    lines += wrapped_call(
        "  header->setData(", section["title"], ", PlaceholderValue);", True
    )
    lines.append(
        f'  header->setData(registry.icon(QStringLiteral("{section["iconCategory"]}"),'
    )
    lines.append(
        f'                                QStringLiteral("{section["iconName"]}"), 16),'
    )
    lines.append("                  ParameterIcon);")
    lines.append("  model->appendRow(header);")
    lines.append("")
    return lines


def render_form(manifest: dict) -> str:
    props = by_id(manifest)
    sections = {s["id"]: s for s in manifest["sections"]}

    lines: list[str] = [LICENSE.rstrip(), "", BANNER.rstrip(), ""]
    lines += [
        '#include "DataModel/Generated/DatasetRegistry.h"',
        '#include "DataModel/ProjectEditor.h"',
        '#include "DataModel/ProjectModel.h"',
        '#include "Misc/IconRegistry.h"',
        "",
        "// clang-format off",
        "",
        "using DataModel::PropertyHooks::RebuildHint;",
        "namespace PropertyHooks = DataModel::PropertyHooks;",
        "namespace Registry      = DataModel::Registry;",
        "",
    ]

    lines += [BAR, "// Constants", BAR, ""]
    for name, source in manifest["optionSources"].items():
        lines += emit_option_table(name, source)

    for name, source in manifest["optionSources"].items():
        lines += emit_option_source(name, source, props)

    for builder in manifest["form"]["builders"]:
        lines += emit_builder(builder, manifest, sections)

    lines += emit_commit_dispatcher(manifest)
    lines += emit_form_value_reader(manifest)
    lines += ["// clang-format on"]
    return "\n".join(lines) + "\n"


def emit_form_value_reader(manifest: dict) -> list[str]:
    """Emit the row-value reader the multi-selection harvest uses instead of a throwaway model."""
    rows = form_rows(manifest)
    hidden = [p for p in rows if p.get("visibleWhen")]

    body = ["  switch (formId) {"]
    for prop in hidden:
        body.append(f"    case {prop['formId']}:")
        body += emit_statement(
            "      ", f"return PropertyHooks::{prop['visibleWhen']}(d, pm);"
        )
    body += ["    default:", "      return true;", "  }"]

    lines = [BAR, ""]
    lines += doxygen(
        "Reports whether a dataset form row is built at all, mirroring the row emitters' "
        "visibleWhen guard."
    )
    lines += [
        "static bool datasetRowVisible(int formId,",
        "                              const DataModel::Dataset& d,",
        "                              const DataModel::ProjectModel& pm)",
        "{",
    ]
    lines += body + ["}", ""]

    values = ["  switch (formId) {"]
    for prop in rows:
        values.append(f"    case {prop['formId']}:")
        values += emit_statement(
            "      ", f"return {value_expression(prop, manifest, 'd', 'pm')};"
        )
    values += ["    default:", "      return {};", "  }"]

    lines += [BAR, ""]
    lines += doxygen(
        "Returns the value a dataset form row would carry, or an invalid QVariant when the row "
        "is not built for this dataset."
    )
    lines += [
        "QVariant Registry::datasetFormValue(int formId,",
        "                                    const DataModel::Dataset& d,",
        "                                    const DataModel::ProjectModel& pm)",
        "{",
        "  if (!datasetRowVisible(formId, d, pm))",
        "    return {};",
        "",
    ]
    lines += values + ["}", ""]
    return lines


def option_table(name: str) -> str:
    return f"k{name[0].upper()}{name[1:]}Entries"


def emit_option_table(name: str, source: dict) -> list[str]:
    """Emit the static entry table backing one choice domain (never a live provider)."""
    kind = source["kind"]
    table = option_table(name)
    if kind == "liveProvider":
        return []

    entry_type = {
        "staticMap": "StaticOptionEntry",
        "extensibleMap": "StaticOptionEntry",
        "parallelValues": "IntOptionEntry",
        "tuple": "TupleOptionEntry",
    }[kind]

    lines = doxygen(f"Entry table backing the {name} choice domain.")
    lines.append(f"static const PropertyHooks::{entry_type} {table}[] = {{")
    for entry in source["entries"]:
        if kind == "tuple":
            first, second = entry["values"]
            lines.append(
                f'  {{{literal(first)}, {literal(second)}, {literal(entry["label"])}}},'
            )
        else:
            lines.append(f'  {{{literal(entry["value"])}, {literal(entry["label"])}}},')
    lines.append("};")
    lines.append("")
    return lines


def emit_option_source(name: str, source: dict, props: dict) -> list[str]:
    kind = source["kind"]
    table = option_table(name)
    context = source.get("translate")
    lines = [BAR, ""]

    if kind == "staticMap":
        lines += doxygen(f"Returns the shared option source for the {name} domain.")
        lines.append(
            f"const PropertyHooks::StaticMapOptions& Registry::{option_accessor(name)}()"
        )
        lines.append("{")
        lines += emit_statement(
            "  ",
            f"static const PropertyHooks::StaticMapOptions source(&{table}[0], "
            f'{len(source["entries"])}, "{context}");',
        )
        lines.append("  return source;")
        lines.append("}")
        lines.append("")
        return lines

    if kind == "extensibleMap":
        lines += doxygen(
            f"Returns the shared option source for the {name} domain, whose declared rows the "
            f"{source['hook']} provider extends at runtime."
        )
        lines.append(
            f"const PropertyHooks::ExtensibleMapOptions& Registry::{option_accessor(name)}()"
        )
        lines.append("{")
        lines += emit_statement(
            "  ",
            f"static const PropertyHooks::ExtensibleMapOptions source(&{table}[0], "
            f'{len(source["entries"])}, "{context}", '
            f"&PropertyHooks::{source['hook']});",
        )
        lines.append("  return source;")
        lines.append("}")
        lines.append("")
        return lines

    if kind == "parallelValues":
        lines += doxygen(f"Returns the shared option source for the {name} domain.")
        lines.append(
            f"const PropertyHooks::ParallelValueOptions& Registry::{option_accessor(name)}()"
        )
        lines.append("{")
        ctx = f'"{context}"' if context else "nullptr"
        lines += emit_statement(
            "  ",
            f"static const PropertyHooks::ParallelValueOptions source(&{table}[0], "
            f'{len(source["entries"])}, {ctx}, {source["notFoundIndex"]});',
        )
        lines.append("  return source;")
        lines.append("}")
        lines.append("")
        return lines

    if kind == "tuple":
        lines += doxygen(f"Returns the shared option source for the {name} domain.")
        lines.append(
            f"const PropertyHooks::TupleOptions& Registry::{option_accessor(name)}()"
        )
        lines.append("{")
        lines += emit_statement(
            "  ",
            f"static const PropertyHooks::TupleOptions source(&{table}[0], "
            f'{len(source["entries"])}, "{context}");',
        )
        lines.append("  return source;")
        lines.append("}")
        lines.append("")
        return lines

    lines += doxygen(f"Returns the shared option source for the {name} domain.")
    lines.append(
        f"const PropertyHooks::LiveProviderOptions& Registry::{option_accessor(name)}()"
    )
    lines.append("{")
    lines.append("  static const PropertyHooks::LiveProviderOptions source(")
    lines.append(f'    &DataModel::ProjectModel::{source["labelProvider"]},')
    lines.append(f'    &DataModel::ProjectModel::{source["valueProvider"]},')
    lines.append(f'    {source["notFoundValue"]});')
    lines.append("  return source;")
    lines.append("}")
    lines.append("")
    return lines


def emit_builder(builder: dict, manifest: dict, sections: dict) -> list[str]:
    props = by_id(manifest)
    name = builder["function"]
    if builder["signature"] == "model+dataset":
        signature = f"void DataModel::ProjectEditor::{name}(CustomModel* model, const Dataset& dataset)"
    else:
        signature = (
            f"void DataModel::ProjectEditor::{name}(CustomModel* model,\n"
            f"{' ' * (len('void DataModel::ProjectEditor::') + len(name) + 1)}const Dataset& dataset,\n"
            f"{' ' * (len('void DataModel::ProjectEditor::') + len(name) + 1)}bool {builder['flagName']})"
        )

    body: list[str] = []
    if builder.get("section"):
        body += emit_section_header(sections[builder["section"]])

    for row in builder["rows"]:
        body += emit_row(props[row], builder, manifest)

    for call in builder.get("calls", []):
        target = next(b for b in manifest["form"]["builders"] if b["function"] == call)
        if target["signature"] == "model+dataset":
            body.append(f"  {call}(model, dataset);")
        else:
            hook = target["flagHook"]
            body += emit_statement(
                "  ",
                f"{call}(model, dataset, PropertyHooks::{hook}(dataset, m_projectModelRef));",
            )
        body.append("")

    while body and body[-1] == "":
        body.pop()

    brief = f"Appends the {name} rows of the dataset form model."
    if builder.get("section"):
        brief = (
            f"Appends the {sections[builder['section']]['title']} section rows to the dataset "
            "form model."
        )

    return [BAR, ""] + doxygen(brief) + [signature, "{"] + body + ["}", ""]


def commit_expression(prop: dict, manifest: dict) -> str:
    if is_choice(prop):
        source = manifest["optionSources"][prop["options"]]
        accessor = f"Registry::{option_accessor(prop['options'])}()"
        if source["valueType"] == "string":
            return f"{accessor}.valueForIndex(pm, value.toInt()).toString()"
        return f"{accessor}.valueForIndex(pm, value.toInt()).toInt()"

    clamp = prop.get("clamp", {}).get("editor", {})
    if prop["type"] == "bool":
        return "value.toBool()"
    if prop["type"] == "string":
        suffix = ".simplified()" if prop.get("commitTransform") == "simplified" else ""
        return f"value.toString(){suffix}"
    if prop["type"] == "double":
        return "SerialStudio::toDouble(value)"

    if "min" in clamp and "max" in clamp:
        return (
            f"qBound({literal(clamp['min'])}, value.toInt(), {literal(clamp['max'])})"
        )
    if "min" in clamp:
        return f"qMax({literal(clamp['min'])}, value.toInt())"

    return "value.toInt()"


def emit_commit_dispatcher(manifest: dict) -> list[str]:
    props = by_id(manifest)
    rows = [
        p for p in manifest["properties"] if p.get("formId") and not p.get("deadFormId")
    ]

    families = {
        "String": [p for p in rows if p["type"] == "string" and not is_choice(p)],
        "Choice": [p for p in rows if is_choice(p)],
        "Number": [
            p for p in rows if p["type"] in ("int", "double") and not is_choice(p)
        ],
        "Flag": [p for p in rows if p["type"] == "bool" and not is_choice(p)],
    }

    lines: list[str] = []
    for family, members in families.items():
        body = ["  switch (formId) {"]
        for prop in members:
            body.append(f"    case {prop['formId']}:")
            if prop.get("tupleFields"):
                source = manifest["optionSources"][prop["options"]]
                accessor = f"Registry::{option_accessor(prop['options'])}()"
                first, second = source["fields"]
                body.append(
                    f"      d.{props[first]['field']} = {accessor}.firstForIndex(value.toInt());"
                )
                body.append(
                    f"      d.{props[second]['field']} = {accessor}.secondForIndex(value.toInt());"
                )
            else:
                body += emit_statement(
                    "      ",
                    f"d.{prop['field']} = {commit_expression(prop, manifest)};",
                )
            body.append("      return true;")
        body.append("    default:")
        body.append("      return false;")
        body.append("  }")

        head = f"static bool applyDataset{family}Edit("
        params = ["int formId", "const QVariant& value", "DataModel::Dataset& d"]
        if family == "Choice":
            params.append("const DataModel::ProjectModel& pm")

        single = head + ", ".join(params) + ")"
        if len(single) <= MAX_COLS:
            signature = [single]
        else:
            signature = [head + params[0] + ","]
            for param in params[1:-1]:
                signature.append(" " * len(head) + param + ",")
            signature.append(" " * len(head) + params[-1] + ")")

        lines += [BAR, ""]
        lines += doxygen(
            f"Applies a {family.lower()}-typed dataset form edit; returns false when the id is not "
            f"a {family.lower()} row."
        )
        lines += signature + ["{"] + body + ["}", ""]

    hook_rows = [
        p for p in rows if p.get("onCommit") and p["onCommit"] != "onAliasRejected"
    ]
    body = ["  switch (formId) {"]
    for prop in hook_rows:
        body.append(f"    case {prop['formId']}:")
        body.append(f"      return PropertyHooks::{prop['onCommit']}(d);")
    body.append("    default:")
    body.append("      return RebuildHint::None;")
    body.append("  }")
    lines += [BAR, ""]
    lines += doxygen(
        "Runs the declared commit-side-effect hook for a form id, if it declares one."
    )
    lines += [
        "static RebuildHint datasetCommitHook(int formId, DataModel::Dataset& d)",
        "{",
    ]
    lines += body + ["}", ""]

    lines += [BAR, ""]
    lines += doxygen(
        "Applies one dataset form edit onto d and reports the rebuild the caller must run."
    )
    lines += [
        "RebuildHint Registry::applyDatasetFormEdit(int formId,",
        "                                          const QVariant& value,",
        "                                          DataModel::Dataset& d,",
        "                                          const DataModel::ProjectModel& pm)",
        "{",
        "  const bool handled = applyDatasetStringEdit(formId, value, d)",
        "                       || applyDatasetChoiceEdit(formId, value, d, pm)",
        "                       || applyDatasetNumberEdit(formId, value, d)",
        "                       || applyDatasetFlagEdit(formId, value, d);",
        "  if (!handled)",
        "    return RebuildHint::None;",
        "",
        "  return datasetCommitHook(formId, d);",
        "}",
        "",
    ]
    return lines


# --------------------------------------------------------------------------------------------------
# DatasetApiFields.cpp
# --------------------------------------------------------------------------------------------------


def api_names(prop: dict, manifest: dict) -> list[str]:
    descriptor = schema_props_for(prop, manifest)[0]
    return [descriptor["name"]] + descriptor["aliases"]


def api_value_expression(prop: dict, key_var: str) -> str:
    clamp = prop.get("clamp", {}).get("api", {})
    value = f"params.value({key_var})"
    if prop["type"] == "bool":
        return f"{value}.toBool()"
    if prop["type"] == "string":
        suffix = ".simplified()" if prop.get("commitTransform") == "simplified" else ""
        return f"{value}.toString(){suffix}"
    if prop["type"] == "double":
        return f"SerialStudio::toDouble({value})"

    if "min" in clamp and "max" in clamp:
        return (
            f"qBound({literal(clamp['min'])}, {value}.toInt(), {literal(clamp['max'])})"
        )
    if "min" in clamp:
        return f"qMax({literal(clamp['min'])}, {value}.toInt())"

    return f"{value}.toInt()"


def emit_api_field(prop: dict, manifest: dict) -> list[str]:
    key_var = f"key_{snake(prop['field']).rstrip('_')}"
    names = ", ".join(json_name(n) for n in api_names(prop, manifest))
    lines = [
        f"  const auto {key_var} = takeDatasetField(params, consumed, {{{names}}});"
    ]
    if len(lines[0]) > MAX_COLS:
        lines = [f"  const auto {key_var} = takeDatasetField("]
        lines.append("    params, consumed,")
        lines.append(f"    {{{names}}});")

    validate = prop.get("validate")
    rebuild = prop.get("api", {}).get("rebuildTree", False)

    if not validate:
        if rebuild:
            lines += [
                f"  if (!{key_var}.isEmpty()) {{",
                f"    d.{prop['field']} = {api_value_expression(prop, key_var)};",
                "    rebuildTree = true;",
                "  }",
                "",
            ]
        else:
            lines += [
                f"  if (!{key_var}.isEmpty())",
                f"    d.{prop['field']} = {api_value_expression(prop, key_var)};",
                "",
            ]
        return lines

    lines.append(f"  if (!{key_var}.isEmpty()) {{")
    lines += emit_statement(
        "    ", f"const auto candidate = {api_value_expression(prop, key_var)};"
    )
    lines += api_rejects(prop)
    suffix = ").arg(candidate);" if "%1" in prop["apiError"] else ");"
    lines += wrapped_call(
        "      return QStringLiteral(", prop["apiError"], suffix, False
    )
    lines.append("")
    lines.append(f"    d.{prop['field']} = candidate;")
    if rebuild:
        lines.append("    rebuildTree = true;")
    lines.append("  }")
    lines.append("")
    return lines


def api_rejects(prop: dict) -> list[str]:
    """Return the lines of the 'value is invalid' condition for a declared validator."""
    validate = prop["validate"]
    if validate == "aliasUnique":
        return [
            "    if (!candidate.isEmpty()",
            "        && PropertyHooks::aliasInUseByOtherDataset(project, candidate, d.uniqueId))",
        ]

    return [f"    if (!PropertyHooks::{validator_function(validate)}(candidate))"]


def validator_function(validate: str) -> str:
    return {
        "colorValid": "isValidColor",
        "indexNonNegative": "isValidDatasetIndex",
        "fftWindowRange": "isValidFftWindow",
        "transformLanguageDomain": "isValidTransformLanguage",
    }[validate]


def render_api(manifest: dict) -> str:
    exposed = [p for p in manifest["properties"] if p.get("api", {}).get("expose")]

    families = {
        "String": [p for p in exposed if p["type"] == "string"],
        "Number": [p for p in exposed if p["type"] in ("int", "double")],
        "Flag": [p for p in exposed if p["type"] == "bool"],
    }

    lines: list[str] = [LICENSE.rstrip(), "", BANNER.rstrip(), ""]
    lines += [
        "#include <optional>",
        "#include <QJsonArray>",
        "",
        '#include "API/Handlers/ProjectHandler.h"',
        '#include "DataModel/Project/PropertyHooks.h"',
        '#include "DataModel/ProjectModel.h"',
        '#include "SerialStudio.h"',
        "",
        "// clang-format off",
        "",
        "namespace PropertyHooks = DataModel::PropertyHooks;",
        "",
        "namespace API::Handlers {",
        "",
    ]

    lines += doxygen(
        "Returns the first present spelling of a declared field and records every present "
        "spelling as consumed, so the unknown-field warning cannot be forgotten."
    )
    lines += [
        "static QString takeDatasetField(const QJsonObject& params,",
        "                                QSet<QString>& consumed,",
        "                                const QStringList& names)",
        "{",
        "  QString found;",
        "  for (const auto& name : names) {",
        "    if (!params.contains(name))",
        "      continue;",
        "",
        "    consumed.insert(name);",
        "    if (found.isEmpty())",
        "      found = name;",
        "  }",
        "",
        "  return found;",
        "}",
        "",
    ]

    appliers: list[str] = []
    for family, members in families.items():
        chunks = [members[i : i + 6] for i in range(0, len(members), 6)]
        for index, chunk in enumerate(chunks, start=1):
            name = f"applyDataset{family}Fields{index}"
            appliers.append(name)
            body = []
            if not any(p.get("api", {}).get("rebuildTree", False) for p in chunk):
                body += ["  Q_UNUSED(rebuildTree);", ""]
            if any(p.get("validate") == "aliasUnique" for p in chunk):
                body += [
                    "  static auto& project = DataModel::ProjectModel::instance();",
                    "",
                ]

            for prop in chunk:
                body += emit_api_field(prop, manifest)

            body.append("  return QString();")
            pad = " " * len(f"static QString {name}(")
            lines += [BAR, ""]
            lines += doxygen(
                f"Applies declared {family.lower()} fields onto d (part {index} of {len(chunks)}); "
                "returns a non-empty error string when a value is rejected."
            )
            lines += (
                [
                    f"static QString {name}(DataModel::Dataset& d,",
                    f"{pad}const QJsonObject& params,",
                    f"{pad}bool& rebuildTree,",
                    f"{pad}QSet<QString>& consumed)",
                    "{",
                ]
                + body
                + ["}", ""]
            )

    lines += emit_api_sub_entities(manifest)
    lines += ["}  // namespace API::Handlers", ""]

    lines += [BAR, ""]
    lines += doxygen(
        "Patches dataset fields from a generic params object; returns an error string on failure."
    )
    lines += [
        "QString API::Handlers::ProjectHandler::applyDatasetUpdateParams(DataModel::Dataset& d,",
        "                                                                const QJsonObject& params,",
        "                                                                bool& rebuildTree,",
        "                                                                QSet<QString>& consumed)",
        "{",
    ]
    for name in appliers:
        lines += [
            f"  if (auto err = {name}(d, params, rebuildTree, consumed); !err.isEmpty())",
            "    return err;",
            "",
        ]
    lines += [
        "  applyDatasetSubEntityFields(d, params, consumed);",
        "  return QString();",
        "}",
        "",
    ]

    lines += emit_api_schema(manifest, exposed)
    lines += ["// clang-format on"]
    return "\n".join(lines) + "\n"


def emit_api_sub_entities(manifest: dict) -> list[str]:
    lines = [BAR, ""]
    lines += doxygen(
        "Replaces the nested alarm-band and frequency-marker collections from their declared "
        "field names, keeping the v3.3 alarmLow/alarmHigh/alarmEnabled inputs alive."
    )
    lines += [
        "static void applyDatasetSubEntityFields(DataModel::Dataset& d,",
        "                                        const QJsonObject& params,",
        "                                        QSet<QString>& consumed)",
        "{",
    ]
    for sub in manifest["subEntities"]:
        var = f"key_{snake(sub['field'])}"
        struct = (
            "DataModel::AlarmBand"
            if sub["field"] == "alarmBands"
            else "DataModel::FrequencyMarker"
        )
        legacy = sub.get("legacy")
        lines += emit_statement(
            "  ",
            f"const auto {var} = takeDatasetField(params, consumed,"
            f" {{{json_name(sub['apiName'])}}});",
        )
        if legacy:
            for arg in legacy["args"]:
                key_var = f"key_{snake(arg['name'])}"
                lines += emit_statement(
                    "  ",
                    f"const auto {key_var} = takeDatasetField(params, consumed,"
                    f" {{{json_name(arg['name'])}}});",
                )

        lines += [
            f"  if (!{var}.isEmpty()) {{",
            f"    d.{sub['field']}.clear();",
            f"    const auto entries = params.value({var}).toArray();",
            f"    d.{sub['field']}.reserve(entries.size());",
            "    for (const auto& entry : entries) {",
            f"      {struct} parsed;",
            "      if (DataModel::read(parsed, entry.toObject()))",
            f"        d.{sub['field']}.push_back(std::move(parsed));",
            "    }",
            "  }",
        ]

        if legacy:
            names = [f"key_{snake(a['name'])}" for a in legacy["args"]]
            condition = " || ".join(f"!{n}.isEmpty()" for n in names)
            lines += emit_statement("  ", f"else if ({condition}) {{")
            args = []
            for arg, key_var in zip(legacy["args"], names):
                local = snake(arg["name"]).replace("alarm_", "")
                ctype = {"bool": "bool", "double": "double", "int": "int"}[arg["type"]]
                reader = {
                    "bool": f"params.value({key_var}).toBool()",
                    "double": f"SerialStudio::toDouble(params.value({key_var}))",
                    "int": f"params.value({key_var}).toInt()",
                }[arg["type"]]
                lines.append(f"    std::optional<{ctype}> {local};")
                lines.append(f"    if (!{key_var}.isEmpty())")
                lines += emit_statement("      ", f"{local} = {reader};")
                lines.append("")
                args.append(local)

            lines += emit_statement("    ", f"{legacy['hook']}(d, {', '.join(args)});")
            lines.append("  }")

        lines.append("")

    while lines and lines[-1] == "":
        lines.pop()

    lines += [
        "}",
        "",
    ]
    return lines


def emit_api_schema(manifest: dict, exposed: list[dict]) -> list[str]:
    lines = [BAR, ""]
    lines += doxygen("Builds one typed schema property entry.")
    lines += [
        "static QJsonObject datasetSchemaProperty(const char* type,",
        "                                         const char* description,",
        "                                         const QJsonArray& domain)",
        "{",
        "  QJsonObject prop;",
        '  prop.insert(QStringLiteral("type"), QString::fromUtf8(type));',
        '  prop.insert(QStringLiteral("description"), QString::fromUtf8(description));',
        "  if (!domain.isEmpty())",
        '    prop.insert(QStringLiteral("enum"), domain);',
        "",
        "  return prop;",
        "}",
        "",
    ]

    chunks = [exposed[i : i + 12] for i in range(0, len(exposed), 12)]
    for index, chunk in enumerate(chunks, start=1):
        lines += [BAR, ""]
        lines += doxygen(
            f"Declares dataset schema properties, part {index} of {len(chunks)}."
        )
        lines += [
            f"static void datasetSchemaPart{index}(QJsonObject& props)",
            "{",
        ]
        for prop in chunk:
            lines += emit_schema_entry(prop, manifest)
        lines += ["}", ""]

    lines += [BAR, ""]
    lines += doxygen(
        "Returns the typed schema properties for the dataset verbs, replacing the prose field "
        "enumeration the API used to publish."
    )
    lines += [
        "QJsonObject API::Handlers::datasetFieldSchema()",
        "{",
        "  QJsonObject props;",
    ]
    for index in range(1, len(chunks) + 1):
        lines.append(f"  datasetSchemaPart{index}(props);")
    lines += ["  return props;", "}", ""]
    return lines


def emit_schema_entry(prop: dict, manifest: dict) -> list[str]:
    descriptor = schema_props_for(prop, manifest)[0]
    domain = "QJsonArray()"
    if descriptor["enum"]:
        values = ", ".join(literal(v) for v in descriptor["enum"])
        domain = f"QJsonArray({{{values}}})"

    lines = [f"  props.insert({json_name(descriptor['name'])},"]
    lines.append(f'               datasetSchemaProperty("{descriptor["type"]}",')
    lines += wrapped_call(
        "                                     ", descriptor["description"], ",", False
    )
    lines += emit_statement("                                     ", f"{domain}));")
    return lines


# --------------------------------------------------------------------------------------------------
# gRPC field-number ledger + typed proto (spec 0037 T4, T6)
# --------------------------------------------------------------------------------------------------

LEDGER_MARKER = (
    "AUTO-GENERATED from app/rcc/api/api-schema.json by "
    "scripts/generate-property-registry.py; never edit by hand."
)

LEDGER_NOTE = (
    "gRPC field numbers are append-only released state. A parameter keeps its number "
    "forever, a parameter that leaves the API moves its number into 'reserved' and is never "
    "reassigned, and 1 is reserved in every message for the request id."
)

PROTO_HEADER = (
    "// Auto-generated by Serial Studio ProtoGenerator\n"
    "// Do not edit manually -- regenerate from Settings > Export .proto\n"
    "\n"
    'syntax = "proto3";\n'
    "\n"
    "package serialstudio.typed;\n"
    "\n"
    'import "google/protobuf/struct.proto";\n'
    'import "google/protobuf/empty.proto";\n'
    "\n"
)

PROTO_SHARED = (
    "// Common response used by all command RPCs\n"
    "message CommandResponse {\n"
    "  string id = 1;\n"
    "  bool success = 2;\n"
    "  google.protobuf.Value result = 3;\n"
    "  string error_code = 4;\n"
    "  string error_message = 5;\n"
    "}\n"
    "\n"
    "// Streaming messages\n"
    "message StreamRequest {}\n"
    "\n"
    "message FrameData {\n"
    "  int64 timestamp_ms = 1;\n"
    "  google.protobuf.Struct frame = 2;\n"
    "}\n"
    "\n"
    "message RawData {\n"
    "  bytes data = 1;\n"
    "  int64 timestamp_ms = 2;\n"
    "}\n"
    "\n"
    "message RawDataRequest {\n"
    "  string id = 1;\n"
    "  bytes data = 2;\n"
    "}\n"
    "\n"
    "message CommandInfo {\n"
    "  string name = 1;\n"
    "  string description = 2;\n"
    "  google.protobuf.Struct input_schema = 3;\n"
    "}\n"
    "\n"
    "message CommandList {\n"
    "  repeated CommandInfo commands = 1;\n"
    "}\n"
    "\n"
)

PROTO_STREAMING = (
    "  // Streaming RPCs\n"
    "  rpc StreamFrames(StreamRequest) returns (stream FrameData);\n"
    "  rpc StreamRawData(StreamRequest) returns (stream RawData);\n"
    "  rpc WriteRawData(RawDataRequest) returns (CommandResponse);\n"
    "  rpc ListCommands(google.protobuf.Empty) returns (CommandList);\n"
    "}\n"
)


def load_api_schema() -> list:
    """Return the committed API snapshot, or an empty list when it cannot be read."""
    if not API_SCHEMA.exists():
        return []
    try:
        data = json.loads(API_SCHEMA.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return []

    return data if isinstance(data, list) else []


def load_ledger() -> dict:
    """Return the committed per-command numbering, or an empty map when absent."""
    if not LEDGER.exists():
        return {}
    try:
        data = json.loads(LEDGER.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return {}

    entries = data.get("commands", {})
    return entries if isinstance(entries, dict) else {}


def ledger_entries(commands: list, existing: dict) -> dict:
    """Assign field numbers append-only: keep every number, append new ones, retire removals."""
    params_by_command = {c["name"]: sorted(c.get("properties", {})) for c in commands}
    out: dict = {}

    for name in sorted(set(params_by_command) | set(existing)):
        prior = existing.get(name, {})
        fields = {k: int(v) for k, v in dict(prior.get("fields", {})).items()}
        reserved = {int(n) for n in prior.get("reserved", [])}
        nxt = max(int(prior.get("next", 2)), 2)

        if name in params_by_command:
            params = params_by_command[name]
            for param in sorted(set(fields) - set(params)):
                reserved.add(fields.pop(param))
            for param in params:
                if param in fields:
                    continue
                while nxt in reserved or nxt in fields.values():
                    nxt += 1
                fields[param] = nxt
                nxt += 1

        highest = max([nxt - 1, *fields.values(), *reserved], default=1)
        out[name] = {
            "fields": dict(sorted(fields.items())),
            "reserved": sorted(reserved),
            "next": max(nxt, highest + 1),
        }

    return out


def render_ledger(entries: dict) -> str:
    payload = {
        "_generated": LEDGER_MARKER,
        "_note": LEDGER_NOTE,
        "commands": entries,
    }
    return json.dumps(payload, indent=2) + "\n"


def sanitize_command_name(name: str) -> str:
    """Mirror ProtoGenerator::sanitizeName: dot notation to a CamelCase proto identifier."""
    out: list[str] = []
    capitalize = True
    for char in name:
        if char in "._-":
            capitalize = True
            continue
        out.append(char.upper() if capitalize else char)
        capitalize = False

    return "".join(out)


def proto_type(json_type) -> str:
    """Mirror ProtoGenerator::jsonTypeToProtoType, including its non-string fallthrough."""
    if json_type == "string":
        return "string"
    if json_type == "number":
        return "double"
    if json_type == "integer":
        return "int64"
    if json_type == "boolean":
        return "bool"
    if json_type == "array":
        return "google.protobuf.ListValue"

    return "google.protobuf.Struct"


def proto_field_name(param: str) -> str:
    """Rename a parameter that would collide with the fixed 'string id = 1' request field."""
    return "id_param" if param == "id" else param


def proto_scalar(value) -> str:
    """Render one enum value exactly as ProtoGenerator::jsonScalar does."""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, str):
        return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'
    if isinstance(value, (int, float)):
        number = float(value)
        return str(int(number)) if number.is_integer() else str(value)

    return "null"


def proto_field_comment(param: str, schema: dict) -> str:
    parts: list[str] = []
    if proto_field_name(param) != param:
        parts.append(f"JSON name: {param}")

    domain = schema.get("enum")
    if isinstance(domain, list) and domain:
        parts.append("enum: " + ", ".join(proto_scalar(v) for v in domain))

    return "  // " + "; ".join(parts) if parts else ""


def proto_comment_block(text: str) -> str:
    """Fold a multi-line command description into a comment protoc can parse."""
    return "  // " + text.replace("\r\n", "\n").replace("\n", "\n  // ")


def numbered_fields(props: dict, entry: dict) -> dict[int, str]:
    """Mirror ProtoGenerator::numberedFields, including its append-after-max fallback."""
    numbers = entry.get("fields", {})
    reserved = entry.get("reserved", [])

    nxt = max([1, *(int(n) for n in reserved), *(int(n) for n in numbers.values())])
    assigned: dict[int, str] = {}
    pending: list[str] = []
    for param in sorted(props):
        number = int(numbers.get(param, 0))
        if number > 1:
            assigned[number] = param
        else:
            pending.append(param)

    for param in pending:
        nxt += 1
        assigned[nxt] = param

    return dict(sorted(assigned.items()))


def render_message(command: dict, entry: dict) -> str:
    name = sanitize_command_name(command["name"]) + "Request"
    props = command.get("properties", {})
    reserved = entry.get("reserved", [])

    lines = [f"message {name} {{"]
    if reserved:
        lines.append("  reserved " + ", ".join(str(n) for n in reserved) + ";")

    lines.append("  string id = 1;")
    for number, param in numbered_fields(props, entry).items():
        schema = props[param] if isinstance(props[param], dict) else {}
        lines.append(
            f"  {proto_type(schema.get('type'))} {proto_field_name(param)} = {number};"
            f"{proto_field_comment(param, schema)}"
        )

    lines.append("}")
    return "\n".join(lines) + "\n"


def render_typed_proto(commands: list, ledger: dict) -> str:
    ordered = sorted(commands, key=lambda c: c["name"])
    out = [PROTO_HEADER, PROTO_SHARED, "// Per-command request messages\n\n"]

    for command in ordered:
        out.append(render_message(command, ledger.get(command["name"], {})) + "\n")

    out.append(
        "\n// Typed service with per-command RPCs\nservice SerialStudioTypedAPI {\n"
    )
    for command in ordered:
        rpc = sanitize_command_name(command["name"])
        out.append(
            f"{proto_comment_block(command.get('description', ''))}\n"
            f"  rpc {rpc}({rpc}Request) returns (CommandResponse);\n\n"
        )

    out.append(PROTO_STREAMING)
    return "".join(out)


# --------------------------------------------------------------------------------------------------
# api-schema.json projection (spec 0037 T2)
# --------------------------------------------------------------------------------------------------


def lower_descriptor(descriptor: dict) -> dict:
    """Lower one descriptor the way API::schemaPropToJson does.

    Only the keys the generated C++ actually registers are emitted: datasetSchemaProperty()
    takes type, description and the enum domain, so minimum/maximum/default stay out of both
    renderings and the two cannot drift apart.
    """
    prop: dict = {}
    kind = descriptor["type"]
    if "|" in kind:
        prop["type"] = [part.strip() for part in kind.split("|") if part.strip()]
    else:
        prop["type"] = kind

    prop["description"] = descriptor["description"]
    if descriptor.get("binary"):
        prop["binary"] = True
    if descriptor["enum"]:
        prop["enum"] = list(descriptor["enum"])

    return prop


def projected_properties(manifest: dict) -> dict[str, dict]:
    """Return the properties block the generated C++ registers for the dataset verbs."""
    return {d["name"]: lower_descriptor(d) for d in schema_descriptors(manifest)}


def snapshot_entry(commands: list, name: str) -> dict | None:
    for entry in commands:
        if entry.get("name") == name:
            return entry
    return None


def snapshot_fix_lines() -> list[str]:
    return [
        "  ordered fix:",
        "    1. build Serial Studio (a commercial build; a GPL dump omits Pro namespaces)",
        f"    2. SerialStudio --dump-api-schema {API_SCHEMA.relative_to(ROOT).as_posix()}",
        "    3. python3 scripts/sanitize-commit.py",
        "    4. commit the refreshed snapshot together with the manifest change",
        "  scope: only the registry-derived dataset verbs are covered; the other ~340 commands",
        "         register hand-written C++ schemas that no buildless check can project.",
    ]


def check_snapshot(manifest: dict, strict: bool) -> int:
    """Compare the manifest's projected dataset schema against the committed API snapshot."""
    if not API_SCHEMA.exists():
        print(
            f"generate-property-registry: cannot check the snapshot -- "
            f"{API_SCHEMA.relative_to(ROOT).as_posix()} is missing"
        )
        return 1

    try:
        commands = json.loads(API_SCHEMA.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        print(f"generate-property-registry: cannot parse the API snapshot -- {error}")
        return 1

    entry = snapshot_entry(commands, SNAPSHOT_COMMAND)
    if entry is None:
        print(
            f"generate-property-registry: cannot check the snapshot -- "
            f"{SNAPSHOT_COMMAND} is absent from it"
        )
        return 1

    expected = projected_properties(manifest)
    actual = entry.get("properties", {})
    required = entry.get("required", [])
    problems: list[str] = []

    for name, prop in expected.items():
        if name not in actual:
            problems.append(
                f"{SNAPSHOT_COMMAND}: declared field '{name}' is not in the snapshot"
            )
        elif actual[name] != prop:
            problems.append(
                f"{SNAPSHOT_COMMAND}: field '{name}' differs -- manifest projects "
                f"{json.dumps(prop, sort_keys=True)}, snapshot has "
                f"{json.dumps(actual[name], sort_keys=True)}"
            )
        if name in required:
            problems.append(
                f"{SNAPSHOT_COMMAND}: declared field '{name}' is marked required; "
                "dataset fields are optional patch inputs"
            )

    for name in sorted(actual):
        if name not in expected and name not in IDENTITY_PARAMS:
            problems.append(
                f"{SNAPSHOT_COMMAND}: snapshot field '{name}' is not declared in "
                f"{MANIFEST.relative_to(ROOT).as_posix()}"
            )

    for name in IDENTITY_PARAMS:
        if name not in actual:
            problems.append(f"{SNAPSHOT_COMMAND}: identity param '{name}' is missing")

    if not problems:
        print(
            f"generate-property-registry: api-schema.json matches the manifest "
            f"({len(expected)} dataset fields)"
        )
        return 0

    label = "FAIL" if strict else "WARNING"
    print(f"generate-property-registry: {label} -- api-schema.json is out of date")
    for problem in problems[:MAX_REPORTED]:
        print(f"  {problem}")
    if len(problems) > MAX_REPORTED:
        print(f"  ... and {len(problems) - MAX_REPORTED} more")
    for line in snapshot_fix_lines():
        print(line)

    if not strict:
        print(
            "  this is a warning locally (the snapshot needs a build to refresh) and a hard "
            "failure in CI"
        )
        return 0

    return 1


# --------------------------------------------------------------------------------------------------
# Entry point
# --------------------------------------------------------------------------------------------------


def render_all(manifest: dict) -> dict[Path, str]:
    rendered = {
        OUT_REGISTRY: render_registry(manifest),
        OUT_SERIAL: render_serialization(manifest),
        OUT_FORM: render_form(manifest),
        OUT_API: render_api(manifest),
    }

    for path, content in rendered.items():
        for number, line in enumerate(content.splitlines(), start=1):
            if len(line) > MAX_COLS:
                raise SystemExit(
                    f"generate-property-registry: {path.name}:{number} exceeds "
                    f"{MAX_COLS} columns ({len(line)})"
                )

            if line.lstrip().startswith(")"):
                raise SystemExit(
                    f"generate-property-registry: {path.name}:{number} starts with a stray "
                    "closing paren -- a wrapper emitted unbalanced text"
                )

        if content.count("(") != content.count(")"):
            raise SystemExit(
                f"generate-property-registry: {path.name} has unbalanced parentheses"
            )

    return rendered


def render_grpc_artifacts() -> dict[Path, str]:
    """Render the field-number ledger and the typed proto from the committed API snapshot."""
    commands = load_api_schema()
    if not commands:
        print(
            "generate-property-registry: skipping the gRPC ledger and typed proto -- "
            f"{API_SCHEMA.relative_to(ROOT).as_posix()} is missing or unreadable"
        )
        return {}

    entries = ledger_entries(commands, load_ledger())
    return {
        LEDGER: render_ledger(entries),
        TYPED_PROTO: render_typed_proto(commands, entries),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--check-snapshot", action="store_true")
    parser.add_argument("--strict", action="store_true")
    args = parser.parse_args()

    manifest = load_manifest()

    if args.check_snapshot:
        strict = args.strict or bool(os.environ.get("CI"))
        return check_snapshot(manifest, strict)

    rendered = render_all(manifest)
    rendered.update(render_grpc_artifacts())

    if args.check:
        stale = []
        for path, content in rendered.items():
            current = path.read_text(encoding="utf-8") if path.exists() else ""
            if current != content:
                stale.append(path.relative_to(ROOT).as_posix())

        if stale:
            print(
                "generate-property-registry: out of date -- regenerate with "
                "python3 scripts/generate-property-registry.py: " + ", ".join(stale)
            )
            return 1

        print("generate-property-registry: up to date")
        return 0

    for path, content in rendered.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8", newline="")

    props = len(manifest["properties"])
    total = sum(len(c.splitlines()) for c in rendered.values())
    print(
        f"generate-property-registry: wrote {len(rendered)} files "
        f"({total} lines) from {props} declared properties"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
