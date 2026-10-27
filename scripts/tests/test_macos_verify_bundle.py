"""The macOS universal-bundle slice check, driven on canned otool/lipo output.

Issue #392: the x86_64 slice of every 4.x DMG was lifted out of its build tree undeployed, so it
still named the runner's Qt directory in its rpath, and lipo merged those load commands untouched.
The build, the signature, the notarization and the arm64 test job all passed.
"""

import importlib.util
from pathlib import Path

import pytest

SCRIPT = Path(__file__).resolve().parents[1] / "ci" / "macos-verify-bundle.py"
FAT_MAGIC = b"\xca\xfe\xba\xbe" + b"\0" * 16

RUNNER_QT = "/Users/runner/work/Serial-Studio/Serial-Studio/Qt-6.11.2-macOS-x64/6.11.2/macos/lib"

UNDEPLOYED = f"""Serial Studio Pro.app/Contents/MacOS/Serial-Studio-Pro (architecture x86_64):
Load command 12
          cmd LC_LOAD_DYLIB
      cmdsize 56
         name @rpath/QtSvg.framework/Versions/A/QtSvg (offset 24)
   time stamp 2 Thu Jan  1 00:00:02 1970
Load command 13
          cmd LC_LOAD_WEAK_DYLIB
      cmdsize 56
         name /usr/lib/swift/libswiftCore.dylib (offset 24)
Load command 14
          cmd LC_LOAD_DYLIB
      cmdsize 56
         name /usr/lib/libSystem.B.dylib (offset 24)
Load command 30
          cmd LC_RPATH
      cmdsize 40
         path {RUNNER_QT} (offset 12)
"""

DEPLOYED = UNDEPLOYED.replace(
    "@rpath/QtSvg.framework/Versions/A/QtSvg",
    "@executable_path/../Frameworks/QtSvg.framework/Versions/A/QtSvg",
).replace(f"path {RUNNER_QT}", "path @executable_path/../Frameworks")

PLUGIN = """libqcocoa.dylib:
Load command 1
          cmd LC_LOAD_DYLIB
      cmdsize 56
         name @rpath/QtCore.framework/Versions/A/QtCore (offset 24)
Load command 2
          cmd LC_RPATH
      cmdsize 40
         path @loader_path/../../Frameworks (offset 12)
"""

FRAMEWORK = """QtCore:
Load command 1
          cmd LC_ID_DYLIB
      cmdsize 56
         name @rpath/QtCore.framework/Versions/A/QtCore (offset 24)
"""


@pytest.fixture(scope="module")
def verify():
    spec = importlib.util.spec_from_file_location("macos_verify_bundle", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.fixture
def bundle(tmp_path):
    """A fake .app with a fat executable, two frameworks and one platform plugin."""
    app = tmp_path / "Serial Studio Pro.app"
    exe = app / "Contents" / "MacOS" / "Serial-Studio-Pro"
    svg = (
        app / "Contents" / "Frameworks" / "QtSvg.framework" / "Versions" / "A" / "QtSvg"
    )
    core = (
        app
        / "Contents"
        / "Frameworks"
        / "QtCore.framework"
        / "Versions"
        / "A"
        / "QtCore"
    )
    plugin = app / "Contents" / "PlugIns" / "platforms" / "libqcocoa.dylib"
    for path in (exe, svg, core, plugin):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(FAT_MAGIC)
    (app / "Contents" / "Info.plist").write_bytes(b"<plist/>")
    return app


def _fake_run(exe_output, plugin_output=PLUGIN):
    def run(*args):
        path = args[-1]
        if args[0] == "lipo":
            return "arm64\n" if path.endswith("libqcocoa.dylib") else "x86_64 arm64\n"
        arch = args[2]
        if path.endswith("libqcocoa.dylib"):
            return plugin_output
        if path.endswith("Serial-Studio-Pro"):
            return exe_output[arch]
        return FRAMEWORK

    return run


def test_parses_load_dylibs_and_rpaths_and_skips_weak_ones(verify):
    dylibs, rpaths = verify.load_commands(UNDEPLOYED)
    assert dylibs == [
        "@rpath/QtSvg.framework/Versions/A/QtSvg",
        "/usr/lib/libSystem.B.dylib",
    ]
    assert rpaths == [RUNNER_QT]


def test_an_undeployed_x86_64_slice_fails_while_the_arm64_one_passes(
    verify, bundle, capsys
):
    """The exact shape of issue #392: one slice deployed by macdeployqt, the other not."""
    verify.run = _fake_run({"arm64": DEPLOYED, "x86_64": UNDEPLOYED})
    assert verify.main(str(bundle)) == 1
    out = capsys.readouterr().out
    assert "::error::" in out
    assert "[x86_64] cannot load @rpath/QtSvg.framework/Versions/A/QtSvg" in out
    assert "[arm64]" not in out


def test_a_fully_deployed_bundle_passes(verify, bundle, capsys):
    verify.run = _fake_run(
        {"arm64": DEPLOYED, "x86_64": DEPLOYED.replace("arm64", "x86_64")}
    )
    assert verify.main(str(bundle)) == 0
    assert "0 unresolved" in capsys.readouterr().out


def test_a_bare_install_name_is_not_a_system_library(verify, bundle, capsys):
    """dyld would fall back to /usr/local/lib for it, which a user's machine does not have."""
    plugin = PLUGIN.replace("@rpath/QtCore.framework/Versions/A/QtCore", "libz.1.dylib")
    verify.run = _fake_run({"arm64": DEPLOYED, "x86_64": DEPLOYED}, plugin)
    assert verify.main(str(bundle)) == 1
    assert "cannot load libz.1.dylib" in capsys.readouterr().out


def test_a_reference_that_escapes_the_bundle_fails_even_if_it_exists(
    verify, bundle, tmp_path
):
    """An rpath naming a directory on the build machine resolves there and nowhere else."""
    outside = tmp_path / "QtSvg.framework" / "Versions" / "A"
    outside.mkdir(parents=True)
    (outside / "QtSvg").write_bytes(FAT_MAGIC)
    leaked = UNDEPLOYED.replace(f"path {RUNNER_QT}", f"path {tmp_path.as_posix()}")
    verify.run = _fake_run({"arm64": DEPLOYED, "x86_64": leaked})
    assert verify.main(str(bundle)) == 1


def test_symlinks_and_non_macho_files_are_skipped(verify, bundle):
    link = bundle / "Contents" / "Frameworks" / "QtCore.framework" / "QtCore"
    try:
        link.symlink_to(Path("Versions") / "A" / "QtCore")
    except (OSError, NotImplementedError):
        pytest.skip("symlinks need privileges on this platform")
    names = sorted(Path(path).name for path in verify.machos(str(bundle)))
    assert names == ["QtCore", "QtSvg", "Serial-Studio-Pro", "libqcocoa.dylib"]
