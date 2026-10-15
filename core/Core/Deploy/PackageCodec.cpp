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

#ifdef BUILD_COMMERCIAL

#  include "Core/Deploy/PackageCodec.h"

#  include <array>
#  include <cstring>
#  include <QCborStreamReader>
#  include <QCborStreamWriter>
#  include <QCryptographicHash>
#  include <QJsonArray>
#  include <QRandomGenerator>
#  include <QSet>
#  include <QtEndian>

#  include "Core/Deploy/PackageFormat.h"
#  include "Core/SSAssert.h"

using namespace Deploy;

static constexpr char kScrambleLabel[] = "Serial Studio deployment scramble v1";

namespace Deploy::CodecDetail {

/**
 * @brief xoshiro256** state. A keystream, not a cipher: it only keeps strings out of the bytes.
 */
struct Keystream {
  std::array<quint64, 4> state;
};

/**
 * @brief Header fields as read; -1 and empty mark a field the header never carried.
 */
struct Header {
  qint64 format    = -1;
  qint64 encoding  = -1;
  qint64 plainSize = -1;
  QString minApp;
  QString writer;
  QByteArray nonce;
  QByteArray digest;
};

}  // namespace Deploy::CodecDetail

using namespace Deploy::CodecDetail;

//--------------------------------------------------------------------------------------------------
// Scramble
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rotates a 64-bit word left by k bits (0 < k < 64).
 */
[[nodiscard]] static quint64 rotl(const quint64 x, const int k) noexcept
{
  return (x << k) | (x >> (64 - k));
}

/**
 * @brief A valid non-zero state for the unreachable case of a short SHA-256 result.
 */
[[nodiscard]] static Keystream fallbackKeystream() noexcept
{
  Keystream ks{};
  ks.state[0] = 1;
  return ks;
}

/**
 * @brief Seeds the keystream from SHA-256 of the fixed label and the package nonce.
 */
[[nodiscard]] static Keystream seedKeystream(const QByteArray& nonce)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  hash.addData(QByteArrayView(kScrambleLabel));
  hash.addData(nonce);
  const auto seed = hash.result();
  SS_ASSERT(seed.size() == kDigestBytes, return fallbackKeystream());

  Keystream ks{};
  for (int i = 0; i < 4; ++i)
    ks.state[i] = qFromLittleEndian<quint64>(seed.constData() + i * 8);

  if ((ks.state[0] | ks.state[1] | ks.state[2] | ks.state[3]) == 0)
    ks.state[0] = 1;

  return ks;
}

/**
 * @brief Advances the xoshiro256** state and returns the next 64-bit keystream word.
 */
[[nodiscard]] static quint64 nextWord(Keystream& ks) noexcept
{
  auto& s              = ks.state;
  const quint64 value  = rotl(s[1] * 5, 7) * 9;
  const quint64 t      = s[1] << 17;
  s[2]                ^= s[0];
  s[3]                ^= s[1];
  s[1]                ^= s[2];
  s[0]                ^= s[3];
  s[2]                ^= t;
  s[3]                 = rotl(s[3], 45);
  return value;
}

/**
 * @brief XORs data with the nonce's keystream; the same call scrambles and unscrambles.
 */
[[nodiscard]] static QByteArray scramble(const QByteArray& data, const QByteArray& nonce)
{
  SS_ASSERT(nonce.size() == kNonceBytes, return {});
  SS_ASSERT(data.size() <= kMaxPackageBytes, return {});

  auto ks        = seedKeystream(nonce);
  QByteArray out = data;
  char* bytes    = out.data();
  quint64 word   = 0;
  for (qsizetype i = 0; i < out.size(); ++i) {
    if ((i & 7) == 0)
      word = nextWord(ks);

    const auto key = static_cast<uchar>(word >> ((i & 7) * 8));
    bytes[i]       = static_cast<char>(static_cast<uchar>(bytes[i]) ^ key);
  }

  return out;
}

/**
 * @brief Draws a fresh 16-byte nonce from the system generator.
 */
