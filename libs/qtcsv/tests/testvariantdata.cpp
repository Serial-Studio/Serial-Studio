#include "testvariantdata.h"
#include "qtcsv/variantdata.h"

const double EPSILON = 0.000001;

TestVariantData::TestVariantData()
{
}

void TestVariantData::testCreation()
{
    QtCSV::VariantData varData;

    QVERIFY2(varData.isEmpty(), "Empty VariantData is not empty");
    QVERIFY2(varData.rowCount() == 0,
             "Empty VariantData have too many rows");
}

void TestVariantData::testAddEmptyRow()
{
    QtCSV::VariantData varData;
    varData.addEmptyRow();

    QVERIFY2(false == varData.isEmpty(), "VariantData is empty with empty line");
    QVERIFY2(1 == varData.rowCount(), "Wrong number of rows");
    QVERIFY2(QStringList() == varData.rowValues(0),
             "Wrong data for empty row");
}

void TestVariantData::testAddOneRow()
{
    QList<QVariant> values;
    QString strValue("Hey");
    int intValue = 7000;
    values << strValue << intValue;

    QtCSV::VariantData varData;
    varData.addRow(values);

    QVERIFY2(false == varData.isEmpty(), "VariantData is empty");
    QVERIFY2(1 == varData.rowCount(), "Wrong number of rows");

    QStringList resultValues = varData.rowValues(0);
    QVERIFY2(2 == resultValues.size(), "Wrong number of values");

    int resultInt = resultValues.at(1).toInt();
    QVERIFY2(strValue == resultValues.at(0), "Wrong string data");
    QVERIFY2(resultInt == intValue, "Wrong int data");
}

void TestVariantData::testAddOneRowUsingOneElement()
{
    int expectedValue = 42;
    QVariant varValue(expectedValue);

    QtCSV::VariantData varData;
    varData.addRow(varValue);

    QVERIFY2(false == varData.isEmpty(), "VariantData is empty");
    QVERIFY2(1 == varData.rowCount(), "Wrong number of rows");

    QStringList values = varData.rowValues(0);
    QVERIFY2(1 == values.size(), "Wrong number of values");

    int resultValue = values.at(0).toInt();
    QVERIFY2(resultValue == expectedValue, "Wrong data");
}

void TestVariantData::testAddRows()
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

    QVERIFY2(false == varData.isEmpty(), "VariantData is empty");
    QVERIFY2(3 == varData.rowCount(), "Wrong number of rows");

    QStringList values = varData.rowValues(0);
    QVERIFY2(1 == values.size(), "Wrong number of values for first row");

    double resultValue = values.at(0).toDouble();
    QVERIFY2(resultValue - expectedValue < EPSILON, "Wrong double value");

    QStringList secondRowValues = varData.rowValues(1);
    QVERIFY2(secondRow.size() == secondRowValues.size(),
             "Wrong number of elements in second row");

    QVERIFY2(secondRow.at(0).toString() == secondRowValues.at(0),
             "Wrong first element in second row");

    QVERIFY2(secondRow.at(1).toInt() == secondRowValues.at(1).toInt(),
             "Wrong second element in second row");

    double diff = secondRow.at(2).toDouble() - secondRowValues.at(2).toDouble();
    QVERIFY2(diff <= EPSILON, "Wrong third element in second row");

    QVERIFY2(thirdRow == varData.rowValues(2), "Wrong third row values");
}

void TestVariantData::testClearEmptyData()
{
    QtCSV::VariantData varData;

    QVERIFY2(true == varData.isEmpty(), "VariantData is not empty");
    QVERIFY2(0 == varData.rowCount(), "Wrong number of rows");

    varData.clear();

    QVERIFY2(true == varData.isEmpty(), "VariantData is not empty");
    QVERIFY2(0 == varData.rowCount(), "Wrong number of rows");
}

void TestVariantData::testClearNotEmptyData()
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

    QVERIFY2(false == varData.isEmpty(), "VariantData is empty");
    QVERIFY2(3 == varData.rowCount(), "Wrong number of rows");

    varData.clear();

    QVERIFY2(true == varData.isEmpty(), "VariantData is not empty");
    QVERIFY2(0 == varData.rowCount(), "Wrong number of rows");
}

void TestVariantData::testInsertRows()
{
    double expectedValue = 42.12309;
    QVariant firstRow(expectedValue);

    QList<QVariant> secondRow;
    secondRow << QVariant("kkoo") << QVariant(771) << QVariant(3.14);

    QStringList thirdRow;
    thirdRow << "one" << "two" << "three";

    QString stringOne("hey testing");

    QtCSV::VariantData varData;
    varData.insertRow(0, firstRow);

    QVERIFY2(1 == varData.rowCount(), "Wrong number of rows");
    QVERIFY2(1 == varData.rowValues(0).size(),
             "Wrong number of elements in first row");

    double diff = expectedValue - varData.rowValues(0).at(0).toDouble();
    QVERIFY2(diff <= EPSILON, "Wrong number in first row");

    varData.addEmptyRow();
    varData.addRow(stringOne);
    varData.insertRow(1, secondRow);
    varData.insertRow(100, thirdRow);

    QVERIFY2(5 == varData.rowCount(), "Wrong number of rows");

    diff = expectedValue - varData.rowValues(0).at(0).toDouble();
    QVERIFY2(diff <= EPSILON, "Wrong data for first row");

    QStringList resultSecondRow = varData.rowValues(1);
    QVERIFY2(secondRow.at(0).toString() == resultSecondRow.at(0) &&
             secondRow.at(1).toString() == resultSecondRow.at(1) &&
             secondRow.at(2).toString() == resultSecondRow.at(2),
             "Wrong data for second row");

    QVERIFY2(QStringList() == varData.rowValues(2),
             "Wrong data for third row");

    QVERIFY2((QStringList() << stringOne) == varData.rowValues(3),
             "Wrong data for fourth row");

    QVERIFY2(thirdRow == varData.rowValues(4), "Wrong data for fifth row");
}

