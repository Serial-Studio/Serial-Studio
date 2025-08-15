#!/bin/bash

#
# Serial Studio - https://github.com/alex-spataru/serial-studio
#
# Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

set -e

#------------------------------------------------------------------------------
# Config
#------------------------------------------------------------------------------

QT_VERSION="${QT_VERSION:-6.9.1}"
QT_ARCH="${QT_ARCH:-x64}"
QT_USERNAME="${QT_USERNAME:?Must provide QT_USERNAME}"
QT_PASSWORD="${QT_PASSWORD:?Must provide QT_PASSWORD}"

#------------------------------------------------------------------------------
# Operating system detection
#------------------------------------------------------------------------------

OS_NAME="$(uname -s)"
case "$OS_NAME" in
  Linux*)   OS_TAG="Linux" ;;
  Darwin*)  OS_TAG="macOS" ;;
  *)        echo "Unsupported OS: $OS_NAME"; exit 1 ;;
esac

#------------------------------------------------------------------------------
# Create output directory with Qt version & operating system
#------------------------------------------------------------------------------

QT_OUTPUT_DIR="${QT_OUTPUT_DIR:-Qt-${QT_VERSION}-${OS_TAG}-${QT_ARCH}}"

#------------------------------------------------------------------------------
# Build alias package name
#------------------------------------------------------------------------------

ALIAS_PACKAGE="qt${QT_VERSION}-full"

#------------------------------------------------------------------------------
# Setup installer URLs
#------------------------------------------------------------------------------

CACHE_DIR="$HOME/.qt_cache"
mkdir -p "$CACHE_DIR"

case "$OS_NAME" in
  Linux*)
    QT_INSTALLER_URL="https://download.qt.io/official_releases/online_installers/qt-online-installer-linux-${QT_ARCH}-online.run"
    INSTALLER_PATH="$CACHE_DIR/qt_installer.run"
    ;;
  Darwin*)
    QT_INSTALLER_URL="https://download.qt.io/official_releases/online_installers/qt-online-installer-mac-${QT_ARCH}-online.dmg"
    DMG_PATH="$CACHE_DIR/qt_installer.dmg"
    MOUNT_POINT="/Volumes/QtInstaller"
    ;;
esac

#------------------------------------------------------------------------------
# Skip install if already present
#------------------------------------------------------------------------------

if [ -d "$QT_OUTPUT_DIR" ]; then
  echo "Qt $QT_VERSION already installed at $QT_OUTPUT_DIR — skipping."
  exit 0
fi

#------------------------------------------------------------------------------
# macOS flow
#------------------------------------------------------------------------------

if [ "$OS_NAME" = "Darwin" ]; then
  if [ ! -f "$DMG_PATH" ]; then
    echo "Downloading Qt installer for macOS..."
    curl -L "$QT_INSTALLER_URL" -o "$DMG_PATH"
  fi

  echo "Mounting DMG..."
  hdiutil attach "$DMG_PATH" -mountpoint "$MOUNT_POINT" > /dev/null

  QT_APP_PATH=$(find "$MOUNT_POINT" -name "*.app" | head -n 1)
  QT_EXEC="$QT_APP_PATH/Contents/MacOS/$(basename "$QT_APP_PATH" .app)"

  echo "Installing Qt via CLI: $ALIAS_PACKAGE"
  "$QT_EXEC" \
    --root "$QT_OUTPUT_DIR" \
    --email "$QT_USERNAME" \
    --pw "$QT_PASSWORD" \
    --accept-licenses \
    --default-answer \
    --confirm-command \
    install "$ALIAS_PACKAGE"

  echo "Unmounting..."
  hdiutil detach "$MOUNT_POINT" > /dev/null
  exit 0
fi

#------------------------------------------------------------------------------
# Linux flow
#------------------------------------------------------------------------------

if [ ! -f "$INSTALLER_PATH" ]; then
  echo "Downloading Qt installer for Linux..."
  curl -L "$QT_INSTALLER_URL" -o "$INSTALLER_PATH"
  chmod +x "$INSTALLER_PATH"
fi

echo "Installing Qt via CLI: $ALIAS_PACKAGE"
"$INSTALLER_PATH" \
  --root "$QT_OUTPUT_DIR" \
  --email "$QT_USERNAME" \
  --pw "$QT_PASSWORD" \
  --accept-licenses \
  --default-answer \
  --confirm-command \
  install "$ALIAS_PACKAGE"
