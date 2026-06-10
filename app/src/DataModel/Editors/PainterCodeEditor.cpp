/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "DataModel/Editors/PainterCodeEditor.h"

#  include <QAbstractItemView>
#  include <QCompleter>
#  include <QCoreApplication>
#  include <QDir>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QInputDialog>
#  include <QInputMethod>
#  include <QJavascriptHighlighter>
#  include <QJsonArray>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QLineNumberArea>
#  include <QMessageBox>
#  include <QTextCursor>
#  include <QTextDocument>
#  include <QVariantList>
#  include <QVariantMap>

#  include "DataModel/Editors/CodeFormatter.h"
#  include "DataModel/Editors/SerialStudioCompleter.h"
#  include "DataModel/ProjectEditor.h"
#  include "DataModel/ProjectModel.h"
#  include "DataModel/Scripting/ScriptTemplates.h"
#  include "Misc/CommonFonts.h"
#  include "Misc/ThemeManager.h"
#  include "Misc/TimerEvents.h"
#  include "Misc/Translator.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side painter widget code editor.
 */
DataModel::PainterCodeEditor::PainterCodeEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent), m_readingCode(false)
{
  setMipmap(false);
  setAntialiasing(false);
  setOpaquePainting(true);
  setAcceptTouchEvents(true);
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFillColor(Misc::ThemeManager::instance().getColor(QStringLiteral("base")));

  m_widget.setTabReplace(true);
  m_widget.setTabReplaceSize(2);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(Misc::CommonFonts::instance().monoFont());
  m_widget.setLayoutDirection(Qt::LeftToRight);
  m_widget.setLanguageHint(QCodeEditor::LanguageHint::JavaScript);
  m_widget.setCompleter(new DataModel::SerialStudioCompleter(false, &m_widget));

  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::PainterCodeEditor::onThemeChanged);

  connect(&m_widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::PainterCodeEditor::textChanged);

  connect(&m_widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    auto& editor = DataModel::ProjectEditor::instance();
    if (!editor.currentGroupIsPainter())
      return;

    editor.setCurrentGroupPainterCode(text());
  });

  connect(
    this, &QQuickPaintedItem::widthChanged, this, &DataModel::PainterCodeEditor::resizeWidget);
  connect(
    this, &QQuickPaintedItem::heightChanged, this, &DataModel::PainterCodeEditor::resizeWidget);
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::PainterCodeEditor::renderWidget);

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &DataModel::PainterCodeEditor::loadTemplates);

  connect(
    &DataModel::ProjectModel::instance(), &DataModel::ProjectModel::groupDataChanged, this, [this] {
      if (m_readingCode)
        return;

      auto& editor = DataModel::ProjectEditor::instance();
      if (!editor.currentGroupIsPainter())
        return;

      const QString live = editor.currentGroupPainterCode();
      if (live != m_widget.toPlainText())
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
  return m_widget.toPlainText();
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::PainterCodeEditor::isModified() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

/**
 * @brief Returns true when an undo step is available.
 */
bool DataModel::PainterCodeEditor::undoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isUndoAvailable();

  return false;
}

/**
 * @brief Returns true when a redo step is available.
 */
bool DataModel::PainterCodeEditor::redoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isRedoAvailable();

  return false;
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::PainterCodeEditor::cut()
{
  m_widget.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::PainterCodeEditor::undo()
{
  m_widget.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::PainterCodeEditor::redo()
{
  m_widget.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::PainterCodeEditor::copy()
{
  m_widget.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::PainterCodeEditor::paste()
{
  m_widget.paste();
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

  auto& editor = DataModel::ProjectEditor::instance();
  if (!editor.currentGroupIsPainter())
    return;

  editor.setCurrentGroupPainterCode(text());
}

/**
 * @brief Selects all editor text.
 */
void DataModel::PainterCodeEditor::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Reformats the entire painter source.
 */
void DataModel::PainterCodeEditor::formatDocument()
{
  const QString original = m_widget.toPlainText();
  const QString formatted =
    CodeFormatter::formatDocument(original, CodeFormatter::Language::JavaScript);
  if (formatted == original)
    return;

  QTextCursor cursor = m_widget.textCursor();
  const int savedPos = cursor.position();
  cursor.beginEditBlock();
  cursor.select(QTextCursor::Document);
  cursor.insertText(formatted);
  cursor.endEditBlock();

  cursor.setPosition(qMin(savedPos, formatted.size()));
  m_widget.setTextCursor(cursor);
}

/**
 * @brief Reformats the selected lines (or current line when nothing is selected).
 */
void DataModel::PainterCodeEditor::formatSelection()
{
  const QString original = m_widget.toPlainText();

  QTextCursor cursor = m_widget.textCursor();
  QTextCursor first(m_widget.document());
  first.setPosition(qMin(cursor.selectionStart(), cursor.selectionEnd()));
  QTextCursor last(m_widget.document());
  last.setPosition(qMax(cursor.selectionStart(), cursor.selectionEnd()));

  const int firstLine     = first.blockNumber();
  const int lastLine      = last.blockNumber();
  const QString formatted = CodeFormatter::formatLineRange(
    original, CodeFormatter::Language::JavaScript, firstLine, lastLine);
  if (formatted == original)
    return;

  const int savedPos = cursor.position();
  cursor.beginEditBlock();
  cursor.select(QTextCursor::Document);
  cursor.insertText(formatted);
  cursor.endEditBlock();

  cursor.setPosition(qMin(savedPos, formatted.size()));
  m_widget.setTextCursor(cursor);
}

/**
 * @brief Opens a file dialog to import an external JS file.
 */
void DataModel::PainterCodeEditor::import()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select Javascript file to import"), QDir::homePath(), QStringLiteral("*.js"));
  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    dialog->deleteLater();
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() {
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
          m_widget.setPlainText(QString::fromUtf8(file.readAll()));
          file.close();
        }
      },
      Qt::QueuedConnection);
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

  auto& editor = DataModel::ProjectEditor::instance();
  QString code = editor.currentGroupPainterCode();
  if (code.isEmpty())
    code = defaultTemplate();

  m_widget.setPlainText(code);
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

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
  if (m_templateNames.isEmpty())
    return;

  bool ok;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Painter Widget Template"),
                                          tr("Choose a template to load:"),
                                          m_templateNames,
                                          0,
                                          false,
                                          &ok);

  if (!ok)
    return;

  const int idx = m_templateNames.indexOf(name);
  if (idx < 0 || idx >= m_templateFiles.size())
    return;

  QFile file(m_templateFiles.at(idx));
  if (file.open(QFile::ReadOnly)) {
    m_widget.setPlainText(QString::fromUtf8(file.readAll()));
    m_widget.document()->clearUndoRedoStacks();
    m_widget.document()->setModified(false);
    Q_EMIT modifiedChanged();
    file.close();
  }

  const QString templateFile = QFileInfo(m_templateFiles.at(idx)).completeBaseName();
  const auto specs           = templateDatasetSpecs(templateFile);
  if (specs.isEmpty())
    return;

  auto& editor = DataModel::ProjectEditor::instance();
  if (!editor.currentGroupIsPainter())
    return;

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const int gid      = editor.currentGroupId();
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
    pm.ensurePainterDatasets(gid, specs);
}

/**
 * @brief Resets the editor to the default painter template.
 */
void DataModel::PainterCodeEditor::reload(bool guiTrigger)
{
  Q_UNUSED(guiTrigger)
  m_widget.setPlainText(defaultTemplate());
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);
  Q_EMIT modifiedChanged();
}

