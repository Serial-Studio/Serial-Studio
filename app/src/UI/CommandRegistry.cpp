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

#include "UI/CommandRegistry.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QKeySequence>
#include <QtLogging>

#include "Misc/Translator.h"
#include "SSAssert.h"

/**
 * @brief Name-to-StandardKey table for the "StandardKey.X" shortcut spellings
 *        the manifests use; extend when a manifest adds a new standard key.
 */
static const QHash<QString, QKeySequence::StandardKey> kStandardKeys = {
  {       QStringLiteral("Open"),        QKeySequence::Open},
  {        QStringLiteral("New"),         QKeySequence::New},
  {       QStringLiteral("Save"),        QKeySequence::Save},
  {       QStringLiteral("Quit"),        QKeySequence::Quit},
  {       QStringLiteral("Back"),        QKeySequence::Back},
  {    QStringLiteral("Forward"),     QKeySequence::Forward},
  {      QStringLiteral("Close"),       QKeySequence::Close},
  {QStringLiteral("Preferences"), QKeySequence::Preferences},
  {       QStringLiteral("Undo"),        QKeySequence::Undo},
  {       QStringLiteral("Redo"),        QKeySequence::Redo},
};

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads and validates every command and layout manifest from resources;
 *        malformed entries warn and are skipped, never fatal.
 */
UI::CommandRegistry::CommandRegistry() : m_translator(Misc::Translator::instance())
{
  loadManifest(QStringLiteral(":/commands/app.json"));
  loadManifest(QStringLiteral(":/commands/dashboard.json"));
  loadManifest(QStringLiteral(":/commands/projecteditor.json"));
  loadManifest(QStringLiteral(":/commands/database.json"));
  loadLayout(QStringLiteral(":/commands/layouts/main-toolbar.json"));
  loadLayout(QStringLiteral(":/commands/layouts/project-toolbar.json"));
  loadLayout(QStringLiteral(":/commands/layouts/start-menu.json"));
  loadLayout(QStringLiteral(":/commands/layouts/database-toolbar.json"));

  QHash<QString, QString> claimed;
  for (const auto& command : m_commands) {
    const auto shortcut = command.value(QStringLiteral("shortcut")).toString();
    if (shortcut.isEmpty())
      continue;

    const auto windows = command.value(QStringLiteral("shortcutWindows")).toArray();
    for (const auto& window : windows) {
      const auto key = window.toString() + QChar('|') + shortcut;
      if (claimed.contains(key)) {
        qWarning() << "CommandRegistry: duplicate shortcut" << shortcut << "in window"
                   << window.toString() << "claimed by" << claimed.value(key) << "and"
                   << command.value(QStringLiteral("id")).toString();
      }

      claimed.insert(key, command.value(QStringLiteral("id")).toString());
    }
  }

  connect(
    &m_translator, &Misc::Translator::languageChanged, this, &CommandRegistry::commandsChanged);
}

/**
 * @brief Returns the global CommandRegistry instance.
 */
UI::CommandRegistry& UI::CommandRegistry::instance()
{
  static CommandRegistry s;
  return s;
}

//--------------------------------------------------------------------------------------------------
// Manifest loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses one command manifest, dropping Pro commands on GPL builds and
 *        duplicate ids everywhere.
 */
void UI::CommandRegistry::loadManifest(const QString& path)
{
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    qWarning() << "CommandRegistry: cannot open" << path;
    return;
  }

  QJsonParseError error{};
  const auto document = QJsonDocument::fromJson(file.readAll(), &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) {
    qWarning() << "CommandRegistry: invalid manifest" << path << error.errorString();
    return;
  }

  const auto commands = document.object().value(QStringLiteral("commands")).toArray();
  for (const auto& entry : commands) {
    auto command  = entry.toObject();
    const auto id = command.value(QStringLiteral("id")).toString();
    if (id.isEmpty() || m_commandIndex.contains(id)) {
      qWarning() << "CommandRegistry: missing or duplicate id in" << path << id;
      continue;
    }

#ifndef BUILD_COMMERCIAL
    if (command.value(QStringLiteral("pro")).toBool())
      continue;
#endif

    m_commandIndex.insert(id, m_commands.size());
    m_commands.append(command);
  }
}

/**
 * @brief Parses one layout manifest and stores its build-tier-filtered tree.
 */
void UI::CommandRegistry::loadLayout(const QString& path)
{
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    qWarning() << "CommandRegistry: cannot open" << path;
    return;
  }

  QJsonParseError error{};
  const auto document = QJsonDocument::fromJson(file.readAll(), &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) {
    qWarning() << "CommandRegistry: invalid layout" << path << error.errorString();
    return;
  }

  auto layout        = document.object();
  const auto surface = layout.value(QStringLiteral("surface")).toString();
  if (surface.isEmpty()) {
    qWarning() << "CommandRegistry: layout without surface name" << path;
    return;
  }

  for (const auto key : {"sections", "items", "pinnedEnd"})
    if (layout.contains(QLatin1String(key)))
      layout.insert(QLatin1String(key),
                    filterLayoutNodes(layout.value(QLatin1String(key)).toArray()));

  m_layouts.insert(surface, layout);
}

