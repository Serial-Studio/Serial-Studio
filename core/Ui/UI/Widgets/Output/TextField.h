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

#include <QStringList>

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
  Q_PROPERTY(bool keypadSign
             READ keypadSign
             CONSTANT)
  Q_PROPERTY(bool keypadDecimal
             READ keypadDecimal
             CONSTANT)
  Q_PROPERTY(int historyCount
             READ historyCount
             NOTIFY historyChanged)
  // clang-format on

signals:
  void historyChanged();
  void reportedTextChanged();

public:
  TextField(const DataModel::OutputWidget& config,
            TransmitTarget target,
            QQuickItem* parent = nullptr);

  [[nodiscard]] bool keypadSign() const noexcept;
  [[nodiscard]] int historyCount() const noexcept;
  [[nodiscard]] QString reportedText() const;
  [[nodiscard]] bool keypadDecimal() const noexcept;

public slots:
  [[nodiscard]] bool sendText(const QString& text);
  [[nodiscard]] QString historyAt(int index) const;
  [[nodiscard]] QString keypadApply(const QString& entry, const QString& key) const;

protected:
  void applyStateVerdict(const StateBinding::Verdict& verdict) override;

private:
  bool m_keypadSign;
  bool m_keypadDecimal;
  QString m_reportedText;
  QStringList m_history;
};

}  // namespace Output
}  // namespace Widgets
