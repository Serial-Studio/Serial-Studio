/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <memory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>

#include "support/FakeDriver.h"
#include "support/FakeProvider.h"
#include "support/FakeTransport.h"

/**
 * @brief Smoke tests for the shared test doubles (spec 0075 WP0-T21).
 *
 * Every suite that pins a connect verdict, an assistant turn or an extension download codes
 * against these three, so a double that quietly stops behaving is a whole tier of green tests
 * proving nothing. This file is the contract they all rely on.
 */
class TestDoubles : public QObject {
  Q_OBJECT

private slots:
  void driverReportsSyncVerdicts();
  void driverLatchesAsyncVerdictExactlyOnce();
  void driverDropsAnEstablishedLink();
  void providerReplaysItsScript();
  void providerBreachesTheStreamBudget();
  void transportAnswersFromItsQueue();
  void transportRefusesToReachTheNetwork();
};

/**
 * @brief A synchronous outcome answers inside open() and never latches isConnecting().
 */
void TestDoubles::driverReportsSyncVerdicts()
{
  Test::FakeDriver driver;
  QSignalSpy spy(&driver, &IO::HAL_Driver::openFinished);

  driver.armOpenReport();
  driver.setOpenOutcome(Test::FakeDriver::Outcome::SyncOk);
  QVERIFY(driver.open(QIODevice::ReadWrite));
  QVERIFY(driver.isOpen());
  QVERIFY(!driver.isConnecting());
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).toBool(), true);

  driver.close();
  driver.armOpenReport();
  driver.setOpenOutcome(Test::FakeDriver::Outcome::SyncFail);
  QVERIFY(!driver.open(QIODevice::ReadWrite));
  QVERIFY(!driver.isOpen());
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).toBool(), false);
  QCOMPARE(driver.openCalls(), 2);
}

/**
 * @brief An async outcome returns true, reports isConnecting() until the verdict, and emits
 *        openFinished exactly once -- the shape that wedges the connect button when a real
 *        driver reports only success.
 */
void TestDoubles::driverLatchesAsyncVerdictExactlyOnce()
{
  const auto outcomes = {Test::FakeDriver::Outcome::AsyncOk, Test::FakeDriver::Outcome::AsyncFail};
  for (const auto outcome : outcomes) {
    Test::FakeDriver driver;
    QSignalSpy spy(&driver, &IO::HAL_Driver::openFinished);

    driver.armOpenReport();
    driver.setOpenOutcome(outcome);
    driver.setAsyncDelayMs(0);

    QVERIFY(driver.open(QIODevice::ReadWrite));
    QVERIFY(driver.isConnecting());
    QCOMPARE(spy.count(), 0);

    QVERIFY(spy.wait(1000));
    QCOMPARE(spy.count(), 1);
    QVERIFY(!driver.isConnecting());
    QCOMPARE(spy.takeFirst().at(0).toBool(), outcome == Test::FakeDriver::Outcome::AsyncOk);
  }
}

/**
 * @brief dropLink() clears isOpen() without a close() call, the way a yanked cable does.
 */
void TestDoubles::driverDropsAnEstablishedLink()
{
  Test::FakeDriver driver;
  driver.armOpenReport();
  QVERIFY(driver.open(QIODevice::ReadWrite));
  QVERIFY(driver.isOpen());

  QSignalSpy spy(&driver, &IO::HAL_Driver::configurationChanged);
  driver.dropLink();

  QVERIFY(!driver.isOpen());
  QCOMPARE(spy.count(), 1);
  QCOMPARE(driver.closeCalls(), 0);
}

/**
 * @brief The scripted events arrive in order, one per event-loop turn, and stop at Done.
 */
void TestDoubles::providerReplaysItsScript()
{
  Test::FakeProvider provider;
  provider.script({
    {    Test::ReplyEvent::Text,QStringLiteral("hello"),{},                                                 {}                                                                 },
    {Test::ReplyEvent::ToolCall,
     {},
     QStringLiteral("read_file"),
     QJsonObject{{QStringLiteral("path"), QStringLiteral("a.txt")}}                                                         },
    {    Test::ReplyEvent::Done,                      {},     {},                                                         {}},
  });

  std::unique_ptr<AI::Reply> reply(provider.sendMessage(QJsonArray{}, QJsonArray{}));
  QVERIFY(reply != nullptr);
  QCOMPARE(provider.sendCount(), 1);

  QSignalSpy text(reply.get(), &AI::Reply::partialText);
  QSignalSpy tools(reply.get(), &AI::Reply::toolCallRequested);
  QSignalSpy done(reply.get(), &AI::Reply::finished);

  QVERIFY(done.wait(1000));
  QCOMPARE(text.count(), 1);
  QCOMPARE(text.takeFirst().at(0).toString(), QStringLiteral("hello"));
  QCOMPARE(tools.count(), 1);
  QCOMPARE(tools.takeFirst().at(1).toString(), QStringLiteral("read_file"));
}

/**
 * @brief BudgetBreach reaches the streamed-byte cap without moving eight megabytes.
 */
void TestDoubles::providerBreachesTheStreamBudget()
{
  Test::FakeProvider provider;
  provider.script({
    {Test::ReplyEvent::BudgetBreach, {}, {}, {}}
  });

  std::unique_ptr<AI::Reply> reply(provider.sendMessage(QJsonArray{}, QJsonArray{}));
  QSignalSpy errors(reply.get(), &AI::Reply::errorOccurred);

  QVERIFY(errors.wait(1000));
  QCOMPARE(errors.count(), 1);
}

/**
 * @brief Queued answers are matched by URL substring, in order, and consumed on match.
 */
void TestDoubles::transportAnswersFromItsQueue()
{
  Test::FakeTransport transport;
  transport.enqueue(QStringLiteral("catalog.json"), 200, QByteArrayLiteral("{\"v\":2}"));
  QCOMPARE(transport.pendingResponses(), 1);

  std::unique_ptr<QNetworkReply> reply(
    transport.get(QNetworkRequest(QUrl(QStringLiteral("https://example.invalid/catalog.json")))));
  QSignalSpy done(reply.get(), &QNetworkReply::finished);

  QVERIFY(done.wait(1000));
  QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
  QCOMPARE(reply->readAll(), QByteArrayLiteral("{\"v\":2}"));
  QCOMPARE(transport.pendingResponses(), 0);
  QCOMPARE(transport.requestedUrls().size(), 1);
}

/**
 * @brief An unmatched request answers 404 instead of falling through to the internet.
 */
void TestDoubles::transportRefusesToReachTheNetwork()
{
  Test::FakeTransport transport;

  std::unique_ptr<QNetworkReply> reply(
    transport.get(QNetworkRequest(QUrl(QStringLiteral("https://example.invalid/missing")))));
  QSignalSpy done(reply.get(), &QNetworkReply::finished);

  QVERIFY(done.wait(1000));
  QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 404);
  QCOMPARE(reply->error(), QNetworkReply::ContentNotFoundError);
}

QTEST_GUILESS_MAIN(TestDoubles)

#include "tst_test_doubles.moc"
