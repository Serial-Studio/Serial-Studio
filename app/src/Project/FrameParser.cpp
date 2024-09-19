/*
 * Copyright (c) 2022-2023 Alex Spataru <https: *github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QFile>
#include <QJSEngine>
#include <QFileDialog>
#include <QDesktopServices>

#include "Model.h"
#include "FrameParser.h"
#include "Misc/Utilities.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"

/**
 * Creates a subclass of @c QWidget that allows us to call the given
 * protected/private
 * @a function and pass the given @a event as a parameter to the @a function.
 */
#define DW_EXEC_EVENT(pointer, function, event)                                \
  class PwnedWidget : public QPlainTextEdit                                    \
  {                                                                            \
  public:                                                                      \
    using QPlainTextEdit::function;                                            \
  };                                                                           \
  static_cast<PwnedWidget *>(pointer)->function(event);                        \
  update();

Project::FrameParser::FrameParser(QQuickItem *parent)
  : QQuickPaintedItem(parent)
{
  // Configure QML painter item
  setMipmap(true);
  setAntialiasing(true);
  setOpaquePainting(true);
  setAcceptTouchEvents(true);
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFillColor(Misc::ThemeManager::instance().getColor(QStringLiteral("base")));

  // Redraw widget when it's size is changed
  connect(this, &QQuickPaintedItem::widthChanged, this,
          &Project::FrameParser::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this,
          &Project::FrameParser::resizeWidget);

  // Setup syntax highlighter
  m_highlighter = new QSourceHighlite::QSourceHighliter(m_textEdit.document());
  m_highlighter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJs);

  // Setup text editor
  m_textEdit.setFont(Misc::CommonFonts::instance().monoFont());

  // Load template code
  reload();

  // Set widget palette
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Project::FrameParser::onThemeChanged);

  // Connect modification check signals
  connect(m_textEdit.document(), &QTextDocument::modificationChanged, this,
          [=] { Q_EMIT modifiedChanged(); });

  // Load code from JSON model automatically
  connect(&Project::Model::instance(), &Model::frameParserCodeChanged, this,
          &Project::FrameParser::readCode);

  // Use a timer to redraw the widget (yeah...I want the block cursor to blink)
  m_timer.setTimerType(Qt::PreciseTimer);
  connect(&m_timer, &QTimer::timeout, this, [=] { update(); });
  connect(this, &QQuickPaintedItem::visibleChanged, this, [=] {
    if (isVisible())
      m_timer.start(1000 / 24);
    else
      m_timer.stop();
  });

  // Start the drawing loop
  if (isVisible())
    m_timer.start(1000 / 24);

  // Bridge signals
  connect(&m_textEdit, &QPlainTextEdit::textChanged, this,
          &Project::FrameParser::textChanged);
}

const QString &Project::FrameParser::defaultCode()
{
  static QString code;
  if (code.isEmpty())
  {
    QFile file(":/rcc/scripts/frame-parser.js");
    if (file.open(QFile::ReadOnly))
    {
      code = QString::fromUtf8(file.readAll());
      file.close();
    }
  }

  return code;
}

QString Project::FrameParser::text() const
{
  return m_textEdit.document()->toPlainText();
}

bool Project::FrameParser::isModified() const
{
  return m_textEdit.document()->isModified();
}

QStringList Project::FrameParser::parse(const QString &frame,
                                        const QString &separator)
{
  // Construct function arguments
  QJSValueList args;
  args << frame << separator;

  // Evaluate frame parsing function
  auto out = m_parseFunction.call(args).toVariant().toList();

  // Convert output to QStringList
  QStringList list;
  for (auto i = 0; i < out.count(); ++i)
    list.append(out.at(i).toString());

  // Return fields list
  return list;
}

/**
 * Grabs an image/pixmap of the contained widget. The pixmap is later
 * used to render the widget in the QML interface without causing signal/slot
 * interferences with the scenegraph render thread.
 */
void Project::FrameParser::update(const QRect &rect)
{
  m_pixmap = m_textEdit.grab();
  QQuickPaintedItem::update(rect);
}

/**
 * Displays the pixmap generated in the @c update() function in the QML
 * interface through the given @a painter pointer.
 */
