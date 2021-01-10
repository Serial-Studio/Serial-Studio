#ifndef TESTWRITER_H
#define TESTWRITER_H

#include <QObject>
#include <QtTest>

#include "qtcsv/stringdata.h"

class TestWriter : public QObject
{
    Q_OBJECT

public:
    TestWriter();

private Q_SLOTS:
    void cleanup();
    void testWriteInvalidArgs();
    void testWriteFromStringData();
    void testWriteFromVariantData();
    void testWriteToFileWithDotsInName();
    void testWriteAppendMode();
    void testWriteWithNotDefaultSeparator();
    void testWriteWithHeader();
    void testWriteWithFooter();
    void testWriteWithHeaderAndFooter();
    void testWriterDataContainSeparators();
    void testWriteDifferentDataAmount();
    void testWriteDataContainCRLF();

private:
    QString getFilePath() const;
    QString getFilePathXLS() const;
    QString getFilePathWithDotsInName() const;
    QtCSV::StringData getTestStringData(const int &symbolsInRow,
                                        const int &rowsNumber);
};

#endif // TESTWRITER_H
