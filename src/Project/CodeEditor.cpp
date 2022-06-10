/*
 * Copyright (c) 2022 Alex Spataru <https: *github.com/alex-spataru>
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

#include <QFile>
#include <QJSEngine>
#include <QFileDialog>
#include <Misc/Utilities.h>
#include <Misc/ThemeManager.h>

Project::CodeEditor::CodeEditor()
{
    // Setup syntax highlighter
    m_highlighter = new QSourceHighlite::QSourceHighliter(m_textEdit.document());
    m_highlighter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJs);

    // Setup text editor
    onNewClicked();
    m_textEdit.setFont(QFont("Roboto Mono"));

    // Setup toolbar
    auto acNew = m_toolbar.addAction(QIcon(":/icons/template.svg"), tr("New"));
    auto acOpen = m_toolbar.addAction(QIcon(":/icons/open.svg"), tr("Open"));
    auto acSave = m_toolbar.addAction(QIcon(":/icons/save.svg"), tr("Save"));
    m_toolbar.addSeparator();
    auto acUndo = m_toolbar.addAction(QIcon(":/icons/undo.svg"), tr("Undo"));
    auto acRedo = m_toolbar.addAction(QIcon(":/icons/redo.svg"), tr("Redo"));
    m_toolbar.addSeparator();
    auto acCut = m_toolbar.addAction(QIcon(":/icons/cut.svg"), tr("Cut"));
    auto acCopy = m_toolbar.addAction(QIcon(":/icons/copy.svg"), tr("Copy"));
    auto acPaste = m_toolbar.addAction(QIcon(":/icons/paste.svg"), tr("Paste"));
    m_toolbar.addSeparator();
    auto acHelp = m_toolbar.addAction(QIcon(":/icons/help.svg"), tr("Help"));

    // Connect action signals/slots
    connect(acUndo, &QAction::triggered, &m_textEdit, &QPlainTextEdit::undo);
    connect(acRedo, &QAction::triggered, &m_textEdit, &QPlainTextEdit::redo);
    connect(acCut, &QAction::triggered, &m_textEdit, &QPlainTextEdit::cut);
    connect(acCopy, &QAction::triggered, &m_textEdit, &QPlainTextEdit::copy);
    connect(acPaste, &QAction::triggered, &m_textEdit, &QPlainTextEdit::paste);
    connect(acNew, &QAction::triggered, this, &Project::CodeEditor::onNewClicked);
    connect(acNew, &QAction::triggered, this, &Project::CodeEditor::onNewClicked);
    connect(acOpen, &QAction::triggered, this, &Project::CodeEditor::onOpenClicked);
    connect(acSave, &QAction::triggered, this, &Project::CodeEditor::onSaveClicked);
    connect(acHelp, &QAction::triggered, this, &Project::CodeEditor::onHelpClicked);

    // Set widget palette
    QPalette palette;
    auto theme = &Misc::ThemeManager::instance();
    palette.setColor(QPalette::Text, theme->consoleText());
    palette.setColor(QPalette::Base, theme->consoleBase());
    palette.setColor(QPalette::Button, theme->consoleButton());
    palette.setColor(QPalette::Window, theme->consoleWindow());
    palette.setColor(QPalette::Highlight, theme->consoleHighlight());
    palette.setColor(QPalette::HighlightedText, theme->consoleHighlightedText());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    palette.setColor(QPalette::PlaceholderText, theme->consolePlaceholderText());
#endif
    setPalette(palette);

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
    setWindowTitle(tr("Customize frame parser"));
}

Project::CodeEditor::~CodeEditor() { }

Project::CodeEditor &Project::CodeEditor::instance()
{
    static CodeEditor instance;
    return instance;
}

QString Project::CodeEditor::defaultCode() const
{
    QString code;
    QFile file(":/scripts/frame-parser.js");
    if (file.open(QFile::ReadOnly))
    {
        code = QString::fromUtf8(file.readAll());
        file.close();
    }

    return code;
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
        Q_NULLPTR, tr("Select Javascript file to import"), QDir::homePath(), "*.js");

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

void Project::CodeEditor::onHelpClicked() { }

bool Project::CodeEditor::save(const bool silent)
{
    // Validate code
    QJSEngine engine;
    QStringList errors;
    QJSValueList args = { "", "," };
    auto fun = engine.evaluate("(" + m_textEdit.toPlainText() + ")", "", 1, &errors);
    auto ret = fun.call(args);

    // Error on engine evaluation
    if (!errors.isEmpty())
    {
        Misc::Utilities::showMessageBox(tr("Frame parser syntax error!"),
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

        Misc::Utilities::showMessageBox(tr("Frame parser error detected!"), errorStr);
        return false;
    }

    // Update text edit
    m_textEdit.document()->setModified(false);

    // Show save messagebox
    if (!silent)
        Misc::Utilities::showMessageBox(tr("Frame parser code updated successfully!"),
                                        tr("No errors have been detected in the code."));

    // Everything good
    return true;
}

void Project::CodeEditor::closeEvent(QCloseEvent *event)
{
    if (m_textEdit.document()->isModified())
    {
        // Ask user if he/she wants to save changes
        auto ret = Misc::Utilities::showMessageBox(
            tr("The document has been modified!"), tr("Do you want to save the changes?"),
            qAppName(), QMessageBox::Yes | QMessageBox::Discard | QMessageBox::Cancel);

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
