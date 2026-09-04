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

#pragma once

#include <functional>
#include <QAbstractListModel>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <vector>

namespace DataModel {
class NotificationCenter;
}  // namespace DataModel

namespace Misc {

/**
 * @brief Session-scoped diagnostics collector that is also the list model the UI binds to; a run
 *        replaces a checker's findings wholesale, so a fixed condition disappears by itself. The
 *        constructor is deliberately inert (member init only) to keep the spec-0001 ctor-edge
 *        proof trivial -- everything is wired in setupExternalConnections().
 */
class ProblemCenter : public QAbstractListModel {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int infoCount
             READ  infoCount
             NOTIFY findingsChanged)
  Q_PROPERTY(int errorCount
             READ  errorCount
             NOTIFY findingsChanged)
  Q_PROPERTY(int totalCount
             READ  totalCount
             NOTIFY findingsChanged)
  Q_PROPERTY(int warningCount
             READ  warningCount
             NOTIFY findingsChanged)
  Q_PROPERTY(QString lastRunTime
             READ  lastRunTime
             NOTIFY lastRunChanged)
  // clang-format on

public:
  enum Severity : int {
    Info    = 0,
    Warning = 1,
    Error   = 2,
  };
  Q_ENUM(Severity)

  enum Trigger : quint8 {
    NoTrigger      = 0,
    ProjectChanged = 1,
    LinkSample     = 2,
    OnDemand       = 4,
  };

  enum Role : int {
    SeverityRole = Qt::UserRole + 1,
    CodeRole,
    JumpRole,
    TitleRole,
    RemedyRole,
    EntityIdRole,
    CheckerIdRole,
    ExplanationRole,
  };

  /**
   * @brief One standing diagnostic: what is wrong, why, what to do about it, and where to go.
   *        @c entityUniqueId is -1 when the finding names no project entity, and @c jump is
   *        empty when it has no navigation target.
   */
  struct Finding {
    Severity severity  = Info;
    int entityUniqueId = -1;
    QString code;
    QString jump;
    QString title;
    QString remedy;
    QString checkerId;
    QString explanation;

    [[nodiscard]] bool operator==(const Finding& other) const noexcept;
  };

  using Checker = std::function<void(QList<Finding>&)>;

signals:
  void lastRunChanged();
  void findingsChanged();
  void jumpRequested(const QString& kind, int uniqueId);

private:
  explicit ProblemCenter();
  ProblemCenter(ProblemCenter&&)                 = delete;
  ProblemCenter(const ProblemCenter&)            = delete;
  ProblemCenter& operator=(ProblemCenter&&)      = delete;
  ProblemCenter& operator=(const ProblemCenter&) = delete;

public:
  [[nodiscard]] static ProblemCenter& instance();

  [[nodiscard]] int infoCount() const noexcept;
  [[nodiscard]] int errorCount() const noexcept;
  [[nodiscard]] int totalCount() const noexcept;
  [[nodiscard]] int warningCount() const noexcept;

  [[nodiscard]] QString lastRunTime() const;
  [[nodiscard]] QVariantList checkerCatalog() const;
  [[nodiscard]] const QList<Finding>& findings() const noexcept;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  void registerChecker(const QString& id, quint8 triggers, Checker checker);

  Q_INVOKABLE [[nodiscard]] bool activate(int row);

public slots:
  void clear();
  void runNow();
  void onLinkSample();
  void onProjectChanged();
  void setupExternalConnections();

private:
  void run(quint8 trigger);
  void rebuildFindings();
  void notifyNewFindings();

  [[nodiscard]] static QString findingKey(const Finding& finding);

private:
  static constexpr int kMaxFindingsPerChecker = 50;

  /**
   * @brief One registered checker: its stable id, the triggers it answers to, and the callable
   *        that appends this run's findings into the scratch list.
   */
  struct CheckerEntry {
    QString id;
    quint8 triggers;
    Checker checker;
  };

  int m_infoCount;
  int m_errorCount;
  int m_warningCount;

  QDateTime m_lastRun;
  QSet<QString> m_previousKeys;
  QList<Finding> m_findings;
  std::vector<CheckerEntry> m_checkers;
  std::vector<QList<Finding>> m_checkerFindings;

  DataModel::NotificationCenter* m_notifications;
};

}  // namespace Misc
