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

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

namespace Misc {

/**
 * @brief One bundled icon pack as declared by its pack.json (spec 0115).
 */
struct IconPackInfo {
  QString id;
  QString title;
  QHash<QString, QString> translations;
  bool mono;
};

/**
 * @brief Resolves fixed UI icons by (category, name, pixel size) from the resource tree at
 *        :/icons/<category>/<tier>/<name>.svg (spec 0028), consulting the active icon pack at
 *        :/iconpacks/<pack>/<category>/<tier>/<name>.svg first for the themed categories
 *        (spec 0115). A mono pack resolves to an image-provider URL carrying the tint, and
 *        monoIconById() draws from the fixed mono pack whatever pack is active, for the surfaces
 *        that are monochrome by design (the dashboard widget toolbars).
 */
class IconRegistry : public QObject {
  Q_OBJECT

signals:
  void packChanged();
  void revisionChanged();

private:
  explicit IconRegistry();
  IconRegistry(IconRegistry&&)                 = delete;
  IconRegistry(const IconRegistry&)            = delete;
  IconRegistry& operator=(IconRegistry&&)      = delete;
  IconRegistry& operator=(const IconRegistry&) = delete;

public:
  enum class Tone {
    Text,
    Strip
  };

  [[nodiscard]] static IconRegistry& instance();
  explicit IconRegistry(const QString& houseRoot, const QString& packsRoot);

  [[nodiscard]] Q_INVOKABLE QString icon(const QString& category,
                                         const QString& name,
                                         int px) const;
  [[nodiscard]] Q_INVOKABLE QString iconById(const QString& id, int px) const;
  [[nodiscard]] QString iconOn(Tone tone,
                               const QString& category,
                               const QString& name,
                               int px) const;
  [[nodiscard]] QString iconByIdOn(Tone tone, const QString& id, int px) const;
  [[nodiscard]] QString monoIconById(const QString& id, int px) const;
  [[nodiscard]] QString iconPath(const QString& category, const QString& name, int px) const;

  [[nodiscard]] const QString& pack() const noexcept;
  [[nodiscard]] const QList<IconPackInfo>& packs() const noexcept;
  [[nodiscard]] bool isMono() const noexcept;
  [[nodiscard]] bool hasPack(const QString& id) const;
  [[nodiscard]] static const QStringList& fixedCategories();
  [[nodiscard]] static const QString& monoPackId();

public slots:
  void setPack(const QString& id);
  void setTints(const QString& text, const QString& strip);

private:
  struct Located {
    QString path;
    bool fromPack;
  };

  void scanTree(const QString& root, const QString& pack);
  void scanPacks(const QString& packsRoot);
  [[nodiscard]] Located locate(const QString& category, const QString& name, int px) const;
  [[nodiscard]] Located locateHouse(const QString& category, const QString& name, int px) const;
  [[nodiscard]] Located notFound(const QString& category, const QString& name) const;
  [[nodiscard]] QString packPath(const QString& category, const QString& name, int px) const;
  [[nodiscard]] QString packPathIn(const QString& pack,
                                   const QString& category,
                                   const QString& name,
                                   int px) const;
  [[nodiscard]] QString url(const Located& hit, Tone tone) const;
  [[nodiscard]] QString missingIcon(const QString& category, const QString& name) const;
  [[nodiscard]] static QString tierPath(const QHash<QString, QList<int>>& names,
                                        const QString& name,
                                        int px);

private:
  bool m_mono;
  QString m_pack;
  QString m_textTint;
  QString m_stripTint;
  QList<IconPackInfo> m_packs;
  mutable QSet<QString> m_warned;
  QHash<QString, QHash<QString, QList<int>>> m_catalog;
  QHash<QString, QHash<QString, QHash<QString, QList<int>>>> m_packCatalog;
};

}  // namespace Misc
