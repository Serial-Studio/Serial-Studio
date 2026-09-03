"""Static checks over the bundled widget extension packages (spec 0038, T24).

Pure Python: no Qt, no Node.js, no running Serial Studio. Everything here reads a
repository file or drives a scripts/registry-verify.py function directly, so the
package contract -- schema shape, the reserved-id pairing that carries R10, qrc sync,
and the trust wording the spec forbids softening -- is gated at lint time.
"""

import importlib.util
import json
import re
import shutil
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
VERIFIER = ROOT / "scripts" / "registry-verify.py"
SCHEMA = ROOT / "app" / "rcc" / "extensions" / "schema" / "widget-manifest.json"
PACKAGES = ROOT / "app" / "rcc" / "extensions" / "widget"
QRC = ROOT / "app" / "rcc" / "rcc.qrc"

# The spec forbids describing extension widgets as contained in any way; these words are legal
# only inside a denial, which is why the scan looks at the sentence around each hit.
FORBIDDEN = re.compile(
    r"\b(sandbox\w*|isolat\w*|secure\w*|safe|safely)\b", re.IGNORECASE
)
DENIALS = (
    "no ",
    "not ",
    "never",
    "nothing",
    "cannot",
    "can't",
    "without",
    "neither",
    "none",
    "forbid",
    "must not",
    "does not",
    "is not",
    "are not",
)

# Files that make up the spec-0038 surface, wherever a promise about the trust model could hide.
SPEC_FILES = (
    "app/src/UI/WidgetExtensions.h",
    "app/src/UI/WidgetExtensions.cpp",
    "app/src/UI/WidgetExtensionManifest.cpp",
    "app/src/UI/Widgets/ExtensionData.h",
    "app/src/UI/Widgets/ExtensionData.cpp",
    "app/src/Misc/Problems/ExtensionCheckers.h",
    "app/src/Misc/Problems/ExtensionCheckers.cpp",
    "app/qml/Dialogs/ExtensionConsent.qml",
    "app/qml/Widgets/Dashboard/ExtensionPlaceholder.qml",
    "app/qml/Widgets/Dashboard/ExtensionWidgetSettings.qml",
    "app/rcc/extensions/schema/widget-manifest.json",
    "app/rcc/extensions/widget/compass/info.json",
    "app/rcc/extensions/widget/datagrid/info.json",
    "doc/help/Widget-Extension-Development.md",
    "examples/widget-extension/README.md",
    "examples/widget-extension/info.json",
    "examples/widget-extension/LevelBar.qml",
    "doc/help/Extensions.md",
)


def load_verifier():
    """Import registry-verify by path; its filename is not a valid module name."""
    spec = importlib.util.spec_from_file_location("registry_verify", VERIFIER)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.fixture(scope="module")
def verifier():
    return load_verifier()


@pytest.fixture(scope="module")
def schema():
    return json.loads(SCHEMA.read_text(encoding="utf-8"))


@pytest.fixture(scope="module")
def manifests():
    return {
        path.parent.name: json.loads(path.read_text(encoding="utf-8"))
        for path in sorted(PACKAGES.glob("*/info.json"))
    }


def validator(schema):
    jsonschema = pytest.importorskip("jsonschema")
    return jsonschema.Draft7Validator(schema)


def errors_for(schema, instance) -> list[str]:
    return [e.message for e in validator(schema).iter_errors(instance)]


# --------------------------------------------------------------------------------------------------
# Bundled manifests
# --------------------------------------------------------------------------------------------------


def test_two_packages_are_bundled(manifests):
    assert set(manifests) == {"compass", "datagrid"}


def test_bundled_manifests_validate(schema, manifests):
    for name, manifest in manifests.items():
        assert not errors_for(schema, manifest), f"{name} does not validate"


def test_the_authoring_example_validates_and_declares_no_replaces(schema):
    """The package a third party copies must be legal as a third-party package."""
    manifest = json.loads(
        (ROOT / "examples" / "widget-extension" / "info.json").read_text(
            encoding="utf-8"
        )
    )
    assert not errors_for(schema, manifest)
    assert "replaces" not in manifest["widget"]
    assert manifest["id"] not in verifier_reserved_ids()
    for entry in manifest["files"]:
        assert (ROOT / "examples" / "widget-extension" / entry).is_file()


def verifier_reserved_ids() -> list[str]:
    return load_verifier().catalog_reserved_ids()


def test_bundled_manifests_replace_their_own_builtin(manifests):
    for name, manifest in manifests.items():
        assert manifest["id"] == name
        assert manifest["widget"]["replaces"] == name
        assert manifest["type"] == "widget"


def test_converted_widgets_keep_their_scope_and_string_declaration(manifests):
    assert manifests["compass"]["widget"]["scope"] == "dataset"
    assert manifests["compass"]["widget"]["readsStringValues"] is False
    assert manifests["datagrid"]["widget"]["scope"] == "group"
    assert manifests["datagrid"]["widget"]["readsStringValues"] is True


