/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file cryptoutil.h
 * \brief Checksum and other encryption routines.
 *
 * The file is a wrapper around the OpenSSL library and simplifies the usage of
 * encryption and hash checksums as MD5.
 */

#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace mdf {

/** \brief Generates an MD5 file checksum (32 character) string.
 *
 * Creates an MD5 file checksum. The output is a hexadecimal string.
 * @param [in] file Full path to the file name.
 * @return A 32 (hex) character string on success else an empty string.
 */
std::string CreateMd5FileString(const std::string& file);

/** \brief Create a file hash byte array according to MD5
 *
 * Generates a file hash MD5 checksum. The output is a byte array.
 * @param [in] file  Full path to the file name.
 * @param [out] md5 Returns a byte array. The vector size is set by the
 * function.
 * @return True on success.
 */
bool CreateMd5FileChecksum(const std::string& file, std::vector<uint8_t>& md5);

}  // namespace mdf
