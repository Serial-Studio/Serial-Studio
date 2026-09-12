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

#include "ProjectEditor/EditorInflux.h"

#include "API/HandlerContext.h"
#include "Core/IconRegistry.h"
#include "Core/Services.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditorItemIds.h"

#ifdef BUILD_COMMERCIAL
#  include "InfluxDB/Export.h"
#endif

namespace DataModel {

using enum ProjectEditor::CustomRoles;
using enum ProjectEditor::EditorWidget;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the export object; the object's own change signals rebuild the
 *        form only while its view is current and never inside one of this form's own edits.
 */
EditorInflux::EditorInflux(ProjectEditor& editor)
  : m_editor(editor)
  , m_applying(false)
#ifdef BUILD_COMMERCIAL
  , m_export(API::handlerContext().influxExport)
#endif
{
#ifdef BUILD_COMMERCIAL
  const auto refresh = [this] {
    if (m_applying || m_editor.m_currentView != ProjectEditor::InfluxSinkView)
      return;

    buildInfluxSinkModel();
  };
  QObject::connect(&m_export, &InfluxDB::Export::configurationChanged, &m_editor, refresh);
  QObject::connect(&m_export, &InfluxDB::Export::enabledChanged, &m_editor, refresh);
#endif
}

//--------------------------------------------------------------------------------------------------
// Model construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the InfluxDB Sink form model from the live export object.
 */
void EditorInflux::buildInfluxSinkModel()
{
  if (m_editor.m_influxSinkModel) {
    m_editor.m_influxSinkModel->disconnect(&m_editor);
    m_editor.m_influxSinkModel->deleteLater();
    m_editor.m_influxSinkModel = nullptr;
  }

  m_editor.m_influxSinkModel = new CustomModel(&m_editor);

#ifdef BUILD_COMMERCIAL
  const bool enabled = m_export.exportEnabled();
  buildSinkSection(enabled);
  buildServerSection(enabled);
#endif

  QObject::connect(m_editor.m_influxSinkModel,
                   &CustomModel::itemChanged,
                   &m_editor,
                   [this](QStandardItem* item) { onInfluxSinkItemChanged(item); });

  Q_EMIT m_editor.influxSinkModelChanged();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Appends the Sink section: the master toggle.
 */
void EditorInflux::buildSinkSection(bool enabled)
{
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("Publishing"), PlaceholderValue);
  auto& registry = Core::services().iconRegistry;
  header->setData(registry.iconById(QStringLiteral("editor/output-range"), 16), ParameterIcon);
  m_editor.m_influxSinkModel->appendRow(header);

  auto* enabledItem = new QStandardItem();
  enabledItem->setEditable(true);
  enabledItem->setData(true, Active);
  enabledItem->setData(CheckBox, WidgetType);
  enabledItem->setData(enabled, EditableValue);
  enabledItem->setData(kInfluxSink_Enabled, ParameterType);
  enabledItem->setData(tr("Enabled"), ParameterName);
  enabledItem->setData(tr("Write every published block to InfluxDB while connected"),
                       ParameterDescription);
  m_editor.m_influxSinkModel->appendRow(enabledItem);
}

/**
 * @brief Appends the Server section: URL, organization, bucket, measurement and the write-only
 *        token (never read back from the vault into the form).
 */
void EditorInflux::buildServerSection(bool enabled)
{
  auto* header = new QStandardItem();
  header->setData(SectionHeader, WidgetType);
  header->setData(tr("Server"), PlaceholderValue);
  auto& registry = Core::services().iconRegistry;
  header->setData(registry.iconById(QStringLiteral("editor/mqtt-broker"), 16), ParameterIcon);
  m_editor.m_influxSinkModel->appendRow(header);

  appendTextRow(kInfluxSink_Url,
                tr("Server URL"),
                m_export.url(),
                QStringLiteral("http://localhost:8086"),
                tr("Base URL of the InfluxDB 2.x HTTP API"),
                enabled);
  appendTextRow(kInfluxSink_Organization,
                tr("Organization"),
                m_export.organization(),
                QStringLiteral("my-org"),
                tr("InfluxDB organization that owns the bucket"),
                enabled);
  appendTextRow(kInfluxSink_Bucket,
                tr("Bucket"),
                m_export.bucket(),
                QStringLiteral("telemetry"),
                tr("Bucket the points are written into"),
                enabled);
  appendTextRow(kInfluxSink_Measurement,
                tr("Measurement"),
                m_export.measurement(),
                QStringLiteral("serial_studio"),
                tr("Measurement name every point is filed under"),
                enabled);

  auto* tokenItem = new QStandardItem();
  tokenItem->setEditable(true);
  tokenItem->setData(enabled, Active);
  tokenItem->setData(PasswordField, WidgetType);
  tokenItem->setData(QString(), EditableValue);
  tokenItem->setData(kInfluxSink_Token, ParameterType);
  tokenItem->setData(tr("API Token"), ParameterName);
  tokenItem->setData(m_export.hasToken() ? tr("Stored; type a new token to replace it")
                                         : tr("Paste the InfluxDB API token"),
                     PlaceholderValue);
  tokenItem->setData(tr("Kept obfuscated in this machine's settings, never in the project file"),
                     ParameterDescription);
  m_editor.m_influxSinkModel->appendRow(tokenItem);
}

/**
 * @brief Appends one editable text row.
 */
void EditorInflux::appendTextRow(int type,
                                 const QString& name,
                                 const QString& value,
                                 const QString& placeholder,
                                 const QString& description,
                                 bool enabled)
{
  auto* item = new QStandardItem();
  item->setEditable(true);
  item->setData(enabled, Active);
  item->setData(TextField, WidgetType);
  item->setData(value, EditableValue);
  item->setData(type, ParameterType);
  item->setData(name, ParameterName);
  item->setData(placeholder, PlaceholderValue);
  item->setData(description, ParameterDescription);
  m_editor.m_influxSinkModel->appendRow(item);
}
#endif

//--------------------------------------------------------------------------------------------------
// Edit handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pushes one edited row back into the export object; the toggle and a new token rebuild
 *        the form (rows enable/disable, the token field clears), the rest edit in place.
 */
void EditorInflux::onInfluxSinkItemChanged(QStandardItem* item)
{
  if (!item)
    return;

#ifdef BUILD_COMMERCIAL
  const auto type  = item->data(ParameterType).toInt();
  const auto value = item->data(EditableValue);

  m_applying = true;
  switch (type) {
    case kInfluxSink_Enabled:
      m_export.setExportEnabled(value.toBool());
      m_applying = false;
      buildInfluxSinkModel();
      return;
    case kInfluxSink_Url:
      m_export.setUrl(value.toString());
      break;
    case kInfluxSink_Organization:
      m_export.setOrganization(value.toString());
      break;
    case kInfluxSink_Bucket:
      m_export.setBucket(value.toString());
      break;
    case kInfluxSink_Measurement:
      m_export.setMeasurement(value.toString());
      break;
    case kInfluxSink_Token:
      if (!value.toString().isEmpty())
        m_export.setToken(value.toString());

      m_applying = false;
      buildInfluxSinkModel();
      return;
    default:
      break;
  }

  m_applying = false;
#else
  Q_UNUSED(item)
#endif
}

}  // namespace DataModel
