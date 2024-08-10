#include <qmqtt_frame.h>
#include <QScopedPointer>
#include <QDataStream>
#include <gtest/gtest.h>

using namespace testing;

class FrameTest : public Test
{
public:
  explicit FrameTest()
    : _frame(new QMQTT::Frame)
  {
  }
  virtual ~FrameTest() {}

  QScopedPointer<QMQTT::Frame> _frame;
};

TEST_F(FrameTest, DISABLED_defaultConstructor_Test)
{
  // Disabled test, getters segfault"
  // todo: these segfault currently. QByteArray contains no data when default
  // constructed.
  EXPECT_EQ(0, _frame->readInt());
  EXPECT_EQ(12, _frame->readChar());
  EXPECT_EQ(QString("blah"), _frame->readString());
}

TEST_F(FrameTest, constructorWithParameters_Test)
{
  QByteArray byteArray;
  _frame.reset(new QMQTT::Frame(0, byteArray));

  EXPECT_FALSE(_frame.isNull());
}

TEST_F(FrameTest, readIntReadsBytearray_Test)
{
  QByteArray byteArray;
  byteArray.resize(2);
  byteArray[0] = 1;
  byteArray[1] = 2;
  _frame.reset(new QMQTT::Frame(0, byteArray));

  EXPECT_EQ(static_cast<int>((1 << 8) + 2), _frame->readInt());
}

TEST_F(FrameTest, readCharReadsBytearray_Test)
{
  QByteArray byteArray;
  byteArray.resize(1);
  byteArray[0] = 42;
  _frame.reset(new QMQTT::Frame(0, byteArray));

  EXPECT_EQ(static_cast<char>(42), _frame->readChar());
}

TEST_F(FrameTest, readStringReadsBytearray_Test)
{
  QByteArray byteArray;
  byteArray.resize(5);
  byteArray[0] = 0;
  byteArray[1] = 3;
  byteArray[2] = 'a';
  byteArray[3] = 'b';
  byteArray[4] = 'c';
  _frame.reset(new QMQTT::Frame(0, byteArray));

  EXPECT_EQ(QString("abc"), _frame->readString());
}

TEST_F(FrameTest, writeIntWritesToBytearray_Test)
{
  _frame->writeInt(42);

  EXPECT_EQ(42, _frame->readInt());
}

TEST_F(FrameTest, writeCharWritesToBytearray_Test)
{
  _frame->writeChar(static_cast<char>(42));

  EXPECT_EQ(42, _frame->readChar());
}

TEST_F(FrameTest, writeStringWritesToBytearray_Test)
{
  _frame->writeString(QString("forty-two"));

  EXPECT_EQ(QString("forty-two"), _frame->readString());
}

TEST_F(FrameTest, writeStringWritesToBytearrayTooLong_Test)
{
  _frame->writeString(QString(70000, 'a'));

  QString s = _frame->readString();
  EXPECT_EQ(static_cast<int>(USHRT_MAX), s.size());
  EXPECT_EQ(QString(USHRT_MAX, 'a'), s);
}

TEST_F(FrameTest, writeRawDataWritesToBytearray_Test)
{
  QByteArray byteArray;
  byteArray.resize(2);
  byteArray[0] = 3;
  byteArray[1] = 4;
  _frame->writeRawData(byteArray);

  EXPECT_EQ(static_cast<int>((3 << 8) + 4), _frame->readInt());
}

TEST_F(FrameTest, writeWritesToDatastream_Test)
{
  QByteArray frameData(128, 0);
  for (unsigned char c = 0; c < 128; c++)
  {
    frameData[c] = c;
  }
  _frame.reset(new QMQTT::Frame(0, frameData));

  QByteArray streamedData;
  QDataStream stream(&streamedData, QIODevice::WriteOnly);
  _frame->write(stream);

  EXPECT_EQ(static_cast<quint8>(0), static_cast<quint8>(streamedData.at(0)));
  EXPECT_EQ(static_cast<quint8>(0x80), static_cast<quint8>(streamedData.at(1)));
  EXPECT_EQ(static_cast<quint8>(0x01), static_cast<quint8>(streamedData.at(2)));
  EXPECT_EQ(frameData, streamedData.mid(3));
}

TEST_F(FrameTest, encodeLength_Test)
{
  QByteArray lenbuf;
  EXPECT_FALSE(
      _frame->encodeLength(lenbuf, 268435456)); // bigger than 268,435,455
  EXPECT_TRUE(_frame->encodeLength(lenbuf, 268435455)
              && lenbuf == QByteArray::fromHex("FFFFFF7F"));
  EXPECT_TRUE(_frame->encodeLength(lenbuf, 2097151)
              && lenbuf == QByteArray::fromHex("FFFF7F"));
  EXPECT_TRUE(_frame->encodeLength(lenbuf, 16383)
              && lenbuf == QByteArray::fromHex("FF7F"));
  EXPECT_TRUE(_frame->encodeLength(lenbuf, 127)
              && lenbuf == QByteArray::fromHex("7F"));
  EXPECT_TRUE(_frame->encodeLength(lenbuf, 1)
              && lenbuf == QByteArray::fromHex("1"));
  EXPECT_TRUE(_frame->encodeLength(lenbuf, 0)
              && lenbuf == QByteArray::fromHex("0"));
}
