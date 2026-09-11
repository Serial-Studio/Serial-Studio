/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include "UI/Widgets/Output/Base.h"

namespace Widgets {
namespace Output {

/**
 * @brief Output widget with a text input field and send button.
 */
class TextField : public Base {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString reportedText
             READ reportedText
             NOTIFY reportedTextChanged)
  // clang-format on

signals:
  void reportedTextChanged();

public:
  TextField(const DataModel::OutputWidget& config,
            TransmitTarget target,
            QQuickItem* parent = nullptr);

  [[nodiscard]] QString reportedText() const;

public slots:
  void sendText(const QString& text);

protected:
  void applyStateVerdict(const StateBinding::Verdict& verdict) override;

private:
  QString m_reportedText;
};

}  // namespace Output
}  // namespace Widgets
