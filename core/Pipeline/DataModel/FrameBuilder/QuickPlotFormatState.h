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

#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QStringList>
#include <vector>

#include "DataModel/Scripting/LineFormat.h"

namespace DataModel {

/**
 * @brief What the frame builder does with one observed line: Text and Header stage nothing,
 *        Rebuild means the channel set changed and the structure must be rebuilt before the
 *        values are applied, Apply means the values go straight into the existing structure.
 */
enum class QuickPlotLineOutcome : quint8 {
  Text,
  Header,
  Rebuild,
  Apply,
};

/**
 * @brief One Quick Plot channel: a name for a named or JSON field, otherwise the field position it
 *        was established at. The unit is read once, from the line that created the channel.
 */
struct QuickPlotChannel {
  QByteArray name;
  QByteArray unit;
  int position = -1;
};

/**
 * @brief Pipeline-thread state of the Quick Plot lane (spec 0106): the format established from the
 *        first qualifying line, the channel registry keyed by name or position, the header
 *        candidate, and the one re-evaluation that runs when the fixed format yields no number.
 *        No Qt object and no bus reach; replay and audio pin a positional format, never detect.
 */
class QuickPlotFormatState {
public:
  QuickPlotFormatState();
  QuickPlotFormatState(QuickPlotFormatState&&)                 = delete;
  QuickPlotFormatState(const QuickPlotFormatState&)            = delete;
  QuickPlotFormatState& operator=(QuickPlotFormatState&&)      = delete;
  QuickPlotFormatState& operator=(const QuickPlotFormatState&) = delete;

  [[nodiscard]] bool pinned() const noexcept { return m_pinned; }

  [[nodiscard]] bool established() const noexcept { return m_established; }

  [[nodiscard]] const LineFormat& format() const noexcept { return m_format; }

  [[nodiscard]] const std::vector<QuickPlotChannel>& channels() const noexcept
  {
    return m_channels;
  }

  [[nodiscard]] bool positionalOnly() const noexcept;
  [[nodiscard]] QString channelTitle(int index) const;
  [[nodiscard]] int indexOf(const LineField& field, qsizetype position) noexcept;

  void reset();
  void pinPositional(int count);
  [[nodiscard]] QuickPlotLineOutcome observe(QByteArrayView line, LineFields& fields);
  [[nodiscard]] QuickPlotLineOutcome observeTokenized(const LineFields& fields);

private:
  [[nodiscard]] QuickPlotLineOutcome classify(QByteArrayView line, LineFields& fields);
  [[nodiscard]] bool grow(const LineFields& fields);
  void establish(const LineFields& fields);
  void acceptHeader(QByteArrayView line, LineFields& fields);
  void rememberHeaderCandidate(QByteArrayView line);
  void appendChannel(const LineField& field, qsizetype position);
  void rebuildPositionIndex();
  void clearRegistry();

private:
  bool m_pinned;
  bool m_established;
  LineFormat m_format;
  QByteArray m_headerCandidate;
  QStringList m_headerTitles;
  std::vector<int> m_lastIndex;
  std::vector<int> m_positionIndex;
  std::vector<QuickPlotChannel> m_channels;
};

}  // namespace DataModel
