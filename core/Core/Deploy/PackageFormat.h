/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#  include <QLatin1StringView>
#  include <QtGlobal>

/**
 * @file PackageFormat.h
 * @brief Constants of the .ssdeploy v1 container (spec 0096): magic, versions, CBOR keys and
 *        the fixed caps every decode loop and allocation is bounded by.
 *
 * Layout: 8-byte magic | u16 container version | u16 reserved | u32 header length (all big
 * endian) | CBOR header map | scrambled CBOR payload map. v1 is encoded, not encrypted.
 */

namespace Deploy {

// clang-format off
inline constexpr char kMagic[]                  = "SSDEPLOY";
inline constexpr qsizetype kMagicBytes          = 8;
inline constexpr qsizetype kPrefixBytes         = 16;
inline constexpr quint16 kContainerVersion      = 1;
inline constexpr qint64 kFormatVersion          = 1;
inline constexpr qint64 kEncodingScrambleV1     = 1;

inline constexpr qsizetype kMaxPackageBytes     = qsizetype(64) * 1024 * 1024;
inline constexpr qsizetype kMaxHeaderBytes      = qsizetype(64) * 1024;
inline constexpr qsizetype kMaxProjectBytes     = qsizetype(10) * 1024 * 1024;
inline constexpr qsizetype kMaxIconBytes        = qsizetype(1) * 1024 * 1024;
inline constexpr qsizetype kMaxPathChars        = 512;
inline constexpr qsizetype kMaxTextChars        = 4096;
inline constexpr qsizetype kMaxKeyChars         = 64;
inline constexpr qsizetype kNonceBytes          = 16;
inline constexpr qsizetype kDigestBytes         = 32;
inline constexpr quint64 kMaxFiles              = 4096;
inline constexpr quint64 kMaxCredentials        = 256;
inline constexpr quint64 kMaxExternals          = 256;
inline constexpr quint64 kMaxOptions            = 64;
inline constexpr quint64 kMaxOptionListItems    = 64;
inline constexpr quint64 kMaxMapEntries         = 32;
inline constexpr int kMaxSkipDepth              = 8;

inline constexpr QLatin1StringView kMountRootPrefix{":/ssdeploy-"};
inline constexpr QLatin1StringView kMountId{"package"};
inline constexpr QLatin1StringView kMountRoot{":/ssdeploy-package"};
// clang-format on

namespace HeaderKey {
inline constexpr QLatin1StringView kFormat{"fmt"};
inline constexpr QLatin1StringView kMinApp{"minApp"};
inline constexpr QLatin1StringView kWriter{"writer"};
inline constexpr QLatin1StringView kEncoding{"enc"};
inline constexpr QLatin1StringView kNonce{"nonce"};
inline constexpr QLatin1StringView kPlainSize{"plainSize"};
inline constexpr QLatin1StringView kDigest{"digest"};
}  // namespace HeaderKey

namespace PayloadKey {
inline constexpr QLatin1StringView kManifest{"manifest"};
inline constexpr QLatin1StringView kProject{"project"};
inline constexpr QLatin1StringView kFiles{"files"};
inline constexpr QLatin1StringView kCredentials{"credentials"};
inline constexpr QLatin1StringView kTitle{"title"};
inline constexpr QLatin1StringView kIcon{"icon"};
inline constexpr QLatin1StringView kOptions{"options"};
inline constexpr QLatin1StringView kProjectFolder{"projectFolder"};
inline constexpr QLatin1StringView kExternals{"externals"};
inline constexpr QLatin1StringView kPath{"path"};
inline constexpr QLatin1StringView kData{"data"};
inline constexpr QLatin1StringView kTarget{"target"};
inline constexpr QLatin1StringView kProperty{"property"};
inline constexpr QLatin1StringView kValue{"value"};
}  // namespace PayloadKey

namespace CredentialTarget {
inline constexpr QLatin1StringView kPublisher{"publisher"};
inline constexpr QLatin1StringView kSourcePrefix{"source:"};
}  // namespace CredentialTarget

namespace OptionKey {
inline constexpr QLatin1StringView kFullscreen{"fullscreen"};
inline constexpr QLatin1StringView kActionsPanel{"actionsPanel"};
inline constexpr QLatin1StringView kFileTransmission{"fileTransmission"};
inline constexpr QLatin1StringView kCsvExport{"csvExport"};
inline constexpr QLatin1StringView kMdfExport{"mdfExport"};
inline constexpr QLatin1StringView kSessionExport{"sessionExport"};
inline constexpr QLatin1StringView kConsoleExport{"consoleExport"};
inline constexpr QLatin1StringView kTaskbarMode{"taskbarMode"};
inline constexpr QLatin1StringView kTaskbarButtons{"taskbarButtons"};
inline constexpr QLatin1StringView kTaskbarSearch{"taskbarSearch"};
inline constexpr QLatin1StringView kTheme{"theme"};
}  // namespace OptionKey

}  // namespace Deploy

#endif  // BUILD_COMMERCIAL
