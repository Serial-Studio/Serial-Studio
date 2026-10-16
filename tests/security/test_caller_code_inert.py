"""
Caller-supplied code is inert in every dry run and compile check.

Spec 0098, AC1 (R5): a dry run or compile check, whoever requests it, leaves no trace outside
itself. Each command below receives code whose top level (and, where the command calls it,
whose entry point) tries to start a program, replace the control script, write a data-table
variable, write to the device, fire an action and post a notification. None of those effects
may happen, the API calls must report `dryRun`, and valid or failing code must still return the
command's usual diagnostics.

Covered commands (JS and Lua where the command takes both):

  project.frameParser.dryCompile    project.frameParser.dryRun    project.dryRun.endToEnd
  project.dataset.transform.dryRun  project.outputWidget.dryRun   project.painter.dryRun
  project.transformLibrary.dryRun   controlScript.dryRun          project.validate

project.validate compiles the PROJECT's code, so those cases load a project first. The load
asks the script-change consent like any remote project load; when the instance does not grant
it (a GUI prompt nobody answered, a headless run without SERIAL_STUDIO_API_AUTO_CONSENT) the
validate cases skip. Their baselines are taken after the load, so only what validate itself
does can fail them.

Runs against a normal GUI instance with the API server enabled (loopback, no token). Every
case starts and ends on `project.new`, which discards the open project, exactly like the
integration suite's `clean_state`. The table cases need Variables (data tables); without them
the table check is skipped and the rest still runs.
"""

import json
import re
import subprocess
import sys
import time
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import APIError

pytestmark = [pytest.mark.security]

JS = 0
LUA = 1
LANGUAGE_NAMES = {JS: "js", LUA: "lua"}

SETTLE_SECONDS = 0.6
MARKER_QUIET_SECONDS = 1.5
MARKER_SETTLE_MAX_SECONDS = 6.0
REGISTER_WAIT_SECONDS = 3.0
TABLE_NAME = "InertTable"
REGISTER_NAME = "level"
REGISTER_BASELINE = 5
ATTACK_VALUE = 999

MARKER_SNIPPET = "import sys; open(sys.argv[1], 'a').write('x')"

_JS_ATTACK = """
function __attack() {
  try { apiCall("system.exec", {program: @PROGRAM@, args: @ARGS@}); } catch (e) {}
  try { apiCall("controlScript.setCode", {code: @EVIL@}); } catch (e) {}
  try { apiCall("project.dataTable.setValue", {table: @TABLE@, name: @REGISTER@, value: @VALUE@}); } catch (e) {}
  try { apiCall("io.writeData", {data: "pwned"}); } catch (e) {}
  try { tableSet(@TABLE@, @REGISTER@, @VALUE@); } catch (e) {}
  try { deviceWrite("pwned"); } catch (e) {}
  try { actionFire(0); } catch (e) {}
  try { notifyInfo(@TOKEN@); } catch (e) {}
}
__attack();
"""

_LUA_ATTACK = """
function __attack()
  pcall(apiCall, "system.exec", {program = @PROGRAM@, args = @ARGS@})
  pcall(apiCall, "controlScript.setCode", {code = @EVIL@})
  pcall(apiCall, "project.dataTable.setValue", {table = @TABLE@, name = @REGISTER@, value = @VALUE@})
  pcall(apiCall, "io.writeData", {data = "pwned"})
  pcall(tableSet, @TABLE@, @REGISTER@, @VALUE@)
  pcall(deviceWrite, "pwned")
  pcall(actionFire, 0)
  pcall(notifyInfo, @TOKEN@)
end
__attack()
"""

_BODIES = {
    ("parse", JS): 'function parse(frame) { __attack(); return frame.split(","); }',
    (
        "parse",
        LUA,
    ): """function parse(frame)
  __attack()
  local cells = {}
  for cell in string.gmatch(frame, "[^,]+") do cells[#cells + 1] = cell end
  return cells
end""",
    ("transform", JS): "function transform(v) { __attack(); return v * 2; }",
    ("transform", LUA): "function transform(v)\n  __attack()\n  return v * 2\nend",
    (
        "transmit",
        JS,
    ): "function transmit(v) { __attack(); return String.fromCharCode(1, v & 0xFF); }",
    ("paint", JS): "function paint(ctx, w, h) { __attack(); }",
    ("library", JS): "function f(v) { return v; }",
    ("library", LUA): "function f(v) return v end",
    ("control", JS): "function setup() { __attack(); }\nfunction loop() {}",
}

