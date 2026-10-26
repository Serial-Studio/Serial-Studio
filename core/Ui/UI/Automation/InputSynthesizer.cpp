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

#include "UI/Automation/InputSynthesizer.h"

#include <algorithm>
#include <iterator>
#include <QGuiApplication>
#include <qpa/qwindowsysteminterface.h>
#include <QQuickItem>
#include <QQuickWindow>
#include <QStyleHints>

#include "Core/SSAssert.h"

using namespace UI::Automation;
using Delivery = QWindowSystemInterface::SynchronousDelivery;

static ulong s_mouseTimestamp = 0;

//--------------------------------------------------------------------------------------------------
// Synthetic clock and modifier table
//--------------------------------------------------------------------------------------------------

namespace detail {
struct ModifierKey {
  Qt::KeyboardModifier flag;
  Qt::Key key;
};

enum class ModifierPhase : quint8 {
  Press,
  Release,
};
}  // namespace detail

using detail::ModifierKey;
using detail::ModifierPhase;

static constexpr ModifierKey kModifierKeys[] = {
  {  Qt::ShiftModifier,   Qt::Key_Shift},
  {Qt::ControlModifier, Qt::Key_Control},
  {    Qt::AltModifier,     Qt::Key_Alt},
  {   Qt::MetaModifier,    Qt::Key_Meta},
};

/**
 * @brief The exported mouse entry QTest uses (qtestsupport_gui.cpp): converts the logical window
 *        point to the native pixels the QPA handler expects, which matters on every scaled
 *        display, and delivers synchronously. Declared here because its only public declaration
 *        lives in QtTest.
 */
Q_GUI_EXPORT void qt_handleMouseEvent(QWindow* window,
                                      const QPointF& local,
                                      const QPointF& global,
                                      Qt::MouseButtons state,
                                      Qt::MouseButton button,
                                      QEvent::Type type,
                                      Qt::KeyboardModifiers mods,
                                      int timestamp);

//--------------------------------------------------------------------------------------------------
// Delivery helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Advances the synthetic clock past the double-click interval, so the next press cannot
 *        fuse with the previous click into a double click.
 */
static void separateClicks()
{
  const int interval  = QGuiApplication::styleHints()->mouseDoubleClickInterval();
  s_mouseTimestamp   += static_cast<ulong>(std::max(interval, 0)) + 1;
}

/**
 * @brief One mouse event on the next clock tick, in logical window coordinates, dispatched before
 *        this returns.
 */
static void deliverMouse(QQuickWindow* window,
                         const QPointF& local,
                         const Qt::MouseButtons state,
                         const Qt::MouseButton button,
                         const QEvent::Type type,
                         const Qt::KeyboardModifiers modifiers)
{
  ++s_mouseTimestamp;
  qt_handleMouseEvent(window,
                      local,
                      window->mapToGlobal(local),
                      state,
                      button,
                      type,
                      modifiers,
                      static_cast<int>(s_mouseTimestamp));
}

/**
 * @brief One key event, dispatched before this returns.
 */
[[nodiscard]] static bool deliverKey(QQuickWindow* window,
                                     const QEvent::Type type,
                                     const int key,
                                     const Qt::KeyboardModifiers modifiers,
                                     const QString& text)
{
  return QWindowSystemInterface::handleKeyEvent<Delivery>(window, type, key, modifiers, text);
}

/**
 * @brief Presses (or releases, in reverse order) the modifier keys of @p modifiers around a key,
 *        each event carrying the modifiers already held, exactly as QTest does.
 */
static void sendModifierKeys(QQuickWindow* window,
                             const Qt::KeyboardModifiers modifiers,
                             const ModifierPhase phase)
{
  const bool press           = phase == ModifierPhase::Press;
  const int count            = static_cast<int>(std::size(kModifierKeys));
  Qt::KeyboardModifiers held = press ? Qt::KeyboardModifiers() : modifiers;
  for (int i = 0; i < count; ++i) {
    const ModifierKey& entry = kModifierKeys[press ? i : count - 1 - i];
    if (!modifiers.testFlag(entry.flag))
      continue;

    const auto type = press ? QEvent::KeyPress : QEvent::KeyRelease;
    (void)deliverKey(window, type, entry.key, held, QString());
    held.setFlag(entry.flag, press);
  }
}

/**
 * @brief One press-release pair for @p key with its modifier keys around it.
 */
[[nodiscard]] static bool tapKey(QQuickWindow* window,
                                 const int key,
                                 const Qt::KeyboardModifiers modifiers,
                                 const QString& text)
{
  sendModifierKeys(window, modifiers, ModifierPhase::Press);
  const bool accepted = deliverKey(window, QEvent::KeyPress, key, modifiers, text);
  (void)deliverKey(window, QEvent::KeyRelease, key, modifiers, text);
  sendModifierKeys(window, modifiers, ModifierPhase::Release);
  return accepted;
}

//--------------------------------------------------------------------------------------------------
// Mouse
//--------------------------------------------------------------------------------------------------

