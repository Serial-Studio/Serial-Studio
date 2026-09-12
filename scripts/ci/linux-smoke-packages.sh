#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# The glibc-2.28 ground truth: bare rockylinux:8 (AppImage extract + rpm install) and debian:10
# (deb install, EOL archive sources) containers, plus the modern-host check on the runner itself.
# A bare container has nothing preinstalled, so an incomplete bundle or an unsatisfiable
# dependency set fails here instead of on a user's machine.
#
# Driven by .github/actions/package-linux. CWD-independent.
#
# Environment:
#   SS_WORKSPACE   repository checkout the packages were written to
#   EXECUTABLE     artifact base name
#   VERSION        release version
#   ARCH_LABEL     artifact-name architecture (x64 / arm64)
#   UNIXNAME       installed executable name
#
set -euo pipefail
cd "${SS_WORKSPACE}"
APPIMAGE="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.AppImage"
DEB="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.deb"
RPM="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.rpm"

chmod +x "$APPIMAGE"
./"$APPIMAGE" --appimage-extract >/dev/null
grep -q '"packageType":"appimage"' squashfs-root/usr/bin/ss-config.json

env QT_QPA_PLATFORM=offscreen ./squashfs-root/AppRun --version
env QT_QPA_PLATFORM=offscreen ./squashfs-root/AppRun --selftest

docker run --rm -v "$PWD/squashfs-root:/app:ro" -e QT_QPA_PLATFORM=offscreen -e HOME=/tmp \
  rockylinux:8 /app/AppRun --version
docker run --rm -v "$PWD/squashfs-root:/app:ro" -e QT_QPA_PLATFORM=offscreen -e HOME=/tmp \
  rockylinux:8 /app/AppRun --selftest

docker run --rm -v "$PWD:/pkg:ro" -e QT_QPA_PLATFORM=offscreen -e HOME=/tmp rockylinux:8 \
  bash -ec "dnf install -y -q --nogpgcheck /pkg/$RPM && ${UNIXNAME} --version"

docker run --rm -v "$PWD:/pkg:ro" -e QT_QPA_PLATFORM=offscreen -e HOME=/tmp debian:10 \
  bash -ec "sed -i -e 's|deb.debian.org|archive.debian.org|g' -e '/security.debian.org/d' /etc/apt/sources.list; apt-get update -qq || true; apt-get install -y -q /pkg/$DEB || dpkg -i /pkg/$DEB; ${UNIXNAME} --version"

# Windowed gate: every check above runs offscreen, which is how a bundle with no
# GLX/EGL vendor libraries once sailed through CI and aborted on real desktops with
# "Could not initialize GLX". Xvfb + xcb forces the bundled GLVND stack to resolve a
# vendor and create a GL context; the GUI must still be alive when the kill fires.
command -v xvfb-run >/dev/null || sudo apt-get install -y -q xvfb
set +e
timeout -k 10 25 xvfb-run -a -s "-screen 0 1280x800x24" \
  env QT_QPA_PLATFORM=xcb HOME=/tmp ./squashfs-root/AppRun
rc=$?
set -e
if [ "$rc" != 137 ] && [ "$rc" != 124 ]; then
  echo "::error::windowed smoke exited early (rc=$rc); compat GL stack is broken"
  exit 1
fi

rm -rf squashfs-root
