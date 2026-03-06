/*
Copyright (C) 2005-2014 Sergey A. Tachenov

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

#include "testquazipdir.h"
#include "qztest.h"
#include <QtTest/QtTest>
#include <quazip.h>
#include <quazipdir.h>

void TestQuaZipDir::entryList_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QString>("dirName");
    QTest::addColumn<QStringList>("nameFilters");
    // QDir::Filters type breaks Qt meta type system on MSVC
    QTest::addColumn<int>("filter");
    QTest::addColumn<int>("sort");
    QTest::addColumn<QStringList>("entries");
    QTest::addColumn<int>("caseSensitivity");
    QTest::newRow("simple") << "simple.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << "testdir2" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Unsorted)
            << (QStringList() << "test2.txt" << "subdir/") << -1;
    QTest::newRow("separate dir") << "sepdir.zip" << (
            QStringList() << "laj/" << "laj/lajfile.txt")
            << "" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Unsorted)
            << (QStringList() << "laj/") << -1;
    QTest::newRow("separate dir (subdir listing)") << "sepdirsub.zip" << (
            QStringList() << "laj/" << "laj/lajfile.txt")
            << "laj" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Unsorted)
            << (QStringList() << "lajfile.txt") << -1;
    QTest::newRow("dirs only") << "dirsonly.zip" << (
            QStringList() << "file" << "dir/")
            << "" << QStringList()
            << static_cast<int>(QDir::Dirs)
            << static_cast<int>(QDir::Unsorted)
            << (QStringList() << "dir/") << -1;
    QTest::newRow("files only") << "filesonly.zip" << (
            QStringList() << "file1" << "parent/dir/" << "parent/file2")
            << "parent" << QStringList()
            << static_cast<int>(QDir::Files)
            << static_cast<int>(QDir::Unsorted)
            << (QStringList() << "file2") << -1;
    QTest::newRow("sorted") << "sorted.zip" << (
            QStringList() << "file1" << "parent/subdir/" << "parent/subdir2/file3" << "parent/file2" << "parent/file0")
            << "parent" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Name)
            << (QStringList() << "file0" << "file2" << "subdir/" << "subdir2/")
            << -1;
    QTest::newRow("sorted dirs first") << "sorted-dirs.zip" << (
            QStringList() << "file1" << "parent/subdir/" << "parent/subdir2/file3" << "parent/file2" << "parent/file0")
            << "parent" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Name | QDir::DirsFirst)
            << (QStringList() << "subdir/" << "subdir2/" << "file0" << "file2")
            << -1;
    QTest::newRow("sorted dirs first reversed") << "sorted-reverse.zip" << (
            QStringList() << "file1" << "parent/subdir/" << "parent/subdir2/file3" << "parent/file2" << "parent/file0")
            << "parent" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Name | QDir::DirsFirst | QDir::Reversed)
            << (QStringList() << "subdir2/" << "subdir/" << "file2" << "file0")
            << -1;
    QTest::newRow("sorted by size") << "sorted-size.zip" << (
            QStringList() << "file000" << "file10")
            << "/" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Size)
            << (QStringList() << "file10" << "file000") << -1;
    QTest::newRow("sorted by time") << "sorted-time.zip" << (
            QStringList() << "file04" << "file03" << "file02" << "subdir/subfile")
            << "/" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Time)
            << (QStringList() << "subdir/" << "file04" << "file02" << "file03")
            << -1;
    QTest::newRow("sorted by type") << "sorted-type.zip" << (
            QStringList() << "file1.txt" << "file2.dat")
            << "/" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Type)
            << (QStringList() << "file2.dat" << "file1.txt") << -1;
    QTest::newRow("name filter") << "name-filter.zip" << (
            QStringList() << "file01" << "file02" << "laj")
            << "/" << QStringList("file*")
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Name)
            << (QStringList() << "file01" << "file02") << -1;
    QTest::newRow("case sensitive") << "case-sensitive.zip" << (
            QStringList() << "a" << "B")
            << "/" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Name)
            << (QStringList() << "B" << "a")
            << static_cast<int>(QuaZip::csSensitive);
    QTest::newRow("case insensitive") << "case-insensitive.zip" << (
            QStringList() << "B" << "a")
            << "/" << QStringList()
            << static_cast<int>(QDir::NoFilter)
            << static_cast<int>(QDir::Name)
            << (QStringList() << "a" << "B")
            << static_cast<int>(QuaZip::csInsensitive);
}

void TestQuaZipDir::entryList()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QString, dirName);
    QFETCH(QStringList, nameFilters);
    QFETCH(int, filter);
    QFETCH(int, sort);
    QDir::Filters filters = static_cast<QDir::Filters>(filter);
    QDir::SortFlags sorting = static_cast<QDir::SortFlags>(sort);
    QFETCH(QStringList, entries);
    QFETCH(int, caseSensitivity);
    QDir curDir;
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Couldn't create test archive");
    }
    removeTestFiles(fileNames);
    QuaZip zip(zipName);
    QVERIFY(zip.open(QuaZip::mdUnzip));
    QuaZipDir dir(&zip, dirName);
    QVERIFY(dir.exists());
    if (caseSensitivity != -1) {
        dir.setCaseSensitivity(static_cast<QuaZip::CaseSensitivity>(
                                   caseSensitivity));
        QCOMPARE(dir.caseSensitivity(), static_cast<QuaZip::CaseSensitivity>(
                     caseSensitivity));
    }
    QCOMPARE(dir.entryList(nameFilters, filters, sorting), entries);
    // Test default sorting setting.
    dir.setSorting(sorting);
    QCOMPARE(dir.sorting(), sorting);
    QCOMPARE(dir.entryList(nameFilters, filters), entries);
    // Test default name filter setting.
    dir.setNameFilters(nameFilters);
    QCOMPARE(dir.nameFilters(), nameFilters);
    QCOMPARE(dir.entryList(filters), entries);
    // Test default filters.
    dir.setFilter(filters);
    QCOMPARE(dir.filter(), filters);
    QCOMPARE(dir.entryList(), entries);
    QCOMPARE(dir.entryList().count(), static_cast<int>(dir.count()));
    zip.close();
    curDir.remove(zipName);
}

void TestQuaZipDir::cd_data()
{
    QTest::addColumn<QString>("zipName");
    QTest::addColumn<QStringList>("fileNames");
    QTest::addColumn<QString>("dirName");
    QTest::addColumn<QString>("targetDirName");
    QTest::addColumn<QString>("result");
    QTest::newRow("cdDown") << "simple.zip" << (
            QStringList() << "cddown.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << "" << "testdir1" << "testdir1";
    QTest::newRow("cdUp") << "cdup.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << "testdir1" << ".." << "";
    QTest::newRow("cdSide") << "cdside.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << "testdir1" << "../testdir2" << "testdir2";
    QTest::newRow("cdDownUp") << "cdside.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << "" << "testdir1/.." << "";
    QTest::newRow("cdDeep") << "cdside.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << "" << "testdir2/subdir" << "testdir2/subdir";
    QTest::newRow("cdDeeper") << "cdside.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/subdir2/subdir3/test2sub.txt")
            << "testdir2/subdir" << "subdir2/subdir3" << "testdir2/subdir/subdir2/subdir3";
    QTest::newRow("cdRoot") << "cdup.zip" << (
            QStringList() << "test0.txt" << "testdir1/test1.txt"
            << "testdir2/test2.txt" << "testdir2/subdir/test2sub.txt")
            << "testdir1" << "/" << "";
}

void TestQuaZipDir::cd()
{
    QFETCH(QString, zipName);
    QFETCH(QStringList, fileNames);
    QFETCH(QString, dirName);
    QFETCH(QString, targetDirName);
    QFETCH(QString, result);
    QDir curDir;
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Couldn't create test archive");
    }
    removeTestFiles(fileNames);
    QuaZip zip(zipName);
    QVERIFY(zip.open(QuaZip::mdUnzip));
    QuaZipDir dir(&zip, dirName);
    if (dirName.startsWith('/')) {
        dirName = dirName.mid(1);
    }
    if (dirName.endsWith('/')) {
        dirName.chop(1);
    }
    QCOMPARE(dir.path(), dirName);
    {
        int lastSlash = dirName.lastIndexOf('/');
        if (lastSlash == -1) {
            // dirName is just a single name
            if (dirName.isEmpty()) {
                QCOMPARE(dir.dirName(), QString::fromLatin1("."));
            } else {
                QCOMPARE(dir.dirName(), dirName);
            }
        } else {
            // dirName is a sequence
            QCOMPARE(dir.dirName(), dirName.mid(lastSlash + 1));
        }
    }
    if (targetDirName == "..") {
        QVERIFY(dir.cdUp());
    } else {
        QVERIFY(dir.cd(targetDirName));
    }
    QCOMPARE(dir.path(), result);
    // Try to go back
    dir.setPath(dirName);
    QCOMPARE(dir.path(), dirName);
    zip.close();
    curDir.remove(zipName);
}

void TestQuaZipDir::entryInfoList()
{
    QString zipName = "entryInfoList.zip";
    QStringList fileNames;
    fileNames << "test.txt";
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Couldn't create test archive");
    }
    removeTestFiles(fileNames);
    QuaZip zip(zipName);
    QDir curDir;
    QVERIFY(zip.open(QuaZip::mdUnzip));
    QuaZipDir dir(&zip, "/");
    QCOMPARE(dir.entryInfoList().size(), 1);
    QCOMPARE(dir.entryInfoList64().size(), 1);
    zip.close();
    curDir.remove(zipName);
}

void TestQuaZipDir::operators()
{
    QString zipName = "zipDirOperators.zip";
    QStringList fileNames;
    fileNames << "dir/test.txt" << "root.txt";
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Couldn't create test archive");
    }
    removeTestFiles(fileNames);
    QuaZip zip(zipName);
    QDir curDir;
    QVERIFY(zip.open(QuaZip::mdUnzip));
    QuaZipDir dir(&zip, "dir");
    QuaZipDir dir2 = dir; // Copy constructor
    QCOMPARE(dir2.path(), QString::fromLatin1("dir"));
    dir2.cdUp();
    QCOMPARE(dir2.path(), QString::fromLatin1(""));
    QCOMPARE(dir.path(), QString::fromLatin1("dir"));
    dir2 = dir; // operator=()
    QCOMPARE(dir2.path(), QString::fromLatin1("dir"));
    QVERIFY(dir2 == dir); // opertor==
    dir.cd("/");
    QCOMPARE(dir[0], QString::fromLatin1("dir/"));
    QCOMPARE(dir[1], QString::fromLatin1("root.txt"));
    zip.close();
    curDir.remove(zipName);
}

void TestQuaZipDir::filePath()
{
    QString zipName = "entryInfoList.zip";
    QStringList fileNames;
    fileNames << "root.txt" << "dir/test.txt";
    if (!createTestFiles(fileNames)) {
        QFAIL("Couldn't create test files");
    }
    if (!createTestArchive(zipName, fileNames)) {
        QFAIL("Couldn't create test archive");
    }
    removeTestFiles(fileNames);
    QuaZip zip(zipName);
    QDir curDir;
    QVERIFY(zip.open(QuaZip::mdUnzip));
    QuaZipDir dir(&zip);
    QVERIFY(dir.cd("dir"));
    QCOMPARE(dir.relativeFilePath("/root.txt"),
             QString::fromLatin1("../root.txt"));
    QCOMPARE(dir.filePath("test.txt"),
             QString::fromLatin1("dir/test.txt"));
    zip.close();
    curDir.remove(zipName);
}
