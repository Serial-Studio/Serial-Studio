/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Importers/DBCImporter.h"

#include <algorithm>
#include <cmath>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QList>
#include <QSet>
#include <QStandardPaths>
#include <QStringList>

#include "Core/DataModel/Frame.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/SerialStudio.h"
#include "DataModel/Importers/AxisTicks.h"
#include "DataModel/Importers/DBCMultiplexing.h"
#include "DataModel/Importers/ImporterCommon.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a DBC importer bound to one session.
 */
DataModel::DBCImporter::DBCImporter(ProjectModel& projectModel)
  : m_projectModel(projectModel), m_skippedExtendedMuxSignals(0)
{}

/**
 * @brief Returns the singleton DBCImporter instance.
 */
DataModel::DBCImporter& DataModel::DBCImporter::instance()
{
  static DBCImporter instance(DataModel::ProjectModel::instance());
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of signals across all loaded messages.
 */
int DataModel::DBCImporter::signalCount() const
{
  return countTotalSignals(m_messages);
}

/**
 * @brief Returns the number of messages loaded from the DBC file.
 */
int DataModel::DBCImporter::messageCount() const
{
  return m_messages.count();
}

/**
 * @brief Returns the file name of the loaded DBC file.
 */
QString DataModel::DBCImporter::dbcFileName() const
{
  return QFileInfo(m_dbcFilePath).fileName();
}

/**
 * @brief Returns a formatted "N: Name @ 0xID (X signals)" string.
 */
QString DataModel::DBCImporter::messageInfo(int index) const
{
  if (index < 0 || index >= m_messages.count())
    return QString();

  const auto& message    = m_messages.at(index);
  const auto msgId       = static_cast<quint32>(message.uniqueId());
  const auto signalCount = static_cast<int>(DBCMux::orderedSignals(message).size());

  return QString("%1: %2 @ 0x%3 (%4 signals)")
    .arg(index + 1)
    .arg(message.name())
    .arg(QString::number(msgId, 16).toUpper())
    .arg(signalCount);
}

//--------------------------------------------------------------------------------------------------
// User interface operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a file dialog to select a DBC file and preview it.
 */
void DataModel::DBCImporter::importDBC()
{
  const auto p = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  auto* dialog = new QFileDialog(
    qApp->activeWindow(), tr("Import DBC File"), p, tr("DBC Files (*.dbc);;All Files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { showPreview(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Cancels the current import and clears loaded data.
 */
void DataModel::DBCImporter::cancelImport()
{
  m_messages.clear();
  m_tableNames.clear();
  m_dbcFilePath.clear();
  m_valueDescriptions.clear();
  Q_EMIT messagesChanged();
  Q_EMIT dbcFileNameChanged();
}

/**
 * @brief Parses the DBC file at filePath and emits previewReady on success.
 */
void DataModel::DBCImporter::showPreview(const QString& filePath)
{
  QCanDbcFileParser parser;
  if (!parser.parse(filePath)) {
    Core::Prompt::showMessageBox(tr("Failed to parse DBC file: %1").arg(parser.errorString()),
                                 tr("Verify the file format and try again."),
                                 Core::Prompt::Critical,
                                 tr("DBC Import Error"));
    return;
  }

  m_messages          = parser.messageDescriptions();
  m_valueDescriptions = parser.messageValueDescriptions();
  if (m_messages.isEmpty()) {
    Core::Prompt::showMessageBox(
      tr("DBC file contains no messages"),
      tr("The selected file does not contain any CAN message definitions."),
      Core::Prompt::Warning,
      tr("DBC Import Warning"));
    return;
  }

  std::sort(m_messages.begin(),
            m_messages.end(),
            [](const QCanMessageDescription& a, const QCanMessageDescription& b) {
              return a.uniqueId() < b.uniqueId();
            });

  m_dbcFilePath = filePath;
  Q_EMIT messagesChanged();
  Q_EMIT dbcFileNameChanged();
  Q_EMIT previewReady();
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates a Serial Studio project from the parsed DBC messages.
 */
void DataModel::DBCImporter::confirmImport()
{
  if (m_messages.isEmpty())
    return;

  const auto project       = projectFromMessages(m_messages);
  const QString suggestion = QFileInfo(m_dbcFilePath).baseName();

  const int messageCount  = m_messages.count();
  const int signalCount   = countTotalSignals(m_messages);
  const int skippedExtMux = m_skippedExtendedMuxSignals;

  auto& pm = m_projectModel;
  QObject::connect(
    &pm,
    &ProjectModel::importCompleted,
    this,
    [messageCount, signalCount, skippedExtMux](bool accepted, const QString&) {
      if (!accepted)
        return;

      QString detail = tr("The project editor is now open for customization.");
      if (skippedExtMux > 0)
        detail += tr(" Skipped %1 signal(s) whose multiplexing could not be resolved: a switch "
                     "value outside the integer range, or a circular SG_MUL_VAL_ chain.")
                    .arg(skippedExtMux);

      Core::Prompt::showMessageBox(
        tr("Successfully imported DBC file with %1 messages and %2 signals.")
          .arg(messageCount)
          .arg(signalCount),
        detail,
        Core::Prompt::Information,
        tr("DBC Import Complete"));
    },
    Qt::SingleShotConnection);

  pm.importProjectFromJson(project, suggestion);
}

/**
 * @brief Builds a complete .ssproj QJsonObject from the DBC messages. Reaches no session
 *        state, so the decode-shape regressions this class has had are reachable from a test.
 */
QJsonObject DataModel::DBCImporter::projectFromMessages(
  const QList<QCanMessageDescription>& messages)
{
  QJsonObject project;

  const auto dbcInfo      = QFileInfo(m_dbcFilePath);
  const auto projectTitle = dbcInfo.baseName();

  project[Keys::Title]   = projectTitle;
  project[Keys::Actions] = QJsonArray();

  buildTableNames(messages);

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("CAN Bus");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::CanBus);
  source[Keys::FrameStart]            = QString("");
  source[Keys::FrameEnd]              = QString("");
  source[Keys::Checksum]              = QString("");
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = generateLuaParser(messages);
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Lua);

  project[Keys::Sources] = QJsonArray{source};

  auto groups       = generateGroups(messages);
  const auto tables = generateTables(messages);
  finalizeImportedProject(project, groups, tables, tr("Overview"));

  return project;
}

/**
 * @brief Titles a dataset after its signal, tagging the multiplexor role and the switch values
 *        that bring a gated signal into the frame so two mux variants never share a title.
 */
QString DataModel::DBCImporter::datasetTitle(const DBCMux::OrderedSignal& entry)
{
  auto name       = entry.signal.name();
  const auto gate = DBCMux::muxTitleSuffix(entry.gates);

  if (entry.role == DBCMux::MuxRole::Selector && gate.isEmpty())
    return QString("%1 (selector)").arg(name);

  if (entry.role == DBCMux::MuxRole::Selector)
    return QString("%1 (selector, mux %2)").arg(name, gate);

  if (!gate.isEmpty())
    return QString("%1 (mux %2)").arg(name, gate);

  return name;
}

/**
 * @brief Builds a Dataset for a CAN signal: a computed dataset whose Lua transform reads the
 *        signal's physical value back from the message's data table.
 */
DataModel::Dataset DataModel::DBCImporter::buildDatasetFromSignal(
  const DBCMux::OrderedSignal& entry,
  const QString& groupWidget,
  const QString& tableName,
  const QCanDbcFileParser::ValueDescriptions& valueLabels,
  int datasetIndex)
{
  const auto& signal = entry.signal;

  DataModel::Dataset dataset;
  dataset.index = datasetIndex;
  dataset.units = signal.physicalUnit();
  dataset.log   = true;
  dataset.fft   = false;
  dataset.title = datasetTitle(entry);

  double minVal = signal.minimum();
  double maxVal = signal.maximum();
  if (std::isnan(minVal) || std::isnan(maxVal) || minVal >= maxVal) {
    minVal = 0.0;
    maxVal = 100.0;
  }

  dataset.pltMin = minVal;
  dataset.pltMax = maxVal;
  dataset.fftMin = minVal;
  dataset.fftMax = maxVal;

  const auto nice          = niceAxisTicks(minVal, maxVal);
  dataset.wgtMin           = nice.min;
  dataset.wgtMax           = nice.max;
  dataset.displayTickCount = nice.tickCount;

  const int decimals =
    qMax(fractionalDecimals(signal.factor()), fractionalDecimals(signal.offset()));
  dataset.displayFormat = QStringLiteral("%1d").arg(decimals);

  applyTableTransform(dataset, tableName, signal.name());

  if (signal.bitLength() == 1) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    applyBooleanLedBand(dataset, tr("Active"));
    return dataset;
  }

  const bool plainScaling = (signal.factor() == 1.0 && signal.offset() == 0.0);
  if (!valueLabels.isEmpty() && plainScaling) {
    dataset.transformCode = enumTransformCode(tableName, signal.name(), valueLabels);
    return dataset;
  }

  const bool dataGridGroup = (groupWidget == SerialStudio::groupWidgetId(SerialStudio::DataGrid));
  dataset.widget           = dataGridGroup ? selectWidgetForSignal(signal) : QString("");
  dataset.plt              = dataGridGroup && isPlottableSignal(signal);
  if (!dataset.widget.isEmpty())
    applyAnalogDisplayPolicy(dataset);

  return dataset;
}

/**
 * @brief Builds Group/Dataset structures from the DBC messages.
 */
std::vector<DataModel::Group> DataModel::DBCImporter::generateGroups(
  const QList<QCanMessageDescription>& messages)
{
  std::vector<DataModel::Group> groups;
  int groupId      = 0;
  int datasetIndex = 1;

  for (const auto& message : messages) {
    if (!hasImportableSignals(message))
      continue;

    DataModel::Group group;
    group.groupId = groupId;
    group.title   = message.name();
    group.widget  = selectGroupWidget(message);

    const auto tableName = tableNameFor(message);
    const auto msgLabels = m_valueDescriptions.value(message.uniqueId());

    for (const auto& entry : DBCMux::orderedSignals(message))
      group.datasets.push_back(buildDatasetFromSignal(
        entry, group.widget, tableName, msgLabels.value(entry.signal.name()), datasetIndex++));

    groups.push_back(group);
    ++groupId;
  }

  return groups;
}

/**
 * @brief Assigns one collision-free data-table name per message (the message name, with the
 *        frame id appended when a DBC reuses a name).
 */
void DataModel::DBCImporter::buildTableNames(const QList<QCanMessageDescription>& messages)
{
  m_tableNames.clear();

  QSet<QString> used;
  for (const auto& message : messages) {
    const auto id  = static_cast<quint32>(message.uniqueId()) & 0x1FFFFFFF;
    const auto hex = QString::number(id, 16).toUpper();

    QString name = message.name().simplified();
    if (name.isEmpty())
      name = QStringLiteral("Message 0x%1").arg(hex);

    if (used.contains(name))
      name += QStringLiteral(" @ 0x%1").arg(hex);

    used.insert(name);
    m_tableNames.insert(id, name);
  }
}

/**
 * @brief Returns the data-table name assigned to the message by buildTableNames().
 */
QString DataModel::DBCImporter::tableNameFor(const QCanMessageDescription& message) const
{
  return m_tableNames.value(static_cast<quint32>(message.uniqueId()));
}

/**
 * @brief Builds one data table per message: one Computed register per importable signal,
 *        defaulting to 0 until the first frame for that message arrives.
 */
std::vector<DataModel::TableDef> DataModel::DBCImporter::generateTables(
  const QList<QCanMessageDescription>& messages)
{
  std::vector<DataModel::TableDef> tables;

  for (const auto& message : messages) {
    if (!hasImportableSignals(message))
      continue;

    DataModel::TableDef table;
    table.name = tableNameFor(message);

    for (const auto& entry : DBCMux::orderedSignals(message)) {
      DataModel::RegisterDef reg;
      reg.name         = entry.signal.name();
      reg.type         = DataModel::RegisterType::Computed;
      reg.defaultValue = QVariant(0.0);
      table.registers.push_back(reg);
    }

    tables.push_back(table);
  }

  return tables;
}

//--------------------------------------------------------------------------------------------------
// Lua parser generation
//--------------------------------------------------------------------------------------------------

// code-verify off
/**
 * @brief Returns the static extract()/parse() machinery appended below the MESSAGES spec.
 *        Fenced: the linter's C++ parser misreads the embedded Lua literal.
 */
[[nodiscard]] static QString dbcLuaParserBody()
{
  return QStringLiteral(R"LUA(
-- Extracts one raw signal value; base is the 1-based index of the first
-- payload byte. The Binary decoder hands parse() the frame as a 1-indexed
-- table of byte values. Motorola (big-endian) walks the in-byte bit index
-- down and jumps +15 across byte boundaries (the DBC sawtooth); Intel
-- (little-endian) reads LSB-first runs. Bytes past the frame read 0.
-- Byte-level masking uses the 32-bit bit library; the VALUE accumulates in
-- plain arithmetic, which stays exact to 53-bit signals (the display
-- pipeline is double-precision anyway; 5.3 bitwise ops would cap at 32).
local function extract(frame, base, sig)
  local value = 0
  if sig.be then
    local bit_pos = sig.start
    for _ = 1, sig.len do
      local byte = frame[base + math.floor(bit_pos / 8)] or 0
      value = value * 2 + bit.band(bit.rshift(byte, bit_pos % 8), 1)
      bit_pos = (bit_pos % 8 == 0) and (bit_pos + 15) or (bit_pos - 1)
    end
  else
    local bits_read = 0
    local bit_shift = sig.start % 8
    local byte_idx = math.floor(sig.start / 8)
    while bits_read < sig.len do
      local byte = frame[base + byte_idx] or 0
      local take = math.min(8 - bit_shift, sig.len - bits_read)
      value = value + bit.band(bit.rshift(byte, bit_shift), 2 ^ take - 1) * 2 ^ bits_read
      bits_read = bits_read + take
      bit_shift = 0
      byte_idx = byte_idx + 1
    end
  end

  if sig.signed and value >= 2 ^ (sig.len - 1) then
    value = value - 2 ^ sig.len
  end

  return value
end

-- Returns the CAN id and the 1-based index of the first payload byte: bit 7
-- of byte 1 selects the extended (29-bit, 4-byte id) header form.
local function frame_id(frame)
  local b1 = frame[1]
  if bit.band(b1, 0x80) ~= 0 then
    if #frame < 5 then
      return nil
    end

    local id = bit.band(b1, 0x1F) * 16777216 + frame[2] * 65536
             + frame[3] * 256 + frame[4]
    return id, 6
  end

  return (b1 * 256 + frame[2]), 4
end

-- True when a raw multiplexor value falls inside one of the inclusive
-- {lo, hi} ranges an SG_MUL_VAL_ entry declared for a signal.
local function in_ranges(value, ranges)
  for _, range in ipairs(ranges) do
    if value >= range[1] and value <= range[2] then
      return true
    end
  end

  return false
end

-- Evaluates a signal's mux field against the selector values decoded so far:
-- a bare number matches the message's top-level multiplexor, a {p=, r=} table
-- one named switch, and a list of those tables a signal that several switches
-- must agree on. Selectors always precede their dependents in the spec, so
-- every name a gate gives has already been read.
local function mux_ok(mux, selectors, root)
  if type(mux) == "number" then
    return mux == root
  end

  if mux.p then
    local value = selectors[mux.p]
    return value ~= nil and in_ranges(value, mux.r)
  end

  for _, gate in ipairs(mux) do
    local value = selectors[gate.p]
    if value == nil or not in_ranges(value, gate.r) then
      return false
    end
  end

  return true
end

function parse(frame)
  if #frame < 3 then
    return {}
  end

  local id, base = frame_id(frame)
  local msg = id and MESSAGES[id]
  if not msg then
    return {}
  end

  local root = nil
  local selectors = {}
  for _, sig in ipairs(msg.signals) do
    if sig.mux == nil or mux_ok(sig.mux, selectors, root) then
      local raw = extract(frame, base, sig)
      if sig.selector then
        selectors[sig.name] = raw
        root = root or raw
      end

      tableSet(msg.table, sig.name, raw * (sig.factor or 1) + (sig.offset or 0))
    end
  end

  return { 0 } -- values flow through tableSet; datasets read them back
end
)LUA");
}

// code-verify on

/**
 * @brief Generates the user-editable Lua frame parser: a documented header, the declarative
 *        MESSAGES spec, and the generic extract()/parse() machinery. Also counts the signals
 *        whose multiplexing the import could not resolve and therefore dropped.
 */
QString DataModel::DBCImporter::generateLuaParser(const QList<QCanMessageDescription>& messages)
{
  m_skippedExtendedMuxSignals = 0;
  for (const auto& message : messages) {
    const auto declared = message.signalDescriptions().size();
    m_skippedExtendedMuxSignals +=
      static_cast<int>(declared - DBCMux::orderedSignals(message).size());
  }

  QString spec;
  for (const auto& message : messages)
    if (hasImportableSignals(message))
      spec += generateMessageSpec(message);

  const QString header = QStringLiteral(R"LUA(--[[
  CAN frame parser generated by the Serial Studio DBC importer.
  Source database: %1

  Wire format (Serial Studio CAN driver):
    standard:  [ID_hi, ID_lo, DLC, data0..dataN]
    extended:  [0x80|ID28..24, ID23..16, ID15..8, ID7..0, DLC, data0..dataN]
    parse() receives these bytes as a 1-indexed Lua table (Binary decoder).

  Data flow:
    parse() decodes every known signal and publishes its physical value
    with tableSet("<message>", "<signal>", value); each dataset reads the
    value back with tableGet() inside its transform, so datasets never
    depend on positional parser channels.

  To add a signal:
    1. Add one line to MESSAGES below (bit layout from the DBC).
    2. Add a register with the same name to the message's data table.
    3. Add a dataset whose transform reads it back with tableGet().
]]

-- Signal layout fields (omitted fields use the defaults):
--   start    DBC start bit               len     bit length
--   be       true = Motorola (@0)        signed  true = two's complement
--   factor   raw -> physical multiplier  offset  raw -> physical offset
--   selector marks the multiplexor       mux     gates the entry on a selector value
-- A numeric mux matches the message's top-level multiplexor; the extended form
-- mux = {p = "Switch", r = {{lo, hi}}} matches inclusive SG_MUL_VAL_ ranges of a
-- named switch, and a list of those gates requires every switch to agree.
local MESSAGES = {
%2}
)LUA")
                           .arg(dbcFileName(), spec);

  return header + dbcLuaParserBody();
}

