/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include "Misc/ProblemCenter.h"
#include "UI/WidgetExtensions.h"

namespace UI {

/**
 * @brief Validator for one widget-extension manifest (spec 0038): turns an info.json object into
 *        either a descriptor or the findings that explain the rejection. Pure and catalog-free, so
 *        the two facts it judges against (host widget API version, reserved built-in widget
 *        strings) are supplied by the catalog that constructs it.
 */
class WidgetManifestParser {
public:
  /**
   * @brief Outcome of validating one manifest: either a descriptor or the findings that explain
   *        why the package was rejected.
   */
  struct Result {
    bool ok = false;
    WidgetExtensions::Descriptor descriptor;
    QList<Misc::ProblemCenter::Finding> findings;
  };

  WidgetManifestParser(const QString& hostApiVersion, const QStringList& reservedIds);

  [[nodiscard]] static bool versionInRange(const QString& version, const QString& range);

  [[nodiscard]] Result parse(const QJsonObject& manifest,
                             const QString& directory,
                             bool bundled) const;

private:
  [[nodiscard]] bool validateIdentity(const QJsonObject& block,
                                      WidgetExtensions::Descriptor& out,
                                      bool bundled,
                                      QList<Misc::ProblemCenter::Finding>& findings) const;
  [[nodiscard]] bool validateCompatibility(const QJsonObject& block,
                                           const WidgetExtensions::Descriptor& out,
                                           QList<Misc::ProblemCenter::Finding>& findings) const;

private:
  QString m_hostApiVersion;
  QStringList m_reservedIds;
};

}  // namespace UI
