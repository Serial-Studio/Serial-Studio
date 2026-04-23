/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

#include "SerialStudio.h"

namespace UI {
using WidgetID                      = quint64;
constexpr WidgetID kInvalidWidgetId = 0;

struct WidgetInfo {
  WidgetID id                        = kInvalidWidgetId;
  SerialStudio::DashboardWidget type = SerialStudio::DashboardNoWidget;

  QString title;
  QString icon;

  int groupId      = -1;
  int datasetIndex = -1;

  bool isGroupWidget = false;

  QVariant userData;

  [[nodiscard]] bool isValid() const { return id != kInvalidWidgetId; }
};

/**
 * @brief Central registry for all dashboard widgets with stable session-scoped IDs.
 */
class WidgetRegistry : public QObject {
  Q_OBJECT

signals:
  void registryCleared();
  void batchUpdateCompleted();
  void widgetDestroyed(UI::WidgetID id);
  void widgetCreated(UI::WidgetID id, const UI::WidgetInfo& info);
  void widgetUpdated(UI::WidgetID id, const UI::WidgetInfo& info);

private:
  WidgetRegistry();
  ~WidgetRegistry()                                = default;
  WidgetRegistry(const WidgetRegistry&)            = delete;
  WidgetRegistry& operator=(const WidgetRegistry&) = delete;

public:
  [[nodiscard]] static WidgetRegistry& instance();

  // clang-format off
  [[nodiscard]] int widgetCount() const;
  [[nodiscard]] bool isInBatchUpdate() const;
  [[nodiscard]] bool contains(WidgetID id) const;
  [[nodiscard]] QVector<WidgetID> allWidgetIds() const;
  [[nodiscard]] WidgetInfo widgetInfo(WidgetID id) const;
  [[nodiscard]] QVector<WidgetID> widgetIdsByGroup(int groupId) const;
  [[nodiscard]] QVector<WidgetID> widgetIdsByType(SerialStudio::DashboardWidget type) const;
  [[nodiscard]] WidgetID widgetIdByTypeAndIndex(SerialStudio::DashboardWidget type, int relativeIndex) const;
  [[nodiscard]] WidgetID createWidget(SerialStudio::DashboardWidget type,
                                      const QString &title,
                                      int groupId = -1,
                                      int datasetIndex = -1,
                                      bool isGroupWidget = true);
  // clang-format on

public slots:
  void clear();
  void endBatchUpdate();
  void beginBatchUpdate();
  void destroyWidget(WidgetID id);
  void updateWidget(WidgetID id,
                    const QString& title     = QString(),
                    const QString& icon      = QString(),
                    const QVariant& userData = QVariant());

private:
  WidgetID m_nextId;
  int m_batchDepth;
  bool m_batchHadChanges;

  QVector<WidgetID> m_widgetOrder;
  QHash<WidgetID, WidgetInfo> m_widgets;
};

}  // namespace UI
