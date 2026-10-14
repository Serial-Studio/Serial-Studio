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

#  include "Misc/DeploymentPackager.h"

#  include <algorithm>
#  include <QBuffer>
#  include <QDir>
#  include <QDirIterator>
#  include <QFile>
#  include <QFileInfo>
#  include <QImage>
#  include <QJsonDocument>
#  include <QRegularExpression>
#  include <QSaveFile>
#  include <QSet>
#  include <QUrl>

#  include "API/HandlerContext.h"
#  include "Core/AppInfo.h"
#  include "Core/DataModel/FrameKeys.h"
#  include "Core/Deploy/PackageFormat.h"
#  include "Core/License.h"
#  include "Core/Services.h"
#  include "Core/SSAssert.h"
#  include "Core/WorkspaceManager.h"
#  include "DataModel/PipelineModules.h"
#  include "DataModel/ProjectModel.h"
#  include "IO/ConnectionManager.h"
#  include "MQTT/Publisher.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr QLatin1StringView kProjectDir{"project/"};
static constexpr QLatin1StringView kExternalDir{"external/"};
static constexpr QLatin1StringView kCertDir{"certs/"};
static constexpr QLatin1StringView kExtensionDir{"extensions/"};
static constexpr QLatin1StringView kBundledExtensions{":/extensions/widget/"};
static constexpr int kMaxExtensionFiles = 256;
static constexpr int kMaxJsonDepth      = 64;
static constexpr int kIconPixels        = 256;

static const QStringList kCertificateKeys = {QStringLiteral("clientCertificatePath"),
                                             QStringLiteral("userCertificatePath"),
                                             QStringLiteral("clientCertPath")};
static const QStringList kPrivateKeyKeys  = {QStringLiteral("privateKeyPath"),
                                             QStringLiteral("userKeyPath")};
static const QStringList kExternalKeys    = {
  QStringLiteral("executable"), QStringLiteral("workingDir"), QStringLiteral("pipePath")};
static const QStringList kPublisherSecrets = {
  QStringLiteral("username"), QStringLiteral("password"), QStringLiteral("keyPassphrase")};

static const QRegularExpression kImageLiteral(
  QStringLiteral(R"((['"`])([^'"`\n]{1,512}\.(?:png|jpe?g|svg|bmp|gif|webp))\1)"),
  QRegularExpression::CaseInsensitiveOption);
static const QRegularExpression kExtensionToken(
  QStringLiteral(R"(^ext:([A-Za-z0-9][A-Za-z0-9._-]{0,63})$)"));

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The mounted path a carried file is read from inside a sealed session.
 */
[[nodiscard]] static QString mountPath(const QString& carried)
{
  return QString(Deploy::kMountRoot) + QLatin1Char('/') + carried;
}

/**
 * @brief A dialog's file:// URL or plain path, as a local path.
 */
[[nodiscard]] static QString localPath(const QString& urlOrPath)
{
  const QUrl url(urlOrPath);
  return url.isLocalFile() ? url.toLocalFile() : urlOrPath;
}

/**
 * @brief Collects every "ext:<id>" token in the document, with a depth cap (Power of Ten).
 */
static void collectExtensionIds(const QJsonValue& value, QSet<QString>& ids, const int depth)
{
  if (depth > kMaxJsonDepth)
    return;

  if (value.isString()) {
    const auto match = kExtensionToken.match(value.toString());
    if (match.hasMatch())
      ids.insert(match.captured(1));

    return;
  }

  if (value.isArray()) {
    for (const auto& item : value.toArray())
      collectExtensionIds(item, ids, depth + 1);

    return;
  }

  const auto object = value.toObject();
  for (auto it = object.begin(); it != object.end(); ++it)
    collectExtensionIds(it.value(), ids, depth + 1);
}

/**
 * @brief The icon as a 256 px PNG, or empty when the file cannot be read as an image.
 */