/**
 * @brief Emits one message's MESSAGES entry, with the DBC comment carried into the Lua.
 */
QString DataModel::DBCImporter::generateMessageSpec(const QCanMessageDescription& message) const
{
  const auto id  = static_cast<quint32>(message.uniqueId()) & 0x1FFFFFFF;
  const auto hex = QString::number(id, 16).toUpper();

  QString heading    = QStringLiteral("  -- %1 @ 0x%2").arg(message.name(), hex);
  const auto comment = message.comment().simplified();
  if (!comment.isEmpty())
    heading += QStringLiteral(": %1").arg(comment);

  const auto entries      = DBCMux::orderedSignals(message);
  const auto rootSelector = DBCMux::rootSelectorName(entries);

  QString out  = heading + QLatin1Char('\n');
  out         += QStringLiteral("  [0x%1] = {\n    table = %2,\n    signals = {\n")
           .arg(hex, luaQuote(tableNameFor(message)));

  for (const auto& entry : entries)
    out += signalSpecLine(entry, rootSelector);

  out += QStringLiteral("    },\n  },\n");
  return out;
}

/**
 * @brief Emits a spec line's mux field: the bare switch value the importer has always written
 *        for a signal gated by one point of the message's top-level multiplexor, otherwise the
 *        {p = ..., r = ...} form that carries SG_MUL_VAL_ ranges and nested switches.
 */
