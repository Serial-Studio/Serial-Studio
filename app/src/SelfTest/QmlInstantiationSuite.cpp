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

#include <cstddef>
#include <memory>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <vector>

#include "Core/SSAssert.h"
#include "SelfTest/SelfTest.h"

namespace SelfTest {

//---------------------------------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------------------------------

static const char* const kQmlRoot = ":/serial-studio.com";

//---------------------------------------------------------------------------------------------------
// Source discovery
//---------------------------------------------------------------------------------------------------

/**
 * @brief Every .qml file compiled into the binary, in a stable order.
 */
static QStringList compiledQmlFiles()
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
 * @brief Every `Cpp_*` name the QML tree reads, scraped from the sources rather than listed:
 *        the failure this suite catches arrives exactly when somebody adds a name to QML that
 *        nobody remembered to add to a hardcoded list.
 */
static QStringList referencedContextNames(const QStringList& files)
{
  static const QRegularExpression pattern(QStringLiteral("\\bCpp_[A-Za-z0-9_]+\\b"));
  SS_ASSERT_LOG(pattern.isValid());

  QSet<QString> names;
  for (const auto& path : files) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      continue;

    const QString source = QString::fromUtf8(file.readAll());
    auto matches         = pattern.globalMatch(source);
    while (matches.hasNext())
      names.insert(matches.next().captured(0));
  }

  SS_ASSERT_LOG(!names.isEmpty());
  QStringList sorted(names.begin(), names.end());
  sorted.sort();
  return sorted;
}

//---------------------------------------------------------------------------------------------------
// Suite
//---------------------------------------------------------------------------------------------------

/**
 * @brief True when @p component failed on an undefined NAME rather than on missing context;
 *        instantiating a delegate or a required-property component standalone legitimately
 *        fails, so only a ReferenceError counts as a finding here.
 */
static bool hasReferenceError(const QQmlComponent& component, QString& detail)
{
  for (const auto& error : component.errors()) {
    const QString text = error.toString();
    if (!text.contains(QStringLiteral("is not defined"))
        && !text.contains(QStringLiteral("ReferenceError")))
      continue;

    detail = text;
    return true;
  }

  return false;
}

/**
 * @brief Instantiates every compiled QML file against stubs for the `Cpp_*` globals, failing on
 *        any ReferenceError. Runs AFTER the composition root, so the QML module's type
 *        registration has happened; the stub context is what keeps the result independent of
 *        which modules a given build actually registered.
 */
void runQmlInstantiationSuite(SuiteResult& result)
{
  const QStringList files = compiledQmlFiles();
  SS_ASSERT(!files.isEmpty(), return);

  QQmlEngine engine;
  const QStringList names = referencedContextNames(files);
  SS_ASSERT(engine.rootContext() != nullptr, return);

  std::vector<std::unique_ptr<QObject>> stubs;
  stubs.reserve(static_cast<std::size_t>(names.size()));
  for (const auto& name : names) {
    stubs.push_back(std::make_unique<QObject>());
    engine.rootContext()->setContextProperty(name, stubs.back().get());
  }

  ++result.checks;
  for (const auto& path : files) {
    QQmlComponent component(&engine, QUrl(QStringLiteral("qrc") + path));
    std::unique_ptr<QObject> instance(component.create());

    QString detail;
    if (!hasReferenceError(component, detail))
      continue;

    ++result.failures;
    qCritical().noquote() << "[selftest] qml FAILED:" << path << detail;
  }
}

}  // namespace SelfTest
