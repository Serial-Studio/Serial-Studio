/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "DataModel/ModbusMapImporter.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/Modbus.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Private constructor for the singleton.
 */
DataModel::ModbusMapImporter::ModbusMapImporter() {}

/**
 * @brief Returns the singleton instance.
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
 * @brief Returns the number of parsed register entries.
 */
int DataModel::ModbusMapImporter::registerCount() const noexcept
{
  return m_registers.count();
}

/**
 * @brief Returns the number of contiguous register blocks.
 */
int DataModel::ModbusMapImporter::groupCount() const noexcept
{
  return computeBlocks().count();
}

/**
 * @brief Returns the base filename of the imported register map.
 */
QString DataModel::ModbusMapImporter::fileName() const
{
  return QFileInfo(m_filePath).fileName();
}

/**
 * @brief Returns a formatted info string for the register at @p index.
 *
 * Format: "N: Name @ Address (Type, DataType) [Units]"
 * Used by QML preview table for regex-based column extraction.
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
 * @brief Opens a file dialog for the user to select a Modbus register map file.
 *
 * Supports CSV, XML, and JSON formats. Auto-detects the format from the file
 * extension and falls back to trying all parsers if the extension is unknown.
 */
void DataModel::ModbusMapImporter::importRegisterMap()
{
  const auto home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

  auto* dialog = new QFileDialog(nullptr, tr("Import Modbus Register Map"), home);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setNameFilter(
    tr("Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files "
       "(*.xml);;JSON Files (*.json);;All Files (*)"));
  dialog->setOption(QFileDialog::DontUseNativeDialog, true);

  connect(dialog, &QFileDialog::fileSelected, this, &DataModel::ModbusMapImporter::showPreview);
  connect(dialog, &QFileDialog::finished, dialog, &QObject::deleteLater);

  dialog->show();
}

/**
 * @brief Parses the selected file and emits @c previewReady() on success.
 *
 * Auto-detects the format from the file extension. Falls back to trying all
 * parsers sequentially if the extension is unrecognized.
 */
