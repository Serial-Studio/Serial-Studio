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
class SerialStudio : public QObject {
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
  enum DecoderMethod {
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
   * @brief Specifies the scripting language used for the frame parser function.
   */
  enum ScriptLanguage {
    JavaScript = 0, /**< JavaScript via Qt's QJSEngine. */
    Lua        = 1, /**< Lua 5.4 via embedded interpreter. */
    /* IMPORTANT: When adding other modes, please don't modify the order of the
     *            enums to ensure backward compatiblity with previous project
     *            files!! */
  };
  Q_ENUM(ScriptLanguage)

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
   * @enum TextEncoding
   * @brief Character encoding used when converting between `QString` and raw
   *        bytes for terminal I/O, action payloads, and output-widget strings.
   *
   * The first few entries are handled natively via `QStringConverter`.  The
   * entries after `System` are multi-byte East-Asian encodings that require
   * `QTextCodec` from `Qt6::Core5Compat` (already linked by QuaZip).
   *
   * Do NOT reorder or remove entries — project files serialize the canonical
   * name string (see `textEncodingName()`), but settings may still read ints.
   */
  enum TextEncoding {
    EncUtf8 = 0, /**< UTF-8 (default). */
    EncUtf16LE,  /**< UTF-16 little-endian. */
    EncUtf16BE,  /**< UTF-16 big-endian. */
    EncLatin1,   /**< ISO-8859-1 / Latin-1. */
    EncSystem,   /**< System locale encoding. */
    EncGbk,      /**< Simplified Chinese — GBK. */
    EncGb18030,  /**< Simplified Chinese — GB18030. */
    EncBig5,     /**< Traditional Chinese — Big5. */
    EncShiftJis, /**< Japanese — Shift_JIS. */
    EncEucJp,    /**< Japanese — EUC-JP. */
    EncEucKr,    /**< Korean — EUC-KR. */
  };
  Q_ENUM(TextEncoding)

  /**
   * @enum OperationMode
   * @brief Specifies the method used to construct a dashboard.
   *
   * The `OperationMode` enum defines the strategies for building dashboards
   * based on the data source or project configuration. Each mode represents
   * a different approach to interpreting and visualizing the incoming data.
   */
  enum OperationMode {
    ProjectFile, /**< Builds the dashboard using a predefined project file. */
    ConsoleOnly, /**< No parsing: raw bytes flow only to the console. */
    QuickPlot,   /**< Quick and simple data plotting mode. */
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
  enum class BusType {
    UART,        /**< Serial port communication. */
    Network,     /**< Network socket communication. */
    BluetoothLE, /**< Bluetooth Low Energy communication. */
#ifdef BUILD_COMMERCIAL
    Audio,     /**< Audio input device */
    ModBus,    /**< MODBUS communication */
    CanBus,    /**< CANBUS communication */
    RawUsb,    /**< Raw USB bulk/control transfers */
    HidDevice, /**< HID device via hidapi */
    Process,   /**< Child process stdout/stdin or named pipe */
#endif
  };
  Q_ENUM(BusType)

  /**
   * @brief Enum representing the different widget types available for groups.
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
  };
  Q_ENUM(GroupWidget)

  /**
   * @brief Enum representing the different widget types available for datasets.
   */
  enum DatasetWidget {
    Bar,
    Gauge,
    Compass,
    NoDatasetWidget
  };
  Q_ENUM(DatasetWidget)

  /**
   * @brief Distinguishes input (visualization) groups from output (control)
   *        groups.  Mirrors DataModel::GroupType for QML access.
   */
  enum GroupType {
    GroupInput  = 0,
    GroupOutput = 1,
  };
  Q_ENUM(GroupType)

  /**
   * @brief Enum representing interactive output widget types for bidirectional
   *        communication.
   */
  enum OutputWidgetType {
    OutputButton,
    OutputSlider,
    OutputToggle,
    OutputTextField,
    OutputKnob,
    OutputRampGenerator,
  };
  Q_ENUM(OutputWidgetType)

