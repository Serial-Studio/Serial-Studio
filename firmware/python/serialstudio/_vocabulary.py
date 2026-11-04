# Serial Studio device library
# https://serial-studio.com/
#
# Copyright (C) 2020-2026 Alex Spataru
#
# SPDX-License-Identifier: MIT
#
# AUTO-GENERATED from app/rcc/properties/dataset.json and declaration.json;
# never edit by hand. Regenerate with: python3 scripts/generate-property-registry.py

"""Wire vocabulary of the Serial Studio device declaration."""

__version__ = "1.0.0"

PREFIX = "#ss"
REQUEST = "#ss?"
COMMAND = "#ss!"
BINARY_TEMPLATE = "packed_struct"
WIRE_VERSION = 1

MAX_LINE_BYTES = 16384
MAX_STRING_BYTES = 64
MAX_GROUPS = 64
MAX_DATASETS = 512
MAX_CONTROLS = 64

KEY_TITLE = "title"
KEY_HIDE_ON_DASHBOARD = "hideOnDashboard"
KEY_INDEX = "index"
KEY_UNITS = "units"
KEY_ALIAS = "alias"
KEY_COLOR = "color"
KEY_PLOT_MIN = "plotMin"
KEY_PLOT_MAX = "plotMax"
KEY_GRAPH = "graph"
KEY_LOG = "log"
KEY_PLOT_LOG_X = "plotLogX"
KEY_PLOT_LOG_Y = "plotLogY"
KEY_FFT = "fft"
KEY_WATERFALL = "waterfall"
KEY_FFT_BALLISTICS = "fftBallistics"
KEY_FFT_BALLISTICS_RELEASE = "fftBallisticsRelease"
KEY_FFT_SAMPLES = "fftSamples"
KEY_FFT_WINDOW = "fftWindow"
KEY_FFT_SAMPLING_RATE = "fftSamplingRate"
KEY_FFT_LOG_X = "fftLogX"
KEY_FFT_MIN = "fftMin"
KEY_FFT_MAX = "fftMax"
KEY_WIDGET = "widget"
KEY_DISPLAY_TICK_COUNT = "displayTickCount"
KEY_DISPLAY_FORMAT = "displayFormat"
KEY_DECIMAL_POINTS = "decimalPoints"
KEY_WIDGET_MIN = "widgetMin"
KEY_WIDGET_MAX = "widgetMax"
KEY_EXTREME_HOLD = "extremeHold"
KEY_SUPPRESS_ALARMS = "suppressAlarms"
KEY_ALARMS_WHILE_VISIBLE = "alarmsWhileVisible"
KEY_ALARM_DEADBAND = "alarmDeadband"
KEY_LED = "led"
KEY_LED_HIGH = "ledHigh"
KEY_OVERVIEW_DISPLAY = "overviewDisplay"
KEY_FORMAT = "format"
KEY_FORMAT_WIDTH = "formatWidth"
KEY_FORMAT_SIGNED = "formatSigned"
KEY_FORMAT_FRACTION_BITS = "formatFractionBits"
KEY_FORMAT_BIT_OFFSET = "formatBitOffset"
KEY_FORMAT_BIT_WIDTH = "formatBitWidth"
KEY_FORMAT_SCALE = "formatScale"
KEY_FORMAT_OFFSET = "formatOffset"
KEY_LABELS_DEFAULT = "labelsDefault"
KEY_MIN = "min"
KEY_MAX = "max"
KEY_ALARM_BANDS = "alarmBands"
KEY_SEVERITY = "severity"
KEY_BLINK = "blink"
KEY_LABEL = "label"
KEY_FFT_MARKERS = "fftMarkers"
KEY_FREQ = "freq"
KEY_END_FREQ = "endFreq"
KEY_WARNING_DB = "warningDb"
KEY_ALARM_DB = "alarmDb"
KEY_LABELS = "labels"
KEY_VALUE = "value"
KEY_TEXT = "text"
KEY_GROUPS = "groups"
KEY_SOURCES = "sources"
KEY_DATASETS = "datasets"
KEY_GROUP_TYPE = "groupType"
KEY_OUTPUT_WIDGETS = "outputWidgets"
KEY_OUTPUT_COLUMNS = "outputColumns"
KEY_FRAME_START = "frameStart"
KEY_FRAME_END = "frameEnd"
KEY_FRAME_DETECTION = "frameDetection"
KEY_CHECKSUM_ALGORITHM = "checksumAlgorithm"
KEY_DECODER_METHOD = "decoderMethod"
KEY_HEXADECIMAL_DELIMITERS = "hexadecimalDelimiters"
KEY_FRAME_PARSER_TEMPLATE = "frameParserTemplate"
KEY_FRAME_PARSER_PARAMS = "frameParserParams"
KEY_OUTPUT_TYPE = "outputType"
KEY_OUTPUT_MIN = "outputMin"
KEY_OUTPUT_MAX = "outputMax"
KEY_OUTPUT_STEP = "outputStep"
KEY_INITIAL_VALUE = "initialValue"
KEY_ON_LABEL = "onLabel"
KEY_OFF_LABEL = "offLabel"
KEY_CHECKABLE = "checkable"
KEY_INPUT_KIND = "inputKind"
KEY_OUTPUT_DECIMALS = "outputDecimals"
KEY_OUTPUT_UNITS = "outputUnits"
KEY_QUANTIZE_TO_STEP = "quantizeToStep"

