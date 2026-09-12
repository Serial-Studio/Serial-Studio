#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# Signs the AppImage, the deb, the rpm and the raw binary, and verifies every artifact right
# after it is signed, so a broken signature fails here instead of on a user's machine. A no-op
# unless gpg-prepare.sh exported SS_GPG_SIGN=1.
#
# Driven by .github/actions/package-linux. CWD-independent.
#
# Environment:
#   SS_WORKSPACE    repository checkout the packages were written to
#   SS_GPG_SIGN     '1' when gpg-prepare.sh imported an identity
#   GPG_PASSPHRASE  passphrase for that key (optional)
#   GPG_KEY_ID      key id the artifacts are signed with
#   EXECUTABLE      artifact base name
#   VERSION         release version
#   ARCH_LABEL      artifact-name architecture (x64 / arm64)
#   UNIXNAME        built executable name under build/app
#
set -euo pipefail
cd "${SS_WORKSPACE}"
if [ "${SS_GPG_SIGN:-0}" != "1" ]; then
  echo "GPG signing secrets not set; skipping Linux signing."
  exit 0
fi
sudo apt-get install -y debsigs
APPIMAGE="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.AppImage"
DEB="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.deb"
RPM="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.rpm"
BIN="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}"
cp "build/app/${UNIXNAME}" "$BIN"

gpg --export --armor "$GPG_KEY_ID" > signing-public-key.asc

rpm --import signing-public-key.asc
rpmsign --addsign "$RPM"
rpm -Kv "$RPM"
rpm -K "$RPM" | grep -qi 'signatures OK'

debsigs --sign=origin --default-key="$GPG_KEY_ID" "$DEB"
ar t "$DEB" | grep -q '_gpgorigin'

chmod +x "$APPIMAGE"
./"$APPIMAGE" --appimage-signature | grep -q 'BEGIN PGP SIGNATURE'

for f in "$APPIMAGE" "$DEB" "$RPM" "$BIN"; do
  gpg --batch --yes --pinentry-mode loopback \
    ${GPG_PASSPHRASE:+--passphrase "$GPG_PASSPHRASE"} \
    --local-user "$GPG_KEY_ID" \
    --detach-sign --armor --output "$f.asc" "$f"
  gpg --verify "$f.asc" "$f"
done
