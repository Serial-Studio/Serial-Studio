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

$ErrorActionPreference = "Stop"

#------------------------------------------------------------------------------
# Config
#------------------------------------------------------------------------------

$QT_VERSION = $env:QT_VERSION
if (-not $QT_VERSION) { $QT_VERSION = "6.9.1" }

$QT_ARCH = $env:QT_ARCH
if (-not $QT_ARCH) { $QT_ARCH = "x64" }

$QT_USERNAME = $env:QT_USERNAME
$QT_PASSWORD = $env:QT_PASSWORD

if (-not $QT_USERNAME -or -not $QT_PASSWORD) {
    Write-Error "Must provide QT_USERNAME and QT_PASSWORD environment variables."
    exit 1
}

#------------------------------------------------------------------------------
# Build output directory and alias package name
#------------------------------------------------------------------------------

$OS_TAG = "Windows"
$QT_OUTPUT_DIR = $env:QT_OUTPUT_DIR
if (-not $QT_OUTPUT_DIR) {
    $QT_OUTPUT_DIR = "Qt-$QT_VERSION-$OS_TAG-$QT_ARCH"
}

$ALIAS_PACKAGE = "qt$QT_VERSION-msvc2022-full"

#------------------------------------------------------------------------------
# Installer setup
#------------------------------------------------------------------------------

$CACHE_DIR = "$env:USERPROFILE\.qt_cache"
$INSTALLER_PATH = "$CACHE_DIR\qt_installer.exe"
$QT_INSTALLER_URL = "https://download.qt.io/official_releases/online_installers/qt-online-installer-windows-$QT_ARCH-online.exe"

if (-not (Test-Path $CACHE_DIR)) {
    New-Item -ItemType Directory -Force -Path $CACHE_DIR | Out-Null
}

#------------------------------------------------------------------------------
# Skip if already installed
#------------------------------------------------------------------------------

if (Test-Path "$QT_OUTPUT_DIR\$QT_VERSION") {
    Write-Host "Qt $QT_VERSION already installed at $QT_OUTPUT_DIR — skipping."
    exit 0
}

#------------------------------------------------------------------------------
# Download installer if needed
#------------------------------------------------------------------------------

if (-not (Test-Path $INSTALLER_PATH)) {
    Write-Host "Downloading Qt installer for Windows..."
    Invoke-WebRequest $QT_INSTALLER_URL -OutFile $INSTALLER_PATH
}

#------------------------------------------------------------------------------
# Run installer CLI
#------------------------------------------------------------------------------

Write-Host "Installing Qt via CLI: $ALIAS_PACKAGE"
Start-Process -FilePath $INSTALLER_PATH `
    -ArgumentList @(
        "--root", "`"$QT_OUTPUT_DIR`"",
        "--email", "`"$QT_USERNAME`"",
        "--pw", "`"$QT_PASSWORD`"",
        "--accept-licenses",
        "--default-answer",
        "--confirm-command",
        "install", "$ALIAS_PACKAGE"
    ) `
    -Wait -NoNewWindow
