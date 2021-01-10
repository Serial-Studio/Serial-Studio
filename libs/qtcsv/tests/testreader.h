#ifndef TESTREADER_H
#define TESTREADER_H

#include <QObject>
#include <QtTest>

class TestReader : public QObject
{
    Q_OBJECT

public:
    TestReader();

private Q_SLOTS:
    void testReadToListInvalidArgs();
    void testReadToDataInvalidArgs();
    void testReadFileWithCommas();
    void testReadFileWithDotsInName();
    void testReadFileWithCommasToStringData();
    void testReadFileWithCommasToVariantData();
    void testReadFileWithSemicolons();
    void testReadFileWithSemicolonsToStringData();
    void testReadFileWithSemicolonsToVariantData();
    void testReadFileWithTextDelimDQoutes();
    void testReadFileWithTextDelimQoutes();
    void testReadFileWithTextDelimDQToStringData();
    void testReadLongWithDQuotes();
    void testReadFieldWithCR();
    void testReadFieldWithCRLF();
    void testReadFieldWithCRLFLong();
    void testReadFieldEndTripleQuotes();
    void testReadFileDataCorrectness();
    void testReadFileWorldCitiesPop();
    void testReadFileWithEmptyFields();
    void testReadFileWithEmptyFieldsComplexSeparator();
    void testReadFileWithMultirowData();
    void testReadByProcessorWithBreak();

private:
    QString getPathToFolderWithTestFiles() const;
    QString getPathToFileTestComma() const;
    QString getPathToFileTestDotsInName() const;
    QString getPathToFileTestSemicolon() const;
    QString getPathToFileTestDataTextDelimDQuotes() const;
    QString getPathToFileTestDataTextDelimQuotes() const;
    QString getPathToFileTestFieldWithDQuotes() const;
    QString getPathToFileTestFieldWithCR() const;
    QString getPathToFileTestFieldWithCRLF() const;
    QString getPathToFileTestFieldWithCRLFLong() const;
    QString getPathToFileTestFieldEndTripleQuotes() const;
    QString getPathToFileTestDataCorrectness() const;
    QString getPathToFileWorldCitiesPop() const;
    QString getPathToFileWithEmptyFields() const;
    QString getPathToFileWithEmptyFieldsComplexSeparator() const;
    QString getPathToFileMultirowData() const;
};

#endif // TESTREADER_H
