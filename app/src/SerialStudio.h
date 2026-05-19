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

#include "DataModel/Frame.h"

/**
 * @brief Central utility class with the shared enums and helpers used by the
 *        data pipeline, visualization widgets, and project model.
 */
class SerialStudio : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Decoding strategy for a continuous data stream.
   */
  enum DecoderMethod {
    PlainText,
    Hexadecimal,
    Base64,
    Binary,
  };
  Q_ENUM(DecoderMethod)

  /**
   * @brief Scripting language used for the frame parser function.
   */
  enum ScriptLanguage {
    JavaScript = 0,
    Lua        = 1,
  };
  Q_ENUM(ScriptLanguage)

  /**
   * @brief Strategy for detecting data frames inside a continuous stream.
   */
  // clang-format off
  enum FrameDetection
  {
    EndDelimiterOnly     = 0x00,
    StartAndEndDelimiter = 0x01,
    NoDelimiters         = 0x02,
    StartDelimiterOnly   = 0x03
  };
  Q_ENUM(FrameDetection)
  // clang-format on

  /**
   * @brief Character encoding used for QString/byte conversions.
   */
  enum TextEncoding {
    EncUtf8 = 0,
    EncUtf16LE,
    EncUtf16BE,
    EncLatin1,
    EncSystem,
    EncGbk,
    EncGb18030,
    EncBig5,
    EncShiftJis,
    EncEucJp,
    EncEucKr,
  };
  Q_ENUM(TextEncoding)

  /**
   * @brief Dashboard construction strategy.
   */
  enum OperationMode {
    ProjectFile,
    ConsoleOnly,
    QuickPlot,
  };
  Q_ENUM(OperationMode)

  /**
   * @brief Interpolation modes for line-based plot widgets.
   */
  enum InterpolationMode {
    InterpolationNone,
    InterpolationLinear,
    InterpolationZoh,
    InterpolationStem,
  };
  Q_ENUM(InterpolationMode)

  /**
   * @brief Available data-source bus types.
   */
  enum class BusType {
    UART,
    Network,
    BluetoothLE,
#ifdef BUILD_COMMERCIAL
    Audio,
    ModBus,
    CanBus,
    RawUsb,
    HidDevice,
    Process,
    Mqtt,
#endif
  };
  Q_ENUM(BusType)

  /**
   * @brief Visualization widget types available for groups.
   */
  enum GroupWidget {
    DataGrid,
    Accelerometer,
    Gyroscope,
    GPS,
    MultiPlot,
    NoGroupWidget,
    Plot3D,
    ImageView,
    Painter,
  };
  Q_ENUM(GroupWidget)

  /**
   * @brief Visualization widget types available for datasets.
   */
  enum DatasetWidget {
    Bar,
    Gauge,
    Compass,
    Meter,
    Thermometer,
    NoDatasetWidget
  };
  Q_ENUM(DatasetWidget)

  /**
   * @brief Distinguishes input (visualization) groups from output (control) groups.
   */
  enum GroupType {
    GroupInput  = 0,
    GroupOutput = 1,
  };
  Q_ENUM(GroupType)

  /**
   * @brief Interactive output widget types for bidirectional communication.
   */
  enum OutputWidgetType {
    OutputButton,
    OutputSlider,
    OutputToggle,
    OutputTextField,
    OutputKnob,
  };
  Q_ENUM(OutputWidgetType)

  /**
   * @brief Visualization widget types available on the dashboard.
   */
  enum DashboardWidget {
    DashboardTerminal,
    DashboardDataGrid,
    DashboardMultiPlot,
    DashboardAccelerometer,
    DashboardGyroscope,
    DashboardGPS,
    DashboardPlot3D,
    DashboardFFT,
    DashboardLED,
    DashboardPlot,
    DashboardBar,
    DashboardGauge,
    DashboardCompass,
    DashboardMeter,
    DashboardThermometer,
    DashboardNoWidget,
#ifdef BUILD_COMMERCIAL
    DashboardImageView,
    DashboardOutputPanel,
    DashboardNotificationLog,
    DashboardWaterfall,
    DashboardPainter,
#endif
  };
  Q_ENUM(DashboardWidget)

  /**
   * @brief Bit-flag options for dataset configurations.
   */
  // clang-format off
  enum DatasetOption
  {
    DatasetGeneric     = 0b000000000,
    DatasetPlot        = 0b000000001,
    DatasetFFT         = 0b000000010,
    DatasetBar         = 0b000000100,
    DatasetGauge       = 0b000001000,
    DatasetCompass     = 0b000010000,
    DatasetLED         = 0b000100000,
    DatasetWaterfall   = 0b001000000,
    DatasetMeter       = 0b010000000,
    DatasetThermometer = 0b100000000,
  };
  Q_ENUM(DatasetOption)
  // clang-format on

  typedef QMap<int, QPair<SerialStudio::DashboardWidget, int>> WidgetMap;

  Q_INVOKABLE [[nodiscard]] static bool activated();
  Q_INVOKABLE [[nodiscard]] static bool proWidgetsEnabled();
  Q_INVOKABLE [[nodiscard]] static bool commercialCfg(const QVector<DataModel::Group>& g);
  Q_INVOKABLE [[nodiscard]] static bool commercialCfg(const std::vector<DataModel::Group>& g);

  // clang-format off
  Q_INVOKABLE [[nodiscard]] static bool isGroupWidget(const SerialStudio::DashboardWidget widget);
  Q_INVOKABLE [[nodiscard]] static bool isDatasetWidget(const SerialStudio::DashboardWidget widget);
  Q_INVOKABLE [[nodiscard]] static QString dashboardWidgetTitle(const SerialStudio::DashboardWidget w);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::DashboardWidget getDashboardWidget(const DataModel::Group& group);
  Q_INVOKABLE [[nodiscard]] static QList<SerialStudio::DashboardWidget> getDashboardWidgets(const DataModel::Dataset& dataset);
  Q_INVOKABLE [[nodiscard]] static QString dashboardWidgetIcon(const SerialStudio::DashboardWidget w, const bool large = false);
  // clang-format on

  // clang-format off
  Q_INVOKABLE [[nodiscard]] static bool groupEligibleForWorkspace(const DataModel::Group& g);
  Q_INVOKABLE [[nodiscard]] static bool groupWidgetEligibleForWorkspace(SerialStudio::DashboardWidget w);
  Q_INVOKABLE [[nodiscard]] static bool datasetWidgetEligibleForWorkspace(SerialStudio::DashboardWidget w);
  // clang-format on

  // clang-format off
  Q_INVOKABLE [[nodiscard]] static QString groupWidgetId(const SerialStudio::GroupWidget widget);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::GroupWidget groupWidgetFromId(const QString& id);
  Q_INVOKABLE [[nodiscard]] static QString datasetWidgetId(const SerialStudio::DatasetWidget widget);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::DatasetWidget datasetWidgetFromId(const QString& id);
  // clang-format on

  Q_INVOKABLE [[nodiscard]] static bool isAnyPlayerOpen();
  Q_INVOKABLE [[nodiscard]] static bool isFinalValuePlayerOpen();
  Q_INVOKABLE [[nodiscard]] static QColor getDatasetColor(const int index);
  Q_INVOKABLE [[nodiscard]] static QColor getDeviceColor(const int sourceId);
  Q_INVOKABLE [[nodiscard]] static QColor getDeviceTopColor(const int sourceId);
  Q_INVOKABLE [[nodiscard]] static QColor getDeviceBottomColor(const int sourceId);

  Q_INVOKABLE [[nodiscard]] static QString hexToString(const QString& hex);
  Q_INVOKABLE [[nodiscard]] static QString stringToHex(const QString& str);
  Q_INVOKABLE [[nodiscard]] static QByteArray hexToBytes(const QString& data);
  Q_INVOKABLE [[nodiscard]] static QString resolveEscapeSequences(const QString& str);
  Q_INVOKABLE [[nodiscard]] static QString escapeControlCharacters(const QString& str);

  Q_INVOKABLE [[nodiscard]] static QString normalizeIconPath(const QString& path);

  // clang-format off
  Q_INVOKABLE [[nodiscard]] static QStringList textEncodings();
  Q_INVOKABLE [[nodiscard]] static QString textEncodingName(SerialStudio::TextEncoding enc);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::TextEncoding textEncodingFromName(const QString& name);
  Q_INVOKABLE [[nodiscard]] static QByteArray encodeText(const QString& text, SerialStudio::TextEncoding enc);
  Q_INVOKABLE [[nodiscard]] static QString decodeText(QByteArrayView bytes, SerialStudio::TextEncoding enc);
  // clang-format on
};
