#include <qmqtt_routedmessage.h>
#include <QTest>
#include <QScopedPointer>

class RoutedMessageTests : public QObject
{
  Q_OBJECT
public:
  explicit RoutedMessageTests();
  virtual ~RoutedMessageTests();

  QScopedPointer<QMQTT::RoutedMessage> _uut;

private slots:
  void init();
  void cleanup();

  void constructor_Test();
};

RoutedMessageTests::RoutedMessageTests()
  : _uut(nullptr)
{
}

RoutedMessageTests::~RoutedMessageTests() {}

void RoutedMessageTests::init()
{
  _uut.reset(new QMQTT::RoutedMessage(QMQTT::Message()));
}

void RoutedMessageTests::cleanup()
{
  _uut.reset();
}

void RoutedMessageTests::constructor_Test()
{
  QCOMPARE(_uut->message(), QMQTT::Message());
  QHash<QString, QString> emptyHash;
  QCOMPARE(_uut->parameters(), emptyHash);
}

QTEST_MAIN(RoutedMessageTests);
#include "routedmessagetests.moc"
