/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file ilogger.h
 * \brief Defines an interface against a generic logger.
 */
#pragma once
#include <array>
#include <atomic>
#include <string>

#include "logmessage.h"

namespace util::log {

/** \enum LogType
 * \ brief Defines type of pre-defined loggers in the system
 *
 */
enum class LogType {
  LogNothing = 0,  ///< No logger.
  LogToConsole,    ///< Log to the cout stream.
  LogToFile,       ///< Log to file.
  LogToListen,     ///< Log to listen window (system messages).
  LogToSyslog      ///< Logs to a syslog server.
};

/** \class ILogger ilogger.h "ilogger.h"
 * \brief Interface against a generic logger.
 *
 * The class is an interface class for implementing a logger. Its main interface
 * is the AddLogMessage() function that handles incoming log messages.
 */
class ILogger {
 public:
  virtual ~ILogger() = default;  ///< Destructor
  virtual void AddLogMessage(
      const LogMessage &message) = 0;  ///< Handle a log message
  virtual void Stop();                 ///< Stops any worker thread

  /** \brief Enable or disable a severity
   *
   * Enables or disable a log severity. By default all log messages are shown
   * but some logger type may disable some severity messages. Typical is the
   * trace level disabled. Disable a severity level means that it isn't saved or
   * sent further by this logger.
   * @param severity Severity Level
   * @param enable True to enable, false to disable.
   */
  void EnableSeverityLevel(LogSeverity severity, bool enable);

  /** \brief Checks if a level is enabled or disabled.
   *
   * Checks if a severity level is enabled or disabled. If it is
   * disbaled, the message should not be dropped by the logger.
   * @param severity Severity Level
   * @return True if enabled or false if disabled.
   */
  [[nodiscard]] bool IsSeverityLevelEnabled(LogSeverity severity) const;

  /** \brief Enable or disable the source location information
   *
   * The source location normally appended to the log message. It logs the
   * file, function, line and column where the message was generated. For some
   * loggers, this information is not needed.
   * @param show True if the source location should be stored.
   */
  void ShowLocation(bool show) { show_location_ = show; }

  /** \brief Returns true if the source location should be shown.
   *
   * Return true if the source location should be included by the
   * logger.
   * @return True if source location should be included.
   */
  [[nodiscard]] bool ShowLocation() const { return show_location_; }

  [[nodiscard]] virtual bool HasLogFile()
      const;  ///< Returns true if the logger has  file.
  [[nodiscard]] virtual std::string Filename()
      const;  ///< Return full path to the log file.
 protected:
  ILogger() = default;  ///< Constructor
 private:
  std::array<std::atomic<bool>, 9> severity_filter_ = {
      true, true, true, true, true, true, true, true, true};
  std::atomic<bool> show_location_ = true;
};

}  // namespace util::log
