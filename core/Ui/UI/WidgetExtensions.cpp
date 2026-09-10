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

#include "UI/WidgetExtensions.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QVariantMap>
#include <utility>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/IconRegistry.h"
#include "Core/JsonValidator.h"
#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "Core/WorkspaceManager.h"
#include "Misc/ContextRegistry.h"
#include "UI/WidgetManifestParser.h"

//--------------------------------------------------------------------------------------------------
// Constants & local aliases
//--------------------------------------------------------------------------------------------------

using Finding = Misc::ProblemCenter::Finding;

static constexpr int kMaxPackages       = 256;
static constexpr qsizetype kMaxManifest = 1024 * 1024;

static const QString kBundledRoot   = QStringLiteral(":/extensions/widget");
static const QString kConsentPrefix = QStringLiteral("WidgetExtensionConsent/");
static const QString kDeclinePrefix = QStringLiteral("WidgetExtensionDecline/");

/**
 * @brief Returns the translated text for the shared "Problems" translation context.
 */
[[nodiscard]] static QString trPackageProblem(const char* text)
{
  return QCoreApplication::translate("Problems", text);
}

/**
 * @brief Assembles one catalog finding; the checker id is stamped by the problem center.
 */
[[nodiscard]] static Finding makePackageFinding(Misc::ProblemCenter::Severity severity,
                                                const QString& code,
                                                const QString& title,
                                                const QString& explanation,
                                                const QString& remedy)
{
  Finding finding;
  finding.severity    = severity;
  finding.code        = code;
  finding.title       = title;
  finding.remedy      = remedy;
  finding.explanation = explanation;
  return finding;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty catalog with no outgoing dependency at all: the spec-0001 ctor-edge
 *        proof holds only while this constructor stays a leaf (see the class documentation).
 */
UI::WidgetExtensions::WidgetExtensions() : m_settings(), m_bus(nullptr) {}

/**
 * @brief Returns the singleton widget-extension catalog.
 */
UI::WidgetExtensions& UI::WidgetExtensions::instance()
{
  static WidgetExtensions singleton;
  return singleton;
}

/**
 * @brief Adopts the root-owned message bus this module publishes on and subscribes to.
 */
void UI::WidgetExtensions::attachMessageBus(Core::Bus::MessageBus& bus)
{
  SS_ASSERT(m_bus == nullptr, return);
  m_bus = &bus;
}

//--------------------------------------------------------------------------------------------------
// Static contract
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the host widget API version a package declares compatibility against.
 */
QString UI::WidgetExtensions::hostApiVersion()
{
  return QStringLiteral("%1.%2")
    .arg(SerialStudio::kWidgetApiVersionMajor)
    .arg(SerialStudio::kWidgetApiVersionMinor);
}

/**
 * @brief Returns every widget string the built-in widgets already answer to. A package may not
 *        claim one, which is what keeps an extension id from ever resolving to a Pro widget type;
 *        the list is build-independent on purpose, so a GPL build reserves the Pro strings too.
 */
QStringList UI::WidgetExtensions::reservedIds()
{
  static const QStringList ids = {
    QStringLiteral("datagrid"),      QStringLiteral("map"),
    QStringLiteral("gps"),           QStringLiteral("gyro"),
    QStringLiteral("gyroscope"),     QStringLiteral("multiplot"),
    QStringLiteral("accelerometer"), QStringLiteral("plot3d"),
    QStringLiteral("image"),         QStringLiteral("painter"),
    QStringLiteral("webview"),       QStringLiteral("barpanel"),
    QStringLiteral("terminal"),      QStringLiteral("clock"),
    QStringLiteral("stopwatch"),     QStringLiteral("notification-log"),
    QStringLiteral("led-panel"),     QStringLiteral("bar"),
    QStringLiteral("gauge"),         QStringLiteral("compass"),
    QStringLiteral("meter"),
  };
  return ids;
}

/**
 * @brief Returns the host context-property names an extension's QML context shadows. This is a
 *        speed bump and not a boundary: the QML engine is shared, so a package that means to
 *        reach the host still can; the consent flow is what makes the trust model honest. The
 *        ONE table is Misc::ContextRegistry's, which is also what the composition root registers.
 */
QStringList UI::WidgetExtensions::hostContextNames()
{
  return Misc::ContextRegistry::objectNames();
}

/**
 * @brief Returns whether @p id belongs to a built-in widget string.
 */
bool UI::WidgetExtensions::isReservedId(const QString& id)
{
  return reservedIds().contains(id);
}

/**
 * @brief Returns the persisted type token for an extension widget ("ext:&lt;id&gt;"). Workspaces,
 *        freeze title modes, display titles, and per-widget settings substitute it for the numeric
 *        widget type, so a package's keys survive installing or removing unrelated packages.
 */
QString UI::WidgetExtensions::persistedTypeToken(const QString& id)
{
  return QStringLiteral("ext:") + id;
}

//--------------------------------------------------------------------------------------------------
// Catalog queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns every registered package id, in scan order (bundled packages first).
 */
QStringList UI::WidgetExtensions::ids() const
{
  return m_order;
}

/**
 * @brief Returns whether a package with @p id validated and is registered.
 */
bool UI::WidgetExtensions::contains(const QString& id) const
{
  return m_descriptors.contains(id);
}

/**
 * @brief Returns the registered package ids that attach to @p scope.
 */
QStringList UI::WidgetExtensions::idsForScope(Scope scope) const
{
  QStringList list;
  for (const auto& id : m_order) {
    const auto it = m_descriptors.constFind(id);
    if (it != m_descriptors.constEnd() && it->scope == scope)
      list.append(id);
  }

  return list;
}

/**
 * @brief Returns the descriptor for @p id, or an invalid descriptor when nothing is registered.
 */
const UI::WidgetExtensions::Descriptor& UI::WidgetExtensions::descriptor(const QString& id) const
{
  static const Descriptor invalid;

  const auto it = m_descriptors.constFind(id);
  return it != m_descriptors.constEnd() ? it.value() : invalid;
}

/**
 * @brief Returns the findings collected by the last scan, for the problem-center checker.
 */
const QList<Finding>& UI::WidgetExtensions::findings() const noexcept
{
  return m_findings;
}

/**
 * @brief Returns the bundled package that ships as the implementation of @p builtinWidgetId, or an
 *        empty string when the built-in keeps its compiled implementation.
 */
QString UI::WidgetExtensions::builtinReplacement(const QString& builtinWidgetId) const
{
  return m_replacements.value(builtinWidgetId);
}

/**
 * @brief Returns whether the entity shape matches what @p id declared it accepts; the picker and
 *        the host both refuse a package for an entity it never claimed to render.
 */
bool UI::WidgetExtensions::acceptsEntity(const QString& id,
                                         Scope scope,
                                         int datasetCount,
                                         bool numericValues) const
{
  const auto& package = descriptor(id);
  if (!package.isValid() || package.scope != scope)
    return false;

  if (datasetCount < package.accepts.minDatasets)
    return false;

  if (package.accepts.maxDatasets >= 0 && datasetCount > package.accepts.maxDatasets)
    return false;

  if (package.accepts.value == NumericValue && !numericValues)
    return false;

  return !(package.accepts.value == StringValue && numericValues);
}

//--------------------------------------------------------------------------------------------------
// Consent gate
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether @p id must be consented to before it runs: packages shipped inside the
 *        application are exempt, everything the user installed is not.
 */
bool UI::WidgetExtensions::consentRequired(const QString& id) const
{
  const auto& package = descriptor(id);
  return package.isValid() && !package.bundled;
}

/**
 * @brief Returns whether the user accepted this exact package version. The decision is stored per
 *        id and per version, so an update re-asks instead of inheriting the previous answer.
 */
bool UI::WidgetExtensions::consentGranted(const QString& id) const
{
  if (!consentRequired(id))
    return descriptor(id).isValid();

  const auto approved = m_settings.value(kConsentPrefix + id).toString();
  return !approved.isEmpty() && approved == descriptor(id).version;
}

/**
 * @brief Returns whether the user refused this exact package version. A refusal is remembered like
 *        an acceptance is, so the package stays installed and inert across restarts instead of
 *        re-asking on every dashboard rebuild; a package update asks again.
 */
bool UI::WidgetExtensions::consentDeclined(const QString& id) const
{
  if (!consentRequired(id))
    return false;

  const auto refused = m_settings.value(kDeclinePrefix + id).toString();
  return !refused.isEmpty() && refused == descriptor(id).version;
}

/**
 * @brief Returns whether the widget may execute: it must be registered, and either bundled with the
 *        application or explicitly consented to at its current version. Default-deny is the point.
 *        Extension QML runs with the application's own privileges; consent, not this gate, is what
 *        makes that honest.
 */
bool UI::WidgetExtensions::canInstantiate(const QString& id) const
{
  return descriptor(id).isValid() && consentGranted(id);
}

/**
 * @brief Asks the user to decide about @p id, once per session and only while the answer is still
 *        open: an accepted, refused, or bundled package never prompts. The dialog lives in QML, so
 *        this only announces the request.
 */
void UI::WidgetExtensions::requestConsent(const QString& id)
{
  if (!consentRequired(id) || consentGranted(id) || consentDeclined(id))
    return;

  if (m_prompted.contains(id))
    return;

  m_prompted.insert(id);
  Q_EMIT consentRequested(id);
}

/**
 * @brief Records the user's acceptance of the installed version of @p id.
 */
void UI::WidgetExtensions::grantConsent(const QString& id)
{
  if (!consentRequired(id))
    return;

  m_settings.remove(kDeclinePrefix + id);
  m_settings.setValue(kConsentPrefix + id, descriptor(id).version);
  announceCatalog();
}

/**
 * @brief Records the user's refusal of the installed version of @p id, which leaves the package
 *        installed and inert until it is updated or the decision is revoked.
 */
void UI::WidgetExtensions::declineConsent(const QString& id)
{
  if (!consentRequired(id))
    return;

  m_settings.remove(kConsentPrefix + id);
  m_settings.setValue(kDeclinePrefix + id, descriptor(id).version);
  announceCatalog();
}

/**
 * @brief Records that a package failed to load on the dashboard, so the problem center explains a
 *        placeholder tile instead of leaving the user with a blank widget. The finding is dropped
 *        on the next scan, which is when a reinstall or an update would fix it.
 */
void UI::WidgetExtensions::reportLoadFailure(const QString& id, const QString& error)
{
  const auto title   = descriptor(id).title.isEmpty() ? id : descriptor(id).title;
  const auto finding = makePackageFinding(
    Misc::ProblemCenter::Error,
    QStringLiteral("widget-load-failed"),
    trPackageProblem("Widget extension failed to load"),
    trPackageProblem("The widget \"%1\" could not be created: %2").arg(title, error),
    trPackageProblem("Update or reinstall the package, then reload the project."));

  if (m_findings.contains(finding))
    return;

  m_findings.append(finding);

  static auto& center = Misc::ProblemCenter::instance();
  center.runNow();
}

/**
 * @brief Drops a previously recorded decision, so the next instantiation attempt asks again.
 */
void UI::WidgetExtensions::revokeConsent(const QString& id)
{
  m_prompted.remove(id);
  m_settings.remove(kConsentPrefix + id);
  m_settings.remove(kDeclinePrefix + id);
  announceCatalog();
}

//--------------------------------------------------------------------------------------------------
// Resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the URL of the package's QML entry, or an empty string when the package is not
 *        registered or has no recorded consent. Every instantiation path resolves its file here, so
 *        this is the single choke point the consent gate needs.
 */
QString UI::WidgetExtensions::qmlUrl(const QString& id) const
{
  if (!canInstantiate(id))
    return {};

  const auto& package = descriptor(id);
  const auto path     = package.directory + QStringLiteral("/") + package.qmlFile;

  if (package.bundled)
    return QStringLiteral("qrc") + path;

  return QUrl::fromLocalFile(path).toString();
}

/**
 * @brief Returns the package's declared settings as plain maps, in declaration order, for the
 *        generic configuration form. Metadata only: no consent is needed to read a declaration.
 */
QVariantList UI::WidgetExtensions::configProperties(const QString& id) const
{
  QVariantList list;
  const auto& package = descriptor(id);

  for (const auto& property : package.config) {
    QVariantMap entry;
    entry[QStringLiteral("id")]          = property.id;
    entry[QStringLiteral("type")]        = property.type;
    entry[QStringLiteral("label")]       = property.label;
    entry[QStringLiteral("description")] = property.description;
    entry[QStringLiteral("default")]     = property.defaultValue;
    entry[QStringLiteral("min")]         = property.minimum;
    entry[QStringLiteral("max")]         = property.maximum;
    entry[QStringLiteral("options")]     = property.options;
    list.append(entry);
  }

  return list;
}

/**
 * @brief Returns the artwork a package declares, either as an icon-registry id ("category/name")
 *        or as a file inside the package. Empty when it declares none, which leaves the caller on
 *        the built-in fallback artwork.
 */
QString UI::WidgetExtensions::iconUrl(const QString& id, const int px) const
{
  const auto& package = descriptor(id);
  if (!package.isValid() || package.iconId.isEmpty())
    return {};

  auto& icons = Core::services().iconRegistry;
  if (!package.iconId.contains(QStringLiteral(".")))
    return icons.iconById(package.iconId, px);

  const auto path = package.directory + QStringLiteral("/") + package.iconId;
  if (package.bundled)
    return QStringLiteral("qrc") + path;

  return QUrl::fromLocalFile(path).toString();
}

/**
 * @brief Returns the package facts the consent dialog states before a widget runs for the first
 *        time: what it is, who published it, and where it came from.
 */
QVariantMap UI::WidgetExtensions::packageInfo(const QString& id) const
{
  const auto& package = descriptor(id);

  QVariantMap map;
  map[QStringLiteral("id")]           = package.id;
  map[QStringLiteral("title")]        = package.title;
  map[QStringLiteral("author")]       = package.author;
  map[QStringLiteral("version")]      = package.version;
  map[QStringLiteral("license")]      = package.license;
  map[QStringLiteral("description")]  = package.description;
  map[QStringLiteral("path")]         = package.directory;
  map[QStringLiteral("width")]        = package.defaultWidth;
  map[QStringLiteral("height")]       = package.defaultHeight;
  map[QStringLiteral("bundled")]      = package.bundled;
  map[QStringLiteral("experimental")] = package.experimental;
  map[QStringLiteral("consented")]    = consentGranted(id);
  return map;
}

//--------------------------------------------------------------------------------------------------
// Scanning
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the catalog: bundled packages first, then the ones installed under the workspace.
 *        Metadata is read eagerly and validated here; no QML is compiled or instantiated, which is
 *        what keeps ten unused packages free at startup.
 */
void UI::WidgetExtensions::rescan()
{
  m_order.clear();
  m_prompted.clear();
  m_findings.clear();
  m_descriptors.clear();
  m_replacements.clear();

  scanDirectory(kBundledRoot, true);

  auto& workspaceManager = Core::services().workspaceManager;
  scanDirectory(workspaceManager.path(QStringLiteral("Extensions/widget")), false);

  resolveDependencies();

  announceCatalog();
}

/**
 * @brief Retains the catalog on the bus for the pipeline's widget resolver and the property
 *        option providers (id, scope ordinal, title, replaced builtin), then notifies the editor.
 */
void UI::WidgetExtensions::announceCatalog()
{
  if (m_bus) {
    QVector<Core::Bus::WidgetExtensionEntry> entries;
    entries.reserve(m_order.size());
    for (const auto& id : m_order) {
      const auto& package = descriptor(id);
      entries.append({id, static_cast<int>(package.scope), package.title, package.replaces});
    }

    m_bus->publishState<Core::Bus::WidgetExtensionCatalog>(std::move(entries));
  }

  Q_EMIT catalogChanged();
}

/**
 * @brief Walks one package root, loading every immediate subdirectory that holds an info.json.
 */
void UI::WidgetExtensions::scanDirectory(const QString& root, bool bundled)
{
  QDir dir(root);
  if (!dir.exists())
    return;

  const auto packages = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const auto& package : packages) {
    if (m_order.count() >= kMaxPackages)
      return;

    loadPackage(root + QStringLiteral("/") + package, bundled);
  }
}

