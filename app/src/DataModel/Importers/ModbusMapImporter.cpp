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

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include "DataModel/Frame.h"
#include "DataModel/Importers/AxisTicks.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/Modbus.h"
#include "Misc/JsonValidator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

namespace detail {

/**
 * @brief CSV header column indices auto-detected from the first row.
 */
struct CsvColumnMap {
  int addr     = -1;
  int name     = -1;
  int type     = -1;
  int dataType = -1;
  int units    = -1;
  int min      = -1;
  int max      = -1;
  int scale    = -1;
  int offset   = -1;
};

}  // namespace detail

using detail::CsvColumnMap;

/**
 * @brief Returns true if any of the literals matches the lowercase column name.
 */
[[nodiscard]] static bool matchAny(const QString& col, std::initializer_list<QLatin1String> options)
{
  for (const auto& opt : options)
    if (col == opt)
      return true;

  return false;
}

/**
 * @brief Maps a single header cell to its column-map slot.
 */
static void mapCsvHeaderColumn(const QString& col, int index, CsvColumnMap& map)
{
  if (matchAny(col,
               {QLatin1String("address"),
                QLatin1String("addr"),
                QLatin1String("register"),
                QLatin1String("reg")})) {
    map.addr = index;
    return;
  }

  if (matchAny(col, {QLatin1String("name"), QLatin1String("label"), QLatin1String("tag")})) {
    map.name = index;
    return;
  }

  if (matchAny(col,
               {QLatin1String("type"),
                QLatin1String("register_type"),
                QLatin1String("function"),
                QLatin1String("fc")})) {
    map.type = index;
    return;
  }

  if (matchAny(col,
               {QLatin1String("datatype"), QLatin1String("data_type"), QLatin1String("format")})) {
    map.dataType = index;
    return;
  }

  if (matchAny(col, {QLatin1String("units"), QLatin1String("unit"), QLatin1String("eng_units")})) {
    map.units = index;
    return;
  }

  if (matchAny(col, {QLatin1String("min"), QLatin1String("minimum"), QLatin1String("range_min")})) {
    map.min = index;
    return;
  }

  if (matchAny(col, {QLatin1String("max"), QLatin1String("maximum"), QLatin1String("range_max")})) {
    map.max = index;
    return;
  }

  if (matchAny(col,
               {QLatin1String("scale"), QLatin1String("factor"), QLatin1String("multiplier")})) {
    map.scale = index;
    return;
  }

  if (col == QLatin1String("offset")) {
    map.offset = index;
    return;
  }

  if (matchAny(col,
               {QLatin1String("description"), QLatin1String("desc"), QLatin1String("comment")})) {
    if (map.name < 0)
      map.name = index;
  }
}

/**
 * @brief Builds the CSV column-map by inspecting the header row.
 */
[[nodiscard]] static CsvColumnMap buildCsvColumnMap(const QStringList& header)
{
  CsvColumnMap map;
  for (int i = 0; i < header.count(); ++i) {
    const auto col = header[i].trimmed().toLower().remove('"');
    mapCsvHeaderColumn(col, i, map);
  }

  return map;
}

/**
 * @brief Returns the trimmed/unquoted value at column index, or fallback when out of range.
 */
[[nodiscard]] static QString csvCell(const QStringList& cols, int colIndex, const QString& fallback)
{
  if (colIndex < 0 || colIndex >= cols.count())
    return fallback;

  return cols[colIndex].trimmed().remove('"');
}

/**
 * @brief Returns a numeric cell value, falling back when missing or empty.
 */
[[nodiscard]] static double csvCellDouble(const QStringList& cols, int colIndex, double fallback)
{
  if (colIndex < 0 || colIndex >= cols.count())
    return fallback;

  const auto text = cols[colIndex].trimmed().remove('"');
  if (text.isEmpty())
    return fallback;

  return SerialStudio::toDouble(text);
}

/**
 * @brief Maps an XML container tag name to a register-type index, or -1 if not a container.
 */