DATASET_KEYS = (
    "title",
    "hideOnDashboard",
    "index",
    "units",
    "alias",
    "color",
    "plotMin",
    "plotMax",
    "graph",
    "log",
    "plotLogX",
    "plotLogY",
    "fft",
    "waterfall",
    "fftBallistics",
    "fftBallisticsRelease",
    "fftSamples",
    "fftWindow",
    "fftSamplingRate",
    "fftLogX",
    "fftMin",
    "fftMax",
    "widget",
    "displayTickCount",
    "displayFormat",
    "decimalPoints",
    "widgetMin",
    "widgetMax",
    "extremeHold",
    "suppressAlarms",
    "alarmsWhileVisible",
    "alarmDeadband",
    "led",
    "ledHigh",
    "overviewDisplay",
    "format",
    "formatWidth",
    "formatSigned",
    "formatFractionBits",
    "formatBitOffset",
    "formatBitWidth",
    "formatScale",
    "formatOffset",
    "labelsDefault",
    "min",
    "max",
)

FRAME_KEYS = (
    "title",
    "groups",
    "sources",
)

GROUP_KEYS = (
    "title",
    "widget",
    "datasets",
    "groupType",
    "outputWidgets",
    "outputColumns",
)

SOURCE_KEYS = (
    "frameStart",
    "frameEnd",
    "frameDetection",
    "checksumAlgorithm",
    "decoderMethod",
    "hexadecimalDelimiters",
    "frameParserTemplate",
    "frameParserParams",
)

CONTROL_KEYS = (
    "title",
    "outputType",
    "outputMin",
    "outputMax",
    "outputStep",
    "initialValue",
    "onLabel",
    "offLabel",
    "checkable",
    "inputKind",
    "outputDecimals",
    "outputUnits",
    "quantizeToStep",
)

SUB_ENTITY_KEYS = {
    "alarmBands": ("min", "max", "severity", "blink", "color", "label"),
    "fftMarkers": ("freq", "endFreq", "warningDb", "alarmDb", "color", "label"),
    "labels": ("value", "text"),
}

SUB_ENTITY_CAPS = {
    "alarmBands": 16,
    "fftMarkers": 16,
    "labels": 256,
}

GROUP_WIDGETS = (
    "datagrid",
    "multiplot",
    "accelerometer",
    "gyro",
    "gps",
    "plot3d",
    "barpanel",
)

DATASET_WIDGETS = (
    "bar",
    "compass",
    "gauge",
    "meter",
)
