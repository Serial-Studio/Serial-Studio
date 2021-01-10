#include "testwriter.h"

#include <exception>

#include <QDir>
#include <QFile>
#include <QElapsedTimer>
#include <QDebug>

#include "qtcsv/writer.h"
#include "qtcsv/reader.h"
#include "qtcsv/variantdata.h"

TestWriter::TestWriter()
{
}

void TestWriter::cleanup()
{
    if ( QFile::exists(getFilePath()) &&
         false == QFile::remove(getFilePath()) )
    {
        qDebug() << "Can't remove file:" << getFilePath();
    }

    if ( QFile::exists(getFilePathXLS()) &&
         false == QFile::remove(getFilePathXLS()) )
    {
        qDebug() << "Can't remove file:" << getFilePathXLS();
    }

    if ( QFile::exists(getFilePathWithDotsInName()) &&
         false == QFile::remove(getFilePathWithDotsInName()) )
    {
        qDebug() << "Can't remove file:" << getFilePathWithDotsInName();
    }
}

QString TestWriter::getFilePath() const
{
    return QDir::currentPath() + "/test-file.csv";
}

QString TestWriter::getFilePathXLS() const
{
    return QDir::currentPath() + "/test-file.xls";
}

QString TestWriter::getFilePathWithDotsInName() const
{
    return QDir::currentPath() + "/test.file.dots.csv";
}

void TestWriter::testWriteInvalidArgs()
{
    QVERIFY2(false == QtCSV::Writer::write(QString(), QtCSV::StringData()),
             "Invalid arguments was accepted");

    QVERIFY2(false == QtCSV::Writer::write(getFilePath(), QtCSV::StringData()),
             "Empty data was accepted");

    QtCSV::StringData strData;
    strData << "one" << "two" << "three";

    QVERIFY2(false == QtCSV::Writer::write(QString(), strData),
             "Empty path was accepted");

    QVERIFY2(false == QtCSV::Writer::write("./some/path.csv", strData),
             "Relative path to csv-file was accepted");

    QVERIFY2(QtCSV::Writer::write(getFilePathXLS(), strData),
             "Absolute path to xls-file was not accepted");
}

void TestWriter::testWriteFromStringData()
{
    QStringList strList;
    strList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strList);

    bool writeResult = QtCSV::Writer::write(getFilePath(), strData);
    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath());
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(1 == data.size(), "Wrong number of rows");
    QVERIFY2(strList == data.at(0), "Wrong data");
}

void TestWriter::testWriteFromVariantData()
{
    double expectedValue = 42.12309;
    QVariant firstRow(expectedValue);

    QList<QVariant> secondRow;
    secondRow << QVariant("kkoo") << QVariant(771) << QVariant(3.14);

    QStringList thirdRow;
    thirdRow << "one" << "two" << "three";

    QtCSV::VariantData varData;
    varData.addRow(firstRow);
    varData.addRow(secondRow);
    varData.addRow(thirdRow);

    bool writeResult = QtCSV::Writer::write(getFilePath(), varData);
    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath());
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(3 == data.size(), "Wrong number of rows");
    QVERIFY2(varData.rowValues(0) == data.at(0),
             "Wrong values in first row");

    QVERIFY2(varData.rowValues(1) == data.at(1),
             "Wrong values in second row");

    QVERIFY2(varData.rowValues(2) == data.at(2),
             "Wrong values in third row");
}

void TestWriter::testWriteToFileWithDotsInName()
{
    QStringList strList;
    strList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strList);

    bool writeResult =
            QtCSV::Writer::write(getFilePathWithDotsInName(), strData);
    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data =
            QtCSV::Reader::readToList(getFilePathWithDotsInName());
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(1 == data.size(), "Wrong number of rows");
    QVERIFY2(strList == data.at(0), "Wrong data");
}

