#include <QtTest>

#include "teststringdata.h"
#include "testvariantdata.h"
#include "testreader.h"
#include "testwriter.h"

int AssertTest(QObject* obj)
{
    int status = QTest::qExec(obj);
    delete obj;

    return status;
}

int main()
{
    int status = 0;
    status |= AssertTest(new TestStringData());
    status |= AssertTest(new TestVariantData());
    status |= AssertTest(new TestReader());
    status |= AssertTest(new TestWriter());

    return status;
}
