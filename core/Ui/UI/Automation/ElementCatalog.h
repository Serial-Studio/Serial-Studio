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

#include <optional>
#include <QJsonObject>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QString>

struct QMetaObject;
class QQuickItem;
class QQuickWindow;

namespace UI::Automation {

/**
 * @brief One interactive control as the catalog lists it: identifier, accessibility role, what
 *        it shows, where it sits in window coordinates, and its state. The item pointer is a
 *        non-owning handle valid only inside the command that built the entry.
 */
struct Element {
  QString id;
  QString role;
  QString label;
  QString text;
  QRectF rect;
  bool enabled;
  bool focused;
  std::optional<bool> checked;
  QQuickItem* item;
};

/**
 * @brief Case-insensitive substring filters on identifier and label; empty matches everything.
 */
struct CatalogFilter {
  QString id;
  QString label;
};

/**
 * @brief The bounded item-tree walk behind listElements, click and key (spec 0101): classifies
 *        controls by Qt Quick Controls type and by the repository's QML component name, reports
 *        geometry in window coordinates, and resolves a control by identifier or by point. The
 *        depth and item caps make the walk a fixed-bound loop; it allocates only the result list.
 */
class ElementCatalog {
public:
  static constexpr int kMaxDepth = 64;
  static constexpr int kMaxItems = 16384;

  [[nodiscard]] static QList<Element> collect(QQuickWindow* window,
                                              const CatalogFilter& filter = CatalogFilter());
  [[nodiscard]] static std::optional<Element> byId(QQuickWindow* window, const QString& id);
  [[nodiscard]] static std::optional<Element> atPoint(QQuickWindow* window, const QPointF& point);
  [[nodiscard]] static Element describe(QQuickItem* item, const QString& role);
  [[nodiscard]] static QString roleOf(const QQuickItem* item);
  [[nodiscard]] static QString componentName(const QMetaObject* meta);
  [[nodiscard]] static QJsonObject toJson(const Element& element);
};

}  // namespace UI::Automation
