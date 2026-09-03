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

private:
  std::atomic<int> m_head;
  std::atomic<int> m_tail;
};
