"""
Generates tests/fixtures/js-number-format.json: the ECMAScript Number::toString text of a corpus
of doubles, produced by Node itself, so the C++ formatter behind the cell lane (spec 0086,
DataModel::formatJsNumber) can be pinned byte-for-byte by tst_js_number_format.

Run from anywhere:  python3 tests/scripts/gen_js_number_corpus.py
Requires Node.js on PATH. The fixture maps the double's IEEE-754 bit pattern (16 hex digits) to
the text Node prints, so no float ever round-trips through JSON.
"""

import json
import random
import struct
import subprocess
from pathlib import Path

FIXTURE = Path(__file__).resolve().parent.parent / "fixtures" / "js-number-format.json"

EDGE_CASES = [
    0.0,
    -0.0,
    1.0,
    -1.0,
    0.5,
    0.1,
    0.2,
    0.1 + 0.2,
    1.5,
    2.5,
    100.0,
    123.456,
    1e-7,
    1e-6,
    9.99e-7,
    1.5e-6,
    0.000001,
    0.00001,
    0.0001,
    0.001,
    1e20,
    1e21,
    9.99e20,
    1.5e21,
    123456789012345680000.0,
    1e22,
    1e100,
    1e300,
    1.7976931348623157e308,
    5e-324,
    2.2250738585072014e-308,
    9007199254740992.0,
    9007199254740993.0,
    9007199254740991.0,
    4294967295.0,
    4294967296.0,
    2147483647.0,
    -2147483648.0,
    3.141592653589793,
    2.718281828459045,
    1.0 / 3.0,
    2.0 / 3.0,
    float("nan"),
    float("inf"),
    float("-inf"),
    12345.678901234567,
    0.30000000000000004,
    1e-5,
    123e-20,
    5e-7,
    5e-8,
]


def bits_of(value: float) -> str:
    return struct.unpack("<Q", struct.pack("<d", value))[0].to_bytes(8, "big").hex()


def main() -> None:
    rng = random.Random(20260912)
    values = list(EDGE_CASES)
    for _ in range(2000):
        exponent = rng.randint(-40, 40)
        mantissa = rng.random() * 10.0
        values.append(mantissa * (10.0**exponent) * rng.choice((1.0, -1.0)))
    for _ in range(500):
        values.append(float(rng.randint(-(10**15), 10**15)))
    for _ in range(300):
        raw = rng.getrandbits(64)
        candidate = struct.unpack("<d", struct.pack("<Q", raw))[0]
        values.append(candidate)

    program = "\n".join(
        [
            "const out = {};",
            "const buf = Buffer.alloc(8);",
            "const keys = require('fs').readFileSync(0, 'utf8').trim().split(',');",
            "for (const hex of keys) {",
            "  buf.writeBigUInt64BE(BigInt('0x' + hex));",
            "  out[hex] = String(buf.readDoubleBE(0));",
            "}",
            "process.stdout.write(JSON.stringify(out));",
        ]
    )
    keys = sorted({bits_of(v) for v in values})
    result = subprocess.run(
        ["node", "-e", program],
        input=",".join(keys),
        capture_output=True,
        text=True,
        check=True,
    )
    corpus = json.loads(result.stdout)
    FIXTURE.write_text(
        json.dumps(corpus, indent=1, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(f"wrote {len(corpus)} entries to {FIXTURE}")


if __name__ == "__main__":
    main()
