/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <array>

#include "Core/Bus/Subscription.h"

namespace DataModel {
class FrameBuilder;

/**
 * @brief The frame builder's wiring to facts published outside the pipeline (spec 0077): replay
 *        players, operation mode, link state, licence, and the transform-engine inputs (Lua mode,
 *        shared library). It lives beside the facade because FrameBuilder.cpp is a hotpath TU that
 *        must never see the bus; the mirror hops write PipelineHost's atomics, the rest run here.
 */
class ExternalWiring {
public:
  static constexpr int kCsvPlayerSlot      = 0;
  static constexpr int kMdf4PlayerSlot     = 1;
  static constexpr int kSessionsPlayerSlot = 2;
  static constexpr int kPlayerSlots        = 3;

  ExternalWiring(FrameBuilder& owner, std::array<bool, kPlayerSlots>& playerOpenMask);
  ExternalWiring(ExternalWiring&&)                 = delete;
  ExternalWiring(const ExternalWiring&)            = delete;
  ExternalWiring& operator=(ExternalWiring&&)      = delete;
  ExternalWiring& operator=(const ExternalWiring&) = delete;

  void watchPlayers();
  void watchOperationMode();
  void watchLinkState();
  void watchLicense();
  void watchProjectScripting();

private:
  FrameBuilder& m_owner;
  std::array<bool, kPlayerSlots>& m_playerOpenMask;

  Core::Bus::Subscription m_playerState;
  Core::Bus::Subscription m_operationModeMirror;
  Core::Bus::Subscription m_operationMode;
  Core::Bus::Subscription m_linkMirror;
  Core::Bus::Subscription m_linkState;
  Core::Bus::Subscription m_license;
};
}  // namespace DataModel
