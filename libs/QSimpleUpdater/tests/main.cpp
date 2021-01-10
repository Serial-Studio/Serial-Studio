/*
 * Copyright (c) 2015-2016 Alex Spataru <alex_spataru@outlook.com>
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

#include "Test_Updater.h"
#include "Test_Downloader.h"
#include "Test_QSimpleUpdater.h"

int main (int argc, char* argv[])
{
    QApplication app (argc, argv);

    app.setApplicationName ("QSimpleUpdater Tests");
    app.setOrganizationName ("The QSimpleUpdater Library");

    QTest::qExec (new Test_Updater, argc, argv);
    QTest::qExec (new Test_Downloader, argc, argv);
    QTest::qExec (new Test_QSimpleUpdater, argc, argv);

    QTimer::singleShot (1000, Qt::PreciseTimer, qApp, SLOT (quit()));

    return app.exec();
}
