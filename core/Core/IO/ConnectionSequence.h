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

#include <QByteArray>
#include <QCoreApplication>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace IO {

inline constexpr int kMaxBreakMs              = 10000;
inline constexpr int kMaxSequenceSteps        = 128;
inline constexpr int kExpectBufferBytes       = 4096;
inline constexpr int kMaxSequenceBudgetMs     = 600000;
inline constexpr int kDefaultSequenceBudgetMs = 10000;
inline constexpr qint32 kMaxSequenceBaudRate  = 15000000;

/**
 * @brief One acceptable reply of an expect step, parsed from text such as "5D ?? 7F": every byte
 *        carries a mask, so "??" accepts any byte and "5?" any byte whose high nibble is 5.
 */
struct ExpectPattern {
  QByteArray value;
  QByteArray mask;

  [[nodiscard]] qsizetype matchIn(const QByteArray& buffer) const;
  [[nodiscard]] static bool parse(const QString& text, ExpectPattern& out);
};

/**
 * @brief One step of a Connection Sequence. Only the fields its kind reads are serialized; the
 *        others keep their defaults.
 */
struct SequenceStep {
  enum class Kind {
    Break,
    Wait,
    SetLine,
    Send,
    Expect,
    Discard,
    SetBaud,
    SetFlowControl,
    Unknown,
  };

  enum class Line {
    Dtr,
    Rts,
  };

  enum class Flow {
    None,
    RtsCts,
    XonXoff,
  };

  Kind kind     = Kind::Wait;
  Line line     = Line::Dtr;
  Flow flow     = Flow::None;
  int ms        = 0;
  int timeoutMs = 1000;
  qint32 rate   = 9600;
  bool high     = true;
  bool hex      = false;
  bool optional = false;
  QString data;
  QString eol;
  QString checksum;
  QStringList patterns;

  [[nodiscard]] QJsonObject toJson() const;
  [[nodiscard]] QByteArray sendBytes() const;
  [[nodiscard]] QList<ExpectPattern> expectPatterns() const;
  [[nodiscard]] static SequenceStep fromJson(const QJsonObject& obj);
};

/**
 * @brief The first problem validate() found: the step it concerns (-1 for the whole sequence)
 *        and a sentence naming it. An empty message means the sequence is valid.
 */
struct SequenceError {
  int step = -1;
  QString message;
};

/**
 * @brief The ordered steps a UART source runs on every open before data flows (spec 0103), plus
 *        the whole-sequence time budget. Qt Core only: no I/O, no timer, no port.
 */
struct ConnectionSequence {
  Q_DECLARE_TR_FUNCTIONS(IO::ConnectionSequence)

public:
  QList<SequenceStep> steps;
  int budgetMs = kDefaultSequenceBudgetMs;

  [[nodiscard]] bool isEmpty() const noexcept;
  [[nodiscard]] QJsonObject toJson() const;
  [[nodiscard]] SequenceError validate() const;
  [[nodiscard]] QString stepText(int index) const;
  [[nodiscard]] static QString hexText(const QByteArray& bytes);
  [[nodiscard]] static QString kindLabel(SequenceStep::Kind kind);
  [[nodiscard]] static ConnectionSequence fromJson(const QJsonObject& obj);
  [[nodiscard]] static ConnectionSequence fromVariant(const QVariant& value);

private:
  [[nodiscard]] static QString stepSummary(const SequenceStep& step);
  [[nodiscard]] static QString stepProblem(const SequenceStep& step);
  [[nodiscard]] static QString sendProblem(const SequenceStep& step);
  [[nodiscard]] static QString expectProblem(const SequenceStep& step);
};

}  // namespace IO
