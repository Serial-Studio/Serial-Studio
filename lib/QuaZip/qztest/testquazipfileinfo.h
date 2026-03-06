#ifndef TESTQUAZIPFILEINFO_H
#define TESTQUAZIPFILEINFO_H

#include <QtCore/QObject>

class TestQuaZipFileInfo : public QObject
{
using QObject::QObject;
    Q_OBJECT
private slots:
    void getNTFSTime_data();
    void getNTFSTime();
    void getExtTime_data();
    void getExtTime();
    void getExtTime_issue43();
    void parseExtraField_data();
    void parseExtraField();
};

#endif // TESTQUAZIPFILEINFO_H
