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

#include <QList>
#include <QRect>
#include <QString>

class QQuickItem;
class QQuickWindow;

namespace UI::Automation {

/**
 * @brief One visible top-level Qt Quick window as the automation surface reports it (spec 0101):
 *        identifier, title, frame in screen pixels, device pixel ratio, activation, whether a
 *        modal popup is open, and the identifier that holds keyboard focus.
 */
struct WindowInfo {
  QString id;
  QString title;
  QRect frame;
  qreal scale;
  bool active;
  bool modal;
  QString focusedId;
};

/**
 * @brief Enumerates the visible Qt Quick windows and resolves one by its objectName. Reads the
 *        live window list through public QGuiApplication and QQuickWindow API only and never
 *        spins an event loop, so every call is safe inside a command handler.
 */
class WindowInventory {
public:
  [[nodiscard]] static QList<QQuickWindow*> windows();
  [[nodiscard]] static QQuickWindow* resolve(const QString& id);
  [[nodiscard]] static WindowInfo describe(QQuickWindow* window);
  [[nodiscard]] static bool hasModalPopup(QQuickWindow* window);
  [[nodiscard]] static QString focusedId(QQuickWindow* window);
  [[nodiscard]] static QString nearestId(const QQuickItem* item);
};

}  // namespace UI::Automation
