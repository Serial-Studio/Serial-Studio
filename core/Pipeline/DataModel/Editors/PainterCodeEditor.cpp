/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "DataModel/Editors/PainterCodeEditor.h"

#  include <QDir>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QInputDialog>
#  include <QInputMethod>
#  include <QJsonArray>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QMessageBox>
#  include <QVariantList>
#  include <QVariantMap>

#  include "DataModel/Editors/EditorFormatting.h"
#  include "DataModel/ProjectEditor.h"
#  include "DataModel/ProjectModel.h"
#  include "Misc/Translator.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side painter widget code editor. It lives in the Project Editor
 *        window, which stays instantiated once opened, hence the window-visibility render gate.
 */
DataModel::PainterCodeEditor::PainterCodeEditor(QQuickItem* parent)
  : EmbeddedCodeEditorItem(EmbeddedCodeEditor::RenderGate::WindowVisible, parent)
  , m_readingCode(false)
  , m_translator(Misc::Translator::instance())
  , m_projectEditor(DataModel::ProjectEditor::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_templates(QStringLiteral(":/scripts/painter/templates.json"),
                QStringLiteral(":/scripts/painter"),
                QStringLiteral(".js"),
                "DataModel::PainterCodeEditor")
{
  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::PainterCodeEditor::textChanged);

  connect(&widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    if (!m_projectEditor.currentGroupIsPainter())
      return;

    m_projectEditor.setCurrentGroupPainterCode(text());
  });

  connect(&m_translator,
          &Misc::Translator::languageChanged,
          this,
          &DataModel::PainterCodeEditor::loadTemplates);

  connect(&m_projectModel, &DataModel::ProjectModel::groupDataChanged, this, [this] {
    if (m_readingCode)
      return;

    if (!m_projectEditor.currentGroupIsPainter())
      return;

    const QString live = m_projectEditor.currentGroupPainterCode();
    if (live != text())
      readCode();
  });

  loadTemplates();
  readCode();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::PainterCodeEditor::text() const
{
  return m_editor.text();
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::PainterCodeEditor::isModified() const noexcept
{
  return m_editor.isModified();
}

/**
 * @brief Returns true when an undo step is available.
 */
bool DataModel::PainterCodeEditor::undoAvailable() const noexcept
{
  return m_editor.undoAvailable();
}

/**
 * @brief Returns true when a redo step is available.
 */
bool DataModel::PainterCodeEditor::redoAvailable() const noexcept
{
  return m_editor.redoAvailable();
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::PainterCodeEditor::cut()
{
  m_editor.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::PainterCodeEditor::undo()
{
  m_editor.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::PainterCodeEditor::redo()
{
  m_editor.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::PainterCodeEditor::copy()
{
  m_editor.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::PainterCodeEditor::paste()
{
  m_editor.paste();
}

/**
 * @brief Force-flushes the current editor text to the selected painter group.
 */
void DataModel::PainterCodeEditor::commit()
{
  if (m_readingCode)
    return;

  if (auto* ime = QGuiApplication::inputMethod()) {
    if (ime->isVisible() || !ime->inputItemRectangle().isEmpty())
      ime->commit();
  }

  if (!m_projectEditor.currentGroupIsPainter())
    return;

  m_projectEditor.setCurrentGroupPainterCode(text());
}

/**
 * @brief Selects all editor text.
 */
void DataModel::PainterCodeEditor::selectAll()
{
  m_editor.selectAll();
}

/**
 * @brief Reformats the entire painter source.
 */
void DataModel::PainterCodeEditor::formatDocument()
{
  EditorFormatting::formatDocument(m_editor.widget(), CodeFormatter::Language::JavaScript);
}

/**
 * @brief Reformats the selected lines (or current line when nothing is selected).
 */
void DataModel::PainterCodeEditor::formatSelection()
{
  EditorFormatting::formatSelection(m_editor.widget(), CodeFormatter::Language::JavaScript);
}

/**
 * @brief Opens a file dialog to import an external JS file.
 */
void DataModel::PainterCodeEditor::importFile()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select Javascript file to import"), QDir::homePath(), QStringLiteral("*.js"));
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this, [this, path]() { (void)m_editor.importFromFile(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Loads the painter code from the currently selected group.
 */
void DataModel::PainterCodeEditor::readCode()
{
  if (m_readingCode)
    return;

  m_readingCode = true;

  QString code = m_projectEditor.currentGroupPainterCode();
  if (code.isEmpty())
    code = defaultTemplate();

  m_editor.setSourceText(code);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

//--------------------------------------------------------------------------------------------------
// Templates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the painter template manifest and returns the `datasets` array
 *        attached to a given template file (empty when not declared).
 */
static QVariantList templateDatasetSpecs(const QString& templateFile)
{
  QFile manifest(QStringLiteral(":/scripts/painter/templates.json"));
  if (!manifest.open(QFile::ReadOnly))
    return {};

  const auto doc = QJsonDocument::fromJson(manifest.readAll());
  if (!doc.isArray())
    return {};

  for (const auto& v : doc.array()) {
    const auto obj = v.toObject();
    if (obj.value(QStringLiteral("file")).toString() != templateFile)
      continue;

    const auto arr = obj.value(QStringLiteral("datasets")).toArray();
    QVariantList specs;
    specs.reserve(arr.size());
    for (const auto& d : arr)
      specs.append(d.toObject().toVariantMap());

    return specs;
  }
  return {};
}

/**
 * @brief Shows a dialog to pick and load a built-in painter template, then
 *        offers to add the datasets the chosen template expects.
 */
void DataModel::PainterCodeEditor::selectTemplate()
{
  if (m_templates.isEmpty())
    return;

  bool ok;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Canvas Widget Template"),
                                          tr("Choose a template to load:"),
                                          m_templates.names(),
                                          0,
                                          false,
                                          &ok);

  if (!ok)
    return;

  const int idx = m_templates.names().indexOf(name);
  if (idx < 0 || idx >= m_templates.files().size())
    return;

  QFile file(m_templates.files().at(idx));
  if (file.open(QFile::ReadOnly)) {
    m_editor.setSourceText(QString::fromUtf8(file.readAll()));
    Q_EMIT modifiedChanged();
    file.close();
  }

  const QString templateFile = QFileInfo(m_templates.files().at(idx)).completeBaseName();
  const auto specs           = templateDatasetSpecs(templateFile);
  if (specs.isEmpty())
    return;

  if (!m_projectEditor.currentGroupIsPainter())
    return;

  const auto& groups = m_projectModel.groups();
  const int gid      = m_projectEditor.currentGroupId();
  if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
    return;

  const int existing = static_cast<int>(groups[gid].datasets.size());
  const int missing  = qMax(0, specs.size() - existing);
  if (missing == 0)
    return;

  const auto choice =
    QMessageBox::question(nullptr,
                          tr("Add datasets for this template?"),
                          tr("\"%1\" expects %2 dataset(s); the current group has %3.\n\n"
                             "Add %4 dataset(s) using the template's defaults?")
                            .arg(name)
                            .arg(specs.size())
                            .arg(existing)
                            .arg(missing),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::Yes);

  if (choice == QMessageBox::Yes)
    m_projectModel.ensurePainterDatasets(gid, specs);
}

/**
 * @brief Resets the editor to the default painter template.
 */
void DataModel::PainterCodeEditor::reload(bool guiTrigger)
{
  Q_UNUSED(guiTrigger)
  m_editor.setSourceText(defaultTemplate());
  Q_EMIT modifiedChanged();
}

/**
 * @brief Returns the bundled default-template source.
 */
QString DataModel::PainterCodeEditor::defaultTemplate()
{
  return defaultScriptTemplateCode(QStringLiteral(":/scripts/painter/templates.json"),
                                   QStringLiteral(":/scripts/painter"),
                                   QStringLiteral(".js"),
                                   "DataModel::PainterCodeEditor");
}

/**
 * @brief Rebuilds the cached list of painter templates from resources.
 */
void DataModel::PainterCodeEditor::loadTemplates()
{
  m_templates.reload();
}

#endif
