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
  alignas(64) std::atomic<int> m_head;
  alignas(64) std::atomic<int> m_tail;
};
