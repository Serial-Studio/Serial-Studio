#ifndef TIMER_MOCK_H
#define TIMER_MOCK_H

#include <qmqtt_timerinterface.h>
#include <gmock/gmock.h>

class TimerMock : public QMQTT::TimerInterface
{
public:
  MOCK_CONST_METHOD0(isSingleShot, bool());
  MOCK_METHOD1(setSingleShot, void(bool));
  MOCK_CONST_METHOD0(interval, int());
  MOCK_METHOD1(setInterval, void(int));
  MOCK_METHOD0(start, void());
  MOCK_METHOD0(stop, void());
};

#endif // TIMER_MOCK_H
