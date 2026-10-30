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
#include <QByteArray>
#include <QByteArrayView>
#include <QCoreApplication>
#include <QString>
#include <QStringList>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder/QuickPlotFormatState.h"

namespace DataModel {

/**
 * @brief Owns the synthetic Quick Plot frame, the format state that titles its channels, and the
 *        per-line field scratch. Structure is rebuilt only on a Rebuild outcome, an audio layout
 *        announcement or a header registration, never per frame; applyFields writes one line's
 *        values in place and the frame builder stages the result through frame().
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

  [[nodiscard]] const LineFormat& format() const noexcept { return m_state.format(); }

  [[nodiscard]] qsizetype tokenSpans(const QByteArrayView*& spans) noexcept;
  [[nodiscard]] QuickPlotLineOutcome observeReplayRow(QByteArrayView row);
  [[nodiscard]] QuickPlotLineOutcome observe(QByteArrayView line);

  void setHeaders(const QStringList& headers);
  void buildAudio(const QStringList& channels);
  void pinPositional(int count);
  void applyFields();
  void build();
  void reset();

private:
  [[nodiscard]] DataModel::Source makeSource() const;
  [[nodiscard]] QString channelTitle(int index) const;
  void applyField(DataModel::Dataset& dataset, int fieldIndex);
  void mapFieldsToChannels() noexcept;

private:
  const SerialStudio::OperationMode& m_operationMode;
  bool m_hasHeader;
  QStringList m_channelNames;
  QByteArray m_replayScratch;
  DataModel::Frame m_frame;
  LineFields m_fields;
  QuickPlotFormatState m_state;
  std::array<int, kMaxLineFields> m_fieldOfChannel;
  std::array<QByteArrayView, kMaxLineFields> m_tokenSpans;
};

}  // namespace DataModel