/**
 * @brief Reads and validates one package directory, registering it or recording why it was refused.
 *        A package installed from disk may not shadow a bundled id: the bundled copy cannot be
 *        deleted or corrupted, which is why it wins.
 */
void UI::WidgetExtensions::loadPackage(const QString& directory, bool bundled)
{
  QFile file(directory + QStringLiteral("/info.json"));
  if (!file.exists())
    return;

  if (!file.open(QFile::ReadOnly)) {
    m_findings.append(makePackageFinding(
      Misc::ProblemCenter::Error,
      QStringLiteral("widget-manifest-unreadable"),
      trPackageProblem("Widget extension manifest cannot be read"),
      trPackageProblem("The manifest in \"%1\" could not be opened.").arg(directory),
      trPackageProblem("Check the file permissions of the package folder, then reinstall "
                       "the extension.")));
    return;
  }

  Misc::JsonValidator::Limits limits;
  limits.maxFileSize = kMaxManifest;

  const auto parsed = Misc::JsonValidator::parseAndValidate(file.readAll(), limits);
  if (!parsed.valid || !parsed.document.isObject()) {
    m_findings.append(makePackageFinding(
      Misc::ProblemCenter::Error,
      QStringLiteral("widget-manifest-invalid"),
      trPackageProblem("Widget extension manifest is not valid JSON"),
      trPackageProblem("The manifest in \"%1\" could not be parsed: %2")
        .arg(directory, parsed.errorMessage),
      trPackageProblem("Reinstall the package; if the problem persists, report it to its "
                       "author.")));
    return;
  }

  const WidgetManifestParser parser(hostApiVersion(), reservedIds());
  auto result = parser.parse(parsed.document.object(), directory, bundled);
  m_findings.append(result.findings);
  if (!result.ok)
    return;

  const auto& id = result.descriptor.id;
  if (m_descriptors.contains(id)) {
    m_findings.append(makePackageFinding(
      Misc::ProblemCenter::Warning,
      QStringLiteral("widget-package-shadowed"),
      trPackageProblem("Widget extension is already registered"),
      trPackageProblem("A package with the identifier \"%1\" is already loaded, so the copy "
                       "in \"%2\" was ignored.")
        .arg(id, directory),
      trPackageProblem("Uninstall the duplicate copy from the workspace extensions "
                       "folder.")));
    return;
  }

  if (!result.descriptor.replaces.isEmpty())
    m_replacements.insert(result.descriptor.replaces, id);

  m_order.append(id);
  m_descriptors.insert(id, result.descriptor);
}

