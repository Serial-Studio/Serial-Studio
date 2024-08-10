#include <qmqtt_routesubscription.h>
#include <QTest>
#include <QScopedPointer>

class RouteSubscriptionTests : public QObject
{
  Q_OBJECT
public:
  explicit RouteSubscriptionTests();
  virtual ~RouteSubscriptionTests();

  QScopedPointer<QMQTT::RouteSubscription> _uut;

private slots:
  void init();
  void cleanup();

  // todo: can't really test directly here, only Route class can insantiate for
  // now
};

RouteSubscriptionTests::RouteSubscriptionTests()
  : _uut(nullptr)
{
}

RouteSubscriptionTests::~RouteSubscriptionTests() {}

void RouteSubscriptionTests::init()
{
  _uut.reset();
}

void RouteSubscriptionTests::cleanup()
{
  _uut.reset();
}

QTEST_MAIN(RouteSubscriptionTests);
#include "routesubscriptiontests.moc"