[[nodiscard]] static QByteArray iconBytes(const QString& path)
{
  const QImage image(localPath(path));
  if (image.isNull())
    return {};

  QByteArray bytes;
  QBuffer buffer(&bytes);
  buffer.open(QIODevice::WriteOnly);
  (void)image.scaled(kIconPixels, kIconPixels, Qt::KeepAspectRatio, Qt::SmoothTransformation)
    .save(&buffer, "PNG");
  return bytes;
}

/**
 * @brief The runtime options the Deploy dialog sent, kept only when they have the expected type.
 */
[[nodiscard]] static QJsonObject runtimeOptions(const QVariantMap& options)
{
  using namespace Deploy::OptionKey;
  QJsonObject out;
  for (const auto key : {kFullscreen,
                         kActionsPanel,
                         kFileTransmission,
                         kCsvExport,
                         kMdfExport,
                         kSessionExport,
                         kConsoleExport,
                         kTaskbarSearch})
    if (options.contains(QString(key)))
      out.insert(key, options.value(QString(key)).toBool());

  for (const auto key : {kTheme, kTaskbarMode})
    if (options.contains(QString(key)))
      out.insert(key, options.value(QString(key)).toString());

  if (options.contains(QString(kTaskbarButtons)))
    out.insert(kTaskbarButtons,
               QJsonArray::fromStringList(options.value(QString(kTaskbarButtons)).toStringList()));

  return out;
}

//--------------------------------------------------------------------------------------------------
// Construction & accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty packager; refresh() scans the open project.
 */
Misc::DeploymentPackager::DeploymentPackager(QObject* parent) : QObject(parent), m_estimatedSize(0)
{}

/**
 * @brief Credentials and private keys as {label, isKey, included} rows, in scan order.
 */
QVariantList Misc::DeploymentPackager::secrets() const
{
  QVariantList rows;
  for (const auto& secret : m_secrets)
    rows.append(QVariantMap{
      {   QStringLiteral("label"),    secret.label},
      {   QStringLiteral("isKey"),    secret.isKey},
      {QStringLiteral("included"), secret.included}
    });

  return rows;
}

/**
 * @brief What the package depends on but cannot carry: programs, folders, pipes, web pages.
 */
QStringList Misc::DeploymentPackager::externals() const
{
  return m_externals;
}

/**
 * @brief The project document plus every carried file, before encoding overhead.
 */
qint64 Misc::DeploymentPackager::estimatedSize() const
{
  return m_estimatedSize;
}

/**
 * @brief The folder of the open project file; empty for an unsaved project.
 */
QString Misc::DeploymentPackager::projectFolder() const
{
  return m_folder;
}

/**
 * @brief References that point at files that do not exist; export refuses while any remain (R2).
 */
QStringList Misc::DeploymentPackager::missingFiles() const
{
  return m_missing;
}

/**
 * @brief Carried files as {path, source, size, extra} rows, sorted by their path inside the
 *        package; extra marks a file the author added by hand.
 */
QVariantList Misc::DeploymentPackager::carriedFiles() const
{
  QVariantList rows;
  for (auto it = m_files.begin(); it != m_files.end(); ++it)
    rows.append(QVariantMap{
      {  QStringLiteral("path"),                          it.key()},
      {QStringLiteral("source"),                        it.value()},
      {  QStringLiteral("size"),      QFileInfo(it.value()).size()},
      { QStringLiteral("extra"), m_extraFiles.contains(it.value())}
    });

  return rows;
}

/**
 * @brief Whether any credential or key is marked for inclusion, which shows the warning.
 */
bool Misc::DeploymentPackager::anySecretIncluded() const
{
  return std::any_of(
    m_secrets.begin(), m_secrets.end(), [](const Secret& secret) { return secret.included; });
}

//--------------------------------------------------------------------------------------------------
// Scanning
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rescans the open project. The author's include choices survive a rescan of the same
 *        project; a different project starts with everything excluded and no added files.
 */
