/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include "DataModel/CachedFlag.h"

namespace DataModel {

class FrameBuilder;

/**
 * @brief The single derivation of every frame-builder cached hotpath flag (spec 0095 M1): the
 *        refresh slots assign from these helpers and the 1 Hz cross-check compares against them,
 *        so a refresh and its audit can never disagree on what the flag means. Static-only.
 */
class BuilderFlagAudit {
public:
  static const CachedFlagSpec kOperationMode;
  static const CachedFlagSpec kPlayerOpen;
  static const CachedFlagSpec kCaptureDatasetValues;
  static const CachedFlagSpec kCaptureLatestFrame;
  static const CachedFlagSpec kChangeDriven;
  static const CachedFlagSpec kAnyAsyncSink;

  [[nodiscard]] static bool deriveChangeDriven();
  [[nodiscard]] static bool deriveCaptureLatest(const FrameBuilder& builder);
  [[nodiscard]] static bool deriveCaptureDatasetValues(const FrameBuilder& builder);

private:
  [[nodiscard]] static bool builderLive(const void* owner);
  [[nodiscard]] static bool datasetFlagsSettled(const void* owner);

  [[nodiscard]] static int freshOperationMode(const void* owner);
  [[nodiscard]] static int freshPlayerOpen(const void* owner);
  [[nodiscard]] static int freshCaptureDatasetValues(const void* owner);
  [[nodiscard]] static int freshCaptureLatestFrame(const void* owner);
  [[nodiscard]] static int freshChangeDriven(const void* owner);
  [[nodiscard]] static int freshAnyAsyncSink(const void* owner);

  static void repairOperationMode(void* owner);
  static void repairPlayerOpen(void* owner);
  static void repairDatasetFlags(void* owner);
  static void repairCaptureLatestFrame(void* owner);
  static void repairAnyAsyncSink(void* owner);
};

}  // namespace DataModel