QString DataModel::DBCImporter::muxSpecField(const DBCMux::OrderedSignal& entry,
                                             const QString& rootSelector)
{
  qint64 value = 0;
  if (entry.role != DBCMux::MuxRole::Selector
      && DBCMux::simpleMuxValue(entry.gates, rootSelector, value))
    return QStringLiteral(", mux = %1").arg(value);

  QStringList conditions;
  for (const auto& gate : entry.gates) {
    QStringList ranges;
    for (const auto& range : gate.ranges)
      ranges.append(
        QStringLiteral("{%1, %2}").arg(QString::number(range.lo), QString::number(range.hi)));

    conditions.append(QStringLiteral("{p = %1, r = {%2}}")
                        .arg(luaQuote(gate.parent), ranges.join(QStringLiteral(", "))));
  }

  if (conditions.size() == 1)
    return QStringLiteral(", mux = %1").arg(conditions.constFirst());

  return QStringLiteral(", mux = {%1}").arg(conditions.join(QStringLiteral(", ")));
}

/**
 * @brief Emits one signal's spec line; default-valued fields are omitted so the spec stays
 *        scannable, and the DBC signal comment rides along as a Lua comment.
 */
QString DataModel::DBCImporter::signalSpecLine(const DBCMux::OrderedSignal& entry,
                                               const QString& rootSelector)
{
  const auto& signal = entry.signal;

  QString line = QStringLiteral("      { name = %1, start = %2, len = %3")
                   .arg(luaQuote(signal.name()),
                        QString::number(signal.startBit()),
                        QString::number(signal.bitLength()));

  if (signal.dataEndian() == QSysInfo::BigEndian)
    line += QStringLiteral(", be = true");

  if (signal.dataFormat() == QtCanBus::DataFormat::SignedInteger)
    line += QStringLiteral(", signed = true");

  if (std::isfinite(signal.factor()) && signal.factor() != 1.0)
    line += QStringLiteral(", factor = %1").arg(luaNumber(signal.factor()));

  if (std::isfinite(signal.offset()) && signal.offset() != 0.0)
    line += QStringLiteral(", offset = %1").arg(luaNumber(signal.offset()));

  if (entry.role == DBCMux::MuxRole::Selector)
    line += QStringLiteral(", selector = true");

  if (!entry.gates.isEmpty())
    line += muxSpecField(entry, rootSelector);

  line += QStringLiteral(" },");

  const auto comment = signal.comment().simplified();
  if (!comment.isEmpty())
    line += QStringLiteral(" -- %1").arg(comment);

  return line + QLatin1Char('\n');
}

