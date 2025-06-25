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
  QImage m_image;
  QWidget *m_widget;
  int m_contentWidth;
  int m_contentHeight;
  bool m_updateRequested;
};
