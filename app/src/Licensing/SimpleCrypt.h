// SPDX-License-Identifier: BSD-3-Clause
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

namespace Licensing {
/**
 * @brief Simple (non-strong) encryption and decryption of strings and byte arrays.
 */
class SimpleCrypt {
public:
  enum CompressionMode {
    CompressionAuto,
    CompressionAlways,
    CompressionNever
  };

  enum IntegrityProtectionMode {
    ProtectionNone,
    ProtectionChecksum,
    ProtectionHash
  };

  enum Error {
    ErrorNoError,
    ErrorNoKeySet,
    ErrorUnknownVersion,
    ErrorIntegrityFailed,
  };

  enum CryptoFlag {
    CryptoFlagNone        = 0,
    CryptoFlagCompression = 0x01,
    CryptoFlagChecksum    = 0x02,
    CryptoFlagHash        = 0x04
  };
  Q_DECLARE_FLAGS(CryptoFlags, CryptoFlag)

  SimpleCrypt();
  explicit SimpleCrypt(quint64 key);

  [[nodiscard]] bool hasKey() const;
  [[nodiscard]] Error lastError() const;
  [[nodiscard]] CompressionMode compressionMode() const;
  [[nodiscard]] IntegrityProtectionMode integrityProtectionMode() const;

  [[nodiscard]] QString encryptToString(const QString& plaintext);
  [[nodiscard]] QString encryptToString(const QByteArray& plaintext);
  [[nodiscard]] QByteArray encryptToByteArray(const QString& plaintext);
  [[nodiscard]] QByteArray encryptToByteArray(const QByteArray& plaintext);

  [[nodiscard]] QString decryptToString(const QByteArray& cypher);
  [[nodiscard]] QString decryptToString(const QString& cyphertext);
  [[nodiscard]] QByteArray decryptToByteArray(const QByteArray& cypher);
  [[nodiscard]] QByteArray decryptToByteArray(const QString& cyphertext);

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
}  // namespace Licensing