/**
 * @brief Builds the Lua transform that maps a VAL_ enum signal's raw value to its DBC label,
 *        so the dataset shows readable state text instead of a bare number.
 */
QString DataModel::DBCImporter::enumTransformCode(
  const QString& table, const QString& reg, const QCanDbcFileParser::ValueDescriptions& valueLabels)
{
  auto keys = valueLabels.keys();
  std::sort(keys.begin(), keys.end());

  QString out = QStringLiteral("local LABELS = {\n");
  for (const auto key : keys)
    out +=
      QStringLiteral("  [%1] = %2,\n").arg(QString::number(key), luaQuote(valueLabels.value(key)));

  out += QStringLiteral("}\n\n"
                        "function transform(value)\n"
                        "  local raw = math.floor((tableGet(%1, %2) or 0) + 0.5)\n"
                        "  return LABELS[raw] or (\"Unknown (\" .. raw .. \")\")\n"
                        "end\n")
           .arg(luaQuote(table), luaQuote(reg));

  return out;
}

//--------------------------------------------------------------------------------------------------
// Signal processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if the message contributes at least one signal the import can decode.
 */
bool DataModel::DBCImporter::hasImportableSignals(const QCanMessageDescription& message) const
{
  return !DBCMux::orderedSignals(message).isEmpty();
}

