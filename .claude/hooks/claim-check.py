#!/usr/bin/env python3
"""Claude Code hook: ask for evidence behind a done-claim (Stop event).

Mechanizes CLAUDE.md's "report outcomes faithfully" and the trust contract's
"self-review before handoff". A turn that EDITED files and then ends on a
completion claim ("done", "fixed", "verified", "all tests pass") without
having run any repo verification tool in between gets a warning naming the
matched phrase, the files touched, and the evidence that is missing.

Evidence is read from the transcript, not self-reported. Since the last human
prompt, on this session's main thread: was there a Bash/PowerShell tool call
running a verification command (code-verify, claim-verify, sanitize-commit,
documentation-verify, layer-verify, registry-verify, pytest, ctest, --selftest,
--benchmark-hotpath) that the harness can vouch for? A run is credited only when
its recorded result exists and is clean (no `is_error`, and none of the
top-level `toolUseResult` markers `interrupted`, `backgroundTaskId`,
`timedOutAfterMs`, `returnCodeInterpretation`, all of which report success
without a finished, honest exit) AND its command line is a plain form whose exit
status is the verifier's own (see creditable_command). If a credited run exists,
or the turn made no edits, or the final message makes no claim, the hook stays
silent. Refusing credit never creates a warning on its own.

Design choices specific to this repo:
  - Adapted from PrimeFoldTools/andon's claim_check_hook.py (MIT). That
    version accepts any self-written log line inside a time window; this one
    keys on tool calls the harness itself recorded, so nothing has to be
    logged and a stale entry cannot satisfy a fresh claim.
  - Edits are the trigger, not claims alone. A read-only analysis turn may
    legitimately say "confirmed"; an edit turn saying "done" without running
    the linter is the defect class (andon ledger 2026-04, "the false done").
    Shell writes count as edits through a heuristic (sed -i, tee, redirects,
    Set-Content, cp/mv, git apply); a miss there is a silent pass, never a
    false warning.
  - Warn only, never block: a block re-invokes the model, and canary-check.py
    already settled that a Stop hook here never spends tokens. The escape
    hatch is SS_CLAIM_CHECK=off.
  - Retry before deciding, same flush race as canary-check.py: the final
    assistant entry can land a few hundred ms after Stop fires.
  - Fail-open everywhere: any internal error exits 0 silently.
  - The plain forms that register are listed in doc/claude/scripts.md
    ("Creditable verification forms"); the warning names the nearest run that
    was not credited and why, so the next turn runs the plain form.
  - Marker placement was read off real transcripts (2026-10-17): every marker
    sits in the user entry's top-level toolUseResult, beside (not inside) the
    tool_result content block, which carries only is_error.
"""

import json
import os
import re
import sys
import time



def _retry_attempts(default: int) -> int:
    """Retry count from SS_HOOK_RETRIES (tests set 1); a bad value keeps the default."""
    try:
        return max(1, int(os.environ.get("SS_HOOK_RETRIES", default)))
    except ValueError:
        return default


RETRY_ATTEMPTS = _retry_attempts(6)
RETRY_DELAY_S = 0.4
MAX_LISTED = 5

EDIT_TOOLS = {"Edit", "Write", "MultiEdit", "NotebookEdit"}
SHELL_TOOLS = {"Bash", "PowerShell"}

VERIFY_RE = re.compile(
    r"code-verify\.py|claim-verify\.py|sanitize-commit\.py|documentation-verify\.py"
    r"|layer-verify\.py|registry-verify\.py|\bpytest\b|\bctest\b"
    r"|--selftest\b|--benchmark-hotpath\b"
)
VERIFIER_SCRIPTS = (
    "code-verify.py",
    "claim-verify.py",
    "sanitize-commit.py",
    "documentation-verify.py",
    "layer-verify.py",
    "registry-verify.py",
)
PYTHON_NAMES = ("python3", "python", "py")
REFUSED_FLAGS = {
    "pytest": ("--collect-only", "--co", "--setup-only", "--setup-plan"),
    "ctest": ("-N", "--show-only"),
    "code-verify.py": ("--fix", "--accept"),
}
ALLOWED_REDIRECT_RE = re.compile(r"\s(?:2>&1|[12&]?>\s*/dev/null|2>\s*/dev/null)(?=\s|$)")
FORBIDDEN_RE = re.compile(r"[;|&#`$!]|\bexit\b|\btrap\b")
ASSIGNMENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*=")
SHELL_WRITE_RE = re.compile(
    r"\bsed\s+(?:-[a-zA-Z]*i|--in-place)\b|\btee\b|\bcp\s|\bmv\s|\bgit\s+apply\b"
    r"|\bpatch\s|\bSet-Content\b|\bOut-File\b|\bAdd-Content\b"
    r"|(?<![<>&|\d])>{1,2}\s*(?!&|/dev/null|\$null|nul\b)[\"'$\w./\\]"
    r"|\bopen\([^)]*[\"'][wa]b?[\"']"
)

