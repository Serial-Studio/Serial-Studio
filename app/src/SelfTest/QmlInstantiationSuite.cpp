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

#include <algorithm>
#include <cstddef>
#include <memory>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <QVariant>
#include <vector>

#include "Core/SSAssert.h"
#include "Misc/ContextRegistry.h"
#include "Misc/IconEngine.h"
#include "SelfTest/SelfTest.h"

namespace SelfTest {

//---------------------------------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------------------------------

static const char* const kQmlRoot            = ":/serial-studio.com";
static const char* const kActionIconProvider = "actionicon";
static const char* const kRequiredProperty   = "Required property";

//---------------------------------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------------------------------

/**
 * @brief Every warning the engine raised while the files were instantiated. Warnings stage per
 *        file first, because a file whose root declares required properties cannot be created
 *        standalone and every warning below it follows from that; the rest sort into the three
 *        buckets the report distinguishes: a name nobody defines (a failure), a name that is an
 *        `id` of an enclosing file (expected when a fragment is created standalone), and
 *        everything else (printed once per distinct location).
 */
struct WarningTally {
  QList<QQmlError> staged;
  QSet<QString> seen;
  QStringList unresolved;
  QStringList findings;
  int outer_id_references;
  int required_property_files;
  int total;
};

//---------------------------------------------------------------------------------------------------
// Source discovery
//---------------------------------------------------------------------------------------------------

/**
 * @brief Every .qml file compiled into the binary, in a stable order.
 */
[[nodiscard]] static QStringList compiledQmlFiles()
{
  QStringList files;
  QDirIterator it(QString::fromLatin1(kQmlRoot),
                  {QStringLiteral("*.qml")},
                  QDir::Files,
                  QDirIterator::Subdirectories);
  while (it.hasNext())
    files.append(it.next());

  SS_ASSERT_LOG(!files.isEmpty());
  files.sort();
  return files;
}

/**
 * @brief Every distinct capture of @p pattern across the sources of @p files, sorted.
 */
[[nodiscard]] static QStringList scrapeSources(const QStringList& files,
                                               const QRegularExpression& pattern)
{
  SS_ASSERT(pattern.isValid(), return {});
  SS_ASSERT_LOG(!files.isEmpty());

  QSet<QString> names;
  for (const auto& path : files) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      continue;

    const QString source = QString::fromUtf8(file.readAll());
    auto matches         = pattern.globalMatch(source);
    while (matches.hasNext())
      names.insert(matches.next().captured(1));
  }

  QStringList sorted(names.begin(), names.end());
  sorted.sort();
  return sorted;
}

/**
 * @brief Every `Cpp_*` name the QML tree reads, scraped from the sources rather than listed:
 *        the failure this suite catches arrives exactly when somebody adds a name to QML that
 *        nobody remembered to register.
 */
[[nodiscard]] static QStringList referencedContextNames(const QStringList& files)
{
  static const QRegularExpression pattern(QStringLiteral("\\b(Cpp_[A-Za-z0-9_]+)\\b"));
  return scrapeSources(files, pattern);
}

/**
 * @brief Every `id:` the QML tree declares. A fragment created standalone legitimately fails to
 *        resolve the id of the file that normally encloses it, so those names are not findings.
 */
[[nodiscard]] static QStringList declaredIds(const QStringList& files)
{
  static const QRegularExpression pattern(
    QStringLiteral("^\\s*id\\s*:\\s*([A-Za-z_][A-Za-z0-9_]*)"),
    QRegularExpression::MultilineOption);
  return scrapeSources(files, pattern);
}

//---------------------------------------------------------------------------------------------------
// Context preparation
//---------------------------------------------------------------------------------------------------

/**
 * @brief True when the root context carries no usable object under @p name: either the headless
 *        root never registered it or registered a null (the updater), so a binding would read
 *        undefined or null instead of the global the GUI build sees.
 */
[[nodiscard]] static bool rootGlobalUnset(const QQmlContext& root, const QString& name)
{
  SS_ASSERT(!name.isEmpty(), return false);

  const QVariant value = root.contextProperty(name);
  if (!value.isValid())
    return true;

  const bool pointer = (value.metaType().flags() & QMetaType::PointerToQObject) != 0;
  return pointer && value.value<QObject*>() == nullptr;
}

