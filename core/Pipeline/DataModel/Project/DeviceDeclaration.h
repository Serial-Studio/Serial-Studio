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

#include <chrono>
#include <QByteArray>
#include <QByteArrayView>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "DataModel/FrameBuilder/QuickPlotFormatState.h"
#include "DataModel/Generated/DeclarationVocabulary.h"

namespace DataModel {

/**
 * @brief What offering one declaration line did: nothing (the bytes of the last accepted, refused
 *        or held line), a rebuild from new content, a refusal with a reason, or a hold because the
 *        last rebuild was under a second ago.
 */
enum class DeclarationOutcome : quint8 {
  Unchanged,
  Rebuild,
  Rejected,
  Held,
};

/**
 * @brief One parsed device declaration (spec 0111): the project title, the groups in project shape
 *        with frame indices assigned, the number of frame positions, the notes the console shows,
 *        and for a binary device (R14) the framing and packed_struct source. valid is false when
 *        the line was refused, and reason says why.
 */
struct DeviceDeclaration {
  bool valid   = false;
  bool binary  = false;
  int channels = 0;
  QString title;
  QString reason;
  QStringList notes;
  Source source;
  std::vector<Group> groups;
};

/**
 * @brief True when @p line starts with the reserved declaration prefix: a declaration, or an echo
 *        of the app's request or command lines, never a data frame. The first byte decides the
 *        common case, so a hotpath caller pays one compare.
 */
[[nodiscard]] inline bool isDeclarationLine(QByteArrayView line) noexcept
{
  return !line.isEmpty() && line.front() == '#' && line.startsWith(Declaration::kPrefix);
}

/**
 * @brief True when @p line is a declaration proper: the prefix, then a wire version digit. The
 *        app's request and command lines share the prefix with no digit, so a device that echoes
 *        them is ignored without a console message in either mode.
 */
[[nodiscard]] inline bool isDeclarationContent(QByteArrayView line) noexcept
{
  const auto digit = Declaration::kPrefix.size();
  return isDeclarationLine(line) && line.size() > digit && line.at(digit) >= '0'
      && line.at(digit) <= '9';
}

[[nodiscard]] DeviceDeclaration parseDeclaration(QByteArrayView line);
[[nodiscard]] QJsonObject wireProjection(const QString& title, const std::vector<Group>& groups);
[[nodiscard]] std::vector<QuickPlotChannel> declaredChannels(const DeviceDeclaration& declaration);

/**
 * @brief The declaration a Quick Plot session runs on: the last accepted content, the bytes of the
 *        last accepted, refused and held lines (so a re-sent line changes nothing), and the one
 *        rebuild per second that a device alternating two declarations is held to. Pure: no Qt
 *        object, no bus reach, no file and no connection setting.
 */
class DeclarationState {
public:
  using TimePoint = std::chrono::steady_clock::time_point;

  DeclarationState();
  DeclarationState(DeclarationState&&)                 = delete;
  DeclarationState(const DeclarationState&)            = delete;
  DeclarationState& operator=(DeclarationState&&)      = delete;
  DeclarationState& operator=(const DeclarationState&) = delete;

  [[nodiscard]] bool active() const noexcept { return m_current.valid; }

  [[nodiscard]] const QString& message() const noexcept { return m_message; }

  [[nodiscard]] const QString& reason() const noexcept { return m_reason; }

  [[nodiscard]] const DeviceDeclaration& current() const noexcept { return m_current; }

  [[nodiscard]] DeclarationOutcome offer(QByteArrayView line, TimePoint timestamp);
  void reset();

private:
  bool m_rebuilt;
  TimePoint m_lastRebuild;
  QString m_reason;
  QString m_message;
  QByteArray m_heldLine;
  QByteArray m_acceptedLine;
  QByteArray m_rejectedLine;
  DeviceDeclaration m_current;
};

}  // namespace DataModel
