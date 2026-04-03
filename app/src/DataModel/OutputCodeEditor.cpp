/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/OutputCodeEditor.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QJavascriptHighlighter>
#include <QLineNumberArea>
#include <QTextDocument>

#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the OutputCodeEditor, loads templates from rcc resources.
 */
DataModel::OutputCodeEditor::OutputCodeEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent), m_readingCode(false), m_testDialog(nullptr)
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

  // Configure code editor
  m_widget.setTabReplace(true);
  m_widget.setTabReplaceSize(4);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(Misc::CommonFonts::instance().monoFont());

  // Apply current theme
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::OutputCodeEditor::onThemeChanged);

  // Emit modification signals
  connect(&m_widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::OutputCodeEditor::textChanged);

  // Push code to the selected output widget on every edit
  connect(&m_widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    auto& editor = DataModel::ProjectEditor::instance();
    if (editor.currentView() != DataModel::ProjectEditor::OutputWidgetView)
      return;

    auto& pm  = DataModel::ProjectModel::instance();
    auto& sel = editor.selectedOutputWidget();
    if (sel.groupId < 0 || sel.widgetId < 0)
      return;

    DataModel::OutputWidget updated = sel;
    updated.transmitFunction        = text();
    pm.updateOutputWidget(sel.groupId, sel.widgetId, updated, false);
  });

  // Reload when the output widget model changes
  connect(&DataModel::ProjectEditor::instance(),
          &DataModel::ProjectEditor::outputWidgetModelChanged,
          this,
          &DataModel::OutputCodeEditor::readCode);

  // Resize and render
  connect(this, &QQuickPaintedItem::widthChanged, this, &DataModel::OutputCodeEditor::resizeWidget);
  connect(
    this, &QQuickPaintedItem::heightChanged, this, &DataModel::OutputCodeEditor::resizeWidget);
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::OutputCodeEditor::renderWidget);

  // clang-format off
  // Load templates from resources
  const struct { const char *file; const char *name; } templates[] = {
    {"at_command.js",       QT_TR_NOOP("AT command")      },
    {"binary_packet.js",    QT_TR_NOOP("Binary packet")   },
    {"default_template.js", QT_TR_NOOP("Default template")},
    {"gcode_command.js",    QT_TR_NOOP("G-Code command")  },
    {"grbl_command.js",     QT_TR_NOOP("GRBL command")    },
    {"json_command.js",     QT_TR_NOOP("JSON command")    },
    {"modbus_write.js",     QT_TR_NOOP("Modbus write")    },
    {"nmea_sentence.js",    QT_TR_NOOP("NMEA sentence")   },
    {"pid_setpoint.js",     QT_TR_NOOP("PID setpoint")    },
    {"pwm_control.js",      QT_TR_NOOP("PWM control")     },
    {"relay_toggle.js",     QT_TR_NOOP("Relay toggle")    },
    {"scpi_command.js",     QT_TR_NOOP("SCPI command")    },
    {"simple_command.js",   QT_TR_NOOP("Simple command")  },
    {"slcan_command.js",    QT_TR_NOOP("SLCAN command")   },
  };
  // clang-format on

  for (const auto& t : templates)
  {
    m_templateFiles.append(QStringLiteral(":/rcc/scripts/output/%1").arg(QLatin1String(t.file)));
    m_templateNames.append(tr(t.name));
  }

  readCode();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text content.
 */
QString DataModel::OutputCodeEditor::text() const
{
  return m_widget.toPlainText();
}

/**
 * @brief Returns @c true if the document has unsaved modifications.
 */
bool DataModel::OutputCodeEditor::isModified() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

/**
 * @brief Returns @c true if an undo step is available.
 */
bool DataModel::OutputCodeEditor::undoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isUndoAvailable();

  return false;
}

/**
 * @brief Returns @c true if a redo step is available.
 */
bool DataModel::OutputCodeEditor::redoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isRedoAvailable();

  return false;
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the selection to the clipboard.
 */
void DataModel::OutputCodeEditor::cut()
{
  m_widget.cut();
}

/**
 * @brief Undoes the last edit operation.
 */
void DataModel::OutputCodeEditor::undo()
{
  m_widget.undo();
}

/**
 * @brief Re-applies the last undone edit.
 */
void DataModel::OutputCodeEditor::redo()
{
  m_widget.redo();
}

/**
 * @brief Copies the selection to the clipboard.
 */
void DataModel::OutputCodeEditor::copy()
{
  m_widget.copy();
}

/**
 * @brief Pastes clipboard content at the cursor.
 */
