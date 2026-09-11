/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QString>
#include <QTest>
#include <QUrl>

#include "AI/Conversation/HelpFetcher.h"

// Every test function here is self-contained: the fetcher's URL policy is a set of static
// functions, so no fetcher is constructed and no request ever leaves the process. The live 404
// round trip (page miss, then help.json at the same ref) is not covered here because the fetcher
// owns a real QNetworkAccessManager; the two URL builders it consumes are what these cases pin,
// and spec 0078 AC3 covers the wire on a tagged build.

/**
 * @brief Pins the help-fetch URL policy of spec 0078 (AC6): a bare page name resolves against
 *        the commit the binary was built from, the help.json fallback lands at that same ref,
 *        an unstamped build falls back to the development branch, a full URL passes through
 *        untouched, and the host allowlist is unchanged by any of it.
 */
class TstHelpFetcher : public QObject {
  Q_OBJECT

private slots:
  void bareNameResolvesAgainstCommitRef();
  void indexUrlUsesTheSameRef();
  void emptyRefFallsBackToMaster();
  void fullUrlPassesThroughUnchanged();
  void allowlistIsUnchanged();
};

//--------------------------------------------------------------------------------------------------
// Ref pinning
//--------------------------------------------------------------------------------------------------

/**
 * @brief A bare page name becomes a raw-GitHub doc/help URL at the given ref, with the
 *        leading slash dropped and the .md extension appended exactly once.
 */
void TstHelpFetcher::bareNameResolvesAgainstCommitRef()
{
  const auto ref = QStringLiteral("0123456789abcdef0123456789abcdef01234567");
  const auto url = AI::HelpFetcher::pageUrl(QStringLiteral("API-Reference"), ref);
  QCOMPARE(url.toString(),
           QStringLiteral("https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/"
                          "0123456789abcdef0123456789abcdef01234567/doc/help/API-Reference.md"));

  const auto slashed = AI::HelpFetcher::pageUrl(QStringLiteral("/FAQ.md"), ref);
  QVERIFY(
    slashed.toString().endsWith(QStringLiteral("/") + ref + QStringLiteral("/doc/help/FAQ.md")));

  const auto home = AI::HelpFetcher::pageUrl(QString(), ref);
  QVERIFY(
    home.toString().endsWith(QStringLiteral("/") + ref + QStringLiteral("/doc/help/Home.md")));
}

/**
 * @brief The 404 fallback reads help.json from the same ref the missed page was requested at,
 *        never from the development branch.
 */
void TstHelpFetcher::indexUrlUsesTheSameRef()
{
  const auto ref  = QStringLiteral("feedfacefeedfacefeedfacefeedfacefeedface");
  const auto page = AI::HelpFetcher::pageUrl(QStringLiteral("Nope"), ref);
  const auto idx  = AI::HelpFetcher::indexUrl(ref);

  QCOMPARE(idx.toString(),
           QStringLiteral("https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/"
                          "feedfacefeedfacefeedfacefeedfacefeedface/doc/help/help.json"));
  QCOMPARE(idx.adjusted(QUrl::RemoveFilename), page.adjusted(QUrl::RemoveFilename));
  QVERIFY(!idx.toString().contains(QStringLiteral("/master/")));
}

/**
 * @brief Without a stamped commit the ref is the development branch, so a local build keeps
 *        reading current documentation; buildRef() never returns an empty ref.
 */
void TstHelpFetcher::emptyRefFallsBackToMaster()
{
  QCOMPARE(AI::HelpFetcher::buildRef(QString()), QStringLiteral("master"));
  const auto sha = QStringLiteral("0123456789abcdef0123456789abcdef01234567");
  QCOMPARE(AI::HelpFetcher::buildRef(sha), sha);

  const auto ref = AI::HelpFetcher::buildRef();
  QVERIFY(!ref.isEmpty());
  QVERIFY(ref == QStringLiteral("master") || ref.size() == 40);

  const auto url = AI::HelpFetcher::pageUrl(QStringLiteral("Getting-Started"));
  QVERIFY(url.toString().contains(QStringLiteral("/") + ref + QStringLiteral("/doc/help/")));
}

//--------------------------------------------------------------------------------------------------
// Passthrough and allowlist
//--------------------------------------------------------------------------------------------------

/**
 * @brief A full http(s) URL is returned as given: the ref never rewrites a URL the user or
 *        the model spelled out.
 */
void TstHelpFetcher::fullUrlPassesThroughUnchanged()
{
  const auto given = QStringLiteral("https://github.com/Serial-Studio/Serial-Studio/blob/master/"
                                    "doc/help/FAQ.md");
  const auto ref   = QStringLiteral("0123456789abcdef0123456789abcdef01234567");
  QCOMPARE(AI::HelpFetcher::pageUrl(given, ref).toString(), given);
  QCOMPARE(AI::HelpFetcher::pageUrl(given).toString(), given);
}

/**
 * @brief The exactly-anchored https allowlist is unchanged by the ref pin: the three hosts and
 *        their subdomains pass, lookalikes, plain http and userinfo are refused.
 */
void TstHelpFetcher::allowlistIsUnchanged()
{
  QVERIFY(AI::HelpFetcher::urlAllowed(QUrl(QStringLiteral("https://github.com/x"))));
  QVERIFY(AI::HelpFetcher::urlAllowed(QUrl(QStringLiteral("https://raw.githubusercontent.com/x"))));
  QVERIFY(AI::HelpFetcher::urlAllowed(QUrl(QStringLiteral("https://serial-studio.com/x"))));
  QVERIFY(AI::HelpFetcher::urlAllowed(AI::HelpFetcher::indexUrl(QStringLiteral("master"))));

  QVERIFY(!AI::HelpFetcher::urlAllowed(QUrl(QStringLiteral("https://evilgithub.com/x"))));
  QVERIFY(!AI::HelpFetcher::urlAllowed(QUrl(QStringLiteral("http://github.com/x"))));
  QVERIFY(!AI::HelpFetcher::urlAllowed(QUrl(QStringLiteral("https://user@github.com/x"))));
}

QTEST_GUILESS_MAIN(TstHelpFetcher)
#include "tst_help_fetcher.moc"