[[nodiscard]] static QByteArray makeNonce()
{
  std::array<quint32, 4> words{};
  QRandomGenerator::system()->fillRange(words.data(), static_cast<qsizetype>(words.size()));

  QByteArray nonce(kNonceBytes, Qt::Uninitialized);
  for (int i = 0; i < 4; ++i)
    qToLittleEndian<quint32>(words[i], nonce.data() + i * 4);

  SS_ASSERT(nonce.size() == kNonceBytes, return QByteArray(kNonceBytes, '\0'));
  return nonce;
}

/**
 * @brief SHA-256 over every header field and the payload, each length-prefixed, so a change to
 *        any byte the decoder trusts (versions, encoding, nonce, payload) fails the compare.
 */
[[nodiscard]] static QByteArray contentDigest(const Header& header, const QByteArray& plain)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  const auto addField = [&hash](const QByteArrayView field) {
    char length[4];
    qToBigEndian<quint32>(static_cast<quint32>(field.size()), length);
    hash.addData(QByteArrayView(length, 4));
    hash.addData(field);
  };

  addField(QByteArray::number(header.format));
  addField(header.minApp.toUtf8());
  addField(header.writer.toUtf8());
  addField(QByteArray::number(header.encoding));
  addField(header.nonce);
  addField(plain);
  return hash.result();
}

//--------------------------------------------------------------------------------------------------
// Encode: validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when every option is a bool, an integer, a bounded string or a string list.
 */
[[nodiscard]] static bool validOptions(const QJsonObject& options)
{
  if (static_cast<quint64>(options.size()) > kMaxOptions)
    return false;

  for (auto it = options.begin(); it != options.end(); ++it) {
    const auto& value = it.value();
    if (it.key().isEmpty() || it.key().size() > kMaxKeyChars)
      return false;

    if (value.isBool() || (value.isString() && value.toString().size() <= kMaxTextChars))
      continue;

    if (value.isDouble() && value.toInteger(0) == value.toInteger(1))
      continue;

    if (!value.isArray() || static_cast<quint64>(value.toArray().size()) > kMaxOptionListItems)
      return false;

    for (const auto& item : value.toArray())
      if (!item.isString() || item.toString().size() > kMaxTextChars)
        return false;
  }

  return true;
}

/**
 * @brief Checks the manifest against the fixed caps; fills error on the first violation.
 */
[[nodiscard]] static bool validManifest(const Manifest& manifest, QString& error)
{
  if (manifest.title.size() > kMaxTextChars || manifest.projectFolder.size() > kMaxTextChars) {
    error = QStringLiteral("title or project folder is too long");
    return false;
  }

  if (manifest.icon.size() > kMaxIconBytes) {
    error = QStringLiteral("icon exceeds 1 MiB");
    return false;
  }

  if (static_cast<quint64>(manifest.externals.size()) > kMaxExternals) {
    error = QStringLiteral("too many external dependencies");
    return false;
  }

  for (const auto& external : manifest.externals) {
    if (external.size() > kMaxTextChars) {
      error = QStringLiteral("external dependency entry is too long");
      return false;
    }
  }

  if (!validOptions(manifest.options)) {
    error = QStringLiteral("runtime options contain an unsupported value");
    return false;
  }

  return true;
}

/**
 * @brief Checks carried files (safe unique paths, count) and credentials against the caps.
 */
[[nodiscard]] static bool validEntries(const Package& package, QString& error)
{
  if (static_cast<quint64>(package.files.size()) > kMaxFiles) {
    error = QStringLiteral("too many carried files");
    return false;
  }

  QSet<QString> seen;
  qsizetype carried = 0;
  for (const auto& file : package.files) {
    if (!isSafeEntryPath(file.path) || seen.contains(file.path)) {
      error = QStringLiteral("invalid or duplicate carried file path: %1").arg(file.path);
      return false;
    }

    carried += file.data.size();
    if (carried > kMaxPackageBytes) {
      error = QStringLiteral("carried files exceed 64 MiB");
      return false;
    }

    seen.insert(file.path);
  }

  if (static_cast<quint64>(package.credentials.size()) > kMaxCredentials) {
    error = QStringLiteral("too many credentials");
    return false;
  }

  for (const auto& credential : package.credentials) {
    const bool keysOk = !credential.target.isEmpty() && credential.target.size() <= kMaxKeyChars
                     && !credential.property.isEmpty()
                     && credential.property.size() <= kMaxKeyChars;
    if (!keysOk || credential.value.size() > kMaxTextChars) {
      error = QStringLiteral("invalid credential entry");
      return false;
    }
  }

  return true;
}

