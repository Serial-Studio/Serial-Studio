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

#include <QChar>
#include <QList>
#include <QPoint>

namespace Widgets {
/**
 * @brief Which side of the cursor an erase consumes.
 */
enum class AnsiEraseDirection {
  Left,
  Right
};

/**
 * @brief Everything the escape-sequence state machine is allowed to do to a terminal. The machine
 *        never sees the text buffer, the painter, or the widget: it reads the cursor and issues
 *        these eight operations, which keeps the parser testable against a recording fake with no
 *        QQuickItem in the test binary.
 */
class AnsiSink {
public:
  virtual ~AnsiSink() = default;

  [[nodiscard]] virtual QPoint currentCursor() const = 0;

  virtual void eraseAllRows()                                            = 0;
  virtual void eraseRowsAfter(int row)                                   = 0;
  virtual void eraseRowsBefore(int row)                                  = 0;
  virtual void setCursorHidden(bool hidden)                              = 0;
  virtual void moveCursor(const QPoint& position)                        = 0;
  virtual void applySgrCodes(const QList<int>& codes)                    = 0;
  virtual void eraseFromCursor(AnsiEraseDirection direction, int length) = 0;
};

/**
 * @brief The terminal's VT-100 / ANSI escape-sequence parser. Owns only parse state: the current
 *        state, the CSI parameter accumulator, the private-mode marker and the DECSC save slot.
 *        Printable text never reaches it; the facade feeds it the bytes that follow an ESC and
 *        asks inTextState() to know when the run is over.
 */
class AnsiStateMachine {
public:
  enum State {
    Text,
    Escape,
    Format,
    ResetFont,
    OSC,
    IgnoreSeq
  };

  explicit AnsiStateMachine(AnsiSink& sink);

  [[nodiscard]] State state() const;
  [[nodiscard]] bool inTextState() const;

  void beginEscape();
  void feed(const QChar& byte);

private:
  void processOsc(const QChar& byte);
  void handleCsiEraseLine();
  void handleCsiEraseDisplay();
  void processEscape(const QChar& byte);
  void processFormat(const QChar& byte);
  void handleCsiCursorMove(char final);
  void processIgnoreSeq(const QChar& byte);
  void handleCsiCursorAbsolute(char final);
  void handleCsiDecPrivateMode(const QChar& byte);
  [[nodiscard]] bool dispatchCsiFinal(const QChar& byte);

private:
  AnsiSink& m_sink;

  State m_state;
  bool m_privateMode;
  int m_currentFormatValue;
  QList<int> m_formatValues;
  QPoint m_savedCursorPosition;
};
}  // namespace Widgets