//--------------------------------------------------------------------------------------------------
// Widget selection helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks the group widget: a GPS/motion summary when detected, otherwise a data grid.
 */
QString DataModel::DBCImporter::selectGroupWidget(const QCanMessageDescription& message)
{
  const auto signalDescriptions = message.signalDescriptions();

  const auto gpsId = detectGpsWidget(signalDescriptions);
  if (!gpsId.isEmpty())
    return gpsId;

  const auto motionId = detectMotionWidget(signalDescriptions);
  if (!motionId.isEmpty())
    return motionId;

  return SerialStudio::groupWidgetId(SerialStudio::DataGrid);
}

/**
 * @brief Returns the GPS widget id when the message carries lat/lon-style signals.
 */
QString DataModel::DBCImporter::detectGpsWidget(
  const QList<QCanSignalDescription>& signalDescriptions) const
{
  if (signalDescriptions.size() < 2)
    return QString();

  bool hasLat = false;
  bool hasLon = false;

  for (const auto& signal : signalDescriptions) {
    const auto name = signal.name().toLower();
    if (name.contains("lat"))
      hasLat = true;

    if (name.contains("lon") || name.contains("lng"))
      hasLon = true;
  }

  if (hasLat && hasLon)
    return SerialStudio::groupWidgetId(SerialStudio::GPS);

  return QString();
}

