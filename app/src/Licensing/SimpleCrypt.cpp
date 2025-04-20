/*
 * Copyright (c) 2011, Andre Somers
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Rathenau Instituut, Andre Somers nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL ANDRE SOMERS BE LIABLE FOR AN DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR #######;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Licensing/SimpleCrypt.h"

#include <QIODevice>
#include <QDateTime>
#include <QByteArray>
#include <QDataStream>
#include <QRandomGenerator>
#include <QCryptographicHash>

//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

/**
 * Constructs a SimpleCrypt instance without a valid key set on it.
 */
Licensing::SimpleCrypt::SimpleCrypt()
  : m_key(0)
  , m_lastError(ErrorNoError)
  , m_compressionMode(CompressionAuto)
  , m_protectionMode(ProtectionChecksum)
{
}

/**
 * Constructs a SimpleCrypt instance and initializes it with the given key.
 */
Licensing::SimpleCrypt::SimpleCrypt(quint64 key)
  : m_key(key)
  , m_lastError(ErrorNoError)
  , m_compressionMode(CompressionAuto)
  , m_protectionMode(ProtectionChecksum)
{
  splitKey();
}

//------------------------------------------------------------------------------
// Property getters
//------------------------------------------------------------------------------

/**
 * Returns true if SimpleCrypt has been initialized with a key.
 */
bool Licensing::SimpleCrypt::hasKey() const
{
  return !m_keyParts.isEmpty();
}

/**
 * Returns the last error that occurred.
 */
Licensing::SimpleCrypt::Error Licensing::SimpleCrypt::lastError() const
{
  return m_lastError;
}

/**
 * Returns the CompressionMode that is currently in use.
 */
Licensing::SimpleCrypt::CompressionMode
Licensing::SimpleCrypt::compressionMode() const
{
  return m_compressionMode;
}

/**
 * Returns the IntegrityProtectionMode that is currently in use.
 */
Licensing::SimpleCrypt::IntegrityProtectionMode
Licensing::SimpleCrypt::integrityProtectionMode() const
{
  return m_protectionMode;
}

//------------------------------------------------------------------------------
// Property setters
//------------------------------------------------------------------------------

/**
 * (Re-) initializes the key with the given @arg key.
 */
void Licensing::SimpleCrypt::setKey(quint64 key)
{
  m_key = key;
  splitKey();
}

/**
 * Sets the compression mode to use when encrypting data. The default mode is
 * Auto.
 *
 * Note that decryption is not influenced by this mode, as the decryption
 * recognizes what mode was used when encrypting.
 */
void Licensing::SimpleCrypt::setCompressionMode(CompressionMode mode)
{
  m_compressionMode = mode;
}

/**
 * Sets the integrity mode to use when encrypting data. The default mode is
 * Checksum.
 *  * Note that decryption is not influenced by this mode, as the decryption
 * recognizes what mode was used when encrypting.
 */
void Licensing::SimpleCrypt::setIntegrityProtectionMode(
    IntegrityProtectionMode mode)
{
  m_protectionMode = mode;
}

//------------------------------------------------------------------------------
// Data encryption
//------------------------------------------------------------------------------

/**
 * Encrypts the @arg plaintext string with the key the class was initialized
 * with, and returns a cyphertext the result. The result is a base64 encoded
 * version of the binary array that is the actual result of the string, so it
 * can be stored easily in a text format.
 */
QString Licensing::SimpleCrypt::encryptToString(const QString &plaintext)
{
  QByteArray plaintextArray = plaintext.toUtf8();
  QByteArray cypher = encryptToByteArray(plaintextArray);
  QString cypherString = QString::fromLatin1(cypher.toBase64());
  return cypherString;
}

/**
 * Encrypts the @arg plaintext QByteArray with the key the class was
 * initialized with, and returns a cyphertext the result. The result is a
 * base64 encoded version of the binary array that is the actual result of the
 * encryption, so it can be stored easily in a text format.
 */
QString Licensing::SimpleCrypt::encryptToString(const QByteArray &plaintext)
{
  QByteArray cypher = encryptToByteArray(plaintext);
  QString cypherString = QString::fromLatin1(cypher.toBase64());
  return cypherString;
}

/**
 * Encrypts the @arg plaintext string with the key the class was initialized
 * with, and returns a binary cyphertext in a QByteArray the result.
 *
 * This method returns a byte array, that is useable for storing a binary
 * format. If you need a string you can store in a text file, use
 * encryptToString() instead.
 */
QByteArray Licensing::SimpleCrypt::encryptToByteArray(const QString &plaintext)
{
  QByteArray plaintextArray = plaintext.toUtf8();
  return encryptToByteArray(plaintextArray);
}

/**
 * Encrypts the @arg plaintext QByteArray with the key the class was
 * initialized with, and returns a binary cyphertext in a QByteArray the
 * result.
 *
 * This method returns a byte array, that is useable for storing a binary
 * format. If you need a string you can store in a text file, use
 * encryptToString() instead.
 */
