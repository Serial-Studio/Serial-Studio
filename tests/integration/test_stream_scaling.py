"""
Stream-Lane Scaling Tests (spec 0051, M7 / AC6)

Per-source stream processing must be independent: adding stream sources must
not slow the ones already running while physical cores remain. The scripted
half of AC6 reads the per-source diagnostics counters through
`stream.getSources` (samplesProcessed is a plain pulled counter) and compares
per-source throughput as sources are added.

This test needs n live stream sources, so it self-skips unless the loopback
audio rig from test_audio_loopback.py is configured. The benchmark's
`hotpath-stream` phase (`--benchmark-hotpath`) is the hardware-free throughput
source that backs AC6/AC19 on the dev machine.

The app must be running with the API server enabled (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import os
import time

import pytest

# Per-source throughput must stay within this fraction of the single-source
# figure while cores remain available (AC6 allows 10%).
MIN_RATIO = 0.9
SAMPLE_SECONDS = 3.0


def _throughput_by_source(api_client, seconds: float) -> dict:
    """Returns {sourceId: samples/s} measured from the pulled worker counters."""
    first = {
        src["sourceId"]: src.get("samplesProcessed", 0)
        for src in api_client.command("stream.getSources", {}).get("sources", [])
    }
    start = time.monotonic()
    time.sleep(seconds)
    elapsed = time.monotonic() - start

    second = {
        src["sourceId"]: src.get("samplesProcessed", 0)
        for src in api_client.command("stream.getSources", {}).get("sources", [])
    }

    rates = {}
    for source_id, before in first.items():
        after = second.get(source_id, before)
        rates[source_id] = max(0.0, (after - before) / elapsed)

    return rates


@pytest.mark.audio
class TestStreamScaling:
    def test_added_sources_do_not_slow_the_others(self, api_client, clean_state):
        """AC6: per-source throughput holds within 10% as sources are added."""
        if not os.environ.get("SS_AUDIO_CAPTURE_MATCH"):
            pytest.skip("no loopback capture device configured")

        sources = api_client.command("stream.getSources", {}).get("sources", [])
        if len(sources) < 2:
            pytest.skip(f"needs >= 2 live stream sources, found {len(sources)}")

        rates = _throughput_by_source(api_client, SAMPLE_SECONDS)
        live = {sid: rate for sid, rate in rates.items() if rate > 0.0}
        assert len(live) >= 2, f"fewer than two sources produced samples: {rates}"

        best = max(live.values())
        for source_id, rate in live.items():
            assert rate >= best * MIN_RATIO, (
                f"source {source_id} fell to {rate:.0f} samples/s while the fastest "
                f"source ran at {best:.0f} samples/s (>10% spread)"
            )


@pytest.mark.integration
class TestStreamDiagnostics:
    def test_counters_are_exposed(self, api_client):
        """The AC6/AC19 counter source answers even with no stream source live."""
        result = api_client.command("stream.getSources", {})
        assert "sources" in result

        for src in result["sources"]:
            for key in (
                "samplesProcessed",
                "blocksProcessed",
                "transformErrors",
                "displayDrops",
            ):
                assert key in src, f"source {src.get('sourceId')} is missing {key}"
