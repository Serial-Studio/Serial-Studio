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

#include "UI/Widgets/Terminal/AnsiStateMachine.h"

#include <climits>

//--------------------------------------------------------------------------------------------------
// Construction & state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a parser bound to @p sink; the machine starts idle, in the text state.
 */
Widgets::AnsiStateMachine::AnsiStateMachine(AnsiSink& sink)
  : m_sink(sink), m_state(Text), m_privateMode(false), m_currentFormatValue(0)
{}

/**
 * @brief Returns the parse state the next byte will be interpreted in.
 */
Widgets::AnsiStateMachine::State Widgets::AnsiStateMachine::state() const
{
  return m_state;
}

/**
 * @brief Returns true while no escape sequence is in flight, which is the facade's cue that
 *        the incoming bytes belong to the printable-run fast lane.
 */
bool Widgets::AnsiStateMachine::inTextState() const
{
  return m_state == Text;
}

/**
 * @brief Enters the escape state; the facade calls this when the text lane meets an ESC.
 */
void Widgets::AnsiStateMachine::beginEscape()
{
  m_state = Escape;
}

/**
 * @brief Consumes one byte of an in-flight escape sequence. A byte offered while the machine
 *        is in the text state is ignored: printable text is the facade's lane, never this one.
 */
void Widgets::AnsiStateMachine::feed(const QChar& byte)
{
  switch (m_state) {
    case Escape:
      processEscape(byte);
      break;
    case Format:
      processFormat(byte);
      break;
    case ResetFont:
      m_state = Text;
      break;
    case OSC:
      processOsc(byte);
      break;
    case IgnoreSeq:
      processIgnoreSeq(byte);
      break;
    default:
      break;
  }
}

//--------------------------------------------------------------------------------------------------
// Escape & CSI parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes the character immediately following ESC (0x1B).
 */
void Widgets::AnsiStateMachine::processEscape(const QChar& byte)
{
  m_formatValues.clear();
  m_currentFormatValue = 0;
  m_privateMode        = false;

  switch (byte.toLatin1()) {
    case '[':
      m_state = Format;
      return;
    case '(':
      m_state = ResetFont;
      return;
    case ']':
      m_state = OSC;
      return;
    case '7':
      m_savedCursorPosition = m_sink.currentCursor();
      m_state               = Text;
      return;
    case '8':
      m_sink.moveCursor(m_savedCursorPosition);
      m_state = Text;
      return;
    case 'M': {
      const QPoint cursor = m_sink.currentCursor();
      m_sink.moveCursor(QPoint(cursor.x(), qMax(0, cursor.y() - 1)));
      m_state = Text;
      return;
    }
    default:
      m_state = Text;
      return;
  }
}

/**
 * @brief Processes one byte of a CSI (ESC[...) parameter or final sequence.
 */
void Widgets::AnsiStateMachine::processFormat(const QChar& byte)
{
  if (byte >= '0' && byte <= '9') {
    static constexpr int kMaxCsiParam = 1000000;
    if (m_currentFormatValue < kMaxCsiParam)
      m_currentFormatValue = m_currentFormatValue * 10 + (byte.cell() - '0');

    return;
  }

  if (byte == '?' || byte == '>' || byte == '=') {
    m_privateMode = true;
    return;
  }

  if (byte == ';') {
    static constexpr int kMaxCsiParams = 32;
    if (m_formatValues.size() < kMaxCsiParams)
      m_formatValues.append(m_currentFormatValue);

    m_currentFormatValue = 0;
    m_state              = Format;
    return;
  }

  if (dispatchCsiFinal(byte))
    return;

  m_state = Text;
}

/**
 * @brief Dispatches the final byte of a CSI sequence; returns true if handled.
 */
bool Widgets::AnsiStateMachine::dispatchCsiFinal(const QChar& byte)
{
  const char final = byte.toLatin1();
  switch (final) {
    case 'm':
      if (!m_privateMode) {
        m_formatValues.append(m_currentFormatValue);
        m_sink.applySgrCodes(m_formatValues);
      }

      m_state = Text;
      return true;

    case 's':
      if (!m_privateMode)
        m_savedCursorPosition = m_sink.currentCursor();

      m_state = Text;
      return true;

    case 'u':
      if (!m_privateMode)
        m_sink.moveCursor(m_savedCursorPosition);

      m_state = Text;
      return true;

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      handleCsiCursorMove(final);
      m_state = Text;
      return true;

    case 'H':
    case 'f':
    case 'G':
    case 'd':
      handleCsiCursorAbsolute(final);
      m_state = Text;
      return true;

    case 'J':
      handleCsiEraseDisplay();
      m_state = Text;
      return true;

    case 'K':
      handleCsiEraseLine();
      m_state = Text;
      return true;

    case 'P':
      if (!m_privateMode) {
        m_sink.eraseFromCursor(AnsiEraseDirection::Left, m_currentFormatValue);
        m_sink.eraseFromCursor(AnsiEraseDirection::Right, INT_MAX);
      }

      m_state = Text;
      return true;

    case 'h':
    case 'l':
      handleCsiDecPrivateMode(byte);
      m_state = Text;
      return true;

    default:
      if ((final >= 'A' && final <= 'Z') || (final >= 'a' && final <= 'z')) {
        m_state = Text;
        return true;
      }

      return false;
  }
}