_VERBS = r"done|complete|completed|fixed|verified|resolved|shipped|passing|green"
CLAIM_PATTERNS = (
    re.compile(
        r"(?im)(?:^|(?<=[.!?])\s+)\s*(?:done|complete|completed|fixed|verified|resolved"
        r"|shipped|all\s+set)\b\s*(?:[.!:;]|$)"
    ),
    re.compile(
        r"(?im)(?:^|(?<=[.!?])\s+)\s*(?:shipped|fixed|verified|resolved|completed"
        r"|implemented)\s+(?:the|this|that|these|those|all|both|every)\b[^\n.!?]{0,80}"
    ),
    re.compile(
        rf"\b(?:is|are|has\s+been|have\s+been|now|fully|already)"
        rf"(?:\s+(?:already|now|fully))?\s+(?:{_VERBS})\b",
        re.IGNORECASE,
    ),
    re.compile(r"\*\*(?:done|complete|fixed|verified|resolved|shipped)\*\*", re.IGNORECASE),
    re.compile(
        r"\b(?:all\s+)?(?:tests?|suite|lint(?:er)?|code-verify|claim-verify|ctest|pytest)"
        r"\s+(?:now\s+)?(?:pass(?:es|ed)?|green|clean)\b",
        re.IGNORECASE,
    ),
    re.compile(r"\bverified\s+end[\s-]to[\s-]end\b", re.IGNORECASE),
)
FENCE_RE = re.compile(r"```.*?```", re.DOTALL)


def load_entries(transcript_path: str) -> list[dict]:
    """Parse the JSONL transcript, skipping blank and malformed lines."""
    entries = []
    with open(transcript_path, encoding="utf-8") as handle:
        for raw in handle:
            raw = raw.strip()
            if not raw:
                continue
            try:
                entries.append(json.loads(raw))
            except ValueError:
                continue
    return entries


def blocks_of(entry: dict) -> list[dict]:
    """Return the content blocks of a transcript entry, or [] for string content."""
    content = (entry.get("message") or {}).get("content")
    if isinstance(content, list):
        return [block for block in content if isinstance(block, dict)]
    return []


def is_human_prompt(entry: dict) -> bool:
    """True for a typed user turn; tool results and injected meta turns are not."""
    if entry.get("type") != "user" or entry.get("isSidechain") or entry.get("isMeta"):
        return False
    content = (entry.get("message") or {}).get("content")
    if isinstance(content, str):
        return True
    return not any(block.get("type") == "tool_result" for block in blocks_of(entry))


def turn_entries(entries: list[dict]) -> list[dict]:
    """Slice the transcript from the last human prompt to the end."""
    for index in range(len(entries) - 1, -1, -1):
        if is_human_prompt(entries[index]):
            return entries[index:]
    return entries


def last_assistant_text(turn: list[dict]) -> str | None:
    """Return the visible text of the final main-thread assistant message."""
    for entry in reversed(turn):
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


def _unquoted_glob(segment: str) -> bool:
    """True when `* ? [` appears outside single or double quotes (shell expands it)."""
    quote = ""
    for char in segment:
        if quote:
            if char == quote:
                quote = ""
        elif char in "'\"":
            quote = char
        elif char in "*?[":
            return True
    return False


