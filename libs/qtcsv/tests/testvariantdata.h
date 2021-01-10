#ifndef TESTVARIANTDATA_H
#define TESTVARIANTDATA_H

#include <QObject>
#include <QtTest>

class TestVariantData : public QObject
{
    Q_OBJECT

public:
    TestVariantData();

private Q_SLOTS:
    void testCreation();
    void testAddEmptyRow();
    void testAddOneRow();
    void testAddOneRowUsingOneElement();
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

#endif // TESTVARIANTDATA_H
