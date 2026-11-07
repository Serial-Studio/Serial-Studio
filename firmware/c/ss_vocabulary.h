/*
 * Serial Studio device library
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * AUTO-GENERATED from app/rcc/properties/dataset.json and declaration.json;
 * never edit by hand. Regenerate with: python3 scripts/generate-property-registry.py
 */

#ifndef SS_VOCABULARY_H
#define SS_VOCABULARY_H

#define SS_PREFIX "#ss"
#define SS_REQUEST "#ss?"
#define SS_COMMAND "#ss!"
#define SS_BINARY_TEMPLATE "packed_struct"
#define SS_WIRE_VERSION 1
#define SS_LIBRARY_VERSION "1.0.0"
#define SS_LIBRARY_VERSION_MAJOR 1
#define SS_LIBRARY_VERSION_MINOR 0
#define SS_LIBRARY_VERSION_PATCH 0

#define SS_MAX_LINE_BYTES 16384
#define SS_MAX_STRING_BYTES 64
#define SS_MAX_GROUPS 64
#define SS_MAX_DATASETS 512
#define SS_MAX_CONTROLS 64
#define SS_MAX_ALARM_BANDS 16
#define SS_MAX_FFT_MARKERS 16
#define SS_MAX_LABELS 256

#define SS_KEY_TITLE "title"
#define SS_KEY_HIDE_ON_DASHBOARD "hideOnDashboard"
#define SS_KEY_INDEX "index"
#define SS_KEY_UNITS "units"
#define SS_KEY_ALIAS "alias"
#define SS_KEY_COLOR "color"
#define SS_KEY_PLOT_MIN "plotMin"
#define SS_KEY_PLOT_MAX "plotMax"
#define SS_KEY_GRAPH "graph"
#define SS_KEY_LOG "log"
#define SS_KEY_PLOT_LOG_X "plotLogX"
#define SS_KEY_PLOT_LOG_Y "plotLogY"
#define SS_KEY_FFT "fft"
#define SS_KEY_WATERFALL "waterfall"
#define SS_KEY_FFT_BALLISTICS "fftBallistics"
#define SS_KEY_FFT_BALLISTICS_RELEASE "fftBallisticsRelease"
#define SS_KEY_FFT_SAMPLES "fftSamples"
#define SS_KEY_FFT_WINDOW "fftWindow"
#define SS_KEY_FFT_SAMPLING_RATE "fftSamplingRate"
#define SS_KEY_FFT_LOG_X "fftLogX"
#define SS_KEY_FFT_MIN "fftMin"
#define SS_KEY_FFT_MAX "fftMax"
#define SS_KEY_WIDGET "widget"
#define SS_KEY_DISPLAY_TICK_COUNT "displayTickCount"
#define SS_KEY_DISPLAY_FORMAT "displayFormat"
#define SS_KEY_DECIMAL_POINTS "decimalPoints"
#define SS_KEY_WIDGET_MIN "widgetMin"
#define SS_KEY_WIDGET_MAX "widgetMax"
#define SS_KEY_EXTREME_HOLD "extremeHold"
#define SS_KEY_SUPPRESS_ALARMS "suppressAlarms"
#define SS_KEY_ALARMS_WHILE_VISIBLE "alarmsWhileVisible"
#define SS_KEY_ALARM_DEADBAND "alarmDeadband"
#define SS_KEY_LED_HIGH "ledHigh"
#define SS_KEY_OVERVIEW_DISPLAY "overviewDisplay"
#define SS_KEY_FORMAT "format"
#define SS_KEY_FORMAT_WIDTH "formatWidth"
#define SS_KEY_FORMAT_SIGNED "formatSigned"
#define SS_KEY_FORMAT_FRACTION_BITS "formatFractionBits"
#define SS_KEY_FORMAT_BIT_OFFSET "formatBitOffset"
#define SS_KEY_FORMAT_BIT_WIDTH "formatBitWidth"
#define SS_KEY_FORMAT_SCALE "formatScale"
#define SS_KEY_FORMAT_OFFSET "formatOffset"
#define SS_KEY_LABELS_DEFAULT "labelsDefault"
#define SS_KEY_MIN "min"
#define SS_KEY_MAX "max"
#define SS_KEY_ALARM_BANDS "alarmBands"
#define SS_KEY_SEVERITY "severity"
#define SS_KEY_BLINK "blink"
#define SS_KEY_LABEL "label"
#define SS_KEY_FFT_MARKERS "fftMarkers"
#define SS_KEY_FREQ "freq"
#define SS_KEY_END_FREQ "endFreq"
#define SS_KEY_WARNING_DB "warningDb"
#define SS_KEY_ALARM_DB "alarmDb"
#define SS_KEY_LABELS "labels"
#define SS_KEY_VALUE "value"
#define SS_KEY_TEXT "text"
#define SS_KEY_GROUPS "groups"
#define SS_KEY_SOURCES "sources"
#define SS_KEY_DATASETS "datasets"
#define SS_KEY_GROUP_TYPE "groupType"
#define SS_KEY_OUTPUT_WIDGETS "outputWidgets"
#define SS_KEY_OUTPUT_COLUMNS "outputColumns"
#define SS_KEY_FRAME_START "frameStart"
#define SS_KEY_FRAME_END "frameEnd"
#define SS_KEY_FRAME_DETECTION "frameDetection"
#define SS_KEY_CHECKSUM_ALGORITHM "checksumAlgorithm"
#define SS_KEY_DECODER_METHOD "decoderMethod"
#define SS_KEY_HEXADECIMAL_DELIMITERS "hexadecimalDelimiters"
#define SS_KEY_FRAME_PARSER_TEMPLATE "frameParserTemplate"
#define SS_KEY_FRAME_PARSER_PARAMS "frameParserParams"
#define SS_KEY_OUTPUT_TYPE "outputType"
#define SS_KEY_OUTPUT_MIN "outputMin"
#define SS_KEY_OUTPUT_MAX "outputMax"
#define SS_KEY_OUTPUT_STEP "outputStep"
#define SS_KEY_INITIAL_VALUE "initialValue"
#define SS_KEY_ON_LABEL "onLabel"
#define SS_KEY_OFF_LABEL "offLabel"
#define SS_KEY_CHECKABLE "checkable"
#define SS_KEY_INPUT_KIND "inputKind"
#define SS_KEY_OUTPUT_DECIMALS "outputDecimals"
#define SS_KEY_OUTPUT_UNITS "outputUnits"
#define SS_KEY_QUANTIZE_TO_STEP "quantizeToStep"

#define SS_GROUP_WIDGET_DATAGRID "datagrid"
#define SS_GROUP_WIDGET_MULTIPLOT "multiplot"
#define SS_GROUP_WIDGET_ACCELEROMETER "accelerometer"
#define SS_GROUP_WIDGET_GYRO "gyro"
#define SS_GROUP_WIDGET_GPS "gps"
#define SS_GROUP_WIDGET_PLOT3D "plot3d"
#define SS_GROUP_WIDGET_BARPANEL "barpanel"
#define SS_GROUP_WIDGET_LEDPANEL "ledpanel"

#define SS_DATASET_WIDGET_BAR "bar"
#define SS_DATASET_WIDGET_COMPASS "compass"
#define SS_DATASET_WIDGET_GAUGE "gauge"
#define SS_DATASET_WIDGET_METER "meter"

#endif
