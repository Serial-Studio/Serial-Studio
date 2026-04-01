/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QQuickItem>
#include <QVariantList>

#include "DataModel/Frame.h"

namespace Widgets {
namespace Output {

/**
 * @class Widgets::Output::Panel
 * @brief Dashboard widget model for an output control panel.
 *
 * Exposes the list of output widgets and their computed layout geometry.
 * QML calls updateLayout() with the available size; the engine computes
 * optimal 2D bin-packed placement.
 */
class Panel : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int count
             READ count
             CONSTANT)
  Q_PROPERTY(QVariantList widgets
             READ widgets
             CONSTANT)
  Q_PROPERTY(QVariantList models
             READ models
             CONSTANT)
  Q_PROPERTY(QVariantList geometry
             READ geometry
             NOTIFY geometryChanged)
  // clang-format on

signals:
  void geometryChanged();

public:
  explicit Panel(int index, QQuickItem* parent = nullptr);

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] QVariantList widgets() const;
  [[nodiscard]] QVariantList models() const;
  [[nodiscard]] QVariantList geometry() const;

public slots:
  void updateLayout(qreal width, qreal height);

private:
  QVariantList m_widgets;
  QVariantList m_models;
  QVariantList m_geometry;
  std::vector<DataModel::OutputWidget> m_outputWidgets;
};

}  // namespace Output
}  // namespace Widgets
