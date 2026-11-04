"""Static checks over the property registry's cross-field validators (spec 0108).

Pure Python: drives the real generator. A cross-field check reads several fields, so an API
update naming any one of them must run it: a request carrying only formatWidth can shrink the
width under a fraction or a bit field it never named.
"""

import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GENERATOR = ROOT / "scripts" / "generate-property-registry.py"


def load_generator():
    """Import the generator by path; its filename is not a valid module name."""
    spec = importlib.util.spec_from_file_location("property_registry", GENERATOR)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def gate_of(source: str, check: str) -> str:
    """The `if` condition guarding one cross-field check in the generated API appliers."""
    end = source.index(f"!PropertyHooks::{check}(d)")
    return source[source.rindex("  if (", 0, end) : end]


def test_each_cross_field_check_runs_once_gated_on_every_field_it_reads():
    gen = load_generator()
    manifest = gen.load_manifest()
    props = gen.by_id(manifest)
    source = gen.render_api(manifest)
    for name, check in gen.DATASET_VALIDATORS.items():
        assert (
            source.count(f"!PropertyHooks::{check}(d)") == 1
        ), f"{check} emitted twice"
        gate = gate_of(source, check)
        for prop_id in manifest["hooks"][name]["reads"]:
            for api in gen.api_names(props[prop_id], manifest):
                assert f"consumed.contains({gen.json_name(api)})" in gate, (check, api)


def test_a_width_only_update_revalidates_fraction_and_bit_field():
    gen = load_generator()
    source = gen.render_api(gen.load_manifest())
    width = f"consumed.contains({gen.json_name('formatWidth')})"
    assert width in gate_of(source, "isValidFormatFraction")
    assert width in gate_of(source, "isValidFormatBitField")
