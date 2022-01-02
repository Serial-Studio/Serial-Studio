/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#pragma once

#include <QObject>
#include <QMessageBox>
#include <QApplication>

namespace Misc
{
/**
 * @brief The Utilities class
 *
 * The @c Utilitities module provides commonly used functionality to the rest of the
 * application. For example, showing a messagebox with a nice format or revealing files in
 * the operating system's preffered file manager.
 */
class Utilities : public QObject
{
    Q_OBJECT

public:
    // clang-format off
    static Utilities &instance();
    static void rebootApplication();
    Q_INVOKABLE bool askAutomaticUpdates();
    static int showMessageBox(const QString &text, 
                              const QString &informativeText = "",
                              const QString &windowTitle = "",
                              const QMessageBox::StandardButtons &bt = QMessageBox::Ok);
    //clang-format on

public Q_SLOTS:
    static void aboutQt();
    static void revealFile(const QString& pathToReveal);
};
}
