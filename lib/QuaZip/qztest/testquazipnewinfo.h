#ifndef TESTQUAZIPNEWINFO_H
#define TESTQUAZIPNEWINFO_H

#include <QtCore/QObject>

class TestQuaZipNewInfo : public QObject
{
using QObject::QObject;
    Q_OBJECT
private slots:
    void setFileNTFSTimes();
};

#endif // TESTQUAZIPNEWINFO_H
