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

#include "DataModel/ModbusMapImporter.h"

#include <QDebug>
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

  auto* dialog = new QFileDialog(nullptr, tr("Import Modbus Register Map"), home);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setNameFilter(
    tr("Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files "
       "(*.xml);;JSON Files (*.json);;All Files (*)"));

  connect(dialog, &QFileDialog::fileSelected, this, &DataModel::ModbusMapImporter::showPreview);
  connect(dialog, &QFileDialog::finished, dialog, &QObject::deleteLater);

  dialog->show();
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

  loadRegisterGroups(blocks);

  // Open project and prompt user to save
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

  if (col_addr < 0)
    return false;

  // Parse data rows
  for (int row = 1; row < lines.count(); ++row) {
    const auto line = lines[row].trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      continue;

    const auto cols = line.split(',');
    if (cols.count() <= col_addr) {
      qWarning().nospace() << "[ModbusMapImporter] CSV row " << row
                           << " skipped: not enough columns (expected address at index " << col_addr
                           << ", got " << cols.count() << " columns)";
      continue;
    }

    // Parse the register address; skip the row if it isn't a valid uint16
    bool addrOk              = false;
    const auto addrText      = cols[col_addr].trimmed().remove('"');
    const quint16 addrParsed = addrText.toUShort(&addrOk);
    if (!addrOk) {
      qWarning().nospace() << "[ModbusMapImporter] CSV row " << row
                           << " skipped: invalid register address '" << addrText << "'";
      continue;
    }

    RegisterEntry entry;
    entry.address      = addrParsed;
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

    else if (tag_name == QLatin1String("register")) {
      const auto attrs = xml.attributes();

      // Validate the address attribute -- skip the element if missing or malformed
      bool addrOk              = false;
      const auto addrText      = attrs.value("address").toString();
      const quint16 addrParsed = addrText.toUShort(&addrOk);
      if (!addrOk) {
        qWarning().nospace() << "[ModbusMapImporter] XML <register> at line " << xml.lineNumber()
                             << " skipped: invalid or missing 'address' attribute ('" << addrText
                             << "')";
        continue;
      }

      RegisterEntry entry;
      entry.address  = addrParsed;
      entry.name     = attrs.value("name").toString();
      entry.dataType = attrs.value("dataType").toString().toLower();
      entry.units    = attrs.value("units").toString();
      entry.min      = attrs.value("min").toDouble();
      entry.max      = attrs.value("max").isEmpty() ? 65535 : attrs.value("max").toDouble();
      entry.scale    = attrs.value("scale").isEmpty() ? 1.0 : attrs.value("scale").toDouble();
      entry.offset   = attrs.value("offset").toDouble();

      if (attrs.hasAttribute("type"))
        entry.registerType = parseRegisterType(attrs.value("type").toString());
      else if (current_type >= 0)
        entry.registerType = static_cast<quint8>(current_type);
      else
        entry.registerType = 0;

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
 * @brief Parses a JSON register map (flat registers array or grouped by type).
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
  source[Keys::FrameParserCode]       = buildFrameParser(blocks);
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Lua);

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
        dataset.wgtMin = entry.min;
        dataset.wgtMax = entry.max;
        dataset.pltMin = entry.min;
        dataset.pltMax = entry.max;

        // Select widget based on units and range
        const auto u = entry.units.toLower();
        // code-verify off
        if (u == QLatin1String("%") || (entry.min == 0 && entry.max == 100))
          dataset.widget = QStringLiteral("bar");
        else if (u.contains(QLatin1String("°")) || u == QLatin1String("rpm")
                 || u == QLatin1String("psi") || u == QLatin1String("bar")
                 || u == QLatin1String("kpa") || u == QLatin1String("v") || u == QLatin1String("a"))
          dataset.widget = QStringLiteral("gauge");
        // code-verify on

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
 * @brief Generates the Lua frame parser with typed extraction and scaling.
 */
QString DataModel::ModbusMapImporter::buildFrameParser(const QVector<RegisterBlock>& blocks) const
{
  int total_datasets = 0;
  for (const auto& block : blocks)
    total_datasets += block.entries.count();

  const int group_count = blocks.count();

  QString code;

  // Emit header comment
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Modbus Register Map Parser\n");
  code += QStringLiteral("-- Auto-generated from: %1\n").arg(QFileInfo(m_filePath).fileName());
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Groups: %1, Datasets: %2\n").arg(group_count).arg(total_datasets);
  code += QStringLiteral("-- Frame: {slaveAddr, funcCode, byteCount, ...data}\n");
  code += QStringLiteral("--\n\n");

  // Emit global state and helpers
  code += QStringLiteral("local values = {}\n");
  code += QStringLiteral("for i = 1, %1 do values[i] = 0 end\n").arg(total_datasets);
  code += QStringLiteral("local currentGroup = 0\n\n");

  bool needs_float = false;
  for (const auto& block : blocks)
    for (const auto& entry : block.entries)
      if (entry.dataType == QLatin1String("float32"))
        needs_float = true;

  if (needs_float) {
    code += QStringLiteral("local function toFloat32(b0, b1, b2, b3)\n");
    code += QStringLiteral("  local sign = (b0 & 0x80) ~= 0 and -1 or 1\n");
    code += QStringLiteral("  local exp  = ((b0 & 0x7F) << 1) | (b1 >> 7)\n");
    code += QStringLiteral("  local mant = ((b1 & 0x7F) << 16) | (b2 << 8) | b3\n");
    code += QStringLiteral("  if exp == 0 then return sign * mant * 2.0^(-149) end\n");
    code +=
      QStringLiteral("  if exp == 0xFF then return mant ~= 0 and (0/0) or sign * math.huge end\n");
    code += QStringLiteral("  return sign * (mant | 0x800000) * 2.0^(exp - 150)\n");
    code += QStringLiteral("end\n\n");
  }

  // Emit main parse function with group dispatch
  code += QStringLiteral("function parse(frame)\n");
  code += QStringLiteral("  if #frame < 3 then return values end\n\n");
  code += QStringLiteral("  -- Extract data payload (skip slave addr, func code, byte count)\n");
  code += QStringLiteral("  local data = {}\n");
  code += QStringLiteral("  for i = 4, #frame do data[#data + 1] = frame[i] end\n\n");

  int ds_offset = 0;
  for (int g = 0; g < group_count; ++g) {
    const auto& block = blocks[g];
    const bool is_bit = (block.registerType >= 2);

    code += QStringLiteral("  -- %1 @ %2\n")
              .arg(registerTypeName(block.registerType))
              .arg(block.startAddress);
    if (g == 0)
      code += QStringLiteral("  if currentGroup == %1 then\n").arg(g);
    else
      code += QStringLiteral("  elseif currentGroup == %1 then\n").arg(g);

    for (int e = 0; e < block.entries.count(); ++e) {
      const auto& entry = block.entries[e];
      const int reg_off = entry.address - block.startAddress;
      const int idx     = ds_offset + e + 1;

      if (e > 0)
        code += QStringLiteral("\n");

      code += QStringLiteral("    -- %1\n").arg(entry.name);

      if (is_bit) {
        const int byte_idx = reg_off / 8 + 1;
        const int bit_idx  = reg_off % 8;
        code += QStringLiteral("    values[%1] = (data[%2] >> %3) & 1\n")
                  .arg(idx)
                  .arg(byte_idx)
                  .arg(bit_idx);
      } else if (entry.dataType == QLatin1String("float32")) {
        const int byte_off = reg_off * 2 + 1;
        if (entry.scale != 1.0 || entry.offset != 0.0)
          code += QStringLiteral("    values[%1] = toFloat32(data[%2], data[%3], data[%4], "
                                 "data[%5]) * %6 + %7\n")
                    .arg(idx)
                    .arg(byte_off)
                    .arg(byte_off + 1)
                    .arg(byte_off + 2)
                    .arg(byte_off + 3)
                    .arg(entry.scale)
                    .arg(entry.offset);
        else
          code +=
            QStringLiteral("    values[%1] = toFloat32(data[%2], data[%3], data[%4], data[%5])\n")
              .arg(idx)
              .arg(byte_off)
              .arg(byte_off + 1)
              .arg(byte_off + 2)
              .arg(byte_off + 3);
      } else if (entry.dataType == QLatin1String("int16")) {
        const int byte_off = reg_off * 2 + 1;
        code += QStringLiteral("    values[%1] = (data[%2] << 8) | data[%3]\n")
                  .arg(idx)
                  .arg(byte_off)
                  .arg(byte_off + 1);
        code +=
          QStringLiteral("    if values[%1] > 32767 then values[%1] = values[%1] - 65536 end\n")
            .arg(idx);
        if (entry.scale != 1.0 || entry.offset != 0.0)
          code += QStringLiteral("    values[%1] = values[%1] * %2 + %3\n")
                    .arg(idx)
                    .arg(entry.scale)
                    .arg(entry.offset);
      } else {
        const int byte_off = reg_off * 2 + 1;
        if (entry.scale != 1.0 || entry.offset != 0.0)
          code += QStringLiteral("    values[%1] = ((data[%2] << 8) | data[%3]) * %4 + %5\n")
                    .arg(idx)
                    .arg(byte_off)
                    .arg(byte_off + 1)
                    .arg(entry.scale)
                    .arg(entry.offset);
        else
          code += QStringLiteral("    values[%1] = (data[%2] << 8) | data[%3]\n")
                    .arg(idx)
                    .arg(byte_off)
                    .arg(byte_off + 1);
      }
    }

    ds_offset += block.entries.count();
  }

  if (group_count > 0)
    code += QStringLiteral("  end\n\n");

  code += QStringLiteral("  currentGroup = (currentGroup + 1) %1 %2\n")
            .arg(QLatin1Char('%'))
            .arg(group_count);
  code += QStringLiteral("  return values\n");
  code += QStringLiteral("end\n");

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
