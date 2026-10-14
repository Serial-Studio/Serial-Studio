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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QDir>
#  include <QHash>
#  include <QJsonArray>
#  include <QJsonObject>
#  include <QMap>
#  include <QObject>
#  include <QStringList>
#  include <QVariantList>
#  include <QVariantMap>

#  include "Core/Deploy/PackageCodec.h"

namespace Misc {

/**
 * @brief Builds a deployment package from the open project (spec 0096): the files it references,
 *        the external dependencies it cannot carry, and the credentials and private keys the
 *        author may include one by one. Packages are encoded, not encrypted.
 */
class DeploymentPackager : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QVariantList secrets
             READ secrets
             NOTIFY secretsChanged)
  Q_PROPERTY(QStringList externals
             READ externals
             NOTIFY contentsChanged)
  Q_PROPERTY(qint64 estimatedSize
             READ estimatedSize
             NOTIFY contentsChanged)
  Q_PROPERTY(QString projectFolder
             READ projectFolder
             NOTIFY contentsChanged)
  Q_PROPERTY(QStringList missingFiles
             READ missingFiles
             NOTIFY contentsChanged)
  Q_PROPERTY(QVariantList carriedFiles
             READ carriedFiles
             NOTIFY contentsChanged)
  Q_PROPERTY(bool anySecretIncluded
             READ anySecretIncluded
             NOTIFY inclusionChanged)
  // clang-format on

signals:
  void secretsChanged();
  void contentsChanged();
  void inclusionChanged();
  void exportFinished(const QString& path);
  void exportFailed(const QString& message);

public:
  explicit DeploymentPackager(QObject* parent = nullptr);

  [[nodiscard]] QVariantList secrets() const;
  [[nodiscard]] QStringList externals() const;
  [[nodiscard]] qint64 estimatedSize() const;
  [[nodiscard]] QString projectFolder() const;
  [[nodiscard]] QStringList missingFiles() const;
  [[nodiscard]] QVariantList carriedFiles() const;
  [[nodiscard]] bool anySecretIncluded() const;

public slots:
  void refresh();
  void addExtraFiles(const QStringList& urls);
  void removeExtraFile(const QString& carriedPath);
  void setSecretIncluded(int index, bool included);
  void exportPackage(const QString& outputPath, const QVariantMap& options);

private:
  /**
   * @brief One credential or private key the author may include.
   */
  struct Secret {
    bool isKey      = false;
    bool included   = false;
    int sourceIndex = -1;
    QString label;
    QString value;
    QString target;
    QString jsonKey;
    QString keyFile;
    QString property;
  };

  void collectSounds();
  void collectSecrets();
  void collectExternals();
  void collectExtensions();
  void collectCertificates();
  void collectCanvasImages();
  void dropStaleExtraFiles();
  void carryExtension(const QString& id, const QDir& dir);
  void collectCredentials(int sourceIndex, const QJsonObject& source);
  void notePrivateKey(int sourceIndex,
                      const QString& label,
                      const QString& jsonKey,
                      const QString& path);

  [[nodiscard]] qint64 measure() const;
  [[nodiscard]] QString carryFile(const QString& reference);
  [[nodiscard]] static QString secretIdentity(const Secret& secret);
  [[nodiscard]] bool admitCarriedPath(const QString& carried, const QString& source);
  [[nodiscard]] QString carryCertificate(const QString& path);
  [[nodiscard]] QJsonArray rewriteBands(QJsonArray bands);
  [[nodiscard]] QJsonObject packagedProject(QMap<QString, QString>& files) const;
  [[nodiscard]] bool buildPackage(const QVariantMap& options,
                                  Deploy::Package& package,
                                  QString& error) const;

private:
  qint64 m_estimatedSize;
  QString m_folder;
  QString m_projectPath;
  QJsonObject m_project;
  QStringList m_missing;
  QStringList m_externals;
  QStringList m_extraFiles;
  QList<Secret> m_secrets;
  QMap<QString, QString> m_files;
  QHash<QString, QString> m_carriedBySource;
};

}  // namespace Misc

#endif  // BUILD_COMMERCIAL
