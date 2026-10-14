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

#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QTest>

#include "API/LaunchConsent.h"
#include "Core/Prompt/IUserPrompter.h"
#include "Core/Prompt/UserPrompt.h"

/**
 * @brief A prompter that answers with a scripted button and counts what it was asked.
 */
class LaunchPrompter : public Core::Prompt::IUserPrompter {
public:
  int promptMessage(const QString& text,
                    const QString& informativeText,
                    Core::Prompt::Icon icon,
                    const QString& windowTitle,
                    Core::Prompt::Buttons buttons,
                    Core::Prompt::Button defaultButton,
                    const Core::Prompt::ButtonLabels& buttonLabels) override
  {
    Q_UNUSED(text);
    Q_UNUSED(informativeText);
    Q_UNUSED(icon);
    Q_UNUSED(windowTitle);
    Q_UNUSED(buttons);
    Q_UNUSED(defaultButton);
    Q_UNUSED(buttonLabels);
    ++prompts;
    return answer;
  }

  void promptDirectory(const QString& title,
                       const QString& initialPath,
                       DirectoryHandler onSelected) override
  {
    Q_UNUSED(title);
    Q_UNUSED(initialPath);
    Q_UNUSED(onSelected);
  }

  void revealInFileManager(const QString& path) override { Q_UNUSED(path); }

  int prompts = 0;
  int answer  = Core::Prompt::No;
};

/**
 * @brief The per-project launch consent (spec 0098 R8, R13, AC11): a remote change clears it once
 *        per project per session, a re-grant then holds, a save-as onto a trusted file cannot
 *        inherit its grant, the headless operator's project starts granted, and AUTO_CONSENT is
 *        never cleared.
 */
class TstLaunchConsent : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void cleanupTestCase();

  void remoteChangeClearsOnceThenHolds();
  void saveAsOntoATrustedFileAsksAgain();
  void operatorProjectStartsGranted();
  void autoConsentIsNeverCleared();
  void keyFormatIsUnchanged();

private:
  LaunchPrompter m_prompter;
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

void TstLaunchConsent::initTestCase()
{
  QStandardPaths::setTestModeEnabled(true);
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_launch_consent"));
  Core::Prompt::setPrompter(&m_prompter);
  qunsetenv("SERIAL_STUDIO_API_AUTO_CONSENT");
}

void TstLaunchConsent::init()
{
  m_prompter.prompts = 0;
  m_prompter.answer  = Core::Prompt::No;
  QSettings settings;
  settings.clear();
}

void TstLaunchConsent::cleanupTestCase()
{
  Core::Prompt::setPrompter(nullptr);
  QSettings settings;
  settings.clear();
}

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief AC11: a trusted project's grant is cleared by the first remote change, the next launch
 *        asks, and after the user says yes again a second remote change leaves it granted.
 */
void TstLaunchConsent::remoteChangeClearsOnceThenHolds()
{
  QSettings settings;
  const auto path = QStringLiteral("/projects/rig.ssproj");
  settings.setValue(API::LaunchConsent::keyFor(path), true);

  API::LaunchConsent consent(settings);
  consent.bindProject(path);
  QVERIFY(consent.isGranted());

  consent.noteRemoteChange();
  QVERIFY(!consent.isGranted());
  QVERIFY(!settings.contains(API::LaunchConsent::keyFor(path)));
  QCOMPARE(consent.authorize(QStringLiteral("run")), API::ConsentVerdict::ConsentRequired);

  m_prompter.answer = Core::Prompt::Yes;
  QCoreApplication::processEvents();
  QCOMPARE(m_prompter.prompts, 1);
  QCOMPARE(consent.authorize(QStringLiteral("run")), API::ConsentVerdict::Allowed);

  consent.noteRemoteChange();
  QVERIFY(consent.isGranted());
  QCOMPARE(consent.authorize(QStringLiteral("run")), API::ConsentVerdict::Allowed);
}

/**
 * @brief After a remote change, binding another already-trusted file clears that file's grant
 *        once, so "load a hostile document, then save it over a trusted file" asks again.
 */
void TstLaunchConsent::saveAsOntoATrustedFileAsksAgain()
{
  QSettings settings;
  const auto first   = QStringLiteral("/projects/a.ssproj");
  const auto trusted = QStringLiteral("/projects/trusted.ssproj");
  settings.setValue(API::LaunchConsent::keyFor(trusted), true);

  API::LaunchConsent consent(settings);
  consent.bindProject(first);
  consent.noteRemoteChange();

  consent.bindProject(trusted);
  QVERIFY(!consent.isGranted());
  QVERIFY(!settings.contains(API::LaunchConsent::keyFor(trusted)));
}

/**
 * @brief The operator's command-line project starts granted in memory, nothing persists, and a
 *        remote change still clears it.
 */
void TstLaunchConsent::operatorProjectStartsGranted()
{
  QSettings settings;
  const auto path = QStringLiteral("/srv/plant/line.ssproj");

  API::LaunchConsent consent(settings);
  consent.setOperatorConsentedPath(path);
  QVERIFY(!consent.isGranted());

  consent.bindProject(path);
  QVERIFY(consent.isGranted());
  QVERIFY(!settings.contains(API::LaunchConsent::keyFor(path)));

  consent.bindProject(QStringLiteral("/srv/plant/other.ssproj"));
  QVERIFY(!consent.isGranted());

  consent.bindProject(path);
  QVERIFY(consent.isGranted());
  consent.noteRemoteChange();
  QVERIFY(!consent.isGranted());
}

/**
 * @brief The headless override grants for good: no rebind or remote change clears it.
 */
void TstLaunchConsent::autoConsentIsNeverCleared()
{
  qputenv("SERIAL_STUDIO_API_AUTO_CONSENT", QByteArrayLiteral("1"));
  QSettings settings;
  API::LaunchConsent consent(settings);
  qunsetenv("SERIAL_STUDIO_API_AUTO_CONSENT");

  QVERIFY(consent.isGranted());
  consent.bindProject(QStringLiteral("/projects/x.ssproj"));
  consent.noteRemoteChange();
  QVERIFY(consent.isGranted());
  QCOMPARE(consent.authorize(QStringLiteral("run")), API::ConsentVerdict::Allowed);
  QCOMPARE(m_prompter.prompts, 0);
  QVERIFY(settings.allKeys().isEmpty());
}

/**
 * @brief Existing grants keep applying: the key is the pre-0098 one, and an untitled project has
 *        none (session-only).
 */
void TstLaunchConsent::keyFormatIsUnchanged()
{
  QVERIFY(API::LaunchConsent::keyFor(QString()).isEmpty());
  const auto key = API::LaunchConsent::keyFor(QStringLiteral("/a/b/../c.ssproj"));
  QVERIFY(key.startsWith(QStringLiteral("API/ScriptExecConsent/")));
  QCOMPARE(key, API::LaunchConsent::keyFor(QStringLiteral("/a/c.ssproj")));
}

QTEST_GUILESS_MAIN(TstLaunchConsent)

#include "tst_launch_consent.moc"