/**
 * @brief Checks the whole package before a single byte is written.
 */
[[nodiscard]] static bool validPackage(const Package& package, QString& error)
{
  const auto minApp = QVersionNumber::fromString(package.minAppVersion);
  if (minApp.isNull() || package.minAppVersion.size() > kMaxKeyChars
      || package.writerVersion.isEmpty() || package.writerVersion.size() > kMaxKeyChars) {
    error = QStringLiteral("invalid minimum or writer version");
    return false;
  }

  if (package.project.isEmpty() || package.project.size() > kMaxProjectBytes) {
    error = QStringLiteral("project document is empty or exceeds 10 MiB");
    return false;
  }

  return validManifest(package.manifest, error) && validEntries(package, error);
}

//--------------------------------------------------------------------------------------------------
// Encode: writers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes one validated option value (bool, integer, string or string list).
 */
static void writeOptionValue(QCborStreamWriter& writer, const QJsonValue& value)
{
  if (value.isBool()) {
    writer.append(value.toBool());
    return;
  }

  if (value.isDouble()) {
    writer.append(static_cast<qint64>(value.toInteger()));
    return;
  }

  if (value.isString()) {
    const auto text = value.toString();
    writer.append(QStringView(text));
    return;
  }

  const auto list = value.toArray();
  writer.startArray(static_cast<quint64>(list.size()));
  for (const auto& item : list) {
    const auto text = item.toString();
    writer.append(QStringView(text));
  }

  writer.endArray();
}

/**
 * @brief Writes the manifest map: title, icon, options, project folder and externals.
 */
static void writeManifest(QCborStreamWriter& writer, const Manifest& manifest)
{
  writer.startMap(5);
  writer.append(PayloadKey::kTitle);
  writer.append(QStringView(manifest.title));
  writer.append(PayloadKey::kIcon);
  writer.append(manifest.icon);
  writer.append(PayloadKey::kOptions);
  writer.startMap(static_cast<quint64>(manifest.options.size()));
  for (auto it = manifest.options.begin(); it != manifest.options.end(); ++it) {
    const auto key = it.key();
    writer.append(QStringView(key));
    writeOptionValue(writer, it.value());
  }

  writer.endMap();
  writer.append(PayloadKey::kProjectFolder);
  writer.append(QStringView(manifest.projectFolder));
  writer.append(PayloadKey::kExternals);
  writer.startArray(static_cast<quint64>(manifest.externals.size()));
  for (const auto& external : manifest.externals)
    writer.append(QStringView(external));

  writer.endArray();
  writer.endMap();
}

/**
 * @brief Writes the carried files and the included credentials as two arrays of maps.
 */
static void writeEntries(QCborStreamWriter& writer, const Package& package)
{
  writer.append(PayloadKey::kFiles);
  writer.startArray(static_cast<quint64>(package.files.size()));
  for (const auto& file : package.files) {
    writer.startMap(2);
    writer.append(PayloadKey::kPath);
    writer.append(QStringView(file.path));
    writer.append(PayloadKey::kData);
    writer.append(file.data);
    writer.endMap();
  }

  writer.endArray();
  writer.append(PayloadKey::kCredentials);
  writer.startArray(static_cast<quint64>(package.credentials.size()));
  for (const auto& credential : package.credentials) {
    writer.startMap(3);
    writer.append(PayloadKey::kTarget);
    writer.append(QStringView(credential.target));
    writer.append(PayloadKey::kProperty);
    writer.append(QStringView(credential.property));
    writer.append(PayloadKey::kValue);
    writer.append(QStringView(credential.value));
    writer.endMap();
  }

  writer.endArray();
}

/**
 * @brief Serializes the payload map (manifest, project, files, credentials) before scrambling.
 */
[[nodiscard]] static QByteArray writePayload(const Package& package)
{
  QByteArray plain;
  {
    QCborStreamWriter writer(&plain);
    writer.startMap(4);
    writer.append(PayloadKey::kManifest);
    writeManifest(writer, package.manifest);
    writer.append(PayloadKey::kProject);
    writer.append(package.project);
    writeEntries(writer, package);
    writer.endMap();
  }

  return plain;
}

