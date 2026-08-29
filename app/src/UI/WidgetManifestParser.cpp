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

#include "UI/WidgetManifestParser.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QRegularExpression>
#include <QVersionNumber>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants & local aliases
//--------------------------------------------------------------------------------------------------

using Finding    = Misc::ProblemCenter::Finding;
using Severity   = Misc::ProblemCenter::Severity;
using Descriptor = UI::WidgetExtensions::Descriptor;

static constexpr int kMaxConfigProperties = 128;
static constexpr int kMaxDependencies     = 64;

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated text for the shared "Problems" translation context, which the
 *        problem-center panel and the API surface both read.
 */
[[nodiscard]] static QString trManifestProblem(const char* text)
{
  return QCoreApplication::translate("Problems", text);
}

/**
 * @brief Assembles one rejection finding; the checker id is stamped by the problem center.
 */
[[nodiscard]] static Finding makeManifestFinding(Severity severity,
                                                 const char* code,
                                                 const QString& title,
                                                 const QString& explanation,
                                                 const QString& remedy)
{
  Finding finding;
  finding.severity    = severity;
  finding.code        = QString::fromLatin1(code);
  finding.title       = title;
  finding.remedy      = remedy;
  finding.explanation = explanation;
  return finding;
}

/**
 * @brief Parses one comparator clause ("&gt;=1.0", "&lt;2.0", "1.4") and evaluates it against the
 *        host version; an unparsable clause fails closed so a malformed range never opens the gate.
 */
[[nodiscard]] static bool clauseSatisfied(const QString& clause, const QVersionNumber& host)
{
  static const QRegularExpression pattern(
    QStringLiteral("^(>=|<=|==|=|>|<)?\\s*([0-9]+(?:\\.[0-9]+)*)$"));

  const auto match = pattern.match(clause.trimmed());
  if (!match.hasMatch())
    return false;

  const auto op    = match.captured(1);
  const auto bound = QVersionNumber::fromString(match.captured(2));
  const int order  = QVersionNumber::compare(host, bound);

  if (op == QStringLiteral(">="))
    return order >= 0;

  if (op == QStringLiteral("<="))
    return order <= 0;

  if (op == QStringLiteral(">"))
    return order > 0;

  if (op == QStringLiteral("<"))
    return order < 0;

  return order == 0;
}

//--------------------------------------------------------------------------------------------------
// Field readers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies the package-level identity fields shared with every other extension type.
 */
static void readPackageFields(const QJsonObject& manifest, Descriptor& out)
{
  out.id          = manifest.value(QStringLiteral("id")).toString().trimmed();
  out.title       = manifest.value(QStringLiteral("title")).toString().trimmed();
  out.author      = manifest.value(QStringLiteral("author")).toString().trimmed();
  out.license     = manifest.value(QStringLiteral("license")).toString().trimmed();
  out.version     = manifest.value(QStringLiteral("version")).toString().trimmed();
  out.category    = manifest.value(QStringLiteral("category")).toString().trimmed();
  out.description = manifest.value(QStringLiteral("description")).toString().trimmed();
}

/**
 * @brief Reads the declared data-acceptance bounds, clamping a missing or nonsensical pair to the
 *        permissive default so a package can only ever narrow what the picker offers.
 */
[[nodiscard]] static UI::WidgetExtensions::Accepts readAccepts(const QJsonObject& block)
{
  UI::WidgetExtensions::Accepts accepts;

  const auto obj      = block.value(QStringLiteral("accepts")).toObject();
  const auto datasets = obj.value(QStringLiteral("datasets")).toObject();

  accepts.minDatasets = qMax(0, datasets.value(QStringLiteral("min")).toInt(1));
  accepts.maxDatasets = datasets.value(QStringLiteral("max")).toInt(-1);
  if (accepts.maxDatasets >= 0 && accepts.maxDatasets < accepts.minDatasets)
    accepts.maxDatasets = accepts.minDatasets;

  const auto value = obj.value(QStringLiteral("value")).toString();
  if (value == QStringLiteral("numeric"))
    accepts.value = UI::WidgetExtensions::NumericValue;
  else if (value == QStringLiteral("string"))
    accepts.value = UI::WidgetExtensions::StringValue;
  else
    accepts.value = UI::WidgetExtensions::AnyValue;

  return accepts;
}

/**
 * @brief Reads one declared config property, appending a finding when its id or type is unusable;
 *        v1 accepts the scalar and fixed-choice kinds of the spec-0036 vocabulary only.
 */
