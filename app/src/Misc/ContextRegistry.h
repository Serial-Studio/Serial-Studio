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
#include <QString>
#include <QStringList>
#include <QVariant>
#include <utility>

class QObject;
class QQmlContext;

namespace Misc {

/**
 * @brief The QML context globals, collected then registered in one loop (spec 0075, G4). The name
 *        tables in the .cpp are the single source of truth: the composition root registers through
 *        them, `UI::WidgetExtensions::hostContextNames()` shadows them, and `registry-verify.py`
 *        fails a name that drifts out of either side.
 */
class ContextRegistry {
public:
  ContextRegistry() = default;

  void add(const QString& name, QObject* object);
  void add(const QString& name, const QVariant& value);
  void apply(QQmlContext* context) const;

  [[nodiscard]] static const QStringList& names();
  [[nodiscard]] static const QStringList& objectNames();
  [[nodiscard]] static const QStringList& valueNames();

private:
  QList<std::pair<QString, QObject*>> m_objects;
  QList<std::pair<QString, QVariant>> m_values;
};

}  // namespace Misc
