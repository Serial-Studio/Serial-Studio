#include "teststringdata.h"
#include "qtcsv/stringdata.h"
#include <QDebug>
#include <QTest>

void TestStringData::testCreation()
{
  QtCSV::StringData strData;

  QVERIFY2(strData.isEmpty(), "Empty StringData is not empty");
  QVERIFY2(strData.rowCount() == 0, "Empty StringData have too many rows");
}

void TestStringData::testAddEmptyRow()
{
  QtCSV::StringData strData;
  strData.addEmptyRow();

  QVERIFY2(!strData.isEmpty(), "StringData is empty with empty line");
  QVERIFY2(1 == strData.rowCount(), "Wrong number of rows");
  QVERIFY2(QList<QString>() == strData.rowValues(0),
           "Wrong data for empty row");
}

void TestStringData::testAddOneRow()
{
  QList<QString> rowValues;
  rowValues << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(rowValues);

  QVERIFY2(!strData.isEmpty(), "StringData is empty");
  QVERIFY2(1 == strData.rowCount(), "Wrong number of rows");
  QVERIFY2(rowValues == strData.rowValues(0), "Wrong data for empty row");
}

void TestStringData::testAddOneRowUsingOneString()
{
  QString value("faklj;");

  QtCSV::StringData strData;
  strData.addRow(value);

  QVERIFY2(!strData.isEmpty(), "StringData is empty");
  QVERIFY2(1 == strData.rowCount(), "Wrong number of rows");

  QList<QString> expectedRow;
  expectedRow << value;
  QVERIFY2(expectedRow == strData.rowValues(0), "Wrong data for empty row");
}

void TestStringData::testAddRows()
{
  QList<QString> valuesFirst;
  valuesFirst << "1" << "2" << "3";

  QList<QString> valuesSecond;

  QList<QString> valuesThird;
  valuesFirst << "hhh" << "ttyyeeqp[" << "n...589129";

  QtCSV::StringData strData;
  strData.addRow(valuesFirst);
  strData.addRow(valuesSecond);
  strData.addRow(valuesThird);

  QVERIFY2(!strData.isEmpty(), "StringData is empty");
  QVERIFY2(3 == strData.rowCount(), "Wrong number of rows");
  QVERIFY2(valuesFirst == strData.rowValues(0), "Wrong data for first row");
  QVERIFY2(valuesSecond == strData.rowValues(1), "Wrong data for second row");
  QVERIFY2(valuesThird == strData.rowValues(2), "Wrong data for third row");
}

void TestStringData::testClearEmptyData()
{
  QtCSV::StringData strData;
  QVERIFY2(strData.isEmpty(), "StringData is not empty");

  strData.clear();
  QVERIFY2(strData.isEmpty(), "StringData is not empty");
}

void TestStringData::testClearNotEmptyData()
{
  QList<QString> rowValues;
  rowValues << "one" << "two" << "three";

  QtCSV::StringData strData;
  strData.addRow(rowValues);
  QVERIFY2(!strData.isEmpty(), "StringData is empty");

  strData.clear();
  QVERIFY2(strData.isEmpty(), "StringData is not empty");
}

void TestStringData::testInsertRows()
{
  QList<QString> valuesFirst, valuesSecond;
  valuesFirst << "one" << "two" << "three";
  valuesSecond << "asgreg" << "ertetw" << "";

  QString stringOne("hey test"), stringTwo("sdfwioiouoioi");

  QtCSV::StringData strData;
  strData.insertRow(0, valuesFirst);

  QVERIFY2(1 == strData.rowCount(), "Wrong number of rows");
  QVERIFY2(valuesFirst == strData.rowValues(0), "Wrong data for first row");

  strData.addEmptyRow();
  strData.addRow(stringOne);
  strData.insertRow(1, valuesSecond);
  strData.insertRow(100, stringTwo);

  QVERIFY2(5 == strData.rowCount(), "Wrong number of rows");
  QVERIFY2(valuesFirst == strData.rowValues(0), "Wrong data for first row");
  QVERIFY2(valuesSecond == strData.rowValues(1), "Wrong data for second row");
  QVERIFY2(strData.rowValues(2).isEmpty(), "Wrong data for third row");
  QVERIFY2((QList<QString>() << stringOne) == strData.rowValues(3),
           "Wrong data for fourth row");

  QVERIFY2((QList<QString>() << stringTwo) == strData.rowValues(4),
           "Wrong data for fifth row");
}

