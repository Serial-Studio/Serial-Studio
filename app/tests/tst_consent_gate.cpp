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
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include "API/Server/ConsentGate.h"
#include "Core/Prompt/UserPrompt.h"

/**
 * @brief A prompter that answers with a scripted button and records what it was asked, so the
 *        gate's queued prompt can be observed without a dialog.
 */
class ScriptedPrompter : public Core::Prompt::IUserPrompter {
public:
  int promptMessage(const QString& text,
                    const QString& informativeText,
                    Core::Prompt::Icon icon,
                    const QString& windowTitle,
                    Core::Prompt::Buttons buttons,
                    Core::Prompt::Button defaultButton,
                    const Core::Prompt::ButtonLabels& buttonLabels) override
  {
    Q_UNUSED(icon);
    Q_UNUSED(windowTitle);
    Q_UNUSED(buttons);
    Q_UNUSED(defaultButton);
    Q_UNUSED(buttonLabels);
    ++prompts;
    lastText        = text;
    lastInformative = informativeText;
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
  QString lastText;
  QString lastInformative;
};

/**
 * @brief The consent gate behind every API prompt: non-blocking refusal, one prompt per
 *        decision, persistence only for a "yes" under a bound key, rebind and the headless grant.
 */
class TstConsentGate : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void cleanupTestCase();

  void unansweredRefusesAndPromptsOnce();
  void noIsRememberedForTheSessionOnly();
  void yesPersistsUnderTheKey();
  void yesOnAnEmptyKeyIsSessionOnly();
  void grantIsInMemoryAndSignals();
  void rebindForgetsTheDecision();
  void persistedYesRestoresOnConstruction();

private:
  [[nodiscard]] static QString freshKey();

