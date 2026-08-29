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

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#include "UI/WidgetManifestParser.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Validation KATs for the spec-0038 widget manifest parser: the version-range grammar and
 *        every rejection the trust model depends on (reserved ids, the bundled-only "replaces"
 *        key, host compatibility, and package containment of the declared QML entry).
 */
class TstWidgetManifest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void versionInRange_data();
  void versionInRange();

  void acceptsWellFormedPackage();
  void refusesUnusableShape();
  void refusesReservedId();
  void acceptsBundledReplacement();
  void refusesReplacesFromDisk();
  void refusesIncompatibleHost();
  void refusesNewerManifestFormat();
  void refusesEscapingEntryPoint();
  void refusesUnusableConfigProperty();
  void clampsDeclaredBounds();

private:
  [[nodiscard]] static QJsonObject baseManifest(const QString& id);
  [[nodiscard]] UI::WidgetManifestParser::Result parse(const QJsonObject& manifest,
                                                       bool bundled) const;

  QTemporaryDir m_package;
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Materializes the one package directory every case parses against: the entry-point check
 *        stats the declared QML file, so it needs a real file on disk to reach the later stages.
 */
void TstWidgetManifest::initTestCase()
{
  QVERIFY(m_package.isValid());

  QFile qml(m_package.path() + QStringLiteral("/Widget.qml"));
  QVERIFY(qml.open(QFile::WriteOnly));
  QVERIFY(qml.write("import QtQuick\nItem {}\n") > 0);
  qml.close();
}

/**
 * @brief Returns a manifest that passes every stage, so each case can break exactly one key.
 */
QJsonObject TstWidgetManifest::baseManifest(const QString& id)
{
  QJsonObject widget;
  widget.insert(QStringLiteral("qml"), QStringLiteral("Widget.qml"));
  widget.insert(QStringLiteral("scope"), QStringLiteral("dataset"));

  QJsonObject manifest;
  manifest.insert(QStringLiteral("id"), id);
  manifest.insert(QStringLiteral("type"), QStringLiteral("widget"));
  manifest.insert(QStringLiteral("title"), QStringLiteral("Test Widget"));
  manifest.insert(QStringLiteral("version"), QStringLiteral("1.2.3"));
  manifest.insert(QStringLiteral("widget"), widget);
  return manifest;
}

/**
 * @brief Parses against a host that publishes widget API 1.0 and reserves two builtin strings.
 */
UI::WidgetManifestParser::Result TstWidgetManifest::parse(const QJsonObject& manifest,
                                                          const bool bundled) const
{
  const UI::WidgetManifestParser parser(QStringLiteral("1.0"),
                                        {QStringLiteral("gauge"), QStringLiteral("compass")});
  return parser.parse(manifest, m_package.path(), bundled);
}

//--------------------------------------------------------------------------------------------------
// Version ranges
//--------------------------------------------------------------------------------------------------

void TstWidgetManifest::versionInRange_data()
{
  QTest::addColumn<QString>("version");
  QTest::addColumn<QString>("range");
  QTest::addColumn<bool>("expected");

  QTest::newRow("empty range is any") << QStringLiteral("1.0") << QString() << true;
  QTest::newRow("star is any") << QStringLiteral("1.0") << QStringLiteral("*") << true;
  QTest::newRow("whitespace star is any") << QStringLiteral("1.0") << QStringLiteral(" * ") << true;

  QTest::newRow("bare equality holds") << QStringLiteral("1.0") << QStringLiteral("1.0") << true;
  QTest::newRow("bare equality fails") << QStringLiteral("1.1") << QStringLiteral("1.0") << false;
  QTest::newRow("explicit ==") << QStringLiteral("2.4") << QStringLiteral("==2.4") << true;
  QTest::newRow("single =") << QStringLiteral("2.4") << QStringLiteral("=2.4") << true;

  QTest::newRow(">= at the bound") << QStringLiteral("1.0") << QStringLiteral(">=1.0") << true;
  QTest::newRow(">= below the bound") << QStringLiteral("0.9") << QStringLiteral(">=1.0") << false;
  QTest::newRow("<= at the bound") << QStringLiteral("1.0") << QStringLiteral("<=1.0") << true;
  QTest::newRow("> at the bound") << QStringLiteral("1.0") << QStringLiteral(">1.0") << false;
  QTest::newRow("< below the bound") << QStringLiteral("0.9") << QStringLiteral("<1.0") << true;

  QTest::newRow("both clauses hold")
    << QStringLiteral("1.4") << QStringLiteral(">=1.0 <2.0") << true;
  QTest::newRow("second clause fails")
    << QStringLiteral("2.1") << QStringLiteral(">=1.0 <2.0") << false;

  QTest::newRow("garbage clause fails closed")
    << QStringLiteral("1.0") << QStringLiteral("~>1.0") << false;
  QTest::newRow("unparsable version fails closed")
    << QStringLiteral("nightly") << QStringLiteral(">=1.0") << false;
}