void TestStringData::testCompareForEquality()
{
  QList<QString> firstRow, secondRow;
  firstRow << "one" << "two" << "three";
  secondRow << "four" << "five";

  QtCSV::StringData firstData;
  firstData.addRow(firstRow);
  firstData.addRow(secondRow);

  QtCSV::StringData secondData;
  secondData.addRow(firstRow);
  secondData.addRow(secondRow);

  QVERIFY2(firstData == firstData,
           "Failed to compare for equality same object");

  QVERIFY2(!(firstData != firstData),
           "Failed to compare for equality same object");

  QVERIFY2(firstData == secondData, "Objects are not the same");
  QVERIFY2(!(firstData != secondData), "Objects are not the same");

  secondData.addRow(firstRow);

  QVERIFY2(!(firstData == secondData), "Objects are the same");
  QVERIFY2(firstData != secondData, "Objects are the same");
}

void TestStringData::testCopyConstruction()
{
  QList<QString> firstRow, secondRow;
  firstRow << "one" << "two" << "three";
  secondRow << "four" << "five";

  QtCSV::StringData firstData;
  firstData.addRow(firstRow);
  firstData.addRow(secondRow);

  {
    QtCSV::StringData secondData(firstData);

    QVERIFY2(firstData.rowCount() == secondData.rowCount(),
             "Wrong number of rows");

    QVERIFY2(firstRow == secondData.rowValues(0), "Wrong data for first row");

    QVERIFY2(secondRow == secondData.rowValues(1), "Wrong data for second row");
  }

  QVERIFY2(2 == firstData.rowCount(), "Wrong number of rows");
  QVERIFY2(firstRow == firstData.rowValues(0), "Wrong data for first row");
  QVERIFY2(secondRow == firstData.rowValues(1), "Wrong data for second row");
}

void TestStringData::testCopyAssignment()
{
  QList<QString> firstRow, secondRow;
  firstRow << "one" << "two" << "three";
  secondRow << "four" << "five";

  QtCSV::StringData firstData;
  firstData.addRow(firstRow);
  firstData.addRow(secondRow);

  {
    QtCSV::StringData secondData;
    secondData = firstData;

    QVERIFY2(firstData.rowCount() == secondData.rowCount(),
             "Wrong number of rows");

    QVERIFY2(firstRow == secondData.rowValues(0), "Wrong data for first row");

    QVERIFY2(secondRow == secondData.rowValues(1), "Wrong data for second row");
  }

  QVERIFY2(2 == firstData.rowCount(), "Wrong number of rows");
  QVERIFY2(firstRow == firstData.rowValues(0), "Wrong data for first row");
  QVERIFY2(secondRow == firstData.rowValues(1), "Wrong data for second row");
}

void TestStringData::testOperatorInput()
{
  QtCSV::StringData data;
  data << QString("1") << "one";

  QList<QString> thirdRow;
  thirdRow << "one" << "two" << "three";

  data << thirdRow;

  QVERIFY2(3 == data.rowCount(), "Wrong number of rows");

  QList<QString> expectedFirstRow, expectedSecondRow;
  expectedFirstRow << "1";
  expectedSecondRow << "one";

  QVERIFY2(expectedFirstRow == data.rowValues(0), "Wrong data for first row");
  QVERIFY2(expectedSecondRow == data.rowValues(1), "Wrong data for second row");

  QVERIFY2(thirdRow == data.rowValues(2), "Wrong data for third row");
}

void TestStringData::testRemoveRow()
{
  QtCSV::StringData strData;
  strData.removeRow(0);
  strData.removeRow(563);

  QVERIFY2(strData.isEmpty(), "Container is not empty");

  QList<QString> valuesFirst, valuesSecond;
  valuesFirst << "one" << "two" << "three";
  valuesSecond << "asgreg" << "ertetw" << "";

  QString stringOne("hey test"), stringTwo("sdfwioiouoioi");

  strData << valuesFirst << valuesSecond << stringOne << stringTwo;

  strData.removeRow(2);

  QVERIFY2(3 == strData.rowCount(), "Wrong number of rows");
  QVERIFY2(valuesFirst == strData.rowValues(0), "Wrong data for first row");
  QVERIFY2(valuesSecond == strData.rowValues(1), "Wrong data for second row");
  QVERIFY2((QList<QString>() << stringTwo) == strData.rowValues(2),
           "Wrong data for third row");
}

void TestStringData::testReplaceRow()
{
  QList<QString> valuesFirst, valuesSecond;
  valuesFirst << "one" << "two" << "three";
  valuesSecond << "asgreg" << "ertetw" << "";

  QString stringOne("hey test"), stringTwo("sdfwioiouoioi");

  QtCSV::StringData strData;
  strData << valuesFirst << valuesSecond << stringOne;

  strData.replaceRow(0, stringTwo);
  QVERIFY2((QList<QString>() << stringTwo) == strData.rowValues(0),
           "Wrong data for first row");

  strData.replaceRow(2, QList<QString>());
  QVERIFY2(QList<QString>() == strData.rowValues(2),
           "Wrong data for third row");

  strData.replaceRow(1, valuesFirst);
  QVERIFY2(valuesFirst == strData.rowValues(1), "Wrong data for second row");
}
