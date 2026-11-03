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

#include "API/CommandSafety.h"

#include <algorithm>
#include <QChar>
#include <QJsonObject>
#include <QStringList>

#include "API/CommandCapabilities.h"

namespace API::CommandSafety {

//--------------------------------------------------------------------------------------------------
// Title vocabulary: a dotted segment or a camelCase piece that does not read as a capitalized word
//--------------------------------------------------------------------------------------------------

namespace detail {
struct Spelling {
  QLatin1StringView key;
  QLatin1StringView title;
};
}  // namespace detail

using detail::Spelling;

// clang-format off
static constexpr Spelling kSegmentSpellings[] = {
  {QLatin1StringView("canbus"), QLatin1StringView("CAN Bus")},
  {QLatin1StringView("iec104"), QLatin1StringView("IEC 104")},
};

static constexpr Spelling kPieceSpellings[] = {
  {QLatin1StringView("ansi"),  QLatin1StringView("ANSI")},
  {QLatin1StringView("api"),   QLatin1StringView("API")},
  {QLatin1StringView("ble"),   QLatin1StringView("BLE")},
  {QLatin1StringView("can"),   QLatin1StringView("CAN")},
  {QLatin1StringView("csv"),   QLatin1StringView("CSV")},
  {QLatin1StringView("db"),    QLatin1StringView("DB")},
  {QLatin1StringView("dtr"),   QLatin1StringView("DTR")},
  {QLatin1StringView("eip"),   QLatin1StringView("EtherNet/IP")},
  {QLatin1StringView("fd"),    QLatin1StringView("FD")},
  {QLatin1StringView("fft"),   QLatin1StringView("FFT")},
  {QLatin1StringView("fps"),   QLatin1StringView("FPS")},
  {QLatin1StringView("hid"),   QLatin1StringView("HID")},
  {QLatin1StringView("http"),  QLatin1StringView("HTTP")},
  {QLatin1StringView("id"),    QLatin1StringView("ID")},
  {QLatin1StringView("io"),    QLatin1StringView("IO")},
  {QLatin1StringView("iso"),   QLatin1StringView("ISO")},
  {QLatin1StringView("json"),  QLatin1StringView("JSON")},
  {QLatin1StringView("mdf"),   QLatin1StringView("MDF")},
  {QLatin1StringView("mqtt"),  QLatin1StringView("MQTT")},
  {QLatin1StringView("opcua"), QLatin1StringView("OPC UA")},
  {QLatin1StringView("tcp"),   QLatin1StringView("TCP")},
  {QLatin1StringView("tls"),   QLatin1StringView("TLS")},
  {QLatin1StringView("uart"),  QLatin1StringView("UART")},
  {QLatin1StringView("udp"),   QLatin1StringView("UDP")},
  {QLatin1StringView("ui"),    QLatin1StringView("UI")},
  {QLatin1StringView("url"),   QLatin1StringView("URL")},
  {QLatin1StringView("usb"),   QLatin1StringView("USB")},
  {QLatin1StringView("uuid"),  QLatin1StringView("UUID")},
  {QLatin1StringView("vt"),    QLatin1StringView("VT")},
};
// clang-format on

//--------------------------------------------------------------------------------------------------
// Lookup helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The generated row for @p command, or null when the table does not know the name.
 */
static const Entry* findEntry(const QStringView command) noexcept
{
  const auto table = entries();
  const auto it =
    std::lower_bound(table.begin(), table.end(), command, [](const Entry& a, QStringView b) {
      return a.name.compare(b) < 0;
    });

  if (it == table.end() || it->name != command)
    return nullptr;

  return &*it;
}

/**
 * @brief The title spelling of @p key in @p table, or an empty view when the key is not listed.
 */
static QLatin1StringView spellingOf(std::span<const Spelling> table, const QStringView key)
{
  for (const auto& spelling : table)
    if (spelling.key == key)
      return spelling.title;

  return QLatin1StringView();
}

//--------------------------------------------------------------------------------------------------
// Title derivation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a new title piece starts at @p i: an upper-case letter after a lower-case one
 *        or a digit, or the last letter of an upper-case run that a lower-case letter follows.
 */
static bool startsPiece(const QStringView text, const qsizetype i)
{
  if (i <= 0 || !text[i].isUpper())
    return false;

  const auto prev = text[i - 1];
  if (prev.isLower() || prev.isDigit())
    return true;

  return prev.isUpper() && i + 1 < text.size() && text[i + 1].isLower();
}

/**
 * @brief One piece as it reads in a title: the vocabulary spelling of its letters, else the
 *        capitalized word, with trailing digits kept (Vt100 reads VT100).
 */
static QString humanizePiece(const QStringView piece)
{
  qsizetype letters = 0;
  while (letters < piece.size() && piece[letters].isLetter())
    ++letters;

  const auto word   = piece.first(letters).toString().toLower();
  const auto digits = piece.sliced(letters);
  const auto known  = spellingOf(kPieceSpellings, word);
  if (!known.isEmpty())
    return QString(known) + digits;

  if (word.isEmpty())
    return digits.toString();

  return word.first(1).toUpper() + word.sliced(1) + digits;
}

/**
 * @brief One dotted segment: the vocabulary spelling of the whole segment, else its camelCase
 *        pieces humanized and joined with spaces.
 */
static QString humanizeSegment(const QStringView segment)
{
  const auto whole = spellingOf(kSegmentSpellings, segment);
  if (!whole.isEmpty())
    return QString(whole);

  QStringList pieces;
  qsizetype start = 0;
  for (qsizetype i = 1; i < segment.size(); ++i) {
    if (!startsPiece(segment, i))
      continue;

    pieces.append(humanizePiece(segment.sliced(start, i - start)));
    start = i;
  }

  pieces.append(humanizePiece(segment.sliced(start)));
  return pieces.join(QLatin1Char(' '));
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief The command's tier, or nothing when the table does not list it.
 */
std::optional<Tier> tierOf(const QStringView command) noexcept
{
  if (const auto* entry = findEntry(command))
    return entry->tier;

  return std::nullopt;
}

/**
 * @brief Whether the "Allow device control" toggle gates the command.
 */
bool isDeviceGated(const QStringView command) noexcept
{
  const auto* entry = findEntry(command);
  return entry && entry->device_gated;
}

/**
 * @brief The MCP hints the tier implies: Safe is read-only, Confirm is a plain mutation, and
 *        AlwaysConfirm, Blocked or a device-gated name is destructive. An unlisted name is the
 *        assistant's Confirm fallthrough, a plain mutation.
 */
ToolHints hintsFor(const QStringView command) noexcept
{
  const auto* entry = findEntry(command);
  if (!entry)
    return ToolHints{.read_only = false, .destructive = false};

  if (entry->device_gated)
    return ToolHints{.read_only = false, .destructive = true};

  switch (entry->tier) {
    case Tier::Safe:
      return ToolHints{.read_only = true, .destructive = false};
    case Tier::Confirm:
      return ToolHints{.read_only = false, .destructive = false};
    case Tier::AlwaysConfirm:
    case Tier::Blocked:
      return ToolHints{.read_only = false, .destructive = true};
  }

  return ToolHints{.read_only = false, .destructive = false};
}

/**
 * @brief Whether the MCP tool list shows the command: not the blocked tier, and not a name the
 *        remote gate refuses on classification alone (the control-script-only class). Listing
 *        is discovery; a call still goes through the gate.
 */
bool isListedTool(const QStringView command)
{
  if (tierOf(command) == Tier::Blocked)
    return false;

  const auto capabilities = CommandCapabilities::of(command, QJsonObject());
  return !(capabilities && capabilities->testFlag(CommandCapabilities::ControlScriptOnly));
}

/**
 * @brief The dotted name as a title: segments joined with " / ", each one spelled from the
 *        vocabulary or split on its camelCase boundaries.
 */
QString humanizeToolName(const QStringView command)
{
  QStringList segments;
  for (const auto segment : command.split(QLatin1Char('.'), Qt::SkipEmptyParts))
    segments.append(humanizeSegment(segment));

  return segments.join(QStringLiteral(" / "));
}
}  // namespace API::CommandSafety
