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

static const QString DEFAULT_CODE
    = "/* \n"
      " * Frame parsing function, you can modify this to suit your\n"
      " * needs. By customizing this code, you can use a single JSON\n"
      " * project file to process different kinds of frames that are\n"
      " * sent by the microcontroller or device that is connected to\n"
      " * Serial Studio.\n"
      " *\n"
      " * @note only data that is *inside* the data delimiters will\n"
      " *       be processed by this function!\n"
      " *\n"
      " * @param frame string with the latest received frame.\n"
      " * @param separator data sepatator sequence set by the JSON project.\n"
      " */\n"
      "function parse(frame, separator) {\n"
      "    return frame.split(separator);\n"
      "}";

Project::CodeEditor::CodeEditor()
{
    setWindowTitle(tr("Frame parser code"));
    m_textEdit.setPlainText(DEFAULT_CODE);
    m_textEdit.setFont(QFont("Roboto Mono"));

    auto layout = new QVBoxLayout(this);
    layout->addWidget(&m_textEdit);

    setLayout(layout);
    setMinimumSize(QSize(640, 480));
}

Project::CodeEditor::~CodeEditor() { }

Project::CodeEditor &Project::CodeEditor::instance()
{
    static CodeEditor instance;
    return instance;
}

QString Project::CodeEditor::defaultCode() const
{
    return DEFAULT_CODE;
}

void Project::CodeEditor::displayWindow()
{
    showNormal();
}
