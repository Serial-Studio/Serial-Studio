/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QString>

#  include "SerialStudio.h"

namespace Sessions {
/**
 * @brief Operation mode, project and view state captured before a recording took over, held
 *        together so "captured" cannot desynchronise from the values it guards. A value
 *        object by design: it reads no singleton, leaving orchestration to Player.
 */
class PreSessionState {
public:
  explicit PreSessionState();

  [[nodiscard]] bool captured() const noexcept;
  [[nodiscard]] const QString& viewState() const noexcept;
  [[nodiscard]] const QString& projectPath() const noexcept;
  [[nodiscard]] SerialStudio::OperationMode operationMode() const noexcept;

  void clear();
  void capture(const SerialStudio::OperationMode mode,
               const QString& projectPath,
               const QString& viewState);

private:
  bool m_captured;
  QString m_viewState;
  QString m_projectPath;
  SerialStudio::OperationMode m_operationMode;
};
}  // namespace Sessions

#endif