static void readConfigProperty(const QJsonObject& obj, Descriptor& out, QList<Finding>& findings)
{
  static const QStringList kTypes = {
    QStringLiteral("bool"),
    QStringLiteral("int"),
    QStringLiteral("double"),
    QStringLiteral("string"),
    QStringLiteral("choice"),
  };

  UI::WidgetExtensions::ConfigProperty property;
  property.id           = obj.value(QStringLiteral("id")).toString().trimmed();
  property.type         = obj.value(QStringLiteral("type")).toString().trimmed();
  property.label        = obj.value(QStringLiteral("label")).toString();
  property.description  = obj.value(QStringLiteral("description")).toString();
  property.defaultValue = obj.value(QStringLiteral("default")).toVariant();
  property.minimum      = obj.value(QStringLiteral("min")).toVariant();
  property.maximum      = obj.value(QStringLiteral("max")).toVariant();

  const auto options = obj.value(QStringLiteral("options")).toArray();
  for (const auto& option : options)
    property.options.append(option.toString());

  const bool badChoice = property.type == QStringLiteral("choice") && property.options.isEmpty();
  if (property.id.isEmpty() || !kTypes.contains(property.type) || badChoice) {
    findings.append(makeManifestFinding(
      Misc::ProblemCenter::Error,
      "widget-config-invalid",
      trManifestProblem("Widget extension declares an unusable setting"),
      trManifestProblem(
        "Package \"%1\" declares a setting that has no id, an unsupported type, or an "
        "empty choice list, so its settings form cannot be built.")
        .arg(out.id),
      trManifestProblem("Ask the package author to declare each setting with an id and one of the "
                        "supported types: bool, int, double, string, choice.")));
    return;
  }

  if (property.label.isEmpty())
    property.label = property.id;

  out.config.append(property);
}

/**
 * @brief Reads the declared dependency lists; entries without an id are dropped rather than
 *        blocking the package, since an unnamed dependency can never be resolved either way.
 */