/**
 * @brief Move, press and release at @p point; a double click repeats the pair on adjacent ticks
 *        so Qt synthesizes the double-click event itself (the platform layer refuses an explicit
 *        one). Returns true once the events were dispatched.
 */
bool InputSynthesizer::click(QQuickWindow* window,
                             const QPointF& point,
                             const Qt::MouseButton button,
                             const Qt::KeyboardModifiers modifiers,
                             const bool doubleClick)
{
  SS_ASSERT(window != nullptr, return false);
  SS_ASSERT(button != Qt::NoButton, return false);

  separateClicks();
  deliverMouse(window, point, Qt::NoButton, Qt::NoButton, QEvent::MouseMove, modifiers);
  deliverMouse(window, point, button, button, QEvent::MouseButtonPress, modifiers);
  deliverMouse(window, point, Qt::NoButton, button, QEvent::MouseButtonRelease, modifiers);
  if (!doubleClick)
    return true;

  deliverMouse(window, point, button, button, QEvent::MouseButtonPress, modifiers);
  deliverMouse(window, point, Qt::NoButton, button, QEvent::MouseButtonRelease, modifiers);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Keyboard
//--------------------------------------------------------------------------------------------------

/**
 * @brief Taps every combination of @p sequence in order (a QKeySequence holds at most four).
 *        Returns whether any press was accepted.
 */
bool InputSynthesizer::keys(QQuickWindow* window, const QKeySequence& sequence)
{
  SS_ASSERT(window != nullptr, return false);
  if (sequence.isEmpty())
    return false;

  bool accepted   = false;
  const int count = std::min(sequence.count(), kMaxSequenceKeys);
  for (int i = 0; i < count; ++i) {
    const QKeyCombination combination = sequence[i];
    if (combination.key() == Qt::Key_unknown)
      continue;

    const Qt::KeyboardModifiers modifiers = combination.keyboardModifiers();
    const QString text                    = keyText(combination.key(), modifiers);
    accepted = tapKey(window, combination.key(), modifiers, text) || accepted;
  }

  return accepted;
}

/**
 * @brief Types @p text one character at a time (a surrogate pair travels as one event), each as
 *        a press-release pair carrying the character as the event text; newlines tap Return.
 */
bool InputSynthesizer::text(QQuickWindow* window, const QString& text)
{
  SS_ASSERT(window != nullptr, return false);
  if (text.isEmpty() || text.size() > kMaxTextLength)
    return false;

  bool accepted = false;
  qsizetype i   = 0;
  while (i < text.size()) {
    const QChar first    = text.at(i);
    const bool paired    = first.isHighSurrogate() && i + 1 < text.size();
    const QString unit   = text.mid(i, paired ? 2 : 1);
    const int key        = keyForCharacter(first);
    const auto modifier  = first.isUpper() ? Qt::ShiftModifier : Qt::NoModifier;
    const QString event  = key == Qt::Key_Return ? QStringLiteral("\r") : unit;
    accepted             = tapKey(window, key, modifier, event) || accepted;
    i                   += paired ? 2 : 1;
  }

  return accepted;
}

/**
 * @brief The event text a key produces: none under Control, Alt or Meta, the control character
 *        for Return, Tab, Backspace and Escape, the printable character (shifted or not) in the
 *        ASCII range, nothing for everything else.
 */
QString InputSynthesizer::keyText(const Qt::Key key, const Qt::KeyboardModifiers modifiers)
{
  if (modifiers.testAnyFlags(Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))
    return QString();

  switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
      return QStringLiteral("\r");
    case Qt::Key_Tab:
      return QStringLiteral("\t");
    case Qt::Key_Backspace:
      return QStringLiteral("\b");
    case Qt::Key_Escape:
      return QStringLiteral("\x1b");
    default:
      break;
  }

  if (key < Qt::Key_Space || key > Qt::Key_AsciiTilde)
    return QString();

  const QChar character(static_cast<char16_t>(key));
  const QChar cased =
    modifiers.testFlag(Qt::ShiftModifier) ? character.toUpper() : character.toLower();
  return QString(cased);
}

/**
 * @brief The Qt key for a typed character: Return for a line break, Tab, the upper-cased ASCII
 *        code for printable Latin characters (Qt's key values coincide with it), unknown for the
 *        rest, which still insert through the event text.
 */
int InputSynthesizer::keyForCharacter(const QChar character)
{
  if (character == QLatin1Char('\n') || character == QLatin1Char('\r'))
    return Qt::Key_Return;

  if (character == QLatin1Char('\t'))
    return Qt::Key_Tab;

  const char16_t code = character.toUpper().unicode();
  if (code >= Qt::Key_Space && code <= Qt::Key_AsciiTilde)
    return static_cast<int>(code);

  return static_cast<int>(Qt::Key_unknown);
}

//--------------------------------------------------------------------------------------------------
// Focus
//--------------------------------------------------------------------------------------------------

/**
 * @brief Hands keyboard focus to @p item through its focus scopes.
 */
void InputSynthesizer::focus(QQuickItem* item)
{
  SS_ASSERT(item != nullptr, return);
  item->forceActiveFocus();
}