void Misc::DeploymentPackager::refresh()
{
  auto& projectModel = DataModel::pipelineModules().projectModel;
  const auto path    = projectModel.jsonFilePath();

  QSet<QString> included;
  if (path == m_projectPath) {
    for (const auto& secret : std::as_const(m_secrets))
      if (secret.included)
        included.insert(secretIdentity(secret));
  }

  m_projectPath = path;
  m_project     = projectModel.serializeToJson();
  m_folder      = path.isEmpty() ? QString() : QFileInfo(path).absolutePath();
  m_files.clear();
  m_missing.clear();
  m_secrets.clear();
  m_externals.clear();
  m_carriedBySource.clear();
  dropStaleExtraFiles();

  collectSounds();
  collectCanvasImages();
  collectExtensions();
  collectCertificates();
  collectSecrets();
  collectExternals();

  for (auto& secret : m_secrets)
    secret.included = included.contains(secretIdentity(secret));

  m_estimatedSize = measure();
  Q_EMIT contentsChanged();
  Q_EMIT secretsChanged();
  Q_EMIT inclusionChanged();
}

/**
 * @brief Forgets author-added files that left the project folder or no longer exist, so a stale
 *        addition can neither block export nor ride into another project's package.
 */
void Misc::DeploymentPackager::dropStaleExtraFiles()
{
  const auto folder = QFileInfo(m_folder).canonicalFilePath() + QLatin1Char('/');
  m_extraFiles.removeIf([&folder](const QString& path) {
    return folder.size() <= 1 || !path.startsWith(folder) || !QFileInfo::exists(path);
  });
}

/**
 * @brief The project document plus every carried file, before encoding overhead.
 */
qint64 Misc::DeploymentPackager::measure() const
{
  qint64 total = QJsonDocument(m_project).toJson(QJsonDocument::Compact).size();
  for (const auto& source : m_files)
    total += QFileInfo(source).size();

  return total;
}

/**
 * @brief A secret's identity across rescans: its target and property, or its key slot and file.
 */
QString Misc::DeploymentPackager::secretIdentity(const Secret& secret)
{
  if (secret.isKey)
    return QStringLiteral("key|%1|%2|%3")
      .arg(QString::number(secret.sourceIndex), secret.jsonKey, secret.keyFile);

  return QStringLiteral("credential|%1|%2").arg(secret.target, secret.property);
}

/**
 * @brief Accepts a path inside the package only when the codec will: otherwise records why the
 *        file cannot be carried, before the author ever presses Save.
 */
bool Misc::DeploymentPackager::admitCarriedPath(const QString& carried, const QString& source)
{
  if (Deploy::isSafeEntryPath(carried))
    return true;

  m_missing.append(tr("%1 (its name cannot be carried in a package)").arg(source));
  return false;
}

/**
 * @brief Adds Canvas images the code builds at run time; only files inside the project folder,
 *        because that is the only place a sealed Canvas resolves images from.
 */
void Misc::DeploymentPackager::addExtraFiles(const QStringList& urls)
{
  const auto folder = QFileInfo(m_folder).canonicalFilePath();
  for (const auto& url : urls) {
    const auto path = QFileInfo(localPath(url)).canonicalFilePath();
    if (!folder.isEmpty() && path.startsWith(folder + QLatin1Char('/'))
        && !m_extraFiles.contains(path))
      m_extraFiles.append(path);
  }

  refresh();
}

/**
 * @brief Drops an author-added file again.
 */
void Misc::DeploymentPackager::removeExtraFile(const QString& carriedPath)
{
  m_extraFiles.removeAll(m_files.value(carriedPath));
  refresh();
}

/**
 * @brief Marks one credential or private key for inclusion or exclusion.
 */
void Misc::DeploymentPackager::setSecretIncluded(const int index, const bool included)
{
  if (index < 0 || index >= m_secrets.size() || m_secrets[index].included == included)
    return;

  m_secrets[index].included = included;
  Q_EMIT inclusionChanged();
}

/**
 * @brief Copies one referenced file into the package: under project/ when it sits in the project
 *        folder, under external/ otherwise. Returns its path inside the package, or empty after
 *        recording the reference as missing.
 */
