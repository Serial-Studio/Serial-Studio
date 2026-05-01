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

#include <QStyledItemDelegate>
#include <QTableWidget>

#include "DeclarativeWidget.h"

/**
 * @brief QML-embeddable table widget for displaying structured data in rows and
 *        columns.
 */
class StaticTable : public DeclarativeWidget {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(const QFont& font
             READ font
             WRITE setFont
             NOTIFY fontChanged)
  Q_PROPERTY(const QFont& headerFont
             READ headerFont
             WRITE setHeaderFont
             NOTIFY headerFontChanged)
  Q_PROPERTY(QString placeholderText
             READ placeholderText
             WRITE setPlaceholderText
             NOTIFY placeholderTextChanged)
  // clang-format on

signals:
  void fontChanged();
  void headerFontChanged();
  void placeholderTextChanged();

public:
  explicit StaticTable(QQuickItem* parent = nullptr);

  [[nodiscard]] const QFont& font() const;
  [[nodiscard]] const QFont& headerFont() const;
  [[nodiscard]] const QString& placeholderText() const;
  [[nodiscard]] const QList<QStringList>& data() const;

public slots:
  void setFont(const QFont& font);
  void setHeaderFont(const QFont& font);
  void setPlaceholderText(const QString& text);
  void setData(const QList<QStringList>& data);

private slots:
  void loadTheme();

private:
  QFont m_font;
  QFont m_headerFont;
  QString m_placeholderText;
  QTableWidget m_widget;
  QList<QStringList> m_data;
};