/**
 * @brief Serializes the header map from its fields; the digest covers the fields and payload.
 */
[[nodiscard]] static QByteArray writeHeader(const Header& header)
{
  QByteArray bytes;
  {
    QCborStreamWriter writer(&bytes);
    writer.startMap(7);
    writer.append(HeaderKey::kFormat);
    writer.append(header.format);
    writer.append(HeaderKey::kMinApp);
    writer.append(QStringView(header.minApp));
    writer.append(HeaderKey::kWriter);
    writer.append(QStringView(header.writer));
    writer.append(HeaderKey::kEncoding);
    writer.append(header.encoding);
    writer.append(HeaderKey::kNonce);
    writer.append(header.nonce);
    writer.append(HeaderKey::kPlainSize);
    writer.append(header.plainSize);
    writer.append(HeaderKey::kDigest);
    writer.append(header.digest);
    writer.endMap();
  }

  return bytes;
}

/**
 * @brief Builds the 16-byte fixed prefix: magic, container version, reserved, header length.
 */
[[nodiscard]] static QByteArray writePrefix(const qsizetype headerSize)
{
  SS_ASSERT(headerSize > 0 && headerSize <= kMaxHeaderBytes, return {});

  QByteArray prefix(kPrefixBytes, '\0');
  std::memcpy(prefix.data(), kMagic, kMagicBytes);
  qToBigEndian<quint16>(kContainerVersion, prefix.data() + 8);
  qToBigEndian<quint16>(0, prefix.data() + 10);
  qToBigEndian<quint32>(static_cast<quint32>(headerSize), prefix.data() + 12);
  return prefix;
}

//--------------------------------------------------------------------------------------------------
// Decode: bounded primitives
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads a definite-length text string of at most maxChars characters.
 */
[[nodiscard]] static bool readString(QCborStreamReader& reader,
                                     const qsizetype maxChars,
                                     QString& out)
{
  if (!reader.isString() || !reader.isLengthKnown())
    return false;

  if (reader.length() > static_cast<quint64>(maxChars) * 3)
    return false;

  out = reader.readAllString();
  return reader.lastError() == QCborError::NoError && out.size() <= maxChars;
}

/**
 * @brief Reads a definite-length byte string of at most maxBytes bytes.
 */
[[nodiscard]] static bool readBytes(QCborStreamReader& reader,
                                    const qsizetype maxBytes,
                                    QByteArray& out)
{
  if (!reader.isByteArray() || !reader.isLengthKnown())
    return false;

  if (reader.length() > static_cast<quint64>(maxBytes))
    return false;

  out = reader.readAllByteArray();
  return reader.lastError() == QCborError::NoError;
}

/**
 * @brief Reads one integer and advances past it.
 */
[[nodiscard]] static bool readInteger(QCborStreamReader& reader, qint64& out)
{
  if (!reader.isInteger())
    return false;

  out = reader.toInteger();
  return reader.next();
}

/**
 * @brief Enters the current definite-length container when it holds at most maxCount entries.
 */
[[nodiscard]] static bool enterBounded(QCborStreamReader& reader,
                                       const quint64 maxCount,
                                       quint64& count)
{
  if (!reader.isLengthKnown())
    return false;

  count = reader.length();
  if (count > maxCount)
    return false;

  return reader.enterContainer();
}

/**
 * @brief Enters a definite-length map of at most maxCount key/value pairs.
 */
[[nodiscard]] static bool enterMap(QCborStreamReader& reader,
                                   const quint64 maxCount,
                                   quint64& count)
{
  return reader.isMap() && enterBounded(reader, maxCount, count);
}

/**
 * @brief Enters a definite-length array of at most maxCount items.
 */
[[nodiscard]] static bool enterArray(QCborStreamReader& reader,
                                     const quint64 maxCount,
                                     quint64& count)
{
  return reader.isArray() && enterBounded(reader, maxCount, count);
}

/**
 * @brief Reads a map key; a key seen before in the same map is a refusal, so a repeated section
 *        can never stack past the caps it was checked against.
 */
[[nodiscard]] static bool readKey(QCborStreamReader& reader, QSet<QString>& keys, QString& key)
{
  if (!readString(reader, kMaxKeyChars, key) || keys.contains(key))
    return false;

  keys.insert(key);
  return true;
}

