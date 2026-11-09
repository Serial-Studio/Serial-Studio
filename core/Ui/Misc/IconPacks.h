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

#pragma once

#include <QColor>
#include <QJSValue>
#include <QObject>
#include <QSettings>
#include <QStringList>

#include "Core/IconRegistry.h"

class QJSEngine;

namespace Misc {
class ThemeManager;

/**
 * @brief The QML face of the icon registry under icon packs (spec 0115): the resolvers are
 *        function-valued notifying properties, so every icon binding re-evaluates on a pack or
 *        theme change; the selection persists, "Follow theme" defers to the theme's icon-pack
 *        parameter and "Office" pins the house set. monoIconById serves the dashboard widget
 *        toolbars from their own choice, which follows the theme's widget-icon-pack by default.
 */
class IconPacks : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QJSValue icon
             READ iconFunction
             NOTIFY revisionChanged)
  Q_PROPERTY(QJSValue iconById
             READ iconByIdFunction
             NOTIFY revisionChanged)
  Q_PROPERTY(QJSValue iconOn
             READ iconOnFunction
             NOTIFY revisionChanged)
  Q_PROPERTY(QJSValue iconByIdOn
             READ iconByIdOnFunction
             NOTIFY revisionChanged)
  Q_PROPERTY(QJSValue monoIconById
             READ monoIconByIdFunction
             NOTIFY revisionChanged)
  Q_PROPERTY(int pack
             READ pack
             WRITE setPack
             NOTIFY packChanged)
  Q_PROPERTY(QString activePackId
             READ activePackId
             NOTIFY revisionChanged)
  Q_PROPERTY(bool mono
             READ mono
             NOTIFY revisionChanged)
  Q_PROPERTY(int widgetPack
             READ widgetPack
             WRITE setWidgetPack
             NOTIFY widgetPackChanged)
  Q_PROPERTY(QStringList availablePacks
             READ availablePacks
             NOTIFY availablePacksChanged)
  Q_PROPERTY(QStringList availableWidgetPacks
             READ availableWidgetPacks
             NOTIFY availablePacksChanged)
  // clang-format on

signals:
  void packChanged();
  void revisionChanged();
  void widgetPackChanged();
  void availablePacksChanged();

public:
  explicit IconPacks(QJSEngine& engine,
                     IconRegistry& registry,
                     ThemeManager& theme,
                     QObject* parent = nullptr);
  IconPacks(IconPacks&&)                 = delete;
  IconPacks(const IconPacks&)            = delete;
  IconPacks& operator=(IconPacks&&)      = delete;
  IconPacks& operator=(const IconPacks&) = delete;

  [[nodiscard]] int pack() const;
  [[nodiscard]] int widgetPack() const;
  [[nodiscard]] bool mono() const;
  [[nodiscard]] QString activePackId() const;
  [[nodiscard]] const QStringList& availablePacks() const noexcept;
  [[nodiscard]] const QStringList& availableWidgetPacks() const noexcept;

  [[nodiscard]] QJSValue iconFunction() const;
  [[nodiscard]] QJSValue iconByIdFunction() const;
  [[nodiscard]] QJSValue iconOnFunction() const;
  [[nodiscard]] QJSValue iconByIdOnFunction() const;
  [[nodiscard]] QJSValue monoIconByIdFunction() const;

  [[nodiscard]] Q_INVOKABLE QString resolve(const QString& category,
                                            const QString& name,
                                            int px) const;
  [[nodiscard]] Q_INVOKABLE QString resolveById(const QString& id, int px) const;
  [[nodiscard]] Q_INVOKABLE QString resolveOn(const QString& tone,
                                              const QString& category,
                                              const QString& name,
                                              int px) const;
  [[nodiscard]] Q_INVOKABLE QString resolveByIdOn(const QString& tone,
                                                  const QString& id,
                                                  int px) const;
  [[nodiscard]] Q_INVOKABLE QString resolveMonoById(const QString& id, int px) const;
  [[nodiscard]] Q_INVOKABLE QString retint(const QString& url, const QColor& color) const;

public slots:
  void setPack(int index);
  void setWidgetPack(int index);
  void setSettingsPersistent(bool persistent);

private slots:
  void applyEffectivePack();
  void updateLocalizedNames();

private:
  [[nodiscard]] QList<IconPackInfo> widgetPackChoices() const;
  [[nodiscard]] static IconRegistry::Tone toneFrom(const QString& tone);

private:
  IconRegistry& m_registry;
  ThemeManager& m_theme;
  QSettings m_settings;
  bool m_persistSettings;
  QString m_selected;
  QString m_applied;
  QString m_widgetSelected;
  QStringList m_names;
  QStringList m_widgetNames;
  QJSValue m_icon;
  QJSValue m_iconById;
  QJSValue m_iconOn;
  QJSValue m_iconByIdOn;
  QJSValue m_monoIconById;
};

}  // namespace Misc