[[nodiscard]] static int xmlTagToType(const QString& tag)
{
  if (tag == QLatin1String("holding-registers") || tag == QLatin1String("holdingregisters")
      || tag == QLatin1String("holding"))
    return 0;

  if (tag == QLatin1String("input-registers") || tag == QLatin1String("inputregisters")
      || tag == QLatin1String("input"))
    return 1;

  if (tag == QLatin1String("coils"))
    return 2;

  if (tag == QLatin1String("discrete-inputs") || tag == QLatin1String("discreteinputs")
      || tag == QLatin1String("discrete"))
    return 3;

  return -1;
}

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

  // Defer to next tick; macOS NSSavePanel KVO callback must unwind first.
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
    ok = parseCSV(filePath);
  else if (ext == QLatin1String("xml"))
    ok = parseXML(filePath);
  else if (ext == QLatin1String("json"))
    ok = parseJSON(filePath);
  else
    ok = parseCSV(filePath) || parseJSON(filePath) || parseXML(filePath);

  if (!ok || m_registers.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("No registers found"),
      tr("The file could not be parsed or contains no register definitions."),
      QMessageBox::Warning,
      tr("Modbus Import"));
    return;
  }

  // Sort by (type, address) for consistent block grouping
  std::sort(
    m_registers.begin(), m_registers.end(), [](const RegisterEntry& a, const RegisterEntry& b) {
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

  auto& pm = ProjectModel::instance();
  QObject::connect(
    &pm,
    &ProjectModel::importCompleted,
    this,
    [registerCount, blockCount](bool accepted, const QString&) {
      if (!accepted)
        return;

      Misc::Utilities::showMessageBox(
        tr("Successfully imported %1 registers in %2 groups.")
          .arg(QString::number(registerCount), QString::number(blockCount)),
        tr("The project editor is now open for customization."),
        QMessageBox::Information,
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
// CSV parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses one CSV data row into a RegisterEntry; returns false to skip the row.
 */
static bool parseCsvRow(const QStringList& cols,
                        const CsvColumnMap& map,
                        int row,
                        DataModel::ModbusMapImporter::RegisterEntry& entry)
{
  if (cols.count() <= map.addr) {
    qWarning().nospace() << "[ModbusMapImporter] CSV row " << row
                         << " skipped: not enough columns (expected address at index " << map.addr
                         << ", got " << cols.count() << " columns)";
    return false;
  }

  // Parse the register address; skip the row if it isn't a valid uint16
  bool addrOk              = false;
  const auto addrText      = cols[map.addr].trimmed().remove('"');
  const quint16 addrParsed = addrText.toUShort(&addrOk);
  if (!addrOk) {
    qWarning().nospace() << "[ModbusMapImporter] CSV row " << row
                         << " skipped: invalid register address '" << addrText << "'";
    return false;
  }

  entry.address = addrParsed;
  entry.name    = csvCell(cols, map.name, QStringLiteral("Register %1").arg(entry.address));
  entry.registerType =
    (map.type >= 0 && map.type < cols.count())
      ? DataModel::ModbusMapImporter::parseRegisterType(cols[map.type].trimmed().remove('"'))
      : quint8(0);
  entry.dataType = csvCell(cols, map.dataType, QStringLiteral("uint16")).toLower();
  entry.units    = csvCell(cols, map.units, QString());
  entry.min      = csvCellDouble(cols, map.min, 0.0);
  entry.max      = csvCellDouble(cols, map.max, 65535.0);
  entry.scale    = csvCellDouble(cols, map.scale, 1.0);
  entry.offset   = csvCellDouble(cols, map.offset, 0.0);

  if (entry.dataType == QLatin1String("bool") && entry.max == 65535)
    entry.max = 1;

  return true;
}

/**
 * @brief Parses a CSV register map with header-based column auto-detection.
 */
bool DataModel::ModbusMapImporter::parseCSV(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  const QString content = QString::fromUtf8(file.readAll());
  file.close();

  const auto lines = content.split('\n', Qt::SkipEmptyParts);
  if (lines.count() < 2)
    return false;

  // Map header columns to field indices using common aliases
  const auto map = buildCsvColumnMap(lines[0].trimmed().split(','));
  if (map.addr < 0)
    return false;

  // Parse data rows
  for (int row = 1; row < lines.count(); ++row) {
    const auto line = lines[row].trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      continue;

    RegisterEntry entry;
    if (!parseCsvRow(line.split(','), map, row, entry))
      continue;

    m_registers.append(entry);
  }

  return !m_registers.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// XML parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses an XML <register> element, returning false to skip a malformed entry.
 */
static bool parseXmlRegisterElement(QXmlStreamReader& xml,
                                    int currentType,
                                    DataModel::ModbusMapImporter::RegisterEntry& entry)
{
  const auto attrs = xml.attributes();

  // Validate the address attribute: skip the element if missing or malformed
  bool addrOk              = false;
  const auto addrText      = attrs.value("address").toString();
  const quint16 addrParsed = addrText.toUShort(&addrOk);
  if (!addrOk) {
    qWarning().nospace() << "[ModbusMapImporter] XML <register> at line " << xml.lineNumber()
                         << " skipped: invalid or missing 'address' attribute ('" << addrText
                         << "')";
    return false;
  }

  entry.address  = addrParsed;
  entry.name     = attrs.value("name").toString();
  entry.dataType = attrs.value("dataType").toString().toLower();
  entry.units    = attrs.value("units").toString();
  entry.min      = SerialStudio::toDouble(attrs.value("min"));
  entry.max   = attrs.value("max").isEmpty() ? 65535 : SerialStudio::toDouble(attrs.value("max"));
  entry.scale = attrs.value("scale").isEmpty() ? 1.0 : SerialStudio::toDouble(attrs.value("scale"));
  entry.offset = SerialStudio::toDouble(attrs.value("offset"));

  if (attrs.hasAttribute("type"))
    entry.registerType =
      DataModel::ModbusMapImporter::parseRegisterType(attrs.value("type").toString());
  else if (currentType >= 0)
    entry.registerType = static_cast<quint8>(currentType);
  else
    entry.registerType = 0;

  if (entry.dataType.isEmpty())
    entry.dataType = (entry.registerType >= 2) ? QStringLiteral("bool") : QStringLiteral("uint16");

  if (entry.name.isEmpty())
    entry.name = QStringLiteral("Register %1").arg(entry.address);

  if (entry.dataType == QLatin1String("bool") && entry.max == 65535)
    entry.max = 1;

  return true;
}

/**
 * @brief Parses an XML register map (flat or nested by register type).
 */
bool DataModel::ModbusMapImporter::parseXML(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QXmlStreamReader xml(&file);
  int current_type = -1;

  while (!xml.atEnd() && !xml.hasError()) {
    const auto token = xml.readNext();
    if (token != QXmlStreamReader::StartElement)
      continue;

    const auto tag_name = xml.name().toString().toLower();

    // Container tag updates the current type and falls through
    const int container_type = xmlTagToType(tag_name);
    if (container_type >= 0) {
      current_type = container_type;
      continue;
    }

    if (tag_name != QLatin1String("register"))
      continue;

    RegisterEntry entry;
    if (parseXmlRegisterElement(xml, current_type, entry))
      m_registers.append(entry);
  }

  file.close();
  return !m_registers.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// JSON parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a JSON register map (flat registers array or grouped by type).
 */
bool DataModel::ModbusMapImporter::parseJSON(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  // Route untrusted map files through the shared size/depth/array caps
  const auto result = Misc::JsonValidator::parseAndValidate(file.readAll());
  file.close();

  if (!result.valid) {
    qWarning() << "[ModbusMapImporter] Invalid JSON map:" << result.errorMessage;
    return false;
  }

  if (!result.document.isObject())
    return false;

  const auto root = result.document.object();

  // Flat format: {"registers": [...]}
  if (root.contains(QStringLiteral("registers"))) {
    const auto arr = root.value(QStringLiteral("registers")).toArray();
    for (const auto& item : arr) {
      RegisterEntry entry;
      if (parseRegisterEntry(item.toObject(), entry))
        m_registers.append(entry);
    }
  }

  // Grouped format: {"holdingRegisters": [...], "coils": [...], ...}
  // clang-format off
  static const struct { const char* key; int type; } groups[] = {
    {"holdingRegisters", 0}, {"holding_registers", 0}, {"holding", 0},
    {"inputRegisters",   1}, {"input_registers",   1}, {"input",   1},
    {"coils",            2},
    {"discreteInputs",   3}, {"discrete_inputs",   3}, {"discrete", 3},
  };

  // clang-format on

  for (const auto& g : groups) {
    const auto key = QLatin1String(g.key);
    if (!root.contains(key))
      continue;

    const auto arr = root.value(key).toArray();
    for (const auto& item : arr) {
      RegisterEntry entry;
      if (parseRegisterEntry(item.toObject(), entry, g.type))
        m_registers.append(entry);
    }
  }

  return !m_registers.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// Register block grouping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Groups registers into contiguous blocks of the same type.
 */
QVector<DataModel::ModbusMapImporter::RegisterBlock> DataModel::ModbusMapImporter::computeBlocks()
  const
{
  if (m_registers.isEmpty())
    return {};

  QVector<RegisterBlock> blocks;
  RegisterBlock current;
  current.registerType = m_registers[0].registerType;
  current.startAddress = m_registers[0].address;
  current.count        = registersForDataType(m_registers[0].dataType);
  current.entries.append(m_registers[0]);

  for (int i = 1; i < m_registers.count(); ++i) {
    const auto& entry     = m_registers[i];
    const quint16 endAddr = current.startAddress + current.count;

    if (entry.registerType == current.registerType && entry.address <= endAddr) {
      const quint16 entryEnd = entry.address + registersForDataType(entry.dataType);
      current.count = qMax(current.count, static_cast<quint16>(entryEnd - current.startAddress));
      current.entries.append(entry);
    } else {
      blocks.append(current);
      current.registerType = entry.registerType;
      current.startAddress = entry.address;
      current.count        = registersForDataType(entry.dataType);
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
  source[Keys::FrameParserCode]       = QString();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("modbus_register_map");
  source[Keys::FrameParserParams]     = buildNativeParserParams(blocks);

  // Embed register groups in connection settings
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

  // Build groups with datasets for each register block
  QJsonArray group_array;
  int group_id      = 0;
  int dataset_index = 1;

  for (const auto& block : blocks) {
    DataModel::Group group;
    group.groupId = group_id;
    group.widget  = QStringLiteral("datagrid");
    // Drop the "@ address" suffix for single-block projects.
    if (blocks.size() == 1)
      group.title = registerTypeName(block.registerType);
    else
      group.title =
        QStringLiteral("%1 @ %2").arg(registerTypeName(block.registerType)).arg(block.startAddress);

    const bool is_bool = (block.registerType >= 2);

    for (const auto& entry : block.entries) {
      DataModel::Dataset dataset;
      dataset.index = dataset_index++;
      dataset.title = entry.name;
      dataset.units = entry.units;
      dataset.log   = true;

      if (is_bool || entry.dataType == QLatin1String("bool")) {
        dataset.led     = true;
        dataset.ledHigh = 1;
        dataset.wgtMin  = 0;
        dataset.wgtMax  = 1;
      } else {
        // Plot range stays tied to the raw register limits
        dataset.pltMin = entry.min;
        dataset.pltMax = entry.max;
        dataset.widget = selectDatasetWidget(entry);
        dataset.plt    = dataset.widget.isEmpty();

        // Snap widget bounds and tick count to friendly multiples for analog widgets
        const auto nice          = niceAxisTicks(entry.min, entry.max);
        dataset.wgtMin           = nice.min;
        dataset.wgtMax           = nice.max;
        dataset.displayTickCount = nice.tickCount;
      }

      group.datasets.push_back(dataset);
    }

    group_array.append(DataModel::serialize(group));
    ++group_id;
  }

  project[QStringLiteral("groups")] = group_array;
  return project;
}

/**
 * @brief Builds the native modbus_register_map template params from the register blocks.
 */
QJsonObject DataModel::ModbusMapImporter::buildNativeParserParams(
  const QVector<RegisterBlock>& blocks)
{
  QJsonArray blocks_json;
  for (const auto& block : blocks) {
    QJsonArray entries;
    for (const auto& entry : block.entries) {
      QJsonObject entry_json;
      entry_json.insert(QStringLiteral("address"), entry.address);
      entry_json.insert(QStringLiteral("dataType"), entry.dataType);
      entry_json.insert(QStringLiteral("scale"), entry.scale);
      entry_json.insert(QStringLiteral("offset"), entry.offset);
      entries.append(entry_json);
    }

    QJsonObject block_json;
    block_json.insert(QStringLiteral("type"), block.registerType);
    block_json.insert(QStringLiteral("start"), block.startAddress);
    block_json.insert(QStringLiteral("entries"), entries);
    blocks_json.append(block_json);
  }

  QJsonObject params;
  params.insert(QStringLiteral("blocks"), blocks_json);
  return params;
}

/**
 * @brief Loads the computed register blocks into the Modbus UI driver.
 */
void DataModel::ModbusMapImporter::loadRegisterGroups(const QVector<RegisterBlock>& blocks) const
{
  auto* modbus = IO::ConnectionManager::instance().modbus();
  if (!modbus)
    return;

  modbus->clearRegisterGroups();
  for (const auto& block : blocks)
    modbus->addRegisterGroup(block.registerType, block.startAddress, block.count);
}

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a register type string to an internal type index (0-3).
 */
quint8 DataModel::ModbusMapImporter::parseRegisterType(const QString& str)
{
  const auto s = str.trimmed().toLower();

  // clang-format off
  if (s == QLatin1String("holding") || s == QLatin1String("0x03")
      || s == QLatin1String("3") || s == QLatin1String("hr"))
    return 0;
  if (s == QLatin1String("input") || s == QLatin1String("0x04")
      || s == QLatin1String("4") || s == QLatin1String("ir"))
    return 1;
  if (s == QLatin1String("coil") || s == QLatin1String("coils")
      || s == QLatin1String("0x01") || s == QLatin1String("1"))
    return 2;
  if (s == QLatin1String("discrete") || s == QLatin1String("discrete_input")
      || s == QLatin1String("0x02") || s == QLatin1String("2") || s == QLatin1String("di"))
    return 3;
  // clang-format on

  return 0;
}

/**
 * @brief Returns the number of Modbus registers consumed by a data type.
 */
int DataModel::ModbusMapImporter::registersForDataType(const QString& dataType)
{
  if (dataType == QLatin1String("float32") || dataType == QLatin1String("uint32")
      || dataType == QLatin1String("int32"))
    return 2;
  if (dataType == QLatin1String("float64") || dataType == QLatin1String("uint64")
      || dataType == QLatin1String("int64"))
    return 4;

  return 1;
}

/**
 * @brief Parses a single register entry from a JSON object.
 */
bool DataModel::ModbusMapImporter::parseRegisterEntry(const QJsonObject& obj,
                                                      RegisterEntry& entry,
                                                      int defaultType)
{
  if (!obj.contains(QStringLiteral("address")) && !obj.contains(QStringLiteral("addr")))
    return false;

  entry.address = static_cast<quint16>(obj.contains(QStringLiteral("address"))
                                         ? obj.value(QStringLiteral("address")).toInt()
                                         : obj.value(QStringLiteral("addr")).toInt());

  entry.name =
    obj.value(QStringLiteral("name")).toString(QStringLiteral("Register %1").arg(entry.address));

  if (obj.contains(QStringLiteral("type")))
    entry.registerType = parseRegisterType(obj.value(QStringLiteral("type")).toString());
  else if (defaultType >= 0)
    entry.registerType = static_cast<quint8>(defaultType);
  else
    entry.registerType = 0;

  entry.dataType = obj.value(QStringLiteral("dataType"))
                     .toString(obj.value(QStringLiteral("data_type"))
                                 .toString((entry.registerType >= 2) ? QStringLiteral("bool")
                                                                     : QStringLiteral("uint16")));
  entry.dataType = entry.dataType.toLower();

  entry.units =
    obj.value(QStringLiteral("units")).toString(obj.value(QStringLiteral("unit")).toString());
  entry.min    = SerialStudio::toDouble(obj.value(QStringLiteral("min")), 0.0);
  entry.max    = SerialStudio::toDouble(obj.value(QStringLiteral("max")),
                                     entry.dataType == QLatin1String("bool") ? 1.0 : 65535.0);
  entry.scale  = SerialStudio::toDouble(obj.value(QStringLiteral("scale")), 1.0);
  entry.offset = SerialStudio::toDouble(obj.value(QStringLiteral("offset")), 0.0);

  return true;
}

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
QString DataModel::ModbusMapImporter::selectDatasetWidget(const RegisterEntry& entry)
{
  const auto u = entry.units.toLower();

  // Bar widget for percentage-style ranges
  if (u == QLatin1String("%") || (entry.min == 0 && entry.max == 100))
    return QStringLiteral("bar");

  // Gauge widget for typical engineering units (degree symbol matches original UTF-8 bytes)
  if (u.contains(QLatin1String("\xc2\xb0")) || u == QLatin1String("rpm")
      || u == QLatin1String("psi") || u == QLatin1String("bar") || u == QLatin1String("kpa")
      || u == QLatin1String("v") || u == QLatin1String("a"))
    return QStringLiteral("gauge");

  return QString();
}