QByteArray
Licensing::SimpleCrypt::encryptToByteArray(const QByteArray &plaintext)
{
  if (m_keyParts.isEmpty())
  {
    qWarning() << "No key set.";
    m_lastError = ErrorNoKeySet;
    return QByteArray();
  }

  QByteArray ba = plaintext;

  CryptoFlags flags = CryptoFlagNone;
  if (m_compressionMode == CompressionAlways)
  {
    ba = qCompress(ba, 9);
    flags |= CryptoFlagCompression;
  }

  else if (m_compressionMode == CompressionAuto)
  {
    QByteArray compressed = qCompress(ba, 9);
    if (compressed.size() < ba.size())
    {
      ba = compressed;
      flags |= CryptoFlagCompression;
    }
  }

  QByteArray integrityProtection;
  if (m_protectionMode == ProtectionChecksum)
  {
    flags |= CryptoFlagChecksum;
    QDataStream s(&integrityProtection, QIODevice::WriteOnly);
    s << qChecksum(ba, Qt::ChecksumIso3309);
  }

  else if (m_protectionMode == ProtectionHash)
  {
    flags |= CryptoFlagHash;
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(ba);

    integrityProtection += hash.result();
  }

  // prepend a random char to the string
  char randomChar = char(QRandomGenerator::securelySeeded().generate() & 0xFF);
  ba = randomChar + integrityProtection + ba;

  int pos(0);
  char lastChar(0);

  int cnt = ba.size();

  while (pos < cnt)
  {
    ba[pos] = ba.at(pos) ^ m_keyParts.at(pos % 8) ^ lastChar;
    lastChar = ba.at(pos);
    ++pos;
  }

  QByteArray resultArray;
  resultArray.append(char(0x03));
  resultArray.append(char(flags));
  resultArray.append(ba);

  m_lastError = ErrorNoError;
  return resultArray;
}

//------------------------------------------------------------------------------
// Data decryption
//------------------------------------------------------------------------------

/**
 * Decrypts a cyphertext binary encrypted with this class with the set key
 * back to the plain text version.
 *
 * If an error occured, such as non-matching keys between encryption and
 * decryption, an empty string or a string containing nonsense may be
 * returned.
 */
QString Licensing::SimpleCrypt::decryptToString(const QByteArray &cypher)
{
  QByteArray ba = decryptToByteArray(cypher);
  QString plaintext = QString::fromUtf8(ba, ba.size());

  return plaintext;
}

/**
 * Decrypts a cyphertext string encrypted with this class with the set key
 * back to the plain text version.
 *
 * If an error occured, such as non-matching keys between encryption and
 * decryption, an empty string or a string containing nonsense may be
 * returned.
 */
QString Licensing::SimpleCrypt::decryptToString(const QString &cyphertext)
{
  QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
  QByteArray plaintextArray = decryptToByteArray(cyphertextArray);
  QString plaintext = QString::fromUtf8(plaintextArray, plaintextArray.size());

  return plaintext;
}

/**
 * Decrypts a cyphertext binary encrypted with this class with the set key
 * back to the plain text version.
 *
 * If an error occured, such as non-matching keys between encryption and
 * decryption, an empty string or a string containing nonsense may be
 * returned.
 */
QByteArray Licensing::SimpleCrypt::decryptToByteArray(const QByteArray &cypher)
{
  if (m_keyParts.isEmpty())
  {
    qWarning() << "No key set.";
    m_lastError = ErrorNoKeySet;
    return QByteArray();
  }

  QByteArray ba = cypher;

  if (cypher.size() < 3)
    return QByteArray();

  char version = ba.at(0);

  // we only work with version 3
  if (version != 3)
  {
    m_lastError = ErrorUnknownVersion;
    return QByteArray();
  }

  CryptoFlags flags = CryptoFlags(ba.at(1));

  ba = ba.mid(2);
  int pos(0);
  int cnt(ba.size());
  char lastChar = 0;

  while (pos < cnt)
  {
    char currentChar = ba[pos];
    ba[pos] = ba.at(pos) ^ lastChar ^ m_keyParts.at(pos % 8);
    lastChar = currentChar;
    ++pos;
  }

  // chop off the random number at the start
  ba = ba.mid(1);

  bool integrityOk(true);
  if (flags.testFlag(CryptoFlagChecksum))
  {
    if (ba.length() < 2)
    {
      m_lastError = ErrorIntegrityFailed;
      return QByteArray();
    }
    quint16 storedChecksum;
    {
      QDataStream s(&ba, QIODevice::ReadOnly);
      s >> storedChecksum;
    }

    ba = ba.mid(2);
    quint16 checksum = qChecksum(ba, Qt::ChecksumIso3309);
    integrityOk = (checksum == storedChecksum);
  }

  else if (flags.testFlag(CryptoFlagHash))
  {
    if (ba.length() < 20)
    {
      m_lastError = ErrorIntegrityFailed;
      return QByteArray();
    }

    QByteArray storedHash = ba.left(20);
    ba = ba.mid(20);
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(ba);
    integrityOk = (hash.result() == storedHash);
  }

  if (!integrityOk)
  {
    m_lastError = ErrorIntegrityFailed;
    return QByteArray();
  }

  if (flags.testFlag(CryptoFlagCompression))
    ba = qUncompress(ba);

  m_lastError = ErrorNoError;
  return ba;
}

/**
 * Decrypts a cyphertext string encrypted with this class with the set key
 * back to the plain text version.
 *
 * If an error occured, such as non-matching keys between encryption and
 * decryption, an empty string or a string containing nonsense may be
 * returned.
 */
QByteArray Licensing::SimpleCrypt::decryptToByteArray(const QString &cyphertext)
{
  QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
  QByteArray ba = decryptToByteArray(cyphertextArray);

  return ba;
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

/**
 * @brief Splits the key into 8 bytes, extracting each subsequent lower-order
 *        byte.
 *
 * This function processes the internal `m_key` variable, breaking it down into
 * an array of 8 characters. Each character corresponds to a byte from `m_key`,
 * extracted in reverse order (from highest-order byte to lowest).
 */
void Licensing::SimpleCrypt::splitKey()
{
  m_keyParts.clear();
  m_keyParts.resize(8);

  for (int i = 0; i < 8; i++)
  {
    quint64 part = m_key;
    for (int j = i; j > 0; j--)
      part = part >> 8;

    part = part & 0xff;
    m_keyParts[i] = static_cast<char>(part);
  }
}
