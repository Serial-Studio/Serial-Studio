#include <QtTest>

#include "testreader.h"
#include "teststringdata.h"
#include "testvariantdata.h"
#include "testwriter.h"

int AssertTest(QObject *obj)
{
  int status = QTest::qExec(obj);
  delete obj;

  return status;
}

int main()
{
  auto status = 0;
  status |= AssertTest(new TestStringData());
  status |= AssertTest(new TestVariantData());
  status |= AssertTest(new TestReader());
  status |= AssertTest(new TestWriter());

  return status;
}