/**
 * @brief The comparator grammar fails closed: anything the pattern cannot read is refused rather
 *        than treated as "any version", which is what keeps a malformed range from opening the
 *        compatibility gate.
 */
void TstWidgetManifest::versionInRange()
{
  QFETCH(QString, version);
  QFETCH(QString, range);
  QFETCH(bool, expected);

  QCOMPARE(UI::WidgetManifestParser::versionInRange(version, range), expected);
}

//--------------------------------------------------------------------------------------------------
// Acceptance
//--------------------------------------------------------------------------------------------------

/**
 * @brief A well-formed third-party manifest registers with its declared identity intact, and the
 *        keys the host reads later (scope, readsStringValues) survive verbatim.
 */
void TstWidgetManifest::acceptsWellFormedPackage()
{
  auto manifest = baseManifest(QStringLiteral("com.example.dial"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("scope"), QStringLiteral("group"));
  widget.insert(QStringLiteral("readsStringValues"), true);
  widget.insert(QStringLiteral("hostCompat"), QStringLiteral(">=1.0 <2.0"));
  manifest.insert(QStringLiteral("widget"), widget);

  const auto result = parse(manifest, false);

  QVERIFY(result.ok);
  QVERIFY(result.findings.isEmpty());
  QVERIFY(result.descriptor.isValid());
  QCOMPARE(result.descriptor.id, QStringLiteral("com.example.dial"));
  QCOMPARE(result.descriptor.qmlFile, QStringLiteral("Widget.qml"));
  QCOMPARE(static_cast<int>(result.descriptor.scope),
           static_cast<int>(UI::WidgetExtensions::GroupScope));
  QCOMPARE(result.descriptor.readsStringValues, true);
  QCOMPARE(result.descriptor.bundled, false);
  QVERIFY(result.descriptor.replaces.isEmpty());
}

/**
 * @brief Declared bounds only ever narrow what the picker offers: a max below the min is lifted to
 *        the min, and a default size outside the allowed span is clamped instead of rejected.
 */
void TstWidgetManifest::clampsDeclaredBounds()
{
  QJsonObject datasets;
  datasets.insert(QStringLiteral("min"), 4);
  datasets.insert(QStringLiteral("max"), 2);

  QJsonObject accepts;
  accepts.insert(QStringLiteral("datasets"), datasets);
  accepts.insert(QStringLiteral("value"), QStringLiteral("string"));

  QJsonObject size;
  size.insert(QStringLiteral("width"), 4);
  size.insert(QStringLiteral("height"), 999999);

  auto manifest = baseManifest(QStringLiteral("com.example.bounds"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("accepts"), accepts);
  widget.insert(QStringLiteral("defaultSize"), size);
  manifest.insert(QStringLiteral("widget"), widget);

  const auto result = parse(manifest, false);

  QVERIFY(result.ok);
  QCOMPARE(result.descriptor.accepts.minDatasets, 4);
  QCOMPARE(result.descriptor.accepts.maxDatasets, 4);
  QCOMPARE(static_cast<int>(result.descriptor.accepts.value),
           static_cast<int>(UI::WidgetExtensions::StringValue));
  QCOMPARE(result.descriptor.defaultWidth, 48);
  QCOMPARE(result.descriptor.defaultHeight, 8192);
}

//--------------------------------------------------------------------------------------------------
// Rejections
//--------------------------------------------------------------------------------------------------

/**
 * @brief A manifest that never claims to describe a widget is refused before any other stage runs.
 */
void TstWidgetManifest::refusesUnusableShape()
{
  auto manifest = baseManifest(QStringLiteral("com.example.shape"));
  manifest.insert(QStringLiteral("type"), QStringLiteral("theme"));

  const auto result = parse(manifest, false);

  QVERIFY(!result.ok);
  QCOMPARE(result.findings.size(), qsizetype(1));
  QCOMPARE(result.findings.first().code, QStringLiteral("widget-manifest-invalid"));
}

/**
 * @brief The reserved-id rule is what keeps an extension id from ever resolving to a builtin (Pro
 *        ones included): a package installed from disk may not claim one, whatever else it says.
 */
void TstWidgetManifest::refusesReservedId()
{
  const auto result = parse(baseManifest(QStringLiteral("gauge")), false);

  QVERIFY(!result.ok);
  QVERIFY(!result.descriptor.isValid());
  QCOMPARE(result.findings.size(), qsizetype(1));
  QCOMPARE(result.findings.first().code, QStringLiteral("widget-id-reserved"));
}

/**
 * @brief A package shipped inside the application may ship as the implementation of the builtin
 *        string it names, and only then does "replaces" survive onto the descriptor.
 */
void TstWidgetManifest::acceptsBundledReplacement()
{
  auto manifest = baseManifest(QStringLiteral("compass"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("replaces"), QStringLiteral("compass"));
  manifest.insert(QStringLiteral("widget"), widget);

  const auto result = parse(manifest, true);

  QVERIFY(result.ok);
  QCOMPARE(result.descriptor.replaces, QStringLiteral("compass"));
  QCOMPARE(result.descriptor.bundled, true);
}

/**
 * @brief The same manifest read from the workspace extensions folder is refused: replacing a
 *        builtin is a bundled-only privilege, so an installed package cannot inherit one.
 */
void TstWidgetManifest::refusesReplacesFromDisk()
{
  auto manifest = baseManifest(QStringLiteral("com.example.compass"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("replaces"), QStringLiteral("compass"));
  manifest.insert(QStringLiteral("widget"), widget);

  const auto result = parse(manifest, false);

  QVERIFY(!result.ok);
  QCOMPARE(result.findings.size(), qsizetype(1));
  QCOMPARE(result.findings.first().code, QStringLiteral("widget-replaces-forbidden"));
}

/**
 * @brief A package that supports only a future host range is refused rather than loaded blind.
 */
void TstWidgetManifest::refusesIncompatibleHost()
{
  auto manifest = baseManifest(QStringLiteral("com.example.future"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("hostCompat"), QStringLiteral(">=2.0"));
  manifest.insert(QStringLiteral("widget"), widget);

  const auto result = parse(manifest, false);

  QVERIFY(!result.ok);
  QCOMPARE(result.findings.size(), qsizetype(1));
  QCOMPARE(result.findings.first().code, QStringLiteral("widget-host-incompatible"));
}

/**
 * @brief A manifest grammar the build cannot read is refused on its major version alone; a newer
 *        minor is additive and stays loadable.
 */
void TstWidgetManifest::refusesNewerManifestFormat()
{
  auto manifest = baseManifest(QStringLiteral("com.example.grammar"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("apiVersion"), QStringLiteral("2.0"));
  manifest.insert(QStringLiteral("widget"), widget);

  const auto rejected = parse(manifest, false);
  QVERIFY(!rejected.ok);
  QCOMPARE(rejected.findings.size(), qsizetype(1));
  QCOMPARE(rejected.findings.first().code, QStringLiteral("widget-api-version"));

  widget.insert(QStringLiteral("apiVersion"), QStringLiteral("1.9"));
  manifest.insert(QStringLiteral("widget"), widget);
  QVERIFY(parse(manifest, false).ok);
}

/**
 * @brief The declared QML entry must stay inside the package: every consumer resolves it by
 *        concatenation, so an escaping value would point the loader at an arbitrary file.
 */
void TstWidgetManifest::refusesEscapingEntryPoint()
{
  auto manifest = baseManifest(QStringLiteral("com.example.escape"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("qml"), QStringLiteral("../Widget.qml"));
  manifest.insert(QStringLiteral("widget"), widget);

  const auto escaping = parse(manifest, false);
  QVERIFY(!escaping.ok);
  QCOMPARE(escaping.findings.size(), qsizetype(1));
  QCOMPARE(escaping.findings.first().code, QStringLiteral("widget-qml-missing"));

  widget.insert(QStringLiteral("qml"), QStringLiteral("Missing.qml"));
  manifest.insert(QStringLiteral("widget"), widget);

  const auto absent = parse(manifest, false);
  QVERIFY(!absent.ok);
  QCOMPARE(absent.findings.first().code, QStringLiteral("widget-qml-missing"));
}

/**
 * @brief A settings declaration the generic form cannot render blocks the package, since a widget
 *        whose configuration cannot be built is not usable either.
 */
void TstWidgetManifest::refusesUnusableConfigProperty()
{
  QJsonObject property;
  property.insert(QStringLiteral("id"), QStringLiteral("mode"));
  property.insert(QStringLiteral("type"), QStringLiteral("choice"));

  QJsonArray config;
  config.append(property);

  auto manifest = baseManifest(QStringLiteral("com.example.config"));
  auto widget   = manifest.value(QStringLiteral("widget")).toObject();
  widget.insert(QStringLiteral("config"), config);
  manifest.insert(QStringLiteral("widget"), widget);

  const auto result = parse(manifest, false);

  QVERIFY(!result.ok);
  QCOMPARE(result.findings.size(), qsizetype(1));
  QCOMPARE(result.findings.first().code, QStringLiteral("widget-config-invalid"));
  QVERIFY(result.descriptor.config.isEmpty());
}

QTEST_APPLESS_MAIN(TstWidgetManifest)

#include "tst_widget_manifest.moc"
