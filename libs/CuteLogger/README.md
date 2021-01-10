# CuteLogger
[![Build Status](https://travis-ci.org/dept2/CuteLogger.svg?branch=master)](https://travis-ci.org/dept2/CuteLogger)

Simple, convinient and thread safe logger for Qt-based C++ apps

## Features

- Logs pretty much everything: file name, source line, function signature
- Flexible appender system: log to [file](http://dept2.github.io/CuteLogger/class_file_appender.html), [console](http://dept2.github.io/CuteLogger/class_console_appender.html) or even Android logcat, add custom appenders, customize output format
- Compatible with Qt builtin types. Can be used as a drop-in replacement for qDebug etc.
- Supports [measuring timing](http://dept2.github.io/CuteLogger/_logger_8h.html#af018c228deac1e43b4428889b89dbd13) for an operations
- Supports [log categories](http://dept2.github.io/CuteLogger/_logger_8h.html#a5a9579044777cb4de48b4dbb416b007d), able to log all messages from a class/namespace to custom category
- Thread safe

## Documentation
[Doxygen docs available](http://dept2.github.io/CuteLogger/)

## Short example

```cpp
#include <QCoreApplication>
#include <Logger.h>
#include <ConsoleAppender.h>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  ...
  ConsoleAppender* consoleAppender = new ConsoleAppender;
  consoleAppender->setFormat("[%{type:-7}] <%{Function}> %{message}\n");
  cuteLogger->registerAppender(consoleAppender);
  ...
  LOG_INFO("Starting the application");
  int result = app.exec();
  ...
  if (result)
    LOG_WARNING() << "Something went wrong." << "Result code is" << result;
  return result;
}
```

## Adding CuteLogger to your project

Add this repo as a git submodule to your project. 

```bash
git submodule add git@github.com:dept2/CuteLogger.git CuteLogger
```

Include it to your CMakeLists.txt file:
```cmake
...
ADD_SUBDIRECTORY(Logger)
...
TARGET_LINK_LIBRARIES(${your_target} ... CuteLogger)
```

Include `Logger.h` and one or several appenders of your choice:
```cpp
#include <Logger.h>
#include <ConsoleAppender.h>
```
