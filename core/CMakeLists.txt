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
# Two regimes live here, and the difference is the point.
#
# Core and Protocols are STRICT. Dependencies flow downward only:
#
#   Ui / Api / Storage / Devices  ->  Pipeline  ->  Protocols  ->  Core  ->  Qt6::Core
#
# They link only the layers below, never a sibling, and never include anything from app/src. The
# directory name is the include namespace: every include into one of them is written as "Core/..."
# or "Protocols/...", because ${CMAKE_SOURCE_DIR}/core is the PUBLIC include root each exports.
#
# Pipeline, Devices, Storage, Api and Ui are TRANSITIONAL (spec 0076 stage 2). They are partitions
# of the old monolith, moved whole so every #include line in the tree stayed byte-identical: each
# partition directory is an include root and so is app/src, and the five link each other PUBLIC
# because the code they hold really is a cycle through singletons today. That is measured debt, not
# a layering statement -- scripts/layer-verify.py counts the includes on every directed edge
# against a checked-in baseline and fails on growth, and each follow-up spec drives one edge to
# zero by replacing its singleton reaches with bus topics, then flips that edge to strict.
#
# scripts/layer-verify.py enforces all of this: it fails on an upward include out of a strict
# layer, on a ratcheted edge that grew, on a source under core/ that no target owns (or that two
# own), and on a CMake entry that names a missing file.
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

add_subdirectory(Core)
add_subdirectory(Protocols)

add_subdirectory(Pipeline)
add_subdirectory(Devices)
add_subdirectory(Storage)
add_subdirectory(Api)
add_subdirectory(Ui)

#---------------------------------------------------------------------------------------------------
# The cycle
#---------------------------------------------------------------------------------------------------
#
# Declared once, here, because it needs all five targets to exist. Every partition links the other
# four PUBLIC: that is the dependency graph the code actually has, and stating it is what makes
# CMake repeat the archives on the final link line -- GNU ld and lld resolve a static archive in
# one pass, so a mutual reference between two archives is unresolvable without the repetition.
# MSVC and ld64 do not care. Nothing here shrinks the graph; layer-verify.py's per-edge baseline is
# what does, one follow-up spec at a time.
#
#---------------------------------------------------------------------------------------------------

set(SS_PARTITION_LAYERS Pipeline Devices Storage Api Ui)

foreach(_ss_layer IN LISTS SS_PARTITION_LAYERS)
  foreach(_ss_peer IN LISTS SS_PARTITION_LAYERS)
    if(NOT "${_ss_layer}" STREQUAL "${_ss_peer}")
      target_link_libraries(SerialStudio${_ss_layer} PUBLIC SerialStudio::${_ss_peer})
    endif()
  endforeach()
endforeach()
