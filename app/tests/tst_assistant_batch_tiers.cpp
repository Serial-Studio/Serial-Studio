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

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

#include "AI/CommandRegistry.h"

using AI::Safety;

/**
 * @brief A batch whose ops are the given command names, each with empty params.
 */
static QJsonObject batchOf(const QStringList& commands)
{
  QJsonArray ops;
  for (const auto& command : commands)
    ops.append(QJsonObject{
      {QStringLiteral("command"),       command},
      { QStringLiteral("params"), QJsonObject()}
    });

  return QJsonObject{
    {QStringLiteral("ops"), ops}
  };
}

/**
 * @brief The assistant's tier for a whole call (spec 0098 R9, AC5): a batch takes its strictest
 *        op's tier, so a blocked op, a device op with device control off and an always-confirm op
 *        each get exactly the treatment they get as a direct call, and batches never nest.
 */
class TstAssistantBatchTiers : public QObject {
  Q_OBJECT

private slots:
  void cleanup();

  void strictnessOrdersTheTiers();
  void blockedOpBlocksTheBatch();
  void deviceOpFollowsDeviceControl();
  void alwaysConfirmOpEscalatesTheBatch();
  void confirmOnlyBatchStaysConfirm();
  void nestedDispatcherIsBlocked();
  void bulkApplyOpsArePlanned();
  void forwardersPlanTheirCommands();
  void scriptSettersAlwaysConfirm();
  void argumentsEscalateAProgramChoice();
};

/**
 * @brief Device control is process-wide on the singleton; every case leaves it off.
 */
void TstAssistantBatchTiers::cleanup()
{
  AI::CommandRegistry::instance().setDeviceControlAllowed(false);
}

/**
 * @brief Safe < Confirm < AlwaysConfirm < Blocked, whatever the enum's wire values are.
 */
void TstAssistantBatchTiers::strictnessOrdersTheTiers()
{
  using R = AI::CommandRegistry;
  QVERIFY(R::strictness(Safety::Safe) < R::strictness(Safety::Confirm));
  QVERIFY(R::strictness(Safety::Confirm) < R::strictness(Safety::AlwaysConfirm));
  QVERIFY(R::strictness(Safety::AlwaysConfirm) < R::strictness(Safety::Blocked));
}

/**
 * @brief One blocked op makes the whole batch Blocked, the tier the op has called directly.
 */
void TstAssistantBatchTiers::blockedOpBlocksTheBatch()
{
  auto& registry = AI::CommandRegistry::instance();
  QCOMPARE(registry.safetyOf(QStringLiteral("licensing.deactivate")), Safety::Blocked);

  const auto batch =
    batchOf({QStringLiteral("project.dataset.update"), QStringLiteral("licensing.deactivate")});
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"), batch), Safety::Blocked);
}

/**
 * @brief A device op inside a batch is Blocked while device control is off and AlwaysConfirm
 *        once it is on, exactly as the same op called directly.
 */
void TstAssistantBatchTiers::deviceOpFollowsDeviceControl()
{
  auto& registry   = AI::CommandRegistry::instance();
  const auto write = QStringLiteral("io.writeData");
  const auto batch = batchOf({QStringLiteral("project.dataset.update"), write});

  registry.setDeviceControlAllowed(false);
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"), batch),
           registry.safetyOf(write));
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"), batch), Safety::Blocked);

  registry.setDeviceControlAllowed(true);
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"), batch),
           registry.safetyOf(write));
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"), batch), Safety::AlwaysConfirm);
}

/**
 * @brief An always-confirm op makes the batch always-confirm, so auto-approve cannot run it.
 */
void TstAssistantBatchTiers::alwaysConfirmOpEscalatesTheBatch()
{
  auto& registry  = AI::CommandRegistry::instance();
  const auto code = QStringLiteral("controlScript.setCode");
  QCOMPARE(registry.safetyOf(code), Safety::AlwaysConfirm);

  const auto batch = batchOf({QStringLiteral("project.dataset.update"), code});
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"), batch), Safety::AlwaysConfirm);
}

/**
 * @brief A batch of ordinary edits keeps the batch's own Confirm tier.
 */
void TstAssistantBatchTiers::confirmOnlyBatchStaysConfirm()
{
  auto& registry   = AI::CommandRegistry::instance();
  const auto batch = batchOf(
    {QStringLiteral("project.dataset.update"), QStringLiteral("project.dataset.setOptions")});
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"), batch), Safety::Confirm);
}

/**
 * @brief A batch inside a batch, or a bulk apply inside one, is Blocked: batches never nest.
 */