//--------------------------------------------------------------------------------------------------
// CSI handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles CSI cursor movement letters A-F (CUU/CUD/CUF/CUB/CNL/CPL).
 */
void Widgets::AnsiStateMachine::handleCsiCursorMove(char final)
{
  if (m_privateMode)
    return;

  const int value     = m_currentFormatValue ? m_currentFormatValue : 1;
  const QPoint cursor = m_sink.currentCursor();
  const int cx        = cursor.x();
  const int cy        = cursor.y();

  switch (final) {
    case 'A':
      m_sink.moveCursor(QPoint(cx, qMax(0, cy - value)));
      break;
    case 'B':
      m_sink.moveCursor(QPoint(cx, cy + value));
      break;
    case 'C':
      m_sink.moveCursor(QPoint(cx + value, cy));
      break;
    case 'D':
      m_sink.moveCursor(QPoint(qMax(0, cx - value), cy));
      break;
    case 'E':
      m_sink.moveCursor(QPoint(0, cy + value));
      break;
    case 'F':
      m_sink.moveCursor(QPoint(0, qMax(0, cy - value)));
      break;
    default:
      break;
  }
}

/**
 * @brief Handles CSI absolute cursor placement (H/f/G/d).
 */
void Widgets::AnsiStateMachine::handleCsiCursorAbsolute(char final)
{
  if (m_privateMode)
    return;

  if (final == 'H' || final == 'f') {
    if (m_formatValues.isEmpty()) {
      const int row = qMax(0, m_currentFormatValue > 0 ? m_currentFormatValue - 1 : 0);
      m_sink.moveCursor(QPoint(0, row));
      return;
    }

    const int row = qMax(0, m_formatValues.value(0, 1) - 1);
    const int col =
      m_currentFormatValue > 0 ? m_currentFormatValue - 1 : m_formatValues.value(1, 1) - 1;
    m_sink.moveCursor(QPoint(qMax(0, col), row));
    return;
  }

  const int v = qMax(0, m_currentFormatValue > 0 ? m_currentFormatValue - 1 : 0);
  if (final == 'G')
    m_sink.moveCursor(QPoint(v, m_sink.currentCursor().y()));

  else if (final == 'd')
    m_sink.moveCursor(QPoint(m_sink.currentCursor().x(), v));
}

/**
 * @brief Handles CSI Erase-in-Display (J).
 */
void Widgets::AnsiStateMachine::handleCsiEraseDisplay()
{
  if (m_privateMode)
    return;

  const int cy = m_sink.currentCursor().y();
  switch (m_currentFormatValue) {
    case 0:
      m_sink.eraseFromCursor(AnsiEraseDirection::Right, INT_MAX);
      m_sink.eraseRowsAfter(cy);
      break;
    case 1:
      m_sink.eraseFromCursor(AnsiEraseDirection::Left, INT_MAX);
      m_sink.eraseRowsBefore(cy);
      m_sink.moveCursor(QPoint(m_sink.currentCursor().x(), 0));
      break;
    case 2:
    case 3:
      m_sink.eraseAllRows();
      break;
    default:
      break;
  }
}

/**
 * @brief Handles CSI Erase-in-Line (K).
 */
void Widgets::AnsiStateMachine::handleCsiEraseLine()
{
  if (m_privateMode)
    return;

  switch (m_currentFormatValue) {
    case 0:
      m_sink.eraseFromCursor(AnsiEraseDirection::Right, INT_MAX);
      break;
    case 1:
      m_sink.eraseFromCursor(AnsiEraseDirection::Left, INT_MAX);
      break;
    case 2:
      m_sink.eraseFromCursor(AnsiEraseDirection::Right, INT_MAX);
      m_sink.eraseFromCursor(AnsiEraseDirection::Left, INT_MAX);
      break;
    default:
      break;
  }
}

/**
 * @brief Handles CSI DEC private mode set/reset for cursor visibility (h/l).
 */
void Widgets::AnsiStateMachine::handleCsiDecPrivateMode(const QChar& byte)
{
  if (m_privateMode && m_currentFormatValue == 25)
    m_sink.setCursorHidden(byte == 'l');
}

//--------------------------------------------------------------------------------------------------
// OSC & unknown sequences
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consumes one byte while in OSC state (BEL terminator or ESC -> CSI).
 */
void Widgets::AnsiStateMachine::processOsc(const QChar& byte)
{
  const char latin = byte.toLatin1();
  if (latin == 0x07)
    m_state = Text;

  else if (latin == 0x1b)
    m_state = Escape;
}

/**
 * @brief Consumes one byte while ignoring an unknown CSI sequence.
 */
void Widgets::AnsiStateMachine::processIgnoreSeq(const QChar& byte)
{
  if ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z'))
    m_state = Text;
}
