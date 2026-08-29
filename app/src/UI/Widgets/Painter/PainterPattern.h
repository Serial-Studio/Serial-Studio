/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QBrush>
#  include <QObject>
#  include <QPixmap>
#  include <QString>

namespace Widgets {

/**
 * @brief Canvas2D-shaped pattern handle exposed to JS.
 */
class PainterPattern : public QObject {
  Q_OBJECT

public:
  explicit PainterPattern(const QPixmap& tile,
                          const QString& repetition,
                          QObject* parent = nullptr);
  ~PainterPattern() override = default;

  [[nodiscard]] QBrush brush() const;

private:
  QPixmap m_tile;
  QString m_repetition;
};

}  // namespace Widgets

#endif  // BUILD_COMMERCIAL
