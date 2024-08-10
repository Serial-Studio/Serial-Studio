#ifndef IODEVICEMOCK_H
#define IODEVICEMOCK_H

#include <QIODevice>
#include <gmock/gmock.h>

class IODeviceMock : public QIODevice
{
public:
  MOCK_CONST_METHOD0(atEnd, bool());
  MOCK_METHOD1(waitForBytesWritten, bool(int));

  MOCK_METHOD2(readData, qint64(char *, qint64));
  MOCK_METHOD2(writeData, qint64(const char *, qint64));
  MOCK_CONST_METHOD0(openMode, QIODevice::OpenMode());
  MOCK_CONST_METHOD0(bytesAvailable, qint64());

  MOCK_METHOD2(write, qint64(const char *, qint64));
  MOCK_METHOD1(write, qint64(const char *));
  MOCK_METHOD1(write, qint64(const QByteArray &));
};

#endif // IODEVICEMOCK_H