def _verifier_tool(tokens: list[str]) -> str | None:
    """Name the verifier a simple command runs, or None when it is not a plain form."""
    while tokens and ASSIGNMENT_RE.match(tokens[0]):
        tokens = tokens[1:]
    if not tokens:
        return None
    head = tokens[0]
    if head == "pytest":
        return "pytest"
    if head == "ctest":
        return "ctest"
    if "--selftest" in tokens or "--benchmark-hotpath" in tokens:
        return "selftest" if "/" in head else None
    if head not in PYTHON_NAMES:
        return None
    rest = [tok for tok in tokens[1:] if not tok.startswith("-")] or [""]
    if tokens[1:3] == ["-m", "pytest"]:
        return "pytest"
    script = rest[0].rsplit("/", 1)[-1]
    if script in VERIFIER_SCRIPTS and rest[0].replace("\\", "/").split("/")[-2:-1] == ["scripts"]:
        return script
    return None


def _refused_flag(tool: str, tokens: list[str]) -> str | None:
    """A no-run or masking flag for this tool, or None."""
    if tool == "code-verify.py" and "--check" not in tokens:
        return "code-verify without --check"
    for flag in REFUSED_FLAGS.get(tool, ()):
        for tok in tokens:
            if tok == flag or tok.startswith(flag + "="):
                return f"{tool} {flag}"
    return None


def creditable_command(command: str) -> str | None:
    """Why this command line cannot be credited, or None when its exit status is the verifier's.

    Accepted shape: one line, segments joined by `&&` only, optional NAME=value prefixes and
    `cd <path>` segments, redirects `2>&1` / `>/dev/null`, and one segment that is a plain
    verifier invocation without a no-run or masking flag. Pipes, `;`, `||`, `&`, comments,
    substitutions, `exit`/`trap`, `!` and unquoted globs are refused: each lets a failing
    verifier report success to the harness.
    """
    line = command.strip()
    if "\n" in line or "\r" in line:
        return "multi-line command"
    line = ALLOWED_REDIRECT_RE.sub(" ", line).replace("&&", "\x00")
    if FORBIDDEN_RE.search(line):
        return "separator, pipe, comment, substitution or negation masks the exit status"
    if _unquoted_glob(line):
        return "unquoted glob is expanded by the shell before the verifier runs"
    found = False
    for segment in line.split("\x00"):
        tokens = segment.split()
        tool = _verifier_tool(tokens)
        if tool is None:
            continue
        refused = _refused_flag(tool, tokens)
        if refused:
            return refused
        found = True
    return None if found else "no plain verifier invocation on the command line"


def _result_problem(block: dict, result: dict) -> str | None:
    """Why a recorded tool result cannot vouch for a run, or None when it is clean."""
    if block.get("is_error"):
        return "the run errored"
    for source in (result, block):
        if source.get("interrupted"):
            return "the run was interrupted"
        if source.get("backgroundTaskId"):
            return "the run was backgrounded"
        if source.get("timedOutAfterMs"):
            return "the run hit the tool timeout"
        if source.get("returnCodeInterpretation"):
            return "a non-zero exit was reinterpreted as success"
    return None


def index_results(turn: list[dict]) -> dict[str, str | None]:
    """Map every recorded tool_use id to its problem string (None when clean)."""
    results: dict[str, str | None] = {}
    for entry in turn:
        if entry.get("type") != "user":
            continue
        extra = entry.get("toolUseResult")
        extra = extra if isinstance(extra, dict) else {}
        for block in blocks_of(entry):
            if block.get("type") == "tool_result":
                results[str(block.get("tool_use_id"))] = _result_problem(block, extra)
    return results


def summarize_turn(turn: list[dict]) -> tuple[list[str], list[str], bool, str | None]:
    """Return (edited files, shell writes, a credited run exists, nearest refusal) for the turn."""
    results = index_results(turn)
    edited, shell_writes, verified, nearest = [], [], False, None
    for entry in turn:
        if entry.get("type") != "assistant" or entry.get("isSidechain"):
            continue
        for block in blocks_of(entry):
            if block.get("type") != "tool_use":
                continue
            name = block.get("name")
            inputs = block.get("input") or {}
            if name in EDIT_TOOLS:
                edited.append(str(inputs.get("file_path") or inputs.get("notebook_path") or "?"))
            elif name in SHELL_TOOLS:
                command = str(inputs.get("command") or "")
                if VERIFY_RE.search(command):
                    reason = _credit_problem(command, str(block.get("id")), results)
                    verified = verified or reason is None
                    nearest = nearest if reason is None else f"{command.strip()[:80]} ({reason})"
                if SHELL_WRITE_RE.search(command):
                    shell_writes.append(command.strip().splitlines()[0][:80])
    return edited, shell_writes, verified, nearest


