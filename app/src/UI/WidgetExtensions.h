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

#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>

#include "Misc/ProblemCenter.h"

namespace UI {

/**
 * @brief Catalog of installed widget extension packages (spec 0038): validates every manifest
 *        eagerly and compiles no QML. A package is trusted code the user chose to run, sharing the
 *        application's QML engine and privileges, so canInstantiate() stays default-deny until
 *        consent is recorded for that exact version. The constructor is inert (member init only).
 */
class WidgetExtensions : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Entity kind a package attaches to; a package declares exactly one.
   */
  enum Scope : int {
    GroupScope   = 0,
    DatasetScope = 1,
  };
  Q_ENUM(Scope)

  /**
   * @brief Dataset value kinds a package declares it can render.
   */
  enum ValueKind : int {
    NumericValue = 0,
    StringValue  = 1,
    AnyValue     = 2,
  };
  Q_ENUM(ValueKind)

  /**
   * @brief Dataset-count and value-kind bounds a package accepts; @c maxDatasets is -1 when the
   *        package declares no upper bound.
   */
  struct Accepts {
    int minDatasets = 1;
    int maxDatasets = -1;
    ValueKind value = AnyValue;
  };

  /**
   * @brief One declared per-widget setting, rendered by the generic config form and stored in the
   *        project's widget-settings map. Scalars and fixed choices only in v1.
   */
  struct ConfigProperty {
    QString id;
    QString type;
    QString label;
    QString description;
    QVariant defaultValue;
    QVariant minimum;
    QVariant maximum;
    QStringList options;
  };

  /**
   * @brief A declared dependency on another extension package.
   */
  struct Dependency {
    bool required = true;
    QString id;
    QString versionRange;
  };

  /**
   * @brief Immutable description of one validated package. @c replaces is non-empty only for
   *        bundled packages that ship as the implementation of a builtin widget string.
   */
  struct Descriptor {
    bool bundled           = false;
    bool experimental      = false;
    bool readsStringValues = false;
    Scope scope            = DatasetScope;
    int defaultWidth       = 400;
    int defaultHeight      = 300;

    QString id;
    QString title;
    QString author;
    QString license;
    QString version;
    QString category;
    QString iconId;
    QString replaces;
    QString qmlFile;
    QString directory;
    QString description;

    Accepts accepts;
    QList<Dependency> dependencies;
    QList<ConfigProperty> config;

    [[nodiscard]] bool isValid() const { return !id.isEmpty() && !qmlFile.isEmpty(); }
  };

signals:
  void catalogChanged();
  void consentRequested(const QString& id);

private:
  explicit WidgetExtensions();
  WidgetExtensions(WidgetExtensions&&)                 = delete;
  WidgetExtensions(const WidgetExtensions&)            = delete;
  WidgetExtensions& operator=(WidgetExtensions&&)      = delete;
  WidgetExtensions& operator=(const WidgetExtensions&) = delete;

public:
  [[nodiscard]] static WidgetExtensions& instance();

  [[nodiscard]] static QString hostApiVersion();
  [[nodiscard]] static QStringList reservedIds();
  [[nodiscard]] static QStringList hostContextNames();
  [[nodiscard]] static bool isReservedId(const QString& id);
  [[nodiscard]] static QString persistedTypeToken(const QString& id);

  [[nodiscard]] QStringList ids() const;
  [[nodiscard]] bool contains(const QString& id) const;
  [[nodiscard]] QStringList idsForScope(Scope scope) const;
  [[nodiscard]] const Descriptor& descriptor(const QString& id) const;
  [[nodiscard]] const QList<Misc::ProblemCenter::Finding>& findings() const noexcept;
  [[nodiscard]] QString builtinReplacement(const QString& builtinWidgetId) const;

  [[nodiscard]] bool canInstantiate(const QString& id) const;
  [[nodiscard]] bool acceptsEntity(const QString& id,
                                   Scope scope,
                                   int datasetCount,
                                   bool numericValues) const;

  Q_INVOKABLE [[nodiscard]] QString qmlUrl(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] bool consentGranted(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] bool consentDeclined(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] bool consentRequired(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap packageInfo(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] QVariantList configProperties(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] QString iconUrl(const QString& id, const int px) const;

public slots:
  void rescan();
  void grantConsent(const QString& id);
  void revokeConsent(const QString& id);
  void declineConsent(const QString& id);
  void requestConsent(const QString& id);
  void reportLoadFailure(const QString& id, const QString& error);

private:
  void scanDirectory(const QString& root, bool bundled);
  void loadPackage(const QString& directory, bool bundled);
  void resolveDependencies();

private:
  QSettings m_settings;
  QSet<QString> m_prompted;
  QStringList m_order;
  QHash<QString, Descriptor> m_descriptors;
  QHash<QString, QString> m_replacements;
  QList<Misc::ProblemCenter::Finding> m_findings;
};

}  // namespace UI
