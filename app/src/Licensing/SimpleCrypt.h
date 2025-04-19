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

#pragma once

#include <QFlags>
#include <QString>
#include <QVector>

namespace Licensing
{
/**
 * @short Simple encryption and decryption of strings and byte arrays
 *
 * This class provides a simple implementation of encryption and decryption of
 * strings and byte arrays.
 *
 * @warning The encryption provided by this class is NOT strong encryption.
 * It may help to shield things from curious eyes, but it will NOT stand up to
 * someone determined to break the encryption. Don't say you were not warned.
 *
 * The class uses a 64 bit key. Simply create an instance of the class, set the
 * key, and use the encryptToString() method to calculate an encrypted version
 * of the input string. To decrypt that string again, use an instance of
 * SimpleCrypt initialized with the same key, and call the decryptToString()
 * method with the encrypted string. If the key matches, the decrypted version
 * of the string will be returned again.
 *
 * If you do not provide a key, or if something else is wrong, the encryption
 * and decryption function will return an empty string or will return a string
 * containing nonsense. lastError() will return a value indicating if the method
 * was succesful, and if not, why not.
 *
 * SimpleCrypt is prepared for the case that the encryption and decryption
 * algorithm is changed in a later version, by prepending a version identifier
 * to the cypertext.
 */
class SimpleCrypt
{
public:
  /**
   * CompressionMode describes if compression will be applied to the data to be
   * encrypted.
   */
  enum CompressionMode
  {
    CompressionAuto,
    CompressionAlways,
    CompressionNever
  };

  /**
   * IntegrityProtectionMode describes measures taken to make it possible to
   * detect problems with the data or wrong decryption keys.
   *
   * Measures involve adding a checksum or a cryptograhpic hash to the data to
   * be encrypted. This increases the length of the resulting cypertext, but
   * makes it possible to check if the plaintext appears to be valid after
   * decryption.
   */
  enum IntegrityProtectionMode
  {
    ProtectionNone,
    ProtectionChecksum,
    ProtectionHash
  };

  /**
   * Error describes the type of error that occured.
   */
  enum Error
  {
    ErrorNoError,
    ErrorNoKeySet,
    ErrorUnknownVersion,
    ErrorIntegrityFailed,
  };

  /**
   * enum to describe options that have been used for the encryption.
   * Currently only one, but that leaves room for future extensions like
   * adding a cryptographic hash...
   */
  enum CryptoFlag
  {
    CryptoFlagNone = 0,
    CryptoFlagCompression = 0x01,
    CryptoFlagChecksum = 0x02,
    CryptoFlagHash = 0x04
  };
  Q_DECLARE_FLAGS(CryptoFlags, CryptoFlag);

  SimpleCrypt();
  explicit SimpleCrypt(quint64 key);

  [[nodiscard]] bool hasKey() const;
  [[nodiscard]] Error lastError() const;
  [[nodiscard]] CompressionMode compressionMode() const;
  [[nodiscard]] IntegrityProtectionMode integrityProtectionMode() const;

  [[nodiscard]] QString encryptToString(const QString &plaintext);
  [[nodiscard]] QString encryptToString(const QByteArray &plaintext);
  [[nodiscard]] QByteArray encryptToByteArray(const QString &plaintext);
  [[nodiscard]] QByteArray encryptToByteArray(const QByteArray &plaintext);

  [[nodiscard]] QString decryptToString(const QByteArray &cypher);
  [[nodiscard]] QString decryptToString(const QString &cyphertext);
  [[nodiscard]] QByteArray decryptToByteArray(const QByteArray &cypher);
  [[nodiscard]] QByteArray decryptToByteArray(const QString &cyphertext);

  void setKey(quint64 key);
  void setCompressionMode(CompressionMode mode);
  void setIntegrityProtectionMode(IntegrityProtectionMode mode);

private:
  void splitKey();

  quint64 m_key;
  Error m_lastError;
  QVector<char> m_keyParts;
  CompressionMode m_compressionMode;
  IntegrityProtectionMode m_protectionMode;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(SimpleCrypt::CryptoFlags)
} // namespace Licensing
