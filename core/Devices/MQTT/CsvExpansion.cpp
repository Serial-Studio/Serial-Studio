/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "MQTT/CsvExpansion.h"

#  include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Field and label formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Escapes a single field per RFC 4180; returns the original string if no escape is needed.
 *        The tab is escaped alongside the delimiters because a spreadsheet importing the payload
 *        with tab-separated settings would otherwise split the field.
 */
QString MQTT::escapeCsvField(const QString& field)
{
  const bool needs = field.contains(QChar(',')) || field.contains(QChar('"'))
                  || field.contains(QChar('\n')) || field.contains(QChar('\r'))
                  || field.contains(QChar('\t'));
  if (!needs)
    return field;

  QString out = field;
  out.replace(QChar('"'), QStringLiteral("\"\""));
  return QStringLiteral("\"%1\"").arg(out);
}

/**
 * @brief Label one exported column is published under: "group/dataset", qualified with the source
 *        title when the project reads from more than one. The CSV header and the Sparkplug metric
 *        registry both call this, so the same dataset carries the same identity whichever way the
 *        project pushes it out.
 */
QString MQTT::csvColumnLabel(const DataModel::ExportColumn& column)
{
  SS_ASSERT_LOG(column.uniqueId >= 0);

  const QString label = QStringLiteral("%1/%2").arg(column.groupTitle, column.title).simplified();
  if (column.sourceTitle.isEmpty())
    return label;

  return column.sourceTitle + QStringLiteral("/") + label;
}

//--------------------------------------------------------------------------------------------------
// Row and header assembly
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the retained header payload for a schema, or an empty array when the schema
 *        carries no columns: a header row of nothing would publish a lone newline that a consumer
 *        cannot tell from an empty frame.
 */
QByteArray MQTT::buildCsvHeader(const DataModel::ExportSchema& schema)
{
  QByteArray header;
  if (schema.columns.empty())
    return header;

  header.reserve(256);
  for (std::size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0)
      header.append(',');

    header.append(MQTT::escapeCsvField(csvColumnLabel(schema.columns[i])).toUtf8());
  }

  header.append('\n');
  return header;
}

/**
 * @brief Appends one row to @p out, one field per schema column in schema order. A dataset with no
 *        entry in @p valuesByUniqueId writes an empty field rather than shifting the row, so every
 *        row stays aligned with the header the broker retains.
 */
void MQTT::appendCsvRow(QByteArray& out,
                        const DataModel::ExportSchema& schema,
                        const QMap<int, QString>& valuesByUniqueId)
{
  SS_ASSERT(!schema.columns.empty(), return);

  for (std::size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0)
      out.append(',');

    const int uid = schema.columns[i].uniqueId;
    out.append(MQTT::escapeCsvField(valuesByUniqueId.value(uid, QString())).toUtf8());
  }

  out.append('\n');
}

//--------------------------------------------------------------------------------------------------
// Block materialisation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Materialises the batch's blocks into one frame per sample, capped at @p maxSamples: a
 *        dense source can present millions of samples in one batch, and MQTT is a live feed, not a
 *        lossless recorder. A block whose source has published no structure yet is skipped, because
 *        the template is the published shape and a block may not invent fields in it.
 */
void MQTT::expandBlocks(const std::vector<DataModel::DataBlockPtr>& blocks,
                        std::map<int, DataModel::FrameTemplate>& templates,
                        std::size_t maxSamples,
                        std::vector<DataModel::Frame>& out)
{
  SS_ASSERT(maxSamples > 0, return);

  out.clear();
  for (const auto& block : blocks) {
    if (!block || block->samples <= 0)
      continue;

    const auto tpl = templates.find(block->sourceId);
    if (tpl == templates.end())
      continue;

    for (qsizetype i = 0; i < block->samples; ++i) {
      if (out.size() >= maxSamples)
        return;

      DataModel::apply_block_sample(tpl->second, *block, i);
      out.push_back(tpl->second.frame);
    }
  }
}

#endif  // BUILD_COMMERCIAL
