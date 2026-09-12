#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# Builds the AppDir with linuxdeploy and its Qt plugin; the SQL drivers and the TIFF reader
# are pruned first so the image carries only what the app loads. linuxdeploy-plugin-qt deploys
# ONLY libqxcb by default, so the wayland platform plugin is requested explicitly via
# EXTRA_PLATFORM_PLUGINS (a single libqwayland.so since Qt 6.10 merged the generic/egl pair and
# moved the client into qtbase) and its shell/decoration/graphics integration plugin dirs are
# copied by hand -- without them Plasma/GNOME wayland sessions are forced through XWayland+GLX.
# Two more things the tool cannot bundle by itself are added afterwards: the offscreen platform
# plugin (the integration tests run headless), and Qt WebEngine's NSS crypto backends, which NSS
# dlopen()s (softokn/freebl/nssckbi) so an ldd-based bundler never sees them and leaves a
# half-bundled NSS that crashes against the host's newer copies. No AppImage is packed here: the
# compat conversion in linux-create-appimage.sh produces the only Linux AppImage shipped, so this
# AppDir is purely the input tree for the deb/rpm/compat steps.
#
# Driven by .github/actions/package-linux. CWD-independent.
#
# Environment:
#   SS_WORKSPACE      repository checkout the build tree lives under
#   MACHINE_ARCH      linuxdeploy asset architecture (x86_64 / aarch64)
#   QT_PLUGINS_PATH   plugins directory of the Qt this job built against
#   QML_DIR           QML source path handed to linuxdeploy-plugin-qt (relative to build/app)
#   UNIXNAME          executable / desktop-entry name
#
set -eo pipefail
cd "${SS_WORKSPACE}/build/app"
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20240109-1/linuxdeploy-${MACHINE_ARCH}.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/linuxdeploy-plugin-qt-${MACHINE_ARCH}.AppImage
chmod +x linuxdeploy-${MACHINE_ARCH}.AppImage
chmod +x linuxdeploy-plugin-qt-${MACHINE_ARCH}.AppImage

export QML_SOURCES_PATHS="${QML_DIR}"
export EXTRA_PLATFORM_PLUGINS="libqwayland.so"

rm -f "$QT_PLUGINS_PATH/imageformats/libqtiff.so"
find "$QT_PLUGINS_PATH/sqldrivers" -name 'libqsql*.so' ! -name 'libqsqlite.so' -delete

./linuxdeploy-${MACHINE_ARCH}.AppImage --appdir AppDir -e ${UNIXNAME} -i ../../app/deploy/linux/${UNIXNAME}.svg -d ../../app/deploy/linux/${UNIXNAME}.desktop --plugin qt
mkdir -p AppDir/usr/plugins/canbus
if [ -d "$QT_PLUGINS_PATH/canbus" ]; then
  cp -r "$QT_PLUGINS_PATH/canbus/"* AppDir/usr/plugins/canbus/ || echo "Warning: Could not copy canbus plugins"
else
  echo "Warning: Qt canbus plugins not found at $QT_PLUGINS_PATH/canbus"
fi

mkdir -p AppDir/usr/plugins/opcua
if [ -d "$QT_PLUGINS_PATH/opcua" ]; then
  cp -r "$QT_PLUGINS_PATH/opcua/"* AppDir/usr/plugins/opcua/ || echo "Warning: Could not copy opcua plugins"
else
  echo "Warning: Qt opcua plugins not found at $QT_PLUGINS_PATH/opcua"
fi

for d in wayland-decoration-client wayland-graphics-integration-client wayland-shell-integration; do
  if [ -d "$QT_PLUGINS_PATH/$d" ]; then
    mkdir -p "AppDir/usr/plugins/$d"
    cp -r "$QT_PLUGINS_PATH/$d/"* "AppDir/usr/plugins/$d/"
  fi
done
if [ ! -f AppDir/usr/plugins/platforms/libqwayland.so ]; then
  echo "::error::wayland platform plugin missing from AppDir"
  exit 1
fi

mkdir -p AppDir/usr/plugins/platforms
if [ -f "$QT_PLUGINS_PATH/platforms/libqoffscreen.so" ]; then
  cp "$QT_PLUGINS_PATH/platforms/libqoffscreen.so" AppDir/usr/plugins/platforms/
else
  echo "Warning: offscreen platform plugin not found at $QT_PLUGINS_PATH/platforms"
fi

have_softokn=0
for d in /usr/lib/*/nss /usr/lib/*-linux-gnu /usr/lib64; do
  [ -d "$d" ] || continue
  for m in libsoftokn3 libfreebl3 libfreeblpriv3 libnssckbi libnssdbm3; do
    if [ -f "$d/$m.so" ]; then
      cp -a "$d/$m.so" AppDir/usr/lib/
      [ "$m" = libsoftokn3 ] && have_softokn=1
    fi
  done
done
if [ "$have_softokn" = 0 ]; then
  echo "::error::libsoftokn3.so not found; NSS bundle incomplete (WebEngine will crash)"
  exit 1
fi

rm linuxdeploy-${MACHINE_ARCH}.AppImage
rm linuxdeploy-plugin-qt-${MACHINE_ARCH}.AppImage
