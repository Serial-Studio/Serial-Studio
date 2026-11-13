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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <functional>
#include <QApplication>
#include <QMessageBox>
#include <QObject>

#include "Core/Prompt/IUserPrompter.h"

/**
 * @brief Maps QMessageBox standard buttons to their localized labels.
 */
typedef QMap<QMessageBox::StandardButton, QString> ButtonTextMap;

/**
 * @brief Called with the built box just before its modal loop starts.
 */
using MessageBoxHook = std::function<void(QMessageBox*)>;

namespace Misc {
/**
 * @brief Application-wide utility functions for common UI and system operations; also the Ui
 *        layer's implementation of the Core prompt seam the lower libraries speak through.
 */
class Utilities
  : public QObject
  , public Core::Prompt::IUserPrompter {
  Q_OBJECT

public:
  [[nodiscard]] static Utilities& instance();

  [[nodiscard]] int promptMessage(const QString& text,
                                  const QString& informativeText,
                                  Core::Prompt::Icon icon,
                                  const QString& windowTitle,
                                  Core::Prompt::Buttons buttons,
                                  Core::Prompt::Button defaultButton,
                                  const Core::Prompt::ButtonLabels& buttonLabels) override;
  void promptDirectory(const QString& title,
                       const QString& initialPath,
                       DirectoryHandler onSelected) override;
  void revealInFileManager(const QString& path) override;

  static void rebootApplication();
  static QPixmap getHiDpiPixmap(const QString& path);

  [[nodiscard]] static QIcon coloredSvgIcon(const QString& svgPath, const QColor& color);
  [[nodiscard]] Q_INVOKABLE static QString hdpiImagePath(const QString& path);

  // clang-format off
  static int showMessageBox(const QString& text,
                            const QString& informativeText = "",
                            QMessageBox::Icon icon = QMessageBox::Information,
                            const QString& windowTitle = "",
                            QMessageBox::StandardButtons bt = QMessageBox::Ok,
                            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton,
                            const ButtonTextMap& buttonTexts = ButtonTextMap(),
                            const MessageBoxHook& onOpen = MessageBoxHook());

  static void postMessageBox(const QString& text,
                             const QString& informativeText = "",
                             QMessageBox::Icon icon = QMessageBox::Information,
                             const QString& windowTitle = "");
  // clang-format on

public slots:
  static void aboutQt();
  static void copyText(const QString& text);
  static void revealFile(const QString& pathToReveal);
};
}  // namespace Misc