def test_declared_files_match_the_package_directory(manifests):
    for name, manifest in manifests.items():
        on_disk = {p.name for p in (PACKAGES / name).iterdir() if p.is_file()}
        assert set(manifest["files"]) == on_disk - {".DS_Store"}


def test_qml_entry_and_icon_resolve(verifier, manifests):
    for name, manifest in manifests.items():
        block = manifest["widget"]
        assert (PACKAGES / name / block["qml"]).is_file()
        assert verifier.icon_ref_resolves(
            block["icon"]
        ), f"{name} icon does not resolve"


# --------------------------------------------------------------------------------------------------
# Schema rules
# --------------------------------------------------------------------------------------------------


def test_reserved_id_needs_replaces(schema):
    claimed = {
        "id": "plot3d",
        "type": "widget",
        "title": "Not A Plot3D",
        "version": "1.0.0",
        "widget": {"scope": "dataset", "qml": "Fake.qml"},
    }
    assert errors_for(schema, claimed), "a reserved id was accepted without replaces"

    claimed["widget"]["replaces"] = "plot3d"
    assert not errors_for(schema, claimed), "replaces no longer unlocks a reserved id"


def test_third_party_id_needs_no_replaces(schema):
    manifest = {
        "id": "com.acme.thermal-map",
        "type": "widget",
        "title": "Thermal Map",
        "version": "1.2.0",
        "widget": {"scope": "dataset", "qml": "ThermalMap.qml"},
    }
    assert not errors_for(schema, manifest)


def test_qml_entry_cannot_escape_the_package(schema):
    for entry in ("../escape.qml", "/abs.qml", "..\\escape.qml"):
        manifest = {
            "id": "com.acme.escape",
            "type": "widget",
            "title": "Escape",
            "version": "1.0.0",
            "widget": {"scope": "dataset", "qml": entry},
        }
        assert errors_for(schema, manifest), f"{entry} was accepted as a qml entry"


def test_unknown_widget_keys_and_scopes_are_rejected(schema):
    base = {
        "id": "com.acme.thing",
        "type": "widget",
        "title": "Thing",
        "version": "1.0.0",
        "widget": {"scope": "dataset", "qml": "Thing.qml"},
    }
    unknown = json.loads(json.dumps(base))
    unknown["widget"]["proOnly"] = True
    assert errors_for(schema, unknown), "an undeclared widget key was accepted"

    scoped = json.loads(json.dumps(base))
    scoped["widget"]["scope"] = "tool"
    assert errors_for(schema, scoped), "an unknown scope was accepted"


def test_config_declarations_stay_scalar(schema):
    manifest = {
        "id": "com.acme.thing",
        "type": "widget",
        "title": "Thing",
        "version": "1.0.0",
        "widget": {
            "scope": "dataset",
            "qml": "Thing.qml",
            "config": [{"id": "nested", "type": "object"}],
        },
    }
    assert errors_for(schema, manifest), "a non-scalar config kind was accepted"


# --------------------------------------------------------------------------------------------------
# Reserved-id agreement and qrc sync
# --------------------------------------------------------------------------------------------------


def test_reserved_ids_agree_across_schema_and_catalog(verifier, schema):
    assert sorted(verifier.schema_reserved_ids(schema)) == sorted(
        verifier.catalog_reserved_ids()
    )


def test_every_builtin_widget_string_is_reserved(verifier):
    assert not verifier.builtin_widget_strings() - set(verifier.catalog_reserved_ids())


def test_bundled_files_are_registered_in_the_qrc(manifests):
    listed = set(
        re.findall(r"<file>(extensions/[^<]+)</file>", QRC.read_text(encoding="utf-8"))
    )
    for name, manifest in manifests.items():
        for entry in manifest["files"]:
            assert f"extensions/widget/{name}/{entry}" in listed


def test_the_schema_itself_ships_in_the_qrc():
    assert "extensions/schema/widget-manifest.json" in QRC.read_text(encoding="utf-8")


# --------------------------------------------------------------------------------------------------
# The verifier rule itself
# --------------------------------------------------------------------------------------------------


def test_registry_verify_is_clean_on_the_bundled_tree(verifier):
    errors: list[str] = []
    verifier.check_widget_manifests(errors)
    assert errors == []


def test_registry_verify_is_clean_on_the_context_name_mirror(verifier):
    errors: list[str] = []
    verifier.check_host_context_names(errors)
    assert errors == []


def test_a_seeded_manifest_without_replaces_fails(verifier, schema, tmp_path):
    package = tmp_path / "compass"
    shutil.copytree(PACKAGES / "compass", package)
    manifest = json.loads((package / "info.json").read_text(encoding="utf-8"))
    del manifest["widget"]["replaces"]
    (package / "info.json").write_text(json.dumps(manifest), encoding="utf-8")

    errors: list[str] = []
    verifier.check_widget_package(errors, package, schema, {})
    assert any("replaces" in message for message in errors)


