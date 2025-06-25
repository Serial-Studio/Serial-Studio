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

#include <QObject>
#include <QVector>

#include "JSON/Group.h"
#include "JSON/Dataset.h"

/**
 * @typedef PlotDataX
 * @brief Represents the unique X-axis data points for a plot.
 */
typedef std::vector<double> PlotDataX;

/**
 * @typedef PlotDataY
 * @brief Represents the Y-axis data points for a single curve.
 */
typedef std::vector<double> PlotDataY;

#ifdef BUILD_COMMERCIAL
/**
 * @typedef PlotData3D
 * @brief Represents a list of 3D points.
 */
typedef std::vector<QVector3D> PlotData3D;
#endif

/**
 * @typedef MultiPlotDataY
 * @brief Represents Y-axis data for multiple curves in a multiplot.
 *
 * A vector of `PlotDataY` structures, where each element represents
 * the Y-axis data for one curve in a multiplot widget. This allows
 * managing multiple curves independently.
 */
typedef std::vector<PlotDataY> MultiPlotDataY;

/**
 * @typedef LineSeries
 * @brief Represents a paired series of X-axis and Y-axis data for a plot.
 *
 * The `LineSeries` type is defined as a `QPair` containing:
 * - A pointer to `PlotDataX`, which holds the unique X-axis values.
 * - A pointer to `PlotDataY`, which holds the Y-axis values.
 *
 * This type simplifies data processing by tightly coupling the related X and Y
 * data for a plot, ensuring that they are always accessed and managed together.
 */
typedef struct
{
  PlotDataX *x; ///< X-axis data (e.g., time or samples)
  PlotDataY *y; ///< Y-axis data (e.g., sensor readings)
} LineSeries;

/**
 * @struct MultiLineSeries
 * @brief Represents a set of line plots sharing a common X-axis.
 *
 * This structure is used for rendering multi-curve plots, such as when multiple
 * sensors or variables are plotted against the same time base or domain.
 *
 * - `x`: Pointer to the shared X-axis data (e.g., time).
 * - `y`: A list of Y-axis data vectors, where each entry represents one curve.
 *
 * All Y-series are expected to align with the length and indexing of the
 * shared X-axis.
 */
typedef struct
{
  PlotDataX *x;             ///< Shared X-axis data (e.g., time or index)
  std::vector<PlotDataY> y; ///< Y-axis data for each individual curve
} MultiLineSeries;

/**
 * @struct GpsSeries
 * @brief Represents a time-ordered sequence of GPS position data.
 *
 * This structure stores a trajectory as three parallel vectors:
 * - `latitudes`: The latitude values in degrees.
 * - `longitudes`: The longitude values in degrees.
 * - `altitudes`: The altitude values in meters.
 *
 * Each index across the three vectors corresponds to a single
 * recorded GPS point in time. This format is optimized for
 * visualization, analysis, and storage of path-based geospatial data.
 */
typedef struct
{
  std::vector<double> latitudes;  ///< Latitude values (degrees)
  std::vector<double> longitudes; ///< Longitude values (degrees)
  std::vector<double> altitudes;  ///< Altitude values (meters)
} GpsSeries;

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
  [[nodiscard]] static bool commercialCfg(const QVector<JSON::Group> &groups);

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
  [[nodiscard]] static QString getDatasetColor(const int index);

  //
  // String processing
  //
  [[nodiscard]] static QString hexToString(const QString &hex);
  [[nodiscard]] static QString stringToHex(const QString &str);
  [[nodiscard]] static QByteArray hexToBytes(const QString &data);
  [[nodiscard]] static QString resolveEscapeSequences(const QString &str);
  [[nodiscard]] static QString escapeControlCharacters(const QString &str);
};
