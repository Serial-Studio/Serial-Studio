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

#include <QJSEngine>
#include <QJSValue>
#include <QObject>
#include <QStringList>

namespace DataModel {

/**
 * @brief Singleton JS engine for frame parsing.
 *
 * Owns the QJSEngine, validates and loads JavaScript parser scripts, and
 * executes them against incoming frames. Has no GUI dependency — safe to use
 * in headless / CI mode.
 */
class FrameParser : public QObject
{
  Q_OBJECT

signals:
  void modifiedChanged();
  void templateNamesChanged();

private:
  explicit FrameParser();
  FrameParser(FrameParser &&) = delete;
  FrameParser(const FrameParser &) = delete;
  FrameParser &operator=(FrameParser &&) = delete;
  FrameParser &operator=(const FrameParser &) = delete;

public:
  static FrameParser &instance();

  [[nodiscard]] static QString defaultTemplateCode();

  [[nodiscard]] QString templateCode() const;
  [[nodiscard]] const QStringList &templateNames() const;
  [[nodiscard]] const QStringList &templateFiles() const;

  [[nodiscard]] QStringList parse(const QString &frame);
  [[nodiscard]] QStringList parse(const QByteArray &frame);

  [[nodiscard]] QList<QStringList> parseMultiFrame(const QString &frame);
  [[nodiscard]] QList<QStringList> parseMultiFrame(const QByteArray &frame);

  [[nodiscard]] bool loadScript(const QString &script,
                                const bool showMessageBoxes = true);

  void setSuppressMessageBoxes(const bool suppress);

public slots:
  void readCode();
  void clearContext();
  void collectGarbage();
  void loadTemplateNames();
  void setupExternalConnections();
  void setTemplateIdx(const int idx);
  void loadDefaultTemplate(const bool guiTrigger = false);

private:
  int m_templateIdx;
  bool m_suppressMessageBoxes;

  QJSEngine m_engine;
  QJSValue m_parseFunction;
  QJSValue m_hexToArray;
  QStringList m_templateFiles;
  QStringList m_templateNames;
};

} // namespace DataModel
