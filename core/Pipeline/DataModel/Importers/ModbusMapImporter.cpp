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

  loadRegisterGroups(blocks);

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
 * @brief Groups registers into contiguous blocks of the same type. End addresses are computed
 *        in int and clamped: a quint16 sum wraps for maps that pair high addresses with
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
  current.entries.append(m_registers[0]);

  for (int i = 1; i < m_registers.count(); ++i) {
    const auto& entry = m_registers[i];
    const int endAddr = static_cast<int>(current.startAddress) + current.count;

    if (entry.registerType == current.registerType && entry.address <= endAddr) {
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
  const auto blocks = computeBlocks();

  QStringList table_names;
  QList<QStringList> register_names;
  for (const auto& block : blocks) {
    table_names.append(blockTitle(block, blocks.size()));
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

      group.datasets.push_back(
        buildDatasetFromEntry(block.entries.at(e), is_bool, table.name, reg_name, dataset_index++));
    }

    groups.push_back(group);
    tables.push_back(table);
  }

  finalizeImportedProject(project, groups, tables, tr("Overview"));
  return project;
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

  if (isBool || entry.dataType == QLatin1String("bool")) {
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
 * @brief Returns the block's display title, doubling as its data-table name.
 */
QString DataModel::ModbusMapImporter::blockTitle(const RegisterBlock& block, qsizetype blockCount)
{
  if (blockCount == 1)
    return registerTypeName(block.registerType);

  return QStringLiteral("%1 @ %2")
    .arg(registerTypeName(block.registerType))
    .arg(block.startAddress);
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
 *        packed bits; a bool on a register block decodes the whole 16-bit word.
 */
QString DataModel::ModbusMapImporter::luaEntryType(const ModbusMap::RegisterEntry& entry,
                                                   bool bitBlock)
{
  if (bitBlock)
    return QStringLiteral("bit");

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

local function read_uint(frame, first, n)
  local v = 0
  for i = 0, n - 1 do
    v = v * 256 + string.byte(frame, first + i)
  end
  return v
end

local function to_signed(v, n)
  if v >= 2 ^ (8 * n - 1) then
    v = v - 2 ^ (8 * n)
  end
  return v
end

local function read_float32(frame, first)
  local b1, b2, b3, b4 = string.byte(frame, first, first + 3)
  local sign = bit.band(b1, 0x80) ~= 0 and -1 or 1
  local expo = bit.band(b1, 0x7F) * 2 + bit.rshift(b2, 7)
  local mant = bit.band(b2, 0x7F) * 65536 + b3 * 256 + b4
  if expo == 0 then
    return sign * mant * 2 ^ (-126 - 23)
  elseif expo == 255 then
    if mant == 0 then return sign * math.huge end
    return 0 / 0
  end
  return sign * (1 + mant / 2 ^ 23) * 2 ^ (expo - 127)
end

local function read_float64(frame, first)
  local b1, b2 = string.byte(frame, first, first + 1)
  local sign = bit.band(b1, 0x80) ~= 0 and -1 or 1
  local expo = bit.band(b1, 0x7F) * 16 + bit.rshift(b2, 4)
  local mant = bit.band(b2, 0x0F)
  for i = 2, 7 do
    mant = mant * 256 + string.byte(frame, first + i)
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
-- "bool" reads a whole 16-bit register as 0/1 truthiness.
local function decode(frame, limit, entry)
  if entry.type == "bit" then
    local byte_idx = 4 + math.floor(entry.offset / 8)
    if byte_idx > limit then
      return nil
    end

    return bit.band(bit.rshift(string.byte(frame, byte_idx), entry.offset % 8), 1)
  end

  local size = SIZES[entry.type] or 2
  local first = 4 + entry.offset * 2
  if first + size - 1 > limit then
    return nil
  end

  local raw
  if entry.type == "float32" then
    raw = read_float32(frame, first)
  elseif entry.type == "float64" then
    raw = read_float64(frame, first)
  elseif entry.type == "int16" or entry.type == "int32" or entry.type == "int64" then
    raw = to_signed(read_uint(frame, first, size), size)
  else
    raw = read_uint(frame, first, size)
  end

  if entry.type == "bool" then
    return (raw ~= 0) and 1 or 0
  end

  return raw * (entry.scale or 1) + (entry.shift or 0)
end

local cursor = 1

-- The Binary decoder hands parse() the frame as a 1-indexed table of byte
-- values; string.unpack needs a string, so convert once up front (Modbus
-- ADUs are at most 256 bytes).
function parse(frame)
  if type(frame) == "table" then
    frame = string.char(table.unpack(frame))
  end

  if #frame < 3 then
    return {}
  end

  local func = string.byte(frame, 2)
  if func >= 0x80 then
    return {} -- Modbus exception response
  end

  local block = nil
  for probe = 0, #BLOCKS - 1 do
    local candidate = ((cursor - 1 + probe) % #BLOCKS) + 1
    if BLOCKS[candidate].func == func then
      block = BLOCKS[candidate]
      cursor = (candidate % #BLOCKS) + 1
      break
    end
  end

  if not block then
    return {}
  end

  local limit = math.min(3 + string.byte(frame, 3), #frame)
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
 * @brief Generates the user-editable Lua frame parser: a documented header, the declarative
 *        BLOCKS spec, and the cursor-latching decode machinery mirroring the driver's
 *        round-robin polling.
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

    spec += QStringLiteral("  {\n    func = 0x%1,\n    table = %2,\n    entries = {\n")
              .arg(QString::number(func, 16).toUpper().rightJustified(2, QLatin1Char('0')),
                   luaQuote(tableNames.at(b)));

    for (qsizetype e = 0; e < block.entries.size(); ++e) {
      const auto& entry = block.entries.at(e);

      QString line = QStringLiteral("      { name = %1, offset = %2, type = %3")
                       .arg(luaQuote(registerNames.at(b).at(e)),
                            QString::number(entry.address - block.startAddress),
                            luaQuote(luaEntryType(entry, is_bits)));

      if (std::isfinite(entry.scale) && entry.scale != 1.0)
        line += QStringLiteral(", scale = %1").arg(luaNumber(entry.scale));

      if (std::isfinite(entry.offset) && entry.offset != 0.0)
        line += QStringLiteral(", shift = %1").arg(luaNumber(entry.offset));

      spec += line + QStringLiteral(" },\n");
    }

    spec += QStringLiteral("    },\n  },\n");
  }

  const QString header = QStringLiteral(R"LUA(--[[
  Modbus register-map parser generated by the Serial Studio Modbus importer.
  Source map: %1

  Wire format (response ADU): [slave, function, byteCount, data...]

  The Modbus driver polls the configured register blocks round-robin.
  Responses carry no block identity, so parse() advances a cursor through
  BLOCKS and resyncs on the response function code when a reply is dropped.

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

-- Entry fields: offset = register offset within the block (words for
-- registers, bit index for coils/discrete inputs); type drives decoding;
-- scale/shift convert raw values to engineering units (omitted = 1 / 0).
local BLOCKS = {
%2}
)LUA")
                           .arg(QFileInfo(m_filePath).fileName(), spec);

  return header + modbusLuaParserBody();
}

/**
 * @brief Publishes the computed register blocks for the Modbus UI driver to adopt (spec 0077): one
 *        object per block with its type, start address and register count.
 */
void DataModel::ModbusMapImporter::loadRegisterGroups(const QVector<RegisterBlock>& blocks) const
{
  auto* bus = &Core::services().bus;
  SS_ASSERT(bus != nullptr, return);

  QJsonArray groups;
  for (const auto& block : blocks) {
    QJsonObject group;
    group.insert(QStringLiteral("type"), static_cast<int>(block.registerType));
    group.insert(QStringLiteral("start"), static_cast<int>(block.startAddress));
    group.insert(QStringLiteral("count"), static_cast<int>(block.count));
    groups.append(group);
  }

  bus->publish<Core::Bus::ModbusRegisterGroupsLoaded>(QJsonDocument(groups));
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