void TstAssistantBatchTiers::nestedDispatcherIsBlocked()
{
  auto& registry = AI::CommandRegistry::instance();
  QVERIFY(AI::CommandRegistry::isDispatcher(QStringLiteral("project.batch")));
  QVERIFY(AI::CommandRegistry::isDispatcher(QStringLiteral("assistant.project.bulkApply")));

  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"),
                                    batchOf({QStringLiteral("project.batch")})),
           Safety::Blocked);
  QCOMPARE(registry.effectiveSafety(QStringLiteral("project.batch"),
                                    batchOf({QStringLiteral("assistant.project.bulkApply")})),
           Safety::Blocked);
  QCOMPARE(registry.effectiveSafety(QStringLiteral("assistant.project.bulkApply"),
                                    batchOf({QStringLiteral("project.batch")})),
           Safety::Blocked);
}

/**
 * @brief A bulk apply's ops are planned exactly like a batch's.
 */
void TstAssistantBatchTiers::bulkApplyOpsArePlanned()
{
  const auto batch =
    batchOf({QStringLiteral("project.dataset.update"), QStringLiteral("project.group.update")});
  const auto planned =
    AI::CommandRegistry::plannedCommands(QStringLiteral("assistant.project.bulkApply"), batch);
  QCOMPARE(planned,
           QStringList(
             {QStringLiteral("project.dataset.update"), QStringLiteral("project.group.update")}));
}

/**
 * @brief The fixed forwarders plan the commands they run: a script apply its setter, a tile add
 *        its five-command fan-out; both stay within their own Confirm tier.
 */
void TstAssistantBatchTiers::forwardersPlanTheirCommands()
{
  auto& registry     = AI::CommandRegistry::instance();
  const auto painter = QJsonObject{
    {QStringLiteral("kind"), QStringLiteral("painter")}
  };
  QCOMPARE(AI::CommandRegistry::plannedCommands(QStringLiteral("assistant.script.apply"), painter),
           QStringList({QStringLiteral("project.painter.setCode")}));
  QCOMPARE(registry.effectiveSafety(QStringLiteral("assistant.script.apply"), painter),
           Safety::AlwaysConfirm);

  const auto tile =
    AI::CommandRegistry::plannedCommands(QStringLiteral("assistant.workspace.addTile"), {});
  QCOMPARE(tile.size(), 5);
  QCOMPARE(registry.effectiveSafety(QStringLiteral("assistant.workspace.addTile"), {}),
           Safety::Confirm);

  QVERIFY(
    AI::CommandRegistry::plannedCommands(QStringLiteral("project.dataset.update"), {}).isEmpty());
}

/**
 * @brief The script-installing setters are always-confirm for the assistant (spec 0098 review):
 *        installed code runs with full trust, so an auto-approve must not run one silently.
 */
void TstAssistantBatchTiers::scriptSettersAlwaysConfirm()
{
  auto& registry = AI::CommandRegistry::instance();
  for (const auto& setter : {QStringLiteral("project.frameParser.setCode"),
                             QStringLiteral("project.source.setFrameParserCode"),
                             QStringLiteral("project.dataset.setTransformCode"),
                             QStringLiteral("project.transformLibrary.set"),
                             QStringLiteral("project.painter.setCode")})
    QCOMPARE(registry.safetyOf(setter), Safety::AlwaysConfirm);
}

/**
 * @brief A generic source setter that chooses a program is escalated by its arguments to the
 *        device-gated treatment, the same as the dedicated io.process.* setter (spec 0098 review):
 *        Blocked while device control is off, AlwaysConfirm once it is on. A non-program key stays
 *        at the command's own Confirm tier.
 */
void TstAssistantBatchTiers::argumentsEscalateAProgramChoice()
{
  auto& registry        = AI::CommandRegistry::instance();
  const auto executable = QJsonObject{
    {QStringLiteral("key"), QStringLiteral("executable")}
  };
  const auto baudRate = QJsonObject{
    {QStringLiteral("key"), QStringLiteral("baudRate")}
  };

  registry.setDeviceControlAllowed(false);
  QCOMPARE(registry.argumentAwareSafety(QStringLiteral("project.source.setProperty"), executable),
           Safety::Blocked);
  QCOMPARE(registry.argumentAwareSafety(QStringLiteral("project.source.setProperty"), baudRate),
           Safety::Confirm);

  registry.setDeviceControlAllowed(true);
  QCOMPARE(registry.argumentAwareSafety(QStringLiteral("project.source.setProperty"), executable),
           Safety::AlwaysConfirm);
  QCOMPARE(registry.argumentAwareSafety(QStringLiteral("project.source.setProperty"), baudRate),
           Safety::Confirm);
}

QTEST_GUILESS_MAIN(TstAssistantBatchTiers)

#include "tst_assistant_batch_tiers.moc"