QString Misc::DeploymentPackager::carryFile(const QString& reference)
{
  const bool relative = !QDir::isAbsolutePath(reference);
  const QString candidate =
    relative && !m_folder.isEmpty() ? QDir(m_folder).absoluteFilePath(reference) : reference;
  const QFileInfo info(candidate);
  if (!info.isFile()) {
    if (!m_missing.contains(reference))
      m_missing.append(reference);

    return {};
  }

  const auto canonical = info.canonicalFilePath();
  if (m_carriedBySource.contains(canonical))
    return m_carriedBySource.value(canonical);

  const auto folder  = QFileInfo(m_folder).canonicalFilePath();
  const auto inside  = !folder.isEmpty() && canonical.startsWith(folder + QLatin1Char('/'));
  const auto carried = inside ? kProjectDir + canonical.mid(folder.size() + 1)
                              : kExternalDir + QString::number(m_carriedBySource.size())
                                  + QLatin1Char('-') + info.fileName();

  if (!admitCarriedPath(carried, reference))
    return {};

  m_files.insert(carried, canonical);
  m_carriedBySource.insert(canonical, carried);
  return carried;
}

/**
 * @brief Copies a certificate into certs/ and returns its mounted path, or empty when missing.
 */
QString Misc::DeploymentPackager::carryCertificate(const QString& path)
{
  const QFileInfo info(path);
  if (!info.isFile()) {
    m_missing.append(path);
    return {};
  }

  const auto carried =
    kCertDir + QString::number(m_files.size()) + QLatin1Char('-') + info.fileName();
  if (!admitCarriedPath(carried, path))
    return {};

  m_files.insert(carried, info.canonicalFilePath());
  return mountPath(carried);
}

/**
 * @brief Rewrites the sound path of each alarm band to its carried copy.
 */
QJsonArray Misc::DeploymentPackager::rewriteBands(QJsonArray bands)
{
  for (qsizetype i = 0; i < bands.size(); ++i) {
    auto band        = bands[i].toObject();
    const auto sound = band.value(Keys::Sound).toString();
    if (sound.isEmpty() || sound.startsWith(QLatin1Char(':')))
      continue;

    const auto carried = carryFile(sound);
    band.insert(Keys::Sound, carried.isEmpty() ? sound : mountPath(carried));
    bands[i] = band;
  }

  return bands;
}

/**
 * @brief Carries every alarm sound: per-band sounds and the project's channel sound map.
 */
void Misc::DeploymentPackager::collectSounds()
{
  auto groups = m_project.value(Keys::Groups).toArray();
  for (qsizetype g = 0; g < groups.size(); ++g) {
    auto group    = groups[g].toObject();
    auto datasets = group.value(Keys::Datasets).toArray();
    for (qsizetype d = 0; d < datasets.size(); ++d) {
      auto dataset = datasets[d].toObject();
      if (dataset.contains(Keys::AlarmBands))
        dataset.insert(Keys::AlarmBands, rewriteBands(dataset.value(Keys::AlarmBands).toArray()));

      datasets[d] = dataset;
    }

    group.insert(Keys::Datasets, datasets);
    groups[g] = group;
  }

  if (m_project.contains(Keys::Groups))
    m_project.insert(Keys::Groups, groups);

  auto sounds   = m_project.value(Keys::Sounds).toObject();
  auto channels = sounds.value(Keys::Channels).toObject();
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    auto entry = it.value().toObject();
    for (const auto key : {Keys::SoundWarning, Keys::SoundCaution, Keys::SoundAdvisory}) {
      const auto sound = entry.value(key).toString();
      const auto carried =
        sound.isEmpty() || sound.startsWith(QLatin1Char(':')) ? QString() : carryFile(sound);
      if (!carried.isEmpty())
        entry.insert(key, mountPath(carried));
    }

    it.value() = entry;
  }

  if (!channels.isEmpty()) {
    sounds.insert(Keys::Channels, channels);
    m_project.insert(Keys::Sounds, sounds);
  }
}

/**
 * @brief Carries image files named by string literals in Canvas code, plus author-added files.
 */