static void readDependencies(const QJsonObject& block, Descriptor& out)
{
  const auto obj = block.value(QStringLiteral("dependencies")).toObject();

  const QList<QPair<QString, bool>> lists = {
    {QStringLiteral("required"),  true},
    {QStringLiteral("optional"), false},
  };

  for (const auto& list : lists) {
    const auto entries = obj.value(list.first).toArray();
    for (const auto& entry : entries) {
      if (out.dependencies.count() >= kMaxDependencies)
        return;

      UI::WidgetExtensions::Dependency dependency;
      dependency.required     = list.second;
      dependency.id           = entry.toObject().value(QStringLiteral("id")).toString().trimmed();
      dependency.versionRange = entry.toObject().value(QStringLiteral("version")).toString();

      if (!dependency.id.isEmpty())
        out.dependencies.append(dependency);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Validation stages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates the package-level shape: object, widget type, identity fields, and the nested
 *        widget block. Returns false as soon as the manifest cannot describe a widget at all.
 */
[[nodiscard]] static bool validateShape(const QJsonObject& manifest,
                                        const Descriptor& out,
                                        const QString& directory,
                                        QList<Finding>& findings)
{
  const bool isWidget =
    manifest.value(QStringLiteral("type")).toString() == QStringLiteral("widget");
  const bool hasBlock = manifest.value(QStringLiteral("widget")).isObject();

  if (!isWidget || !hasBlock || out.id.isEmpty() || out.title.isEmpty()) {
    findings.append(makeManifestFinding(
      Misc::ProblemCenter::Error,
      "widget-manifest-invalid",
      trManifestProblem("Widget extension manifest is not usable"),
      trManifestProblem(
        "The manifest in \"%1\" is missing its id, title, \"type\": \"widget\", or the "
        "\"widget\" block, so the package cannot be registered.")
        .arg(directory),
      trManifestProblem(
        "Compare the manifest against the widget manifest reference and reinstall the "
        "package.")));
    return false;
  }

  return true;
}

/**
 * @brief True when @p relative names a file that stays under @p directory. The only containment
 *        rule a manifest-declared path gets: every consumer resolves such paths by concatenation,
 *        so an escaping value would point the loader (or the icon fetch) at an arbitrary file.
 */
[[nodiscard]] static bool containedInPackage(const QString& directory, const QString& relative)
{
  const auto base = QFileInfo(directory).absoluteFilePath() + QStringLiteral("/");
  const auto path = QFileInfo(directory + QStringLiteral("/") + relative).absoluteFilePath();

  return !relative.isEmpty() && !relative.startsWith(QStringLiteral("/"))
      && !relative.contains(QStringLiteral("..")) && path.startsWith(base);
}

/**
 * @brief Resolves the declared QML entry inside the package directory: the file must exist and
 *        must stay under that directory, so a manifest cannot point the loader at another path.
 */
[[nodiscard]] static bool validateEntryPoint(const QJsonObject& block,
                                             Descriptor& out,
                                             const QString& directory,
                                             QList<Finding>& findings)
{
  const auto entry = block.value(QStringLiteral("qml")).toString().trimmed();

  const bool contained = containedInPackage(directory, entry);
  if (!contained || !QFileInfo::exists(directory + QStringLiteral("/") + entry)) {
    findings.append(makeManifestFinding(
      Misc::ProblemCenter::Error,
      "widget-qml-missing",
      trManifestProblem("Widget extension has no usable QML file"),
      trManifestProblem(
        "Package \"%1\" declares \"%2\" as its widget file, but that file is missing from "
        "the package or points outside it.")
        .arg(out.id, entry),
      trManifestProblem(
        "Reinstall the package; if the problem persists, report it to its author.")));
    return false;
  }

  out.qmlFile = entry;
  return true;
}

/**
 * @brief Reads the presentation and behavior keys that cannot fail validation: scope, icon,
 *        default size, the string-value declaration, and the experimental flag. A file-shaped
 *        icon that escapes the package is dropped rather than rejected -- artwork is optional,
 *        so the widget still loads on the built-in fallback.
 */
static void readWidgetBlock(const QJsonObject& block, Descriptor& out, const QString& directory)
{
  out.scope = block.value(QStringLiteral("scope")).toString() == QStringLiteral("group")
              ? UI::WidgetExtensions::GroupScope
              : UI::WidgetExtensions::DatasetScope;

  out.iconId = block.value(QStringLiteral("icon")).toString().trimmed();
  if (out.iconId.contains(QStringLiteral(".")) && !containedInPackage(directory, out.iconId))
    out.iconId.clear();

  out.experimental      = block.value(QStringLiteral("experimental")).toBool(false);
  out.readsStringValues = block.value(QStringLiteral("readsStringValues")).toBool(false);

  const auto size   = block.value(QStringLiteral("defaultSize")).toObject();
  out.defaultWidth  = qBound(48, size.value(QStringLiteral("width")).toInt(400), 8192);
  out.defaultHeight = qBound(48, size.value(QStringLiteral("height")).toInt(300), 8192);

  out.accepts = readAccepts(block);
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a parser that judges manifests against @p hostApiVersion and refuses any package
 *        claiming one of @p reservedIds. Both facts are injected rather than read from the catalog
 *        so validation stays a pure function of its inputs.
 */
UI::WidgetManifestParser::WidgetManifestParser(const QString& hostApiVersion,
                                               const QStringList& reservedIds)
  : m_hostApiVersion(hostApiVersion), m_reservedIds(reservedIds)
{}

//--------------------------------------------------------------------------------------------------
// Version ranges
//--------------------------------------------------------------------------------------------------

/**
 * @brief Evaluates a space-separated comparator range ("&gt;=1.0 &lt;2.0") against a version; an
 *        empty range or "*" means "any version", and every other clause must hold. Shared with the
 *        catalog, which evaluates package-to-package dependency ranges with the same grammar.
 */
bool UI::WidgetManifestParser::versionInRange(const QString& version, const QString& range)
{
  const auto trimmed = range.trimmed();
  if (trimmed.isEmpty() || trimmed == QStringLiteral("*"))
    return true;

  const auto target = QVersionNumber::fromString(version);
  if (target.isNull())
    return false;

  static const QRegularExpression separator(QStringLiteral("\\s+"));
  const auto clauses = trimmed.split(separator, Qt::SkipEmptyParts);
  for (const auto& clause : clauses)
    if (!clauseSatisfied(clause, target))
      return false;

  return !clauses.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// Catalog-dependent validation stages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enforces the reserved-id rule: no package may claim a builtin widget string, and only a
 *        package bundled with the application may declare "replaces" for the string it ships as.
 *        This is the mechanism that keeps an extension id from ever resolving to a Pro builtin.
 */
bool UI::WidgetManifestParser::validateIdentity(const QJsonObject& block,
                                                Descriptor& out,
                                                const bool bundled,
                                                QList<Finding>& findings) const
{
  const auto replaces = block.value(QStringLiteral("replaces")).toString().trimmed();
  if (!replaces.isEmpty() && !bundled) {
    findings.append(makeManifestFinding(
      Misc::ProblemCenter::Error,
      "widget-replaces-forbidden",
      trManifestProblem("Widget extension may not replace a built-in widget"),
      trManifestProblem("Package \"%1\" declares that it replaces the built-in widget \"%2\". Only "
                        "packages shipped inside the application may do that.")
        .arg(out.id, replaces),
      trManifestProblem(
        "Remove the \"replaces\" key from the package, or uninstall the package.")));
    return false;
  }

  const bool claimsBuiltin = m_reservedIds.contains(out.id);
  if (claimsBuiltin && !(bundled && replaces == out.id)) {
    findings.append(makeManifestFinding(
      Misc::ProblemCenter::Error,
      "widget-id-reserved",
      trManifestProblem("Widget extension uses a reserved identifier"),
      trManifestProblem(
        "Package \"%1\" claims an identifier that belongs to a built-in widget, so it was "
        "not registered.")
        .arg(out.id),
      trManifestProblem("Ask the package author to publish it under a unique identifier, such as a "
                        "reverse-domain name.")));
    return false;
  }

  out.replaces = bundled ? replaces : QString();
  return true;
}

/**
 * @brief Checks the manifest grammar version and the declared host range against the host widget
 *        API version, so an application update never silently loads a package built for another.
 */
bool UI::WidgetManifestParser::validateCompatibility(const QJsonObject& block,
                                                     const Descriptor& out,
                                                     QList<Finding>& findings) const
{
  const auto host = QVersionNumber::fromString(m_hostApiVersion);
  SS_ASSERT(!host.isNull(), return false);

  const auto apiVersion = block.value(QStringLiteral("apiVersion")).toString().trimmed();
  const auto declared   = QVersionNumber::fromString(apiVersion);
  if (!apiVersion.isEmpty()
      && (declared.isNull() || declared.majorVersion() > host.majorVersion())) {
    findings.append(makeManifestFinding(
      Misc::ProblemCenter::Error,
      "widget-api-version",
      trManifestProblem("Widget extension needs a newer manifest format"),
      trManifestProblem(
        "Package \"%1\" declares manifest version %2, which this build (%3) cannot read.")
        .arg(out.id, apiVersion, m_hostApiVersion),
      trManifestProblem("Update Serial Studio, or install a build of the package that targets this "
                        "version.")));
    return false;
  }

  const auto compat = block.value(QStringLiteral("hostCompat")).toString();
  if (!versionInRange(m_hostApiVersion, compat)) {
    findings.append(makeManifestFinding(
      Misc::ProblemCenter::Error,
      "widget-host-incompatible",
      trManifestProblem("Widget extension is not compatible with this version"),
      trManifestProblem(
        "Package \"%1\" supports widget API %2, but this build provides %3, so it was not "
        "loaded.")
        .arg(out.id, compat, m_hostApiVersion),
      trManifestProblem(
        "Check the package for an update, or keep using the Serial Studio version it was "
        "built for.")));
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates one widget manifest and returns either a descriptor or the findings explaining
 *        the rejection. Dependency resolution needs the whole catalog and therefore runs later, in
 *        UI::WidgetExtensions::rescan(); nothing here touches QML.
 */
UI::WidgetManifestParser::Result UI::WidgetManifestParser::parse(const QJsonObject& manifest,
                                                                 const QString& directory,
                                                                 const bool bundled) const
{
  SS_ASSERT(!directory.isEmpty(), return Result());

  Result result;
  auto& out   = result.descriptor;
  out.bundled = bundled;

  readPackageFields(manifest, out);
  if (!validateShape(manifest, out, directory, result.findings))
    return result;

  out.directory    = directory;
  const auto block = manifest.value(QStringLiteral("widget")).toObject();

  if (!validateIdentity(block, out, bundled, result.findings))
    return result;

  if (!validateCompatibility(block, out, result.findings))
    return result;

  if (!validateEntryPoint(block, out, directory, result.findings))
    return result;

  readWidgetBlock(block, out, directory);
  readDependencies(block, out);

  const auto config = block.value(QStringLiteral("config")).toArray();
  for (const auto& property : config) {
    if (out.config.count() >= kMaxConfigProperties)
      break;

    readConfigProperty(property.toObject(), out, result.findings);
  }

  result.ok = result.findings.isEmpty() && out.isValid();
  return result;
}