/**
 * @brief Returns Accelerometer/Gyroscope when the 3-signal message names match an IMU pattern.
 */
QString DataModel::DBCImporter::detectMotionWidget(
  const QList<QCanSignalDescription>& signalDescriptions) const
{
  if (signalDescriptions.size() != 3)
    return QString();

  int accelCount = 0;
  for (const auto& signal : signalDescriptions) {
    const auto name = signal.name().toLower();
    const auto unit = signal.physicalUnit().toLower();
    if (name.contains("accel") || unit.contains("m/s") || unit.contains("g"))
      accelCount++;
  }

  if (accelCount == 3)
    return SerialStudio::groupWidgetId(SerialStudio::Accelerometer);

  int gyroCount = 0;
  for (const auto& signal : signalDescriptions) {
    const auto name = signal.name().toLower();
    const auto unit = signal.physicalUnit().toLower();
    if (name.contains("gyro") || name.contains("roll") || name.contains("pitch")
        || name.contains("yaw") || unit.contains("deg/s") || unit.contains("rad/s"))
      gyroCount++;
  }

  if (gyroCount == 3)
    return SerialStudio::groupWidgetId(SerialStudio::Gyroscope);

  return QString();
}

//--------------------------------------------------------------------------------------------------
// Text utilities
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks a dataset widget (bar/gauge/meter/empty) for one CAN signal.
 */