void Misc::DeploymentPackager::collectCanvasImages()
{
  const auto groups = m_project.value(Keys::Groups).toArray();
  for (const auto& value : groups) {
    auto matches = kImageLiteral.globalMatch(value.toObject().value(Keys::PainterCode).toString());
    while (matches.hasNext()) {
      const auto reference = matches.next().captured(2);
      if (reference.startsWith(QLatin1Char(':')) || reference.contains(QLatin1String(":/")))
        continue;

      (void)carryFile(reference);
    }
  }

  for (const auto& path : std::as_const(m_extraFiles))
    (void)carryFile(path);
}

/**
 * @brief Carries every installed widget extension the project uses; ones the app bundles stay.
 */
void Misc::DeploymentPackager::collectExtensions()
{
  QSet<QString> ids;
  collectExtensionIds(m_project, ids, 0);

  const auto root = Core::services().workspaceManager.path(QStringLiteral("Extensions/widget"));
  for (const auto& id : std::as_const(ids)) {
    if (QFileInfo::exists(kBundledExtensions + id))
      continue;

    const QDir dir(root + QLatin1Char('/') + id);
    if (!dir.exists()) {
      m_missing.append(tr("Widget extension \"%1\"").arg(id));
      continue;
    }

    carryExtension(id, dir);
  }
}

/**
 * @brief Carries one extension folder; more files than the cap, or a name the codec refuses, is
 *        a named problem rather than a silently incomplete extension (R2).
 */
void Misc::DeploymentPackager::carryExtension(const QString& id, const QDir& dir)
{
  int count = 0;
  QDirIterator it(dir.path(), QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const auto file    = it.next();
    const auto carried = kExtensionDir + id + QLatin1Char('/') + dir.relativeFilePath(file);
    if (++count > kMaxExtensionFiles) {
      m_missing.append(tr("Widget extension \"%1\" has more than %2 files")
                         .arg(id, QString::number(kMaxExtensionFiles)));
      return;
    }

    if (admitCarriedPath(carried, file))
      m_files.insert(carried, file);
  }
}

/**
 * @brief Carries client certificates and lists private keys as secrets the author may include.
 */
void Misc::DeploymentPackager::collectCertificates()
{
  auto sources = m_project.value(Keys::Sources).toArray();
  for (qsizetype i = 0; i < sources.size(); ++i) {
    auto source     = sources[i].toObject();
    auto connection = source.value(Keys::SourceConn).toObject();
    for (const auto& key : kCertificateKeys)
      if (!connection.value(key).toString().isEmpty())
        connection.insert(key, carryCertificate(connection.value(key).toString()));

    for (const auto& key : kPrivateKeyKeys)
      notePrivateKey(static_cast<int>(i),
                     tr("%1: private key").arg(source.value(Keys::Title).toString()),
                     key,
                     connection.value(key).toString());

    source.insert(Keys::SourceConn, connection);
    sources[i] = source;
  }

  if (m_project.contains(Keys::Sources))
    m_project.insert(Keys::Sources, sources);

  auto publisher  = m_project.value(Keys::MqttPublisher).toObject();
  const auto cert = publisher.value(QStringLiteral("clientCertPath")).toString();
  if (!cert.isEmpty()) {
    publisher.insert(QStringLiteral("clientCertPath"), carryCertificate(cert));
    m_project.insert(Keys::MqttPublisher, publisher);
  }

  notePrivateKey(-1,
                 tr("MQTT publisher: private key"),
                 QStringLiteral("privateKeyPath"),
                 publisher.value(QStringLiteral("privateKeyPath")).toString());
}

/**
 * @brief Records a private key as a secret the author may include, or as missing when its file
 *        is gone; an empty path is no key at all.
 */
void Misc::DeploymentPackager::notePrivateKey(const int sourceIndex,
                                              const QString& label,
                                              const QString& jsonKey,
                                              const QString& path)
{
  if (path.isEmpty())
    return;

  if (!QFileInfo(path).isFile()) {
    m_missing.append(path);
    return;
  }

  m_secrets.append(Secret{.isKey       = true,
                          .sourceIndex = sourceIndex,
                          .label       = label,
                          .jsonKey     = jsonKey,
                          .keyFile     = path});
}

