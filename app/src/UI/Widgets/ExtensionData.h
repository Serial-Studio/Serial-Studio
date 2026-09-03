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

#include <QAbstractListModel>
#include <QQuickItem>
#include <QVariant>
#include <QVariantMap>
#include <QVector>

#include "DataModel/Frame.h"
#include "UI/Dashboard.h"

namespace Widgets {

/**
 * @brief One row of an extension widget's data: the dataset's identity, its formatted and raw
 *        values, and the dashboard widgets that also display it.
 */
struct ExtensionRow {
  bool isNumeric      = false;
  bool alarmsDefined  = false;
  int index           = 0;
  int uniqueId        = -1;
  int decimalPoints   = -1;
  int alarmSeverity   = -1;
  double numericValue = 0;
  double minValue     = 0;
  double maxValue     = 0;
  QString title;
  QString text;
  QString value;
  QString units;
  QString displayFormat;
  QVariantList widgets;
};

/**
 * @brief List model backing an extension widget's datasets with per-row dataChanged() updates,
 *        so a package renders a fifty-dataset group without rebuilding its delegates every tick.
 */
class ExtensionRowsModel : public QAbstractListModel {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  enum Role {
    TitleRole = Qt::UserRole + 1,
    TextRole,
    ValueRole,
    UnitsRole,
    IndexRole,
    MinimumRole,
    MaximumRole,
    UniqueIdRole,
    IsNumericRole,
    NumericValueRole,
    DecimalPointsRole,
    DisplayFormatRole,
    AlarmsDefinedRole,
    AlarmSeverityRole,
    WidgetsRole,
  };

  explicit ExtensionRowsModel(QObject* parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  void reset(const QVector<ExtensionRow>& rows);
  bool updateRow(int row, const ExtensionRow& fresh);

private:
  QVector<ExtensionRow> m_rows;
};

/**
 * @brief The one model every widget-extension package receives. It republishes the group or
 *        dataset payload the host already resolved on the dashboard's update tick, plus the
 *        package's declared configuration, so no extension QML touches a host singleton.
 */
class ExtensionData : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool paused
             READ  paused
             WRITE setPaused
             NOTIFY pausedChanged)
  Q_PROPERTY(bool groupScope
             READ groupScope
             CONSTANT)
  Q_PROPERTY(bool alarmsDefined
             READ alarmsDefined
             NOTIFY updated)
  Q_PROPERTY(bool alarmTriggered
             READ alarmTriggered
             NOTIFY updated)
  Q_PROPERTY(bool isNumeric
             READ isNumeric
             NOTIFY updated)
  Q_PROPERTY(int groupId
             READ groupId
             CONSTANT)
  Q_PROPERTY(int sourceId
             READ sourceId
             CONSTANT)
  Q_PROPERTY(int uniqueId
             READ uniqueId
             CONSTANT)
  Q_PROPERTY(int datasetCount
             READ datasetCount
             NOTIFY updated)
  Q_PROPERTY(int decimalPoints
             READ decimalPoints
             NOTIFY updated)
  Q_PROPERTY(int alarmSeverity
             READ alarmSeverity
             NOTIFY updated)
  Q_PROPERTY(double value
             READ value
             NOTIFY updated)
  Q_PROPERTY(double minValue
             READ minValue
             NOTIFY updated)
  Q_PROPERTY(double maxValue
             READ maxValue
             NOTIFY updated)
  Q_PROPERTY(QString text
             READ text
             NOTIFY updated)
  Q_PROPERTY(QString title
             READ title
             NOTIFY updated)
  Q_PROPERTY(QString units
             READ units
             NOTIFY updated)
  Q_PROPERTY(QString stringValue
             READ stringValue
             NOTIFY updated)
  Q_PROPERTY(QString displayFormat
             READ displayFormat
             NOTIFY updated)
  Q_PROPERTY(QString extensionId
             READ extensionId
             CONSTANT)
  Q_PROPERTY(QVariantMap config
             READ config
             NOTIFY configChanged)
  Q_PROPERTY(ExtensionRowsModel* datasets
             READ datasets
             CONSTANT)
  // clang-format on

signals:
  void updated();
  void pausedChanged();
  void configChanged();

public:
  explicit ExtensionData(const QString& extensionId               = QString(),
                         const SerialStudio::DashboardWidget type = SerialStudio::DashboardNoWidget,
                         const int index                          = -1,
                         QQuickItem* parent                       = nullptr);

  [[nodiscard]] bool paused() const noexcept;
  [[nodiscard]] bool isNumeric() const noexcept;
  [[nodiscard]] bool groupScope() const noexcept;
  [[nodiscard]] bool alarmsDefined() const noexcept;
  [[nodiscard]] bool alarmTriggered() const noexcept;
  [[nodiscard]] int groupId() const;
  [[nodiscard]] int sourceId() const;
  [[nodiscard]] int uniqueId() const;
  [[nodiscard]] int datasetCount() const noexcept;
  [[nodiscard]] int decimalPoints() const noexcept;
  [[nodiscard]] int alarmSeverity() const noexcept;
  [[nodiscard]] double value() const noexcept;
  [[nodiscard]] double minValue() const noexcept;
  [[nodiscard]] double maxValue() const noexcept;
  [[nodiscard]] const QString& text() const noexcept;
  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] const QString& units() const noexcept;
  [[nodiscard]] const QString& stringValue() const noexcept;
  [[nodiscard]] const QString& displayFormat() const noexcept;
  [[nodiscard]] const QString& extensionId() const noexcept;
  [[nodiscard]] const QVariantMap& config() const noexcept;
  [[nodiscard]] ExtensionRowsModel* datasets() const noexcept;

public slots:
  void setPaused(const bool paused);
  void setConfigValue(const QString& key, const QVariant& value);

private slots:
  void updateData();
  void reloadConfig();

private:
  [[nodiscard]] bool valid() const;
  [[nodiscard]] QString widgetId() const;
  [[nodiscard]] QString currentTitle() const;
  [[nodiscard]] int sourceDatasetCount() const;
  [[nodiscard]] const DataModel::Dataset& datasetAt(int index) const;
  [[nodiscard]] QVector<ExtensionRow> collectRows() const;
  [[nodiscard]] ExtensionRow buildRow(const DataModel::Dataset& dataset) const;
  [[nodiscard]] ExtensionRow buildVolatileRow(const DataModel::Dataset& dataset) const;
  [[nodiscard]] QVariantList datasetWidgets(const DataModel::Dataset& dataset) const;
  [[nodiscard]] bool refreshLead(const ExtensionRow& row);
  void rebuildRows();

private:
  bool m_paused;
  bool m_groupScope;
  int m_index;
  int m_bucketIndex;
  int m_lastRowCount;
  SerialStudio::DashboardWidget m_type;

  QString m_title;
  QString m_extensionId;
  QVariantMap m_config;
  ExtensionRow m_lead;
  ExtensionRowsModel* m_rowsModel;

  UI::Dashboard& m_dashboard;
};
}  // namespace Widgets
