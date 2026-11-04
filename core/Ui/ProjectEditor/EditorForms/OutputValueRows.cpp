/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "ProjectEditor/EditorForms/OutputValueRows.h"

#include <QCoreApplication>
#include <QStandardItemModel>

#include "Core/IconRegistry.h"
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
 * @brief Appends one editable row of the given widget kind.
 */
static void appendRow(QStandardItemModel* model,
                      const int widgetType,
                      const QVariant& value,
                      const int parameter,
                      const QString& name,
                      const QString& description = QString())
{
  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(widgetType, WidgetType);
  item->setData(value, EditableValue);
  item->setData(parameter, ParameterType);
  item->setData(name, ParameterName);
  if (!description.isEmpty())
    item->setData(description, ParameterDescription);

  model->appendRow(item);
}

/**
 * @brief Appends the Decimals row, bounded to what the commit and normalize() accept.
 */
static void appendDecimalsRow(QStandardItemModel* model, const DataModel::OutputWidget& widget)
{
  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(IntField, WidgetType);
  item->setData(-1, MinValue);
  item->setData(DataModel::kMaxOutputDecimals, MaxValue);
  item->setData(widget.decimals, EditableValue);
  item->setData(kOutputWidget_Decimals, ParameterType);
  item->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Decimals"), ParameterName);
  item->setData(QCoreApplication::translate("DataModel::ProjectEditor",
                                            "Decimals shown for the value, -1 to pick from Step"),
                ParameterDescription);
  model->appendRow(item);
}

//--------------------------------------------------------------------------------------------------
// Rows
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends the Value Range section: nothing for a button or toggle, the Input row for a text
 *        field, and the range rows for every control the admission rule governs.
 */
void DataModel::appendOutputValueRows(QStandardItemModel* model, const OutputWidget& widget)
{
  SS_ASSERT(model != nullptr, return);

  const bool textField = widget.type == OutputWidgetType::TextField;
  if (!textField && !outputIsNumeric(widget))
    return;

  auto& registry = Core::services().iconRegistry;
  auto* header   = new QStandardItem();
  header->setData(true, Active);
  header->setData(SectionHeader, WidgetType);
  header->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Value Range"),
                  PlaceholderValue);
  header->setData(registry.icon(QStringLiteral("editor"), QStringLiteral("output-range"), 16),
                  ParameterIcon);
  model->appendRow(header);

  if (textField) {
    auto* kindItem = new QStandardItem();
    kindItem->setEditable(true);
    kindItem->setData(true, Active);
    kindItem->setData(ComboBox, WidgetType);
    kindItem->setData(
      QStringList{QCoreApplication::translate("DataModel::ProjectEditor", "Text"),
                  QCoreApplication::translate("DataModel::ProjectEditor", "Number")},
      ComboBoxData);
    kindItem->setData(static_cast<int>(widget.inputKind), EditableValue);
    kindItem->setData(kOutputWidget_InputKind, ParameterType);
    kindItem->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Input"),
                      ParameterName);
    kindItem->setData(QCoreApplication::translate(
                        "DataModel::ProjectEditor",
                        "Text sends what is typed. Number reads it as a number and refuses a "
                        "value outside Min to Max before the transmit function runs."),
                      ParameterDescription);
    model->appendRow(kindItem);
  }

  if (!outputIsNumeric(widget))
    return;

  appendRow(model,
            FloatField,
            widget.minValue,
            kOutputWidget_MinValue,
            QCoreApplication::translate("DataModel::ProjectEditor", "Min"));
  appendRow(model,
            FloatField,
            widget.maxValue,
            kOutputWidget_MaxValue,
            QCoreApplication::translate("DataModel::ProjectEditor", "Max"));
  appendRow(model,
            FloatField,
            widget.stepSize,
            kOutputWidget_StepSize,
            QCoreApplication::translate("DataModel::ProjectEditor", "Step"));
  appendDecimalsRow(model, widget);
  appendRow(model,
            TextField,
            widget.units,
            kOutputWidget_Units,
            QCoreApplication::translate("DataModel::ProjectEditor", "Units"));

  if (textField)
    appendRow(model,
              CheckBox,
              widget.quantize,
              kOutputWidget_Quantize,
              QCoreApplication::translate("DataModel::ProjectEditor", "Quantize to Step"),
              QCoreApplication::translate(
                "DataModel::ProjectEditor",
                "Snap a typed value to the nearest Step counted from Min before the range check"));
}
