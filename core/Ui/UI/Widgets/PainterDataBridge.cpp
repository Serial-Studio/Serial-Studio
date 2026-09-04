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

#ifdef BUILD_COMMERCIAL

#  include "UI/Widgets/PainterDataBridge.h"

#  include "DataModel/Frame.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initialises the bridge with no bound group and a zero frame stamp.
 */
Widgets::PainterDataBridge::PainterDataBridge(QObject* parent)
  : QObject(parent), m_group(nullptr), m_frameNumber(0), m_frameTimestampMs(0)
{}

//--------------------------------------------------------------------------------------------------
// Per-tick mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Points the bridge at the group whose datasets the script should read.
 */
void Widgets::PainterDataBridge::setGroup(const DataModel::Group* group)
{
  m_group = group;
}

/**
 * @brief Refreshes the per-tick frame counter and wall-clock stamp.
 */
void Widgets::PainterDataBridge::setFrame(qulonglong frameNumber, qint64 timestampMs)
{
  m_frameNumber      = frameNumber;
  m_frameTimestampMs = timestampMs;
}

//--------------------------------------------------------------------------------------------------
// Group accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the bound group's groupId, or -1 when nothing is bound.
 */
int Widgets::PainterDataBridge::gId() const
{
  return m_group ? m_group->groupId : -1;
}

/**
 * @brief Returns the bound group's title.
 */
QString Widgets::PainterDataBridge::gTitle() const
{
  return m_group ? m_group->title : QString();
}

/**
 * @brief Returns the bound group's column count.
 */
int Widgets::PainterDataBridge::gColumns() const
{
  return m_group ? m_group->columns : 0;
}

/**
 * @brief Returns the source ID feeding this group.
 */
int Widgets::PainterDataBridge::gSourceId() const
{
  return m_group ? m_group->sourceId : 0;
}

//--------------------------------------------------------------------------------------------------
// Dataset accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the dataset count for the bound group.
 */
int Widgets::PainterDataBridge::dsCount() const
{
  return m_group ? static_cast<int>(m_group->datasets.size()) : 0;
}

/**
 * @brief Returns the dataset's frame index, or -1 if out of range.
 */
int Widgets::PainterDataBridge::dsIndex(int i) const
{
  return valid(i) ? m_group->datasets[i].index : -1;
}

/**
 * @brief Returns the dataset's stable unique ID.
 */
int Widgets::PainterDataBridge::dsUniqueId(int i) const
{
  return valid(i) ? m_group->datasets[i].uniqueId : -1;
}

/**
 * @brief Returns the dataset's title.
 */
QString Widgets::PainterDataBridge::dsTitle(int i) const
{
  return valid(i) ? m_group->datasets[i].title : QString();
}

/**
 * @brief Returns the dataset's measurement units.
 */
QString Widgets::PainterDataBridge::dsUnits(int i) const
{
  return valid(i) ? m_group->datasets[i].units : QString();
}

/**
 * @brief Returns the dataset's widget type ("bar", "gauge", ...).
 */
QString Widgets::PainterDataBridge::dsWidget(int i) const
{
  return valid(i) ? m_group->datasets[i].widget : QString();
}

/**
 * @brief Returns the post-transform numeric value, or NaN if invalid.
 */
double Widgets::PainterDataBridge::dsValue(int i) const
{
  return valid(i) ? m_group->datasets[i].numericValue : qQNaN();
}

/**
 * @brief Returns the pre-transform numeric value, or NaN if invalid.
 */
double Widgets::PainterDataBridge::dsRawValue(int i) const
{
  return valid(i) ? m_group->datasets[i].rawNumericValue : qQNaN();
}

/**
 * @brief Returns the post-transform string value.
 */
QString Widgets::PainterDataBridge::dsValueStr(int i) const
{
  return valid(i) ? m_group->datasets[i].value : QString();
}

/**
 * @brief Returns the pre-transform string value.
 */
QString Widgets::PainterDataBridge::dsRawValueStr(int i) const
{
  return valid(i) ? m_group->datasets[i].rawValue : QString();
}

/**
 * @brief Returns the dataset's "best" lower bound.
 */
double Widgets::PainterDataBridge::dsMin(int i) const
{
  if (!valid(i))
    return 0.0;

  const auto& d = m_group->datasets[i];
  if (d.wgtMin != 0.0 || d.wgtMax != 0.0)
    return d.wgtMin;

  if (d.pltMin != 0.0 || d.pltMax != 0.0)
    return d.pltMin;

  if (d.fftMin != 0.0 || d.fftMax != 0.0)
    return d.fftMin;

  return 0.0;
}

/**
 * @brief Returns the dataset's "best" upper bound (mirror of dsMin).
 */
double Widgets::PainterDataBridge::dsMax(int i) const
{
  if (!valid(i))
    return 0.0;

  const auto& d = m_group->datasets[i];
  if (d.wgtMin != 0.0 || d.wgtMax != 0.0)
    return d.wgtMax;

  if (d.pltMin != 0.0 || d.pltMax != 0.0)
    return d.pltMax;

  if (d.fftMin != 0.0 || d.fftMax != 0.0)
    return d.fftMax;

  return 0.0;
}

/**
 * @brief Returns the widget-specific lower bound.
 */