void DataModel::ModbusMapImporter::showPreview(const QString& filePath)
{
  if (filePath.isEmpty())
    return;

  m_registers.clear();

  // Auto-detect format from extension
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

  // Sort by register type then address for consistent ordering
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

  const auto blocks    = computeBlocks();
  const auto project   = buildProject();
  const auto file_info = QFileInfo(m_filePath);
  const auto temp_dir  = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  const auto temp_path = temp_dir + "/" + file_info.baseName() + "_temp.ssproj";

  // Write temporary project file
  QFile file(temp_path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(tr("Failed to create temporary project file"),
                                    tr("Check write permissions to the temporary directory."),
                                    QMessageBox::Critical,
                                    tr("Modbus Import"));
    return;
  }

  QJsonDocument doc(project);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  // Load register groups into the Modbus driver
  loadRegisterGroups(blocks);

  // Load project and prompt save
  ProjectModel::instance().openJsonFile(temp_path);
  if (ProjectModel::instance().saveJsonFile(true)) {
    QFile::remove(temp_path);
    Misc::Utilities::showMessageBox(tr("Successfully imported %1 registers in %2 groups.")
                                      .arg(m_registers.count())
                                      .arg(blocks.count()),
                                    tr("The project editor is now open for customization."),
                                    QMessageBox::Information,
                                    tr("Modbus Import Complete"));
  } else {
    QFile::remove(temp_path);
  }
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
 * @brief Parses a CSV register map file with flexible column detection.
 *
 * Reads the header row to auto-detect column positions using common aliases
 * (e.g., "addr"/"register" for address, "fc"/"function" for register type).
 * Skips empty lines and comment lines starting with '#'.
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

  // Parse header — find column indices by matching aliases
  const auto header = lines[0].trimmed().split(',');
  int col_addr = -1, col_name = -1, col_type = -1, col_data_type = -1;
  int col_units = -1, col_min = -1, col_max = -1, col_scale = -1, col_offset = -1;

  for (int i = 0; i < header.count(); ++i) {
    const auto col = header[i].trimmed().toLower().remove('"');

    // clang-format off
    if (col == QLatin1String("address") || col == QLatin1String("addr")
        || col == QLatin1String("register") || col == QLatin1String("reg"))
      col_addr = i;
    else if (col == QLatin1String("name") || col == QLatin1String("label")
             || col == QLatin1String("tag"))
      col_name = i;
    else if (col == QLatin1String("type") || col == QLatin1String("register_type")
             || col == QLatin1String("function") || col == QLatin1String("fc"))
      col_type = i;
    else if (col == QLatin1String("datatype") || col == QLatin1String("data_type")
             || col == QLatin1String("format"))
      col_data_type = i;
    else if (col == QLatin1String("units") || col == QLatin1String("unit")
             || col == QLatin1String("eng_units"))
      col_units = i;
    else if (col == QLatin1String("min") || col == QLatin1String("minimum")
             || col == QLatin1String("range_min"))
      col_min = i;
    else if (col == QLatin1String("max") || col == QLatin1String("maximum")
             || col == QLatin1String("range_max"))
      col_max = i;
    else if (col == QLatin1String("scale") || col == QLatin1String("factor")
             || col == QLatin1String("multiplier"))
      col_scale = i;
    else if (col == QLatin1String("offset"))
      col_offset = i;
    else if (col == QLatin1String("description") || col == QLatin1String("desc")
             || col == QLatin1String("comment")) {
      if (col_name < 0)
        col_name = i;
    }
    // clang-format on
  }

  // Address column is mandatory
  if (col_addr < 0)
    return false;

  // Parse data rows
  for (int row = 1; row < lines.count(); ++row) {
    const auto line = lines[row].trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      continue;

    const auto cols = line.split(',');
    if (cols.count() <= col_addr)
      continue;

    RegisterEntry entry;
    entry.address      = cols[col_addr].trimmed().remove('"').toUShort();
    entry.name         = (col_name >= 0 && col_name < cols.count())
                         ? cols[col_name].trimmed().remove('"')
                         : QStringLiteral("Register %1").arg(entry.address);
    entry.registerType = (col_type >= 0 && col_type < cols.count())
                         ? parseRegisterType(cols[col_type].trimmed().remove('"'))
                         : 0;
    entry.dataType     = (col_data_type >= 0 && col_data_type < cols.count())
                         ? cols[col_data_type].trimmed().remove('"').toLower()
                         : QStringLiteral("uint16");
    entry.units        = (col_units >= 0 && col_units < cols.count())
                         ? cols[col_units].trimmed().remove('"')
                         : QString();
    entry.min =
      (col_min >= 0 && col_min < cols.count()) ? cols[col_min].trimmed().remove('"').toDouble() : 0;
    entry.max = (col_max >= 0 && col_max < cols.count())
                ? cols[col_max].trimmed().remove('"').toDouble()
                : 65535;
    entry.scale =
      (col_scale >= 0 && col_scale < cols.count() && !cols[col_scale].trimmed().isEmpty())
        ? cols[col_scale].trimmed().remove('"').toDouble()
        : 1.0;
    entry.offset =
      (col_offset >= 0 && col_offset < cols.count() && !cols[col_offset].trimmed().isEmpty())
        ? cols[col_offset].trimmed().remove('"').toDouble()
        : 0.0;

    // Default max for boolean types
    if (entry.dataType == QLatin1String("bool") && entry.max == 65535)
      entry.max = 1;

    m_registers.append(entry);
  }

  return !m_registers.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// XML parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses an XML register map file.
 *
 * Supports two layouts:
 * - Flat: <modbus><register address="0" type="holding" .../></modbus>
 * - Nested: <modbus><holding-registers><register .../></holding-registers></modbus>
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

    // Detect nested type containers
    // clang-format off
    if (tag_name == QLatin1String("holding-registers")
        || tag_name == QLatin1String("holdingregisters")
        || tag_name == QLatin1String("holding"))
      current_type = 0;
    else if (tag_name == QLatin1String("input-registers")
             || tag_name == QLatin1String("inputregisters")
             || tag_name == QLatin1String("input"))
      current_type = 1;
    else if (tag_name == QLatin1String("coils"))
      current_type = 2;
    else if (tag_name == QLatin1String("discrete-inputs")
             || tag_name == QLatin1String("discreteinputs")
             || tag_name == QLatin1String("discrete"))
      current_type = 3;
    // clang-format on

    // Parse <register> elements
    else if (tag_name == QLatin1String("register")) {
      const auto attrs = xml.attributes();

      RegisterEntry entry;
      entry.address  = attrs.value("address").toUShort();
      entry.name     = attrs.value("name").toString();
      entry.dataType = attrs.value("dataType").toString().toLower();
      entry.units    = attrs.value("units").toString();
      entry.min      = attrs.value("min").toDouble();
      entry.max      = attrs.value("max").isEmpty() ? 65535 : attrs.value("max").toDouble();
      entry.scale    = attrs.value("scale").isEmpty() ? 1.0 : attrs.value("scale").toDouble();
      entry.offset   = attrs.value("offset").toDouble();

      // Determine register type from attribute or enclosing tag
      if (attrs.hasAttribute("type"))
        entry.registerType = parseRegisterType(attrs.value("type").toString());
      else if (current_type >= 0)
        entry.registerType = static_cast<quint8>(current_type);
      else
        entry.registerType = 0;

      // Defaults
      if (entry.dataType.isEmpty())
        entry.dataType =
          (entry.registerType >= 2) ? QStringLiteral("bool") : QStringLiteral("uint16");
      if (entry.name.isEmpty())
        entry.name = QStringLiteral("Register %1").arg(entry.address);
      if (entry.dataType == QLatin1String("bool") && entry.max == 65535)
        entry.max = 1;

      m_registers.append(entry);
    }
  }

  file.close();
  return !m_registers.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// JSON parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a JSON register map file.
 *
 * Supports two layouts:
 * - Flat: {"registers": [{address, type, name, ...}]}
 * - Grouped: {"holdingRegisters": [...], "coils": [...], ...}
 */