def _credit_problem(command: str, use_id: str, results: dict[str, str | None]) -> str | None:
    """Why this verifier call is not credited, or None when it is."""
    if use_id not in results:
        return "no result was recorded"
    return results[use_id] or creditable_command(command)


def _in_backticks(text: str, start: int, end: int) -> bool:
    open_tick = text.rfind("`", max(0, start - 200), start)
    if open_tick == -1:
        return False
    close_tick = text.find("`", end, min(len(text), end + 200))
    return close_tick != -1 and "`" not in text[open_tick + 1 : start]


def _in_blockquote(text: str, start: int) -> bool:
    line_start = text.rfind("\n", 0, start) + 1
    return text[line_start:start].lstrip().startswith(">")


def _ends_in_question(text: str, end: int) -> bool:
    for char in text[end : end + 160]:
        if char in ".!?\n":
            return char == "?"
    return False


def find_claims(text: str) -> list[str]:
    """Completion phrases in the assistant's own prose, minus quotes, code and questions."""
    text = FENCE_RE.sub("", text)
    found, seen = [], set()
    for pattern in CLAIM_PATTERNS:
        for match in pattern.finditer(text):
            phrase = " ".join(match.group(0).split())
            key = phrase.lower()
            if key in seen or _ends_in_question(text, match.end()):
                continue
            if _in_backticks(text, match.start(), match.end()):
                continue
            if _in_blockquote(text, match.start()):
                continue
            seen.add(key)
            found.append(phrase)
    return found[:MAX_LISTED]


def verdict(turn: list[dict]) -> str | None:
    """None when silent; otherwise the warning text."""
    text = last_assistant_text(turn)
    if text is None:
        return None
    claims = find_claims(text)
    if not claims:
        return None
    edited, shell_writes, verified, nearest = summarize_turn(turn)
    if verified or not (edited or shell_writes):
        return None

    touched = edited[:MAX_LISTED] + [f"shell: {cmd}" for cmd in shell_writes[:MAX_LISTED]]
    uncredited = f"Nearest verification run not credited: {nearest}\n" if nearest else ""
    return (
        "CLAIM WITHOUT EVIDENCE: this turn edited files and ended on a completion "
        "claim, but no credited verification run exists since the last prompt.\n"
        "Claimed:\n" + "\n".join(f'  - "{c}"' for c in claims) + "\n"
        "Touched:\n" + "\n".join(f"  - {t}" for t in touched) + "\n" + uncredited +
        "Run the check the claim rests on in its plain form (code-verify.py --check on "
        "the touched files at minimum; pytest/ctest where one exists; `cmd 2>&1`, never "
        "piped) and report its actual result, or scope the claim to what was verified by "
        "reading. Escape hatch: SS_CLAIM_CHECK=off."
    )


def session_entries(entries: list[dict], session_id: str) -> list[dict]:
    """Drop entries another session wrote; an entry without a session id is kept."""
    if not session_id:
        return entries
    kept = []
    for entry in entries:
        owner = entry.get("sessionId") or entry.get("session_id")
        if not owner or owner == session_id:
            kept.append(entry)
    return kept


def check(transcript_path: str, session_id: str = "") -> str | None:
    """Read the transcript, waiting briefly for the final assistant entry to land."""
    turn: list[dict] = []
    for attempt in range(RETRY_ATTEMPTS):
        turn = turn_entries(session_entries(load_entries(transcript_path), session_id))
        if last_assistant_text(turn) is not None:
            break
        if attempt < RETRY_ATTEMPTS - 1:
            time.sleep(RETRY_DELAY_S)
    return verdict(turn)


def main() -> None:
    try:
        if os.environ.get("SS_CLAIM_CHECK", "on").lower() == "off":
            sys.exit(0)
        event = json.load(sys.stdin)
        if event.get("stop_hook_active"):
            sys.exit(0)
        transcript_path = event.get("transcript_path")
        if transcript_path:
            message = check(str(transcript_path), str(event.get("session_id") or ""))
            if message is not None:
                print(json.dumps({"systemMessage": message}))
    except SystemExit:
        raise
    except Exception:
        pass
    sys.exit(0)


if __name__ == "__main__":
    main()
