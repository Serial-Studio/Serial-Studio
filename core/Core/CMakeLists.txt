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
# SerialStudioCore -- the foundation layer (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# Assertions, hotpath macros, the SIMD kernels, checksums, the SPSC ring, the parse budget and the
# async task tree: what every layer above needs and what needs nothing but Qt Core itself.
#
# Headers are listed alongside the sources on purpose -- AUTOMOC only scans what a target lists,
# and Async/TaskTree.h carries nine Q_OBJECT classes (Bus/MessageBus.h one more). The same headers
# must NOT appear in the executable's HEADERS list, or moc runs twice and the link fails on
# duplicate staticMetaObject symbols.
#
# Qt6::Core is the whole link set: no file here reaches Gui, Network or any bus module (QPointF,
# the one geometry type in the closure, lives in QtCore).
#
# No unity build on this target. The optimisation, SIMD, PGO/LTO and sanitizer options are
# directory-scoped from the root CMakeLists.txt and reach it unchanged; serial_studio_harden()
# adds the per-target hardening deltas the directory-level block cannot express.
#
#---------------------------------------------------------------------------------------------------

qt_add_library(
  SerialStudioCore STATIC

  SSAssert.h
  SSAssert.cpp
  Concepts.h
  DSPSimd.h
  HotpathOptimization.h
  ParseBudget.h
  CircularBuffer.h
  Checksum.h
  Checksum.cpp
  Async/AsyncClock.h
  Async/RetryPolicy.h
  Async/RetryPolicy.cpp
  Async/TaskTree.h
  Async/TaskTree.cpp
  Bus/Messages.h
  Bus/Subscription.h
  Bus/Subscription.cpp
  Bus/MessageBus.h
  Bus/MessageBus.cpp
)

add_library(SerialStudio::Core ALIAS SerialStudioCore)

target_include_directories(SerialStudioCore PUBLIC ${CMAKE_SOURCE_DIR}/core)
target_link_libraries(SerialStudioCore PUBLIC Qt6::Core)

if(COMMAND serial_studio_harden)
  serial_studio_harden(SerialStudioCore)
endif()
