#
# Serial Studio — Code signing (Windows / macOS / Linux)
# https://serial-studio.com/
#
# Copyright (C) 2020–2025 Alex Spataru
#
# This file is dual-licensed:
#
# - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
# - Under the Serial Studio Commercial License for builds that include
#   any Pro functionality.
#
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial

include_guard(GLOBAL)

#---------------------------------------------------------------------------------------------------
# Code signing
#---------------------------------------------------------------------------------------------------
#
# serial_studio_sign(<target>) attaches a platform-native code signature as a POST_BUILD step,
# gated behind ENABLE_CODE_SIGNING (OFF by default; each platform no-ops with a warning when its
# credentials are unset):
#
#   Windows   signtool Authenticode (SHA-256 + RFC-3161 timestamp); cert by SHA1 thumbprint or .pfx.
#   macOS     codesign with the hardened runtime, then optional notarytool submit + stapler staple.
#   Linux     gpg detached --armor signature next to the binary (<binary>.asc).
#
# Why: shipped binaries must be signed for the OS to trust them -- Windows SmartScreen/UAC and macOS
# Gatekeeper warn on or block unsigned apps, and macOS distribution outside the App Store requires
# notarization. Running it as a POST_BUILD step keeps signing in the build graph (CI signs every
# artifact automatically) instead of leaving it a manual, forgettable afterthought.
#
#---------------------------------------------------------------------------------------------------

option(ENABLE_CODE_SIGNING "Run platform code signing as a POST_BUILD step" OFF)

set(WINDOWS_SIGN_SHA1     "" CACHE STRING "Code-signing cert SHA1 thumbprint (Windows cert store)")
set(WINDOWS_SIGN_PFX      "" CACHE STRING "Path to .pfx code-signing certificate")
set(WINDOWS_SIGN_PFX_PASS "" CACHE STRING "Password for the .pfx (prefer an env var in CI)")
set(WINDOWS_SIGN_TS_URL   "http://timestamp.digicert.com" CACHE STRING "RFC-3161 timestamp URL")

set(MACOS_SIGN_IDENTITY   "" CACHE STRING "e.g. 'Developer ID Application: Alex Spataru (TEAMID)'")
set(MACOS_NOTARY_PROFILE  "" CACHE STRING "notarytool keychain profile (xcrun notarytool store-credentials)")
set(MACOS_ENTITLEMENTS    "" CACHE STRING "Path to hardened-runtime entitlements .plist (optional)")

set(LINUX_SIGN_GPG_KEY    "" CACHE STRING "GPG key id/email for a detached .asc of the binary")

#---------------------------------------------------------------------------------------------------
# serial_studio_sign(<target>) — POST_BUILD code signing
#---------------------------------------------------------------------------------------------------

function(serial_studio_sign _tgt)
  if(NOT ENABLE_CODE_SIGNING)
    return()
  endif()
  if(NOT TARGET ${_tgt})
    message(WARNING "serial_studio_sign: '${_tgt}' is not a target")
    return()
  endif()

  if(WIN32 AND MSVC)
    find_program(SIGNTOOL_EXECUTABLE signtool)
    if(NOT SIGNTOOL_EXECUTABLE)
      message(WARNING "signtool not found; skipping Windows signing")
      return()
    endif()
    if(WINDOWS_SIGN_SHA1)
      set(_cert_args /sha1 ${WINDOWS_SIGN_SHA1})
    elseif(WINDOWS_SIGN_PFX)
      set(_cert_args /f "${WINDOWS_SIGN_PFX}")
      if(WINDOWS_SIGN_PFX_PASS)
        list(APPEND _cert_args /p "${WINDOWS_SIGN_PFX_PASS}")
      endif()
    else()
      message(WARNING "No WINDOWS_SIGN_SHA1 or WINDOWS_SIGN_PFX set; skipping signing")
      return()
    endif()
    add_custom_command(TARGET ${_tgt} POST_BUILD
      COMMAND "${SIGNTOOL_EXECUTABLE}" sign /fd SHA256 ${_cert_args}
              /tr "${WINDOWS_SIGN_TS_URL}" /td SHA256 "$<TARGET_FILE:${_tgt}>"
      COMMENT "Authenticode-signing $<TARGET_FILE_NAME:${_tgt}>"
      VERBATIM)

  elseif(APPLE)
    if(NOT MACOS_SIGN_IDENTITY)
      message(WARNING "MACOS_SIGN_IDENTITY not set; skipping macOS signing")
      return()
    endif()
    set(_cs_args --force --options runtime --timestamp --sign "${MACOS_SIGN_IDENTITY}")
    if(MACOS_ENTITLEMENTS)
      list(APPEND _cs_args --entitlements "${MACOS_ENTITLEMENTS}")
    endif()
    add_custom_command(TARGET ${_tgt} POST_BUILD
      COMMAND codesign ${_cs_args} "$<TARGET_BUNDLE_DIR:${_tgt}>"
      COMMENT "codesign (hardened runtime) $<TARGET_FILE_NAME:${_tgt}>"
      VERBATIM)
    if(MACOS_NOTARY_PROFILE)
      add_custom_command(TARGET ${_tgt} POST_BUILD
        COMMAND ditto -c -k --keepParent "$<TARGET_BUNDLE_DIR:${_tgt}>" "$<TARGET_BUNDLE_DIR:${_tgt}>.zip"
        COMMAND xcrun notarytool submit "$<TARGET_BUNDLE_DIR:${_tgt}>.zip"
                --keychain-profile "${MACOS_NOTARY_PROFILE}" --wait
        COMMAND xcrun stapler staple "$<TARGET_BUNDLE_DIR:${_tgt}>"
        COMMENT "notarize + staple $<TARGET_FILE_NAME:${_tgt}>"
        VERBATIM)
    endif()

  elseif(UNIX)
    if(NOT LINUX_SIGN_GPG_KEY)
      message(WARNING "LINUX_SIGN_GPG_KEY not set; skipping Linux signing")
      return()
    endif()
    find_program(GPG_EXECUTABLE gpg)
    if(NOT GPG_EXECUTABLE)
      message(WARNING "gpg not found; skipping Linux signing")
      return()
    endif()
    add_custom_command(TARGET ${_tgt} POST_BUILD
      COMMAND "${GPG_EXECUTABLE}" --batch --yes --local-user "${LINUX_SIGN_GPG_KEY}"
              --detach-sign --armor --output "$<TARGET_FILE:${_tgt}>.asc" "$<TARGET_FILE:${_tgt}>"
      COMMENT "GPG detached-signing $<TARGET_FILE_NAME:${_tgt}>"
      VERBATIM)
  endif()
endfunction()
