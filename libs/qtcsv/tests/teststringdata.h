#ifndef TESTSTRINGDATA_H
#define TESTSTRINGDATA_H

#include <QtTest>

class TestStringData : public QObject
{
    Q_OBJECT

public:
    TestStringData();

private Q_SLOTS:
    void testCreation();
    void testAddEmptyRow();
    void testAddOneRow();
    void testAddOneRowUsingOneString();
    void testAddRows();
    void testClearEmptyData();
    void testClearNotEmptyData();
    void testInsertRows();
    void testCompareForEquality();
    void testCopyConstruction();
    void testCopyAssignment();
    void testOperatorInput();
    void testRemoveRow();
    void testReplaceRow();
};

#endif // TESTSTRINGDATA_H
