## QsLog - the simple Qt logger ##
QsLog is an easy to use logger that is based on Qt's QDebug class. QsLog is released as open source, under the BSD license. 

###Contribution policy###
Bug fixes are welcome, larger changes however are not encouraged at this point due to the lack of time on my side for reviewing and integrating them. Your best bet in this case would be to open a ticket for your change or forking the project and implementing your change there, with the possibility of having it integrated in the future. 
All contributions will be credited, license of the contributions should be 3-clause BSD. 

### Features ###
* Six logging levels (from trace to fatal)
* Logging level threshold configurable at runtime.
* Minimum overhead when logging is turned off.
* Supports multiple destinations, comes with file and debug destinations.
* Thread-safe
* Supports logging of common Qt types out of the box.
* Small dependency: just drop it in your project directly.

### Usage ###
Directly including the QsLog source code in a project:

1. Include QsLog.pri in your pro file
2. Include QsLog.h in your C++ files. Include QsLogDest.h only where you create/add destinations.
3. Get the instance of the logger by calling QsLogging::Logger::instance();
4. Optionally set the logging level. Info is default.
5. Create as many destinations as you want by using the QsLogging::DestinationFactory.
6. Add the destinations to the logger instance by calling addDestination.
7. Start logging!

Linking to QsLog dynamically:

1. Build QsLog using the QsLogSharedLibrary.pro.
2. Add the QsLog shared library to your LIBS project dependencies.
3. Follow the steps in "directly including QsLog in your project" starting with step 2.

Note: when you want to use QsLog both from an executable and a shared library you have to
      link QsLog dynamically due to a limitation with static variables.

### Configuration ###
QsLog has several configurable parameters in its .pri file:

* defining QS_LOG_LINE_NUMBERS in the .pri file enables writing the file and line number
  automatically for each logging call
* defining QS_LOG_SEPARATE_THREAD will route all log messages to a separate thread.
* defining QS_LOG_WIN_PRINTF_CONSOLE will use fprintf instead of OutputDebugString on Windows

Sometimes it's necessary to turn off logging. This can be done in several ways:

* globally, at compile time, by enabling the QS_LOG_DISABLE macro in the supplied .pri file.
* globally, at run time, by setting the log level to "OffLevel".
* per file, at compile time, by including QsLogDisableForThisFile.h in the target file.

### Thread safety ###
The Qt docs say: 
A **thread-safe** function can be called simultaneously from multiple threads, even when the invocations use shared data, because all references to the shared data are serialized.
A **reentrant** function can also be called simultaneously from multiple threads, but only if each invocation uses its own data.

Since sending the log message to the destinations is protected by a mutex, the logging macros are thread-safe provided that the log has been initialized - i.e: instance() has been called. Adding and removing destinations and the instance function are likewise thread-safe.
The setup functions (e.g: setLoggingLevel) are NOT thread-safe and are NOT reentrant, they must be called at program startup.