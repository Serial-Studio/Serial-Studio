/*
Copyright (C) 2025 cen1

This file is part of QuaZip test suite.

QuaZip is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

QuaZip is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with QuaZip.  If not, see <http://www.gnu.org/licenses/>.

See COPYING file for the full LGPL text.

Original ZIP package is copyrighted by Gilles Vollant and contributors,
see quazip/(un)zip.h files for details. Basically it's the zlib license.
*/

#include "testjlcp_extract.h"

#include "qztest.h"

#include <QSet>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QMetaType>

#include <QtTest/QtTest>

#include <JlCompress.h>
#include <quazip_qt_compat.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Q_DECLARE_METATYPE(JlCompress::Options::CompressionStrategy)

/* Inside the CI env, <build>/quazip directory contains directories like
 * macos-13_qt5.15.2_sharedOFF_cp
 * ubuntu-22.04_qt5.15.2_sharedOFF_cp
 * Inside, there is a cp.zip which is a bundle of several zip files produced by TestJlCpCompress
 * We extract all, check content. This verifies an archive created on different platform can be properly extracted.
 */
void TestJlCpExtract::extract()
{
	QSet<QString> zipNames = {
	  "jlsimplefile.zip",
	  "jlsimplefile-storage.zip",
	  "jlsimplefile-best.zip"
	};

    QSet<QString> fileNames = {
	  "test0.txt"
	};

    qDebug() << "Performing CP extract tests in " << QDir::currentPath();

    QDirIterator it(QDir::currentPath(), QStringList() << "*_cp", QDir::Dirs, QDirIterator::Subdirectories);

    QVERIFY(it.hasNext());

    while (it.hasNext()) {
        QFileInfo artifact_dir(it.next());
        qDebug() << "====== Found artifact:" << artifact_dir.fileName() << "======";

        QFileInfo cpZip(artifact_dir.absoluteFilePath(), "cp.zip");
        QVERIFY(cpZip.exists());

        QDir target2(artifact_dir.absoluteFilePath()+"/cp");
        // macos-13_qt6.8.2_sharedON_cp/cp/
        JlCompress::extractDir(cpZip.absoluteFilePath(), target2.absolutePath());

        QList<QString> extractedZipList = target2.entryList(QStringList() << "*.zip", QDir::Files);
        QSet<QString> extractedZipSet(extractedZipList.begin(), extractedZipList.end());

        extractedZipSet = extractedZipSet.subtract(zipNames);
        QVERIFY(extractedZipSet.isEmpty());

        for (const QString &zipFile : zipNames) {
        	qDebug() << "Found ZIP:" << zipFile;
            QFileInfo zip(target2, zipFile);
			QDir target3(target2.absolutePath()+"/"+zip.completeBaseName());
            // macos-13_qt6.8.2_sharedON_cp/cp/jlsimplefile/
			JlCompress::extractDir(zip.absoluteFilePath(), target3.absolutePath());

			QList<QString> extractedFileList = target3.entryList(QDir::Files);
			QSet<QString> extractedFileSet(extractedFileList.begin(), extractedFileList.end());

            extractedFileSet = extractedFileSet.subtract(fileNames);
			QVERIFY(extractedFileSet.isEmpty());

            // Check file content
            for (const QString &fileName : extractedFileList) {
                qDebug() << "Found File:" << fileName;

                QFile file(target3.absolutePath()+"/"+fileName);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                  QFAIL("Failed to open file");
                }

                QTextStream in(&file);
                QString fileContent = in.readAll();
                file.close();

                QString expectedContent("");
                QTextStream ss(&expectedContent);
                ss << "This is a test file named " << fileName << quazip_endl;

                QVERIFY(fileContent == expectedContent);
            }
        }
	}
}
