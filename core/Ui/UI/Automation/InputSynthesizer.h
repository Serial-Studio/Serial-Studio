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

#include <QKeySequence>
#include <QPointF>
#include <QString>
#include <Qt>

class QQuickItem;
class QQuickWindow;

namespace UI::Automation {

/**
 * @brief Delivers synthetic mouse and key input to a Qt Quick window the way QTest does (spec
 *        0101): logical window points converted to native pixels, synchronous delivery with no
 *        event loop inside a command, coherent button and modifier state, and single clicks
 *        spaced past the double-click interval.
 */
class InputSynthesizer {
public:
  static constexpr int kMaxTextLength   = 4096;
  static constexpr int kMaxSequenceKeys = 4;

  [[nodiscard]] static bool click(QQuickWindow* window,
                                  const QPointF& point,
                                  Qt::MouseButton button,
                                  Qt::KeyboardModifiers modifiers,
                                  bool doubleClick);
  [[nodiscard]] static bool keys(QQuickWindow* window, const QKeySequence& sequence);
  [[nodiscard]] static bool text(QQuickWindow* window, const QString& text);
  [[nodiscard]] static QString keyText(Qt::Key key, Qt::KeyboardModifiers modifiers);
  [[nodiscard]] static int keyForCharacter(QChar character);
  static void focus(QQuickItem* item);
};

}  // namespace UI::Automation
