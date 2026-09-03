#pragma once

#include <QObject>

/**
 * @brief Sample widget used by the linter fixtures.
 */
class Sample : public QObject
{
  Q_OBJECT

public:
  Sample();

  /**
   * @brief Returns the staged row count.
   */
  [[nodiscard]] int selectionCount() const;
};
