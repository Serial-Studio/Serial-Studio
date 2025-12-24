/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file mdffactory.h
 * \brief Factory class for the MDF library.
 */
#pragma once
#include <functional>
#include <cstdint>
#include <string>
#include <memory>

#include "mdf/mdfwriter.h"
#include "mdf/ichannelobserver.h"

namespace mdf {


/** \brief MDF file type. */
enum class MdfFileType : int {
  Mdf3FileType = 0, ///< MDF version 3 file.
  Mdf4FileType = 1  ///< MDF version 4 file.
};

/** \brief Defines the log severity level. */
enum class MdfLogSeverity : uint8_t {
  kTrace = 0,  ///< Trace or listen message
  kDebug,      ///< Debug message
  kInfo,       ///< Informational message
  kNotice,     ///< Notice message. Notify the user.
  kWarning,    ///< Warning message
  kError,      ///< Error message
  kCritical,   ///< Critical message (device error)
  kAlert,      ///< Alert or alarm message
  kEmergency   ///< Fatal error message
};

/** \brief MDF logging function definition. */
using MdfLogFunction2 = std::function<void(MdfLogSeverity severity,
  const std::string& function, const std::string& text)>;

class MdfWriter;
class MdfFile;
class IChannelObserver;

/** \brief MDF factory class. */
class MdfFactory {
 public:
  /** \brief Creates an MDF writer object. */
  static std::unique_ptr<MdfWriter> CreateMdfWriter(MdfWriterType type);
  /** \brief Create an MDF file object.*/
  static std::unique_ptr<MdfFile> CreateMdfFile(MdfFileType type);

  /** \brief Creates an MDF writer object. */
  static MdfWriter* CreateMdfWriterEx(MdfWriterType type);
  /** \brief Create an MDF file object.*/
  static MdfFile* CreateMdfFileEx(MdfFileType type);

  /** \brief Sets the log function. */
  static void SetLogFunction2(const MdfLogFunction2& func);

  static std::unique_ptr<IChannelObserver> CreateChannelObserver(
    const IDataGroup& data_group, const IChannelGroup& channel_group,
    const IChannel& channel);

};

}  // namespace mdf