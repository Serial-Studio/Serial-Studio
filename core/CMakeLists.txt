#
# Serial Studio
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
# You must comply with the terms of one of these licenses, depending
# on your use case.
#
# For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
# For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial

#---------------------------------------------------------------------------------------------------
# Modular core (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# Statically linked first-party libraries, one CMakeLists.txt per layer, all of them folded into
# the single SerialStudio executable. This directory is added from app/CMakeLists.txt after Qt
# discovery and before include_directories(src), so a library translation unit cannot see app/src
# at all: the isolation is a property of the build order, not of a lint rule.
#
# Dependencies flow downward only (spec 0076 for Core and Protocols, spec 0077 for the rest):
#
#   Ui -> Api -> Storage -> Pipeline -> Protocols -> Core -> Qt6::Core
#              Devices -----^ (a sibling of Pipeline; Api and Ui see both)
#
# Each library links only the layers below it and never includes anything from app/src. Core and
# Protocols are reached through their directory name ("Core/...", "Protocols/...", root = core/);
# the five partitions carved out of the old monolith keep their own relative include root and see
# the roots of the layers they may include, nothing else. Cross-library state travels over
# Core::Bus::MessageBus (retained topics) or through the interfaces and root-bound module sets
# the composition root binds; no library constructs or reaches another library's singleton.
#
# scripts/layer-verify.py enforces all of this: it fails on an include outside a layer's graph, on
# an include root or a link a CMakeLists.txt here names outside the graph, on a source under core/
# that no target owns (or that two own), and on a CMake entry that names a missing file.
#
#---------------------------------------------------------------------------------------------------

#
# app/CMakeLists.txt sets these for the executable after this directory has been added, and CMake
# snapshots directory definitions at add_subdirectory time; the archives compile the same std::min
# / std::max code and need the same Windows header hygiene.
#
if(WIN32)
  add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WINSOCKAPI_
  )
endif()

#---------------------------------------------------------------------------------------------------
# MiniAudio feature switches, shared by SerialStudio::Devices and SerialStudio::Pipeline
#---------------------------------------------------------------------------------------------------
#
# The MA_* switches decide which members miniaudio's structs carry, so every archive that includes
# ThirdParty/miniaudio.h must set exactly the same ones or two of them disagree about the size of a
# ma_device. SerialStudio::Devices owns the vendored header and implementation
# (core/Devices/ThirdParty/), the .cpp's warning suppressions and the ALSA link; Pipeline still sees
# the header through QuickPlotBuilder.cpp and is added BEFORE Devices, which is why the function is
# defined here rather than next to the file it configures.
#
# Disabled features (only raw PCM capture is needed for the oscilloscope/FFT features):
#   MA_NO_DECODING         - Disable audio file decoding (WAV, MP3, FLAC)
#   MA_NO_ENCODING         - Disable audio file encoding
#   MA_NO_WAV/FLAC/MP3     - Disable codec support (only need raw PCM)
#   MA_NO_RESOURCE_MANAGER - Disable resource loading system
#   MA_NO_NODE_GRAPH       - Disable audio graph processing
#   MA_NO_ENGINE           - Disable high-level engine API
#   MA_NO_GENERATION       - Disable waveform generation
#   MA_NO_CUSTOM           - Disable custom backend support
#   MA_NO_NULL             - Disable null audio backend
#
# Platform backends: Linux links libasound (ALSA, in core/Devices); Windows uses WASAPI and macOS
# CoreAudio, both built in. SIMD support (SSE2 on x86-64, NEON on ARM64) is what the header
# detects from the compiler's own macros; MA_SUPPORT_* are its internal results, and defining one
# on the command line only redefines it. Dropping the unused features saves ~200KB of binary and a
# good share of the compile time.
#
#---------------------------------------------------------------------------------------------------

function(ss_apply_miniaudio_definitions TARGET)
  target_compile_definitions(${TARGET} PRIVATE
    MA_NO_DECODING
    MA_NO_ENCODING
    MA_NO_WAV
    MA_NO_FLAC
    MA_NO_MP3
    MA_NO_RESOURCE_MANAGER
    MA_NO_NODE_GRAPH
    MA_NO_ENGINE
    MA_NO_GENERATION
    MA_NO_CUSTOM
    MA_NO_NULL
  )
endfunction()

add_subdirectory(Core)
add_subdirectory(Protocols)

add_subdirectory(Pipeline)
add_subdirectory(Devices)
add_subdirectory(Storage)
add_subdirectory(Api)
add_subdirectory(Ui)