/**
 * @brief Shadows, in @p context, every registered global the headless root left unset with an
 *        empty QObject, so a binding reaches an object rather than undefined. A name this build
 *        does NOT register stays undefined on purpose: reading it is the ReferenceError the GPL
 *        run exists to catch.
 */
static void stubUnsetGlobals(QQmlContext& context,
                             const QStringList& names,
                             std::vector<std::unique_ptr<QObject>>& stubs)
{
  SS_ASSERT(context.parentContext() != nullptr, return);
  SS_ASSERT_LOG(!names.isEmpty());

  const QStringList& registered = Misc::ContextRegistry::names();
  for (const auto& name : names) {
    if (!registered.contains(name) || !rootGlobalUnset(*context.parentContext(), name))
      continue;

    stubs.push_back(std::make_unique<QObject>());
    context.setContextProperty(name, stubs.back().get());
  }
}

/**
 * @brief Registers the action-icon image provider when the headless root skipped it, so an
 *        `image://actionicon/` source resolves instead of warning.
 */
static void ensureActionIconProvider(QQmlEngine& engine)
{
  const QString id = QString::fromLatin1(kActionIconProvider);
  SS_ASSERT(!id.isEmpty(), return);
  if (engine.imageProvider(id) != nullptr)
    return;

  engine.addImageProvider(id, new Misc::ActionIconProvider());
  SS_ASSERT_LOG(engine.imageProvider(id) != nullptr);
}

//---------------------------------------------------------------------------------------------------
// Warning classification
//---------------------------------------------------------------------------------------------------

/**
 * @brief The name a ReferenceError complains about, or an empty string for any other warning.
 */
[[nodiscard]] static QString unresolvedName(const QString& description)
{
  static const QRegularExpression pattern(
    QStringLiteral("ReferenceError: ([A-Za-z_][A-Za-z0-9_]*) is not defined"));
  SS_ASSERT(pattern.isValid(), return {});

  const auto match = pattern.match(description);
  return match.hasMatch() ? match.captured(1) : QString();
}

/**
 * @brief Files one engine warning into @p tally, collapsing repeats of the same location.
 */
static void classifyWarning(const QQmlError& error, const QStringList& ids, WarningTally& tally)
{
  SS_ASSERT(tally.total >= 0, return);

  ++tally.total;
  const QString name = unresolvedName(error.description());
  if (!name.isEmpty() && ids.contains(name)) {
    ++tally.outer_id_references;
    return;
  }

  const QString line = QStringLiteral("%1:%2: %3")
                         .arg(error.url().toString())
                         .arg(error.line())
                         .arg(error.description());
  if (tally.seen.contains(line))
    return;

  tally.seen.insert(line);
  if (name.isEmpty())
    tally.findings.append(line);
  else
    tally.unresolved.append(line);
}

/**
 * @brief True when @p errors say the file's root declares a required property nobody set: the
 *        file is a fragment whose contract is that property, not a standalone component.
 */
[[nodiscard]] static bool needsRequiredProperty(const QList<QQmlError>& errors)
{
  const QString marker = QString::fromLatin1(kRequiredProperty);
  SS_ASSERT(!marker.isEmpty(), return false);

  return std::any_of(errors.begin(), errors.end(), [&marker](const QQmlError& error) {
    return error.description().contains(marker);
  });
}

/**
 * @brief Files everything staged for one file, or drops it when the file turned out to be a
 *        fragment that cannot be created without its required properties.
 */
static void settleFile(const QList<QQmlError>& errors, const QStringList& ids, WarningTally& tally)
{
  SS_ASSERT(tally.required_property_files >= 0, return);

  if (needsRequiredProperty(errors) || needsRequiredProperty(tally.staged)) {
    ++tally.required_property_files;
    tally.staged.clear();
    return;
  }

  for (const auto& error : tally.staged)
    classifyWarning(error, ids, tally);

  for (const auto& error : errors)
    classifyWarning(error, ids, tally);

  tally.staged.clear();
}