/**
 * @brief Returns the bundled default-template source.
 */
QString DataModel::PainterCodeEditor::defaultTemplate()
{
  const auto templates = loadScriptTemplateManifest(
    QStringLiteral(":/scripts/painter/templates.json"), "DataModel::PainterCodeEditor");

  QString defaultFile;
  for (const auto& tmpl : templates) {
    if (tmpl.isDefault) {
      defaultFile = tmpl.file;
      break;
    }
  }

  if (defaultFile.isEmpty() && !templates.isEmpty())
    defaultFile = templates.constFirst().file;

  if (defaultFile.isEmpty())
    return {};

  return readTextResource(
    templateResourcePath(QStringLiteral(":/scripts/painter"), defaultFile, QStringLiteral(".js")));
}

/**
 * @brief Rebuilds the cached list of painter templates from resources.
 */
void DataModel::PainterCodeEditor::loadTemplates()
{
  m_defaultTemplateFile.clear();
  m_templateNames.clear();
  m_templateFiles.clear();

  const auto templates = loadScriptTemplateManifest(
    QStringLiteral(":/scripts/painter/templates.json"), "DataModel::PainterCodeEditor");

  for (const auto& tmpl : templates) {
    m_templateNames.append(tmpl.name);
    m_templateFiles.append(
      templateResourcePath(QStringLiteral(":/scripts/painter"), tmpl.file, QStringLiteral(".js")));
    if (m_defaultTemplateFile.isEmpty() && tmpl.isDefault)
      m_defaultTemplateFile = tmpl.file;
  }

  if (m_defaultTemplateFile.isEmpty() && !templates.isEmpty())
    m_defaultTemplateFile = templates.constFirst().file;
}