  /**
   * @brief Enum representing the different widget types available for the
   *        dashboard.
   *
   * @warning Window order in dashboard depends on this!
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
    DashboardNoWidget,
#ifdef BUILD_COMMERCIAL
    DashboardImageView,
    DashboardOutputPanel,
#endif
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
  Q_INVOKABLE [[nodiscard]] static bool activated();
  Q_INVOKABLE [[nodiscard]] static bool commercialCfg(const QVector<DataModel::Group>& g);
  Q_INVOKABLE [[nodiscard]] static bool commercialCfg(const std::vector<DataModel::Group>& g);

  //
  // Dashboard logic
  //
  // clang-format off
  Q_INVOKABLE [[nodiscard]] static bool isGroupWidget(const SerialStudio::DashboardWidget widget);
  Q_INVOKABLE [[nodiscard]] static bool isDatasetWidget(const SerialStudio::DashboardWidget widget);
  Q_INVOKABLE [[nodiscard]] static QString dashboardWidgetTitle(const SerialStudio::DashboardWidget w);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::DashboardWidget getDashboardWidget(const DataModel::Group& group);
  Q_INVOKABLE [[nodiscard]] static QList<SerialStudio::DashboardWidget> getDashboardWidgets(const DataModel::Dataset& dataset);
  Q_INVOKABLE [[nodiscard]] static QString dashboardWidgetIcon(const SerialStudio::DashboardWidget w, const bool large = false);
  // clang-format on

  //
  // Filters used by workspace-ref and auto-workspace builders. Keeping these
  // centralized so all walkers agree on which widgets can be placed on a
  // workspace (must mirror Dashboard::buildWidgetGroups exactly).
  //
  // clang-format off
  Q_INVOKABLE [[nodiscard]] static bool groupEligibleForWorkspace(const DataModel::Group& g);
  Q_INVOKABLE [[nodiscard]] static bool groupWidgetEligibleForWorkspace(SerialStudio::DashboardWidget w);
  Q_INVOKABLE [[nodiscard]] static bool datasetWidgetEligibleForWorkspace(SerialStudio::DashboardWidget w);
  // clang-format on

  //
  // Parsing & project model logic
  //
  // clang-format off
  Q_INVOKABLE [[nodiscard]] static QString groupWidgetId(const SerialStudio::GroupWidget widget);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::GroupWidget groupWidgetFromId(const QString& id);
  Q_INVOKABLE [[nodiscard]] static QString datasetWidgetId(const SerialStudio::DatasetWidget widget);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::DatasetWidget datasetWidgetFromId(const QString& id);
  // clang-format on

  //
  // Utility functions
  //
  Q_INVOKABLE [[nodiscard]] static bool isAnyPlayerOpen();
  Q_INVOKABLE [[nodiscard]] static QColor getDatasetColor(const int index);
  Q_INVOKABLE [[nodiscard]] static QColor getDeviceColor(const int sourceId);
  Q_INVOKABLE [[nodiscard]] static QColor getDeviceTopColor(const int sourceId);
  Q_INVOKABLE [[nodiscard]] static QColor getDeviceBottomColor(const int sourceId);

  //
  // String processing
  //
  Q_INVOKABLE [[nodiscard]] static QString hexToString(const QString& hex);
  Q_INVOKABLE [[nodiscard]] static QString stringToHex(const QString& str);
  Q_INVOKABLE [[nodiscard]] static QByteArray hexToBytes(const QString& data);
  Q_INVOKABLE [[nodiscard]] static QString resolveEscapeSequences(const QString& str);
  Q_INVOKABLE [[nodiscard]] static QString escapeControlCharacters(const QString& str);

  //
  // Text encoding helpers
  //
  // clang-format off
  Q_INVOKABLE [[nodiscard]] static QStringList textEncodings();
  Q_INVOKABLE [[nodiscard]] static QString textEncodingName(SerialStudio::TextEncoding enc);
  Q_INVOKABLE [[nodiscard]] static SerialStudio::TextEncoding textEncodingFromName(const QString& name);
  Q_INVOKABLE [[nodiscard]] static QByteArray encodeText(const QString& text, SerialStudio::TextEncoding enc);
  Q_INVOKABLE [[nodiscard]] static QString decodeText(QByteArrayView bytes, SerialStudio::TextEncoding enc);
  // clang-format on
};