/**
 * @brief Skips the value of an unknown key; a text value longer than the text cap is refused
 *        first, because skipping one validates its UTF-8 into memory.
 */
[[nodiscard]] static bool skipUnknown(QCborStreamReader& reader)
{
  if (reader.isString()
      && (!reader.isLengthKnown() || reader.length() > static_cast<quint64>(kMaxTextChars) * 3))
    return false;

  return reader.next(kMaxSkipDepth);
}

/**
 * @brief Leaves the current container once every declared entry has been consumed.
 */
[[nodiscard]] static bool leaveContainer(QCborStreamReader& reader)
{
  if (reader.lastError() != QCborError::NoError || reader.hasNext())
    return false;

  return reader.leaveContainer();
}

/**
 * @brief Reads a definite-length array of bounded strings.
 */
[[nodiscard]] static bool readStringList(QCborStreamReader& reader,
                                         const quint64 maxCount,
                                         const qsizetype maxChars,
                                         QStringList& out)
{
  quint64 count = 0;
  if (!enterArray(reader, maxCount, count))
    return false;

  for (quint64 i = 0; i < count; ++i) {
    QString item;
    if (!readString(reader, maxChars, item))
      return false;

    out.append(item);
  }

  return leaveContainer(reader);
}

//--------------------------------------------------------------------------------------------------
// Decode: header
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads one header key/value pair; unknown keys are skipped with a bounded depth.
 */
[[nodiscard]] static bool readHeaderEntry(QCborStreamReader& reader,
                                          QSet<QString>& keys,
                                          Header& header)
{
  QString key;
  if (!readKey(reader, keys, key))
    return false;

  if (key == HeaderKey::kFormat)
    return readInteger(reader, header.format);

  if (key == HeaderKey::kMinApp)
    return readString(reader, kMaxKeyChars, header.minApp);

  if (key == HeaderKey::kWriter)
    return readString(reader, kMaxKeyChars, header.writer);

  if (key == HeaderKey::kEncoding)
    return readInteger(reader, header.encoding);

  if (key == HeaderKey::kNonce)
    return readBytes(reader, kNonceBytes, header.nonce);

  if (key == HeaderKey::kPlainSize)
    return readInteger(reader, header.plainSize);

  if (key == HeaderKey::kDigest)
    return readBytes(reader, kDigestBytes, header.digest);

  return skipUnknown(reader);
}

/**
 * @brief Parses the header map from its exact byte range.
 */
[[nodiscard]] static bool readHeader(const QByteArray& bytes, Header& header)
{
  QCborStreamReader reader(bytes);
  quint64 count = 0;
  if (!enterMap(reader, kMaxMapEntries, count))
    return false;

  QSet<QString> keys;
  for (quint64 i = 0; i < count; ++i)
    if (!readHeaderEntry(reader, keys, header))
      return false;

  return leaveContainer(reader);
}

//--------------------------------------------------------------------------------------------------
// Decode: payload
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads one runtime option value: bool, integer, bounded string or string list.
 */
[[nodiscard]] static bool readOptionValue(QCborStreamReader& reader, QJsonValue& value)
{
  if (reader.isBool()) {
    value = reader.toBool();
    return reader.next();
  }

  if (reader.isInteger()) {
    value = reader.toInteger();
    return reader.next();
  }

  if (reader.isString()) {
    QString text;
    const bool ok = readString(reader, kMaxTextChars, text);
    value         = text;
    return ok;
  }

  QStringList list;
  const bool ok = readStringList(reader, kMaxOptionListItems, kMaxTextChars, list);
  value         = QJsonArray::fromStringList(list);
  return ok;
}

/**
 * @brief Reads the runtime options map.
 */
[[nodiscard]] static bool readOptions(QCborStreamReader& reader, QJsonObject& options)
{
  quint64 count = 0;
  if (!enterMap(reader, kMaxOptions, count))
    return false;

  QSet<QString> keys;
  for (quint64 i = 0; i < count; ++i) {
    QString key;
    QJsonValue value;
    if (!readKey(reader, keys, key) || !readOptionValue(reader, value))
      return false;

    options.insert(key, value);
  }

  return leaveContainer(reader);
}

/**
 * @brief Reads one manifest key/value pair; unknown keys are skipped.
 */
