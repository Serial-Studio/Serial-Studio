#include <qmqtt_client.h>
#include <qmqtt_client_p.h>
#include <qmqtt_router.h>
// #include <qmqtt_routesubscription.h>
#include <QTest>
#include <QScopedPointer>

class RouterTests : public QObject
{
  Q_OBJECT
public:
  explicit RouterTests();
  virtual ~RouterTests();

  QScopedPointer<QMQTT::Client> _client;
  QScopedPointer<QMQTT::Router> _uut;

private slots:
  void init();
  void cleanup();

  void subscribe_Test();
};

RouterTests::RouterTests()
  : _uut(nullptr)
{
}

RouterTests::~RouterTests() {}

void RouterTests::init()
{
  //    _client.reset(new QMQTT::Client);
  //    _uut.reset(new QMQTT::Router(_client.data()));
}

void RouterTests::cleanup()
{
  //    _uut.reset();
  //    _client.reset();
}

void RouterTests::subscribe_Test()
{
  //    QMQTT::RouteSubscription* subscription = _uut->subscribe("route");
  //    QVERIFY(nullptr != subscription);
  //    QCOMPARE(subscription->route(), QString("route"));
}

// todo: need to figure out how to test subscribe a little better

QTEST_MAIN(RouterTests);
#include "routertests.moc"
