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
  struct KeySequence {
    int key;
    const char* seq;
  };

  static constexpr KeySequence kTable[] = {
    {Qt::Key_Backspace,     "\x7f"},
    {   Qt::Key_Delete,  "\x1b[3~"},
    {      Qt::Key_Tab,       "\t"},
    {  Qt::Key_Backtab,   "\x1b[Z"},
    {   Qt::Key_Escape,     "\x1b"},
    {       Qt::Key_Up,   "\x1b[A"},
    {     Qt::Key_Down,   "\x1b[B"},
    {    Qt::Key_Right,   "\x1b[C"},
    {     Qt::Key_Left,   "\x1b[D"},
    {     Qt::Key_Home,   "\x1b[H"},
    {      Qt::Key_End,   "\x1b[F"},
    {   Qt::Key_PageUp,  "\x1b[5~"},
    { Qt::Key_PageDown,  "\x1b[6~"},
    {   Qt::Key_Insert,  "\x1b[2~"},
    {       Qt::Key_F1,   "\x1bOP"},
    {       Qt::Key_F2,   "\x1bOQ"},
    {       Qt::Key_F3,   "\x1bOR"},
    {       Qt::Key_F4,   "\x1bOS"},
    {       Qt::Key_F5, "\x1b[15~"},
    {       Qt::Key_F6, "\x1b[17~"},
    {       Qt::Key_F7, "\x1b[18~"},
    {       Qt::Key_F8, "\x1b[19~"},
    {       Qt::Key_F9, "\x1b[20~"},
    {      Qt::Key_F10, "\x1b[21~"},
    {      Qt::Key_F11, "\x1b[23~"},
    {      Qt::Key_F12, "\x1b[24~"},
  };

  for (const auto& entry : kTable)
    if (entry.key == key)
      return QByteArray(entry.seq);

  return QByteArray();
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
