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

#  include <QFont>
#  include <QString>

namespace Misc {
class CommonFonts;
}  // namespace Misc

namespace Widgets::PainterFont {

/**
 * @brief Parses a "<size>px <family>" CSS font shorthand into a QFont, resolving generic CSS
 *        families through @a fonts. The font catalog arrives by reference so this stays a pure
 *        mapper with no singleton of its own.
 */
[[nodiscard]] QFont parseFontSpec(const QString& spec, const Misc::CommonFonts& fonts);

}  // namespace Widgets::PainterFont

#endif  // BUILD_COMMERCIAL
