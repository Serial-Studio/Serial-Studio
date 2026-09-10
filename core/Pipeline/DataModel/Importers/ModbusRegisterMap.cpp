/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include "DataModel/Importers/ModbusRegisterMap.h"

#include <initializer_list>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QXmlStreamReader>

#include "Core/JsonValidator.h"
#include "Core/SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr qsizetype kMaxImportFileBytes = 10 * 1024 * 1024;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

namespace detail::modbusmap {

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

/**
 * @brief Parses one CSV data row into a RegisterEntry; returns false to skip the row.
 */
[[nodiscard]] static bool parseCsvRow(const QStringList& cols,
                                      const CsvColumnMap& map,
                                      int row,
                                      DataModel::ModbusMap::RegisterEntry& entry)
{
  if (cols.count() <= map.addr) {
    qWarning().nospace() << "[ModbusMapImporter] CSV row " << row
                         << " skipped: not enough columns (expected address at index " << map.addr
                         << ", got " << cols.count() << " columns)";
    return false;
  }

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
      ? DataModel::ModbusMap::parseRegisterType(cols[map.type].trimmed().remove('"'))
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
 * @brief Parses an XML <register> element, returning false to skip a malformed entry.
 */
[[nodiscard]] static bool parseXmlRegisterElement(QXmlStreamReader& xml,
                                                  int currentType,
                                                  DataModel::ModbusMap::RegisterEntry& entry)
{
  const auto attrs = xml.attributes();

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
    entry.registerType = DataModel::ModbusMap::parseRegisterType(attrs.value("type").toString());
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
 * @brief Parses a single register entry from a JSON object.
 */
[[nodiscard]] static bool parseRegisterEntry(const QJsonObject& obj,
                                             DataModel::ModbusMap::RegisterEntry& entry,
                                             int defaultType = -1)
{
  if (!obj.contains(QStringLiteral("address")) && !obj.contains(QStringLiteral("addr")))
    return false;

  entry.address = static_cast<quint16>(obj.contains(QStringLiteral("address"))
                                         ? obj.value(QStringLiteral("address")).toInt()
                                         : obj.value(QStringLiteral("addr")).toInt());

  entry.name =
    obj.value(QStringLiteral("name")).toString(QStringLiteral("Register %1").arg(entry.address));

  if (obj.contains(QStringLiteral("type")))
    entry.registerType =
      DataModel::ModbusMap::parseRegisterType(obj.value(QStringLiteral("type")).toString());
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

}  // namespace detail::modbusmap

//--------------------------------------------------------------------------------------------------
// Register-model queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a register type string to an internal type index (0-3).
 */
quint8 DataModel::ModbusMap::parseRegisterType(const QString& str)
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
int DataModel::ModbusMap::registersForDataType(const QString& dataType)
{
  if (dataType == QLatin1String("float32") || dataType == QLatin1String("uint32")
      || dataType == QLatin1String("int32"))
    return 2;
  if (dataType == QLatin1String("float64") || dataType == QLatin1String("uint64")
      || dataType == QLatin1String("int64"))
    return 4;

  return 1;
}

//--------------------------------------------------------------------------------------------------
// CSV parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a CSV register map with header-based column auto-detection, appending to @p out.
 *        Returns true when @p out holds at least one entry, so a caller may probe several formats
 *        against the same accumulator.
 */
bool DataModel::ModbusMap::parseCsv(const QString& path, QVector<RegisterEntry>& out)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  if (file.size() > kMaxImportFileBytes)
    return false;

  const QString content = QString::fromUtf8(file.readAll());
  file.close();

  const auto lines = content.split('\n', Qt::SkipEmptyParts);
  if (lines.count() < 2)
    return false;

  const auto map = ::detail::modbusmap::buildCsvColumnMap(lines[0].trimmed().split(','));
  if (map.addr < 0)
    return false;

  for (int row = 1; row < lines.count(); ++row) {
    const auto line = lines[row].trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      continue;

    RegisterEntry entry;
    if (!::detail::modbusmap::parseCsvRow(line.split(','), map, row, entry))
      continue;

    out.append(entry);
  }

  return !out.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// XML parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses an XML register map (flat or nested by register type), appending to @p out.
 */
bool DataModel::ModbusMap::parseXml(const QString& path, QVector<RegisterEntry>& out)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  if (file.size() > kMaxImportFileBytes)
    return false;

  QXmlStreamReader xml(&file);
  int current_type = -1;

  while (!xml.atEnd() && !xml.hasError()) {
    const auto token = xml.readNext();
    if (token != QXmlStreamReader::StartElement)
      continue;

    const auto tag_name = xml.name().toString().toLower();

    const int container_type = ::detail::modbusmap::xmlTagToType(tag_name);
    if (container_type >= 0) {
      current_type = container_type;
      continue;
    }

    if (tag_name != QLatin1String("register"))
      continue;

    RegisterEntry entry;
    if (::detail::modbusmap::parseXmlRegisterElement(xml, current_type, entry))
      out.append(entry);
  }

  file.close();
  return !out.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// JSON parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a JSON register map (flat registers array or grouped by type), appending to @p out.
 */
bool DataModel::ModbusMap::parseJson(const QString& path, QVector<RegisterEntry>& out)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  const auto result = Misc::JsonValidator::parseAndValidate(file.readAll());
  file.close();

  if (!result.valid) {
    qWarning() << "[ModbusMapImporter] Invalid JSON map:" << result.errorMessage;
    return false;
  }

  if (!result.document.isObject())
    return false;

  const auto root = result.document.object();

  if (root.contains(QStringLiteral("registers"))) {
    const auto arr = root.value(QStringLiteral("registers")).toArray();
    for (const auto& item : arr) {
      RegisterEntry entry;
      if (::detail::modbusmap::parseRegisterEntry(item.toObject(), entry))
        out.append(entry);
    }
  }

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
      if (::detail::modbusmap::parseRegisterEntry(item.toObject(), entry, g.type))
        out.append(entry);
    }
  }

  return !out.isEmpty();
}