[[nodiscard]] static bool readManifestEntry(QCborStreamReader& reader,
                                            QSet<QString>& keys,
                                            Manifest& manifest)
{
  QString key;
  if (!readKey(reader, keys, key))
    return false;

  if (key == PayloadKey::kTitle)
    return readString(reader, kMaxTextChars, manifest.title);

  if (key == PayloadKey::kIcon)
    return readBytes(reader, kMaxIconBytes, manifest.icon);

  if (key == PayloadKey::kOptions)
    return readOptions(reader, manifest.options);

  if (key == PayloadKey::kProjectFolder)
    return readString(reader, kMaxTextChars, manifest.projectFolder);

  if (key == PayloadKey::kExternals)
    return readStringList(reader, kMaxExternals, kMaxTextChars, manifest.externals);

  return skipUnknown(reader);
}

/**
 * @brief Reads the manifest map.
 */
[[nodiscard]] static bool readManifest(QCborStreamReader& reader, Manifest& manifest)
{
  quint64 count = 0;
  if (!enterMap(reader, kMaxMapEntries, count))
    return false;

  QSet<QString> keys;
  for (quint64 i = 0; i < count; ++i)
    if (!readManifestEntry(reader, keys, manifest))
      return false;

  return leaveContainer(reader);
}

/**
 * @brief Reads one key/value pair of a file map; unknown keys are skipped.
 */
[[nodiscard]] static bool readFileField(QCborStreamReader& reader,
                                        QSet<QString>& keys,
                                        FileEntry& file)
{
  QString key;
  if (!readKey(reader, keys, key))
    return false;

  if (key == PayloadKey::kPath)
    return readString(reader, kMaxPathChars, file.path);

  if (key == PayloadKey::kData)
    return readBytes(reader, kMaxPackageBytes, file.data);

  return skipUnknown(reader);
}

/**
 * @brief Reads one {path, data} file map; the path must be safe and not seen before.
 */
[[nodiscard]] static bool readFileEntry(QCborStreamReader& reader,
                                        QSet<QString>& seen,
                                        FileEntry& file)
{
  quint64 count = 0;
  if (!enterMap(reader, kMaxMapEntries, count))
    return false;

  QSet<QString> keys;
  for (quint64 i = 0; i < count; ++i)
    if (!readFileField(reader, keys, file))
      return false;

  if (!isSafeEntryPath(file.path) || seen.contains(file.path))
    return false;

  seen.insert(file.path);
  return leaveContainer(reader);
}

/**
 * @brief Reads the carried-files array.
 */
[[nodiscard]] static bool readFiles(QCborStreamReader& reader, QList<FileEntry>& files)
{
  quint64 count = 0;
  if (!enterArray(reader, kMaxFiles, count))
    return false;

  QSet<QString> seen;
  for (const auto& file : files)
    seen.insert(file.path);

  for (quint64 i = 0; i < count; ++i) {
    FileEntry file;
    if (!readFileEntry(reader, seen, file))
      return false;

    files.append(file);
  }

  return leaveContainer(reader) && static_cast<quint64>(files.size()) <= kMaxFiles;
}

/**
 * @brief Reads one key/value pair of a credential map; unknown keys are skipped.
 */
[[nodiscard]] static bool readCredentialField(QCborStreamReader& reader,
                                              QSet<QString>& keys,
                                              Credential& credential)
{
  QString key;
  if (!readKey(reader, keys, key))
    return false;

  if (key == PayloadKey::kTarget)
    return readString(reader, kMaxKeyChars, credential.target);

  if (key == PayloadKey::kProperty)
    return readString(reader, kMaxKeyChars, credential.property);

  if (key == PayloadKey::kValue)
    return readString(reader, kMaxTextChars, credential.value);

  return skipUnknown(reader);
}

/**
 * @brief Reads one {target, property, value} credential map.
 */
[[nodiscard]] static bool readCredential(QCborStreamReader& reader, Credential& credential)
{
  quint64 count = 0;
  if (!enterMap(reader, kMaxMapEntries, count))
    return false;

  QSet<QString> keys;
  for (quint64 i = 0; i < count; ++i)
    if (!readCredentialField(reader, keys, credential))
      return false;

  return leaveContainer(reader) && !credential.target.isEmpty() && !credential.property.isEmpty();
}