  ScriptedPrompter m_prompter;
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

void TstConsentGate::initTestCase()
{
  QStandardPaths::setTestModeEnabled(true);
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_consent_gate"));
  Core::Prompt::setPrompter(&m_prompter);
}

void TstConsentGate::init()
{
  m_prompter.prompts = 0;
  m_prompter.answer  = Core::Prompt::No;
  m_prompter.lastText.clear();
  m_prompter.lastInformative.clear();

  QSettings settings;
  settings.clear();
}

void TstConsentGate::cleanupTestCase()
{
  Core::Prompt::setPrompter(nullptr);
  QSettings settings;
  settings.clear();
}

/**
 * @brief A key no other case wrote.
 */
QString TstConsentGate::freshKey()
{
  static int counter = 0;
  return QStringLiteral("Test/Consent/%1").arg(++counter);
}

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief The first ask refuses with ConsentRequired and posts exactly one prompt, even when asked
 *        again before the event loop ran; the detail of the first request reaches the prompt.
 */
void TstConsentGate::unansweredRefusesAndPromptsOnce()
{
  QSettings settings;
  API::ConsentGate gate(settings, freshKey(), QStringLiteral("Allow?"), QStringLiteral("Why."));

  QCOMPARE(gate.authorize(QStringLiteral("first")), API::ConsentVerdict::ConsentRequired);
  QCOMPARE(gate.authorize(QStringLiteral("second")), API::ConsentVerdict::ConsentRequired);
  QCOMPARE(m_prompter.prompts, 0);

  QCoreApplication::processEvents();
  QCOMPARE(m_prompter.prompts, 1);
  QCOMPARE(m_prompter.lastText, QStringLiteral("Allow?"));
  QVERIFY(m_prompter.lastInformative.startsWith(QStringLiteral("Why.")));
  QVERIFY(m_prompter.lastInformative.contains(QStringLiteral("first")));
  QVERIFY(!m_prompter.lastInformative.contains(QStringLiteral("second")));
}

/**
 * @brief A "no" denies every later ask without prompting again, and is never written to settings.
 */
void TstConsentGate::noIsRememberedForTheSessionOnly()
{
  QSettings settings;
  const auto key = freshKey();
  API::ConsentGate gate(settings, key, QStringLiteral("Allow?"), QStringLiteral("Why."));

  QCOMPARE(gate.authorize(), API::ConsentVerdict::ConsentRequired);
  QCoreApplication::processEvents();
  QCOMPARE(gate.authorize(), API::ConsentVerdict::Denied);
  QCOMPARE(gate.authorize(), API::ConsentVerdict::Denied);
  QCOMPARE(m_prompter.prompts, 1);
  QVERIFY(!settings.contains(key));
  QVERIFY(!gate.isGranted());
}

/**
 * @brief A "yes" allows, signals, and persists under the bound key.
 */
void TstConsentGate::yesPersistsUnderTheKey()
{
  QSettings settings;
  const auto key = freshKey();
  API::ConsentGate gate(settings, key, QStringLiteral("Allow?"), QStringLiteral("Why."));
  QSignalSpy granted(&gate, &API::ConsentGate::granted);

  m_prompter.answer = Core::Prompt::Yes;
  QCOMPARE(gate.authorize(), API::ConsentVerdict::ConsentRequired);
  QCoreApplication::processEvents();

  QCOMPARE(gate.authorize(), API::ConsentVerdict::Allowed);
  QCOMPARE(granted.count(), 1);
  QVERIFY(gate.isGranted());
  QCOMPARE(settings.value(key, false).toBool(), true);
}

/**
 * @brief Without a key the "yes" holds for the session and nothing is written.
 */
void TstConsentGate::yesOnAnEmptyKeyIsSessionOnly()
{
  QSettings settings;
  API::ConsentGate gate(settings, QString(), QStringLiteral("Allow?"), QStringLiteral("Why."));

  m_prompter.answer = Core::Prompt::Yes;
  QCOMPARE(gate.authorize(), API::ConsentVerdict::ConsentRequired);
  QCoreApplication::processEvents();

  QCOMPARE(gate.authorize(), API::ConsentVerdict::Allowed);
  QVERIFY(gate.settingsKey().isEmpty());
  QVERIFY(settings.allKeys().isEmpty());
}

/**
 * @brief The headless grant allows at once, fires the signal once, and never touches settings.
 */
void TstConsentGate::grantIsInMemoryAndSignals()
{
  QSettings settings;
  const auto key = freshKey();
  API::ConsentGate gate(settings, key, QStringLiteral("Allow?"), QStringLiteral("Why."));
  QSignalSpy granted(&gate, &API::ConsentGate::granted);

  gate.grant();
  gate.grant();
  QCOMPARE(granted.count(), 1);
  QCOMPARE(gate.authorize(), API::ConsentVerdict::Allowed);
  QCOMPARE(m_prompter.prompts, 0);
  QVERIFY(!settings.contains(key));
}

/**
 * @brief Rebinding to another key forgets the answer given under the first, so a "yes" for one
 *        project never carries to the next.
 */
void TstConsentGate::rebindForgetsTheDecision()
{
  QSettings settings;
  const auto first  = freshKey();
  const auto second = freshKey();
  API::ConsentGate gate(settings, first, QStringLiteral("Allow?"), QStringLiteral("Why."));

  m_prompter.answer = Core::Prompt::Yes;
  QCOMPARE(gate.authorize(), API::ConsentVerdict::ConsentRequired);
  QCoreApplication::processEvents();
  QCOMPARE(gate.authorize(), API::ConsentVerdict::Allowed);

  gate.rebind(second);
  QCOMPARE(gate.settingsKey(), second);
  QVERIFY(!gate.isGranted());
  QCOMPARE(gate.authorize(), API::ConsentVerdict::ConsentRequired);

  gate.rebind(first);
  QCOMPARE(gate.authorize(), API::ConsentVerdict::Allowed);
}

/**
 * @brief A persisted "yes" is restored by the constructor without a prompt.
 */
void TstConsentGate::persistedYesRestoresOnConstruction()
{
  QSettings settings;
  const auto key = freshKey();
  settings.setValue(key, true);

  API::ConsentGate gate(settings, key, QStringLiteral("Allow?"), QStringLiteral("Why."));
  QVERIFY(gate.isGranted());
  QCOMPARE(gate.authorize(), API::ConsentVerdict::Allowed);
  QCOMPARE(m_prompter.prompts, 0);
}

QTEST_GUILESS_MAIN(TstConsentGate)

#include "tst_consent_gate.moc"
