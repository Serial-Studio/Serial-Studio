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

#include "UI/Widgets/Terminal/Vt100Keymap.h"

#include <QKeyEvent>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Key translation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps editing, navigation, and F-keys to their VT-100 byte sequences.
 */
QByteArray Widgets::Vt100Keymap::specialKey(int key)
{
  QByteArray seq;
  switch (key) {
    case Qt::Key_Backspace:
      seq.append('\x7f');
      break;
    case Qt::Key_Delete:
      seq.append("\x1b[3~");
      break;
    case Qt::Key_Tab:
      seq.append('\t');
      break;
    case Qt::Key_Backtab:
      seq.append("\x1b[Z");
      break;
    case Qt::Key_Escape:
      seq.append('\x1b');
      break;
    case Qt::Key_Up:
      seq.append("\x1b[A");
      break;
    case Qt::Key_Down:
      seq.append("\x1b[B");
      break;
    case Qt::Key_Right:
      seq.append("\x1b[C");
      break;
    case Qt::Key_Left:
      seq.append("\x1b[D");
      break;
    case Qt::Key_Home:
      seq.append("\x1b[H");
      break;
    case Qt::Key_End:
      seq.append("\x1b[F");
      break;
    case Qt::Key_PageUp:
      seq.append("\x1b[5~");
      break;
    case Qt::Key_PageDown:
      seq.append("\x1b[6~");
      break;
    case Qt::Key_Insert:
      seq.append("\x1b[2~");
      break;
    case Qt::Key_F1:
      seq.append("\x1bOP");
      break;
    case Qt::Key_F2:
      seq.append("\x1bOQ");
      break;
    case Qt::Key_F3:
      seq.append("\x1bOR");
      break;
    case Qt::Key_F4:
      seq.append("\x1bOS");
      break;
    case Qt::Key_F5:
      seq.append("\x1b[15~");
      break;
    case Qt::Key_F6:
      seq.append("\x1b[17~");
      break;
    case Qt::Key_F7:
      seq.append("\x1b[18~");
      break;
    case Qt::Key_F8:
      seq.append("\x1b[19~");
      break;
    case Qt::Key_F9:
      seq.append("\x1b[20~");
      break;
    case Qt::Key_F10:
      seq.append("\x1b[21~");
      break;
    case Qt::Key_F11:
      seq.append("\x1b[23~");
      break;
    case Qt::Key_F12:
      seq.append("\x1b[24~");
      break;
    default:
      break;
  }

  return seq;
}

/**
 * @brief Maps a Qt key event to its VT-100 byte sequence; empty if unmapped. Return and Enter
 *        expand to @p enterSequence, which the caller derives from the console line ending.
 */
QByteArray Widgets::Vt100Keymap::translate(const QKeyEvent* event, const QByteArray& enterSequence)
{
  SS_ASSERT(event != nullptr, return QByteArray());

  QByteArray seq;
  const Qt::KeyboardModifiers mods = event->modifiers();
  const int key                    = event->key();

  if ((mods & Qt::ControlModifier) && key >= Qt::Key_A && key <= Qt::Key_Z) {
    seq.append(char(key - Qt::Key_A + 1));
    return seq;
  }

  if ((mods & Qt::ControlModifier) && key == Qt::Key_BracketLeft) {
    seq.append('\x1b');
    return seq;
  }

  if (key == Qt::Key_Return || key == Qt::Key_Enter)
    return enterSequence;

  seq = specialKey(key);
  if (!seq.isEmpty())
    return seq;

  if (!event->text().isEmpty())
    seq = event->text().toUtf8();

  return seq;
}
