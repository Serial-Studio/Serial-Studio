#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# Runs linuxdeploy-plugin-native_packages over the converted AppDir. ldnp emits minimal metadata
# (no ELF find-requires), which is what keeps the packages installable on glibc 2.28 hosts; the
# smoke gate in linux-smoke-packages.sh proves that per run instead of trusting it.
#
# Driven by .github/actions/package-linux. CWD-independent.
#
# Environment:
#   SS_WORKSPACE   repository checkout the build tree and the artifacts live under
#   UNIXNAME       package / executable name
#   EXECUTABLE     artifact base name
#   VERSION        release version
#   DESCRIPTION    package description
#   DEB_ARCH       LDNP_META_ARCHITECTURE (amd64 / arm64)
#   MACHINE_ARCH   LDNP_META_BUILD_ARCH (x86_64 / aarch64)
#   CONFIG_ARCH    arch stamped into ss-config.json (x86_64 / arm64)
#   ARCH_LABEL     artifact-name architecture (x64 / arm64)
#
set -euo pipefail
cd "${SS_WORKSPACE}"
python3 -m venv /tmp/ldnp-venv
/tmp/ldnp-venv/bin/pip install --upgrade pip
/tmp/ldnp-venv/bin/pip install \
  "git+https://github.com/linuxdeploy/linuxdeploy-plugin-native_packages.git@91638818c35a240d0874b7de9cdab0cb3d3655ea"

# ldnp writes %files entries unquoted, so rpmbuild splits paths that contain
# spaces (the converted AppDir bundles ALSA UCM profiles like
# "Google Nexus 7 ALC5642.conf"); quote every entry in the spec template
sed -i 's|^{{ file }}$|"{{ file }}"|' \
  /tmp/ldnp-venv/lib/python3.*/site-packages/ldnp/templates/rpm/spec
grep -q '^"{{ file }}"$' /tmp/ldnp-venv/lib/python3.*/site-packages/ldnp/templates/rpm/spec

export LINUXDEPLOY_OUTPUT_APP_NAME="${UNIXNAME}"
export LINUXDEPLOY_OUTPUT_VERSION="${VERSION}"
export LDNP_META_PACKAGE_NAME="${UNIXNAME}"
export LDNP_META_DESCRIPTION="${DESCRIPTION}"
export LDNP_META_SHORT_DESCRIPTION="${DESCRIPTION}"
export LDNP_META_ARCHITECTURE="${DEB_ARCH}"
export LDNP_META_BUILD_ARCH="${MACHINE_ARCH}"

printf '{"packageType":"deb","arch":"%s"}\n' "${CONFIG_ARCH}" > build/app/AppDir-compat/usr/bin/ss-config.json
/tmp/ldnp-venv/bin/linuxdeploy-plugin-native_packages \
  --appdir build/app/AppDir-compat \
  --build deb

printf '{"packageType":"rpm","arch":"%s"}\n' "${CONFIG_ARCH}" > build/app/AppDir-compat/usr/bin/ss-config.json
/tmp/ldnp-venv/bin/linuxdeploy-plugin-native_packages \
  --appdir build/app/AppDir-compat \
  --build rpm

mv "${UNIXNAME}"_*.deb "${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.deb"
mv "${UNIXNAME}"_*.rpm "${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.rpm"

dpkg-deb --fsys-tarfile "${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.deb" \
  | tar -xO --wildcards '*/ss-config.json' | grep -q '"packageType":"deb"'
rpm2cpio "${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.rpm" \
  | cpio --quiet -i --to-stdout '*ss-config.json' | grep -q '"packageType":"rpm"'
