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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QQuickImageProvider>

namespace Misc {

/**
 * @brief Provides online icon search via the Iconify API and inline SVG
 *        resolution for action icons.
 *
 * Exposes a search interface to QML, downloads SVG data on selection, and
 * provides a QQuickImageProvider that can render base64-encoded inline SVGs
 * via the @c image://actionicon/ scheme.
 */
class IconEngine : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool busy
             READ busy
             NOTIFY busyChanged)
  Q_PROPERTY(QStringList iconNames
             READ iconNames
             NOTIFY searchResultsChanged)
  Q_PROPERTY(QStringList iconPreviews
             READ iconPreviews
             NOTIFY searchResultsChanged)
  // clang-format on

signals:
  void busyChanged();
  void searchResultsChanged();
  void iconDownloaded(const QString& svgData);
  void iconDownloadFailed(const QString& error);

private:
  explicit IconEngine();
  IconEngine(IconEngine&&)                 = delete;
  IconEngine(const IconEngine&)            = delete;
  IconEngine& operator=(IconEngine&&)      = delete;
  IconEngine& operator=(const IconEngine&) = delete;

public:
  [[nodiscard]] static IconEngine& instance();

  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] const QStringList& iconNames() const noexcept;
  [[nodiscard]] const QStringList& iconPreviews() const noexcept;

  [[nodiscard]] Q_INVOKABLE static QString resolveActionIconSource(const QString& icon);
  [[nodiscard]] Q_INVOKABLE static bool isInlineSvg(const QString& icon);

public slots:
  void searchIcons(const QString& query);
  void downloadIcon(int index);

private slots:
  void onSearchFinished(QNetworkReply* reply);
  void onDownloadFinished(QNetworkReply* reply);

private:
  bool m_busy;
  QStringList m_iconNames;
  QStringList m_iconPreviews;
  QNetworkAccessManager m_manager;
};

/**
 * @brief QQuickImageProvider for rendering inline base64-encoded SVG icons.
 *
 * Registered under @c "actionicon" so QML can use URLs of the form:
 * @code
 *   image://actionicon/<base64SvgData>
 * @endcode
 */
class ActionIconProvider : public QQuickImageProvider {
public:
  explicit ActionIconProvider();

  QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
};

}  // namespace Misc