/**
 * @brief Lists every credential the drivers and the MQTT publisher hold for this project.
 */
void Misc::DeploymentPackager::collectSecrets()
{
  auto sources = m_project.value(Keys::Sources).toArray();
  for (qsizetype i = 0; i < sources.size(); ++i) {
    auto source = sources[i].toObject();
    collectCredentials(static_cast<int>(i), source);
  }

  if (!m_project.contains(Keys::MqttPublisher))
    return;

  const auto& publisher = API::handlerContext().mqttPublisher;
  for (const auto& name : kPublisherSecrets) {
    const auto value = publisher.property(qPrintable(name)).toString();
    if (!value.isEmpty())
      m_secrets.append(Secret{.label    = tr("MQTT publisher: %1").arg(name),
                              .value    = value,
                              .target   = QString(Deploy::CredentialTarget::kPublisher),
                              .property = name});
  }
}

/**
 * @brief Lists one source's Password-typed driver properties and strips them from the packaged
 *        connection settings, so an excluded credential never reaches the package (R4).
 */
void Misc::DeploymentPackager::collectCredentials(const int sourceIndex, const QJsonObject& source)
{
  const int sourceId = source.value(Keys::SourceId).toInt(sourceIndex);
  auto* driver       = API::handlerContext().connectionManager.driverForEditing(sourceId);
  if (!driver)
    return;

  auto connection  = source.value(Keys::SourceConn).toObject();
  const auto title = source.value(Keys::Title).toString();
  for (const auto& property : driver->driverProperties()) {
    if (property.type != IO::DriverProperty::Password)
      continue;

    connection.remove(property.key);
    const auto value = property.value.toString();
    if (!value.isEmpty())
      m_secrets.append(
        Secret{.sourceIndex = sourceIndex,
               .label       = tr("%1: %2").arg(title, property.label),
               .value       = value,
               .target      = Deploy::CredentialTarget::kSourcePrefix + QString::number(sourceId),
               .property    = property.key});
  }

  auto sources  = m_project.value(Keys::Sources).toArray();
  auto stripped = source;
  stripped.insert(Keys::SourceConn, connection);
  sources[sourceIndex] = stripped;
  m_project.insert(Keys::Sources, sources);
}

/**
 * @brief Lists what the package depends on but cannot carry (R3).
 */
void Misc::DeploymentPackager::collectExternals()
{
  const auto sources = m_project.value(Keys::Sources).toArray();
  for (const auto& value : sources) {
    const auto connection = value.toObject().value(Keys::SourceConn).toObject();
    for (const auto& key : kExternalKeys)
      if (!connection.value(key).toString().isEmpty())
        m_externals.append(tr("Process %1: %2").arg(key, connection.value(key).toString()));
  }

  const auto groups = m_project.value(Keys::Groups).toArray();
  for (const auto& value : groups) {
    const auto url = value.toObject().value(Keys::WebViewUrl).toString();
    if (!url.isEmpty())
      m_externals.append(tr("Web page: %1").arg(url));
  }
}

//--------------------------------------------------------------------------------------------------
// Export
//--------------------------------------------------------------------------------------------------

/**
 * @brief The project as packaged: an included private key points at its carried copy, an excluded
 *        one is cleared so the operator picks it on first connect.
 */
QJsonObject Misc::DeploymentPackager::packagedProject(QMap<QString, QString>& files) const
{
  auto project   = m_project;
  auto sources   = project.value(Keys::Sources).toArray();
  auto publisher = project.value(Keys::MqttPublisher).toObject();
  for (const auto& secret : m_secrets) {
    if (!secret.isKey)
      continue;

    const auto carried = kCertDir + QString::number(files.size()) + QLatin1String("-key-")
                       + QFileInfo(secret.keyFile).fileName();
    const auto value   = secret.included ? mountPath(carried) : QString();
    if (secret.included)
      files.insert(carried, QFileInfo(secret.keyFile).canonicalFilePath());

    if (secret.sourceIndex < 0) {
      publisher.insert(secret.jsonKey, value);
      continue;
    }

    auto source     = sources[secret.sourceIndex].toObject();
    auto connection = source.value(Keys::SourceConn).toObject();
    connection.insert(secret.jsonKey, value);
    source.insert(Keys::SourceConn, connection);
    sources[secret.sourceIndex] = source;
  }

  if (project.contains(Keys::Sources))
    project.insert(Keys::Sources, sources);

  if (project.contains(Keys::MqttPublisher))
    project.insert(Keys::MqttPublisher, publisher);

  return project;
}