void Project::FrameParser::paint(QPainter *painter)
{
  if (painter)
    painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::keyPressEvent(QKeyEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, keyPressEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::keyReleaseEvent(QKeyEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, keyReleaseEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::inputMethodEvent(QInputMethodEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, inputMethodEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::focusInEvent(QFocusEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, focusInEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::focusOutEvent(QFocusEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, focusOutEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::mousePressEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mousePressEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::mouseMoveEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mouseMoveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::mouseReleaseEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mouseReleaseEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::mouseDoubleClickEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mouseDoubleClickEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::wheelEvent(QWheelEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, wheelEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::dragEnterEvent(QDragEnterEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dragEnterEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::dragMoveEvent(QDragMoveEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dragMoveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::dragLeaveEvent(QDragLeaveEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dragLeaveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void Project::FrameParser::dropEvent(QDropEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dropEvent, event);
}

void Project::FrameParser::cut()
{
  m_textEdit.cut();
}

void Project::FrameParser::undo()
{
  m_textEdit.undo();
}

void Project::FrameParser::redo()
{
  m_textEdit.redo();
}

void Project::FrameParser::help()
{ // clang-format off
  const auto url = QStringLiteral("https://github.com/Serial-Studio/Serial-Studio/wiki");
  QDesktopServices::openUrl(QUrl(url));
  // clang-format on
}

void Project::FrameParser::copy()
{
  m_textEdit.copy();
}

void Project::FrameParser::paste()
{
  if (m_textEdit.canPaste())
  {
    m_textEdit.paste();
    Q_EMIT modifiedChanged();
  }
}

void Project::FrameParser::apply()
{
  save(true);
}

void Project::FrameParser::reload()
{
  // Document has been modified, ask user if he/she wants to continue
  if (m_textEdit.document()->isModified())
  {
    auto ret = Misc::Utilities::showMessageBox(
        tr("The document has been modified!"),
        tr("Are you sure you want to continue?"), qAppName(),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
      return;
  }

  // Load default template
  m_textEdit.setPlainText(defaultCode());
  save(true);
}

void Project::FrameParser::import()
{
  // Document has been modified, ask user if he/she wants to continue
  if (m_textEdit.document()->isModified())
  {
    auto ret = Misc::Utilities::showMessageBox(
        tr("The document has been modified!"),
        tr("Are you sure you want to continue?"), qAppName(),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
      return;
  }

  // Get file from system
  auto path = QFileDialog::getOpenFileName(
      Q_NULLPTR, tr("Select Javascript file to import"), QDir::homePath(),
      "*.js");

  // Load file into code editor
  if (!path.isEmpty())
  {
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
      auto data = file.readAll();
      m_textEdit.setPlainText(QString::fromUtf8(data));
      save(true);
    }
  }
}

void Project::FrameParser::selectAll()
{
  m_textEdit.selectAll();
  update();
}

/**
 * Resizes the widget to fit inside the QML painted item.
 */
void Project::FrameParser::resizeWidget()
{
  if (width() > 0 && height() > 0)
  {
    m_textEdit.setFixedSize(width(), height());
    update();
  }
}

void Project::FrameParser::onThemeChanged()
{
  QPalette palette;
  auto theme = &Misc::ThemeManager::instance();
  palette.setColor(QPalette::Text, theme->getColor("text"));
  palette.setColor(QPalette::Base, theme->getColor("base"));
  palette.setColor(QPalette::Button, theme->getColor("button"));
  palette.setColor(QPalette::Window, theme->getColor("window"));
  palette.setColor(QPalette::Highlight, theme->getColor("highlight"));
  palette.setColor(QPalette::HighlightedText,
                   theme->getColor("highlighted_text"));
  palette.setColor(QPalette::PlaceholderText,
                   theme->getColor("placeholder_text"));
  m_textEdit.setPalette(palette);
}

bool Project::FrameParser::save(const bool silent)
{
  // Update text edit
  if (loadScript(m_textEdit.toPlainText()))
  {
    m_textEdit.document()->setModified(false);
    Model::instance().setFrameParserCode(m_textEdit.toPlainText());

    // Show save messagebox
    if (!silent)
      Misc::Utilities::showMessageBox(
          tr("Frame parser code updated successfully!"),
          tr("No errors have been detected in the code."));

    // Everything good
    return true;
  }

  // Something did not work quite right
  return false;
}

bool Project::FrameParser::loadScript(const QString &script)
{
  // Check if there are no general JS errors
  QStringList errors;
  m_engine.evaluate(script, "", 1, &errors);

  // Check if parse() function exists
  auto fun = m_engine.globalObject().property("parse");
  if (fun.isNull() || !fun.isCallable())
  {
    Misc::Utilities::showMessageBox(
        tr("Frame parser error!"),
        tr("No parse() function has been declared!"));
    return false;
  }

  // Try to run parse() function
  QJSValueList args = {"", ","};
  auto ret = fun.call(args);

  // Error on engine evaluation
  if (!errors.isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("Frame parser syntax error!"),
        tr("Error on line %1.").arg(errors.first()));
    return false;
  }

  // Error on function execution
  else if (ret.isError())
  {
    QString errorStr;
    switch (ret.errorType())
    {
      case QJSValue::GenericError:
        errorStr = tr("Generic error");
        break;
      case QJSValue::EvalError:
        errorStr = tr("Evaluation error");
        break;
      case QJSValue::RangeError:
        errorStr = tr("Range error");
        break;
      case QJSValue::ReferenceError:
        errorStr = tr("Reference error");
        break;
      case QJSValue::SyntaxError:
        errorStr = tr("Syntax error");
        break;
      case QJSValue::TypeError:
        errorStr = tr("Type error");
        break;
      case QJSValue::URIError:
        errorStr = tr("URI error");
        break;
      default:
        errorStr = tr("Unknown error");
        break;
    }

    Misc::Utilities::showMessageBox(tr("Frame parser error detected!"),
                                    errorStr);
    return false;
  }

  // We have reached this point without any errors, set function caller
  m_parseFunction = fun;
  return true;
}

void Project::FrameParser::readCode()
{
  m_textEdit.setPlainText(Model::instance().frameParserCode());
  m_textEdit.document()->setModified(false);
  loadScript(m_textEdit.toPlainText());
}