double Widgets::PainterDataBridge::dsWidgetMin(int i) const
{
  return valid(i) ? m_group->datasets[i].wgtMin : 0.0;
}

/**
 * @brief Returns the widget-specific upper bound.
 */
double Widgets::PainterDataBridge::dsWidgetMax(int i) const
{
  return valid(i) ? m_group->datasets[i].wgtMax : 0.0;
}

/**
 * @brief Returns the plot lower bound (independent of widget bounds).
 */
double Widgets::PainterDataBridge::dsPlotMin(int i) const
{
  return valid(i) ? m_group->datasets[i].pltMin : 0.0;
}

/**
 * @brief Returns the plot upper bound.
 */
double Widgets::PainterDataBridge::dsPlotMax(int i) const
{
  return valid(i) ? m_group->datasets[i].pltMax : 0.0;
}

/**
 * @brief Returns the FFT lower bound.
 */
double Widgets::PainterDataBridge::dsFftMin(int i) const
{
  return valid(i) ? m_group->datasets[i].fftMin : 0.0;
}

/**
 * @brief Returns the FFT upper bound.
 */
double Widgets::PainterDataBridge::dsFftMax(int i) const
{
  return valid(i) ? m_group->datasets[i].fftMax : 0.0;
}

/**
 * @brief Returns the lower edge of the first Warning+ band; NaN when none defined.
 */
double Widgets::PainterDataBridge::dsAlarmLow(int i) const
{
  if (!valid(i))
    return 0.0;

  for (const auto& band : m_group->datasets[i].alarmBands)
    if (static_cast<int>(band.severity) >= 2)
      return qMin(band.min, band.max);

  return std::numeric_limits<double>::quiet_NaN();
}

/**
 * @brief Returns the upper edge of the last Warning+ band; NaN when none defined.
 */
double Widgets::PainterDataBridge::dsAlarmHigh(int i) const
{
  if (!valid(i))
    return 0.0;

  double hi = std::numeric_limits<double>::quiet_NaN();
  for (const auto& band : m_group->datasets[i].alarmBands)
    if (static_cast<int>(band.severity) >= 2)
      hi = qMax(band.min, band.max);

  return hi;
}

/**
 * @brief Returns the dataset's alarm bands as a list of {min, max, severity, color, label,
 *        blink}.
 */
QVariantList Widgets::PainterDataBridge::dsAlarmBands(int i) const
{
  QVariantList out;
  if (!valid(i))
    return out;

  out.reserve(static_cast<int>(m_group->datasets[i].alarmBands.size()));
  for (const auto& band : m_group->datasets[i].alarmBands) {
    QVariantMap entry;
    entry.insert(QStringLiteral("min"), qMin(band.min, band.max));
    entry.insert(QStringLiteral("max"), qMax(band.min, band.max));
    entry.insert(QStringLiteral("severity"), static_cast<int>(band.severity));
    entry.insert(QStringLiteral("color"), band.color);
    entry.insert(QStringLiteral("label"), band.label);
    entry.insert(QStringLiteral("blink"), band.blink);
    out.append(entry);
  }

  return out;
}

/**
 * @brief Returns the dataset's LED activation threshold.
 */
double Widgets::PainterDataBridge::dsLedHigh(int i) const
{
  return valid(i) ? m_group->datasets[i].ledHigh : 0.0;
}

/**
 * @brief Returns true when this dataset has a plot tile enabled.
 */
bool Widgets::PainterDataBridge::dsHasPlot(int i) const
{
  return valid(i) ? m_group->datasets[i].plt : false;
}

/**
 * @brief Returns true when this dataset has FFT visualisation enabled.
 */
bool Widgets::PainterDataBridge::dsHasFft(int i) const
{
  return valid(i) ? m_group->datasets[i].fft : false;
}

/**
 * @brief Returns true when this dataset has an LED tile enabled.
 */
bool Widgets::PainterDataBridge::dsHasLed(int i) const
{
  return valid(i) ? m_group->datasets[i].led : false;
}

//--------------------------------------------------------------------------------------------------
// Frame metadata
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the running frame counter from the dashboard.
 */
qulonglong Widgets::PainterDataBridge::frameNumber() const
{
  return m_frameNumber;
}

/**
 * @brief Returns the wall-clock timestamp of the last frame in ms.
 */
qint64 Widgets::PainterDataBridge::frameTimestampMs() const
{
  return m_frameTimestampMs;
}

//--------------------------------------------------------------------------------------------------
// Console capture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forwards a console.log message to the consoleLine signal.
 */
void Widgets::PainterDataBridge::log(const QString& msg)
{
  Q_EMIT consoleLine(0, msg);
}

/**
 * @brief Forwards a console.warn message to the consoleLine signal.
 */
void Widgets::PainterDataBridge::warn(const QString& msg)
{
  Q_EMIT consoleLine(1, msg);
}

/**
 * @brief Forwards a console.error message to the consoleLine signal.
 */
void Widgets::PainterDataBridge::error(const QString& msg)
{
  Q_EMIT consoleLine(2, msg);
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when index `i` falls inside the bound group's datasets.
 */
bool Widgets::PainterDataBridge::valid(int i) const
{
  return m_group && i >= 0 && static_cast<size_t>(i) < m_group->datasets.size();
}

#endif  // BUILD_COMMERCIAL
