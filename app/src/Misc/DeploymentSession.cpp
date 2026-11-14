/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Misc/DeploymentSession.h"

#  include <algorithm>
#  include <QCoreApplication>
#  include <QCryptographicHash>
#  include <QDir>
#  include <QFile>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QIcon>
#  include <QJsonArray>
#  include <QMessageBox>
#  include <QPixmap>
#  include <QVersionNumber>

#  include "API/HandlerContext.h"
#  include "AppState.h"
#  include "Core/AppInfo.h"
#  include "Core/Crypto/CredentialVault.h"
#  include "Core/Deploy/PackageFormat.h"
#  include "Core/JsonValidator.h"
#  include "Core/Runtime.h"
#  include "Core/SerialStudio.h"
#  include "Core/SSAssert.h"
#  include "DataModel/PipelineModules.h"
#  include "DataModel/ProjectModel.h"
#  include "IO/ConnectionManager.h"
#  include "MQTT/Publisher.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QStringList kPublisherSecrets = {
  QStringLiteral("username"), QStringLiteral("password"), QStringLiteral("keyPassphrase")};
static constexpr int kLeftoverMinimumAgeSeconds = 60;

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether @p key is a Password-typed property of @p driver, the only kind the packager
 *        ever writes; anything else in a package is ignored.
 */
[[nodiscard]] static bool isPasswordProperty(const IO::HAL_Driver& driver, const QString& key)
{
  const auto properties = driver.driverProperties();
  return std::any_of(properties.begin(), properties.end(), [&key](const IO::DriverProperty& p) {
    return p.key == key && p.type == IO::DriverProperty::Password;
  });
}

/**
 * @brief Applies one source credential through the driver property it came from. The source id
 *        and property name come from the package, so both are checked before use.
 */
static void applySourceCredential(const Deploy::Credential& credential)
{
  const QString prefix = Deploy::CredentialTarget::kSourcePrefix;
  if (!credential.target.startsWith(prefix))
    return;

  bool ok            = false;
  const int sourceId = credential.target.mid(prefix.size()).toInt(&ok);
  if (!ok || sourceId < 0)
    return;

  auto& connections = API::handlerContext().connectionManager;
  auto* driver      = connections.driverForEditing(sourceId);
  if (!driver || !isPasswordProperty(*driver, credential.property))
    return;

  if (sourceId == 0)
    connections.setUiDriverProperty(credential.property, credential.value);
  else
    driver->setDriverProperty(credential.property, credential.value);
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a closed session.
 */
Misc::DeploymentSession::DeploymentSession() : m_open(false) {}

/**
 * @brief Destroys the session; the resource tree unregisters its mount.
 */
Misc::DeploymentSession::~DeploymentSession() = default;

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a package was opened and the process sealed.
 */
bool Misc::DeploymentSession::isOpen() const noexcept
{
  return m_open;
}

/**
 * @brief The package's title, used for the window title.
 */
const QString& Misc::DeploymentSession::title() const noexcept
{
  return m_package.manifest.title;
}

/**
 * @brief The runtime options the Deploy dialog stored in the package.
 */
Misc::RuntimeOptions Misc::DeploymentSession::runtimeOptions() const
{
  using namespace Deploy;
  const auto& stored = m_package.manifest.options;

  RuntimeOptions options;
  options.fullscreen       = stored.value(OptionKey::kFullscreen).toBool(false);
  options.actionsPanel     = stored.value(OptionKey::kActionsPanel).toBool(false);
  options.fileTransmission = stored.value(OptionKey::kFileTransmission).toBool(false);
  options.csvExport        = stored.value(OptionKey::kCsvExport).toBool(false);
  options.mdfExport        = stored.value(OptionKey::kMdfExport).toBool(false);
  options.sessionExport    = stored.value(OptionKey::kSessionExport).toBool(false);
  options.consoleExport    = stored.value(OptionKey::kConsoleExport).toBool(false);
  options.taskbarSearch    = stored.value(OptionKey::kTaskbarSearch).toBool(true);
  options.theme            = stored.value(OptionKey::kTheme).toString().trimmed();
  options.taskbarMode      = stored.value(OptionKey::kTaskbarMode).toString().toLower();

  const auto buttons = stored.value(OptionKey::kTaskbarButtons).toArray();
  for (const auto& button : buttons)
    options.taskbarButtons.append(button.toString());

  return options;
}

//--------------------------------------------------------------------------------------------------
// Opening and loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Before the composition root: reads and decodes the package within the 64 MiB cap, then
 *        mounts its files and seals the process. Any refusal shows why and returns false; nothing
 *        from a refused package is kept, and no singleton is reached. Resources are written to
 *        disk first, and only for a package whose author allowed helper programs (spec 0123).
 */
bool Misc::DeploymentSession::open(const QString& path)
{
  SS_ASSERT(!m_open, return false);

  QFile file(path);
  if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
    showRefusal(
      QCoreApplication::translate("DeploymentSession",
                                  "The deployment package \"%1\" could not be found or opened.")
        .arg(path));
    return false;
  }

  const auto bytes = file.read(Deploy::kMaxPackageBytes + 1);
  auto result      = Deploy::decode(bytes, QVersionNumber::fromString(QStringLiteral(APP_VERSION)));
  if (result.status != Deploy::DecodeStatus::Ok) {
    qWarning() << "[DeploymentSession] Refused" << path << "-" << result.detail;
    showRefusal(refusalMessage(result));
    return false;
  }

  m_package = std::move(result.package);
  Core::Runtime::SealedHelpers helpers;
  if (!prepareHelpers(path, bytes, helpers)) {
    m_package = Deploy::Package();
    return false;
  }

  const bool mounted = m_tree.mount(m_package.files, QString(Deploy::kMountId));
  m_package.files    = QList<Deploy::FileEntry>();
  if (!mounted) {
    m_package = Deploy::Package();
    m_resources.remove();
    showRefusal(refusalMessage(Deploy::DecodeResult()));
    return false;
  }

  Core::Runtime::seal(m_tree.root(), m_package.manifest.projectFolder, helpers);
  configureQmlCache();
  applyIdentity();

  m_open = true;
  return true;
}

