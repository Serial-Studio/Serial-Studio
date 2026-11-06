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

//--------------------------------------------------------------------------------------------------
// Presentation rows
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends one text row with a placeholder shown while it is empty.
 */
static void appendTextRow(QStandardItemModel* model,
                          const QString& value,
                          const int parameter,
                          const QString& name,
                          const QString& placeholder)
{
  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(TextField, WidgetType);
  item->setData(value, EditableValue);
  item->setData(parameter, ParameterType);
  item->setData(name, ParameterName);
  item->setData(placeholder, PlaceholderValue);
  model->appendRow(item);
}

/**
 * @brief Appends the knob's style row and, for a selector, its position labels. A selector has
 *        one detent per Step from Min to Max, so three positions are Min 0, Max 2, Step 1.
 */
static void appendSelectorRows(QStandardItemModel* model, const DataModel::OutputWidget& widget)
{
  const bool selector = widget.style == DataModel::OutputWidgetStyle::Selector;

  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(ComboBox, WidgetType);
  item->setData(
    QStringList{QCoreApplication::translate("DataModel::ProjectEditor", "Standard"),
                QCoreApplication::translate("DataModel::ProjectEditor", "Rotary Selector")},
    ComboBoxData);
  item->setData(selector ? 1 : 0, EditableValue);
  item->setData(kOutputWidget_Style, ParameterType);
  item->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Knob Style"),
                ParameterName);
  item->setData(QCoreApplication::translate(
                  "DataModel::ProjectEditor",
                  "A rotary selector has one labelled position per Step from Min to Max"),
                ParameterDescription);
  model->appendRow(item);

  if (selector)
    appendTextRow(
      model,
      widget.positionLabels,
      kOutputWidget_PositionLabels,
      QCoreApplication::translate("DataModel::ProjectEditor", "Position Labels"),
      QCoreApplication::translate("DataModel::ProjectEditor",
                                  "Comma-separated, one per position (Off, Low, High)"));
}

/**
 * @brief Appends what a text field lets the user dress: the placeholder, and the send button's
 *        caption, icon and colour. They reuse the off label, on label, icon and colour a button
 *        already stores, so the project file gains no key.
 */
static void appendTextFieldRows(QStandardItemModel* model, const DataModel::OutputWidget& widget)
{
  appendTextRow(
    model,
    widget.offLabel,
    kOutputWidget_OffLabel,
    QCoreApplication::translate("DataModel::ProjectEditor", "Placeholder Text"),
    QCoreApplication::translate("DataModel::ProjectEditor", "Shown while the field is empty"));
  appendTextRow(model,
                widget.onLabel,
                kOutputWidget_OnLabel,
                QCoreApplication::translate("DataModel::ProjectEditor", "Button Text"),
                QCoreApplication::translate("DataModel::ProjectEditor", "Send"));
  appendRow(model,
            IconPicker,
            widget.icon,
            kOutputWidget_Icon,
            QCoreApplication::translate("DataModel::ProjectEditor", "Button Icon"));

  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(true, Active);
  item->setData(ColorPicker, WidgetType);
  item->setData(widget.color, EditableValue);
  item->setData(kOutputWidget_Color, ParameterType);
  item->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Automatic"),
                PlaceholderValue);
  item->setData(QCoreApplication::translate("DataModel::ProjectEditor", "Button Color"),
                ParameterName);
  model->appendRow(item);
}

/**
 * @brief Appends the presentation rows of spec 0113; see the header.
 */
void DataModel::appendOutputPresentationRows(QStandardItemModel* model, const OutputWidget& widget)
{
  SS_ASSERT_LOG(model != nullptr);

  appendRow(model,
            CheckBox,
            !widget.hideTitle,
            kOutputWidget_ShowTitle,
            QCoreApplication::translate("DataModel::ProjectEditor", "Show Title"),
            QCoreApplication::translate(
              "DataModel::ProjectEditor",
              "Draw the control's title and frame; turn off for a control that stands alone"));

  if (widget.type == OutputWidgetType::Knob)
    appendSelectorRows(model, widget);

  if (widget.type == OutputWidgetType::TextField)
    appendTextFieldRows(model, widget);
}
