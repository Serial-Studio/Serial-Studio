/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/EnumLabels.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
// BusType
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a short slug for a BusType value.
 */
QString API::EnumLabels::busTypeSlug(int value)
{
  switch (static_cast<SerialStudio::BusType>(value)) {
    case SerialStudio::BusType::UART:
      return QStringLiteral("uart");
    case SerialStudio::BusType::Network:
      return QStringLiteral("network");
    case SerialStudio::BusType::BluetoothLE:
      return QStringLiteral("bluetooth-le");
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      return QStringLiteral("audio");
    case SerialStudio::BusType::ModBus:
      return QStringLiteral("modbus");
    case SerialStudio::BusType::CanBus:
      return QStringLiteral("canbus");
    case SerialStudio::BusType::RawUsb:
      return QStringLiteral("usb");
    case SerialStudio::BusType::HidDevice:
      return QStringLiteral("hid");
    case SerialStudio::BusType::Process:
      return QStringLiteral("process");
    case SerialStudio::BusType::Mqtt:
      return QStringLiteral("mqtt");
#endif
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns a human-friendly label for a BusType value.
 */
QString API::EnumLabels::busTypeLabel(int value)
{
  switch (static_cast<SerialStudio::BusType>(value)) {
    case SerialStudio::BusType::UART:
      return QStringLiteral("UART (serial port)");
    case SerialStudio::BusType::Network:
      return QStringLiteral("Network (TCP/UDP)");
    case SerialStudio::BusType::BluetoothLE:
      return QStringLiteral("Bluetooth LE");
#ifdef BUILD_COMMERCIAL
    case SerialStudio::BusType::Audio:
      return QStringLiteral("Audio input");
    case SerialStudio::BusType::ModBus:
      return QStringLiteral("Modbus");
    case SerialStudio::BusType::CanBus:
      return QStringLiteral("CAN bus");
    case SerialStudio::BusType::RawUsb:
      return QStringLiteral("USB (libusb)");
    case SerialStudio::BusType::HidDevice:
      return QStringLiteral("HID");
    case SerialStudio::BusType::Process:
      return QStringLiteral("Process I/O");
    case SerialStudio::BusType::Mqtt:
      return QStringLiteral("MQTT subscriber");
#endif
  }
  return QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// FrameDetection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a short slug for a FrameDetection value.
 */
QString API::EnumLabels::frameDetectionSlug(int value)
{
  switch (static_cast<SerialStudio::FrameDetection>(value)) {
    case SerialStudio::EndDelimiterOnly:
      return QStringLiteral("end-delimiter");
    case SerialStudio::StartAndEndDelimiter:
      return QStringLiteral("start-end-delimiter");
    case SerialStudio::NoDelimiters:
      return QStringLiteral("no-delimiters");
    case SerialStudio::StartDelimiterOnly:
      return QStringLiteral("start-delimiter");
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns a human-friendly label for a FrameDetection value.
 */
QString API::EnumLabels::frameDetectionLabel(int value)
{
  switch (static_cast<SerialStudio::FrameDetection>(value)) {
    case SerialStudio::EndDelimiterOnly:
      return QStringLiteral("End delimiter only (split on frameEnd)");
    case SerialStudio::StartAndEndDelimiter:
      return QStringLiteral("Start and end delimiters (split between frameStart and frameEnd)");
    case SerialStudio::NoDelimiters:
      return QStringLiteral("No delimiters (raw byte stream)");
    case SerialStudio::StartDelimiterOnly:
      return QStringLiteral("Start delimiter only");
  }
  return QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// DecoderMethod
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a short slug for a DecoderMethod value.
 */
QString API::EnumLabels::decoderMethodSlug(int value)
{
  switch (static_cast<SerialStudio::DecoderMethod>(value)) {
    case SerialStudio::PlainText:
      return QStringLiteral("plain-text");
    case SerialStudio::Hexadecimal:
      return QStringLiteral("hex");
    case SerialStudio::Base64:
      return QStringLiteral("base64");
    case SerialStudio::Binary:
      return QStringLiteral("binary");
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns a human-friendly label for a DecoderMethod value.
 */
QString API::EnumLabels::decoderMethodLabel(int value)
{
  switch (static_cast<SerialStudio::DecoderMethod>(value)) {
    case SerialStudio::PlainText:
      return QStringLiteral("Plain text (UTF-8)");
    case SerialStudio::Hexadecimal:
      return QStringLiteral("Hexadecimal-encoded ASCII");
    case SerialStudio::Base64:
      return QStringLiteral("Base64-encoded ASCII");
    case SerialStudio::Binary:
      return QStringLiteral("Raw binary bytes");
  }
  return QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// OperationMode
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a short slug for an OperationMode value.
 */
QString API::EnumLabels::operationModeSlug(int value)
{
  switch (static_cast<SerialStudio::OperationMode>(value)) {
    case SerialStudio::ProjectFile:
      return QStringLiteral("project-file");
    case SerialStudio::ConsoleOnly:
      return QStringLiteral("console-only");
    case SerialStudio::QuickPlot:
      return QStringLiteral("quick-plot");
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns a human-friendly label for an OperationMode value.
 */
QString API::EnumLabels::operationModeLabel(int value)
{
  switch (static_cast<SerialStudio::OperationMode>(value)) {
    case SerialStudio::ProjectFile:
      return QStringLiteral("Project File (full dashboard with parser)");
    case SerialStudio::ConsoleOnly:
      return QStringLiteral("Console Only (raw terminal, no dashboard)");
    case SerialStudio::QuickPlot:
      return QStringLiteral("Quick Plot (auto CSV plotting)");
  }
  return QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// ScriptLanguage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a short slug for a ScriptLanguage value (-1 = inherit).
 */
QString API::EnumLabels::scriptLanguageSlug(int value)
{
  if (value == -1)
    return QStringLiteral("inherit");

  switch (static_cast<SerialStudio::ScriptLanguage>(value)) {
    case SerialStudio::JavaScript:
      return QStringLiteral("javascript");
    case SerialStudio::Lua:
      return QStringLiteral("lua");
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns a human-friendly label for a ScriptLanguage value (handles -1 = inherit).
 */
QString API::EnumLabels::scriptLanguageLabel(int value)
{
  if (value == -1)
    return QStringLiteral("Inherit from source");

  switch (static_cast<SerialStudio::ScriptLanguage>(value)) {
    case SerialStudio::JavaScript:
      return QStringLiteral("JavaScript (QJSEngine)");
    case SerialStudio::Lua:
      return QStringLiteral("Lua 5.4");
  }
  return QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// GroupWidget
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a short slug for a GroupWidget value.
 */
QString API::EnumLabels::groupWidgetSlug(int value)
{
  switch (static_cast<SerialStudio::GroupWidget>(value)) {
    case SerialStudio::DataGrid:
      return QStringLiteral("data-grid");
    case SerialStudio::Accelerometer:
      return QStringLiteral("accelerometer");
    case SerialStudio::Gyroscope:
      return QStringLiteral("gyroscope");
    case SerialStudio::GPS:
      return QStringLiteral("gps");
    case SerialStudio::MultiPlot:
      return QStringLiteral("multi-plot");
    case SerialStudio::NoGroupWidget:
      return QStringLiteral("none");
    case SerialStudio::Plot3D:
      return QStringLiteral("plot-3d");
    case SerialStudio::ImageView:
      return QStringLiteral("image-view");
    case SerialStudio::Painter:
      return QStringLiteral("painter");
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns a human-friendly label for a GroupWidget value.
 */
QString API::EnumLabels::groupWidgetLabel(int value)
{
  switch (static_cast<SerialStudio::GroupWidget>(value)) {
    case SerialStudio::DataGrid:
      return QStringLiteral("Data grid (table)");
    case SerialStudio::Accelerometer:
      return QStringLiteral("Accelerometer (3-axis bar)");
    case SerialStudio::Gyroscope:
      return QStringLiteral("Gyroscope (3-axis dial)");
    case SerialStudio::GPS:
      return QStringLiteral("GPS map");
    case SerialStudio::MultiPlot:
      return QStringLiteral("Multi-plot (overlaid line chart)");
    case SerialStudio::NoGroupWidget:
      return QStringLiteral("No group widget");
    case SerialStudio::Plot3D:
      return QStringLiteral("3D plot");
    case SerialStudio::ImageView:
      return QStringLiteral("Image view");
    case SerialStudio::Painter:
      return QStringLiteral("Painter (custom canvas)");
  }
  return QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// DatasetWidget
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a short slug for a DatasetWidget value.
 */
QString API::EnumLabels::datasetWidgetSlug(int value)
{
  switch (static_cast<SerialStudio::DatasetWidget>(value)) {
    case SerialStudio::Bar:
      return QStringLiteral("bar");
    case SerialStudio::Gauge:
      return QStringLiteral("gauge");
    case SerialStudio::Compass:
      return QStringLiteral("compass");
    case SerialStudio::Meter:
      return QStringLiteral("meter");
    case SerialStudio::NoDatasetWidget:
      return QStringLiteral("none");
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns a human-friendly label for a DatasetWidget value.
 */
QString API::EnumLabels::datasetWidgetLabel(int value)
{
  switch (static_cast<SerialStudio::DatasetWidget>(value)) {
    case SerialStudio::Bar:
      return QStringLiteral("Bar (level meter)");
    case SerialStudio::Gauge:
      return QStringLiteral("Gauge (analog dial)");
    case SerialStudio::Compass:
      return QStringLiteral("Compass");
    case SerialStudio::Meter:
      return QStringLiteral("Meter (analog half-arc)");
    case SerialStudio::NoDatasetWidget:
      return QStringLiteral("No dataset widget");
  }
  return QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// DatasetOption
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a comma-separated list of dataset option flags set in value.
 */
QString API::EnumLabels::datasetOptionsLabel(int value)
{
  QStringList parts;
  if (value & SerialStudio::DatasetPlot)
    parts.append(QStringLiteral("plot"));

  if (value & SerialStudio::DatasetFFT)
    parts.append(QStringLiteral("FFT"));

  if (value & SerialStudio::DatasetBar)
    parts.append(QStringLiteral("bar"));

  if (value & SerialStudio::DatasetGauge)
    parts.append(QStringLiteral("gauge"));

  if (value & SerialStudio::DatasetCompass)
    parts.append(QStringLiteral("compass"));

  if (value & SerialStudio::DatasetLED)
    parts.append(QStringLiteral("LED"));

  if (value & SerialStudio::DatasetWaterfall)
    parts.append(QStringLiteral("waterfall"));

  if (value & SerialStudio::DatasetMeter)
    parts.append(QStringLiteral("meter"));

  if (parts.isEmpty())
    return QStringLiteral("generic");

  return parts.join(QStringLiteral(", "));
}

//--------------------------------------------------------------------------------------------------
// DashboardWidget slug -- bidirectional
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a stable string slug for a DashboardWidget enum value.
 */
QString API::EnumLabels::dashboardWidgetSlug(int value)
{
  switch (static_cast<SerialStudio::DashboardWidget>(value)) {
    case SerialStudio::DashboardTerminal:
      return QStringLiteral("terminal");
    case SerialStudio::DashboardDataGrid:
      return QStringLiteral("datagrid");
    case SerialStudio::DashboardMultiPlot:
      return QStringLiteral("multiplot");
    case SerialStudio::DashboardAccelerometer:
      return QStringLiteral("accelerometer");
    case SerialStudio::DashboardGyroscope:
      return QStringLiteral("gyroscope");
    case SerialStudio::DashboardGPS:
      return QStringLiteral("gps");
    case SerialStudio::DashboardPlot3D:
      return QStringLiteral("plot3d");
    case SerialStudio::DashboardFFT:
      return QStringLiteral("fft");
    case SerialStudio::DashboardLED:
      return QStringLiteral("led");
    case SerialStudio::DashboardPlot:
      return QStringLiteral("plot");
    case SerialStudio::DashboardBar:
      return QStringLiteral("bar");
    case SerialStudio::DashboardGauge:
      return QStringLiteral("gauge");
    case SerialStudio::DashboardCompass:
      return QStringLiteral("compass");
    case SerialStudio::DashboardMeter:
      return QStringLiteral("meter");
    case SerialStudio::DashboardClock:
      return QStringLiteral("clock");
    case SerialStudio::DashboardStopwatch:
      return QStringLiteral("stopwatch");
    case SerialStudio::DashboardNoWidget:
      return QStringLiteral("none");
#ifdef BUILD_COMMERCIAL
    case SerialStudio::DashboardImageView:
      return QStringLiteral("imageview");
    case SerialStudio::DashboardOutputPanel:
      return QStringLiteral("output-panel");
    case SerialStudio::DashboardNotificationLog:
      return QStringLiteral("notification-log");
    case SerialStudio::DashboardWaterfall:
      return QStringLiteral("waterfall");
    case SerialStudio::DashboardPainter:
      return QStringLiteral("painter");
#endif
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Resolves a DashboardWidget slug back to its enum integer; returns -1 on miss.
 */
int API::EnumLabels::dashboardWidgetFromSlug(const QString& slug)
{
  const auto s = slug.trimmed().toLower();
  if (s == QLatin1String("terminal"))
    return SerialStudio::DashboardTerminal;

  if (s == QLatin1String("datagrid") || s == QLatin1String("data-grid"))
    return SerialStudio::DashboardDataGrid;

  if (s == QLatin1String("multiplot") || s == QLatin1String("multi-plot"))
    return SerialStudio::DashboardMultiPlot;

  if (s == QLatin1String("accelerometer"))
    return SerialStudio::DashboardAccelerometer;

  if (s == QLatin1String("gyroscope"))
    return SerialStudio::DashboardGyroscope;

  if (s == QLatin1String("gps"))
    return SerialStudio::DashboardGPS;

  if (s == QLatin1String("plot3d") || s == QLatin1String("plot-3d"))
    return SerialStudio::DashboardPlot3D;

  if (s == QLatin1String("fft"))
    return SerialStudio::DashboardFFT;

  if (s == QLatin1String("led"))
    return SerialStudio::DashboardLED;

  if (s == QLatin1String("plot"))
    return SerialStudio::DashboardPlot;

  if (s == QLatin1String("bar"))
    return SerialStudio::DashboardBar;

  if (s == QLatin1String("gauge"))
    return SerialStudio::DashboardGauge;

  if (s == QLatin1String("compass"))
    return SerialStudio::DashboardCompass;

  if (s == QLatin1String("meter"))
    return SerialStudio::DashboardMeter;

  if (s == QLatin1String("clock"))
    return SerialStudio::DashboardClock;

  if (s == QLatin1String("stopwatch"))
    return SerialStudio::DashboardStopwatch;

  if (s == QLatin1String("none"))
    return SerialStudio::DashboardNoWidget;
#ifdef BUILD_COMMERCIAL
  if (s == QLatin1String("imageview") || s == QLatin1String("image-view"))
    return SerialStudio::DashboardImageView;

  if (s == QLatin1String("output-panel") || s == QLatin1String("outputpanel"))
    return SerialStudio::DashboardOutputPanel;

  if (s == QLatin1String("notification-log") || s == QLatin1String("notificationlog"))
    return SerialStudio::DashboardNotificationLog;

  if (s == QLatin1String("waterfall"))
    return SerialStudio::DashboardWaterfall;

  if (s == QLatin1String("painter"))
    return SerialStudio::DashboardPainter;
#endif
  return -1;
}

//--------------------------------------------------------------------------------------------------
// DatasetOption slug -- bidirectional
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a stable slug for a single DatasetOption bit.
 */
QString API::EnumLabels::datasetOptionSlug(int singleBitValue)
{
  switch (singleBitValue) {
    case SerialStudio::DatasetPlot:
      return QStringLiteral("plot");
    case SerialStudio::DatasetFFT:
      return QStringLiteral("fft");
    case SerialStudio::DatasetBar:
      return QStringLiteral("bar");
    case SerialStudio::DatasetGauge:
      return QStringLiteral("gauge");
    case SerialStudio::DatasetCompass:
      return QStringLiteral("compass");
    case SerialStudio::DatasetLED:
      return QStringLiteral("led");
    case SerialStudio::DatasetWaterfall:
      return QStringLiteral("waterfall");
  }
  return QString();
}

/**
 * @brief Resolves a DatasetOption slug to its bit value; returns 0 on miss.
 */
int API::EnumLabels::datasetOptionFromSlug(const QString& slug)
{
  const auto s = slug.trimmed().toLower();
  if (s == QLatin1String("plot"))
    return SerialStudio::DatasetPlot;

  if (s == QLatin1String("fft"))
    return SerialStudio::DatasetFFT;

  if (s == QLatin1String("bar"))
    return SerialStudio::DatasetBar;

  if (s == QLatin1String("gauge"))
    return SerialStudio::DatasetGauge;

  if (s == QLatin1String("compass"))
    return SerialStudio::DatasetCompass;

  if (s == QLatin1String("led"))
    return SerialStudio::DatasetLED;

  if (s == QLatin1String("waterfall"))
    return SerialStudio::DatasetWaterfall;

  if (s == QLatin1String("meter"))
    return SerialStudio::DatasetMeter;

  return 0;
}

/**
 * @brief Splits a DatasetOption bitflag into individual slug strings (in canonical order).
 */
QStringList API::EnumLabels::datasetOptionsBitsToSlugs(int value)
{
  QStringList out;
  if (value & SerialStudio::DatasetPlot)
    out.append(QStringLiteral("plot"));

  if (value & SerialStudio::DatasetFFT)
    out.append(QStringLiteral("fft"));

  if (value & SerialStudio::DatasetBar)
    out.append(QStringLiteral("bar"));

  if (value & SerialStudio::DatasetGauge)
    out.append(QStringLiteral("gauge"));

  if (value & SerialStudio::DatasetCompass)
    out.append(QStringLiteral("compass"));

  if (value & SerialStudio::DatasetLED)
    out.append(QStringLiteral("led"));

  if (value & SerialStudio::DatasetWaterfall)
    out.append(QStringLiteral("waterfall"));

  if (value & SerialStudio::DatasetMeter)
    out.append(QStringLiteral("meter"));

  return out;
}

/**
 * @brief Combines an array of DatasetOption slugs into a single bitflag value.
 */
int API::EnumLabels::datasetOptionsSlugsToBits(const QStringList& slugs)
{
  int bits = 0;
  for (const auto& slug : slugs)
    bits |= datasetOptionFromSlug(slug);

  return bits;
}