/**
 * @brief Removes nodes gated to the other build tier (pro on GPL, gplOnly on
 *        commercial), recursing into child item lists.
 */
QJsonArray UI::CommandRegistry::filterLayoutNodes(const QJsonArray& nodes) const
{
  QJsonArray result;
  for (const auto& entry : nodes) {
    auto node = entry.toObject();
#ifdef BUILD_COMMERCIAL
    if (node.value(QStringLiteral("gplOnly")).toBool())
      continue;
#else
    if (node.value(QStringLiteral("pro")).toBool())
      continue;
#endif

    if (node.contains(QStringLiteral("items")))
      node.insert(QStringLiteral("items"),
                  filterLayoutNodes(node.value(QStringLiteral("items")).toArray()));

    result.append(node);
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated command list for a palette/search context, in
 *        manifest "order" (falling back to load order).
 */
QVariantList UI::CommandRegistry::commands(const QString& context) const
{
  SS_ASSERT(!context.isEmpty(), return {});

  QList<QPair<int, int>> selected;
  for (int i = 0; i < m_commands.size(); ++i) {
    const auto& command = m_commands.at(i);
    const auto contexts = command.value(QStringLiteral("contexts")).toArray();
    for (const auto& entry : contexts) {
      if (entry.toString() == context) {
        const auto order = command.value(QStringLiteral("order")).toInt(1000);
        selected.append({order, i});
        break;
      }
    }
  }

  std::stable_sort(selected.begin(), selected.end());

  QVariantList result;
  result.reserve(selected.size());
  for (const auto& [order, index] : selected)
    result.append(toVariant(m_commands.at(index)));

  return result;
}

/**
 * @brief Returns one command as a translated variant map (empty when unknown).
 */
QVariantMap UI::CommandRegistry::command(const QString& id) const
{
  SS_ASSERT(!id.isEmpty(), return {});

  const auto index = m_commandIndex.value(id, -1);
  if (index < 0) {
    qWarning() << "CommandRegistry: unknown command" << id;
    return {};
  }

  return toVariant(m_commands.at(index));
}

/**
 * @brief Returns a surface layout with command nodes enriched by their command
 *        data (node-level title/tooltip/icon overrides applied).
 */
QVariantMap UI::CommandRegistry::layout(const QString& surface) const
{
  SS_ASSERT(!surface.isEmpty(), return {});

  const auto stored = m_layouts.value(surface);
  if (stored.isEmpty()) {
    qWarning() << "CommandRegistry: unknown surface" << surface;
    return {};
  }

  QVariantMap result;
  result.insert(QStringLiteral("surface"), surface);
  for (const auto key : {"sections", "items", "pinnedEnd"})
    if (stored.contains(QLatin1String(key)))
      result.insert(QLatin1String(key), layoutNodes(stored.value(QLatin1String(key)).toArray()));

  return result;
}

/**
 * @brief Returns the commands whose shortcuts a host window instantiates,
 *        each carrying resolved "sequences" and display text.
 */
QVariantList UI::CommandRegistry::shortcutCommands(const QString& window) const
{
  SS_ASSERT(!window.isEmpty(), return {});

  QVariantList result;
  for (const auto& command : m_commands) {
    const auto shortcut = command.value(QStringLiteral("shortcut")).toString();
    if (shortcut.isEmpty())
      continue;

    bool applies       = false;
    const auto windows = command.value(QStringLiteral("shortcutWindows")).toArray();
    for (const auto& entry : windows)
      applies = applies || (entry.toString() == window);

    if (!applies)
      continue;

    result.append(toVariant(command));
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Conversion helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts a stored command to its QML-facing variant map, translating
 *        user-visible strings and resolving shortcut sequences.
 */
QVariantMap UI::CommandRegistry::toVariant(const QJsonObject& command) const
{
  QVariantMap map;
  map.insert(QStringLiteral("id"), command.value(QStringLiteral("id")).toString());
  map.insert(QStringLiteral("kind"),
             command.value(QStringLiteral("kind")).toString(QStringLiteral("action")));
  map.insert(QStringLiteral("title"), translated(command, "title"));
  map.insert(QStringLiteral("tooltip"), translated(command, "tooltip"));
  map.insert(QStringLiteral("titleChecked"), translated(command, "titleChecked"));
  map.insert(QStringLiteral("icon"), command.value(QStringLiteral("icon")).toString());
  map.insert(QStringLiteral("iconChecked"),
             command.value(QStringLiteral("iconChecked")).toString());
  map.insert(QStringLiteral("category"), command.value(QStringLiteral("category")).toString());

  const auto shortcut  = command.value(QStringLiteral("shortcut")).toString();
  const auto sequences = sequencesFor(shortcut);
  map.insert(QStringLiteral("sequences"), sequences);
  map.insert(QStringLiteral("shortcutContext"),
             command.value(QStringLiteral("shortcutContext")).toString());
  if (!sequences.isEmpty())
    map.insert(QStringLiteral("shortcutText"),
               QKeySequence(sequences.first()).toString(QKeySequence::NativeText));
  else
    map.insert(QStringLiteral("shortcutText"), QString());

  return map;
}

/**
 * @brief Enriches layout nodes: command nodes merge their command data plus
 *        node overrides; container nodes translate titles, resolve collapsed
 *        commands, and recurse into children.
 */
QVariantList UI::CommandRegistry::layoutNodes(const QJsonArray& nodes) const
{
  QVariantList result;
  for (const auto& entry : nodes) {
    const auto node = entry.toObject();
    const auto type = node.value(QStringLiteral("type")).toString();
    auto map        = (type == QStringLiteral("command")) ? commandNode(node) : containerNode(node);
    if (map.isEmpty())
      continue;

    map.insert(QStringLiteral("type"), type);
    result.append(map);
  }

  return result;
}

/**
 * @brief Builds a layout command node: the command's data with node-level
 *        title/tooltip/icon/hints overrides applied. Commands absent from the
 *        catalog (dropped by build tier) resolve to an empty map by design.
 */
QVariantMap UI::CommandRegistry::commandNode(const QJsonObject& node) const
{
  const auto index = m_commandIndex.value(node.value(QStringLiteral("id")).toString(), -1);
  if (index < 0)
    return {};

  auto map = toVariant(m_commands.at(index));
  if (node.contains(QStringLiteral("title")))
    map.insert(QStringLiteral("title"), translated(node, "title"));

  if (node.contains(QStringLiteral("tooltip")))
    map.insert(QStringLiteral("tooltip"), translated(node, "tooltip"));

  if (node.contains(QStringLiteral("icon")))
    map.insert(QStringLiteral("icon"), node.value(QStringLiteral("icon")).toString());

  if (node.contains(QStringLiteral("hints")))
    map.insert(QStringLiteral("hints"),
               node.value(QStringLiteral("hints")).toObject().toVariantMap());

  map.insert(QStringLiteral("role"), node.value(QStringLiteral("role")).toString());
  return map;
}

/**
 * @brief Builds a section/submenu/grid/slot/separator node: translated title,
 *        recursed children, and the first build-visible collapsed command's
 *        title + icon for ribbon collapse.
 */
QVariantMap UI::CommandRegistry::containerNode(const QJsonObject& node) const
{
  auto map = node.toVariantMap();
  if (node.contains(QStringLiteral("title")))
    map.insert(QStringLiteral("title"), translated(node, "title"));

  if (node.contains(QStringLiteral("collapsedTitle")))
    map.insert(QStringLiteral("collapsedTitle"), translated(node, "collapsedTitle"));

  if (node.contains(QStringLiteral("items")))
    map.insert(QStringLiteral("items"), layoutNodes(node.value(QStringLiteral("items")).toArray()));

  const auto collapsed = node.value(QStringLiteral("collapsedCommands")).toArray();
  for (const auto& candidate : collapsed) {
    const auto index = m_commandIndex.value(candidate.toString(), -1);
    if (index < 0)
      continue;

    const auto resolved = toVariant(m_commands.at(index));
    if (!map.contains(QStringLiteral("collapsedTitle")))
      map.insert(QStringLiteral("collapsedTitle"), resolved.value(QStringLiteral("title")));

    if (!map.contains(QStringLiteral("collapsedIcon")))
      map.insert(QStringLiteral("collapsedIcon"), resolved.value(QStringLiteral("icon")));

    break;
  }

  return map;
}

/**
 * @brief Resolves a manifest shortcut spelling to portable key sequences
 *        ("StandardKey.X" expands to all platform bindings).
 */
QStringList UI::CommandRegistry::sequencesFor(const QString& shortcut)
{
  if (shortcut.isEmpty())
    return {};

  const auto prefix = QStringLiteral("StandardKey.");
  if (!shortcut.startsWith(prefix))
    return {shortcut};

  const auto name = shortcut.mid(prefix.length());
  if (!kStandardKeys.contains(name)) {
    qWarning() << "CommandRegistry: unknown standard key" << shortcut;
    return {};
  }

  QStringList result;
  const auto bindings = QKeySequence::keyBindings(kStandardKeys.value(name));
  for (const auto& binding : bindings)
    result.append(binding.toString(QKeySequence::PortableText));

  return result;
}

/**
 * @brief Translates a manifest string through the "Commands" context (empty
 *        stays empty so optional fields never yield stray translations).
 */
QString UI::CommandRegistry::translated(const QJsonObject& object, const char* key)
{
  const auto value = object.value(QLatin1String(key)).toString();
  if (value.isEmpty())
    return {};

  return QCoreApplication::translate("Commands", value.toUtf8().constData());
}
