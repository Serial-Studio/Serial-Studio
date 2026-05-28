/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#  include <CoreFoundation/CoreFoundation.h>
#  include <QByteArray>
#  include <Security/Security.h>

#  include "Platform/SecretStore.h"

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wraps a QString as a CFString the caller must CFRelease.
 */
static CFStringRef toCFString(const QString& value)
{
  const auto utf8 = value.toUtf8();
  return CFStringCreateWithBytes(kCFAllocatorDefault,
                                 reinterpret_cast<const UInt8*>(utf8.constData()),
                                 utf8.size(),
                                 kCFStringEncodingUTF8,
                                 false);
}

/**
 * @brief Builds the base generic-password query identifying one secret.
 */
static CFMutableDictionaryRef baseQuery(const QString& service, const QString& account)
{
  auto query = CFDictionaryCreateMutable(
    kCFAllocatorDefault, 5, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);

  // Data-protection keychain: no per-item ACL, so no prompt across rebuilds
  CFDictionarySetValue(query, kSecUseDataProtectionKeychain, kCFBooleanTrue);

  const auto svc = toCFString(service);
  const auto acc = toCFString(account);
  CFDictionarySetValue(query, kSecAttrService, svc);
  CFDictionarySetValue(query, kSecAttrAccount, acc);
  CFRelease(svc);
  CFRelease(acc);
  return query;
}

//--------------------------------------------------------------------------------------------------
// SecretStore (macOS Keychain Services)
//--------------------------------------------------------------------------------------------------

/**
 * @brief The login keychain is always available on macOS.
 */
bool Platform::SecretStore::available()
{
  return true;
}

/**
 * @brief Adds or updates a generic-password keychain item for the given secret.
 */
bool Platform::SecretStore::store(const QString& service,
                                  const QString& account,
                                  const QString& secret)
{
  const auto data = secret.toUtf8();
  auto value      = CFDataCreate(
    kCFAllocatorDefault, reinterpret_cast<const UInt8*>(data.constData()), data.size());

  auto query = baseQuery(service, account);
  CFDictionarySetValue(query, kSecValueData, value);

  // Device-local, readable after first unlock: no iCloud sync, no read prompt
  CFDictionarySetValue(query, kSecAttrAccessible, kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly);

  auto status = SecItemAdd(query, nullptr);
  if (status == errSecDuplicateItem) {
    CFDictionaryRemoveValue(query, kSecValueData);
    CFDictionaryRemoveValue(query, kSecAttrAccessible);
    auto update = CFDictionaryCreateMutable(
      kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(update, kSecValueData, value);
    CFDictionarySetValue(
      update, kSecAttrAccessible, kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly);
    status = SecItemUpdate(query, update);
    CFRelease(update);
  }

  CFRelease(value);
  CFRelease(query);
  return status == errSecSuccess;
}

/**
 * @brief Returns the secret stored under the given service/account, if present.
 */
std::optional<QString> Platform::SecretStore::retrieve(const QString& service,
                                                       const QString& account)
{
  auto query = baseQuery(service, account);
  CFDictionarySetValue(query, kSecReturnData, kCFBooleanTrue);
  CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);

  CFTypeRef result  = nullptr;
  const auto status = SecItemCopyMatching(query, &result);
  CFRelease(query);

  if (status != errSecSuccess || result == nullptr)
    return std::nullopt;

  const auto data      = reinterpret_cast<CFDataRef>(result);
  const QString secret = QString::fromUtf8(reinterpret_cast<const char*>(CFDataGetBytePtr(data)),
                                           static_cast<qsizetype>(CFDataGetLength(data)));
  CFRelease(result);
  return secret;
}

/**
 * @brief Deletes the keychain item for the given service/account.
 */
bool Platform::SecretStore::remove(const QString& service, const QString& account)
{
  auto query        = baseQuery(service, account);
  const auto status = SecItemDelete(query);
  CFRelease(query);
  return status == errSecSuccess || status == errSecItemNotFound;
}

#endif  // BUILD_COMMERCIAL
