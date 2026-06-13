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
#include <QSet>
#include <QStandardPaths>

#include "DataModel/Frame.h"
#include "DataModel/Importers/AxisTicks.h"
#include "DataModel/Importers/ImporterCommon.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the DBCImporter singleton.
 */
DataModel::DBCImporter::DBCImporter() : m_skippedExtendedMuxSignals(0) {}

/**
 * @brief Returns the singleton DBCImporter instance.
 */
DataModel::DBCImporter& DataModel::DBCImporter::instance()
{
  static DBCImporter instance;
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

  const auto& message = m_messages.at(index);
  const auto msgId    = static_cast<quint32>(message.uniqueId());

  int signalCount = 0;
  for (const auto& signal : message.signalDescriptions()) {
    qint64 mv       = 0;
    const auto role = classifyMux(signal, message, mv);
    if (role != MuxRole::ExtendedMuxed)
      ++signalCount;
  }

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
    Misc::Utilities::showMessageBox(tr("Failed to parse DBC file: %1").arg(parser.errorString()),
                                    tr("Verify the file format and try again."),
                                    QMessageBox::Critical,
                                    tr("DBC Import Error"));
    return;
  }

  m_messages          = parser.messageDescriptions();
  m_valueDescriptions = parser.messageValueDescriptions();
  if (m_messages.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("DBC file contains no messages"),
      tr("The selected file does not contain any CAN message definitions."),
      QMessageBox::Warning,
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

  const auto project       = generateProject(m_messages);
  const QString suggestion = QFileInfo(m_dbcFilePath).baseName();

  const int messageCount  = m_messages.count();
  const int signalCount   = countTotalSignals(m_messages);
  const int skippedExtMux = m_skippedExtendedMuxSignals;

  auto& pm = ProjectModel::instance();
  QObject::connect(
    &pm,
    &ProjectModel::importCompleted,
    this,
    [messageCount, signalCount, skippedExtMux](bool accepted, const QString&) {
      if (!accepted)
        return;

      QString detail = tr("The project editor is now open for customization.");
      if (skippedExtMux > 0)
        detail += tr(" Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); "
                     "only simple multiplexing is supported.")
                    .arg(skippedExtMux);

      Misc::Utilities::showMessageBox(
        tr("Successfully imported DBC file with %1 messages and %2 signals.")
          .arg(messageCount)
          .arg(signalCount),
        detail,
        QMessageBox::Information,
        tr("DBC Import Complete"));
    },
    Qt::SingleShotConnection);

  pm.importProjectFromJson(project, suggestion);
}

/**
 * @brief Builds a complete .ssproj QJsonObject from the DBC messages.
 */
QJsonObject DataModel::DBCImporter::generateProject(const QList<QCanMessageDescription>& messages)
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
 * @brief Builds a Dataset for a CAN signal: a virtual dataset whose Lua transform reads the
 *        signal's physical value back from the message's data table.
 */
