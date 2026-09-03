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

#include <cstdint>
#include <QByteArray>
#include <QString>

#include "CSV/Player/RowSyntax.h"

/**
 * @brief Drives the CSV replay row scanners with untrusted bytes (spec 0075 R14.3). A replayed
 *        file is user input: unbalanced quotes, embedded NULs, a header alone, a row alone and
 *        invalid UTF-8 all reach these scanners before anything validates the file.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  const QByteArray bytes(reinterpret_cast<const char*>(data), static_cast<qsizetype>(size));
  const qsizetype split   = bytes.indexOf('\n');
  const QByteArray header = split < 0 ? bytes : bytes.left(split);
  const QByteArray row    = split < 0 ? QByteArray() : bytes.mid(split + 1);

  for (const char separator : {',', ';', '\t', '|'}) {
    (void)CSV::firstTopLevelSeparator(header, separator);
    (void)CSV::topLevelSeparatorCount(row, separator);
  }

  (void)CSV::sniffSeparator(header, row);
  (void)CSV::timestampUnitScale(QString::fromUtf8(header));
  return 0;
}
