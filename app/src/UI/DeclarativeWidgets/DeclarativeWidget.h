/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QTimer>
#include <QWidget>
#include <QPainter>
#include <QQuickPaintedItem>

/**
 * @class DeclarativeWidget
 *
 * @brief Base class for embedding a QWidget inside a QML UI.
 *
 * This class enables a QWidget to be rendered within a QML interface by
 * capturing its output as a QPixmap and displaying it in QML. User interaction
 * is disabled to avoid slowing down the rendering engine.
 *
 * QWidget and QML use fundamentally different rendering models—native OS
 * rendering vs. GPU-accelerated rendering—which makes direct embedding
 * normally impossible. This class works around that by bridging the two.
 */
class DeclarativeWidget : public QQuickPaintedItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QPalette palette
             READ palette
             WRITE setPalette
             NOTIFY paletteChanged)
  Q_PROPERTY(QWidget *widget
             READ widget
             WRITE setWidget
             NOTIFY widgetChanged)
  Q_PROPERTY(int contentWidth
             READ contentWidth
             NOTIFY geometryChanged)
  Q_PROPERTY(int contentHeight
             READ contentHeight
             NOTIFY geometryChanged)
  // clang-format on

Q_SIGNALS:
  void widgetChanged();
  void geometryChanged();

public:
  explicit DeclarativeWidget(QQuickItem *parent = nullptr);

  [[nodiscard]] QWidget *widget() const;
  [[nodiscard]] QPalette palette() const;

  [[nodiscard]] int contentWidth() const;
  [[nodiscard]] int contentHeight() const;

  void redraw(const QRect &rect = QRect());
  void paint(QPainter *painter) override;

public Q_SLOTS:
  void resizeWidget();
  void requestUpdate();
  void setWidget(QWidget *widget);
  void setContentWidth(const int width);
  void setContentHeight(const int height);
  void setPalette(const QPalette &palette);

private:
  QImage m_pixmap;
  QWidget *m_widget;
  int m_contentWidth;
  int m_contentHeight;
  bool m_updateRequested;
};
