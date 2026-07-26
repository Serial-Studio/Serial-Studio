/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QVariantList>
#include <QVariantMap>

namespace Misc {
class Translator;
}  // namespace Misc

namespace UI {

/**
 * @brief Serves the fixed UI command catalog (spec 0028): per-context command
 *        lists, per-surface layout trees, and per-window shortcut sets, all
 *        loaded from the :/commands manifests. Behavior never lives here;
 *        QML bindings files map ids to run/enabled/checked/visible.
 */
class CommandRegistry : public QObject {
  Q_OBJECT

signals:
  void commandsChanged();

private:
  explicit CommandRegistry();
  CommandRegistry(CommandRegistry&&)                 = delete;
  CommandRegistry(const CommandRegistry&)            = delete;
  CommandRegistry& operator=(CommandRegistry&&)      = delete;
  CommandRegistry& operator=(const CommandRegistry&) = delete;

public:
  [[nodiscard]] static CommandRegistry& instance();

  Q_INVOKABLE [[nodiscard]] QVariantList commands(const QString& context) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap layout(const QString& surface) const;
  Q_INVOKABLE [[nodiscard]] QVariantList shortcutCommands(const QString& window) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap command(const QString& id) const;

private:
  void loadManifest(const QString& path);
  void loadLayout(const QString& path);
  [[nodiscard]] QJsonArray filterLayoutNodes(const QJsonArray& nodes) const;
  [[nodiscard]] QVariantMap toVariant(const QJsonObject& command) const;
  [[nodiscard]] QVariantList layoutNodes(const QJsonArray& nodes) const;
  [[nodiscard]] QVariantMap commandNode(const QJsonObject& node) const;
  [[nodiscard]] QVariantMap containerNode(const QJsonObject& node) const;
  [[nodiscard]] static QStringList sequencesFor(const QString& shortcut);
  [[nodiscard]] static QString translated(const QJsonObject& object, const char* key);

private:
  Misc::Translator& m_translator;

  QList<QJsonObject> m_commands;
  QHash<QString, int> m_commandIndex;
  QHash<QString, QJsonObject> m_layouts;
};

}  // namespace UI
