/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QString>

//--------------------------------------------------------------------------------------------------
// Standard keys for loading/offloading frame structures using JSON files
//--------------------------------------------------------------------------------------------------

namespace Keys {
/**
 * @brief Non-allocating view alias used for inline-constexpr JSON key constants.
 */
using KeyView = QLatin1StringView;

// Action keys
inline constexpr KeyView ActionId("actionId");
inline constexpr KeyView EOL("eol");
inline constexpr KeyView Icon("icon");
inline constexpr KeyView Title("title");
inline constexpr KeyView TxData("txData");
inline constexpr KeyView Binary("binary");
inline constexpr KeyView TxEncoding("txEncoding");
inline constexpr KeyView TimerMode("timerMode");
inline constexpr KeyView RepeatCount("repeatCount");
inline constexpr KeyView TimerInterval("timerIntervalMs");
inline constexpr KeyView AutoExecute("autoExecuteOnConnect");

// Source / connection keys
inline constexpr KeyView Sources("sources");
inline constexpr KeyView SourceId("sourceId");
inline constexpr KeyView SourceConn("connection");
inline constexpr KeyView DatasetSourceId("datasetSourceId");
inline constexpr KeyView BusType("busType");
inline constexpr KeyView FrameStart("frameStart");
inline constexpr KeyView FrameEnd("frameEnd");
inline constexpr KeyView Checksum("checksum");
inline constexpr KeyView ChecksumAlgorithm("checksumAlgorithm");
inline constexpr KeyView FrameDetection("frameDetection");
inline constexpr KeyView Decoder("decoder");
inline constexpr KeyView DecoderMethod("decoderMethod");
inline constexpr KeyView HexadecimalDelimiters("hexadecimalDelimiters");
inline constexpr KeyView FrameParserCode("frameParserCode");
inline constexpr KeyView FrameParserLanguage("frameParserLanguage");
inline constexpr KeyView FrameParserTemplate("frameParserTemplate");
inline constexpr KeyView FrameParserParams("frameParserParams");
inline constexpr KeyView SourceStreamLane("streamLane");

// Dataset keys
inline constexpr KeyView FFT("fft");
inline constexpr KeyView LED("led");
inline constexpr KeyView Log("log");
inline constexpr KeyView Waterfall("waterfall");
inline constexpr KeyView WaterfallYAxis("waterfallYAxis");
inline constexpr KeyView Min("min");
inline constexpr KeyView Max("max");
inline constexpr KeyView Graph("graph");
inline constexpr KeyView Index("index");
inline constexpr KeyView XAxis("xAxis");
inline constexpr KeyView Alarm("alarm");
inline constexpr KeyView Units("units");
inline constexpr KeyView Value("value");
inline constexpr KeyView Widget("widget");
inline constexpr KeyView FFTMin("fftMin");
inline constexpr KeyView FFTMax("fftMax");
inline constexpr KeyView PltMin("plotMin");
inline constexpr KeyView PltMax("plotMax");
inline constexpr KeyView LedHigh("ledHigh");
inline constexpr KeyView WgtMin("widgetMin");
inline constexpr KeyView WgtMax("widgetMax");
inline constexpr KeyView AlarmLow("alarmLow");
inline constexpr KeyView AlarmHigh("alarmHigh");
inline constexpr KeyView DisplayTickCount("displayTickCount");
inline constexpr KeyView DisplayFormat("displayFormat");
inline constexpr KeyView DecimalPoints("decimalPoints");
inline constexpr KeyView FFTSamples("fftSamples");
inline constexpr KeyView FFTLogX("fftLogX");
inline constexpr KeyView FFTBallistics("fftBallistics");
inline constexpr KeyView FFTBallisticsRelease("fftBallisticsRelease");
inline constexpr KeyView PltLogX("plotLogX");
inline constexpr KeyView PltLogY("plotLogY");
inline constexpr KeyView Overview("overviewDisplay");
inline constexpr KeyView AlarmEnabled("alarmEnabled");
inline constexpr KeyView AlarmBands("alarmBands");
inline constexpr KeyView Color("color");
inline constexpr KeyView Alias("alias");
inline constexpr KeyView Label("label");
inline constexpr KeyView Blink("blink");
inline constexpr KeyView Severity("severity");
inline constexpr KeyView FFTSamplingRate("fftSamplingRate");
inline constexpr KeyView FFTWindow("fftWindow");
inline constexpr KeyView FFTMarkers("fftMarkers");
inline constexpr KeyView Frequency("freq");
inline constexpr KeyView EndFrequency("endFreq");
inline constexpr KeyView WarningDb("warningDb");
inline constexpr KeyView AlarmDb("alarmDb");
inline constexpr KeyView TransformCode("transformCode");
inline constexpr KeyView TransformLanguage("transformLanguage");
inline constexpr KeyView DatasetId("datasetId");
inline constexpr KeyView UniqueId("uniqueId");
inline constexpr KeyView NumericValue("numericValue");
inline constexpr KeyView ExtremeHold("extremeHold");

// Frame container keys
inline constexpr KeyView Groups("groups");
inline constexpr KeyView Actions("actions");
inline constexpr KeyView Datasets("datasets");
inline constexpr KeyView OutputWidgets("outputWidgets");
inline constexpr KeyView ControlScriptCode("controlScriptCode");

// Output widget keys
inline constexpr KeyView OutputType("outputType");
inline constexpr KeyView OutputMinValue("outputMin");
inline constexpr KeyView OutputMaxValue("outputMax");
inline constexpr KeyView OutputStepSize("outputStep");
inline constexpr KeyView OutputInitialValue("initialValue");
inline constexpr KeyView OutputOnLabel("onLabel");
inline constexpr KeyView OutputOffLabel("offLabel");
inline constexpr KeyView OutputMonoIcon("monoIcon");
inline constexpr KeyView OutputColumns("outputColumns");
inline constexpr KeyView TransmitFunction("transmitFunction");
inline constexpr KeyView OutputTxEncoding("outputTxEncoding");

// Group keys
inline constexpr KeyView GroupId("groupId");
inline constexpr KeyView GroupType("groupType");
inline constexpr KeyView GroupFolders("groupFolders");

// Image-group keys
inline constexpr KeyView ImgMode("imgDetectionMode");
inline constexpr KeyView ImgStart("imgStartSequence");
inline constexpr KeyView ImgEnd("imgEndSequence");

// Painter-group keys
inline constexpr KeyView PainterCode("painterCode");
inline constexpr KeyView HideOnDashboard("hideOnDashboard");

// Web-view-group keys
inline constexpr KeyView WebViewUrl("webViewUrl");

// Bar-panel-group keys
inline constexpr KeyView BarPanelStyle("barPanelStyle");

// Dashboard layout keys
inline constexpr KeyView DashboardLayout("dashboardLayout");
inline constexpr KeyView ActiveGroupId("activeGroupId");
inline constexpr KeyView WidgetSettings("widgetSettings");
inline constexpr KeyView Frozen("frozen");

// Widget display overrides (ProjectModel state, never serialized into frames)
inline constexpr KeyView WidgetDisplay("widgetDisplay");
inline constexpr KeyView Titles("titles");
inline constexpr KeyView FreezeTitle("freezeTitle");

// Project-editor tree state (path-keyed node expansion map)
inline constexpr KeyView TreeExpansion("treeExpansion");

// Project-overview diagram state (stable-id keyed node collapse map)
inline constexpr KeyView DiagramCollapse("diagramCollapse");

// Plot history keys
inline constexpr KeyView PointCount("pointCount");
inline constexpr KeyView PlotTimeRange("plotTimeRange");

// Transform execution
inline constexpr KeyView ChangeDrivenTransforms("changeDrivenTransforms");
inline constexpr KeyView LuaFastMode("luaFastMode");
inline constexpr KeyView kActiveGroupSubKey("activeGroup");
inline constexpr KeyView kDashboardWindowsSubKey("dashboardWindows");
inline constexpr KeyView HiddenGroups("hiddenGroups");

// Workspace keys
inline constexpr KeyView Workspaces("workspaces");
inline constexpr KeyView WorkspaceId("workspaceId");
inline constexpr KeyView WidgetRefs("widgetRefs");
inline constexpr KeyView WidgetType("widgetType");
inline constexpr KeyView RelativeIndex("relativeIndex");
inline constexpr KeyView CustomizeWorkspaces("customizeWorkspaces");
inline constexpr KeyView WorkspaceDescription("description");
inline constexpr KeyView WorkspaceFolders("workspaceFolders");
inline constexpr KeyView FolderId("folderId");
inline constexpr KeyView ParentFolderId("parentFolderId");

inline constexpr KeyView Virtual("virtual");

// Group/dataset enablement: written only when off, so absence means enabled.
inline constexpr KeyView Disabled("disabled");

// Data table keys
inline constexpr KeyView Tables("tables");
inline constexpr KeyView TableFolders("tableFolders");
inline constexpr KeyView Registers("registers");
inline constexpr KeyView RegisterTypeName("type");
inline constexpr KeyView Name("name");

// Project metadata keys (file format / app version stamps)
inline constexpr KeyView SchemaVersion("schemaVersion");
inline constexpr KeyView WriterVersion("writerVersion");
inline constexpr KeyView WriterVersionAtCreation("writerVersionAtCreation");
inline constexpr KeyView NextUniqueId("nextUniqueId");

// Project lock: PBKDF2 hash (legacy MD5 still accepted on load).
inline constexpr KeyView PasswordHash("passwordHash");

// Per-project MQTT publisher configuration (Pro).
inline constexpr KeyView MqttPublisher("mqttPublisher");

// Per-project InfluxDB sink configuration (Pro); absent means disabled, token lives in the vault.
inline constexpr KeyView InfluxSink("influxSink");

// Sparkplug B slot table in the MQTT connection block; absent means derive on the first birth.
inline constexpr KeyView SparkplugSlots("sparkplugSlots");
inline constexpr KeyView SparkplugNode("node");
inline constexpr KeyView SparkplugGroup("group");
inline constexpr KeyView SparkplugDevice("device");
inline constexpr KeyView SparkplugMetric("metric");

inline QString layoutKey(int groupId)
{
  return QStringLiteral("layout:") + QString::number(groupId);
}

inline QString layoutKey(const QString& scope, int groupId)
{
  if (scope.isEmpty())
    return layoutKey(groupId);

  return QStringLiteral("layout:") + scope + QStringLiteral(":") + QString::number(groupId);
}
}  // namespace Keys
