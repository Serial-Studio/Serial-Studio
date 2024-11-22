#pragma once

// Qt
#include <QCompleter> // Required for inheritance

/**
 * @brief Class, that describes completer with Javascript
 * specific types and functions.
 */
class QJavascriptCompleter : public QCompleter
{
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent Pointer to parent QObject.
   */
  explicit QJavascriptCompleter(QObject *parent = nullptr);
};
