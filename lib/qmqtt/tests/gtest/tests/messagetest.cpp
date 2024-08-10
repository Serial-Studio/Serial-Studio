#include <qmqtt_message.h>
#include <QScopedPointer>
#include <QString>
#include <gtest/gtest.h>

using namespace testing;

class MessageTest : public Test
{
public:
  explicit MessageTest()
    : _message(new QMQTT::Message)
  {
  }
  virtual ~MessageTest() {}

  QScopedPointer<QMQTT::Message> _message;
};

TEST_F(MessageTest, defaultConstructor_Test)
{
  EXPECT_EQ(0, _message->id());
  EXPECT_EQ(0, _message->qos());
  EXPECT_FALSE(_message->retain());
  EXPECT_FALSE(_message->dup());
  EXPECT_EQ(QString(), _message->topic());
  EXPECT_EQ(QByteArray(), _message->payload());
}

TEST_F(MessageTest, constructorWithParametersHasDefaultValues_Test)
{
  _message.reset(new QMQTT::Message(5, "topic", "payload"));
  EXPECT_EQ(0, _message->qos());
  EXPECT_FALSE(_message->retain());
  EXPECT_FALSE(_message->dup());
}

TEST_F(MessageTest, constructorWithParameters_Test)
{
  _message.reset(new QMQTT::Message(5, "topic", "payload", 2, true, true));
  EXPECT_EQ(5, _message->id());
  EXPECT_EQ(2, _message->qos());
  EXPECT_TRUE(_message->retain());
  EXPECT_TRUE(_message->dup());
  EXPECT_EQ(QString("topic"), _message->topic());
  EXPECT_EQ(QByteArray("payload"), _message->payload());
}

TEST_F(MessageTest, copyConstructor_Test)
{
  QMQTT::Message originalMessage(5, "topic", "payload", 1, true, true);
  QMQTT::Message copy(originalMessage);
  _message.reset(new QMQTT::Message(5, "topic", "payload", 1, true, true));
  EXPECT_EQ(5, copy.id());
  EXPECT_EQ(1, copy.qos());
  EXPECT_TRUE(copy.retain());
  EXPECT_TRUE(copy.dup());
  EXPECT_EQ(QString("topic"), copy.topic());
  EXPECT_EQ(QByteArray("payload"), copy.payload());
}

TEST_F(MessageTest, assignmentOperator_Test)
{
  QMQTT::Message originalMessage(5, "topic", "payload", 2, true, true);
  QMQTT::Message copy;

  copy = originalMessage;

  EXPECT_EQ(5, copy.id());
  EXPECT_EQ(2, copy.qos());
  EXPECT_TRUE(copy.retain());
  EXPECT_TRUE(copy.dup());
  EXPECT_EQ(QString("topic"), copy.topic());
  EXPECT_EQ(QByteArray("payload"), copy.payload());
}

TEST_F(MessageTest, idSettable_Test)
{
  _message->setId(5);
  EXPECT_EQ(5, _message->id());
}

TEST_F(MessageTest, qosSettable_Test)
{
  _message->setQos(1);
  EXPECT_EQ(1, _message->qos());
}

TEST_F(MessageTest, retainSettable_Test)
{
  _message->setRetain(true);
  EXPECT_TRUE(_message->retain());
}

TEST_F(MessageTest, dupSettable_Test)
{
  _message->setDup(true);
  EXPECT_TRUE(_message->dup());
}

TEST_F(MessageTest, topicSettable_Test)
{
  _message->setTopic("topic");
  EXPECT_EQ(QString("topic"), _message->topic());
}

TEST_F(MessageTest, payloadSettable_Test)
{
  _message->setPayload("payload");
  EXPECT_EQ(QByteArray("payload"), _message->payload());
}