def test_a_seeded_manifest_with_a_missing_file_fails(verifier, schema, tmp_path):
    package = tmp_path / "compass"
    shutil.copytree(PACKAGES / "compass", package)
    manifest = json.loads((package / "info.json").read_text(encoding="utf-8"))
    manifest["files"].append("Missing.qml")
    (package / "info.json").write_text(json.dumps(manifest), encoding="utf-8")

    errors: list[str] = []
    verifier.check_widget_package(errors, package, schema, {})
    assert any("Missing.qml" in message for message in errors)


def test_dropping_the_pairing_rule_from_the_schema_fails(verifier, schema):
    pytest.importorskip("jsonschema")
    weakened = json.loads(json.dumps(schema))
    del weakened["anyOf"]

    errors: list[str] = []
    verifier.check_widget_schema_rules(errors, weakened)
    assert any("plot3d" in message for message in errors)


def test_an_untabled_context_property_fails(verifier, tmp_path):
    """A global the composition root registers but Misc::ContextRegistry does not list is a name
    that reaches QML without an extension's context shadowing it (spec 0075 G4)."""
    seeded = tmp_path / "ModuleManager.cpp"
    source = verifier.MODULE_MANAGER_CPP.read_text(encoding="utf-8")
    seeded.write_text(
        source + '\n  registry.add("Cpp_Unshadowed_Module", nullptr);\n',
        encoding="utf-8",
    )

    original = verifier.MODULE_MANAGER_CPP
    verifier.MODULE_MANAGER_CPP = seeded
    try:
        errors: list[str] = []
        verifier.check_host_context_names(errors)
    finally:
        verifier.MODULE_MANAGER_CPP = original

    assert any("Cpp_Unshadowed_Module" in message for message in errors)


def test_a_stale_registry_table_entry_fails(verifier, tmp_path):
    """The drift is caught in both directions: a table entry nothing registers any more would
    keep an extension's context shadowing a name that no longer exists."""
    seeded = tmp_path / "ContextRegistry.cpp"
    source = verifier.CONTEXT_REGISTRY_CPP.read_text(encoding="utf-8")
    seeded.write_text(
        source.replace(
            'QStringLiteral("Cpp_AppState"),',
            'QStringLiteral("Cpp_AppState"),\n    QStringLiteral("Cpp_Retired_Module"),',
            1,
        ),
        encoding="utf-8",
    )

    original = verifier.CONTEXT_REGISTRY_CPP
    verifier.CONTEXT_REGISTRY_CPP = seeded
    try:
        errors: list[str] = []
        verifier.check_host_context_names(errors)
    finally:
        verifier.CONTEXT_REGISTRY_CPP = original

    assert any("Cpp_Retired_Module" in message for message in errors)


def test_host_context_names_must_read_the_registry(verifier, tmp_path):
    """hostContextNames() keeping a second hand-written list is the drift the table replaced."""
    seeded = tmp_path / "WidgetExtensions.cpp"
    source = verifier.WIDGET_CATALOG_CPP.read_text(encoding="utf-8")
    seeded.write_text(
        source.replace("Misc::ContextRegistry::objectNames()", "QStringList()", 1),
        encoding="utf-8",
    )

    original = verifier.WIDGET_CATALOG_CPP
    verifier.WIDGET_CATALOG_CPP = seeded
    try:
        errors: list[str] = []
        verifier.check_host_context_names(errors)
    finally:
        verifier.WIDGET_CATALOG_CPP = original

    assert any("hostContextNames" in message for message in errors)


# --------------------------------------------------------------------------------------------------
# Trust wording
# --------------------------------------------------------------------------------------------------


def sentence_around(text: str, index: int) -> str:
    start = max(0, index - 160)
    return text[start : index + 80].replace("\n", " ").lower()


def test_no_spec_0038_file_claims_containment():
    offenders: list[str] = []
    for name in SPEC_FILES:
        path = ROOT / name
        if not path.is_file():
            continue

        text = path.read_text(encoding="utf-8")
        for match in FORBIDDEN.finditer(text):
            context = sentence_around(text, match.start())
            if any(denial in context for denial in DENIALS):
                continue

            line = text.count("\n", 0, match.start()) + 1
            offenders.append(f"{name}:{line}: '{match.group(0)}' outside a denial")

    assert not offenders, "\n".join(offenders)


def test_the_consent_dialog_states_the_privilege():
    dialog = (ROOT / "app" / "qml" / "Dialogs" / "ExtensionConsent.qml").read_text(
        encoding="utf-8"
    )
    assert "same privileges" in dialog
    assert "Allow" in dialog