void TestWriter::testWriteAppendMode()
{
    QStringList strFirstList;
    strFirstList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strFirstList);

    bool writeResult = QtCSV::Writer::write(getFilePath(), strData);
    QVERIFY2(true == writeResult, "Failed to write to file");

    QStringList strSecondList;
    strSecondList << "3" << "2" << "1.1";

    QtCSV::StringData newStrData;
    newStrData.addRow(strSecondList);

    bool newWriteResult = QtCSV::Writer::write(getFilePath(),
                                              newStrData,
                                              ",",
                                              QString(),
                                              QtCSV::Writer::APPEND);

    QVERIFY2(true == newWriteResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath());
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(2 == data.size(), "Wrong number of rows");
    QVERIFY2(strFirstList == data.at(0), "Wrong first row data");
    QVERIFY2(strSecondList == data.at(1), "Wrong second row data");
}

void TestWriter::testWriteWithNotDefaultSeparator()
{
    QStringList strList;
    strList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strList);

    const QString separator("++");
    bool writeResult = QtCSV::Writer::write(getFilePath(), strData, separator);
    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data =
            QtCSV::Reader::readToList(getFilePath(), separator);
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(1 == data.size(), "Wrong number of rows");
    QVERIFY2(strList == data.at(0), "Wrong data");
}

void TestWriter::testWriteWithHeader()
{
    QStringList header;
    header << "1" << "2";

    QStringList strList;
    strList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strList);

    bool writeResult = QtCSV::Writer::write(getFilePath(),
                                            strData,
                                            ",",
                                            QString(),
                                            QtCSV::Writer::REWRITE,
                                            header);

    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath());
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(2 == data.size(), "Wrong number of rows");
    QVERIFY2(header == data.at(0), "Wrong header");
    QVERIFY2(strList == data.at(1), "Wrong data");
}

void TestWriter::testWriteWithFooter()
{
    QStringList footer;
    footer << "Here is a footer";

    QStringList strList;
    strList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strList);

    bool writeResult = QtCSV::Writer::write(getFilePath(),
                                            strData,
                                            ",",
                                            QString(),
                                            QtCSV::Writer::REWRITE,
                                            QStringList(),
                                            footer);

    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath());
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(2 == data.size(), "Wrong number of rows");
    QVERIFY2(strList == data.at(0), "Wrong data");
    QVERIFY2(footer == data.at(1), "Wrong footer");
}

void TestWriter::testWriteWithHeaderAndFooter()
{
    QStringList header;
    header << "1" << "2";

    QStringList footer;
    footer << "Here is a footer";

    QStringList strList;
    strList << "one" << "two" << "three";

    QtCSV::StringData strData;
    strData.addRow(strList);

    bool writeResult = QtCSV::Writer::write(getFilePath(),
                                            strData,
                                            ",",
                                            QString(),
                                            QtCSV::Writer::REWRITE,
                                            header,
                                            footer);

    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath());
    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(3 == data.size(), "Wrong number of rows");
    QVERIFY2(header == data.at(0), "Wrong header");
    QVERIFY2(strList == data.at(1), "Wrong data");
    QVERIFY2(footer == data.at(2), "Wrong footer");
}

void TestWriter::testWriterDataContainSeparators()
{
    QStringList strList;
    strList << "one" << "two" << "three, four";

    QString strLine("this, is, one, element");

    QtCSV::StringData strData;
    strData.addRow(strList);
    strData.addRow(strLine);

    bool writeResult = QtCSV::Writer::write(getFilePath(),
                                            strData,
                                            ",",
                                            "\"");
    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath(),
                                                        ",",
                                                        "\"");

    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(2 == data.size(), "Wrong number of rows");
    QVERIFY2(strList == data.at(0), "Wrong data at first row");
    QVERIFY2((QStringList() << strLine) == data.at(1),
             "Wrong data at second row");
}

