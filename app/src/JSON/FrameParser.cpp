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

#include "JSON/FrameParser.h"
#include "JSON/ProjectModel.h"

#include "Misc/Utilities.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"

JSON::FrameParser::FrameParser(QQuickItem *parent)
  : UI::DeclarativeWidget(parent)
{
  // Set widget
  setWidget(&m_textEdit);

  // Setup syntax highlighter
  m_highlighter = new QSourceHighlite::QSourceHighliter(m_textEdit.document());
  m_highlighter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJs);

  // Setup text editor
  m_textEdit.setFont(Misc::CommonFonts::instance().monoFont());

  // Configure JavaScript engine
  m_engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

  // Load template code
  reload();

  // Set widget palette
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &JSON::FrameParser::onThemeChanged);

  // Connect modification check signals
  connect(m_textEdit.document(), &QTextDocument::modificationChanged, this,
          [=] { Q_EMIT modifiedChanged(); });

  // Load code from JSON model automatically
  connect(&JSON::ProjectModel::instance(),
          &ProjectModel::frameParserCodeChanged, this,
          &JSON::FrameParser::readCode);

  // Bridge signals
  connect(&m_textEdit, &QPlainTextEdit::textChanged, this,
          &JSON::FrameParser::textChanged);
}

const QString &JSON::FrameParser::defaultCode()
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

QString JSON::FrameParser::text() const
{
  return m_textEdit.document()->toPlainText();
}

bool JSON::FrameParser::isModified() const
{
  return m_textEdit.document()->isModified();
}

QStringList JSON::FrameParser::parse(const QString &frame,
                                     const QString &separator)
{
  // Construct function arguments
  QJSValueList args;
  args << frame << separator;

  // Evaluate frame parsing function
  auto out = m_parseFunction.call(args).toVariant().toStringList();

  // Convert output to QStringList
  QStringList list;
  for (auto i = 0; i < out.count(); ++i)
    list.append(out.at(i));

  // Return fields list
  return list;
}

void JSON::FrameParser::cut()
{
  m_textEdit.cut();
}

void JSON::FrameParser::undo()
{
  m_textEdit.undo();
}

void JSON::FrameParser::redo()
{
  m_textEdit.redo();
}

void JSON::FrameParser::help()
{ // clang-format off
  const auto url = QStringLiteral("https://github.com/Serial-Studio/Serial-Studio/wiki");
  QDesktopServices::openUrl(QUrl(url));
  // clang-format on
}

void JSON::FrameParser::copy()
{
  m_textEdit.copy();
}

void JSON::FrameParser::paste()
{
  if (m_textEdit.canPaste())
  {
    m_textEdit.paste();
    Q_EMIT modifiedChanged();
  }
}

void JSON::FrameParser::apply()
{
  save(true);
}

void JSON::FrameParser::reload()
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

void JSON::FrameParser::import()
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

void JSON::FrameParser::selectAll()
{
  m_textEdit.selectAll();
}

void JSON::FrameParser::onThemeChanged()
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

bool JSON::FrameParser::save(const bool silent)
{
  // Update text edit
  if (loadScript(m_textEdit.toPlainText()))
  {
    m_textEdit.document()->setModified(false);
    ProjectModel::instance().setFrameParserCode(m_textEdit.toPlainText());

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

bool JSON::FrameParser::loadScript(const QString &script)
{
  // Ensure that engine is configured correctly
  m_engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

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

void JSON::FrameParser::readCode()
{
  m_textEdit.setPlainText(ProjectModel::instance().frameParserCode());
  m_textEdit.document()->setModified(false);
  loadScript(m_textEdit.toPlainText());
}
