#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
"""Resolve every library reference of every Mach-O slice inside a deployed macOS .app.

Each slice of a fat Mach-O carries its own load commands and rpaths, and lipo merges them
untouched. The x86_64 half of the universal bundle can therefore name the build runner's Qt
directory while the arm64 half, which macdeployqt rewrote, loads fine: the build, the signature,
the notarization and the arm64 test job all pass, and the first Intel Mac aborts in dyld before
main (issue #392, every 4.x DMG up to 4.2.0).

Usage: macos-verify-bundle.py <path to .app>

Walks every Mach-O under the bundle, asks lipo for its architectures and otool for each slice's
LC_LOAD_DYLIB / LC_RPATH commands, and resolves every reference the way dyld would against the
bundle itself or the system library directories. Any reference that resolves nowhere inside those
is an error, including one that happens to exist on the machine running the check: a path on the
build runner is exactly the defect. Weak references are skipped, they are allowed to be missing.
Exit 1 on any unresolved reference. Driven by the build-macos job in ci.yml after the lipo steps.
"""

import os
import subprocess
import sys

SYSTEM = ("/usr/lib/", "/System/Library/")
LOADS = {
    "LC_LOAD_DYLIB",
    "LC_REEXPORT_DYLIB",
    "LC_LOAD_UPWARD_DYLIB",
    "LC_LAZY_LOAD_DYLIB",
}
MAGICS = (b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe", b"\xca\xfe\xba\xbe")


def run(*args):
    return subprocess.run(args, capture_output=True, text=True, check=True).stdout


def machos(app):
    """Every regular file under the bundle that starts with a thin or fat Mach-O magic."""
    for root, _, files in os.walk(app):
        for name in files:
            path = os.path.join(root, name)
            if os.path.islink(path):
                continue
            with open(path, "rb") as handle:
                if handle.read(4) in MAGICS:
                    yield path


def load_commands(text):
    """(dylib references, rpaths) parsed from one slice's `otool -l` output."""
    dylibs, rpaths, cmd = [], [], None
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("cmd "):
            cmd = line.split()[1]
        elif cmd in LOADS and line.startswith("name "):
            dylibs.append(line.split()[1])
        elif cmd == "LC_RPATH" and line.startswith("path "):
            rpaths.append(line.split()[1])
    return dylibs, rpaths


def expand(path, exe_dir, loader_dir):
    if path.startswith("@executable_path/"):
        return os.path.join(exe_dir, path[len("@executable_path/") :])
    if path.startswith("@loader_path/"):
        return os.path.join(loader_dir, path[len("@loader_path/") :])
    return path


def resolves(ref, app, exe_dir, loader_dir, rpaths):
    """Whether dyld would find `ref` inside the bundle or the system, never anywhere else."""
    if ref.startswith(SYSTEM):
        return True
    if ref.startswith("@rpath/"):
        candidates = [
            os.path.join(expand(rpath, exe_dir, loader_dir), ref[len("@rpath/") :])
            for rpath in rpaths
        ]
    else:
        candidates = [expand(ref, exe_dir, loader_dir)]
    for candidate in candidates:
        candidate = os.path.normpath(candidate)
        if candidate.startswith(app + os.sep) and os.path.exists(candidate):
            return True
    return False


def main(app):
    app = os.path.abspath(app)
    exe_dir = os.path.join(app, "Contents", "MacOS")
    errors, checked = [], 0
    for path in sorted(machos(app)):
        rel = os.path.relpath(path, app)
        for arch in run("lipo", "-archs", path).split():
            dylibs, rpaths = load_commands(run("otool", "-arch", arch, "-l", path))
            checked += 1
            for ref in dylibs:
                if not resolves(ref, app, exe_dir, os.path.dirname(path), rpaths):
                    errors.append(
                        f"{rel} [{arch}] cannot load {ref} (rpaths: {rpaths or '-'})"
                    )
    print(f"{checked} Mach-O slice(s) checked, {len(errors)} unresolved reference(s)")
    for error in errors:
        print(f"::error::{error}")
    return 1 if errors else 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__.split("\n\n")[1])
    sys.exit(main(sys.argv[1]))
