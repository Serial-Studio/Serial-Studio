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

#include "CodeEditor.h"
#include "Model.h"

#include <QFile>
#include <QTimer>
#include <QJSEngine>
#include <QFileDialog>
#include <QDesktopServices>
#include <Misc/Utilities.h>
#include <Misc/CommonFonts.h>
#include <Misc/ThemeManager.h>

Project::CodeEditor::CodeEditor()
{
  // Setup syntax highlighter
  m_highlighter = new QSourceHighlite::QSourceHighliter(m_textEdit.document());
  m_highlighter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJs);

  // Setup text editor
  onNewClicked();
  m_textEdit.setFont(Misc::CommonFonts::instance().monoFont());

  // Setup toolbar
  // clang-format off
  auto acNew = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/new.svg"), tr("New"));
  auto acOpen = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/open.svg"), tr("Open"));
  auto acSave = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/save.svg"), tr("Save"));
  m_toolbar.addSeparator();
  auto acUndo = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/undo.svg"), tr("Undo"));
  auto acRedo = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/redo.svg"), tr("Redo"));
  m_toolbar.addSeparator();
  auto acCut = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/cut.svg"), tr("Cut"));
  auto acCopy = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/copy.svg"), tr("Copy"));
  auto acPaste = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/paste.svg"), tr("Paste"));
  m_toolbar.addSeparator();
  auto acHelp = m_toolbar.addAction(QIcon(":/rcc/icons/code-editor/help.svg"), tr("Help"));
  // clang-format on

  // Connect action signals/slots
  connect(acUndo, &QAction::triggered, &m_textEdit, &QPlainTextEdit::undo);
  connect(acRedo, &QAction::triggered, &m_textEdit, &QPlainTextEdit::redo);
  connect(acCut, &QAction::triggered, &m_textEdit, &QPlainTextEdit::cut);
  connect(acCopy, &QAction::triggered, &m_textEdit, &QPlainTextEdit::copy);
  connect(acPaste, &QAction::triggered, &m_textEdit, &QPlainTextEdit::paste);
  connect(acNew, &QAction::triggered, this, &Project::CodeEditor::onNewClicked);
  connect(acNew, &QAction::triggered, this, &Project::CodeEditor::onNewClicked);
  connect(acOpen, &QAction::triggered, this,
          &Project::CodeEditor::onOpenClicked);
  connect(acSave, &QAction::triggered, this,
          &Project::CodeEditor::onSaveClicked);
  connect(acHelp, &QAction::triggered, this,
          &Project::CodeEditor::onHelpClicked);

  // Set widget palette
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Project::CodeEditor::onThemeChanged);

  // Setup layout
  auto layout = new QVBoxLayout(this);
  layout->addWidget(&m_toolbar);
  layout->addWidget(&m_textEdit);
  layout->setStretch(0, 0);
  layout->setStretch(1, 1);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  // Setup window
  setMinimumSize(QSize(640, 480));
  setWindowTitle(tr("Frame Parsing Code"));

  // Load code from JSON model automatically
  connect(&Project::Model::instance(), &Model::frameParserCodeChanged, this,
          &Project::CodeEditor::readCode);
}

Project::CodeEditor::~CodeEditor() {}

Project::CodeEditor &Project::CodeEditor::instance()
{
  static CodeEditor instance;
  return instance;
}

QString Project::CodeEditor::defaultCode() const
{
  QString code;
  QFile file(":/rcc/scripts/frame-parser.js");
  if (file.open(QFile::ReadOnly))
  {
    code = QString::fromUtf8(file.readAll());
    file.close();
  }

  return code;
}

QStringList Project::CodeEditor::parse(const QString &frame,
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

void Project::CodeEditor::displayWindow()
{
  showNormal();
}

void Project::CodeEditor::onNewClicked()
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

void Project::CodeEditor::onOpenClicked()
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

void Project::CodeEditor::onSaveClicked()
{
  if (save(false))
    close();
}

void Project::CodeEditor::onHelpClicked()
{
  // clang-format off
  const auto url = QStringLiteral("https://github.com/Serial-Studio/Serial-Studio/wiki");
  QDesktopServices::openUrl(QUrl(url));
  // clang-format on
}

void Project::CodeEditor::onThemeChanged()
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
  setPalette(palette);
}

bool Project::CodeEditor::save(const bool silent)
{
  // Update text edit
  if (loadScript(m_textEdit.toPlainText()))
  {
    m_textEdit.document()->setModified(false);

    // Save code inside JSON project
    QTimer::singleShot(500, this, SLOT(writeChanges()));

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

bool Project::CodeEditor::loadScript(const QString &script)
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

void Project::CodeEditor::closeEvent(QCloseEvent *event)
{
  if (m_textEdit.document()->isModified())
  {
    // Ask user if he/she wants to save changes
    auto ret = Misc::Utilities::showMessageBox(
        tr("The document has been modified!"),
        tr("Do you want to save the changes?"), qAppName(),
        QMessageBox::Yes | QMessageBox::Discard | QMessageBox::Cancel);

    // User wants to save changes, validate code & apply
    if (ret == QMessageBox::Yes)
    {
      if (!save(true))
      {
        event->ignore();
        return;
      }
    }

    // User wants to continue editing the code
    if (ret == QMessageBox::Cancel)
    {
      event->ignore();
      return;
    }
  }

  // User saved changes (with no errors) or discarded the changes
  event->accept();
}

void Project::CodeEditor::readCode()
{
  m_textEdit.setPlainText(Model::instance().frameParserCode());
  m_textEdit.document()->setModified(false);
  loadScript(m_textEdit.toPlainText());
}

void Project::CodeEditor::writeChanges()
{
  Model::instance().setFrameParserCode(m_textEdit.toPlainText());
}
