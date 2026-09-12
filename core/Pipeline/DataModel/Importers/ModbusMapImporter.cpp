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

#include "DataModel/Importers/ModbusMapImporter.h"

#include <algorithm>
#include <cmath>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QSet>
#include <QStandardPaths>
#include <utility>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/DataModel/Frame.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "DataModel/Importers/AxisTicks.h"
#include "DataModel/Importers/ImporterCommon.h"
#include "DataModel/Importers/ModbusRegisterMap.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ModbusMapImporter singleton.
 */
DataModel::ModbusMapImporter::ModbusMapImporter() {}

/**
 * @brief Returns the singleton ModbusMapImporter instance.
 */
DataModel::ModbusMapImporter& DataModel::ModbusMapImporter::instance()
{
  static ModbusMapImporter inst;
  return inst;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of imported registers.
 */
int DataModel::ModbusMapImporter::registerCount() const noexcept
{
  return m_registers.count();
}

/**
 * @brief Returns the number of contiguous register blocks the import would yield.
 */
int DataModel::ModbusMapImporter::groupCount() const noexcept
{
  return computeBlocks().count();
}

/**
 * @brief Returns the file name of the loaded register map.
 */
QString DataModel::ModbusMapImporter::fileName() const
{
  return QFileInfo(m_filePath).fileName();
}

/**
 * @brief Returns "N: Name @ Address (Type, DataType) [Units]" for the register.
 */
QString DataModel::ModbusMapImporter::registerInfo(int index) const
{
  if (index < 0 || index >= m_registers.count())
    return QString();

  const auto& r = m_registers[index];
  QString info  = QStringLiteral("%1: %2 @ %3 (%4, %5)")
                    .arg(index + 1)
                    .arg(r.name)
                    .arg(r.address)
                    .arg(registerTypeName(r.registerType))
                    .arg(r.dataType);

  if (!r.units.isEmpty())
    info += QStringLiteral(" [%1]").arg(r.units);

  if (r.unitId != 0)
    info += QStringLiteral(" unit %1").arg(r.unitId);

  if (r.bitIndex >= 0)
    info += QStringLiteral(" bit %1").arg(r.bitIndex);

  if (r.writable)
    info += QStringLiteral(" rw");

  return info;
}

//--------------------------------------------------------------------------------------------------
// Import workflow
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a file dialog to select a CSV/XML/JSON register map.
 */
void DataModel::ModbusMapImporter::importRegisterMap()
{
  const auto home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

  auto* dialog = new QFileDialog(qApp->activeWindow(), tr("Import Modbus Register Map"), home);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setNameFilter(
    tr("Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files "
       "(*.xml);;JSON Files (*.json);;All Files (*)"));

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { showPreview(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Parses the selected file and emits previewReady on success.
 */
void DataModel::ModbusMapImporter::showPreview(const QString& filePath)
{
  if (filePath.isEmpty())
    return;

  m_registers.clear();

  const auto ext = QFileInfo(filePath).suffix().toLower();
  bool ok        = false;

  if (ext == QLatin1String("csv"))
    ok = ModbusMap::parseCsv(filePath, m_registers);
  else if (ext == QLatin1String("xml"))
    ok = ModbusMap::parseXml(filePath, m_registers);
  else if (ext == QLatin1String("json"))
    ok = ModbusMap::parseJson(filePath, m_registers);
  else
    ok = ModbusMap::parseCsv(filePath, m_registers) || ModbusMap::parseJson(filePath, m_registers)
      || ModbusMap::parseXml(filePath, m_registers);

  if (!ok || m_registers.isEmpty()) {
    Core::Prompt::showMessageBox(
      tr("No registers found"),
      tr("The file could not be parsed or contains no register definitions."),
      Core::Prompt::Warning,
      tr("Modbus Import"));
    return;
  }

  std::sort(m_registers.begin(),
            m_registers.end(),
            [](const ModbusMap::RegisterEntry& a, const ModbusMap::RegisterEntry& b) {
              if (a.unitId != b.unitId)
                return a.unitId < b.unitId;

              if (a.registerType != b.registerType)
                return a.registerType < b.registerType;

              return a.address < b.address;
            });

  m_filePath = filePath;
  Q_EMIT registersChanged();
  Q_EMIT fileNameChanged();
  Q_EMIT previewReady();
}

/**
 * @brief Generates a project and loads register groups into the Modbus driver.
 */
void DataModel::ModbusMapImporter::confirmImport()
{
  if (m_registers.isEmpty())
    return;

  const auto blocks        = computeBlocks();
  const auto project       = buildProject();
  const QString suggestion = QFileInfo(m_filePath).baseName();

  loadRegisterGroups(blocks, false);

  const int registerCount = m_registers.count();
  const int blockCount    = blocks.count();

  static auto& pm = ProjectModel::instance();
  QObject::connect(
    &pm,
    &ProjectModel::importCompleted,
    this,
    [registerCount, blockCount](bool accepted, const QString&) {
      if (!accepted)
        return;

      Core::Prompt::showMessageBox(
        tr("Successfully imported %1 registers in %2 groups.")
          .arg(QString::number(registerCount), QString::number(blockCount)),
        tr("The project editor is now open for customization."),
        Core::Prompt::Information,
        tr("Modbus Import Complete"));
    },
    Qt::SingleShotConnection);

  pm.importProjectFromJson(project, suggestion);
}

/**
 * @brief Adds the generated source, groups, tables and workspaces to the open project (spec 0083)
 *        and appends the polled blocks to the Modbus driver's groups.
 */
void DataModel::ModbusMapImporter::confirmMerge()
{
  if (m_registers.isEmpty())
    return;

  const auto blocks   = computeBlocks();
  const auto project  = buildProject();
  const QString label = QFileInfo(m_filePath).baseName();
  if (!DataModel::pipelineModules().projectModel.mergeImportedProject(project, label))
    return;

  loadRegisterGroups(blocks, true);
  Core::Prompt::showMessageBox(
    tr("Added %1 registers in %2 groups to the open project.")
      .arg(QString::number(m_registers.count()), QString::number(blocks.count())),
    tr("The new source, groups, tables and workspaces are filed under \"%1\".").arg(label),
    Core::Prompt::Information,
    tr("Modbus Import Complete"));
}

/**
 * @brief Clears the import state.
 */
void DataModel::ModbusMapImporter::cancelImport()
{
  m_registers.clear();
  m_filePath.clear();
  Q_EMIT registersChanged();
  Q_EMIT fileNameChanged();
}

//--------------------------------------------------------------------------------------------------
// Register block grouping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the register count for an entry at @p address clamped so a block can never
 *        run past the 65535 end of the Modbus register space.
 */
static quint16 clampedRegisterSpan(quint16 address, int registers)
{
  const int available = 0x10000 - address;
  return static_cast<quint16>(qBound(0, registers, available));
}

/**
 * @brief Groups registers into contiguous blocks of the same unit and type. End addresses are
 *        computed in int and clamped: a quint16 sum wraps for maps that pair high addresses with
 *        multi-register data types and silently corrupts the block counts.
 */
QVector<DataModel::ModbusMapImporter::RegisterBlock> DataModel::ModbusMapImporter::computeBlocks()
  const
{
  if (m_registers.isEmpty())
    return {};

  QVector<RegisterBlock> blocks;
  RegisterBlock current;
  const int firstSpan  = ModbusMap::registersForDataType(m_registers[0].dataType);
  current.registerType = m_registers[0].registerType;
  current.startAddress = m_registers[0].address;
  current.count        = clampedRegisterSpan(current.startAddress, firstSpan);
  current.slaveAddress = m_registers[0].unitId;
  current.entries.append(m_registers[0]);

  for (int i = 1; i < m_registers.count(); ++i) {
    const auto& entry = m_registers[i];
    const int endAddr = static_cast<int>(current.startAddress) + current.count;
    const bool sameBlock =
      entry.registerType == current.registerType && entry.unitId == current.slaveAddress;

    if (sameBlock && entry.address <= endAddr) {
      const int entryEnd =
        static_cast<int>(entry.address) + ModbusMap::registersForDataType(entry.dataType);
      const int span = qMin(qMin(entryEnd, 0x10000) - current.startAddress, 0xFFFF);
      current.count  = static_cast<quint16>(qMax<int>(current.count, span));
      current.entries.append(entry);
    } else {
      blocks.append(current);
      current.registerType = entry.registerType;
      current.startAddress = entry.address;
      current.count =
        clampedRegisterSpan(entry.address, ModbusMap::registersForDataType(entry.dataType));
      current.slaveAddress = entry.unitId;
      current.entries.clear();
      current.entries.append(entry);
    }
  }

  blocks.append(current);
  return blocks;
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a complete .ssproj JSON from the parsed registers.
 */
QJsonObject DataModel::ModbusMapImporter::buildProject() const
{
  const auto blocks     = computeBlocks();
  const bool multi_unit = spansSeveralUnits(blocks);

  QStringList table_names;
  QList<QStringList> register_names;
  for (const auto& block : blocks) {
    table_names.append(blockTitle(block, blocks.size(), multi_unit));
    register_names.append(blockRegisterNames(block));
  }

  QJsonObject project;
  project[Keys::Title]   = QFileInfo(m_filePath).baseName();
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("Modbus");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::ModBus);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = buildLuaParser(blocks, table_names, register_names);
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Lua);

  QJsonArray reg_groups;
  for (const auto& block : blocks) {
    QJsonObject obj;
    obj[QStringLiteral("type")]  = block.registerType;
    obj[QStringLiteral("start")] = block.startAddress;
    obj[QStringLiteral("count")] = block.count;
    if (block.slaveAddress != 0)
      obj[QStringLiteral("slave")] = block.slaveAddress;

    reg_groups.append(obj);
  }

  QJsonObject conn_settings;
  conn_settings[QStringLiteral("registerGroups")] = reg_groups;
  source[Keys::SourceConn]                        = conn_settings;

  project[Keys::Sources] = QJsonArray{source};

  std::vector<DataModel::Group> groups;
  std::vector<DataModel::TableDef> tables;
  int dataset_index = 1;

  for (qsizetype b = 0; b < blocks.size(); ++b) {
    const auto& block  = blocks.at(b);
    const bool is_bool = (block.registerType >= 2);

    DataModel::Group group;
    group.groupId = static_cast<int>(b);
    group.widget  = QStringLiteral("datagrid");
    group.title   = table_names.at(b);

    DataModel::TableDef table;
    table.name = table_names.at(b);

    for (qsizetype e = 0; e < block.entries.size(); ++e) {
      const auto reg_name = register_names.at(b).at(e);

      DataModel::RegisterDef reg;
      reg.name         = reg_name;
      reg.type         = DataModel::RegisterType::Computed;
      reg.defaultValue = QVariant(0.0);
      table.registers.push_back(reg);

      const auto& entry = block.entries.at(e);
      if (entry.writable && entry.bitIndex >= 0 && !is_bool) {
        DataModel::RegisterDef word;
        word.name         = rawWordRegisterName(reg_name);
        word.type         = DataModel::RegisterType::Computed;
        word.defaultValue = QVariant(0.0);
        table.registers.push_back(word);
      }

      group.datasets.push_back(
        buildDatasetFromEntry(entry, is_bool, table.name, reg_name, dataset_index++));
    }

    groups.push_back(group);
    tables.push_back(table);
  }

  auto controls =
    buildOutputGroup(blocks, table_names, register_names, static_cast<int>(groups.size()));
  if (!controls.outputWidgets.empty())
    groups.push_back(std::move(controls));

  finalizeImportedProject(project, groups, tables, tr("Overview"));
  return project;
}

/**
 * @brief Builds the Controls panel (spec 0083): one output control per writable row, writing the
 *        row's register on the row's unit and reading its state back from the block's table.
 */
DataModel::Group DataModel::ModbusMapImporter::buildOutputGroup(
  const QVector<RegisterBlock>& blocks,
  const QStringList& tableNames,
  const QList<QStringList>& registerNames,
  int groupIndex) const
{
  DataModel::Group group;
  group.groupId   = groupIndex;
  group.groupType = DataModel::GroupType::Output;
  group.title     = tr("%1 Controls").arg(QFileInfo(m_filePath).baseName());

  for (qsizetype b = 0; b < blocks.size(); ++b) {
    const auto& block = blocks.at(b);
    for (qsizetype e = 0; e < block.entries.size(); ++e) {
      const auto& entry = block.entries.at(e);
      if (!entry.writable)
        continue;

      group.outputWidgets.push_back(buildControl(block,
                                                 entry,
                                                 tableNames.at(b),
                                                 registerNames.at(b).at(e),
                                                 static_cast<int>(group.outputWidgets.size())));
    }
  }

  return group;
}

/**
 * @brief One control for a writable row: a toggle for coils and bit rows, a slider over the row's
 *        engineering range otherwise. Latin-1 transmit encoding keeps the helper's byte string
 *        one byte per character; the state binding follows the table register the parser fills.
 */
DataModel::OutputWidget DataModel::ModbusMapImporter::buildControl(
  const RegisterBlock& block,
  const ModbusMap::RegisterEntry& entry,
  const QString& tableName,
  const QString& regName,
  int widgetId)
{
  const bool is_bit = block.registerType >= 2 || entry.bitIndex >= 0;

  DataModel::OutputWidget control;
  control.widgetId   = widgetId;
  control.groupId    = -1;
  control.sourceId   = 0;
  control.txEncoding = static_cast<int>(SerialStudio::EncLatin1);
  control.title      = entry.name;
  control.type       = is_bit ? OutputWidgetType::Toggle : OutputWidgetType::Slider;
  control.minValue   = is_bit ? 0.0 : entry.min;
  control.maxValue   = is_bit ? 1.0 : entry.max;
  control.stepSize = (!is_bit && std::isfinite(entry.scale) && entry.scale > 0) ? entry.scale : 1.0;
  control.initialValue = control.minValue;
  control.onLabel      = is_bit ? tr("On") : QString();
  control.offLabel     = is_bit ? tr("Off") : QString();

  control.stateSource      = OutputStateSource::Table;
  control.stateTable       = tableName;
  control.stateVariable    = regName;
  control.transmitFunction = controlTransmitScript(block, entry, tableName, regName);
  normalize(control);
  return control;
}

/**
 * @brief The JavaScript a generated control transmits with: engineering value back to raw words,
 *        the row's unit as the helper's trailing argument, and a read-modify-write of the raw word
 *        for a bit row so its neighbours stay untouched. Names are quoted with luaQuote, whose
 *        three escapes are the same in a JavaScript string literal.
 */
QString DataModel::ModbusMapImporter::controlTransmitScript(const RegisterBlock& block,
                                                            const ModbusMap::RegisterEntry& entry,
                                                            const QString& tableName,
                                                            const QString& regName)
{
  const QString unit    = entry.unitId != 0 ? QStringLiteral(", %1").arg(entry.unitId) : QString();
  const QString address = QStringLiteral("0x%1").arg(entry.address, 4, 16, QLatin1Char('0'));

  if (block.registerType >= 2)
    return QStringLiteral(
             "function transmit(value) {\n  return modbusWriteCoil(%1, !!value%2);\n}\n")
      .arg(address, unit);

  if (entry.bitIndex >= 0)
    return QStringLiteral("var WORD_REGISTER = %1;\n"
                          "var BIT = %2;\n\n"
                          "function transmit(value) {\n"
                          "  var word = tableGet(%3, WORD_REGISTER) | 0;\n"
                          "  word = value ? (word | (1 << BIT)) : (word & ~(1 << BIT));\n"
                          "  return modbusWriteRegister(%4, word & 0xFFFF%5);\n"
                          "}\n")
      .arg(luaQuote(rawWordRegisterName(regName)),
           QString::number(entry.bitIndex),
           luaQuote(tableName),
           address,
           unit);

  const int words = ModbusMap::registersForDataType(entry.dataType);
  const QString to_raw =
    QStringLiteral("Math.round((value - %1) / %2)")
      .arg(luaNumber(std::isfinite(entry.offset) ? entry.offset : 0.0),
           luaNumber(std::isfinite(entry.scale) && entry.scale != 0.0 ? entry.scale : 1.0));
  if (entry.dataType == QLatin1String("float32") && entry.wordOrder.isEmpty())
    return QStringLiteral(
             "function transmit(value) {\n  return modbusWriteFloat(%1, value%2);\n}\n")
      .arg(address, unit);

  if (words == 1)
    return QStringLiteral(
             "function transmit(value) {\n  return modbusWriteRegister(%1, %2%3);\n}\n")
      .arg(address, to_raw, unit);

  const QString swap =
    entry.wordOrder == QLatin1String("cdab") ? QStringLiteral("  words.reverse();\n") : QString();
  return QStringLiteral("var WORDS = %1;\n\n"
                        "function transmit(value) {\n"
                        "  var raw = %2;\n"
                        "  var words = [];\n"
                        "  for (var i = WORDS - 1; i >= 0; i--) {\n"
                        "    words[i] = raw % 65536;\n"
                        "    raw = Math.floor(raw / 65536);\n"
                        "  }\n"
                        "%3"
                        "  return modbusWriteRegisters(%4, words%5);\n"
                        "}\n")
    .arg(QString::number(words), to_raw, swap, address, unit);
}

/**
 * @brief Builds a Dataset for one register entry: a computed dataset whose Lua transform reads
 *        the value back from the block's data table.
 */
DataModel::Dataset DataModel::ModbusMapImporter::buildDatasetFromEntry(
  const ModbusMap::RegisterEntry& entry,
  bool isBool,
  const QString& tableName,
  const QString& regName,
  int datasetIndex) const
{
  DataModel::Dataset dataset;
  dataset.index = datasetIndex;
  dataset.title = entry.name;
  dataset.units = entry.units;
  dataset.log   = true;
  applyTableTransform(dataset, tableName, regName);

  if (isBool || entry.dataType == QLatin1String("bool") || entry.bitIndex >= 0) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.wgtMin  = 0;
    dataset.wgtMax  = 1;
    applyBooleanLedBand(dataset, tr("On"));
    return dataset;
  }

  dataset.pltMin = entry.min;
  dataset.pltMax = entry.max;
  dataset.widget = selectDatasetWidget(entry);
  dataset.plt    = true;

  const auto nice          = niceAxisTicks(entry.min, entry.max);
  dataset.wgtMin           = nice.min;
  dataset.wgtMax           = nice.max;
  dataset.displayTickCount = nice.tickCount;

  const bool is_float   = entry.dataType.startsWith(QLatin1String("float"));
  const int decimals    = qMax(fractionalDecimals(entry.scale), is_float ? 2 : 0);
  dataset.displayFormat = QStringLiteral("%1d").arg(decimals);

  if (!dataset.widget.isEmpty())
    applyAnalogDisplayPolicy(dataset);

  return dataset;
}

/**
 * @brief Returns the block's display title, doubling as its data-table name; a map that polls
 *        more than one unit prefixes the unit so two devices' blocks stay apart.
 */
QString DataModel::ModbusMapImporter::blockTitle(const RegisterBlock& block,
                                                 qsizetype blockCount,
                                                 bool multiUnit)
{
  const QString unit =
    multiUnit ? QStringLiteral("Unit %1 ").arg(qMax<int>(1, block.slaveAddress)) : QString();

  if (blockCount == 1)
    return unit + registerTypeName(block.registerType);

  return QStringLiteral("%1%2 @ %3")
    .arg(unit, registerTypeName(block.registerType), QString::number(block.startAddress));
}

/**
 * @brief True when the blocks name more than one unit (the connection's own unit counts as one).
 */
bool DataModel::ModbusMapImporter::spansSeveralUnits(const QVector<RegisterBlock>& blocks)
{
  if (blocks.isEmpty())
    return false;

  const quint8 first = blocks.first().slaveAddress;
  for (const auto& block : blocks)
    if (block.slaveAddress != first)
      return true;

  return false;
}

/**
 * @brief Returns one collision-free register name per block entry (the entry name, with the
 *        register address appended when a map reuses a name).
 */
QStringList DataModel::ModbusMapImporter::blockRegisterNames(const RegisterBlock& block)
{
  QStringList names;
  QSet<QString> used;

  for (const auto& entry : block.entries) {
    QString name = entry.name.simplified();
    if (name.isEmpty())
      name = QStringLiteral("Register %1").arg(entry.address);

    if (used.contains(name))
      name += QStringLiteral(" @ %1").arg(entry.address);

    used.insert(name);
    names.append(name);
  }

  return names;
}

/**
 * @brief Maps an entry to its Lua spec decode type: coil/discrete blocks always decode as
 *        packed bits; a bit row on a register block reads one bit of the word ("rbit"); a bool
 *        on a register block decodes the whole 16-bit word.
 */
QString DataModel::ModbusMapImporter::luaEntryType(const ModbusMap::RegisterEntry& entry,
                                                   bool bitBlock)
{
  if (bitBlock)
    return QStringLiteral("bit");

  if (entry.bitIndex >= 0)
    return QStringLiteral("rbit");

  static const QSet<QString> kKnownTypes = {QStringLiteral("uint16"),
                                            QStringLiteral("int16"),
                                            QStringLiteral("uint32"),
                                            QStringLiteral("int32"),
                                            QStringLiteral("uint64"),
                                            QStringLiteral("int64"),
                                            QStringLiteral("float32"),
                                            QStringLiteral("float64"),
                                            QStringLiteral("bool")};

  if (kKnownTypes.contains(entry.dataType))
    return entry.dataType;

  return QStringLiteral("uint16");
}

// code-verify off
/**
 * @brief Returns the static FORMATS/decode/parse machinery appended below the BLOCKS spec.
 *        Fenced: the linter's C++ parser misreads the embedded Lua literal.
 */
[[nodiscard]] static QString modbusLuaParserBody()
{
  return QStringLiteral(R"LUA(
-- Byte widths per entry type: Modbus payloads are big-endian. Decoding is pure
-- arithmetic plus 8-bit-safe bit ops (LuaJIT has no string.unpack); 64-bit
-- integers stay exact to 53 bits, the pipeline's double-precision ceiling.
local SIZES = {
  uint16 = 2, int16 = 2,
  uint32 = 4, int32 = 4,
  uint64 = 8, int64 = 8,
  float32 = 4, float64 = 8,
  bool = 2,
}

-- Byte permutations for multi-register values; the wire order is ABCD.
local ORDERS = {
  cdab = { [4] = { 3, 4, 1, 2 }, [8] = { 7, 8, 5, 6, 3, 4, 1, 2 } },
  badc = { [4] = { 2, 1, 4, 3 }, [8] = { 2, 1, 4, 3, 6, 5, 8, 7 } },
  dcba = { [4] = { 4, 3, 2, 1 }, [8] = { 8, 7, 6, 5, 4, 3, 2, 1 } },
}

-- Returns the entry's bytes in big-endian order after undoing its word order.
local function ordered_bytes(frame, first, size, order)
  local perm = order and ORDERS[order] and ORDERS[order][size]
  local bytes = {}
  for i = 1, size do
    local src = perm and perm[i] or i
    bytes[i] = string.byte(frame, first + src - 1)
  end
  return bytes
end

local function read_uint(bytes, n)
  local v = 0
  for i = 1, n do
    v = v * 256 + bytes[i]
  end
  return v
end

local function to_signed(v, n)
  if v >= 2 ^ (8 * n - 1) then
    v = v - 2 ^ (8 * n)
  end
  return v
end

local function read_float32(b)
  local sign = bit.band(b[1], 0x80) ~= 0 and -1 or 1
  local expo = bit.band(b[1], 0x7F) * 2 + bit.rshift(b[2], 7)
  local mant = bit.band(b[2], 0x7F) * 65536 + b[3] * 256 + b[4]
  if expo == 0 then
    return sign * mant * 2 ^ (-126 - 23)
  elseif expo == 255 then
    if mant == 0 then return sign * math.huge end
    return 0 / 0
  end
  return sign * (1 + mant / 2 ^ 23) * 2 ^ (expo - 127)
end

local function read_float64(b)
  local sign = bit.band(b[1], 0x80) ~= 0 and -1 or 1
  local expo = bit.band(b[1], 0x7F) * 16 + bit.rshift(b[2], 4)
  local mant = bit.band(b[2], 0x0F)
  for i = 3, 8 do
    mant = mant * 256 + b[i]
  end
  if expo == 0 then
    return sign * mant * 2 ^ (-1022 - 52)
  elseif expo == 2047 then
    if mant == 0 then return sign * math.huge end
    return 0 / 0
  end
  return sign * (1 + mant / 2 ^ 52) * 2 ^ (expo - 1023)
end

-- Decodes one entry; "bit" reads an LSB-first packed coil/discrete bit,
-- "rbit" reads one bit of a register word (and publishes the word too, so
-- a control can write it back), "bool" reads a whole register as 0/1.
local function decode(frame, limit, entry)
  if entry.type == "bit" then
    local byte_idx = 4 + math.floor(entry.offset / 8)
    if byte_idx > limit then
      return nil
    end

    return bit.band(bit.rshift(string.byte(frame, byte_idx), entry.offset % 8), 1)
  end

  local size = entry.width or SIZES[entry.type] or 2
  local first = 4 + entry.offset * 2
  if first + size - 1 > limit then
    return nil
  end

  local b = ordered_bytes(frame, first, size, entry.order)
  local raw
  if entry.type == "float32" then
    raw = read_float32(b)
  elseif entry.type == "float64" then
    raw = read_float64(b)
  elseif entry.type == "int16" or entry.type == "int32" or entry.type == "int64" then
    raw = to_signed(read_uint(b, size), size)
  else
    raw = read_uint(b, size)
  end

  if entry.type == "bool" then
    return (raw ~= 0) and 1 or 0
  end

  if entry.type == "rbit" then
    if entry.word then
      tableSet(entry.table, entry.word, raw)
    end
    return math.floor(raw / 2 ^ entry.bit) % 2
  end

  return raw * (entry.scale or 1) + (entry.shift or 0)
end

local cursor = 1

-- Picks the block a reply belongs to from what the reply itself carries: the
-- responding unit, the function code and the byte count. Only when several
-- blocks share that signature does the driver's round-robin order decide.
local function match_block(unit, func, count)
  local first = nil
  local candidates = 0
  for i = 1, #BLOCKS do
    local b = BLOCKS[i]
    if b.func == func and b.bytes == count and (b.unit == 0 or b.unit == unit) then
      candidates = candidates + 1
      first = first or i
    end
  end

  if candidates <= 1 then
    return first
  end

  for probe = 0, #BLOCKS - 1 do
    local i = ((cursor - 1 + probe) % #BLOCKS) + 1
    local b = BLOCKS[i]
    if b.func == func and b.bytes == count and (b.unit == 0 or b.unit == unit) then
      return i
    end
  end

  return nil
end

-- The Binary decoder hands parse() the frame as a 1-indexed table of byte
-- values; string.byte needs a string, so convert once up front (Modbus
-- ADUs are at most 256 bytes).
function parse(frame)
  if type(frame) == "table" then
    frame = string.char(table.unpack(frame))
  end

  if #frame < 3 then
    return {}
  end

  local unit = string.byte(frame, 1)
  local func = string.byte(frame, 2)
  if func >= 0x80 then
    return {} -- Modbus exception response
  end

  local count = string.byte(frame, 3)
  local index = match_block(unit, func, count)
  if not index then
    return {}
  end

  cursor = (index % #BLOCKS) + 1
  local block = BLOCKS[index]
  local limit = math.min(3 + count, #frame)
  for _, entry in ipairs(block.entries) do
    local value = decode(frame, limit, entry)
    if value ~= nil then
      tableSet(block.table, entry.name, value)
    end
  end

  return { 0 } -- values flow through tableSet; datasets read them back
end
)LUA");
}

// code-verify on

/**
 * @brief Returns one BLOCKS entry line: name, word offset, decode type, then only the fields that
 *        differ from the defaults (scale, shift, word order, the rbit bit index and width, and the
 *        raw-word register a writable bit row publishes for its control).
 */
QString DataModel::ModbusMapImporter::luaEntryLine(const RegisterBlock& block,
                                                   const QString& registerName,
                                                   const QString& tableName,
                                                   qsizetype index)
{
  const auto& entry  = block.entries.at(index);
  const bool is_bits = (block.registerType >= 2);
  const auto type    = luaEntryType(entry, is_bits);

  QString line = QStringLiteral("      { name = %1, offset = %2, type = %3")
                   .arg(luaQuote(registerName),
                        QString::number(entry.address - block.startAddress),
                        luaQuote(type));

  if (type == QLatin1String("rbit")) {
    line += QStringLiteral(", bit = %1, width = %2")
              .arg(QString::number(entry.bitIndex),
                   QString::number(2 * ModbusMap::registersForDataType(entry.dataType)));
    if (entry.writable)
      line += QStringLiteral(", table = %1, word = %2")
                .arg(luaQuote(tableName), luaQuote(rawWordRegisterName(registerName)));
  }

  if (!entry.wordOrder.isEmpty() && ModbusMap::registersForDataType(entry.dataType) > 1)
    line += QStringLiteral(", order = %1").arg(luaQuote(entry.wordOrder));

  if (std::isfinite(entry.scale) && entry.scale != 1.0)
    line += QStringLiteral(", scale = %1").arg(luaNumber(entry.scale));

  if (std::isfinite(entry.offset) && entry.offset != 0.0)
    line += QStringLiteral(", shift = %1").arg(luaNumber(entry.offset));

  return line + QStringLiteral(" },\n");
}

/**
 * @brief The table register a writable bit row's raw word is published under, so its control can
 *        read-modify-write the word without disturbing the neighbouring bits.
 */
QString DataModel::ModbusMapImporter::rawWordRegisterName(const QString& registerName)
{
  return registerName + QStringLiteral(" (word)");
}

/**
 * @brief Generates the user-editable Lua frame parser: a documented header, the declarative
 *        BLOCKS spec, and the reply-matching decode machinery.
 */
QString DataModel::ModbusMapImporter::buildLuaParser(const QVector<RegisterBlock>& blocks,
                                                     const QStringList& tableNames,
                                                     const QList<QStringList>& registerNames) const
{
  static const quint8 kFunctionCodes[4] = {0x03, 0x04, 0x01, 0x02};

  QString spec;
  for (qsizetype b = 0; b < blocks.size(); ++b) {
    const auto& block  = blocks.at(b);
    const bool is_bits = (block.registerType >= 2);
    const auto func    = kFunctionCodes[qBound<quint8>(0, block.registerType, 3)];
    const int bytes    = is_bits ? (block.count + 7) / 8 : block.count * 2;

    spec += QStringLiteral("  {\n    func = 0x%1,\n    unit = %2,\n    bytes = %3,\n"
                           "    table = %4,\n    entries = {\n")
              .arg(QString::number(func, 16).toUpper().rightJustified(2, QLatin1Char('0')),
                   QString::number(block.slaveAddress),
                   QString::number(bytes),
                   luaQuote(tableNames.at(b)));

    for (qsizetype e = 0; e < block.entries.size(); ++e)
      spec += luaEntryLine(block, registerNames.at(b).at(e), tableNames.at(b), e);

    spec += QStringLiteral("    },\n  },\n");
  }

  const QString header = QStringLiteral(R"LUA(--[[
  Modbus register-map parser generated by the Serial Studio Modbus importer.
  Source map: %1

  Wire format (response ADU): [unit, function, byteCount, data...]

  The Modbus driver polls the configured register blocks round-robin, each
  from the unit its map named. parse() matches a reply to its block by the
  unit, function code and byte count the reply itself carries, so a dropped
  reply never shifts the ones after it; only blocks that share all three
  fall back to the driver's polling order.

  Data flow:
    parse() decodes every entry of the matched block and publishes the
    engineering value with tableSet("<block>", "<register>", value); each
    dataset reads its value back with tableGet() inside its transform.

  To add a register:
    1. Add one line to BLOCKS below (offset is relative to the block start;
       remember to extend the driver's polled register groups if needed).
    2. Add a register with the same name to the block's data table.
    3. Add a dataset whose transform reads it back with tableGet().
]]

-- Block fields: func = function code, unit = responding unit (0 = the
-- connection's), bytes = expected payload length, table = the data table
-- the block publishes into.
-- Entry fields: offset = register offset within the block (words for
-- registers, bit index for coils/discrete inputs); type drives decoding
-- ("rbit" = one bit of a register word, with bit and width); order undoes
-- a device's word order (cdab, badc, dcba); scale/shift convert raw values
-- to engineering units (omitted = 1 / 0).
local BLOCKS = {
%2}
)LUA")
                           .arg(QFileInfo(m_filePath).fileName(), spec);

  return header + modbusLuaParserBody();
}

/**
 * @brief Publishes the computed register blocks for the Modbus UI driver to adopt (spec 0077): one
 *        object per block with its type, start address and register count; @p append keeps the
 *        driver's existing groups (a merge into an open project).
 */
void DataModel::ModbusMapImporter::loadRegisterGroups(const QVector<RegisterBlock>& blocks,
                                                      bool append) const
{
  auto* bus = &Core::services().bus;
  SS_ASSERT(bus != nullptr, return);

  QJsonArray groups;
  for (const auto& block : blocks) {
    QJsonObject group;
    group.insert(QStringLiteral("type"), static_cast<int>(block.registerType));
    group.insert(QStringLiteral("start"), static_cast<int>(block.startAddress));
    group.insert(QStringLiteral("count"), static_cast<int>(block.count));
    if (block.slaveAddress != 0)
      group.insert(QStringLiteral("slave"), static_cast<int>(block.slaveAddress));

    groups.append(group);
  }

  bus->publish<Core::Bus::ModbusRegisterGroupsLoaded>(QJsonDocument(groups), append);
}

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a human-readable name for a register type index.
 */
QString DataModel::ModbusMapImporter::registerTypeName(quint8 type)
{
  switch (type) {
    case 0:
      return QStringLiteral("Holding Registers");
    case 1:
      return QStringLiteral("Input Registers");
    case 2:
      return QStringLiteral("Coils");
    case 3:
      return QStringLiteral("Discrete Inputs");
    default:
      return QStringLiteral("Unknown");
  }
}

/**
 * @brief Selects the dashboard widget kind for a numeric register based on units and range.
 */
QString DataModel::ModbusMapImporter::selectDatasetWidget(const ModbusMap::RegisterEntry& entry)
{
  const auto u = entry.units.toLower();

  if (u == QLatin1String("%") || (entry.min == 0 && entry.max == 100))
    return QStringLiteral("bar");

  if (u.contains(QLatin1String("\xc2\xb0")) || u == QLatin1String("rpm")
      || u == QLatin1String("psi") || u == QLatin1String("bar") || u == QLatin1String("kpa")
      || u == QLatin1String("v") || u == QLatin1String("a"))
    return QStringLiteral("gauge");

  return QString();
}