void TestVariantData::testCompareForEquality()
{
    QStringList firstRow, secondRow;
    firstRow << "one" << "two" << "three";
    secondRow << "four" << "five";

    QtCSV::VariantData firstData;
    firstData.addRow(firstRow);
    firstData.addRow(secondRow);

    QtCSV::VariantData secondData;
    secondData.addRow(firstRow);
    secondData.addRow(secondRow);

    QVERIFY2(firstData == firstData,
             "Failed to compare for equality same object");
    QVERIFY2(false == (firstData != firstData),
             "Failed to compare for equality same object");

    QVERIFY2(firstData == secondData, "Objects are not the same");
    QVERIFY2(false == (firstData != secondData), "Objects are not the same");

    secondData.addRow(firstRow);

    QVERIFY2(false == (firstData == secondData), "Objects are the same");
    QVERIFY2(firstData != secondData, "Objects are the same");
}

void TestVariantData::testCopyConstruction()
{
    QStringList firstRow, secondRow;
    firstRow << "one" << "two" << "three";
    secondRow << "four" << "five";

    QtCSV::VariantData firstData;
    firstData.addRow(firstRow);
    firstData.addRow(secondRow);

    {
        QtCSV::VariantData secondData(firstData);

        QVERIFY2(firstData.rowCount() == secondData.rowCount(),
                 "Wrong number of rows");
        QVERIFY2(firstRow == secondData.rowValues(0),
                 "Wrong data for first row");
        QVERIFY2(secondRow == secondData.rowValues(1),
                 "Wrong data for second row");
    }

    QVERIFY2(2 == firstData.rowCount(), "Wrong number of rows");
    QVERIFY2(firstRow == firstData.rowValues(0), "Wrong data for first row");
    QVERIFY2(secondRow == firstData.rowValues(1), "Wrong data for second row");
}

void TestVariantData::testCopyAssignment()
{
    QStringList firstRow, secondRow;
    firstRow << "one" << "two" << "three";
    secondRow << "four" << "five";

    QtCSV::VariantData firstData;
    firstData.addRow(firstRow);
    firstData.addRow(secondRow);

    {
        QtCSV::VariantData secondData;
        secondData = firstData;

        QVERIFY2(firstData.rowCount() == secondData.rowCount(),
                 "Wrong number of rows");
        QVERIFY2(firstRow == secondData.rowValues(0),
                 "Wrong data for first row");
        QVERIFY2(secondRow == secondData.rowValues(1),
                 "Wrong data for second row");
    }

    QVERIFY2(2 == firstData.rowCount(), "Wrong number of rows");
    QVERIFY2(firstRow == firstData.rowValues(0), "Wrong data for first row");
    QVERIFY2(secondRow == firstData.rowValues(1), "Wrong data for second row");
}

void TestVariantData::testOperatorInput()
{
    QtCSV::VariantData data;
    data << QString("1") << QVariant(double(3.14));

    QStringList thirdRow;
    thirdRow << "one" << "two" << "three";

    data << thirdRow;

    QVERIFY2(3 == data.rowCount(), "Wrong number of rows");

    QStringList expectedFirstRow, expectedSecondRow;
    expectedFirstRow << "1";
    expectedSecondRow << QVariant(double(3.14)).toString();

    QVERIFY2(expectedFirstRow == data.rowValues(0), "Wrong data for first row");
    QVERIFY2(expectedSecondRow == data.rowValues(1),
             "Wrong data for second row");
    QVERIFY2(thirdRow == data.rowValues(2), "Wrong data for third row");
}

void TestVariantData::testRemoveRow()
{
    QtCSV::VariantData data;
    data.removeRow(0);
    data.removeRow(563);

    QVERIFY2(true == data.isEmpty(), "Container is not empty");

    QStringList valuesFirst, valuesSecond;
    valuesFirst << "one" << "two" << "three";
    valuesSecond << "asgreg" << "ertetw" << "";

    QString stringOne("hey test"), stringTwo("sdfwioiouoioi");

    data << valuesFirst << valuesSecond << stringOne << stringTwo;

    data.removeRow(2);

    QVERIFY2(3 == data.rowCount(), "Wrong number of rows");
    QVERIFY2(valuesFirst == data.rowValues(0), "Wrong data for first row");
    QVERIFY2(valuesSecond == data.rowValues(1), "Wrong data for second row");
    QVERIFY2((QStringList() << stringTwo) == data.rowValues(2),
             "Wrong data for third row");
}

void TestVariantData::testReplaceRow()
{
    QStringList valuesFirst, valuesSecond;
    valuesFirst << "one" << "two" << "three";
    valuesSecond << "asgreg" << "ertetw" << "";

    QString stringOne("hey test"), stringTwo("sdfwioiouoioi");

    QtCSV::VariantData data;
    data << valuesFirst << valuesSecond << stringOne;

    data.replaceRow(0, stringTwo);
    QVERIFY2((QStringList() << stringTwo) == data.rowValues(0),
             "Wrong data for first row");

    data.replaceRow(2, QStringList());
    QVERIFY2(QStringList() == data.rowValues(2), "Wrong data for third row");

    data.replaceRow(1, valuesFirst);
    QVERIFY2(valuesFirst == data.rowValues(1), "Wrong data for second row");
}