/**
 * @brief Instantiates one file inside @p context; a file that does not even compile reports its
 *        errors through the same tally as the runtime warnings.
 */
static void instantiateFile(QQmlEngine& engine,
                            QQmlContext& context,
                            const QString& path,
                            const QStringList& ids,
                            WarningTally& tally)
{
  SS_ASSERT(!path.isEmpty(), return);
  SS_ASSERT_LOG(tally.staged.isEmpty());

  QQmlComponent component(&engine, QUrl(QStringLiteral("qrc") + path));
  if (component.isError()) {
    settleFile(component.errors(), ids, tally);
    return;
  }

  SS_ASSERT_LOG(component.isReady());
  std::unique_ptr<QObject> instance(component.create(&context));
  settleFile(component.errors(), ids, tally);
}

/**
 * @brief Prints the tally: every unresolved name as a failure, every other distinct warning once,
 *        and the counts that explain what was folded away.
 */
static void reportTally(const WarningTally& tally, qsizetype files, SuiteResult& result)
{
  SS_ASSERT(files > 0, return);
  SS_ASSERT_LOG(tally.total >= tally.outer_id_references);

  for (const auto& line : tally.unresolved)
    qCritical().noquote() << "[selftest] qml FAILED:" << line;

  QStringList findings = tally.findings;
  std::sort(findings.begin(), findings.end());
  for (const auto& line : findings)
    qWarning().noquote() << "[selftest] qml warning:" << line;

  qInfo().noquote() << QStringLiteral("[selftest] qml: %1 files (%2 need required properties), "
                                      "%3 warnings (%4 distinct, %5 unresolved names, %6 reads "
                                      "of an enclosing file's id)")
                         .arg(files)
                         .arg(tally.required_property_files)
                         .arg(tally.total)
                         .arg(findings.size() + tally.unresolved.size())
                         .arg(tally.unresolved.size())
                         .arg(tally.outer_id_references);

  result.failures += static_cast<int>(tally.unresolved.size());
}

/**
 * @brief Records one failure for a precondition the suite cannot run without.
 */
static void failPrecondition(SuiteResult& result)
{
  SS_ASSERT_LOG(result.failures >= 0);
  SS_ASSERT_LOG(result.checks >= 0);
  ++result.failures;
}

//---------------------------------------------------------------------------------------------------
// Suite
//---------------------------------------------------------------------------------------------------

/**
 * @brief Instantiates every compiled QML file against the composition root's real `Cpp_*` globals,
 *        failing on any ReferenceError that names something neither registered by this build nor
 *        declared as an `id` elsewhere in the tree. Runs AFTER the composition root on its engine,
 *        inside a child context, and leaves the engine as it found it.
 */
void runQmlInstantiationSuite(SuiteResult& result, const SuiteEnvironment& env)
{
  SS_ASSERT(env.engine != nullptr, return failPrecondition(result));
  QQmlEngine& engine = *env.engine;
  SS_ASSERT(engine.rootContext() != nullptr, return failPrecondition(result));

  const QStringList files = compiledQmlFiles();
  SS_ASSERT(!files.isEmpty(), return failPrecondition(result));

  const QStringList names = referencedContextNames(files);
  const QStringList ids   = declaredIds(files);

  std::vector<std::unique_ptr<QObject>> stubs;
  stubs.reserve(static_cast<std::size_t>(names.size()));
  QQmlContext context(engine.rootContext());
  stubUnsetGlobals(context, names, stubs);
  ensureActionIconProvider(engine);

  WarningTally tally{{}, {}, {}, {}, 0, 0, 0};
  const bool echoed = engine.outputWarningsToStandardError();
  engine.setOutputWarningsToStandardError(false);
  const auto connection = QObject::connect(
    &engine, &QQmlEngine::warnings, &engine, [&tally](const QList<QQmlError>& list) {
      tally.staged.append(list);
    });

  ++result.checks;
  for (const auto& path : files)
    instantiateFile(engine, context, path, ids, tally);

  QObject::disconnect(connection);
  engine.setOutputWarningsToStandardError(echoed);
  reportTally(tally, files.size(), result);
}

}  // namespace SelfTest
