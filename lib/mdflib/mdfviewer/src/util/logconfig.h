/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file logconfig.h
 * \brief Singleton that is used to configure default loggers.
 */
#pragma once
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ilogger.h"
#include "logmessage.h"
#include "stringutil.h"

namespace util::log {

/**
 * Utility function that returns the default program data path
 * @return Typical 'c:/programdata' in Windows.
 */
std::string ProgramDataPath();

/** \class LogConfig logconfig.h "logconfig.h
 * \brief Singleton class that is used for configure the default logger in an
 * application.
 *
 * The class is used to configure the default logger for an application. There
 * is basically 2 default logger to choose between. For large applications, it's
 * recommended to log messages onto a file while simpler applications typically
 * those without a GUI, may log all message onto the cout stream.
 *
 * Note that you may add many loggers to the application. Normally is the
 * default logger good enough but more advanced system may add more logger
 * interfaces.
 *
 * Usage (snippets from mdfviewer.cpp):
 * \code
 * // Set up the log file.
 * // The log file will be in %TEMP%/report_server/mdf_viewer.log
 * auto& log_config = LogConfig::Instance();
 * log_config.Type(LogType::LogToFile);
 * log_config.SubDir("report_server");
 * log_config.BaseName("mdf_viewer");
 * log_config.CreateDefaultLogger();
 * LOG_INFO() << "Log File created. Path: " << log_config.GetLogFile();
 * \endcode
 *
 */
class LogConfig final {
 public:
  LogConfig(const LogConfig &) = delete;
  LogConfig(LogConfig &&) = delete;
  LogConfig &operator=(const LogConfig &) = delete;
  LogConfig &operator=(LogConfig &&) = delete;

  /**
   * The log configuration is a singleton object that defines the default logger
   * as well as holding the actual list of loggers a.k.a. log chain.
   * @return The singleton log configuration
   */
  static LogConfig &Instance();  ///< Returns this class (singleton).

  /**
   * Create the default logger according to the settings done earlier. The
   * logger name is 'Default'.
   * @return True on success.
   */
  bool CreateDefaultLogger();  ///< Create a logger named 'Default'.

  /**
   * Remove all loggers from the application.
   */
  void DeleteLogChain();  ///< Clear the list of loggers.

  /** \brief Adds a logger with a unique name.
   *
   * Adds a known logger to the logger list. The logger should use a unique name
   * and any existing logger with the same name, will be removed. Note that
   * logger names is case insensitive.
   * @param [in] logger_name Unique name of the logger.
   * @param [in] logger Smart pointer to the new logger.
   */
  void AddLogger(const std::string &logger_name,
                 std::unique_ptr<ILogger> logger);

  /** \brief Adds a predefined logger with a unique name.
   *
   * Adds a new logger to the logger list. The logger should use a unique name
   * and any existing logger with the same name, will be removed. Note that
   * logger names is case insensitive.
   * @param [in] logger_name Unique name of the logger.
   * @param [in] type Type of logger.
   * @param [in] arg_list Argument list.
   */
  void AddLogger(const std::string &logger_name, LogType type,
                 const std::vector<std::string> &arg_list);

  /**
   * Deletes a logger by its name.
   * @param logger_name Name of logger. Note case insensitive.
   */
  void DeleteLogger(const std::string &logger_name);  ///< Deletes a logger.

  /**
   * Returns a logger bu searching of its name.
   * @param [in] logger_name Name of the logger.
   * @return Pointer to the logger. Do not delete the pointer.
   */
  ILogger *GetLogger(
      const std::string &logger_name) const;  ///< Returns a logger by name.

  /**
   * Return the log file if any exist.
   * @param [in] logger_name Name of the logger.
   * @return Returns the full path to its log file or empty string if no one
   * exist.
   */
  std::string GetLogFile(const std::string &logger_name =
                             "Default") const;  ///< Returns a log file.

  /**
   * Adds a log message to all the loggers. Note that all log messages uses this
   * function.
   * @param [in] message Message to log.
   */
  void AddLogMessage(const LogMessage &message) const;  ///< Adds a log message.

  void Type(LogType log_type);  ///< Sets the type of default logger.

  [[nodiscard]] LogType Type() const;  ///< Returns the type of default logger.

  /**
   * Sets the base name (stem) of the file name. Don't add any path or
   * extension. Note that the log file backup older log files by adding a number
   * at the end of the base name so avoid using numbers at the end of the base
   * name. Don't use spaces in file name. Use underscore instead.
   * @param [in] base_name Typical the application name is used as base name.
   */
  void BaseName(const std::string &
                    base_name);  ///< Sets the base name (stem) of the log file.
  [[nodiscard]] const std::string &BaseName() const;  ///< Returns the base name

  void Enabled(bool enabled);  ///< Turns off or on the loggers.

  [[nodiscard]] bool Enabled()
      const;  ///< Returns true if the loggers are enabled.

  void RootDir(const std::string &root_dir);  ///< Sets the root directory.
  [[nodiscard]] const std::string &RootDir()
      const;  ///< Return the root directory.

  void SubDir(const std::string &sub_dir);  ///< Sets the sub directory.
  [[nodiscard]] const std::string &SubDir()
      const;  ///< Returns the sub directory.

  /** \brief Sets the default application name.
   *
   * Some logger includes application name in their message logs. The
   * application name may not contain any spaces. UTF8 characters should be
   * avoided.
   * @param app_name Application name
   */
  void ApplicationName(const std::string &app_name) {
    application_name_ = app_name;
  }

  /** \brief Returns the default application name.
   *
   * The application name property is used by some typical
   * log related objects.
   * @return Application name.
   */
  [[nodiscard]] const std::string &ApplicationName() {
    return application_name_;
  }

 private:
  std::atomic<bool> enabled_ = true;
  std::atomic<LogType> log_type_ = LogType::LogNothing;
  std::string base_name_;
  std::string sub_dir_;
  std::string root_dir_;
  std::string application_name_;

  mutable std::mutex locker_;
  std::map<std::string, std::unique_ptr<ILogger>, util::string::IgnoreCase>
      log_chain_;

  LogConfig() = default;
  ~LogConfig();
};
}  // namespace util::log
