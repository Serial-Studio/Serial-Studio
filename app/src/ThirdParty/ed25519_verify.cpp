/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "ThirdParty/ed25519_verify.h"

#include <cstdlib>
#include <cstring>
#include <vector>

//--------------------------------------------------------------------------------------------------
// Vendored TweetNaCl entry point (C linkage; see tweetnacl.c)
//--------------------------------------------------------------------------------------------------

extern "C" int crypto_sign_ed25519_tweet_open(unsigned char* m,
                                              unsigned long long* mlen,
                                              const unsigned char* sm,
                                              unsigned long long n,
                                              const unsigned char* pk);

/**
 * @brief Entropy hook referenced by TweetNaCl's key-generation paths, which Serial Studio
 *        never calls (offline license verification is signature-check only); the symbol must
 *        exist for the link, and reaching it would be a logic error, so it aborts.
 */
extern "C" void randombytes(unsigned char*, unsigned long long)
{
  std::abort();
}

//--------------------------------------------------------------------------------------------------
// Detached verification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Verifies a detached Ed25519 signature over message via TweetNaCl.
 *
 * TweetNaCl only exposes an "attached" verifier (signature || message), so we
 * reconstruct that framing, run the open, and confirm the recovered length
 * matches the input to reject any truncation.
 */
bool ed25519_verify(const std::uint8_t signature[64],
                    const std::uint8_t* message,
                    std::size_t length,
                    const std::uint8_t publicKey[32])
{
  const std::size_t signedLength = 64 + length;
  std::vector<unsigned char> signedMessage(signedLength);
  std::vector<unsigned char> recovered(signedLength);

  std::memcpy(signedMessage.data(), signature, 64);
  if (length > 0)
    std::memcpy(signedMessage.data() + 64, message, length);

  unsigned long long recoveredLength = 0;
  const int rc                       = crypto_sign_ed25519_tweet_open(recovered.data(),
                                                &recoveredLength,
                                                signedMessage.data(),
                                                static_cast<unsigned long long>(signedLength),
                                                publicKey);

  return rc == 0 && recoveredLength == length;
}
