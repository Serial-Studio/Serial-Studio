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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/WebView.h"

/**
 * @brief Constructs a web view model bound to the dashboard slot @p index.
 */
Widgets::WebView::WebView(const int index, QQuickItem* parent) : QQuickItem(parent), m_index(index)
{}

/**
 * @brief Returns the URL this web view should load.
 */
QString Widgets::WebView::url() const
{
  return m_url;
}

/**
 * @brief Updates the URL and notifies the QML surface to reload.
 */
void Widgets::WebView::setUrl(const QString& url)
{
  if (m_url == url)
    return;

  m_url = url;
  Q_EMIT urlChanged();
}