QString DataModel::DBCImporter::selectWidgetForSignal(const QCanSignalDescription& signal)
{
  const auto name = signal.name().toLower();
  const auto unit = signal.physicalUnit().toLower().trimmed();

  if (name.contains("odometer") || name.contains("trip") || name.contains("counter")
      || name.contains("timestamp") || name.contains("status"))
    return QString("");

  if (unit == "%" || (signal.minimum() == 0 && signal.maximum() == 100))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);

  // code-verify off
  if (unit.contains("°c") || unit.contains("°f") || unit.contains("degc") || unit.contains("degf")
      || name.contains("temp") || name.contains("temperature"))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);
  // code-verify on

  static const QSet<QString> kMeterUnits = {
    "rpm", "r/min", "1/min", "km/h", "kmh", "kph", "mph", "knot", "knots", "kn", "m/s", "kw", "hp"};
  if (kMeterUnits.contains(unit) || name.contains("rpm") || name.contains("speed")
      || name.contains("velocity") || name.contains("power"))
    return SerialStudio::datasetWidgetId(SerialStudio::Meter);

  static const QSet<QString> kGaugeUnits = {"v",
                                            "mv",
                                            "kv",
                                            "a",
                                            "ma",
                                            "ka",
                                            "psi",
                                            "bar",
                                            "nm",
                                            "kpa",
                                            "pa",
                                            "hpa",
                                            "mbar",
                                            "kg",
                                            "g",
                                            "n",
                                            "deg",
                                            "rad"};
  if (kGaugeUnits.contains(unit))
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  if (unit.contains("psi") || unit.contains("kpa") || unit.contains("mbar"))
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  const auto range = signal.maximum() - signal.minimum();
  if (range > 0 && range <= 360)
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  return QString("");
}

/**
 * @brief Returns true when the signal is a continuous quantity worth a plot toggle.
 */
bool DataModel::DBCImporter::isPlottableSignal(const QCanSignalDescription& signal)
{
  if (signal.bitLength() <= 1)
    return false;

  const auto name = signal.name().toLower();
  return !name.contains("odometer") && !name.contains("trip") && !name.contains("counter")
      && !name.contains("timestamp") && !name.contains("status") && !name.contains("state");
}

//--------------------------------------------------------------------------------------------------
// Signal counting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of importable signals (excludes the ones whose multiplexing
 *        could not be resolved).
 */
int DataModel::DBCImporter::countTotalSignals(const QList<QCanMessageDescription>& messages) const
{
  int count = 0;
  for (const auto& message : messages)
    count += static_cast<int>(DBCMux::orderedSignals(message).size());

  return count;
}
