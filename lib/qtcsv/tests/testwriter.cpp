#include "testwriter.h"
#include "qtcsv/reader.h"
#include "qtcsv/variantdata.h"
#include "qtcsv/writer.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <exception>

void TestWriter::cleanup()
{
  if (QFile::exists(getFilePath()) && !QFile::remove(getFilePath()))
  {
    qDebug() << "Can't remove file:" << getFilePath();
  }

  if (QFile::exists(getFilePathXLS()) && !QFile::remove(getFilePathXLS()))
  {
    qDebug() << "Can't remove file:" << getFilePathXLS();
  }

  if (QFile::exists(getFilePathWithDotsInName())
      && !QFile::remove(getFilePathWithDotsInName()))
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
  QVERIFY2(!QtCSV::Writer::write(QString(), QtCSV::StringData()),
           "Invalid arguments was accepted");

  QVERIFY2(!QtCSV::Writer::write(getFilePath(), QtCSV::StringData()),
           "Empty data was accepted");

  QtCSV::StringData strData;
  strData << "one" << "two" << "three";

  QVERIFY2(!QtCSV::Writer::write(QString(), strData),
           "Empty path was accepted");

  QVERIFY2(!QtCSV::Writer::write("./some/path.csv", strData),
           "Relative path to csv-file was accepted");

  QVERIFY2(QtCSV::Writer::write(getFilePathXLS(), strData),
           "Absolute path to xls-file was not accepted");
}

void TestWriter::testWriteFromStringData()
{
  QList<QString> strList;
  strList << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(strList);

  const auto writeResult = QtCSV::Writer::write(getFilePath(), strData);
  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath());
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(1 == data.size(), "Wrong number of rows");
  QVERIFY2(strList == data.at(0), "Wrong data");
}

void TestWriter::testWriteFromVariantData()
{
  const auto expectedValue = 42.12309;
  QVariant firstRow(expectedValue);

  QList<QVariant> secondRow;
  secondRow << QVariant("kkoo") << QVariant(771) << QVariant(3.14);

  QList<QString> thirdRow;
  thirdRow << "one" << "two" << "three";

  QtCSV::VariantData varData;
  varData.addRow(firstRow);
  varData.addRow(secondRow);
  varData.addRow(thirdRow);

  const auto writeResult = QtCSV::Writer::write(getFilePath(), varData);
  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath());
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(3 == data.size(), "Wrong number of rows");
  QVERIFY2(varData.rowValues(0) == data.at(0), "Wrong values in first row");
  QVERIFY2(varData.rowValues(1) == data.at(1), "Wrong values in second row");
  QVERIFY2(varData.rowValues(2) == data.at(2), "Wrong values in third row");
}

void TestWriter::testWriteToFileWithDotsInName()
{
  QList<QString> strList;
  strList << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(strList);

  const auto writeResult
      = QtCSV::Writer::write(getFilePathWithDotsInName(), strData);
  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePathWithDotsInName());
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(1 == data.size(), "Wrong number of rows");
  QVERIFY2(strList == data.at(0), "Wrong data");
}

void TestWriter::testWriteAppendMode()
{
  QList<QString> strFirstList;
  strFirstList << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(strFirstList);

  const auto writeResult = QtCSV::Writer::write(getFilePath(), strData);
  QVERIFY2(writeResult, "Failed to write to file");

  QList<QString> strSecondList;
  strSecondList << "3" << "2" << "1.1";

  QtCSV::StringData newStrData;
  newStrData.addRow(strSecondList);

  const auto newWriteResult = QtCSV::Writer::write(
      getFilePath(), newStrData, ",", {}, QtCSV::Writer::WriteMode::APPEND);

  QVERIFY2(newWriteResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath());
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(2 == data.size(), "Wrong number of rows");
  QVERIFY2(strFirstList == data.at(0), "Wrong first row data");
  QVERIFY2(strSecondList == data.at(1), "Wrong second row data");
}

void TestWriter::testWriteWithNotDefaultSeparator()
{
  QList<QString> strList;
  strList << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(strList);

  const QString separator("++");
  const auto writeResult
      = QtCSV::Writer::write(getFilePath(), strData, separator);
  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath(), separator);
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(1 == data.size(), "Wrong number of rows");
  QVERIFY2(strList == data.at(0), "Wrong data");
}

void TestWriter::testWriteWithHeader()
{
  QList<QString> header;
  header << "1" << "2";

  QList<QString> strList;
  strList << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(strList);

  const auto writeResult
      = QtCSV::Writer::write(getFilePath(), strData, ",", {},
                             QtCSV::Writer::WriteMode::REWRITE, header);

  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath());
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(2 == data.size(), "Wrong number of rows");
  QVERIFY2(header == data.at(0), "Wrong header");
  QVERIFY2(strList == data.at(1), "Wrong data");
}

void TestWriter::testWriteWithFooter()
{
  QList<QString> footer;
  footer << "Here is a footer";

  QList<QString> strList;
  strList << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(strList);

  const auto writeResult
      = QtCSV::Writer::write(getFilePath(), strData, ",", {},
                             QtCSV::Writer::WriteMode::REWRITE, {}, footer);

  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath());
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(2 == data.size(), "Wrong number of rows");
  QVERIFY2(strList == data.at(0), "Wrong data");
  QVERIFY2(footer == data.at(1), "Wrong footer");
}

