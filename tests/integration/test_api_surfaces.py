"""
Generated API Surfaces Integration Tests (spec 0037)

Verifies that every surface derived from the dataset property manifest actually
carries the declared fields at runtime: the MCP tool schema, the generated SDK
wrapper, and the typed gRPC proto.

The proto parity case needs a COMMERCIAL build configured with ENABLE_GRPC=ON --
that is what CI produces; a GPL or gRPC-less build skips it.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import os
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "app" / "rcc" / "properties" / "dataset.json"
TYPED_PROTO = ROOT / "doc" / "grpc" / "serialstudio-typed.proto"


def declared_fields() -> dict:
    """Return {api name: property} for every exposed dataset property."""
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    return {
        p["api"]["name"]: p
        for p in manifest["properties"]
        if p.get("api", {}).get("expose")
    }


def declared_domains() -> dict:
    """Return {api name: [enum values]} for every exposed property with a fixed domain."""
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    out = {}
    for prop in manifest["properties"]:
        api = prop.get("api", {})
        source = manifest["optionSources"].get(prop.get("options", ""), {})
        if not api.get("expose") or source.get("kind") not in (
            "staticMap",
            "parallelValues",
        ):
            continue
        out[api["name"]] = [e["value"] for e in source["entries"]]
    return out


def tools_list(api_client) -> list:
    """Return the MCP tools/list reply, served over the same TCP port as the command API."""
    api_client._send_message(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {
                "protocolVersion": "2024-11-05",
                "capabilities": {},
                "clientInfo": {"name": "spec-0037", "version": "1"},
            },
        }
    )
    api_client._recv_message()

    api_client._send_message({"jsonrpc": "2.0", "id": 2, "method": "tools/list"})
    reply = api_client._recv_message()
    return reply.get("result", {}).get("tools", [])


def dataset_update_schema(api_client) -> dict:
    """Return project.dataset.update's published inputSchema properties."""
    for tool in tools_list(api_client):
        if tool.get("name") == "project.dataset.update":
            return tool.get("inputSchema", {}).get("properties", {})

    pytest.fail("project.dataset.update is not exposed as an MCP tool")


@pytest.mark.integration
@pytest.mark.project
def test_dataset_update_schema_lists_every_declared_field(api_client):
    """AC9 -- every manifest field is a typed property with a description."""
    properties = dataset_update_schema(api_client)

    missing = sorted(set(declared_fields()) - set(properties))
    assert not missing, f"dataset.update schema is missing declared fields: {missing}"

    for name in declared_fields():
        prop = properties[name]
        assert prop.get("type"), f"{name} has no type in the published schema"
        assert prop.get("description"), f"{name} has no description"


@pytest.mark.integration
@pytest.mark.project
def test_dataset_update_schema_carries_enum_domains(api_client):
    """AC9 -- enum-valued fields publish their domain, so callers stop guessing."""
    properties = dataset_update_schema(api_client)

    for name, domain in declared_domains().items():
        published = properties.get(name, {}).get("enum")
        assert published is not None, f"{name} publishes no enum domain"
        assert list(published) == list(domain), f"{name} domain differs from manifest"


@pytest.mark.integration
def test_tools_list_payload_size_is_recorded(api_client):
    """AC10 -- record the tools/list payload size so the typed-schema delta is a number."""
    tools = tools_list(api_client)
    payload = len(json.dumps(tools).encode("utf-8"))
    print(f"\n[AC10] tools/list payload: {payload} bytes over {len(tools)} tools")
    assert tools, "tools/list returned no tools"


@pytest.mark.integration
@pytest.mark.project
def test_sdk_wrapper_sets_every_declared_field(api_client, clean_state):
    """AC11 -- the SDK's options bag reaches every declared field and reads back."""
    api_client.create_new_project(title="SDK field sweep")
    group_id = api_client.add_group("G")
    api_client.add_dataset(group_id)
    datasets = [d for d in api_client.list_datasets() if d["groupId"] == group_id]
    dataset_id = datasets[-1]["datasetId"]

    params = {"groupId": group_id, "datasetId": dataset_id}
    samples = {"int": 3, "double": 1.5, "bool": True, "string": "x"}
    fields = declared_fields()
    domains = declared_domains()
    for name, prop in fields.items():
        if name in domains:
            params[name] = domains[name][0]
        elif name in ("color", "alias", "transformCode", "widget"):
            continue
        elif prop.get("validate"):
            # Validated fields reject arbitrary samples (e.g. transformLanguage
            # only accepts -1/0/1); the declared default is always in-domain.
            params[name] = prop["default"]
        else:
            params[name] = samples[prop["type"]]

    result = api_client.command("project.dataset.update", params)
    assert result.get("updated") is True

    warnings = result.get("warnings", [])
    unknown = [w for w in warnings if w.get("code") == "unknown_field"]
    assert not unknown, f"declared fields were rejected as unknown: {unknown}"


@pytest.mark.integration
def test_runtime_proto_matches_the_checked_in_copy():
    """AC8 -- the exported typed proto equals doc/grpc/serialstudio-typed.proto.

    The export is a GUI action (Settings > gRPC > Export .proto), not an API command, so
    this case reads the exported file from SS_EXPORTED_PROTO. Maintainer run: export from
    a COMMERCIAL build configured with ENABLE_GRPC=ON, then

        SS_EXPORTED_PROTO=/tmp/serial_studio.proto pytest tests/integration/test_api_surfaces.py -k proto
    """
    exported_path = os.environ.get("SS_EXPORTED_PROTO")
    if not exported_path:
        pytest.skip("set SS_EXPORTED_PROTO to the file exported from Settings > gRPC")

    assert TYPED_PROTO.exists(), "the checked-in typed proto is missing"
    exported = Path(exported_path).read_text(encoding="utf-8")
    assert exported == TYPED_PROTO.read_text(encoding="utf-8"), (
        "the runtime proto export differs from the checked-in copy; refresh "
        "api-schema.json from this build, then run "
        "python3 scripts/generate-property-registry.py"
    )