/**
 * @brief Resolves declared dependencies against the freshly scanned catalog: a package whose
 *        required dependency is missing or version-incompatible is dropped (and reported), while a
 *        missing optional dependency only degrades the widget.
 */
void UI::WidgetExtensions::resolveDependencies()
{
  QStringList dropped;

  for (const auto& id : std::as_const(m_order)) {
    const auto& package = m_descriptors.value(id);
    for (const auto& dependency : package.dependencies) {
      const bool present = m_descriptors.contains(dependency.id);
      const bool usable  = present
                       && WidgetManifestParser::versionInRange(
                            m_descriptors.value(dependency.id).version, dependency.versionRange);
      if (usable)
        continue;

      const auto severity =
        dependency.required ? Misc::ProblemCenter::Error : Misc::ProblemCenter::Info;
      const auto code  = dependency.required ? QStringLiteral("widget-dependency-missing")
                                             : QStringLiteral("widget-dependency-optional");
      const auto title = dependency.required
                         ? trPackageProblem("Widget extension is missing a required extension")
                         : trPackageProblem("Widget extension is missing an optional extension");

      m_findings.append(makePackageFinding(
        severity,
        code,
        title,
        trPackageProblem("Package \"%1\" depends on \"%2\" %3, which is not installed or is "
                         "a different version.")
          .arg(id, dependency.id, dependency.versionRange),
        trPackageProblem("Install \"%1\" from the extension manager.").arg(dependency.id)));

      if (dependency.required)
        dropped.append(id);
    }
  }

  for (const auto& id : std::as_const(dropped)) {
    m_order.removeAll(id);
    const auto replaces = m_descriptors.value(id).replaces;
    if (!replaces.isEmpty())
      m_replacements.remove(replaces);

    m_descriptors.remove(id);
  }
}
