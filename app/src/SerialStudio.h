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

#include "JSON/Frame.h"

/**
 * @class SerialStudio
 * @brief A central utility class for managing data visualization and decoding
 * logic.
 *
 * The `SerialStudio` class provides a set of tools and enums for working with
 * continuous data streams, decoding strategies, and visualization elements. It
 * serves as a foundation for configuring, parsing, and visualizing incoming
 * data from various sources such as serial ports, network sockets, and
 * Bluetooth LE devices.
 *
 * Key Features:
 * - **Decoding Methods**: Support for PlainText, Hexadecimal, and Base64
 *   decoding.
 * - **Frame Detection**: Configurable methods for detecting data frames in
 *   streams, including end-delimiter-only and start-and-end-delimiter
 * strategies.
 * - **Operation Modes**: Different approaches for building dashboards, such as
 *   reading from project files, JSON devices, or quick plots.
 * - **Visualization Widgets**: Enumerations for different widget types
 *   available for groups, datasets, and dashboards.
 * - **Utility Functions**: Static functions for managing widgets, colors, and
 *   icon/title associations in dashboards.
 *
 * @details
 * This class is highly modular, enabling developers to work with a wide variety
 * of visualization tools and data handling methods. It supports the
 * customization of dashboard components and datasets to suit specific
 * application needs.
 *
 * @enums
 * - **DecoderMethod**: Defines methods for decoding data streams.
 * - **FrameDetection**: Configures strategies for detecting frames in data
 *                       streams.
 * - **OperationMode**: Specifies methods for building dashboards.
 * - **BusType**: Enumerates the available data sources.
 * - **GroupWidget**: Lists visualization widget types for groups.
 * - **DatasetWidget**: Lists visualization widget types for datasets.
 * - **DashboardWidget**: Enumerates visualization widget types for dashboards.
 * - **DatasetOption**: Bit-flag options for dataset configurations.
 */
class SerialStudio : public QObject
{
  Q_OBJECT

public:
  /**
   * @enum DecoderMethod
   * @brief Specifies the available methods for decoding data from a continuous
   * stream.
   *
   * This enum defines different decoding strategies that can be applied to
   * interpret incoming data. Each value represents a specific decoding format.
   */
  enum DecoderMethod
  {
    PlainText,   /**< Standard decoding, interprets data as plain text. */
    Hexadecimal, /**< Decodes data assuming a hexadecimal-encoded format. */
    Base64,      /**< Decodes data assuming a Base64-encoded format. */
    Binary,      /**< Decodes raw data directly. */
    /* IMPORTANT: When adding other modes, please don't modify the order of the
     *            enums to ensure backward compatiblity with previous project
     *            files!! */
  };
  Q_ENUM(DecoderMethod)

  /**
   * @brief Specifies the method used to detect data frames within a continuous
   *        stream.
   *
   * This enum is used to define the strategy for identifying the start and end
   * of a data frame in a continuous stream. Each value represents a specific
   * detection method.
   */
  // clang-format off
  enum FrameDetection
  {
    EndDelimiterOnly     = 0x00, /**< Detects frames based only on an end delimiter. */
    StartAndEndDelimiter = 0x01, /**< Detects frames based on both start and end delimiters. */
    NoDelimiters         = 0x02, /**< Disables frame detection and processes incoming data directly */
    StartDelimiterOnly   = 0x03  /**< Detects frames with only a header */
    /* IMPORTANT: When adding other modes, please don't modify the order of the
     *            enums to ensure backward compatiblity with previous project
     *            files!! */
  };
  Q_ENUM(FrameDetection)
  // clang-format on

  /**
   * @enum OperationMode
   * @brief Specifies the method used to construct a dashboard.
   *
   * The `OperationMode` enum defines the strategies for building dashboards
   * based on the data source or project configuration. Each mode represents
   * a different approach to interpreting and visualizing the incoming data.
   */
  enum OperationMode
  {
    ProjectFile, /**< Builds the dashboard using a predefined project file. */
    DeviceSendsJSON, /**< Builds the dashboard from device-sent JSON. */
    QuickPlot,       /**< Quick and simple data plotting mode. */
    /* IMPORTANT: When adding other modes, please don't modify the order of the
     *            enums to ensure backward compatiblity with previous project
     *            files!! */
  };
  Q_ENUM(OperationMode)