/**
 * @brief Assembles the package from the scan and the dialog's options; reads every carried file
 *        within the 64 MiB cap and fails on the first unreadable one.
 */
bool Misc::DeploymentPackager::buildPackage(const QVariantMap& options,
                                            Deploy::Package& package,
                                            QString& error) const
{
  const auto iconPath = options.value(QStringLiteral("iconPath")).toString();
  const bool custom   = !iconPath.isEmpty() && !iconPath.startsWith(QLatin1Char(':'))
                     && !iconPath.startsWith(QStringLiteral("qrc:"));

  QMap<QString, QString> files = m_files;
  const auto project           = packagedProject(files);

  package.minAppVersion          = QStringLiteral(APP_VERSION);
  package.writerVersion          = QStringLiteral(APP_VERSION);
  package.project                = QJsonDocument(project).toJson(QJsonDocument::Compact);
  package.manifest.title         = options.value(QStringLiteral("title")).toString().trimmed();
  package.manifest.icon          = iconBytes(iconPath);
  package.manifest.options       = runtimeOptions(options);
  package.manifest.projectFolder = m_folder;
  package.manifest.externals     = m_externals;

  if (custom && package.manifest.icon.isEmpty()) {
    error = tr("The icon \"%1\" could not be read as an image.").arg(localPath(iconPath));
    return false;
  }

  qint64 total = package.project.size();
  for (auto it = files.begin(); it != files.end(); ++it) {
    QFile file(it.value());
    total += file.size();
    if (total > Deploy::kMaxPackageBytes || !file.open(QIODevice::ReadOnly)) {
      error = tr("\"%1\" could not be read, or the package would exceed 64 MiB.").arg(it.value());
      return false;
    }

    const auto data = file.readAll();
    if (data.size() != file.size()) {
      error = tr("\"%1\" could not be read completely.").arg(it.value());
      return false;
    }

    package.files.append({it.key(), data});
  }

  for (const auto& secret : m_secrets)
    if (secret.included && !secret.isKey)
      package.credentials.append({secret.target, secret.property, secret.value});

  return true;
}

/**
 * @brief Writes the package to @p outputPath. Refused without a license or the trial, for an
 *        unsaved project, and while any referenced file is missing (R2).
 */
void Misc::DeploymentPackager::exportPackage(const QString& outputPath, const QVariantMap& options)
{
  if (!Core::License::activated()) {
    Q_EMIT exportFailed(tr("Exporting a package needs a Pro license or the trial."));
    return;
  }

  if (m_folder.isEmpty()) {
    Q_EMIT exportFailed(tr("Save the project before exporting a package."));
    return;
  }

  if (!m_missing.isEmpty()) {
    Q_EMIT exportFailed(tr("These referenced files are missing:\n%1").arg(m_missing.join('\n')));
    return;
  }

  QString error;
  Deploy::Package package;
  if (!buildPackage(options, package, error)) {
    Q_EMIT exportFailed(error);
    return;
  }

  const auto encoded = Deploy::encode(package);
  if (!encoded.ok) {
    Q_EMIT exportFailed(encoded.error);
    return;
  }

  auto path = localPath(outputPath);
  if (!path.endsWith(QStringLiteral(".ssdeploy"), Qt::CaseInsensitive))
    path += QStringLiteral(".ssdeploy");

  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly) || file.write(encoded.bytes) != encoded.bytes.size()
      || !file.commit()) {
    Q_EMIT exportFailed(tr("The package could not be written to \"%1\".").arg(path));
    return;
  }

  Q_EMIT exportFinished(path);
}

#endif  // BUILD_COMMERCIAL