/**
 * @brief Reads the included-credentials array.
 */
[[nodiscard]] static bool readCredentials(QCborStreamReader& reader, QList<Credential>& credentials)
{
  quint64 count = 0;
  if (!enterArray(reader, kMaxCredentials, count))
    return false;

  for (quint64 i = 0; i < count; ++i) {
    Credential credential;
    if (!readCredential(reader, credential))
      return false;

    credentials.append(credential);
  }

  return leaveContainer(reader) && static_cast<quint64>(credentials.size()) <= kMaxCredentials;
}

/**
 * @brief Reads one payload key/value pair into the package; unknown keys are skipped.
 */
[[nodiscard]] static bool readPayloadEntry(QCborStreamReader& reader,
                                           QSet<QString>& keys,
                                           Package& package)
{
  QString key;
  if (!readKey(reader, keys, key))
    return false;

  if (key == PayloadKey::kManifest)
    return readManifest(reader, package.manifest);

  if (key == PayloadKey::kProject)
    return readBytes(reader, kMaxProjectBytes, package.project);

  if (key == PayloadKey::kFiles)
    return readFiles(reader, package.files);

  if (key == PayloadKey::kCredentials)
    return readCredentials(reader, package.credentials);

  return skipUnknown(reader);
}

/**
 * @brief Parses the unscrambled payload map; manifest and a non-empty project are required.
 */
[[nodiscard]] static bool readPayload(const QByteArray& plain, Package& package)
{
  QCborStreamReader reader(plain);
  quint64 count = 0;
  if (!enterMap(reader, kMaxMapEntries, count))
    return false;

  QSet<QString> keys;
  for (quint64 i = 0; i < count; ++i)
    if (!readPayloadEntry(reader, keys, package))
      return false;

  if (!leaveContainer(reader))
    return false;

  return keys.contains(PayloadKey::kManifest) && keys.contains(PayloadKey::kProject)
      && !package.project.isEmpty();
}

//--------------------------------------------------------------------------------------------------
// Decode: verdicts
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a decode result, so each decode exit is a single statement.
 */
[[nodiscard]] static DecodeResult verdict(const DecodeStatus status,
                                          const QString& detail,
                                          const QString& requiredVersion = {})
{
  DecodeResult result;
  result.status          = status;
  result.detail          = detail;
  result.requiredVersion = requiredVersion;
  return result;
}

/**
 * @brief Refuses a header that lacks a field v1 always writes.
 */
[[nodiscard]] static bool headerComplete(const Header& header)
{
  return !QVersionNumber::fromString(header.minApp).isNull() && !header.writer.isEmpty()
      && header.nonce.size() == kNonceBytes && header.digest.size() == kDigestBytes
      && header.plainSize >= 0 && header.format >= 1 && header.encoding >= 1;
}

/**
 * @brief The minimum version for the refusal message: only its parsed digits and dots, so
 *        nothing else the header carries reaches the dialog.
 */
[[nodiscard]] static QString displayVersion(const QString& minApp)
{
  return QVersionNumber::fromString(minApp).toString();
}

/**
 * @brief Verdict for a format or encoding this app does not know. A genuinely newer package
 *        declares a newer minimum version; one that claims this version is damaged instead.
 */
