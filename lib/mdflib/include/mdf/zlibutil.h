/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file zlibutil.h
 * \brief Various ZLIB compress functions.
 */

#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace mdf {

using ByteArray = std::vector<uint8_t>;  ///< Defines a dynamic byte array

/**
 * Compress a file directly to another file. This is an efficient way of
 * compressing a file. The compress method is deflate according to ZLIB.
 * @param in Pointer to the input file stream.
 * @param out Pointer to the output file stream.
 * @return True on success.
 */
bool Deflate(std::streambuf& in, std::streambuf& out);  ///< Compress file to file.

/**
 * Compress a byte array directly to another array.
 *
 * The input buffer data is compressed and sent to the output array.
 * The output array needs be sized to a size so the compressed data fits.
 * Normally it should be preset to the same size as the input.
 * The output array is resized to the new compressed size.
 *
 * The compress method is deflate according to ZLIB.
 * @param buf_in Input byte array with data.
 * @param buf_out Output array.
 * @return True on success.
 */
bool Deflate(const ByteArray& buf_in,
             ByteArray& buf_out);  ///< Compress array to array.

/**
 * Compress a file directly to an array.
 *
 * The file data is compressed and sent to an output array. Unlike the other
 * Deflate() functions, the output buffer doesn't need to be sized before
 * calling this function. The compress method is deflate according to ZLIB.
 *
 * @param filename Full name of the file to compress.
 * @param buf_out Output byte array. The function resize the buffer
 * @return True on success.
 */
bool Deflate(const std::string& filename,
             ByteArray& buf_out);  ///< Compress array to array.

bool Inflate(std::streambuf& in, std::streambuf& out);  ///< Decompress file to file.
bool Inflate(std::streambuf& in, std::streambuf& out,
             uint64_t nof_bytes);  ///< Decompress part of file to file
bool Inflate(const ByteArray& in,
             ByteArray& out);  ///< Decompress array to array.
bool Inflate(const ByteArray& in,
             std::streambuf& out);  ///< Decompress array to file.

void Transpose(ByteArray& data,
               size_t record_size);  ///< Transpose of an array.
void InvTranspose(ByteArray& data,
                  size_t record_size);  ///< Invert transpose of an array.

}  // namespace mdf