void TestWriter::testWriteWithHeaderAndFooter()
{
  QList<QString> header;
  header << "1" << "2";

  QList<QString> footer;
  footer << "Here is a footer";

  QList<QString> strList;
  strList << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(strList);

  const auto writeResult
      = QtCSV::Writer::write(getFilePath(), strData, ",", {},
                             QtCSV::Writer::WriteMode::REWRITE, header, footer);

  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath());
  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(3 == data.size(), "Wrong number of rows");
  QVERIFY2(header == data.at(0), "Wrong header");
  QVERIFY2(strList == data.at(1), "Wrong data");
  QVERIFY2(footer == data.at(2), "Wrong footer");
}

void TestWriter::testWriterDataContainSeparators()
{
  QList<QString> strList;
  strList << "one" << "two" << "three, four";

  QString strLine("this, is, one, element");

  QtCSV::StringData strData;
  strData.addRow(strList);
  strData.addRow(strLine);

  const auto writeResult
      = QtCSV::Writer::write(getFilePath(), strData, ",", "\"");
  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath(), ",", "\"");

  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(2 == data.size(), "Wrong number of rows");
  QVERIFY2(strList == data.at(0), "Wrong data at first row");
  QVERIFY2((QList<QString>() << strLine) == data.at(1),
           "Wrong data at second row");
}

void TestWriter::testWriteDifferentDataAmount()
{
  auto rowsNumber = 10;
  auto rowsMultiplier = 2;
  auto rowCycles = 2;
  QElapsedTimer time;
  for (auto rc = 0; rc < rowCycles; ++rc)
  {
    auto symbolsNumber = 10;
    auto symbolsMultiplier = 5;
    auto symbolCycles = 4;
    for (auto sc = 0; sc < symbolCycles; ++sc)
    {
      QtCSV::StringData data;
      try
      {
        data = getTestStringData(symbolsNumber, rowsNumber);
      }
      catch (std::exception & /*e*/)
      {
        QFAIL("No enough memory to create data object");
      }

      QVERIFY2(!data.isEmpty(), "Failed to create content");

      auto writeResult = false;
      time.restart();
      try
      {
        writeResult = QtCSV::Writer::write(getFilePath(), data, ",", {});
      }
      catch (std::exception & /*e*/)
      {
        QFAIL("No enough memory to write data to the file");
      }

      qDebug() << "symbols:" << symbolsNumber << ", rows:" << rowsNumber
               << ", time:" << time.elapsed() << "ms";

      QVERIFY2(writeResult, "Failed to write to file");

      QFile csvFile(getFilePath());
      if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QFAIL("Failed to open created csv-file");
      }

      QTextStream stream(&csvFile);
      for (auto line = 0; line < data.rowCount(); ++line)
      {
        QList<QString> lineElements = stream.readLine().split(",");
        QVERIFY2(data.rowValues(line) == lineElements,
                 "Original and result data are not the same");
      }

      csvFile.close();

      symbolsNumber *= symbolsMultiplier;
    }

    rowsNumber *= rowsMultiplier;
  }
}

QtCSV::StringData TestWriter::getTestStringData(qsizetype symbolsInRow,
                                                qsizetype rowsNumber)
{
  if (symbolsInRow <= 0 || rowsNumber <= 0)
  {
    qDebug() << __FUNCTION__ << "Invalid argumnets";
    return {};
  }

  QList<QString> elements;
  elements << "1234567890" << "3.14159265359" << "abcdefgh" << "ijklmnopqrs"
           << "tuvwxyz" << "ABCDEFGH" << "IJKLMNOPQRS" << "TUVWXYZ"
           << "some_STRANGE-string=" << "?!\\|/*+.<>@#$%^&(){}[]'`~";

  QList<QString> rowElements;
  auto rowLength = 0;
  auto elementIndex = 0;
  while (rowLength < symbolsInRow)
  {
    if (elements.size() <= elementIndex)
    {
      elementIndex = 0;
    }

    QString nextElement = elements.at(elementIndex);
    if (symbolsInRow < rowLength + nextElement.size())
    {
      nextElement.resize(symbolsInRow - rowLength);
    }

    rowElements << nextElement;
    rowLength += nextElement.size();
    ++elementIndex;
  }

  QtCSV::StringData data;
  for (auto i = 0; i < rowsNumber; ++i)
  {
    data.addRow(rowElements);
  }

  return data;
}

void TestWriter::testWriteDataContainCRLF()
{
  QList<QString> firstLine = QList<QString>()
                             << "one" << "two" << "three\nfour,five";

  QList<QString> secondLine = QList<QString>()
                              << "six" << "seven,eight" << "nine,\rten";

  QtCSV::StringData strData;
  strData.addRow(firstLine);
  strData.addRow(secondLine);

  const auto writeResult
      = QtCSV::Writer::write(getFilePath(), strData, ",", QString());
  QVERIFY2(writeResult, "Failed to write to file");

  const auto data = QtCSV::Reader::readToList(getFilePath(), ",", "\"");

  QVERIFY2(!data.isEmpty(), "Failed to read file content");
  QVERIFY2(2 == data.size(), "Wrong number of rows");
  QVERIFY2(firstLine == data.at(0), "Wrong data at first row");
  QVERIFY2(secondLine == data.at(1), "Wrong data at second row");
}