void TestWriter::testWriteDifferentDataAmount()
{
    int rowsNumber = 10;
    int rowsMultiplier  = 2;
    int rowCycles = 2;
    QElapsedTimer time;
    for ( int rc = 0; rc < rowCycles; ++rc )
    {
        int symbolsNumber = 10;
        int symbolsMultiplier  = 5;
        int symbolCycles = 4;
        for ( int sc = 0; sc < symbolCycles; ++sc )
        {
            QtCSV::StringData data;
            try
            {
                data = getTestStringData(symbolsNumber, rowsNumber);
            }
            catch (std::exception &e)
            {
                Q_UNUSED(e);
                QFAIL("No enough memory to create data object");
            }

            QVERIFY2(false == data.isEmpty(), "Failed to create content");

            bool writeResult = false;
            time.restart();
            try
            {
                writeResult = QtCSV::Writer::write(
                                  getFilePath(), data, ",", QString());
            }
            catch (std::exception &e)
            {
                Q_UNUSED(e);
                QFAIL("No enough memory to write data to the file");
            }

            qDebug() << "symbols:" << symbolsNumber <<
                        ", rows:" << rowsNumber <<
                        ", time:" << time.elapsed() << "ms";

            QVERIFY2(true == writeResult, "Failed to write to file");

            QFile csvFile(getFilePath());
            if ( false == csvFile.open(QIODevice::ReadOnly | QIODevice::Text) )
            {
                QFAIL("Failed to open created csv-file");
            }

            QTextStream stream(&csvFile);
            for ( int line = 0; line < data.rowCount(); ++line )
            {
                QStringList lineElements = stream.readLine().split(",");
                QVERIFY2(data.rowValues(line) == lineElements,
                         "Original and result data are not the same");
            }

            csvFile.close();

            symbolsNumber *= symbolsMultiplier;
        }

        rowsNumber *= rowsMultiplier;
    }
}

QtCSV::StringData TestWriter::getTestStringData(const int &symbolsInRow,
                                                const int &rowsNumber)
{
    if ( symbolsInRow <= 0 || rowsNumber <= 0 )
    {
        qDebug() << __FUNCTION__ << "Invalid argumnets";
        return QtCSV::StringData();
    }

    QStringList elements;
    elements << "1234567890" << "3.14159265359" << "abcdefgh" <<
                "ijklmnopqrs" << "tuvwxyz" << "ABCDEFGH" << "IJKLMNOPQRS" <<
                "TUVWXYZ" << "some_STRANGE-string=" <<
                "?!\\|/*+.<>@#$%^&(){}[]'`~";

    QStringList rowElements;
    int rowLength = 0;
    int elementIndex = 0;
    while ( rowLength < symbolsInRow )
    {
        if ( elements.size() <= elementIndex )
        {
            elementIndex = 0;
        }

        QString nextElement = elements.at(elementIndex);
        if ( symbolsInRow < rowLength + nextElement.size() )
        {
            nextElement.resize( symbolsInRow - rowLength );
        }

        rowElements << nextElement;
        rowLength += nextElement.size();
        ++elementIndex;
    }

    QtCSV::StringData data;
    for ( int i = 0; i < rowsNumber; ++i )
    {
        data.addRow(rowElements);
    }

    return data;
}

void TestWriter::testWriteDataContainCRLF()
{
    QStringList firstLine = QStringList() << "one" << "two" <<
                                             "three\nfour,five";

    QStringList secondLine = QStringList() << "six" << "seven,eight" <<
                                              "nine,\rten";

    QtCSV::StringData strData;
    strData.addRow(firstLine);
    strData.addRow(secondLine);

    bool writeResult = QtCSV::Writer::write(getFilePath(),
                                            strData,
                                            ",",
                                            QString());
    QVERIFY2(true == writeResult, "Failed to write to file");

    QList<QStringList> data = QtCSV::Reader::readToList(getFilePath(),
                                                        ",",
                                                        "\"");

    QVERIFY2(false == data.isEmpty(), "Failed to read file content");
    QVERIFY2(2 == data.size(), "Wrong number of rows");
    QVERIFY2(firstLine == data.at(0), "Wrong data at first row");
    QVERIFY2(secondLine == data.at(1), "Wrong data at second row");
}