[[nodiscard]] static DecodeResult unknownFormat(const Header& header, const QVersionNumber& running)
{
  if (QVersionNumber::compare(QVersionNumber::fromString(header.minApp), running) > 0)
    return verdict(
      DecodeStatus::NeedsNewerApp, QStringLiteral("unknown format"), displayVersion(header.minApp));

  return verdict(DecodeStatus::Corrupt, QStringLiteral("unknown format for its version"));
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief True for a relative path of '/'-separated, non-empty components with no '.', '..',
 *        backslash, colon or control character; anything else could escape the mount root.
 */
bool Deploy::isSafeEntryPath(const QString& path)
{
  if (path.isEmpty() || path.size() > kMaxPathChars)
    return false;

  for (const QChar c : path) {
    const auto code = c.unicode();
    if (code < 0x20 || code == 0x7f || c == QLatin1Char('\\') || c == QLatin1Char(':'))
      return false;
  }

  const auto parts = path.split(QLatin1Char('/'));
  for (const auto& part : parts)
    if (part.isEmpty() || part == QLatin1String(".") || part == QLatin1String(".."))
      return false;

  return true;
}

/**
 * @brief Encodes a package into .ssdeploy bytes, or explains why it cannot be written.
 */
Deploy::EncodeResult Deploy::encode(const Package& package)
{
  EncodeResult result;
  if (!validPackage(package, result.error))
    return result;

  const auto plain = writePayload(package);
  Header fields;
  fields.format    = kFormatVersion;
  fields.encoding  = kEncodingScrambleV1;
  fields.plainSize = plain.size();
  fields.minApp    = package.minAppVersion;
  fields.writer    = package.writerVersion;
  fields.nonce     = makeNonce();
  fields.digest    = contentDigest(fields, plain);

  const auto header = writeHeader(fields);
  if (header.size() > kMaxHeaderBytes) {
    result.error = QStringLiteral("package header exceeds 64 KiB");
    return result;
  }

  const auto total = kPrefixBytes + header.size() + plain.size();
  if (total > kMaxPackageBytes) {
    result.error = QStringLiteral("package exceeds 64 MiB");
    return result;
  }

  QByteArray bytes;
  bytes.reserve(total);
  bytes.append(writePrefix(header.size()));
  bytes.append(header);
  bytes.append(scramble(plain, fields.nonce));
  SS_ASSERT(bytes.size() == total, return result);

  result.ok    = true;
  result.bytes = bytes;
  return result;
}

/**
 * @brief Decodes .ssdeploy bytes. Every length is checked against a fixed cap before anything
 *        is read, and the digest is verified before any version claim is believed or the payload
 *        is parsed.
 */
Deploy::DecodeResult Deploy::decode(const QByteArray& bytes, const QVersionNumber& runningVersion)
{
  if (bytes.size() > kMaxPackageBytes)
    return verdict(DecodeStatus::TooLarge, QStringLiteral("file exceeds 64 MiB"));

  if (bytes.size() < kPrefixBytes || std::memcmp(bytes.constData(), kMagic, kMagicBytes) != 0)
    return verdict(DecodeStatus::NotAPackage, QStringLiteral("missing magic"));

  if (qFromBigEndian<quint16>(bytes.constData() + 8) != kContainerVersion)
    return verdict(DecodeStatus::NeedsNewerApp, QStringLiteral("unknown container version"));

  if (qFromBigEndian<quint16>(bytes.constData() + 10) != 0)
    return verdict(DecodeStatus::Corrupt, QStringLiteral("reserved bytes set"));

  const auto headerSize = static_cast<qsizetype>(qFromBigEndian<quint32>(bytes.constData() + 12));
  if (headerSize == 0 || headerSize > kMaxHeaderBytes || headerSize > bytes.size() - kPrefixBytes)
    return verdict(DecodeStatus::Corrupt, QStringLiteral("bad header length"));

  Header header;
  if (!readHeader(bytes.mid(kPrefixBytes, headerSize), header))
    return verdict(DecodeStatus::Corrupt, QStringLiteral("malformed header"));

  if (!headerComplete(header))
    return verdict(DecodeStatus::Corrupt, QStringLiteral("incomplete header"));

  if (header.format != kFormatVersion || header.encoding != kEncodingScrambleV1)
    return unknownFormat(header, runningVersion);

  const auto payload = bytes.mid(kPrefixBytes + headerSize);
  if (payload.size() != header.plainSize)
    return verdict(DecodeStatus::Corrupt, QStringLiteral("payload length mismatch"));

  const auto plain = scramble(payload, header.nonce);
  if (contentDigest(header, plain) != header.digest)
    return verdict(DecodeStatus::Corrupt, QStringLiteral("digest mismatch"));

  if (QVersionNumber::compare(QVersionNumber::fromString(header.minApp), runningVersion) > 0)
    return verdict(
      DecodeStatus::NeedsNewerApp, QStringLiteral("newer app"), displayVersion(header.minApp));

  auto result = verdict(DecodeStatus::Ok, QString());
  if (!readPayload(plain, result.package))
    return verdict(DecodeStatus::Corrupt, QStringLiteral("malformed payload"));

  result.package.minAppVersion = header.minApp;
  result.package.writerVersion = header.writer;
  return result;
}

#endif  // BUILD_COMMERCIAL