  /**
   * @enum BusType
   * @brief Enumerates the available data sources for communication.
   *
   * The `BusType` enum defines the supported communication protocols or data
   * sources used for receiving data streams. Each bus type corresponds to a
   * specific hardware or network interface.
   */
  enum class BusType
  {
    UART,        /**< Serial port communication. */
    Network,     /**< Network socket communication. */
    BluetoothLE, /**< Bluetooth Low Energy communication. */
#ifdef BUILD_COMMERCIAL
    Audio,  /**< Audio input device */
    ModBus, /**< MODBUS communication */
    CanBus, /**< CANBUS communication */
#endif
  };
  Q_ENUM(BusType)

  /**
   * @brief Enum representing the different widget types available for groups.
   */
  enum GroupWidget
  {
    DataGrid,
    Accelerometer,
    Gyroscope,
    GPS,
    MultiPlot,
    NoGroupWidget,
    Plot3D,
  };
  Q_ENUM(GroupWidget)

  /**
   * @brief Enum representing the different widget types available for datasets.
   */
  enum DatasetWidget
  {
    Bar,
    Gauge,
    Compass,
    NoDatasetWidget
  };
  Q_ENUM(DatasetWidget)

  /**
   * @brief Enum representing the different widget types available for the
   *        dashboard.
   *
   * @warning Window order in dashboard depends on this!
   */
  enum DashboardWidget
  {
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
    DashboardNoWidget,
  };
  Q_ENUM(DashboardWidget)

  /**
   * @brief Enum representing the options available for datasets.
   *
   * These options are represented as bit flags, allowing for combinations of
   * multiple options.
   */
  // clang-format off
  enum DatasetOption
  {
    DatasetGeneric = 0b00000000,
    DatasetPlot    = 0b00000001,
    DatasetFFT     = 0b00000010,
    DatasetBar     = 0b00000100,
    DatasetGauge   = 0b00001000,
    DatasetCompass = 0b00010000,
    DatasetLED     = 0b00100000,
  };
  Q_ENUM(DatasetOption)
  // clang-format on

  /**
   * @typedef WidgetMap
   * @brief Defines the data structure used to map dashboard widgets/windows.
   *
   * This map uses the window number (UI order) as the key, and its value is a
   * QPair containing:
   * - The widget type (SerialStudio::DashboardWidget)
   * - The index of the widget relative to its type (0-based)
   *
   * For example, if there are two FFT plot widgets:
   * - Both will have unique keys (window numbers)
   * - Their values will have the same widget type (DashboardFFT) and indices 0
   *   and 1 to indicate their order among FFT widgets.
   *
   * This structure is primarily used to manage and categorize widgets across
   * the dashboard for UI rendering, tracking, and model building.
   *
   * Example type signature:
   * QMap<int, QPair<SerialStudio::DashboardWidget, int>>
   * - int: Window number (unique per widget)
   * - DashboardWidget: Enum/type for widget kind (e.g., FFT, Line Chart)
   * - int: Widget index relative to its type
   */
  typedef QMap<int, QPair<SerialStudio::DashboardWidget, int>> WidgetMap;

  //
  // Commercial-related functions
  //
  [[nodiscard]] static bool activated();
  [[nodiscard]] static bool commercialCfg(const QVector<JSON::Group> &g);
  [[nodiscard]] static bool commercialCfg(const std::vector<JSON::Group> &g);

  //
  // Dashboard logic
  //
  // clang-format off
  [[nodiscard]] static bool isGroupWidget(const DashboardWidget widget);
  [[nodiscard]] static bool isDatasetWidget(const DashboardWidget widget);
  [[nodiscard]] static QString dashboardWidgetTitle(const DashboardWidget w);
  [[nodiscard]] static DashboardWidget getDashboardWidget(const JSON::Group& group);
  [[nodiscard]] static QList<DashboardWidget> getDashboardWidgets(const JSON::Dataset& dataset);
  Q_INVOKABLE static QString dashboardWidgetIcon(const DashboardWidget w, const bool large = false);
  // clang-format on

  //
  // Parsing & project model logic
  //
  [[nodiscard]] static QString groupWidgetId(const GroupWidget widget);
  [[nodiscard]] static GroupWidget groupWidgetFromId(const QString &id);
  [[nodiscard]] static QString datasetWidgetId(const DatasetWidget widget);
  [[nodiscard]] static DatasetWidget datasetWidgetFromId(const QString &id);

  //
  // Utility functions
  //
  [[nodiscard]] static QColor getDatasetColor(const int index);

  //
  // String processing
  //
  [[nodiscard]] static QString hexToString(const QString &hex);
  [[nodiscard]] static QString stringToHex(const QString &str);
  [[nodiscard]] static QByteArray hexToBytes(const QString &data);
  [[nodiscard]] static QString resolveEscapeSequences(const QString &str);
  [[nodiscard]] static QString escapeControlCharacters(const QString &str);
};
