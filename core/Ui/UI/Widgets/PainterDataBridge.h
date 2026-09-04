/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QObject>
#  include <QString>
#  include <QVariant>
#  include <QVariantList>

namespace DataModel {
struct Group;
}  // namespace DataModel

namespace Widgets {

/**
 * @brief Per-painter-widget data bridge exposing this group's metadata to JS.
 */
class PainterDataBridge : public QObject {
  Q_OBJECT

public:
  explicit PainterDataBridge(QObject* parent = nullptr);
  ~PainterDataBridge() override = default;

  void setGroup(const DataModel::Group* group);
  void setFrame(qulonglong frameNumber, qint64 timestampMs);

public slots:
  [[nodiscard]] int gId() const;
  [[nodiscard]] QString gTitle() const;
  [[nodiscard]] int gColumns() const;
  [[nodiscard]] int gSourceId() const;

  [[nodiscard]] int dsCount() const;
  [[nodiscard]] int dsIndex(int i) const;
  [[nodiscard]] int dsUniqueId(int i) const;
  [[nodiscard]] QString dsTitle(int i) const;
  [[nodiscard]] QString dsUnits(int i) const;
  [[nodiscard]] QString dsWidget(int i) const;
  [[nodiscard]] double dsValue(int i) const;
  [[nodiscard]] double dsRawValue(int i) const;
  [[nodiscard]] QString dsValueStr(int i) const;
  [[nodiscard]] QString dsRawValueStr(int i) const;
  [[nodiscard]] double dsMin(int i) const;
  [[nodiscard]] double dsMax(int i) const;
  [[nodiscard]] double dsWidgetMin(int i) const;
  [[nodiscard]] double dsWidgetMax(int i) const;
  [[nodiscard]] double dsPlotMin(int i) const;
  [[nodiscard]] double dsPlotMax(int i) const;
  [[nodiscard]] double dsFftMin(int i) const;
  [[nodiscard]] double dsFftMax(int i) const;
  [[nodiscard]] double dsAlarmLow(int i) const;
  [[nodiscard]] double dsAlarmHigh(int i) const;
  [[nodiscard]] QVariantList dsAlarmBands(int i) const;
  [[nodiscard]] double dsLedHigh(int i) const;
  [[nodiscard]] bool dsHasPlot(int i) const;
  [[nodiscard]] bool dsHasFft(int i) const;
  [[nodiscard]] bool dsHasLed(int i) const;

  [[nodiscard]] qulonglong frameNumber() const;
  [[nodiscard]] qint64 frameTimestampMs() const;

  void log(const QString& msg);
  void warn(const QString& msg);
  void error(const QString& msg);

signals:
  void consoleLine(int level, const QString& text);

private:
  [[nodiscard]] bool valid(int i) const;

  const DataModel::Group* m_group;
  qulonglong m_frameNumber;
  qint64 m_frameTimestampMs;
};

}  // namespace Widgets

#endif  // BUILD_COMMERCIAL