DataModel::Dataset DataModel::DBCImporter::buildDatasetFromSignal(
  const QCanSignalDescription& signal,
  const QString& groupWidget,
  const QString& tableName,
  const QCanDbcFileParser::ValueDescriptions& valueLabels,
  int datasetIndex,
  MuxRole role,
  qint64 muxValue)
{
  DataModel::Dataset dataset;
  dataset.index = datasetIndex;
  dataset.units = signal.physicalUnit();
  dataset.log   = true;
  dataset.fft   = false;

  if (role == MuxRole::SimpleMuxed)
    dataset.title = QString("%1 (mux %2)").arg(signal.name()).arg(muxValue);
  else if (role == MuxRole::Selector)
    dataset.title = QString("%1 (selector)").arg(signal.name());
  else
    dataset.title = signal.name();

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

    for (const auto& entry : orderedSignals(message))
      group.datasets.push_back(buildDatasetFromSignal(entry.signal,
                                                      group.widget,
                                                      tableName,
                                                      msgLabels.value(entry.signal.name()),
                                                      datasetIndex++,
                                                      entry.role,
                                                      entry.muxValue));

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
 * @brief Returns the message's importable signals in canonical order: selector first, then
 *        plain signals, then simple-muxed signals (extended-mux signals are skipped).
 */
QList<DataModel::DBCImporter::OrderedSignal> DataModel::DBCImporter::orderedSignals(
  const QCanMessageDescription& message) const
{
  QList<OrderedSignal> ordered;
  const auto signalList = message.signalDescriptions();

  const auto selectorIndex = findSelectorIndex(message);
  if (selectorIndex >= 0)
    ordered.append({signalList.at(selectorIndex), MuxRole::Selector, 0});

  for (const auto& signal : signalList) {
    qint64 muxValue = 0;
    if (classifyMux(signal, message, muxValue) == MuxRole::Plain)
      ordered.append({signal, MuxRole::Plain, muxValue});
  }

  for (const auto& signal : signalList) {
    qint64 muxValue = 0;
    if (classifyMux(signal, message, muxValue) == MuxRole::SimpleMuxed)
      ordered.append({signal, MuxRole::SimpleMuxed, muxValue});
  }

  return ordered;
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

    for (const auto& entry : orderedSignals(message)) {
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
local function extract(frame, base, sig)
  local value = 0
  if sig.be then
    local bit_pos = sig.start
    for _ = 1, sig.len do
      local byte = frame[base + (bit_pos // 8)] or 0
      value = (value << 1) | ((byte >> (bit_pos % 8)) & 1)
      bit_pos = (bit_pos % 8 == 0) and (bit_pos + 15) or (bit_pos - 1)
    end
  else
    local bits_read = 0
    local bit_shift = sig.start % 8
    local byte_idx = sig.start // 8
    while bits_read < sig.len do
      local byte = frame[base + byte_idx] or 0
      local take = math.min(8 - bit_shift, sig.len - bits_read)
      value = value | (((byte >> bit_shift) & ((1 << take) - 1)) << bits_read)
      bits_read = bits_read + take
      bit_shift = 0
      byte_idx = byte_idx + 1
    end
  end

  if sig.signed and (value & (1 << (sig.len - 1))) ~= 0 then
    value = value - (1 << sig.len)
  end

  return value
end

-- Returns the CAN id and the 1-based index of the first payload byte: bit 7
-- of byte 1 selects the extended (29-bit, 4-byte id) header form.
local function frame_id(frame)
  local b1 = frame[1]
  if (b1 & 0x80) ~= 0 then
    if #frame < 5 then
      return nil
    end

    local id = ((b1 & 0x1F) << 24) | (frame[2] << 16)
             | (frame[3] << 8) | frame[4]
    return id, 6
  end

  return ((b1 << 8) | frame[2]), 4
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

  local selector = nil
  for _, sig in ipairs(msg.signals) do
    if sig.mux == nil or sig.mux == selector then
      local raw = extract(frame, base, sig)
      if sig.selector then
        selector = raw
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
 *        MESSAGES spec, and the generic extract()/parse() machinery. Also counts the
 *        extended-mux signals that the import skips.
 */
QString DataModel::DBCImporter::generateLuaParser(const QList<QCanMessageDescription>& messages)
{
  m_skippedExtendedMuxSignals = 0;
  for (const auto& message : messages) {
    for (const auto& signal : message.signalDescriptions()) {
      qint64 mv = 0;
      if (classifyMux(signal, message, mv) == MuxRole::ExtendedMuxed)
        ++m_skippedExtendedMuxSignals;
    }
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
local MESSAGES = {
%2}
)LUA")
                           .arg(dbcFileName(), spec);

  return header + dbcLuaParserBody();
}

/**
 * @brief Emits one message's MESSAGES entry, with the DBC comment carried into the Lua.
 */
QString DataModel::DBCImporter::generateMessageSpec(const QCanMessageDescription& message)
{
  const auto id  = static_cast<quint32>(message.uniqueId()) & 0x1FFFFFFF;
  const auto hex = QString::number(id, 16).toUpper();

  QString heading    = QStringLiteral("  -- %1 @ 0x%2").arg(message.name(), hex);
  const auto comment = message.comment().simplified();
  if (!comment.isEmpty())
    heading += QStringLiteral(": %1").arg(comment);

  QString out  = heading + QLatin1Char('\n');
  out         += QStringLiteral("  [0x%1] = {\n    table = %2,\n    signals = {\n")
           .arg(hex, luaQuote(tableNameFor(message)));

  for (const auto& entry : orderedSignals(message))
    out += signalSpecLine(entry.signal, entry.role, entry.muxValue);

  out += QStringLiteral("    },\n  },\n");
  return out;
}

/**
 * @brief Emits one signal's spec line; default-valued fields are omitted so the spec stays
 *        scannable, and the DBC signal comment rides along as a Lua comment.
 */
QString DataModel::DBCImporter::signalSpecLine(const QCanSignalDescription& signal,
                                               MuxRole role,
                                               qint64 muxValue) const
{
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

  if (role == MuxRole::Selector)
    line += QStringLiteral(", selector = true");
  else if (role == MuxRole::SimpleMuxed)
    line += QStringLiteral(", mux = %1").arg(muxValue);

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
 * @brief Returns true if the message contributes at least one non-extended-mux signal.
 */
bool DataModel::DBCImporter::hasImportableSignals(const QCanMessageDescription& message) const
{
  for (const auto& signal : message.signalDescriptions()) {
    qint64 mv       = 0;
    const auto role = classifyMux(signal, message, mv);
    if (role != MuxRole::ExtendedMuxed)
      return true;
  }

  return false;
}

/**
 * @brief Returns the index of the message's top-level MultiplexorSwitch signal, or -1.
 */
int DataModel::DBCImporter::findSelectorIndex(const QCanMessageDescription& message) const
{
  const auto signalList = message.signalDescriptions();
  for (qsizetype i = 0; i < signalList.size(); ++i)
    if (signalList.at(i).multiplexState() == QtCanBus::MultiplexState::MultiplexorSwitch)
      return static_cast<int>(i);

  return -1;
}

/**
 * @brief Classifies a signal's multiplex role and extracts the mux value when simple.
 */
DataModel::DBCImporter::MuxRole DataModel::DBCImporter::classifyMux(
  const QCanSignalDescription& signal,
  const QCanMessageDescription& message,
  qint64& outMuxValue) const
{
  outMuxValue = 0;

  const auto state = signal.multiplexState();
  if (state == QtCanBus::MultiplexState::None)
    return MuxRole::Plain;

  if (state == QtCanBus::MultiplexState::MultiplexorSwitch)
    return MuxRole::Selector;

  if (state == QtCanBus::MultiplexState::SwitchAndSignal)
    return MuxRole::ExtendedMuxed;

  const auto parents = signal.multiplexSignals();
  if (parents.isEmpty())
    return MuxRole::Plain;

  if (parents.size() > 1)
    return MuxRole::ExtendedMuxed;

  const auto parentName    = parents.keys().first();
  const auto selectorIndex = findSelectorIndex(message);
  if (selectorIndex < 0)
    return MuxRole::ExtendedMuxed;

  const auto signalList = message.signalDescriptions();
  if (signalList.at(selectorIndex).name() != parentName)
    return MuxRole::ExtendedMuxed;

  const auto ranges = parents.value(parentName);
  if (ranges.size() != 1)
    return MuxRole::ExtendedMuxed;

  const auto& range = ranges.first();
  if (range.minimum != range.maximum)
    return MuxRole::ExtendedMuxed;

  bool ok      = false;
  const auto v = range.minimum.toLongLong(&ok);
  if (!ok)
    return MuxRole::ExtendedMuxed;

  outMuxValue = v;
  return MuxRole::SimpleMuxed;
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
 * @brief Returns the total number of importable signals (excludes extended-mux signals).
 */
int DataModel::DBCImporter::countTotalSignals(const QList<QCanMessageDescription>& messages) const
{
  int count = 0;
  for (const auto& message : messages) {
    for (const auto& signal : message.signalDescriptions()) {
      qint64 mv       = 0;
      const auto role = classifyMux(signal, message, mv);
      if (role != MuxRole::ExtendedMuxed)
        ++count;
    }
  }

  return count;
}
