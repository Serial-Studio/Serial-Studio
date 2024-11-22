#pragma once

// Qt
#include <QCompleter> // Required for inheritance

/**
 * @brief Class, that describes completer with
 * glsl specific types and functions.
 */
class QLuaCompleter : public QCompleter
{
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent Pointer to parent QObject.
   */
  explicit QLuaCompleter(QObject *parent = nullptr);
};