bool DataModel::ModbusMapImporter::parseJSON(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  const auto doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  if (!doc.isObject())
    return false;

  const auto root = doc.object();

  // Try flat format first: {"registers": [...]}
  if (root.contains(QStringLiteral("registers"))) {
    const auto arr = root.value(QStringLiteral("registers")).toArray();
    for (const auto& item : arr) {
      RegisterEntry entry;
      if (parseRegisterEntry(item.toObject(), entry))
        m_registers.append(entry);
    }
  }

  // Try grouped format: {"holdingRegisters": [...], "coils": [...], ...}
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
 * @brief Groups parsed register entries into contiguous blocks of the same type.
 *
 * Accounts for multi-register data types (float32 = 2 registers) when computing
 * contiguity. Non-contiguous entries of the same type produce separate blocks.
 */
QVector<DataModel::ModbusMapImporter::RegisterBlock> DataModel::ModbusMapImporter::computeBlocks()
  const
{
  if (m_registers.isEmpty())
    return {};

  // Registers are already sorted by (type, address) from showPreview()
  QVector<RegisterBlock> blocks;
  RegisterBlock current;
  current.registerType = m_registers[0].registerType;
  current.startAddress = m_registers[0].address;
  current.count        = registersForDataType(m_registers[0].dataType);
  current.entries.append(m_registers[0]);

  for (int i = 1; i < m_registers.count(); ++i) {
    const auto& entry     = m_registers[i];
    const quint16 endAddr = current.startAddress + current.count;

    // Continue the block if same type and contiguous (or overlapping)
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
 * @brief Builds a complete Serial Studio project JSON from the parsed registers.
 */
QJsonObject DataModel::ModbusMapImporter::buildProject() const
{
  const auto blocks = computeBlocks();

  QJsonObject project;
  project[QStringLiteral("title")]   = QFileInfo(m_filePath).baseName();
  project[QStringLiteral("actions")] = QJsonArray();

  // Source configuration
  QJsonObject source;
  source[QStringLiteral("sourceId")]              = 0;
  source[QStringLiteral("title")]                 = tr("Modbus");
  source[QStringLiteral("busType")]               = static_cast<int>(SerialStudio::BusType::ModBus);
  source[QStringLiteral("frameStart")]            = QString();
  source[QStringLiteral("frameEnd")]              = QString();
  source[QStringLiteral("checksum")]              = QString();
  source[QStringLiteral("frameDetection")]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[QStringLiteral("decoder")]               = static_cast<int>(SerialStudio::Binary);
  source[QStringLiteral("hexadecimalDelimiters")] = false;
  source[QStringLiteral("frameParserCode")]       = buildFrameParser(blocks);

  // Save register groups into connection settings
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
  source[QStringLiteral("connection")]            = conn_settings;

  project[QStringLiteral("sources")] = QJsonArray{source};

  // Build groups and datasets
  QJsonArray group_array;
  int group_id      = 0;
  int dataset_index = 1;

  for (const auto& block : blocks) {
    DataModel::Group group;
    group.groupId = group_id;
    group.widget  = QStringLiteral("datagrid");
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
        dataset.wgtMin = entry.min;
        dataset.wgtMax = entry.max;
        dataset.pltMin = entry.min;
        dataset.pltMax = entry.max;

        // Widget heuristics based on units and range
        const auto u = entry.units.toLower();
        if (u == QLatin1String("%") || (entry.min == 0 && entry.max == 100))
          dataset.widget = QStringLiteral("bar");
        else if (u.contains(QLatin1String("°")) || u == QLatin1String("rpm")
                 || u == QLatin1String("psi") || u == QLatin1String("bar")
                 || u == QLatin1String("kpa") || u == QLatin1String("v") || u == QLatin1String("a"))
          dataset.widget = QStringLiteral("gauge");

        // Only enable plot if no specific widget was assigned
        dataset.plt = dataset.widget.isEmpty();
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
 * @brief Generates a JavaScript frame parser for the register blocks.
 *
 * Extends the Modbus driver's frame parser pattern with data-type-aware
 * extraction (int16 sign extension, float32 IEEE 754) and scaling/offset.
 */
QString DataModel::ModbusMapImporter::buildFrameParser(const QVector<RegisterBlock>& blocks) const
{
  // Count total datasets
  int total_datasets = 0;
  for (const auto& block : blocks)
    total_datasets += block.entries.count();

  const int group_count = blocks.count();

  QString code;

  // Header
  code += QStringLiteral("/**\n");
  code += QStringLiteral(" * Modbus Register Map Parser\n");
  code += QStringLiteral(" * Auto-generated from: %1\n").arg(QFileInfo(m_filePath).fileName());
  code += QStringLiteral(" *\n");
  code += QStringLiteral(" * Groups: %1, Datasets: %2\n").arg(group_count).arg(total_datasets);
  code += QStringLiteral(" * Frame: [slaveAddr, funcCode, byteCount, ...data]\n");
  code += QStringLiteral(" */\n\n");

  code += QStringLiteral("var values = new Array(%1).fill(0);\n").arg(total_datasets);
  code += QStringLiteral("var currentGroup = 0;\n\n");

  // Float32 helper (only if needed)
  bool needs_float = false;
  for (const auto& block : blocks)
    for (const auto& entry : block.entries)
      if (entry.dataType == QLatin1String("float32"))
        needs_float = true;

  if (needs_float) {
    code += QStringLiteral("var _f32Buf = new ArrayBuffer(4);\n");
    code += QStringLiteral("var _f32U32 = new Uint32Array(_f32Buf);\n");
    code += QStringLiteral("var _f32F32 = new Float32Array(_f32Buf);\n\n");
  }

  // Main parse function
  code += QStringLiteral("function parse(frame) {\n");
  code += QStringLiteral("  if (frame.length < 3)\n");
  code += QStringLiteral("    return values;\n\n");
  code += QStringLiteral("  var data = frame.slice(3);\n\n");
  code += QStringLiteral("  switch (currentGroup) {\n");

  int ds_offset = 0;
  for (int g = 0; g < group_count; ++g) {
    const auto& block = blocks[g];
    const bool is_bit = (block.registerType >= 2);

    code += QStringLiteral("    case %1: // %2 @ %3\n")
              .arg(g)
              .arg(registerTypeName(block.registerType))
              .arg(block.startAddress);

    for (int e = 0; e < block.entries.count(); ++e) {
      const auto& entry = block.entries[e];
      const int reg_off = entry.address - block.startAddress;

      if (is_bit) {
        // Coils/discrete: bit extraction
        const int byte_idx = reg_off / 8;
        const int bit_idx  = reg_off % 8;
        code += QStringLiteral("      values[%1] = (data[%2] >> %3) & 1;")
                  .arg(ds_offset + e)
                  .arg(byte_idx)
                  .arg(bit_idx);
      } else if (entry.dataType == QLatin1String("float32")) {
        // IEEE 754 float from 2 registers
        const int byte_off = reg_off * 2;
        code += QStringLiteral("      _f32U32[0] = ((data[%1] << 24) | (data[%2] << 16) "
                               "| (data[%3] << 8) | data[%4]) >>> 0; "
                               "values[%5] = _f32F32[0]")
                  .arg(byte_off)
                  .arg(byte_off + 1)
                  .arg(byte_off + 2)
                  .arg(byte_off + 3)
                  .arg(ds_offset + e);
        if (entry.scale != 1.0 || entry.offset != 0.0)
          code += QStringLiteral(" * %1 + %2").arg(entry.scale).arg(entry.offset);
        code += QStringLiteral(";");
      } else if (entry.dataType == QLatin1String("int16")) {
        // Signed 16-bit
        const int byte_off = reg_off * 2;
        code += QStringLiteral("      values[%1] = (data[%2] << 8) | data[%3]; "
                               "if (values[%1] > 32767) values[%1] -= 65536;")
                  .arg(ds_offset + e)
                  .arg(byte_off)
                  .arg(byte_off + 1);
        if (entry.scale != 1.0 || entry.offset != 0.0)
          code += QStringLiteral(" values[%1] = values[%1] * %2 + %3;")
                    .arg(ds_offset + e)
                    .arg(entry.scale)
                    .arg(entry.offset);
      } else {
        // Default: unsigned 16-bit (uint16, uint32 first word, etc.)
        const int byte_off = reg_off * 2;
        code += QStringLiteral("      values[%1] = (data[%2] << 8) | data[%3];")
                  .arg(ds_offset + e)
                  .arg(byte_off)
                  .arg(byte_off + 1);
        if (entry.scale != 1.0 || entry.offset != 0.0)
          code += QStringLiteral(" values[%1] = values[%1] * %2 + %3;")
                    .arg(ds_offset + e)
                    .arg(entry.scale)
                    .arg(entry.offset);
      }

      code += QStringLiteral(" // %1\n").arg(entry.name);
    }

    code += QStringLiteral("      break;\n");
    ds_offset += block.entries.count();
  }

  code += QStringLiteral("  }\n\n");
  code += QStringLiteral("  currentGroup = (currentGroup + 1) % %1;\n").arg(group_count);
  code += QStringLiteral("  return values;\n");
  code += QStringLiteral("}\n");

  return code;
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
 * @param defaultType If >= 0, used when the object lacks a "type" field.
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
  entry.min = obj.value(QStringLiteral("min")).toDouble(0);
  entry.max =
    obj.value(QStringLiteral("max")).toDouble(entry.dataType == QLatin1String("bool") ? 1 : 65535);
  entry.scale  = obj.value(QStringLiteral("scale")).toDouble(1.0);
  entry.offset = obj.value(QStringLiteral("offset")).toDouble(0.0);

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