void DataModel::OutputCodeEditor::paste()
{
  m_widget.paste();
}

/**
 * @brief Selects the entire document.
 */
void DataModel::OutputCodeEditor::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Opens a file dialog to import an external JS file.
 */
void DataModel::OutputCodeEditor::import()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select Javascript file to import"), QDir::homePath(), QStringLiteral("*.js"));
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty()) {
      QFile file(path);
      if (file.open(QFile::ReadOnly)) {
        m_widget.setPlainText(QString::fromUtf8(file.readAll()));
        file.close();
      }
    }
    dialog->deleteLater();
  });

  dialog->open();
}

/**
 * @brief Loads the transmit function from the currently selected output widget.
 */
void DataModel::OutputCodeEditor::readCode()
{
  // Guard against reentrancy during code loading
  if (m_readingCode)
    return;

  m_readingCode = true;

  auto& editor    = DataModel::ProjectEditor::instance();
  const auto& sel = editor.selectedOutputWidget();

  QString code = sel.transmitFunction;
  if (code.isEmpty())
    code = defaultTemplate();

  m_widget.setPlainText(code);
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Shows a dialog to pick and load a built-in template.
 */
void DataModel::OutputCodeEditor::selectTemplate()
{
  // Abort if no templates are available
  if (m_templateNames.isEmpty())
    return;

  bool ok;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Output Widget Template"),
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
}

/**
 * @brief Opens the transmit test dialog with the current editor code.
 */
void DataModel::OutputCodeEditor::testTransmitFunction()
{
  m_testDialog.setTransmitCode(text());
  m_testDialog.clear();
  m_testDialog.showNormal();
}

/**
 * @brief Resets the editor to the default transmit function template.
 */
void DataModel::OutputCodeEditor::reload(bool guiTrigger)
{
  // Reset editor to the default transmit function
  Q_UNUSED(guiTrigger)
  m_widget.setPlainText(defaultTemplate());
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);
  Q_EMIT modifiedChanged();
}

/**
 * @brief Loads the default transmit function template from application resources.
 */
QString DataModel::OutputCodeEditor::defaultTemplate()
{
  QString code;
  QFile file(QStringLiteral(":/rcc/scripts/output/default_template.js"));
  if (file.open(QFile::ReadOnly)) {
    code = QString::fromUtf8(file.readAll());
    file.close();
  }

  return code;
}

//--------------------------------------------------------------------------------------------------
// Theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the current theme to the code editor widget.
 */
void DataModel::OutputCodeEditor::onThemeChanged()
{
  // Resolve theme path and load the syntax style XML
  static const auto* t = &Misc::ThemeManager::instance();
  const auto name      = t->parameters().value(QStringLiteral("code-editor-theme")).toString();

  const auto path =
    name.startsWith('/') ? name : QStringLiteral(":/rcc/themes/code-editor/%1.xml").arg(name);

  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    m_style.load(QString::fromUtf8(file.readAll()));
    m_widget.setSyntaxStyle(&m_style);
    file.close();
  }
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering.
 */
void DataModel::OutputCodeEditor::renderWidget()
{
  if (isVisible()) {
    m_pixmap = m_widget.grab();
    update();
  }
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item dimensions.
 */
void DataModel::OutputCodeEditor::resizeWidget()
{
  if (width() > 0 && height() > 0) {
    m_widget.setFixedSize(static_cast<int>(width()), static_cast<int>(height()));
    renderWidget();
  }
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

void DataModel::OutputCodeEditor::paint(QPainter* painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

void DataModel::OutputCodeEditor::keyPressEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

void DataModel::OutputCodeEditor::keyReleaseEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

void DataModel::OutputCodeEditor::inputMethodEvent(QInputMethodEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

void DataModel::OutputCodeEditor::focusInEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

void DataModel::OutputCodeEditor::focusOutEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

void DataModel::OutputCodeEditor::mousePressEvent(QMouseEvent* event)
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
}

void DataModel::OutputCodeEditor::mouseMoveEvent(QMouseEvent* event)
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
}

void DataModel::OutputCodeEditor::mouseReleaseEvent(QMouseEvent* event)
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
}

void DataModel::OutputCodeEditor::mouseDoubleClickEvent(QMouseEvent* event)
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
}

void DataModel::OutputCodeEditor::wheelEvent(QWheelEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

void DataModel::OutputCodeEditor::dragEnterEvent(QDragEnterEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

void DataModel::OutputCodeEditor::dragMoveEvent(QDragMoveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

void DataModel::OutputCodeEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

void DataModel::OutputCodeEditor::dropEvent(QDropEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}
