/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "ProjectEditor/EditorForms/SourceFramingRows.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>

#include "Core/IconRegistry.h"
#include "Core/IO/FrameConfig.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditor/ProjectEditorItemIds.h"

using enum DataModel::ProjectEditor::CustomRoles;
using enum DataModel::ProjectEditor::EditorWidget;

//--------------------------------------------------------------------------------------------------
// Row builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends one Fixed Length row: the frame length (IntField) or a one-byte hex rule.
 */
static void appendFixedFrameRow(QStandardItemModel* model,
                                const int widget,
                                const int id,
                                const int value,
                                const QString& name,
                                const QString& description)
{
  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(widget, WidgetType);
  item->setData(id, ParameterType);
  item->setData(name, ParameterName);
  item->setData(description, ParameterDescription);

  if (widget == IntField) {
    item->setData(1, MinValue);
    item->setData(IO::kMaxFixedFrameLength, MaxValue);
    item->setData(value, EditableValue);
  } else {
    item->setData(QStringLiteral("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper(),
                  EditableValue);
  }

  model->appendRow(item);
}

/**
 * @brief Appends the Fixed Length rows: frame length, Header Rule and Body Rule (mask, value).
 */
static void appendFixedFrameRows(QStandardItemModel* model, const DataModel::Source& source)
{
  appendFixedFrameRow(
    model,
    IntField,
    kSourceView_FrameLength,
    source.frameLength,
    QCoreApplication::translate("DataModel::ProjectEditor", "Frame Length"),
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Number of bytes in every frame, checksum included"));
  appendFixedFrameRow(
    model,
    HexTextField,
    kSourceView_FrameHeaderMask,
    source.frameHeaderMask,
    QCoreApplication::translate("DataModel::ProjectEditor", "Header Rule Mask"),
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Bits of a frame's first byte that must match; 00 disables"));
  appendFixedFrameRow(
    model,
    HexTextField,
    kSourceView_FrameHeaderValue,
    source.frameHeaderValue,
    QCoreApplication::translate("DataModel::ProjectEditor", "Header Rule Value"),
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Value the masked bits of the first byte must equal"));
  appendFixedFrameRow(
    model,
    HexTextField,
    kSourceView_FrameBodyMask,
    source.frameBodyMask,
    QCoreApplication::translate("DataModel::ProjectEditor", "Body Rule Mask"),
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Bits of the other bytes that must match; 00 disables"));
  appendFixedFrameRow(
    model,
    HexTextField,
    kSourceView_FrameBodyValue,
    source.frameBodyValue,
    QCoreApplication::translate("DataModel::ProjectEditor", "Body Rule Value"),
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Value the masked bits of every byte after the first must equal"));
}

/**
 * @brief Appends one frame delimiter row, a hex or a plain text field per @p hex.
 */
static void appendDelimiterRow(QStandardItemModel* model,
                               const bool hex,
                               const QString& value,
                               const int id,
                               const QString& name,
                               const QString& placeholder,
                               const QString& description)
{
  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(hex ? HexTextField : TextField, WidgetType);
  item->setData(value, EditableValue);
  item->setData(id, ParameterType);
  item->setData(name, ParameterName);
  item->setData(placeholder, PlaceholderValue);
  item->setData(description, ParameterDescription);
  model->appendRow(item);
}

/**
 * @brief Appends the Hexadecimal Delimiters checkbox and the start/end delimiter rows the
 *        detection method uses.
 */
static void appendDelimiterRows(QStandardItemModel* model,
                                const DataModel::Source& source,
                                const SerialStudio::FrameDetection detection)
{
  const bool hexDelimiters = source.hexadecimalDelimiters;

  auto* hexSeqItem = new QStandardItem();
  hexSeqItem->setEditable(true);
  hexSeqItem->setData(true, Active);
  hexSeqItem->setData(CheckBox, WidgetType);
  hexSeqItem->setData(hexDelimiters, EditableValue);
  hexSeqItem->setData(kSourceView_HexadecimalSequence, ParameterType);
  hexSeqItem->setData(
    QCoreApplication::translate("DataModel::ProjectEditor", "Hexadecimal Delimiters"),
    ParameterName);
  hexSeqItem->setData(
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Enter frame start/end sequences as hexadecimal values"),
    ParameterDescription);
  model->appendRow(hexSeqItem);

  const bool showStart = (detection == SerialStudio::StartDelimiterOnly
                          || detection == SerialStudio::StartAndEndDelimiter);
  const bool showEnd   = (detection == SerialStudio::EndDelimiterOnly
                          || detection == SerialStudio::StartAndEndDelimiter);

  if (showStart)
    appendDelimiterRow(
      model,
      hexDelimiters,
      source.frameStart,
      kSourceView_FrameStartSequence,
      QCoreApplication::translate("DataModel::ProjectEditor", "Frame Start Delimiter"),
      QCoreApplication::translate("DataModel::ProjectEditor", "e.g. /*"),
      QCoreApplication::translate("DataModel::ProjectEditor",
                                  "Sequence that marks the beginning of a data frame"));

  if (showEnd)
    appendDelimiterRow(
      model,
      hexDelimiters,
      source.frameEnd,
      kSourceView_FrameEndSequence,
      QCoreApplication::translate("DataModel::ProjectEditor", "Frame End Delimiter"),
      QCoreApplication::translate("DataModel::ProjectEditor", "e.g. */"),
      QCoreApplication::translate("DataModel::ProjectEditor",
                                  "Sequence that marks the end of a data frame"));
}

//--------------------------------------------------------------------------------------------------
// Rows
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the Frame Detection header and method picker, then the Fixed Length rows or the
 *        delimiter rows the selected method needs.
 */
void DataModel::appendSourceFramingRows(QStandardItemModel* model,
                                        const Source& source,
                                        const QStringList& methods,
                                        const QList<SerialStudio::FrameDetection>& methodValues)
{
  SS_ASSERT(model != nullptr, return);
  SS_ASSERT_LOG(methods.size() == methodValues.size());

  auto& registry = Core::services().iconRegistry;
  auto* fdHdr    = new QStandardItem();
  fdHdr->setData(SectionHeader, WidgetType);
  fdHdr->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Frame Detection"),
                 PlaceholderValue);
  fdHdr->setData(registry.iconById(QStringLiteral("editor/frame-detection"), 16), ParameterIcon);
  model->appendRow(fdHdr);

  const auto detection = static_cast<SerialStudio::FrameDetection>(source.frameDetection);

  auto* frameDetectionItem = new QStandardItem();
  frameDetectionItem->setEditable(true);
  frameDetectionItem->setData(true, Active);
  frameDetectionItem->setData(ComboBox, WidgetType);
  frameDetectionItem->setData(methods, ComboBoxData);
  frameDetectionItem->setData(methodValues.indexOf(detection), EditableValue);
  frameDetectionItem->setData(kSourceView_FrameDetection, ParameterType);
  frameDetectionItem->setData(
    QCoreApplication::translate("DataModel::ProjectEditor", "Frame Detection Method"),
    ParameterName);
  frameDetectionItem->setData(
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Select how incoming data frames are identified"),
    ParameterDescription);
  model->appendRow(frameDetectionItem);

  if (detection == SerialStudio::FixedLength)
    appendFixedFrameRows(model, source);
  else
    appendDelimiterRows(model, source, detection);
}

/**
 * @brief Appends the Connection Sequence button row, its label carrying the step count read from
 *        the driver's opaque connectionSequence property.
 */
void DataModel::appendConnectionSequenceRow(QStandardItemModel* model,
                                            const QList<IO::DriverProperty>& props)
{
  SS_ASSERT(model != nullptr, return);

  qsizetype steps = 0;
  for (const auto& prop : props)
    if (prop.key == QLatin1String("connectionSequence"))
      steps = prop.value.toJsonObject().value(QStringLiteral("steps")).toArray().size();

  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(Button, WidgetType);
  item->setData(kSourceView_ConnectionSequence, ParameterType);
  item->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Connection Sequence"),
                ParameterName);
  item->setData(
    QCoreApplication::translate("DataModel::ProjectEditor",
                                "Steps run on every open, before any byte reaches the dashboard"),
    ParameterDescription);
  const QString label =
    steps == 1
      ? QCoreApplication::translate("DataModel::ProjectEditor", "Connection Sequence… (1 step)")
      : QCoreApplication::translate("DataModel::ProjectEditor", "Connection Sequence… (%1 steps)")
          .arg(steps);
  item->setData(label, PlaceholderValue);
  model->appendRow(item);
}