/**
 * @brief At the CLI project step: loads the package's project as the one document this sealed
 *        process holds. The included credentials are applied one loop pass later, after the
 *        connection manager's queued source sync and device rebuild, so they land on the drivers
 *        that will actually connect.
 */
bool Misc::DeploymentSession::loadProject()
{
  SS_ASSERT(m_open, return false);

  const auto parsed = Misc::JsonValidator::parseAndValidate(m_package.project);
  auto& modules     = DataModel::pipelineModules();
  modules.appState.setOperationMode(SerialStudio::ProjectFile);
  if (!parsed.valid || !parsed.document.isObject()
      || !modules.projectModel.loadFromJsonDocument(parsed.document, QString())) {
    qWarning() << "[DeploymentSession] Project refused:" << parsed.errorMessage;
    showRefusal(refusalMessage(Deploy::DecodeResult()));
    return false;
  }

  m_package.project = QByteArray();
  QMetaObject::invokeMethod(qApp, [this] { applyCredentials(); }, Qt::QueuedConnection);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Takes the resource entries out of the carried files, so they never enter the in-memory
 *        mount, and writes them to this session's folder when the package's author allowed helper
 *        programs (spec 0123); without that option nothing is written. The identity is the digest
 *        of the package's bytes, so launch consent never carries to a changed package.
 */
bool Misc::DeploymentSession::prepareHelpers(const QString& path,
                                             const QByteArray& bytes,
                                             Core::Runtime::SealedHelpers& helpers)
{
  SS_ASSERT(!m_open, return false);
  SS_ASSERT(!bytes.isEmpty(), return false);

  QList<Deploy::FileEntry> resources;
  for (const auto& entry : std::as_const(m_package.files))
    if (Deploy::ResourceExtractor::isResourceEntry(entry.path))
      resources.append(entry);

  m_package.files.removeIf([](const Deploy::FileEntry& entry) {
    return Deploy::ResourceExtractor::isResourceEntry(entry.path);
  });

  Deploy::ResourceExtractor::sweepStale(QDir::tempPath(), kLeftoverMinimumAgeSeconds);
  if (!m_package.manifest.options.value(Deploy::OptionKey::kHelperPrograms).toBool(false))
    return true;

  QString error;
  if (!resources.isEmpty() && !m_resources.extract(resources, QDir::tempPath(), error)) {
    qWarning() << "[DeploymentSession] Resources refused:" << error;
    showRefusal(QCoreApplication::translate(
                  "DeploymentSession",
                  "The files this deployment package needs could not be written to this "
                  "computer.\n\n%1")
                  .arg(error));
    return false;
  }

  const QFileInfo info(path);
  helpers.enabled        = true;
  helpers.packagePath    = info.absoluteFilePath();
  helpers.packageFolder  = info.absolutePath();
  helpers.resourceFolder = m_resources.folder();
  helpers.identity =
    QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
  return true;
}

/**
 * @brief Gives the process the package's name and icon, before any window reads them.
 */
void Misc::DeploymentSession::applyIdentity() const
{
  if (!m_package.manifest.title.isEmpty())
    QGuiApplication::setApplicationDisplayName(m_package.manifest.title);

  QPixmap pixmap;
  if (!m_package.manifest.icon.isEmpty() && pixmap.loadFromData(m_package.manifest.icon))
    QGuiApplication::setWindowIcon(QIcon(pixmap));
}

/**
 * @brief Applies each included credential inside the vault's transient scope, so it lives in
 *        memory only and never in settings, then drops the plaintext copies.
 */
void Misc::DeploymentSession::applyCredentials()
{
  if (m_package.credentials.isEmpty())
    return;

  const MQTT::CredentialVault::TransientScope scope;
  auto& publisher = API::handlerContext().mqttPublisher;
  for (const auto& credential : std::as_const(m_package.credentials)) {
    if (credential.target != Deploy::CredentialTarget::kPublisher) {
      applySourceCredential(credential);
      continue;
    }

    if (kPublisherSecrets.contains(credential.property))
      publisher.setProperty(qPrintable(credential.property), credential.value);
  }

  m_package.credentials = QList<Deploy::Credential>();
}

/**
 * @brief Keeps the QML engine from writing compiled copies of package QML to its disk cache. Runs
 *        before the engine exists; an operator's QML_FORCE_DISK_CACHE is dropped for this process.
 */
void Misc::DeploymentSession::configureQmlCache()
{
  qunsetenv("QML_FORCE_DISK_CACHE");
  qputenv("QML_DISK_CACHE", QByteArrayLiteral("aot,qmlc-read"));
}

/**
 * @brief Tells the operator why the package will not open.
 */
void Misc::DeploymentSession::showRefusal(const QString& message)
{
  QMessageBox box;
  box.setTextFormat(Qt::PlainText);
  box.setIcon(QMessageBox::Critical);
  box.setWindowTitle(
    QCoreApplication::translate("DeploymentSession", "Cannot open deployment package"));
  box.setText(message);
  box.exec();
}

/**
 * @brief The operator-facing reason for a refused decode; a default result reads as damaged.
 */
QString Misc::DeploymentSession::refusalMessage(const Deploy::DecodeResult& result)
{
  switch (result.status) {
    case Deploy::DecodeStatus::TooLarge:
      return QCoreApplication::translate("DeploymentSession",
                                         "This deployment package is larger than 64 MiB.");
    case Deploy::DecodeStatus::NotAPackage:
      return QCoreApplication::translate("DeploymentSession",
                                         "This file is not a Serial Studio deployment package.");
    case Deploy::DecodeStatus::NeedsNewerApp:
      return QCoreApplication::translate("DeploymentSession",
                                         "This deployment package needs Serial Studio %1 or later.")
        .arg(result.requiredVersion.isEmpty() ? QStringLiteral("(newer)") : result.requiredVersion);
    case Deploy::DecodeStatus::Ok:
    case Deploy::DecodeStatus::Corrupt:
      break;
  }

  return QCoreApplication::translate(
    "DeploymentSession",
    "This deployment package is damaged or was changed after it was exported.");
}

#endif  // BUILD_COMMERCIAL