_SYNTAX_ERRORS = {
    ("parse", JS): "function parse(frame { return [frame]; }",
    ("parse", LUA): "function parse(frame\n",
    ("transform", JS): "function transform(v { return v; }",
    ("transform", LUA): "function transform(v\n",
    ("transmit", JS): "function transmit(v) { return [1,",
    ("paint", JS): "function paint(ctx, w, h) {",
    ("library", JS): "function f(v) {\n  return v +;\n}",
    ("library", LUA): "function f( return",
    ("control", JS): "function setup( {",
}

_UNAVAILABLE_PARSER = {
    JS: """function parse(frame) {
  var replies = [apiCall("controlScript.setCode", {code: @EVIL@}), deviceWrite("x"), actionFire(0)];
  return replies.map(function (r) { return String(r.ok) + ":" + String(r.error); });
}""",
    LUA: """function parse(frame)
  local replies = {apiCall("controlScript.setCode", {code = @EVIL@}), deviceWrite("x"), actionFire(0)}
  local out = {}
  for i = 1, #replies do out[i] = tostring(replies[i].ok) .. ":" .. tostring(replies[i].error) end
  return out
end""",
}

_UNAVAILABLE_TRANSFORM = {
    JS: """function transform(v) {
  var r = apiCall("controlScript.setCode", {code: @EVIL@});
  return String(r.ok) + ":" + String(r.error);
}""",
    LUA: """function transform(v)
  local r = apiCall("controlScript.setCode", {code = @EVIL@})
  return tostring(r.ok) .. ":" .. tostring(r.error)
end""",
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _lua_string(text: str) -> str:
    """Render text as a Lua 5.1 string literal (decimal escapes; no \\u, no \\x)."""
    out = []
    for byte in text.encode("utf-8"):
        char = chr(byte)
        if byte < 128 and (char.isalnum() or char in " /_-.:=,"):
            out.append(char)
        else:
            out.append("\\%03d" % byte)

    return '"' + "".join(out) + '"'


def _fill(template: str, values: dict[str, str]) -> str:
    """Single-pass @NAME@ substitution, so a value can never be re-expanded."""
    return re.sub(r"@[A-Z]+@", lambda match: values[match.group(0)], template)


def _is_consent_refusal(error: APIError) -> bool:
    text = error.message.lower()
    return error.code == "CONSENT_REQUIRED" or "denied" in text or "consent" in text


def _attempt(call: Callable[..., Any], *args: Any) -> Any:
    """Return the command's result, or the APIError it raised."""
    try:
        return call(*args)
    except APIError as error:
        return error


class Sandbox:
    """
    Builds the hostile code, takes baselines of everything it targets and checks them again.

    The targets: a marker file a spawned program would append to, the control script, one
    data-table variable and the notification log.
    """

    def __init__(self, client, tmp_path):
        self.client = client
        self.marker = tmp_path / "caller-code.marker"
        self.token = uuid.uuid4().hex[:12]
        self.evil_control = f"function setup() {{}}\n// caller-supplied {self.token}\n"
        self.control_before = ""
        self.marker_before = 0
        self.table_before = None
        self.notifications_before = 0
        self.has_table = False

    # -- lifecycle ----------------------------------------------------------

    def start(self) -> None:
        self._drop_leftover_link()
        self.client.command("project.new")
        self.has_table = self._create_table()
        self.baseline()

    def _drop_leftover_link(self) -> None:
        """A link left open by an earlier test freezes the live table layout (a live session
        never re-lays the store), so the table this sandbox adds would never become readable.
        """
        try:
            self.client.command("io.disconnect")
        except (APIError, ConnectionError, TimeoutError):
            pass

    def finish(self) -> None:
        try:
            self.client.command("project.new")
        except (APIError, ConnectionError, TimeoutError):
            pass

    def adopt_loaded_project(self) -> None:
        """Re-baseline after a project load; the table (if any) came with the project. The load
        compiles the parser in the LIVE engine, where a consented launch really runs, so the
        marker is let settle first or that launch's late write is blamed on what follows.
        """
        self.has_table = self._wait_for_register()
        self._wait_for_marker_to_settle()
        self.baseline()

    def _wait_for_marker_to_settle(self) -> None:
        deadline = time.monotonic() + MARKER_SETTLE_MAX_SECONDS
        size = self._marker_size()
        stable_since = time.monotonic()
        while time.monotonic() < deadline:
            time.sleep(0.1)
            current = self._marker_size()
            if current != size:
                size = current
                stable_since = time.monotonic()
            elif time.monotonic() - stable_since >= MARKER_QUIET_SECONDS:
                return

    def baseline(self) -> None:
        self.control_before = self._control_code()
        self.marker_before = self._marker_size()
        self.table_before = self._read_register() if self.has_table else None
        self.notifications_before = self._notification_count()

    # -- observations -------------------------------------------------------

    def _control_code(self) -> str:
        return self.client.command("controlScript.getCode").get("code", "")

    def _marker_size(self) -> int:
        return self.marker.stat().st_size if self.marker.exists() else 0

    def _read_register(self):
        try:
            reply = self.client.command(
                "project.dataTable.getValue",
                {"table": TABLE_NAME, "name": REGISTER_NAME},
            )
        except APIError:
            return None

        return reply.get("value")

    def _wait_for_register(self) -> bool:
        deadline = time.monotonic() + REGISTER_WAIT_SECONDS
        while time.monotonic() < deadline:
            if self._read_register() == REGISTER_BASELINE:
                return True

            time.sleep(0.1)

        return False

    def _create_table(self) -> bool:
        try:
            self.client.command("project.dataTable.add", {"name": TABLE_NAME})
            self.client.command(
                "project.dataTable.addRegister",
                {
                    "table": TABLE_NAME,
                    "name": REGISTER_NAME,
                    "computed": True,
                    "defaultValue": REGISTER_BASELINE,
                },
            )
        except APIError:
            return False

        return self._wait_for_register()

    def _notification_count(self) -> int:
        """Times the token appears in the log; a count, since a project load's live parser may
        already have posted it before the baseline."""
        try:
            listing = self.client.command("notifications.list")
        except APIError:
            return 0

        return json.dumps(listing).count(self.token)

    def assert_inert(self, what: str) -> None:
        """Nothing the hostile code attempted may have happened."""
        deadline = time.monotonic() + SETTLE_SECONDS
        while time.monotonic() < deadline and self._marker_size() == self.marker_before:
            time.sleep(0.05)

        assert (
            self._marker_size() == self.marker_before
        ), f"{what}: caller code started a program (the marker file grew)"
        assert (
            self._control_code() == self.control_before
        ), f"{what}: caller code replaced the control script"
        if self.has_table:
            assert (
                self._read_register() == self.table_before
            ), f"{what}: caller code changed the live data-table variable"

        assert (
            self._notification_count() == self.notifications_before
        ), f"{what}: caller code posted a notification"

    # -- code builders ------------------------------------------------------

    def attack(self, language: int) -> str:
        literal = json.dumps if language == JS else _lua_string
        opener, closer = ("[", "]") if language == JS else ("{", "}")
        args = ["-c", MARKER_SNIPPET, str(self.marker)]
        values = {
            "@PROGRAM@": literal(sys.executable),
            "@ARGS@": opener + ", ".join(literal(arg) for arg in args) + closer,
            "@EVIL@": literal(self.evil_control),
            "@TABLE@": literal(TABLE_NAME),
            "@REGISTER@": literal(REGISTER_NAME),
            "@VALUE@": str(ATTACK_VALUE),
            "@TOKEN@": literal(self.token),
        }
        template = _JS_ATTACK if language == JS else _LUA_ATTACK
        return _fill(template, values)

    def code(self, role: str, language: int, kind: str) -> str:
        """kind: valid (attack then a working entry point), failing (attack then an error)
        or syntax (a plain syntax error, no attack)."""
        if kind == "syntax":
            return _SYNTAX_ERRORS[(role, language)]

        body = _BODIES[(role, language)]
        if kind == "valid":
            return self.attack(language) + "\n" + body + "\n"

        boom = 'throw new Error("boom");' if language == JS else 'error("boom")'
        return self.attack(language) + "\n" + boom + "\n" + body + "\n"

    def unavailable_probe(self, templates: dict[int, str], language: int) -> str:
        literal = json.dumps if language == JS else _lua_string
        return _fill(templates[language], {"@EVIL@": literal(self.evil_control)})


@pytest.fixture
def sandbox(api_client, tmp_path):
    box = Sandbox(api_client, tmp_path)
    box.start()
    yield box
    box.finish()


# ---------------------------------------------------------------------------
# The commands under test
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class Case:
    command: str
    role: str
    languages: tuple
    call: Callable[..., Any]
    check_valid: Callable[[Any], None]
    check_failed: Callable[[Any], None]


def _call_dry_compile(client, code, language):
    return client.command(
        "project.frameParser.dryCompile", {"code": code, "language": language}
    )


def _call_dry_run(client, code, language):
    return client.command(
        "project.frameParser.dryRun",
        {
            "code": code,
            "language": language,
            "inputBytes": "1,2,3\n",
            "frameDetection": 0,
            "frameEnd": "\n",
        },
    )


def _call_transform(client, code, language):
    return client.command(
        "project.dataset.transform.dryRun",
        {"code": code, "language": language, "values": [1, 2]},
    )


def _call_end_to_end(client, code, language):
    return client.command(
        "project.dryRun.endToEnd",
        {"sampleFrame": "1,2,3", "code": code, "language": language},
    )


def _call_output_widget(client, code, language):
    return client.command(
        "project.outputWidget.dryRun", {"code": code, "inputValue": 7}
    )


def _call_painter(client, code, language):
    return client.command("project.painter.dryRun", {"code": code})


def _call_library(client, code, language):
    return client.command(
        "project.transformLibrary.dryRun",
        {"code": code, "language": LANGUAGE_NAMES[language]},
    )


def _call_control(client, code, language):
    return client.command("controlScript.dryRun", {"code": code})


def _valid_compile(result):
    assert result["ok"] is True, result


def _valid_dry_run(result):
    assert result["ok"] is True, result
    assert result["frameCount"] == 1, result
    assert result["frames"][0]["rows"] == [["1", "2", "3"]], result


def _valid_transform(result):
    assert result["ok"] is True, result
    assert result["outputs"] == [2, 4], result


def _valid_end_to_end(result):
    assert result["ok"] is True, result
    assert result["frames"][0]["rows"][0]["rawCells"] == ["1", "2", "3"], result


def _valid_output_widget(result):
    assert result["ok"] is True and result["hasTransmit"] is True, result
    assert result["sampleRun"]["ok"] is True, result
    assert result["sampleRun"]["byteCount"] == 2, result


def _valid_painter(result):
    assert result["ok"] is True and result["hasPaint"] is True, result


def _valid_library(result):
    assert result["valid"] is True, result


def _valid_control(result):
    assert result["valid"] is True and result["hasSetup"] is True, result


def _failed_with_error(outcome):
    assert isinstance(outcome, APIError), f"expected the usual failure, got {outcome!r}"
    assert outcome.code == "EXECUTION_ERROR", outcome


def _failed_flag(flag, message):
    def check(outcome):
        assert isinstance(outcome, dict), f"expected a diagnostic, got {outcome!r}"
        assert outcome[flag] is False, outcome
        assert outcome.get(message), outcome

    return check


CASES = [
    Case(
        "project.frameParser.dryCompile",
        "parse",
        (JS, LUA),
        _call_dry_compile,
        _valid_compile,
        _failed_flag("ok", "error"),
    ),
    Case(
        "project.frameParser.dryRun",
        "parse",
        (JS, LUA),
        _call_dry_run,
        _valid_dry_run,
        _failed_with_error,
    ),
    Case(
        "project.dataset.transform.dryRun",
        "transform",
        (JS, LUA),
        _call_transform,
        _valid_transform,
        _failed_with_error,
    ),
    Case(
        "project.dryRun.endToEnd",
        "parse",
        (JS, LUA),
        _call_end_to_end,
        _valid_end_to_end,
        _failed_with_error,
    ),
    Case(
        "project.outputWidget.dryRun",
        "transmit",
        (JS,),
        _call_output_widget,
        _valid_output_widget,
        _failed_flag("ok", "compileError"),
    ),
    Case(
        "project.painter.dryRun",
        "paint",
        (JS,),
        _call_painter,
        _valid_painter,
        _failed_flag("ok", "compileError"),
    ),
    Case(
        "project.transformLibrary.dryRun",
        "library",
        (JS, LUA),
        _call_library,
        _valid_library,
        _failed_flag("valid", "error"),
    ),
    Case(
        "controlScript.dryRun",
        "control",
        (JS,),
        _call_control,
        _valid_control,
        _failed_flag("valid", "error"),
    ),
]


def _case_params():
    return [
        pytest.param(case, language, id=f"{case.command}-{LANGUAGE_NAMES[language]}")
        for case in CASES
        for language in case.languages
    ]


# ---------------------------------------------------------------------------
# Harness self-checks
# ---------------------------------------------------------------------------


def test_marker_program_creates_its_marker(tmp_path):
    """The program the hostile code asks for really leaves a marker, so 'absent' means unrun."""
    marker = tmp_path / "control.marker"
    subprocess.run(
        [sys.executable, "-c", MARKER_SNIPPET, str(marker)], check=True, timeout=15
    )
    assert marker.stat().st_size == 1


def test_table_baseline_is_observable(sandbox):
    """The variable the code targets reads back, so 'unchanged' means untouched."""
    if not sandbox.has_table:
        pytest.skip("data tables (Variables) unavailable; the table check is skipped")

    assert sandbox.table_before == REGISTER_BASELINE


# ---------------------------------------------------------------------------
# Dry runs and compile checks
# ---------------------------------------------------------------------------


@pytest.mark.parametrize("case,language", _case_params())
def test_valid_code_is_inert_and_still_accepted(sandbox, api_client, case, language):
    """Hostile top-level and entry-point code has no effect, and the verdict is unchanged."""
    code = sandbox.code(case.role, language, "valid")

    result = case.call(api_client, code, language)

    case.check_valid(result)
    sandbox.assert_inert(case.command)


@pytest.mark.parametrize("case,language", _case_params())
def test_failing_code_is_inert_and_still_refused(sandbox, api_client, case, language):
    """Code that attacks and then errors, or never parses, keeps the usual failure shape."""
    for kind in ("failing", "syntax"):
        code = sandbox.code(case.role, language, kind)

        outcome = _attempt(case.call, api_client, code, language)

        case.check_failed(outcome)

    sandbox.assert_inert(case.command)


@pytest.mark.parametrize("language", [JS, LUA], ids=["js", "lua"])
def test_frame_parser_dry_run_reports_the_api_as_unavailable(
    sandbox, api_client, language
):
    """apiCall, deviceWrite and actionFire answer {ok:false, error:'dryRun'} inside the parser."""
    code = sandbox.unavailable_probe(_UNAVAILABLE_PARSER, language)

    result = _call_dry_run(api_client, code, language)

    assert result["ok"] is True, result
    assert result["frames"][0]["rows"] == [["false:dryRun"] * 3], result
    sandbox.assert_inert("project.frameParser.dryRun")


@pytest.mark.parametrize("language", [JS, LUA], ids=["js", "lua"])
def test_transform_dry_run_reports_the_api_as_unavailable(
    sandbox, api_client, language
):
    """apiCall answers {ok:false, error:'dryRun'} inside a transform."""
    code = sandbox.unavailable_probe(_UNAVAILABLE_TRANSFORM, language)

    result = _call_transform(api_client, code, language)

    assert result["ok"] is True, result
    assert result["outputs"] == ["false:dryRun", "false:dryRun"], result
    sandbox.assert_inert("project.dataset.transform.dryRun")


# ---------------------------------------------------------------------------
# project.validate compiles the project's own code
# ---------------------------------------------------------------------------


def _validate_project(parser_code: str, language: int) -> dict:
    """A project whose source 0 carries the parser, plus the table the code targets."""
    return {
        "title": "Inert validate",
        "frameStart": "/*",
        "frameEnd": "*/",
        "decoder": 0,
        "decoderMethod": 0,
        "checksum": "",
        "checksumAlgorithm": "",
        "frameDetection": 1,
        "sources": [
            {
                "sourceId": 0,
                "title": "Device A",
                "busType": 0,
                "frameStart": "/*",
                "frameEnd": "*/",
                "frameDetection": 1,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": parser_code,
                "frameParserLanguage": language,
                "connectionSettings": {},
            }
        ],
        "tables": [
            {
                "name": TABLE_NAME,
                "registers": [
                    {
                        "name": REGISTER_NAME,
                        "type": "computed",
                        "value": REGISTER_BASELINE,
                    }
                ],
            }
        ],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {
                        "title": "X",
                        "units": "",
                        "value": "%1",
                        "index": 1,
                        "min": 0,
                        "max": 100,
                    }
                ],
            }
        ],
        "actions": [],
    }


@pytest.mark.parametrize("language", [JS, LUA], ids=["js", "lua"])
@pytest.mark.parametrize("kind", ["valid", "failing"])
def test_project_validate_compiles_the_project_inert(
    sandbox, api_client, language, kind
):
    """validate's scratch compile of the parser has no effect and still reports the verdict."""
    parser = sandbox.code("parse", language, kind)
    project = _validate_project(parser, language)
    try:
        api_client.command("project.loadJson", {"config": project})
    except APIError as error:
        if _is_consent_refusal(error):
            pytest.skip(
                f"project.loadJson needs the script-change consent here: {error}"
            )

        raise

    sandbox.adopt_loaded_project()

    result = api_client.command("project.validate")

    parser_issues = [
        issue
        for issue in result["issues"]
        if issue.get("level") == "error" and "frameParser" in issue.get("location", "")
    ]
    if kind == "valid":
        assert not parser_issues, result
    else:
        assert result["ok"] is False, result
        assert parser_issues, result

    sandbox.assert_inert("project.validate")
