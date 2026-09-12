"""
Spec 0086 R5 -- the committed JS number corpus is what Node prints today.

`tests/fixtures/js-number-format.json` maps a double's IEEE-754 bits to ECMAScript
Number::toString text; `app/tests/tst_js_number_format.cpp` holds `formatJsNumber` to it byte for
byte. This test keeps the fixture honest: every entry is re-derived through a fresh Node process
and must match, and the edge cases the generator promises are present.
"""

import json
import shutil
import struct
import subprocess
from pathlib import Path

import pytest

FIXTURE = Path(__file__).resolve().parent.parent / "fixtures" / "js-number-format.json"

REQUIRED_BITS = {
    "0000000000000000": "0",
    "8000000000000000": "0",
    "7ff0000000000000": "Infinity",
    "fff0000000000000": "-Infinity",
    "7ff8000000000000": "NaN",
    "444b1ae4d6e2ef50": "1e+21",
    "3e7ad7f29abcaf48": "1e-7",
    "3eb0c6f7a0b5ed8d": "0.000001",
    "3fd3333333333334": "0.30000000000000004",
    "4340000000000000": "9007199254740992",
    "0000000000000001": "5e-324",
}


def _node_texts(keys: list) -> dict:
    program = (
        "const out = {};"
        "const buf = Buffer.alloc(8);"
        "const keys = require('fs').readFileSync(0, 'utf8').trim().split(',');"
        "for (const hex of keys) {"
        "  buf.writeBigUInt64BE(BigInt('0x' + hex));"
        "  out[hex] = String(buf.readDoubleBE(0));"
        "}"
        "process.stdout.write(JSON.stringify(out));"
    )
    result = subprocess.run(
        ["node", "-e", program],
        input=",".join(keys),
        capture_output=True,
        text=True,
        check=True,
    )
    return json.loads(result.stdout)


@pytest.fixture(scope="module")
def corpus() -> dict:
    assert FIXTURE.is_file(), f"missing fixture {FIXTURE}; run gen_js_number_corpus.py"
    return json.loads(FIXTURE.read_text(encoding="utf-8"))


def test_fixture_shape(corpus):
    assert len(corpus) >= 2000
    for key, text in corpus.items():
        assert len(key) == 16 and int(key, 16) >= 0, key
        assert isinstance(text, str) and text, key


def test_required_edge_cases_present(corpus):
    for bits, text in REQUIRED_BITS.items():
        assert corpus.get(bits) == text, (bits, corpus.get(bits), text)


def test_keys_round_trip_as_doubles(corpus):
    for key in corpus:
        value = struct.unpack("<d", struct.pack("<Q", int(key, 16)))[0]
        assert struct.pack("<d", value).hex() == bytes.fromhex(key)[::-1].hex()


@pytest.mark.skipif(shutil.which("node") is None, reason="Node.js not on PATH")
def test_fixture_matches_fresh_node_output(corpus):
    keys = sorted(corpus)
    fresh = _node_texts(keys)
    mismatches = {
        k: (corpus[k], fresh.get(k)) for k in keys if corpus[k] != fresh.get(k)
    }
    assert (
        not mismatches
    ), f"{len(mismatches)} entries differ from Node: {list(mismatches.items())[:5]}"
