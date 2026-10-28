#!/usr/bin/env python3
"""Claude Code hook: verify the Context Canary on every assistant response.

Mechanizes the CLAUDE.md "Context Canary" probe (Stop event). The canary is
only useful if a mutated or missing line is actually *noticed*; a human stops
reading a boilerplate footer after a day, so this hook does the noticing
deterministically, at zero model-token cost.

On Stop it reads the session transcript (JSONL), takes the last non-empty
line of the final assistant message, and compares it against the expected
canary. Markdown decoration (backtick/bold/italic wrappers, whitespace
runs, formatting-only trailing lines like a closing code fence) is
normalized away first - the probe measures whether the *values* survived
in context, and a backtick-wrapped but value-perfect canary is a healthy
context, not a mutation. Outcomes:

  exact match      - silent (healthy context; success is invisible by design).
  "canary: lost"   - loud systemMessage: the model itself signaled degradation.
  mutated line     - loud systemMessage quoting the mutation (which value
                     drifted tells you which fact fell out of the workspace).
  missing entirely - loud systemMessage: instruction no longer steering.

What the probe measures: CLAUDE.md is in context every turn, so the line is
reproduced from the system prompt, not recalled. A mutated or missing line
means the always-present instruction stopped steering (heavy compaction, a
truncated prompt, attention drift), not that one fact was forgotten on its
own; which value drifted still says which rule lost its hold.

Design choices specific to this repo:
  - The final text is the Stop event's last_assistant_message, which the
    harness fills from memory. The transcript is written asynchronously and
    can lag it (observed on macOS, 2026-07-14 - the hook read the file ~300 ms
    before the canary-bearing entry landed and reported MISSING on a healthy
    turn; PrimeFoldTools/Andon's 2026-10-28 advisory showed the same race on
    the done-claim hook). A harness that omits the field falls back to the
    transcript, re-reading a bad verdict a few times with a short sleep: a
    flush race resolves in milliseconds, while a truly missing canary stays
    missing, so only the failure path pays the delay.
  - Fail-open everywhere: any internal error exits 0 silently, matching the
    other hooks. A broken probe must never block real work.
  - Warn only, never block ("decision": "block" would re-invoke the model and
    burn the tokens this hook exists to avoid).
  - The expected line is duplicated from CLAUDE.md on purpose: reading it
    from the doc at check time would let a corrupted doc validate itself.
    The `qt-version` anchor in scripts/doc-anchors.json binds this copy, so
    claim-verify.py fails when either side drifts (the 2026-08-20 Qt bump
    left this file three weeks behind, warning on every healthy turn).
"""

import json
import os
import sys
import time


def _retry_attempts(default: int) -> int:
    """Retry count from SS_HOOK_RETRIES (tests set 1); a bad value keeps the default."""
    try:
        return max(1, int(os.environ.get("SS_HOOK_RETRIES", default)))
    except ValueError:
        return default


RETRY_ATTEMPTS = _retry_attempts(8)
RETRY_DELAY_S = 0.4

EXPECTED_CANARY = (
    "canary: qt 6.11.2 | cpp20 | hotpath 256k (native 1024k, js 64k) "
    "| queue 65536 | api 7777 | style 100/2"
)
LOST_MARKER = "canary: lost"
ANCHOR = "canary:"
FORMATTING_CHARS = "`*_~\"' \t"


def normalize(line: str) -> str:
    """Strip markdown wrappers and collapse whitespace; values pass through."""
    return " ".join(line.strip(FORMATTING_CHARS).split())


def last_assistant_text(transcript_path: str) -> str | None:
    """Return the visible text of the final main-thread assistant message."""
    lines = []
    with open(transcript_path, encoding="utf-8") as handle:
        lines = handle.readlines()

    for raw in reversed(lines):
        raw = raw.strip()
        if not raw:
            continue

        try:
            entry = json.loads(raw)
        except ValueError:
            continue

        if entry.get("type") != "assistant" or entry.get("isSidechain"):
            continue

        content = (entry.get("message") or {}).get("content")
        if isinstance(content, str):
            text = content
        elif isinstance(content, list):
            text = "\n".join(
                block.get("text", "")
                for block in content
                if isinstance(block, dict) and block.get("type") == "text"
            )
        else:
            continue

        if text.strip():
            return text
    return None


def verdict(text: str) -> str | None:
    """Compare the response's last line against the canary; None means healthy."""
    tail_lines = [normalize(line) for line in text.strip().splitlines()]
    tail_lines = [line for line in tail_lines if line]
    last_line = tail_lines[-1] if tail_lines else ""
    if last_line == EXPECTED_CANARY:
        return None

    if LOST_MARKER in last_line.lower():
        return (
            "CANARY LOST: the model reported it cannot reproduce the context "
            "canary. Context is degraded - checkpoint now, then /compact or "
            "restart before continuing non-trivial work."
        )

    anchored = [line for line in tail_lines if ANCHOR in line.lower()]
    if anchored:
        return (
            "CANARY MUTATED: expected\n  " + EXPECTED_CANARY + "\nbut the "
            "response ended with\n  " + anchored[-1] + "\nA drifted value "
            "means that fact fell out of the working context - treat recent "
            "output as suspect and consider /compact or a restart."
        )

    return (
        "CANARY MISSING: the response did not end with the context canary "
        "line from CLAUDE.md. The instruction is no longer steering - the "
        "context window is likely degraded; checkpoint and /compact or "
        "restart before continuing non-trivial work."
    )


def check(transcript_path: str) -> str | None:
    """One transcript read; None means healthy (or no assistant text yet)."""
    text = last_assistant_text(transcript_path)
    if text is None:
        return None
    return verdict(text)


def main() -> None:
    try:
        event = json.load(sys.stdin)
        final = event.get("last_assistant_message")
        transcript_path = event.get("transcript_path")
        message = None
        if isinstance(final, str) and final.strip():
            message = verdict(final)
        elif transcript_path:
            for attempt in range(RETRY_ATTEMPTS):
                message = check(str(transcript_path))
                if message is None:
                    break
                if attempt < RETRY_ATTEMPTS - 1:
                    time.sleep(RETRY_DELAY_S)
        if message is not None:
            print(json.dumps({"systemMessage": message}))
    except Exception:
        pass
    sys.exit(0)


if __name__ == "__main__":
    main()
