/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QObject>
#  include <QString>
#  include <QStringList>

namespace Misc {

/**
 * @brief Builds operator-friendly desktop shortcuts that relaunch Serial Studio
 *        with a fixed project file and logging configuration.
 */
class ShortcutGenerator : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString defaultIconPath
             READ defaultIconPath
             CONSTANT)
  Q_PROPERTY(QString shortcutFileFilter
             READ shortcutFileFilter
             CONSTANT)
  Q_PROPERTY(QString shortcutDefaultSuffix
             READ shortcutDefaultSuffix
             CONSTANT)
  Q_PROPERTY(QString platformNote
             READ platformNote
             CONSTANT)
  Q_PROPERTY(QString iconFileFilter
             READ iconFileFilter
             CONSTANT)
  // clang-format on

signals:
  void shortcutGenerated(const QString& path);
  void shortcutFailed(const QString& errorMessage);

private:
  explicit ShortcutGenerator();
  ShortcutGenerator(ShortcutGenerator&&)                 = delete;
  ShortcutGenerator(const ShortcutGenerator&)            = delete;
  ShortcutGenerator& operator=(ShortcutGenerator&&)      = delete;
  ShortcutGenerator& operator=(const ShortcutGenerator&) = delete;

  ~ShortcutGenerator() override;

public:
  [[nodiscard]] static ShortcutGenerator& instance();

  [[nodiscard]] QString platformNote() const;
  [[nodiscard]] QString defaultIconPath() const;
  [[nodiscard]] QString iconFileFilter() const;
  [[nodiscard]] QString shortcutFileFilter() const;
  [[nodiscard]] QString shortcutDefaultSuffix() const;

public slots:
  void generate(const QString& outputPath,
                const QString& title,
                const QString& iconPath,
                const QString& projectFile,
                bool fullscreen,
                bool actionsPanel,
                bool csvExport,
                bool mdfExport,
                bool sessionExport,
                bool consoleExport);

  void deleteShortcut(const QString& shortcutPath);

private:
  [[nodiscard]] QStringList buildArguments(const QString& projectFile,
                                           bool fullscreen,
                                           bool actionsPanel,
                                           bool csvExport,
                                           bool mdfExport,
                                           bool sessionExport,
                                           bool consoleExport) const;

  [[nodiscard]] bool writeShortcutFile(const QString& outputPath,
                                       const QString& title,
                                       const QString& iconPath,
                                       const QStringList& args,
                                       QString& errorOut);

  [[nodiscard]] bool writeWindowsLnk(const QString& outputPath,
                                     const QString& target,
                                     const QStringList& args,
                                     const QString& title,
                                     const QString& iconPath,
                                     QString& errorOut);

  [[nodiscard]] bool writeMacCommand(const QString& outputPath,
                                     const QString& target,
                                     const QStringList& args,
                                     const QString& title,
                                     const QString& iconPath,
                                     QString& errorOut);

  [[nodiscard]] bool writeLinuxDesktop(const QString& outputPath,
                                       const QString& target,
                                       const QStringList& args,
                                       const QString& title,
                                       const QString& iconPath,
                                       QString& errorOut);

  [[nodiscard]] QString quoteArg(const QString& arg) const;
  [[nodiscard]] bool hasProLicense() const;
  [[nodiscard]] QString extractDefaultIcon() const;
};

}  // namespace Misc

#endif  // BUILD_COMMERCIAL
