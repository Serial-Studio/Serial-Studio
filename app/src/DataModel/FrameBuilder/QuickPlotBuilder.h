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

#include <QCoreApplication>
#include <QStringList>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

namespace DataModel {

/**
 * @brief Owns the synthetic Quick Plot frame and the header state it titles channels from.
 *        Structure only: the frame is rebuilt on a channel-count change, an audio channel
 *        layout announcement, or a header row, never per frame. Per-frame value writes and
 *        block staging stay in the frame builder, which reaches the frame through frame().
 */
class QuickPlotBuilder {
  Q_DECLARE_TR_FUNCTIONS(DataModel::QuickPlotBuilder)

public:
  explicit QuickPlotBuilder(const SerialStudio::OperationMode& operationMode);
  QuickPlotBuilder(QuickPlotBuilder&&)                 = delete;
  QuickPlotBuilder(const QuickPlotBuilder&)            = delete;
  QuickPlotBuilder& operator=(QuickPlotBuilder&&)      = delete;
  QuickPlotBuilder& operator=(const QuickPlotBuilder&) = delete;

  [[nodiscard]] const DataModel::Frame& frame() const noexcept { return m_frame; }

  [[nodiscard]] DataModel::Frame& frame() noexcept { return m_frame; }

  void setHeaders(const QStringList& headers);
  void build(const QStringList& channels);
  void buildAudio(const QStringList& channels);

private:
  [[nodiscard]] DataModel::Source makeSource() const;

private:
  const SerialStudio::OperationMode& m_operationMode;
  bool m_hasHeader;
  QStringList m_channelNames;
  DataModel::Frame m_frame;
};

}  // namespace DataModel