//--------------------------------------------------------------------------------------------------
// Theme + rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the current theme to the code editor widget.
 */
void DataModel::PainterCodeEditor::onThemeChanged()
{
  static const auto* t = &Misc::ThemeManager::instance();
  const auto name      = t->parameters().value(QStringLiteral("code-editor-theme")).toString();
  const auto path =
    name.startsWith('/') ? name : QStringLiteral(":/themes/code-editor/%1.xml").arg(name);

  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    m_style.load(QString::fromUtf8(file.readAll()));
    m_widget.setSyntaxStyle(&m_style);
    file.close();
  }
}

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering.
 */
void DataModel::PainterCodeEditor::renderWidget()
{
  if (isVisible()) {
    syncWidgetPosition();
    m_pixmap = m_widget.grab();
    update();
  }
}

/**
 * @brief Aligns the hidden widget's top-level position with the item's on-screen position so
 *        completer popups and drag auto-scroll resolve correct global coordinates.
 */
void DataModel::PainterCodeEditor::syncWidgetPosition()
{
  if (!window())
    return;

  const QPoint global = mapToGlobal(QPointF(0, 0)).toPoint();
  if (m_widget.pos() != global)
    m_widget.move(global);
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item.
 */
void DataModel::PainterCodeEditor::resizeWidget()
{
  if (width() > 0 && height() > 0) {
    m_widget.setFixedSize(static_cast<int>(width()), static_cast<int>(height()));
    renderWidget();
  }
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::PainterCodeEditor::paint(QPainter* painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...)
 *        are handled natively instead of being consumed by QML Shortcut bindings.
 */
bool DataModel::PainterCodeEditor::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride) {
    QCoreApplication::sendEvent(&m_widget, event);
    if (event->isAccepted())
      return true;
  }

  return QQuickPaintedItem::event(event);
}

/**
 * @brief Forwards completer navigation/commit keys to the popup when visible; everything else
 *        goes straight to the editor widget so QCompleter's focus check cannot hide the popup.
 */
void DataModel::PainterCodeEditor::keyPressEvent(QKeyEvent* event)
{
  auto* completer = m_widget.completer();
  if (completer && completer->popup() && completer->popup()->isVisible()
      && SerialStudioCompleter::popupHandlesKey(event->key()))
    QCoreApplication::sendEvent(completer->popup(), event);
  else
    QCoreApplication::sendEvent(&m_widget, event);

  renderWidget();
}

/**
 * @brief Forwards key-release events to the backing widget.
 */
void DataModel::PainterCodeEditor::keyReleaseEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards input-method events to the backing widget.
 */
void DataModel::PainterCodeEditor::inputMethodEvent(QInputMethodEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
  renderWidget();
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::PainterCodeEditor::focusInEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
void DataModel::PainterCodeEditor::focusOutEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards mouse-press events to the backing widget viewport.
 */
void DataModel::PainterCodeEditor::mousePressEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
  forceActiveFocus();
  renderWidget();
}

/**
 * @brief Forwards mouse-move events to the backing widget viewport.
 */
void DataModel::PainterCodeEditor::mouseMoveEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
  renderWidget();
}

/**
 * @brief Forwards mouse-release events to the backing widget viewport.
 */
void DataModel::PainterCodeEditor::mouseReleaseEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
  renderWidget();
}

/**
 * @brief Forwards double-click events to the backing widget viewport.
 */
void DataModel::PainterCodeEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
  renderWidget();
}

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::PainterCodeEditor::wheelEvent(QWheelEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
  renderWidget();
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::PainterCodeEditor::dragEnterEvent(QDragEnterEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::PainterCodeEditor::dragMoveEvent(QDragMoveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::PainterCodeEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::PainterCodeEditor::dropEvent(QDropEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

#endif  // BUILD_COMMERCIAL
